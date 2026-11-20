#include "MGGuardActor.h"

#include "MGPlayerCharacter.h"
#include "MGGrayFoxMission.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "mg_legacy_core.h"
#include "EngineUtils.h"

// The perception model may classify an audible footfall as suspicious; only
// a fresh sight confirmation authorises aimed close-range damage.

AMGGuardActor::AMGGuardActor()
{
    PrimaryActorTick.bCanEverTick = true;
    CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
    CollisionCapsule->InitCapsuleSize(35.f, 89.f);
    CollisionCapsule->SetCollisionProfileName(TEXT("Pawn"));
    CollisionCapsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    RootComponent = CollisionCapsule;
    ProxyBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuardBody"));
    ProxyBody->SetupAttachment(RootComponent);
    // Match the 176 cm player proxy; guard feet sit on the prototype floor.
    ProxyBody->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.76f));
    ProxyBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProxyBody->SetCollisionResponseToAllChannels(ECR_Ignore);
    ProxyBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (Cylinder.Succeeded()) ProxyBody->SetStaticMesh(Cylinder.Object);

    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GuardSkeletalBody"));
    CharacterMesh->SetupAttachment(RootComponent);
    CharacterMesh->SetRelativeLocation(FVector(0.f, 0.f, -89.f));
    CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMGGuardActor::BeginPlay()
{
    Super::BeginPlay();
    PatrolOrigin = GetActorLocation();
    StationFacing = GetActorRotation();
    if (!CharacterMesh->GetSkeletalMeshAsset())
        if (USkeletalMesh* Asset = LoadObject<USkeletalMesh>(nullptr,
                TEXT("/Game/Characters/Guard/SK_Guard_1995.SK_Guard_1995")))
            CharacterMesh->SetSkeletalMeshAsset(Asset);
    if (HasAuthoredCharacterArt())
    {
        if (!CharacterMesh->GetAnimClass())
            if (UClass* Class = LoadClass<UAnimInstance>(nullptr,
                    TEXT("/Game/Characters/Guard/ABP_Guard_1995.ABP_Guard_1995_C")))
                CharacterMesh->SetAnimInstanceClass(Class);
        ProxyBody->SetVisibility(false, true);
        ProxyBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CharacterMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
        CharacterMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
}

void AMGGuardActor::ConfigurePatrol(const FVector& Axis,
                                    float HalfLengthCm, float SpeedCmPerSec)
{
    PatrolAxis = Axis.GetSafeNormal2D();
    if (PatrolAxis.IsNearlyZero()) PatrolAxis = FVector::ForwardVector;
    PatrolHalfLength = FMath::Clamp(HalfLengthCm, 60.f, 320.f);
    PatrolSpeed = FMath::Clamp(SpeedCmPerSec, 45.f, 130.f);
}

bool AMGGuardActor::HasAuthoredCharacterArt() const
{
    return CharacterMesh && CharacterMesh->GetSkeletalMeshAsset() != nullptr;
}

bool AMGGuardActor::CanAcceptDetention(const AMGPlayerCharacter* Suspect) const
{
    if (Role != EMGGuardRole::Custody || bIncapacitated || !bCustodyChallenge ||
        !IsValid(Suspect) || Suspect->IsAiming() || Suspect->IsSprinting()
        || Suspect->IsProne() || ChallengeAge < .75f
        || CustodyThreatMemorySeconds > 0.f || AwarenessState >= 3)
        return false;
    if (FVector::Dist2D(GetActorLocation(), Suspect->GetActorLocation()) > 620.f)
        return false;
    // Unarmed/procedurally compliant suspect. The custody officer has an
    // observable legal reason to detain, not foreknowledge of Snake's plan.
    FHitResult Hit;
    FCollisionQueryParams Trace(SCENE_QUERY_STAT(MGCustodyLOS), false, this);
    const bool bHit = GetWorld() && GetWorld()->LineTraceSingleByChannel(
        Hit, GetActorLocation() + FVector(0,0,65.f),
        Suspect->GetActorLocation() + FVector(0,0,65.f), ECC_Visibility, Trace);
    return !bHit || Hit.GetActor() == Suspect;
}

void AMGGuardActor::EmitGuardCallout(FName EventId, const FString& Caption)
{
    ActiveGuardCallout = Caption;
    CalloutRemaining = 2.6f;
    // Hook to replace provisional subtitles with authored 1995 tactical
    // performance, subtitled VO and sound occlusion. Caption remains optional
    // for the player but is never a hidden mission-state trigger.
    OnGuardCallout(EventId);
}

void AMGGuardActor::SetSecurityPhase(EMGGuardAwarenessPhase NewPhase)
{
    if (SecurityPhase == NewPhase) return;
    const EMGGuardAwarenessPhase Old = SecurityPhase;
    SecurityPhase = NewPhase;
    TimeInSecurityState = 0.f;
    OnAwarenessPhaseChanged(Old, NewPhase, EvidenceKind);
    if (NewPhase == EMGGuardAwarenessPhase::Suspicious &&
        (EvidenceKind == EMGGuardEvidenceKind::HeardMovement ||
         EvidenceKind == EMGGuardEvidenceKind::StructuralImpact))
        EmitGuardCallout(TEXT("Question"), TEXT("GUARD: WHAT WAS THAT?"));
    else if (NewPhase == EMGGuardAwarenessPhase::Investigating)
        EmitGuardCallout(TEXT("Investigate"), TEXT("GUARD: CHECK THAT NOISE."));
    else if (NewPhase == EMGGuardAwarenessPhase::Engaged)
        EmitGuardCallout(TEXT("Contact"), TEXT("GUARD: INTRUDER!"));
    else if (NewPhase == EMGGuardAwarenessPhase::Searching &&
             Old == EMGGuardAwarenessPhase::Engaged)
        EmitGuardCallout(TEXT("LostSight"), TEXT("GUARD: LOST VISUAL. SEARCH THE AREA."));
    else if (NewPhase == EMGGuardAwarenessPhase::Returning)
        EmitGuardCallout(TEXT("Return"), TEXT("GUARD: BACK TO POST."));
}

void AMGGuardActor::ObserveEvidence(EMGGuardEvidenceKind Kind, const FVector& Location)
{
    EvidenceKind = Kind;
    EvidenceAge = 0.f;
    LastKnownPlayerPosition = Location; // search destination; may be a BODY, not a Snake sighting
}

void AMGGuardActor::ObserveApproximateFootstep(const FVector& AudibleSource, bool bOccluded)
{
    // A continuous movement/noise factor is not a continuous GPS fix.
    // Sample a rough bearing and distance no more often than the refresh window.
    if (FootstepLocalizationRefreshSeconds > 0.f) return;
    // Direct visual proof and urgent concrete discoveries retain priority:
    // an innocuous footstep cannot erase a confirmed sighting or body report.
    if ((EvidenceKind == EMGGuardEvidenceKind::ObservedIntruder &&
         ContactEvidenceAge < 4.5f) ||
        (EvidenceKind == EMGGuardEvidenceKind::SquadSighting && EvidenceAge < 4.f) ||
        ((EvidenceKind == EMGGuardEvidenceKind::DiscoveredBody ||
          EvidenceKind == EMGGuardEvidenceKind::SquadBodyReport) && EvidenceAge < 6.f) ||
        (EvidenceKind == EMGGuardEvidenceKind::StructuralImpact && EvidenceAge < 3.f))
        return;

    const FVector Relative = AudibleSource - GetActorLocation();
    const float Range = Relative.Size2D();
    if (Range < SMALL_NUMBER) return;
    const float BearingStep = FMath::DegreesToRadians(bOccluded ? 35.f : 20.f);
    const float RangeStep = bOccluded ? 260.f : 170.f; // cm, provisional
    const float Bearing = FMath::RoundToFloat(
        FMath::Atan2(Relative.Y, Relative.X) / BearingStep) * BearingStep;
    const float EstimatedRange = FMath::Max(
        65.f, FMath::RoundToFloat(Range / RangeStep) * RangeStep);
    const FVector ApproximateLocation = GetActorLocation() + FVector(
        FMath::Cos(Bearing) * EstimatedRange,
        FMath::Sin(Bearing) * EstimatedRange, 0.f);
    ObserveEvidence(EMGGuardEvidenceKind::HeardMovement, ApproximateLocation);
    FootstepLocalizationRefreshSeconds = bOccluded ? 2.2f : 1.2f;
}

void AMGGuardActor::SetGuardPosture(EMGGuardPosture Desired)
{
    if (PresentedPosture == Desired) return;
    const EMGGuardPosture Old = PresentedPosture;
    PresentedPosture = Desired;
    OnPostureChanged(Old, Desired);
}

void AMGGuardActor::ReceiveEnvironmentalReport(const FVector& EvidenceAt, const AActor* Sender)
{
    if (bIncapacitated || !IsValid(Sender) || AwarenessState == 3 ||
        FVector::Dist2D(GetActorLocation(), Sender->GetActorLocation()) > 950.f)
        return;
    // This is the location of a discovered *body* or disturbed environment,
    // NOT a report of Snake's current or last directly seen coordinates.
    ObserveEvidence(EMGGuardEvidenceKind::SquadBodyReport, EvidenceAt);
    Suspicion = FMath::Max(Suspicion, .72f);
    AwarenessState = FMath::Max(AwarenessState, 2u);
    SearchForLastKnownSeconds = FMath::Max(SearchForLastKnownSeconds, 5.5f);
    SetGuardPosture(EMGGuardPosture::Search);
}

void AMGGuardActor::CheckVisibleIncapacitatedColleagues()
{
    if (bIncapacitated || !GetWorld()) return;
    // Small slice population; evaluate infrequently, and do not see a body
    // through walls, around the back of the head or from unlimited range.
    for (TActorIterator<AMGGuardActor> Other(GetWorld()); Other; ++Other)
    {
        AMGGuardActor* Colleague = *Other;
        if (Colleague == this || !Colleague->IsIncapacitated() ||
            SeenIncapacitatedColleagues.Contains(Colleague))
            continue;
        const FVector ToBody = Colleague->GetActorLocation() - GetActorLocation();
        const float Range = ToBody.Size2D();
        const bool bNearby = Range < 180.f;
        if (Range > 720.f || (!bNearby && FVector::DotProduct(
                GetActorForwardVector().GetSafeNormal2D(),
                ToBody.GetSafeNormal2D()) < 0.42f))
            continue;
        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(MGBodyEvidenceLOS), false, this);
        const bool bOccluded = GetWorld()->LineTraceSingleByChannel(
            Hit, GetActorLocation() + FVector(0,0,65.f),
            Colleague->GetActorLocation() + FVector(0,0,45.f),
            ECC_Visibility, Params);
        if (bOccluded && Hit.GetActor() != Colleague) continue;
        SeenIncapacitatedColleagues.Add(Colleague);
        ObserveEvidence(EMGGuardEvidenceKind::DiscoveredBody, Colleague->GetActorLocation());
        Suspicion = FMath::Max(Suspicion, .78f);
        AwarenessState = FMath::Max(AwarenessState, 2u);
        SearchForLastKnownSeconds = FMath::Max(SearchForLastKnownSeconds, 7.f);
        BodyReportLocation = Colleague->GetActorLocation();
        BodyReportDelay = 1.2f;
        OnBodyDiscovered(BodyReportLocation);
        EmitGuardCallout(TEXT("BodyDiscovered"), TEXT("GUARD: MAN DOWN!"));
        SetGuardPosture(EMGGuardPosture::Investigate);
        break; // One discovery / report action per brief scan.
    }
}

void AMGGuardActor::ReceiveLocalSquadReport(const FVector& SeenAt, const AActor* Sender)
{
    if (bIncapacitated || AwarenessState >= 3 || !IsValid(Sender)
        || FVector::Dist2D(GetActorLocation(), Sender->GetActorLocation()) > 950.f)
        return;
    // Radio shares last *observed* location only; it does not mark the player
    // in a world-state oracle, force precise lock, or grant through-wall hits.
    ObserveEvidence(EMGGuardEvidenceKind::SquadSighting, SeenAt);
    SearchForLastKnownSeconds = 4.0f;
    Suspicion = FMath::Max(Suspicion, .66f);
    AwarenessState = FMath::Max(AwarenessState, 2u);
}

void AMGGuardActor::HearWallKnock(const FVector& SoundLocation, const FVector& WallNormal, float HearingRadius, AActor* SoundInstigator)
{
    if (bIncapacitated || !GetWorld() || AwarenessState >= 3) return;
    // A discovered body is serious evidence. A random subsequent knock must
    // not delete the guard's ongoing security investigation of a casualty.
    if ((EvidenceKind == EMGGuardEvidenceKind::DiscoveredBody ||
         EvidenceKind == EMGGuardEvidenceKind::SquadBodyReport) &&
        EvidenceAge < 5.f)
        return;
    const float Distance = FVector::Dist2D(GetActorLocation(), SoundLocation);
    if (Distance > HearingRadius) return;
    const FVector GuardSide = (GetActorLocation() - SoundLocation).GetSafeNormal2D();

    // Concrete obstacles attenuate a knock. A guard hears a location, not a
    // magically transmitted player transform. The player isn't acoustic cover.
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MGKnockOcclusion), false, this);
    if (IsValid(SoundInstigator)) Query.AddIgnoredActor(SoundInstigator);
    FHitResult Obstacle;
    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        Obstacle, GetActorLocation() + FVector(0.f, 0.f, 55.f),
        SoundLocation + FVector(0.f, 0.f, 18.f), ECC_Visibility, Query);
    const bool bSourceWallOccludes = FVector::DotProduct(GuardSide, WallNormal) < -0.10f;
    const bool bOtherObstacle = bBlocked &&
        FVector::Dist(Obstacle.ImpactPoint, SoundLocation + FVector(0.f, 0.f, 18.f)) > 28.f;
    if (Distance > HearingRadius * ((bSourceWallOccludes || bOtherObstacle) ? 0.55f : 1.f)) return;

    // Repeated knocks renew attention, not instant omniscient combat alert.
    ConsecutiveKnocks = DistractionPhase == EDistractionPhase::None ?
        0 : FMath::Min(ConsecutiveKnocks + 1, 3);
    DistractionLocation = SoundLocation + GuardSide * 90.f;
    DistractionLocation.Z = GetActorLocation().Z;
    ObserveEvidence(EMGGuardEvidenceKind::StructuralImpact, DistractionLocation);
    // Return to the actual duty route, including Y-axis patrols. The older
    // X-only projection stranded northern/southern corridor patrols away
    // from their assigned sweep after checking a noise.
    const float RouteOffset = FVector::DotProduct(
        GetActorLocation() - PatrolOrigin, PatrolAxis);
    ReturnToPatrol = PatrolOrigin + PatrolAxis *
        FMath::Clamp(RouteOffset, -PatrolHalfLength, PatrolHalfLength);
    DistractionPhase = EDistractionPhase::React;
    DistractionTimer = FMath::Max(0.31f, 0.52f - ConsecutiveKnocks * 0.07f);
    InvestigationStuckTimer = 0.f;
    Suspicion = FMath::Max(Suspicion, 0.24f);
    AwarenessState = FMath::Max(AwarenessState, 1u);
}

void AMGGuardActor::MoveToward(const FVector& Destination, float Speed, float DeltaSeconds)
{
    if (DeltaSeconds <= SMALL_NUMBER) return;
    const FVector Before = GetActorLocation();
    const FVector Delta = Destination - Before;
    const float Distance = Delta.Size2D();
    if (Distance < 1.f)
    {
        SmoothedGroundVelocity = FMath::VInterpTo(
            SmoothedGroundVelocity, FVector::ZeroVector, DeltaSeconds, 9.f);
        ActualGroundSpeedCmPerSec = 0.f;
        return;
    }
    const FVector Direction = Delta.GetSafeNormal2D();
    // Actual acceleration/braking rather than snapping directly to full speed
    // or flipping 180 degrees while the body remains standing still.
    const float DesiredSpeed = FMath::Min(
        FMath::Max(0.f, Speed), Distance * 4.5f);
    SmoothedGroundVelocity = FMath::VInterpTo(
        SmoothedGroundVelocity, Direction * DesiredSpeed, DeltaSeconds, 6.f);
    SetActorLocation(Before + SmoothedGroundVelocity * DeltaSeconds, true);
    const FVector Travel = GetActorLocation() - Before;
    ActualGroundSpeedCmPerSec = Travel.Size2D() / DeltaSeconds;
    if (Travel.Size2D() < 0.2f)
        SmoothedGroundVelocity = FMath::VInterpTo(
            SmoothedGroundVelocity, FVector::ZeroVector, DeltaSeconds, 10.f);
    if (Travel.Size2D() > 0.2f)
        SetActorRotation(FMath::RInterpTo(
            GetActorRotation(), Travel.GetSafeNormal2D().Rotation(),
            DeltaSeconds, 5.5f));
}

bool AMGGuardActor::AdvanceNoiseInvestigation(float DeltaSeconds)
{
    if (DistractionPhase == EDistractionPhase::None) return false;
    // Direct visual confirmation supersedes noise. A guard who is merely
    // searching a body or responding to a radio report must NOT forget the
    // local sound stimulus just because numeric awareness reached 2.
    if ((bConfirmedVisualThisFrame && AwarenessState >= 3) ||
        ((EvidenceKind == EMGGuardEvidenceKind::DiscoveredBody ||
          EvidenceKind == EMGGuardEvidenceKind::SquadBodyReport) &&
         EvidenceAge < 5.f))
    {
        DistractionPhase = EDistractionPhase::None;
        ConsecutiveKnocks = 0;
        return false;
    }
    Suspicion = FMath::Max(Suspicion, 0.24f);
    AwarenessState = FMath::Max(AwarenessState, 1u);
    if (DistractionPhase == EDistractionPhase::React)
    {
        const FVector Direction = (DistractionLocation - GetActorLocation()).GetSafeNormal2D();
        if (!Direction.IsNearlyZero())
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaSeconds, 2.8f));
        DistractionTimer -= DeltaSeconds;
        if (DistractionTimer <= 0.f)
        {
            DistractionPhase = EDistractionPhase::Approach;
            InvestigationStuckTimer = 0.f;
        }
    }
    else if (DistractionPhase == EDistractionPhase::Approach)
    {
        const float Before = FVector::Dist2D(DistractionLocation, GetActorLocation());
        MoveToward(DistractionLocation, 115.f, DeltaSeconds);
        const float After = FVector::Dist2D(DistractionLocation, GetActorLocation());
        InvestigationStuckTimer = After >= Before - 0.5f ?
            InvestigationStuckTimer + DeltaSeconds : 0.f;
        if (After < 82.f || InvestigationStuckTimer > 1.5f)
        {
            DistractionPhase = EDistractionPhase::Search;
            DistractionTimer = 4.5f + 1.25f * ConsecutiveKnocks;
        }
    }
    else if (DistractionPhase == EDistractionPhase::Search)
    {
        const FVector Direction = (DistractionLocation - GetActorLocation()).GetSafeNormal2D();
        const float BaseYaw = Direction.IsNearlyZero() ? GetActorRotation().Yaw : Direction.Rotation().Yaw;
        const float Sweep = 32.f * FMath::Sin(GetWorld()->GetTimeSeconds() * 2.2f);
        SetActorRotation(FRotator(0.f, BaseYaw + Sweep, 0.f));
        DistractionTimer -= DeltaSeconds;
        if (DistractionTimer <= 0.f) DistractionPhase = EDistractionPhase::Return;
    }
    else if (DistractionPhase == EDistractionPhase::Return)
    {
        const float Before = FVector::Dist2D(ReturnToPatrol, GetActorLocation());
        MoveToward(ReturnToPatrol, 90.f, DeltaSeconds);
        const float After = FVector::Dist2D(ReturnToPatrol, GetActorLocation());
        InvestigationStuckTimer = After >= Before - 0.5f ?
            InvestigationStuckTimer + DeltaSeconds : 0.f;
        // A blocked path cannot strand the actor indefinitely in Return.
        if (After < 28.f || InvestigationStuckTimer > 2.0f)
        {
            DistractionPhase = EDistractionPhase::None;
            ConsecutiveKnocks = 0;
            InvestigationStuckTimer = 0.f;
        }
    }
    return true;
}

float AMGGuardActor::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                                AController* EventInstigator, AActor* DamageCauser)
{
    const float Accepted = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (DamageAmount > 0.f && !bIncapacitated)
    {
        AttackWindupSeconds = 0.f; // A genuine interruption cancels a strike.
        // Getting hit during the half-second radio procedure cancels the
        // transmission rather than broadcasting from an interrupted posture.
        if (bPendingRadioReport)
        {
            bPendingRadioReport = false;
            PendingRadioDelay = 0.f;
            SquadBroadcastCooldown = FMath::Min(SquadBroadcastCooldown, 1.5f);
        }
        Health = FMath::Max(0.f, Health - DamageAmount);
        StaggerSeconds = 0.22f;
        // A victim can react and report contact; damage never silently vanishes.
        Suspicion = 1.f;
        AwarenessState = 3;
        if (DamageCauser)
            ObserveEvidence(EMGGuardEvidenceKind::Wounded, DamageCauser->GetActorLocation());
        if (Health <= 0.f)
        {
            bIncapacitated = true;
            SetGuardPosture(EMGGuardPosture::Incapacitated);
            SetSecurityPhase(EMGGuardAwarenessPhase::Incapacitated);
            Suspicion = 0.f;
            AwarenessState = 0;
            ProxyBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            // Until a KO/death montage is authored, preserve a readable fallback.
            (HasAuthoredCharacterArt() ? static_cast<USceneComponent*>(CharacterMesh)
                : static_cast<USceneComponent*>(ProxyBody))->SetRelativeRotation(
                    FRotator(0.f, 0.f, 80.f));
            SetActorTickEnabled(false);
        }
    }
    return Accepted;
}

void AMGGuardActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bIncapacitated || !GetWorld()) return;
    TimeInSecurityState += FMath::Max(0.f, DeltaSeconds);
    CalloutRemaining = FMath::Max(0.f, CalloutRemaining - FMath::Max(0.f, DeltaSeconds));
    EvidenceAge = FMath::Min(120.f, EvidenceAge + FMath::Max(0.f, DeltaSeconds));
    FootstepLocalizationRefreshSeconds = FMath::Max(
        0.f, FootstepLocalizationRefreshSeconds - FMath::Max(0.f, DeltaSeconds));
    bConfirmedVisualThisFrame = false;
    StaggerSeconds = FMath::Max(0.f, StaggerSeconds - DeltaSeconds);
    // No movement in this frame unless the state actually produces a step.
    // An AnimBP can use actual capsule travel speed, not a phantom target
    // velocity that animates running feet against an obstructing wall.
    ActualGroundSpeedCmPerSec = FMath::FInterpTo(
        ActualGroundSpeedCmPerSec, 0.f, DeltaSeconds, 8.f);
    AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaSeconds);
    SquadBroadcastCooldown = FMath::Max(0.f, SquadBroadcastCooldown - DeltaSeconds);
    CustodyThreatMemorySeconds = FMath::Max(0.f,
        CustodyThreatMemorySeconds - DeltaSeconds);
    SearchForLastKnownSeconds = FMath::Max(0.f,
        SearchForLastKnownSeconds - DeltaSeconds);
    // A body report is a separate type of intelligence from confirmed enemy
    // contact. Nearby colleagues investigate the evidence, not Snake's GPS.
    if (BodyReportDelay >= 0.f)
    {
        BodyReportDelay -= DeltaSeconds;
        if (BodyReportDelay < 0.f)
        {
            for (TActorIterator<AMGGuardActor> Ally(GetWorld()); Ally; ++Ally)
                if (*Ally != this)
                    Ally->ReceiveEnvironmentalReport(BodyReportLocation, this);
            if (AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
                    UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass())))
                Mission->NotifyGuardBodyReported(BodyReportLocation);
        }
    }
    BodyScanCountdown -= DeltaSeconds;
    if (BodyScanCountdown <= 0.f)
    {
        BodyScanCountdown = 0.70f;
        CheckVisibleIncapacitatedColleagues();
    }
    // Do not counter-rotate an authored skeletal rig: its AnimBP owns hit reactions.
    if (!HasAuthoredCharacterArt())
        ProxyBody->SetRelativeRotation(FMath::RInterpTo(ProxyBody->GetRelativeRotation(),
            StaggerSeconds > 0.f ? FRotator(0.f, 0.f, 12.f) : FRotator::ZeroRotator,
            DeltaSeconds, 9.f));
    const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    const float Distance = ToPlayer.Size2D();

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MGGuardVisibility), false, this);
    const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, 55.f);
    const FVector Target = Player->GetActorLocation() + FVector(0.f, 0.f, 55.f);
    const bool bTraceHit = GetWorld()->LineTraceSingleByChannel(Hit, Eye, Target, ECC_Visibility, Params);
    const bool bHasLineOfSight = !bTraceHit || Hit.GetActor() == Player;
    const AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
        UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass()));
    const EMGFacilitySecurityState BaseState = Mission
        ? Mission->GetFacilitySecurityState() : EMGFacilitySecurityState::Normal;
    const bool bFacilityLockdown = BaseState == EMGFacilitySecurityState::Alert ||
        BaseState == EMGFacilitySecurityState::Evasion;
    const bool bFacilityCaution = bFacilityLockdown ||
        BaseState == EMGFacilitySecurityState::Caution;

    // The officers at the detention truck CHALLENGE instead of automatically
    // attacking a compliant nearby suspect. Challenge requires first-hand
    // sight and is revoked if the suspect retreats behind geometry.
    if (Role == EMGGuardRole::Custody && !bIncapacitated)
    {
        const bool bNear = Distance < 690.f;
        const bool bThreatening = Player->IsAiming() || Player->IsSprinting();
        if (bNear && bHasLineOfSight && bThreatening)
            CustodyThreatMemorySeconds = 3.5f;
        const bool bCanChallenge = bNear && bHasLineOfSight
            && !bThreatening && !Player->IsProne()
            && CustodyThreatMemorySeconds <= 0.f && AwarenessState < 3
            && !bFacilityLockdown;
        const bool bFirstChallengeFrame = bCanChallenge && !bCustodyChallenge;
        bCustodyChallenge = bCanChallenge;
        if (bCustodyChallenge)
        {
            SetSecurityPhase(EMGGuardAwarenessPhase::Confirming);
            SetGuardPosture(EMGGuardPosture::Challenge);
            AttackWindupSeconds = 0.f;
            ChallengeAge += DeltaSeconds;
            if (bFirstChallengeFrame)
            {
                OnCustodyChallenge();
                EmitGuardCallout(TEXT("CustodyChallenge"),
                    TEXT("SECURITY: HALT. KEEP YOUR HANDS VISIBLE."));
            }
            const FVector Direction = ToPlayer.GetSafeNormal2D();
            if (!Direction.IsNearlyZero())
                SetActorRotation(FMath::RInterpTo(GetActorRotation(),
                    Direction.Rotation(), DeltaSeconds, 3.5f));
            AwarenessState = 1;
            Suspicion = FMath::Max(Suspicion, .35f);
            // No damage, chase or magical confiscation on this frame. The
            // player commits compliance via physical custody handoff.
            return;
        }
        ChallengeAge = 0.f;
    }
    const FVector Forward = GetActorForwardVector();

    MgGuardFrame Frame = {};
    Frame.guard_pos = {GetActorLocation().X / 100.f, GetActorLocation().Y / 100.f};
    Frame.guard_forward = {Forward.X, Forward.Y};
    Frame.player_pos = {Player->GetActorLocation().X / 100.f, Player->GetActorLocation().Y / 100.f};
    // Elevated security means more deliberate attention, not omniscient AI.
    // Rust and editor fallback read the SAME actual tuned sensor envelope.
    Frame.vision_range_m = bFacilityLockdown ? 13.2f :
        (bFacilityCaution ? 12.6f : 12.f);
    Frame.vision_half_angle_deg = bFacilityLockdown ? 60.f : 55.f;
    Frame.hearing_range_m = bFacilityLockdown ? 12.2f : 11.f;
    Frame.visibility = Player->GetVisibilityFactor();
    // A footstep can still transmit through walls, but physical barriers
    // reduce its useful evidence radius instead of creating an x-ray acoustic
    // tracker. Direct visual proof remains a separate channel.
    Frame.noise = Player->GetNoiseFactor() *
        (bHasLineOfSight ? 1.f : .42f);
    Frame.suspicion = Suspicion;
    Frame.delta_seconds = DeltaSeconds;
    Frame.line_of_sight = bHasLineOfSight ? 1u : 0u;

    MgGuardResult Result = {};
    bool bConfirmedVisualNow = false;
#if MG_WITH_RUST
    if (mg_core_abi_version() == 1 && mg_step_guard(&Frame, &Result))
    {
        Suspicion = Result.suspicion;
        AwarenessState = Result.state;
        bConfirmedVisualNow = Result.sees_player != 0 && bHasLineOfSight;
        if (bConfirmedVisualNow)
        {
            ObserveEvidence(EMGGuardEvidenceKind::ObservedIntruder,
                Player->GetActorLocation());
            ContactEvidenceAge = 0.f;
        }
        else if (Result.hears_player)
        {
            ObserveApproximateFootstep(Player->GetActorLocation(), !bHasLineOfSight);
        }
    }
#else
    // Reduced fallback for editor prototyping only; use the Rust native ABI
    // for the full independently tested perception implementation.
    const float FacingThreshold =
        FMath::Cos(FMath::DegreesToRadians(Frame.vision_half_angle_deg));
    const bool bFacing = FVector::DotProduct(
        Forward.GetSafeNormal2D(), ToPlayer.GetSafeNormal2D()) > FacingThreshold;
    const bool bVisible = bHasLineOfSight && bFacing &&
        Distance < Frame.vision_range_m * 100.f;
    const bool bAudible = Distance < Frame.hearing_range_m * 100.f &&
        Frame.noise > 0.1f;
    Suspicion = FMath::Clamp(Suspicion + DeltaSeconds *
        ((bVisible ? Frame.visibility * 0.95f : 0.f) +
         (bAudible ? Frame.noise * 0.40f : 0.f) -
         ((!bVisible && !bAudible) ? 0.12f : 0.f)), 0.f, 1.f);
    AwarenessState = Suspicion >= 0.99f ? 3 : Suspicion >= 0.68f ? 2 : Suspicion >= 0.22f ? 1 : 0;
    bConfirmedVisualNow = bVisible;
    if (bConfirmedVisualNow)
    {
        ObserveEvidence(EMGGuardEvidenceKind::ObservedIntruder,
            Player->GetActorLocation());
        ContactEvidenceAge = 0.f;
    }
    else if (bAudible)
    {
        ObserveApproximateFootstep(Player->GetActorLocation(), !bHasLineOfSight);
    }
#endif

    // Only actual first-hand sight refreshes contact age. Repeated muffled
    // footsteps may renew a local search, not extend visual combat privilege.
    ContactEvidenceAge = FMath::Min(ContactEvidenceAge + DeltaSeconds, 100.f);
    bConfirmedVisualThisFrame = bConfirmedVisualNow;
    VisualConfirmationSeconds = bConfirmedVisualNow
        ? FMath::Min(VisualConfirmationSeconds + DeltaSeconds, 1.5f) : 0.f;
    // The solver's fast decay must not erase explicit squad/body
    // investigation. It is *evidence* that fades, not hidden combat privilege.
    if (SearchForLastKnownSeconds > 0.f && AwarenessState < 2)
    {
        AwarenessState = 2;
        Suspicion = FMath::Max(Suspicion, .68f);
    }
    if (AwarenessState == 3 && !bConfirmedVisualNow && ContactEvidenceAge > 4.5f)
    {
        AwarenessState = 2;
        Suspicion = FMath::Min(Suspicion, .67f);
        SearchForLastKnownSeconds = FMath::Max(SearchForLastKnownSeconds, 3.f);
    }
    // Half-second radio call, only on actually evidenced fresh contact. Nearby
    // colleagues receive last known position, not Snake's live world transform.
    if (AwarenessState == 3 && bConfirmedVisualNow
        && SquadBroadcastCooldown <= 0.f && !bPendingRadioReport)
    {
        bPendingRadioReport = true;
        PendingRadioDelay = .6f;
        // Snapshot only what was actually SEEN when the report began. Later
        // sound or body evidence cannot silently replace this radio payload.
        PendingConfirmedRadioSighting = Player->GetActorLocation();
        SquadBroadcastCooldown = 9.f;
    }
    if (bPendingRadioReport)
    {
        PendingRadioDelay -= DeltaSeconds;
        if (PendingRadioDelay <= 0.f)
        {
            bPendingRadioReport = false;
            OnContactReported(PendingConfirmedRadioSighting);
            EmitGuardCallout(TEXT("RadioContact"),
                TEXT("GUARD RADIO: HOSTILE CONTACT. REQUEST SUPPORT."));
            if (AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
                    UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass())))
                Mission->NotifyGuardContactReported(PendingConfirmedRadioSighting);
            for (TActorIterator<AMGGuardActor> Ally(GetWorld()); Ally; ++Ally)
                if (*Ally != this)
                    Ally->ReceiveLocalSquadReport(PendingConfirmedRadioSighting, this);
        }
    }
    // Close combat cannot fire the same tick a guard becomes alerted.
    // A real face-on visual lock starts a readable wind-up; Snake may break
    // sight, retreat or stagger the guard to cancel the impending strike.
    const float FacingDot = FVector::DotProduct(
        GetActorForwardVector().GetSafeNormal2D(), ToPlayer.GetSafeNormal2D());
    const bool bValidStrike = AwarenessState == 3 && bConfirmedVisualNow
        && VisualConfirmationSeconds >= .16f && Distance < 150.f
        && FacingDot >= .68f && StaggerSeconds <= 0.f;
    if (bValidStrike && AttackCooldown <= 0.f)
    {
        if (AttackWindupSeconds <= 0.f)
        {
            AttackWindupSeconds = .38f;
            OnMeleeWarning();
        }
        else
        {
            AttackWindupSeconds -= FMath::Max(0.f, DeltaSeconds);
            if (AttackWindupSeconds <= 0.f)
            {
                UGameplayStatics::ApplyDamage(const_cast<AMGPlayerCharacter*>(Player),
                    12.f, nullptr, this, UDamageType::StaticClass());
                OnMeleeImpact();
                AttackCooldown = 1.25f;
            }
        }
    }
    else AttackWindupSeconds = 0.f;

    if (StaggerSeconds > 0.f) return;
    // Local awareness stays distinct from duty and acting. Combat needs
    // actual visual proof; a body/noise/radio report creates investigation,
    // not an omniscient 'enemy spotted' state.
    if (bConfirmedVisualNow && AwarenessState >= 3)
        SetSecurityPhase(EMGGuardAwarenessPhase::Engaged);
    else if (bConfirmedVisualNow && AwarenessState >= 1)
        SetSecurityPhase(EMGGuardAwarenessPhase::Confirming);
    else if (DistractionPhase == EDistractionPhase::React ||
             DistractionPhase == EDistractionPhase::Approach)
        SetSecurityPhase(EMGGuardAwarenessPhase::Investigating);
    else if (DistractionPhase == EDistractionPhase::Search ||
             SearchForLastKnownSeconds > 0.f || AwarenessState >= 2)
        SetSecurityPhase(EMGGuardAwarenessPhase::Searching);
    else if (DistractionPhase == EDistractionPhase::Return ||
             (EvidenceAge < 9.f && AwarenessState == 0))
        SetSecurityPhase(EMGGuardAwarenessPhase::Returning);
    else if (AwarenessState == 1 || bFacilityLockdown)
        SetSecurityPhase(EMGGuardAwarenessPhase::Suspicious);
    else
        SetSecurityPhase(EMGGuardAwarenessPhase::Routine);

    // States are presented as acting intentions. Missing AnimBPs still have
    // truthful collision and AI behavior; art cannot change these states.
    if (AttackWindupSeconds > 0.f)
        SetGuardPosture(EMGGuardPosture::Strike);
    else if (bPendingRadioReport)
        SetGuardPosture(EMGGuardPosture::Report);
    else if (AwarenessState >= 3)
        SetGuardPosture(EMGGuardPosture::Engage);
    else if (DistractionPhase != EDistractionPhase::None || SearchForLastKnownSeconds > 0.f)
        SetGuardPosture(EMGGuardPosture::Investigate);
    else if (AwarenessState == 2)
        SetGuardPosture(EMGGuardPosture::Search);
    else if (AwarenessState == 1 || bFacilityLockdown)
        SetGuardPosture(EMGGuardPosture::Notice);
    else SetGuardPosture(EMGGuardPosture::Patrol);
    // Noise investigations are interruptible by real detection/alert.
    if (AdvanceNoiseInvestigation(DeltaSeconds))
    {
        // Noise behaviour already selected movement/turning for this frame.
    }
    else if (AwarenessState == 3 || SearchForLastKnownSeconds > 0.f)
    {
        const FVector Location = GetActorLocation();
        const FVector ToLast = LastKnownPlayerPosition - Location;
        const FVector Direction = ToLast.GetSafeNormal2D();
        if (!Direction.IsNearlyZero() && ToLast.Size2D() > 105.f)
        {
            // Collision-aware pursuit. No teleport, no infinite straight-line
            // wall scraping; try a bounded, alternating flank if blocked.
            MoveToward(LastKnownPlayerPosition, 185.f, DeltaSeconds);
            const float Progress = FVector::Dist2D(Location, GetActorLocation());
            PursuitBlockedSeconds = Progress < 1.f
                ? PursuitBlockedSeconds + DeltaSeconds : 0.f;
            if (PursuitBlockedSeconds > .7f)
            {
                const FVector Lateral(-Direction.Y * FlankSide,
                                     Direction.X * FlankSide, 0.f);
                MoveToward(GetActorLocation() + Lateral * 175.f, 110.f, DeltaSeconds);
                if (PursuitBlockedSeconds > 1.8f)
                {
                    FlankSide = -FlankSide;
                    PursuitBlockedSeconds = 0.f;
                }
            }
        }
    }
    else if (AwarenessState > 0)
    {
        const FVector ToSearch = (LastKnownPlayerPosition - GetActorLocation()).GetSafeNormal2D();
        if (!ToSearch.IsNearlyZero())
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), ToSearch.Rotation(), DeltaSeconds, 3.f));
    }
    else if (Role == EMGGuardRole::Custody)
    {
        // The two transport officers hold actual security posts, rather than
        // walking away from the arrest point because a generic patrol tick
        // was reused. Their scan is subtle and visible to the player.
        const float Scan = (bFacilityCaution ? 31.f : 23.f) *
            FMath::Sin(GetWorld()->GetTimeSeconds() *
                (bFacilityCaution ? 1.1f : .75f));
        SetActorRotation(FMath::RInterpTo(GetActorRotation(),
            FRotator(0.f, StationFacing.Yaw + Scan, 0.f), DeltaSeconds, 1.8f));
    }
    else
    {
        const FVector Point = PatrolOrigin
            + PatrolAxis * (bPatrolForward ? PatrolHalfLength : -PatrolHalfLength);
        if (FVector::Dist2D(Point, GetActorLocation()) < 30.f)
            bPatrolForward = !bPatrolForward;
        else MoveToward(Point, PatrolSpeed, DeltaSeconds);
    }

#if !UE_BUILD_SHIPPING
    // Visible perception orbs and forward vectors spoil lighting and
    // narrative staging in Play-In-Editor; turn on explicitly for diagnosis.
    if (bShowPerceptionDebug)
    {
        const FColor DebugColor = AwarenessState == 3 ? FColor::Red :
            (AwarenessState == 2 ? FColor::Orange :
             (AwarenessState == 1 ? FColor::Yellow : FColor::Green));
        DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 180.f),
            22.f, 12, DebugColor, false, 0.f, 0, 2.f);
        DrawDebugLine(GetWorld(), Eye, Eye + GetActorForwardVector() * 300.f,
            DebugColor, false, 0.f, 0, 2.f);
    }
#endif
}
