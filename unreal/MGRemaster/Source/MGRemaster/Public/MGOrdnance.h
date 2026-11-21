#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGOrdnance.generated.h"

class UStaticMeshComponent;

/** Physical rocket, grenade and mine with real collision and finite ammo. */
UCLASS()
class MGREMASTER_API AMGOrdnance : public AActor
{
    GENERATED_BODY()
public:
    AMGOrdnance();
    virtual void Tick(float DeltaSeconds) override;
    void Configure(FName Weapon, FVector Velocity, AActor* FireOwner);
    FName GetOrdnanceId() const { return WeaponId; }
private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Visual;
    FName WeaponId = NAME_None;
    FVector Motion = FVector::ZeroVector;
    TWeakObjectPtr<AActor> Firer;
    float Lifetime = 5.f;
    float ArmDelay = 0.f;
    bool bExploded = false;
    void Explode(const FVector& Point);
};