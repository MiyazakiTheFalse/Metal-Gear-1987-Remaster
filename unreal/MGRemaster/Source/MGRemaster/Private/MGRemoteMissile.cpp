#include "MGRemoteMissile.h"
#include "MGMissionOne.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AMGRemoteMissile::AMGRemoteMissile()
{
    PrimaryActorTick.bCanEverTick = true;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissileBody"));
    SetRootComponent(Body);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Mesh.Succeeded()) Body->SetStaticMesh(Mesh.Object);
    Body->SetWorldScale3D(FVector(.55f,.14f,.14f));
    Body->SetCollisionProfileName(TEXT("BlockAll"));
    Body->SetGenerateOverlapEvents(false);
}
void AMGRemoteMissile::SetYawInput(float Axis)
{
    YawSteer = FMath::Clamp(Axis, -1.f, 1.f);
}
void AMGRemoteMissile::SetPitchInput(float Axis)
{
    PitchSteer = FMath::Clamp(Axis, -1.f, 1.f);
}
void AMGRemoteMissile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (DeltaSeconds <= 0.f) return;
    FuelRemaining -= DeltaSeconds;
    if (FuelRemaining <= 0.f) { Destroy(); return; }

    // Input is an intentional steering decision; no automatic pathfinding
    // missile that finds a control unit Snake has not aimed toward.
    const FRotator NewRotation = GetActorRotation() +
        FRotator(PitchSteer * 78.f * DeltaSeconds,
                 YawSteer * 95.f * DeltaSeconds, 0.f);
    SetActorRotation(NewRotation);
    FHitResult Impact;
    const FVector Step = GetActorForwardVector() * MoveSpeed * DeltaSeconds;
    SetActorLocation(GetActorLocation() + Step, true, &Impact);
    if (!Impact.bBlockingHit) return;
    if (IsValid(Impact.GetActor()) &&
        Impact.GetActor()->ActorHasTag(TEXT("M01_ElectricPanel")))
    {
        if (AMGMissionOne* Mission = Cast<AMGMissionOne>(
            UGameplayStatics::GetActorOfClass(this, AMGMissionOne::StaticClass())))
            Mission->DisableElectricalSupply();
    }
    // A real obstruction destroys the missile, including a missed panel.
    Destroy();
}
