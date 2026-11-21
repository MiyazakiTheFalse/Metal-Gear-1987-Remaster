#include "MGOrdnance.h"
#include "MGSetpieceBoss.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AMGOrdnance::AMGOrdnance()
{
    PrimaryActorTick.bCanEverTick = true;
    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrdnanceMesh"));
    SetRootComponent(Visual);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Visual->SetStaticMesh(Cube.Object);
    Visual->SetCollisionProfileName(TEXT("BlockAll"));
    Visual->SetWorldScale3D(FVector(.45f,.25f,.25f));
}
void AMGOrdnance::Configure(FName Weapon, FVector Velocity, AActor* FireOwner)
{
    WeaponId = Weapon;
    Motion = Velocity;
    Firer = FireOwner;
    if (Weapon == TEXT("MINE"))
    {
        Motion = FVector::ZeroVector;
        ArmDelay = .8f;
        Lifetime = 90.f;
        Visual->SetWorldScale3D(FVector(.62f,.62f,.14f));
    }
    else if (Weapon == TEXT("GRENADE_LAUNCHER"))
    {
        Lifetime = 4.f;
        Visual->SetWorldScale3D(FVector(.35f,.35f,.35f));
    }
    else Lifetime = 5.f;
}
void AMGOrdnance::Explode(const FVector& Point)
{
    if (bExploded || !GetWorld()) return;
    bExploded = true;
    const float Radius = WeaponId == TEXT("MINE") ? 320.f : 260.f;
    for (TActorIterator<AMGSetpieceBoss> Enemy(GetWorld()); Enemy; ++Enemy)
    {
        if (FVector::Dist(Enemy->GetActorLocation(),Point) > Radius) continue;
        // A mine's field is not a magical hit through an entire concrete wall.
        FHitResult CoverHit;
        FCollisionQueryParams Q(SCENE_QUERY_STAT(MGExplosiveLineOfSight),
                                false,this);
        const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
            CoverHit,Point+FVector(0.f,0.f,25.f),
            Enemy->GetActorLocation()+FVector(0.f,0.f,80.f),
            ECC_Visibility,Q) && CoverHit.GetActor()!=*Enemy;
        if (bBlocked) continue;
        const float Power = WeaponId == TEXT("ROCKET_LAUNCHER") ? 65.f :
            WeaponId == TEXT("MINE") ? 52.f : 38.f;
        UGameplayStatics::ApplyDamage(*Enemy,Power,nullptr,this,
                                       UDamageType::StaticClass());
    }
    Destroy();
}
void AMGOrdnance::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!GetWorld() || bExploded) return;
    Lifetime -= Dt;
    if (WeaponId == TEXT("MINE"))
    {
        ArmDelay = FMath::Max(0.f,ArmDelay - Dt);
        if (ArmDelay <= 0.f)
            for (TActorIterator<AMGSetpieceBoss> Enemy(GetWorld()); Enemy; ++Enemy)
                if (FVector::Dist2D(Enemy->GetActorLocation(),GetActorLocation()) < 210.f &&
                    FMath::Abs(Enemy->GetActorLocation().Z-GetActorLocation().Z)<160.f)
                {
                    Explode(GetActorLocation());
                    return;
                }
    }
    else
    {
        FVector NextMotion = Motion;
        if (WeaponId == TEXT("GRENADE_LAUNCHER"))
            Motion.Z -= 980.f * Dt;
        FHitResult Hit;
        SetActorLocation(GetActorLocation()+NextMotion*Dt,true,&Hit);
        if (Hit.bBlockingHit)
        {
            Explode(Hit.ImpactPoint);
            return;
        }
    }
    if (Lifetime <= 0.f) Explode(GetActorLocation());
}
