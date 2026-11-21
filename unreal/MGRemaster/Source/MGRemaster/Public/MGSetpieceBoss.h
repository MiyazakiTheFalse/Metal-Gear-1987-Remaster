#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSetpieceBoss.generated.h"

class UStaticMeshComponent;
class AMGLaterMission;

/** Encounter actor with authored arena duty and weapon vulnerability.
 * Original bosses' existence is source-led; telegraph timings and models are
 * provisional remake implementations, not J2ME-recovered parameters.
 */
UCLASS()
class MGREMASTER_API AMGSetpieceBoss : public AActor
{
    GENERATED_BODY()
public:
    AMGSetpieceBoss();
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float Damage, const FDamageEvent& Event,
                             AController* Instigator, AActor* Causer) override;
    void ConfigureEncounter(AMGLaterMission* Mission, FName EncounterId,
                            FName RequiredOrdnance, float HitPoints,
                            float AttackRadius, float Telegraph,
                            float AttackDamage, const FVector& DefensiveFocus);
    FName GetEncounterId() const { return Id; }
    bool IsDefeated() const { return bDefeated; }
    FName GetRequiredOrdnance() const { return WeaponRequirement; }
private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Body = nullptr;
    TWeakObjectPtr<AMGLaterMission> Director;
    FName Id = NAME_None;
    FName WeaponRequirement = NAME_None;
    float Health = 100.f;
    float AlertRange = 1050.f;
    float TelegraphTime = 1.1f;
    float AttackPower = 12.f;
    float Cooldown = 1.8f;
    float Windup = -1.f;
    bool bDefeated = false;
    FVector DefensiveFocus = FVector::ZeroVector;
    float MoveClock = 0.f;
};