#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSentryDogActor.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class AMGPlayerCharacter;

/** Source-authored 3D guard-dog encounter following the Card 3 door.
 * This is NOT a claim of an original 1987 dog rig or verified room layout.
 * Dogs communicate only observable barks and physical threat locations.
 */
UENUM(BlueprintType)
enum class EMGSentryDogState : uint8 { Patrol, Notice, Pursue, Lunge, Recover, Incapacitated };

UCLASS(Blueprintable)
class MGREMASTER_API AMGSentryDogActor : public AActor
{
    GENERATED_BODY()
public:
    AMGSentryDogActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    void ConfigureBeatPatrol(const FVector& TravelAxis, float RadiusCm);
    UFUNCTION(BlueprintPure, Category="MG|Dog|Acting")
    EMGSentryDogState GetDogState() const { return State; }
    UFUNCTION(BlueprintPure, Category="MG|Dog|Acting")
    bool IsIncapacitated() const { return State == EMGSentryDogState::Incapacitated; }
    UFUNCTION(BlueprintPure, Category="MG|Dog|Acting")
    float GetThreatElapsed() const { return ThreatElapsed; }
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Dog|Acting")
    void OnSentryBark();
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Dog|Acting")
    void OnSentryLungeTelegraph();
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Dog|Acting")
    void OnSentryLungeImpact();
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Dog|Acting")
    void OnSentryIncapacitated();

private:
    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* Capsule;
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* SentryMesh;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* SourceProxy;
    EMGSentryDogState State = EMGSentryDogState::Patrol;
    FVector Home = FVector::ZeroVector;
    FVector PatrolAxis = FVector::ForwardVector;
    FVector LastActualEvidence = FVector::ZeroVector;
    float PatrolRadius = 160.f;
    float PatrolSign = 1.f;
    float Health = 55.f;
    float ThreatElapsed = 0.f;
    float ContactAge = 99.f;
    float BarkCooldown = 0.f;
    float LungeRemaining = 0.f;
    float RecoverRemaining = 0.f;
    bool CanObserve(const AMGPlayerCharacter* Player) const;
    void MoveDog(const FVector& Target, float Speed, float DeltaSeconds);
    void BarkFromActualLocation();
};
