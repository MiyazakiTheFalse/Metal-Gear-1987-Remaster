#include "MGGuardActor.h"

#include "MGPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "mg_legacy_core.h"

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
}

void AMGGuardActor::BeginPlay()
{
    Super::BeginPlay();
    PatrolOrigin = GetActorLocation();
}

void AMGGuardActor::HearWallKnock(const FVector& SoundLocation, const FVector& WallNormal, float HearingRadius, AActor* SoundInstigator)
{
    if (bIncapacitated || !GetWorld() || AwarenessState >= 3) return;
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
    ReturnToPatrol = PatrolOrigin +
        FVector(FMath::Clamp(GetActorLocation().X - PatrolOrigin.X, -180.f, 180.f), 0.f, 0.f);
    DistractionPhase = EDistractionPhase::React;
    DistractionTimer = FMath::Max(0.31f, 0.52f - ConsecutiveKnocks * 0.07f);
    InvestigationStuckTimer = 0.f;
    Suspicion = FMath::Max(Suspicion, 0.24f);
    AwarenessState = FMath::Max(AwarenessState, 1u);
}

void AMGGuardActor::ReceiveCameraAlert(const FVector& SightLocation, float DurationSeconds)
{
    if (bIncapacitated || DurationSeconds <= 0.f) return;
    CameraAlertSeconds = FMath::Max(CameraAlertSeconds, DurationSeconds);
    LastKnownPlayerPosition = SightLocation;
    Suspicion = 1.f;
    AwarenessState = 3;
    DistractionPhase = EDistractionPhase::None;
}

void AMGGuardActor::MoveToward(const FVector& Destination, float Speed, float DeltaSeconds)
{
    const FVector Diff = (Destination - GetActorLocation()).GetSafeNormal2D();
    if (Diff.IsNearlyZero()) return;
    const FVector Step = Diff * FMath::Min(Speed * DeltaSeconds, FVector::Dist2D(Destination, GetActorLocation()));
    SetActorLocation(GetActorLocation() + Step, true);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), Diff.Rotation(), DeltaSeconds, 6.f));
}

bool AMGGuardActor::AdvanceNoiseInvestigation(float DeltaSeconds)
{
    if (DistractionPhase == EDistractionPhase::None) return false;
    // Real visual detection and combat stimuli supersede a mere sound cue.
    if (AwarenessState >= 2)
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
        Health = FMath::Max(0.f, Health - DamageAmount);
        StaggerSeconds = 0.22f;
        // A victim can react and report contact; damage never silently vanishes.
        Suspicion = 1.f;
        AwarenessState = 3;
        if (DamageCauser) LastKnownPlayerPosition = DamageCauser->GetActorLocation();
        if (Health <= 0.f)
        {
            bIncapacitated = true;
            Suspicion = 0.f;
            AwarenessState = 0;
            ProxyBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ProxyBody->SetRelativeRotation(FRotator(0.f, 0.f, 80.f));
            SetActorTickEnabled(false);
        }
    }
    return Accepted;
}

void AMGGuardActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bIncapacitated || !GetWorld()) return;
    StaggerSeconds = FMath::Max(0.f, StaggerSeconds - DeltaSeconds);
    CameraAlertSeconds = FMath::Max(0.f, CameraAlertSeconds - DeltaSeconds);
    AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaSeconds);
    ProxyBody->SetRelativeRotation(FMath::RInterpTo(ProxyBody->GetRelativeRotation(),
        StaggerSeconds > 0.f ? FRotator(0.f, 0.f, 12.f) : FRotator::ZeroRotator,
        DeltaSeconds, 9.f));
    const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;

    // Floors are distinct security volumes. Never perceive or chase a player
    // through several metres of reinforced deck; floor isolation is mandatory
    // until a real stair/elevator traversal and NavMesh handoff exists.
    if (FMath::Abs(Player->GetActorLocation().Z - GetActorLocation().Z) > 260.f)
        return;
    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    const float Distance = ToPlayer.Size2D();

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MGGuardVisibility), false, this);
    const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, 55.f);
    const FVector Target = Player->GetActorLocation() + FVector(0.f, 0.f, 55.f);
    const bool bTraceHit = GetWorld()->LineTraceSingleByChannel(Hit, Eye, Target, ECC_Visibility, Params);
    const bool bHasLineOfSight = !bTraceHit || Hit.GetActor() == Player;
    const FVector Forward = GetActorForwardVector();

    MgGuardFrame Frame = {};
    Frame.guard_pos = {GetActorLocation().X / 100.f, GetActorLocation().Y / 100.f};
    Frame.guard_forward = {Forward.X, Forward.Y};
    Frame.player_pos = {Player->GetActorLocation().X / 100.f, Player->GetActorLocation().Y / 100.f};
    Frame.vision_range_m = 12.f;
    Frame.vision_half_angle_deg = 55.f;
    Frame.hearing_range_m = 11.f;
    Frame.visibility = Player->GetVisibilityFactor();
    Frame.noise = Player->GetNoiseFactor();
    // Basic structural attenuation: solid geometry dampens ordinary footsteps.
    // The native Rust perception model receives the same attenuated input
    // as the Unreal fallback. This is not a full propagated acoustic graph.
    if (!bHasLineOfSight) Frame.noise *= 0.18f;
    Frame.suspicion = Suspicion;
    Frame.delta_seconds = DeltaSeconds;
    Frame.line_of_sight = bHasLineOfSight ? 1u : 0u;

    MgGuardResult Result = {};
#if MG_WITH_RUST
    if (mg_core_abi_version() == 1 && mg_step_guard(&Frame, &Result))
    {
        Suspicion = Result.suspicion;
        AwarenessState = Result.state;
        if (Result.sees_player)
            LastKnownPlayerPosition = Player->GetActorLocation();
        else if (Result.hears_player)
            LastKnownPlayerPosition = Player->GetActorLocation() +
                FVector(180.f * FMath::Sin(GetActorLocation().X * .001f),
                        180.f * FMath::Cos(GetActorLocation().Y * .001f), 0.f);
    }
#else
    // Reduced fallback for editor prototyping only; use the Rust native ABI
    // for the full independently tested perception implementation.
    const bool bFacing = FVector::DotProduct(Forward.GetSafeNormal2D(), ToPlayer.GetSafeNormal2D()) > 0.57f;
    const bool bVisible = bHasLineOfSight && bFacing && Distance < 1200.f;
    const bool bAudible = Distance < 1100.f && Frame.noise > 0.1f;
    Suspicion = FMath::Clamp(Suspicion + DeltaSeconds *
        ((bVisible ? Frame.visibility * 0.95f : 0.f) +
         (bAudible ? Frame.noise * 0.40f : 0.f) -
         ((!bVisible && !bAudible) ? 0.12f : 0.f)), 0.f, 1.f);
    AwarenessState = Suspicion >= 0.99f ? 3 : Suspicion >= 0.68f ? 2 : Suspicion >= 0.22f ? 1 : 0;
    if (bVisible) LastKnownPlayerPosition = Player->GetActorLocation();
    else if (bAudible) LastKnownPlayerPosition = Player->GetActorLocation() +
        FVector(180.f * FMath::Sin(GetActorLocation().X * .001f),
                180.f * FMath::Cos(GetActorLocation().Y * .001f), 0.f);
#endif
    // Camera-confirmed alarms are durable for their short message lifetime,
    // regardless of ordinary perception decay after Snake leaves the frame.
    if (CameraAlertSeconds > 0.f)
    {
        AwarenessState = 3;
        Suspicion = FMath::Max(Suspicion, 0.99f);
    }

    // Enemy attack only after an actual alert and physical approach.
    // Player health and HUD are authoritative; no invisible instant failure.
    if (AwarenessState == 3 && Distance < 145.f && AttackCooldown <= 0.f)
    {
        UGameplayStatics::ApplyDamage(const_cast<AMGPlayerCharacter*>(Player),
            12.f, nullptr, this, UDamageType::StaticClass());
        AttackCooldown = 1.2f;
    }

    if (StaggerSeconds > 0.f) return;
    // Noise investigations are interruptible by real detection/alert.
    if (AdvanceNoiseInvestigation(DeltaSeconds))
    {
        // Noise behaviour already selected movement/turning for this frame.
    }
    else if (AwarenessState == 3)
    {
        const FVector Direction = (LastKnownPlayerPosition - GetActorLocation()).GetSafeNormal2D();
        if (!Direction.IsNearlyZero())
        {
            SetActorLocation(GetActorLocation() + Direction * 185.f * DeltaSeconds, true);
            SetActorRotation(Direction.Rotation());
        }
    }
    else if (AwarenessState > 0)
    {
        const FVector ToSearch = (LastKnownPlayerPosition - GetActorLocation()).GetSafeNormal2D();
        if (!ToSearch.IsNearlyZero())
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), ToSearch.Rotation(), DeltaSeconds, 3.f));
    }
    else
    {
        const FVector Point = PatrolWaypoints.IsEmpty() ?
            PatrolOrigin + FVector(bPatrolForward ? 180.f : -180.f, 0.f, 0.f) :
            PatrolWaypoints[PatrolWaypointIndex];
        const FVector Diff = Point - GetActorLocation();
        if (Diff.Size2D() < 20.f)
        {
            if (!PatrolWaypoints.IsEmpty())
                PatrolWaypointIndex = (PatrolWaypointIndex + 1) % PatrolWaypoints.Num();
            else bPatrolForward = !bPatrolForward;
        }
        else
        {
            const FVector Step = Diff.GetSafeNormal2D();
            SetActorLocation(GetActorLocation() + Step * 90.f * DeltaSeconds, true);
            SetActorRotation(Step.Rotation());
        }
    }

#if !UE_BUILD_SHIPPING
    const FColor DebugColor = AwarenessState == 3 ? FColor::Red :
        (AwarenessState == 2 ? FColor::Orange : (AwarenessState == 1 ? FColor::Yellow : FColor::Green));
    DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 180.f),
        22.f, 12, DebugColor, false, 0.f, 0, 2.f);
    DrawDebugLine(GetWorld(), Eye, Eye + GetActorForwardVector() * 300.f,
        DebugColor, false, 0.f, 0, 2.f);
#endif
}
