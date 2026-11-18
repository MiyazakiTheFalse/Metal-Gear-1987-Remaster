#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGuardActor.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;

UCLASS()
class MGREMASTER_API AMGGuardActor : public AActor
{
    GENERATED_BODY()
public:
    AMGGuardActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    bool IsIncapacitated() const { return bIncapacitated; }
    uint32 GetAwarenessState() const { return AwarenessState; }
    float GetSuspicion() const { return Suspicion; }

private:
    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UCapsuleComponent* CollisionCapsule;

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* ProxyBody;

    FVector PatrolOrigin = FVector::ZeroVector;
    FVector LastKnownPlayerPosition = FVector::ZeroVector;
    bool bPatrolForward = true;
    bool bIncapacitated = false;
    float Suspicion = 0.f;
    float Health = 100.f;
    float StaggerSeconds = 0.f;
    float AttackCooldown = 0.f;
    uint32 AwarenessState = 0;
};
