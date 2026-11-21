#include "MGSpentCasingActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

AMGSpentCasingActor::AMGSpentCasingActor()
{
    PrimaryActorTick.bCanEverTick = false;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpentCase"));
    RootComponent = Body;
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Body->SetSimulatePhysics(false);
}

bool AMGSpentCasingActor::InitialiseCase(EMGCasingType Type,
    const FRotator& WeaponRotation)
{
    const TCHAR* Path = Type == EMGCasingType::HandgunBrass
        ? TEXT("/Game/Weapons/Shared/SM_HandgunCasing_1995.SM_HandgunCasing_1995")
        : TEXT("/Game/Weapons/Shared/SM_RiotHull_1995.SM_RiotHull_1995");
    UStaticMesh* Art = LoadObject<UStaticMesh>(nullptr, Path);
    if (!Art) return false;
    Body->SetStaticMesh(Art);
    Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Body->SetCollisionObjectType(ECC_PhysicsBody);
    // A decorative piece of brass may bounce on the floor; it must NEVER
    // make a character miss by blocking weapon/visibility traces.
    Body->SetCollisionResponseToAllChannels(ECR_Block);
    Body->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    Body->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    Body->SetSimulatePhysics(true);
    Body->SetEnableGravity(true);
    Body->SetLinearDamping(.08f);
    Body->SetAngularDamping(.18f);
    const float Kg = Type == EMGCasingType::HandgunBrass ? .007f : .015f;
    Body->SetMassOverrideInKg(NAME_None, Kg, true);
    const FVector EjectionVelocityCmPerSec =
        WeaponRotation.RotateVector(FVector(18.f, 125.f, 185.f));
    Body->AddImpulse(EjectionVelocityCmPerSec * Kg, NAME_None, false);
    Body->AddAngularImpulseInDegrees(
        WeaponRotation.RotateVector(FVector(55.f, -90.f, 140.f)) * Kg);
    SetLifeSpan(7.f);
    return true;
}

void AMGSpentCasingActor::Eject(UWorld* World, EMGCasingType Type,
    const FVector& Location, const FRotator& WeaponRotation)
{
    if (!World) return;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AMGSpentCasingActor* Actor = World->SpawnActor<AMGSpentCasingActor>(
        Location, WeaponRotation, Params);
    if (Actor && !Actor->InitialiseCase(Type, WeaponRotation))
        Actor->Destroy(); // Never pretend an unimported mesh is a casing.
}
