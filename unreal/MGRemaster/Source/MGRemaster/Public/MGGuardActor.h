#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGuardActor.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;

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
    // Sound stimulus carries a world location, never Snake's position.
    void HearWallKnock(const FVector& SoundLocation, const FVector& WallNormal, float HearingRadius, AActor* SoundInstigator);
    bool IsInvestigatingNoise() const { return DistractionPhase != EDistractionPhase::None; }
    UFUNCTION(BlueprintPure, Category="Character|Art")
    bool HasAuthoredCharacterArt() const;

private:
    enum class EDistractionPhase : uint8 { None, React, Approach, Search, Return };
    EDistractionPhase DistractionPhase = EDistractionPhase::None;
    FVector DistractionLocation = FVector::ZeroVector;
    FVector ReturnToPatrol = FVector::ZeroVector;
    float DistractionTimer = 0.f;
    float InvestigationStuckTimer = 0.f;
    int32 ConsecutiveKnocks = 0;
    bool AdvanceNoiseInvestigation(float DeltaSeconds);
    void MoveToward(const FVector& Destination, float Speed, float DeltaSeconds);

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UCapsuleComponent* CollisionCapsule;

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* ProxyBody;

    UPROPERTY(VisibleAnywhere, Category="Character|Art")
    USkeletalMeshComponent* CharacterMesh;

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
