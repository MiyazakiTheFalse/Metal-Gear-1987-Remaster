#include "MGSecurityCamera.h"
#include "MGGuardActor.h"
#include "MGPlayerCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AMGSecurityCamera::AMGSecurityCamera()
{
    PrimaryActorTick.bCanEverTick = true;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraMount"));
    RootComponent = SceneRoot;
    Housing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraHousing"));
    Housing->SetupAttachment(SceneRoot);
    Housing->SetRelativeScale3D(FVector(.7f,.38f,.32f));
    Housing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Housing->SetStaticMesh(Cube.Object);
}
void AMGSecurityCamera::BeginPlay()
{
    Super::BeginPlay();
    BaseYaw = GetActorRotation().Yaw;
}
void AMGSecurityCamera::Configure(FName StableId, int32 DeckNumber, float SweepDegrees)
{
    Tags.AddUnique(StableId);
    Deck = DeckNumber;
    SweepAmplitude = FMath::Clamp(SweepDegrees, 0.f, 75.f);
    // Deterministic phase offsets prevent synchronised mechanical camera waves.
    ScanPhase = DeckNumber * .8f + static_cast<float>(StableId.ToString().Len()) * .41f;
}
void AMGSecurityCamera::ReportConfirmedSighting(const FVector& LastSeenLocation)
{
    if (!GetWorld()) return;
    AlertSeconds = 6.f;
    AlertCooldown = 7.f;
    // Only guards close enough to this *actual camera* get the message.
    // Last known position originates from a successful unobstructed sighting.
    for (TActorIterator<AMGGuardActor> It(GetWorld()); It; ++It)
    {
        if (It->IsIncapacitated()) continue;
        if (FMath::Abs(It->GetActorLocation().Z - GetActorLocation().Z) > 260.f) continue;
        if (FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) > 1800.f) continue;
        It->ReceiveCameraAlert(LastSeenLocation, 6.f);
    }
}
void AMGSecurityCamera::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!GetWorld()) return;
    AlertSeconds = FMath::Max(0.f, AlertSeconds - DeltaSeconds);
    AlertCooldown = FMath::Max(0.f, AlertCooldown - DeltaSeconds);
    const float Angle = BaseYaw +
        SweepAmplitude * FMath::Sin(GetWorld()->GetTimeSeconds() * .72f + ScanPhase);
    SetActorRotation(FRotator(0.f, Angle, 0.f));
    AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player ||
        FMath::Abs(Player->GetActorLocation().Z - GetActorLocation().Z) > 300.f)
    {
        Suspicion = FMath::Max(0.f, Suspicion - DeltaSeconds * .36f);
        return;
    }
    const FVector Eye = GetActorLocation();
    const FVector Target = Player->GetActorLocation() + FVector(0,0,55.f);
    const FVector Toward = Target - Eye;
    const float Distance = Toward.Size();
    const float Dot = FVector::DotProduct(
        GetActorForwardVector().GetSafeNormal(), Toward.GetSafeNormal());
    bool bCanSee = Distance > 0.f && Distance < 1600.f &&
        Dot >= FMath::Cos(FMath::DegreesToRadians(35.f));
    if (bCanSee)
    {
        FCollisionQueryParams Query(SCENE_QUERY_STAT(MGCameraVision), false, this);
        FHitResult Hit;
        const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
            Hit, Eye, Target, ECC_Visibility, Query);
        bCanSee = !bBlocked || Hit.GetActor() == Player;
    }
    if (bCanSee)
        Suspicion = FMath::Clamp(Suspicion + DeltaSeconds *
            (0.7f + 0.45f * Player->GetVisibilityFactor()), 0.f, 1.f);
    else
        Suspicion = FMath::Max(0.f, Suspicion - DeltaSeconds * .36f);
    if (Suspicion >= 1.f && AlertCooldown <= 0.f)
        ReportConfirmedSighting(Player->GetActorLocation());
}
