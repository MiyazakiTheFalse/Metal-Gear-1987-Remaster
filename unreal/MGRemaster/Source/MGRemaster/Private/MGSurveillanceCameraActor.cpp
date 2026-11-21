#include "MGSurveillanceCameraActor.h"
#include "MGPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MGGrayFoxMission.h"
#include "MGGuardActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AMGSurveillanceCameraActor::AMGSurveillanceCameraActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
    RootComponent = Pivot;
    Mount = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructuralBracket"));
    Mount->SetupAttachment(Pivot);
    Mount->SetRelativeLocation(FVector(-11.f, 0.f, 4.f));
    Mount->SetRelativeScale3D(FVector(.32f, .34f, .20f));
    Mount->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Housing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanTiltOpticHead"));
    Housing->SetupAttachment(Pivot);
    Housing->SetRelativeScale3D(FVector(.40f, .28f, .28f));
    Housing->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Housing->SetCollisionResponseToAllChannels(ECR_Ignore);
    Housing->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    ControlPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EyeLevelIsolatorPanel"));
    ControlPanel->SetupAttachment(Pivot);
    // Actor origin at 235cm; control at 55cm above datum, inside the
    // player's standing interaction zone without an implausible jump.
    ControlPanel->SetRelativeLocation(FVector(-45.f, 0.f, -180.f));
    ControlPanel->SetRelativeScale3D(FVector(.35f, .28f, .55f));
    ControlPanel->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ControlPanel->SetCollisionResponseToAllChannels(ECR_Ignore);
    ControlPanel->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded())
    {
        Mount->SetStaticMesh(Cube.Object);
        Housing->SetStaticMesh(Cube.Object);
        ControlPanel->SetStaticMesh(Cube.Object);
    }
    StatusLamp = CreateDefaultSubobject<UPointLightComponent>(TEXT("CameraStatus"));
    StatusLamp->SetupAttachment(Housing);
    StatusLamp->SetRelativeLocation(FVector(22.f, 0.f, -4.f));
    StatusLamp->SetAttenuationRadius(125.f);
    StatusLamp->SetIntensity(900.f);
    StatusLamp->SetCastShadows(false);
}

void AMGSurveillanceCameraActor::BeginPlay()
{
    Super::BeginPlay();
    InitialWorldRotation = GetActorRotation();
    if (UStaticMesh* Authored = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Security/CCTV/SM_SurveillanceCamera_1995.SM_SurveillanceCamera_1995")))
        Housing->SetStaticMesh(Authored); // Existing collision/behavior remains.
}

void AMGSurveillanceCameraActor::ConfigureSector(float HalfAngleDegrees, float RangeCm)
{
    SweepHalfDegrees = FMath::Clamp(HalfAngleDegrees, 12.f, 62.f);
    DetectionRangeCm = FMath::Clamp(RangeCm, 350.f, 1200.f);
}

bool AMGSurveillanceCameraActor::CanActuallySee(const AMGPlayerCharacter* Player) const
{
    if (!bOperational || !IsValid(Player) || !GetWorld()) return false;
    if (Player->IsStillInCarton()) return false;
    const FVector Eye = Housing->GetComponentLocation() +
        Housing->GetForwardVector() * 25.f;
    const FVector Target = Player->GetActorLocation() + FVector(0.f, 0.f,
        Player->IsProne() ? 18.f : (Player->GetCharacterMovement() &&
        Player->GetCharacterMovement()->IsCrouching() ? 40.f : 58.f));
    const FVector Delta = Target - Eye;
    if (Delta.Size() > DetectionRangeCm || Delta.Size() < 1.f) return false;
    // Averted lens, physical occlusion and diminished silhouette all matter.
    const float Facing = FVector::DotProduct(
        Housing->GetForwardVector(), Delta.GetSafeNormal());
    if (Facing < FMath::Cos(FMath::DegreesToRadians(32.f))) return false;
    FCollisionQueryParams Trace(SCENE_QUERY_STAT(MGCCTVPhysicalLOS), false, this);
    FHitResult Hit;
    const bool bCollision = GetWorld()->LineTraceSingleByChannel(
        Hit, Eye, Target, ECC_Visibility, Trace);
    return bCollision && Hit.GetActor() == Player;
}

void AMGSurveillanceCameraActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bOperational || !GetWorld()) return;
    const float Step = FMath::Max(0.f, DeltaSeconds);
    ReacquirePauseSeconds = FMath::Max(0.f, ReacquirePauseSeconds - Step);
    ReportCooldownSeconds = FMath::Max(0.f, ReportCooldownSeconds - Step);

    // Pan is visible and predictable. No omniscient snap to Snake's world
    // transform: scan cannot rotate through a wall to keep perfect contact.
    const float Angle = FMath::Sin(GetWorld()->GetTimeSeconds() * .55f) * SweepHalfDegrees;
    Housing->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
    const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this, 0));
    const bool bVisible = CanActuallySee(Player);
    if (bVisible)
    {
        if (!bTrackingPlayer) OnVisualContactAcquired();
        bTrackingPlayer = true;
        LastObservedWorldPosition = Player->GetActorLocation(); // firsthand, now
        // Cover/prone narrow the silhouette: a short sighting still gives
        // feedback but is less likely to cause a full facility-wide alarm.
        const float Signature = FMath::Clamp(Player->GetVisibilityFactor(), .18f, 1.f);
        VisualContactSeconds += Step * FMath::Max(.40f, Signature);
        if (VisualContactSeconds >= 1.25f && ReportCooldownSeconds <= 0.f)
        {
            ReportCooldownSeconds = 12.f;
            bReportedIntruder = true;
            OnAlarmTransmitted(LastObservedWorldPosition);
            if (AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
                UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass())))
                Mission->NotifyGuardContactReported(LastObservedWorldPosition);
            for (TActorIterator<AMGGuardActor> G(GetWorld()); G; ++G)
                if (!G->IsIncapacitated())
                    G->ReceiveLocalSquadReport(LastObservedWorldPosition, this);
        }
    }
    else
    {
        if (bTrackingPlayer) OnVisualContactLost();
        bTrackingPlayer = false;
        // Loss of actual visual contact erases prospective detection. This
        // camera cannot accumulate disconnected glimpses into a fake lock.
        VisualContactSeconds = FMath::Max(0.f, VisualContactSeconds - Step * 2.8f);
        if (VisualContactSeconds <= 0.f) bReportedIntruder = false;
    }
    StatusLamp->SetLightColor(bReportedIntruder
        ? FLinearColor(1.f, .10f, .09f)
        : (bTrackingPlayer ? FLinearColor(1.f, .72f, .22f)
            : FLinearColor(.17f, .66f, .41f)));
}

void AMGSurveillanceCameraActor::Disable(bool bByInteraction)
{
    if (!bOperational) return;
    RestoreOfflineState();
    OnCameraDisabled(bByInteraction);
}

void AMGSurveillanceCameraActor::RestoreOfflineState()
{
    bOperational = false;
    bTrackingPlayer = false;
    bReportedIntruder = false;
    VisualContactSeconds = 0.f;
    StatusLamp->SetVisibility(false);
    // Breakable optic remains physical set dressing and can still block an
    // actual bullet. Disabled electronics cannot update facility mission flags.
    SetActorTickEnabled(false);
}

FVector AMGSurveillanceCameraActor::GetControlPanelWorldLocation() const
{
    return IsValid(ControlPanel) ? ControlPanel->GetComponentLocation()
        : GetActorLocation();
}

bool AMGSurveillanceCameraActor::TryDisableAtControlPanel(const AMGPlayerCharacter* Player)
{
    if (!bOperational || !IsValid(Player) || !GetWorld()) return false;
    // The only gameplay disable is a reachable wall isolator, NOT remote
    // interaction by picking a camera in the HUD or looking at its lens.
    const FVector Panel = ControlPanel->GetComponentLocation();
    if (FVector::Dist(Player->GetActorLocation(), Panel) > 168.f) return false;
    if (Player->IsProne() || Player->IsSprinting() || Player->IsAiming()) return false;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MGCCTVPanelReach), false, Player);
    FHitResult Hit;
    const FVector Hand = Player->GetActorLocation() + FVector(0.f, 0.f, 63.f);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Hand, Panel, ECC_Visibility, Query)
        && Hit.GetComponent() != ControlPanel)
        return false;
    Disable(true);
    return true;
}

float AMGSurveillanceCameraActor::TakeDamage(float Amount,
    const FDamageEvent& Event, AController* Instigator, AActor* Causer)
{
    if (Amount > 0.f && bOperational)
        Disable(false); // Authorized damaging shot, not a kill/canon item drop.
    return FMath::Max(0.f, Amount);
}
