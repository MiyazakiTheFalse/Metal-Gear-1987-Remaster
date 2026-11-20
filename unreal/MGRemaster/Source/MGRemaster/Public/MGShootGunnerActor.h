#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGShootGunnerActor.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class AMGPlayerCharacter;
class USoundBase;
class UPointLightComponent;

/**
 * 1995 Shoot Gunner / Shotmaker. A three-blast spread attacker with a long,
 * telegraphed recovery window and real cover obstruction, not an instant-hit
 * scripted movie. Source-only: final mesh/ABP/SFX/VO remain unimported.
 */
UCLASS(Blueprintable)
class MGREMASTER_API AMGShootGunnerActor : public AActor
{
    GENERATED_BODY()
public:
    AMGShootGunnerActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    void SetEncounterActive(bool bEnabled);
    void RestoreDefeated();
    void ReceiveTrackerPing(const FVector& ApproximateLocation);
    // The original MSX2 balance is approximately TEN regular handgun hits.
    // Keep 100 normalized boss HP, while other NPCs retain their own damage.
    UFUNCTION(BlueprintPure, Category="Shotmaker|Combat")
    float GetHandgunDamagePerHit() const
    {
        return FMath::Clamp(HandgunDamagePerHit, 5.f, 25.f);
    }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Gameplay")
    bool IsDefeated() const { return bDefeated; }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Gameplay")
    bool IsEncounterActive() const { return bActive; }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Animation")
    bool IsTelegraphing() const { return State == EAttackState::Warning; }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Animation")
    bool IsFiringVolley() const { return State == EAttackState::Volley; }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Animation")
    bool IsRecovering() const { return State == EAttackState::Recovery; }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Animation")
    bool IsReloading() const { return State == EAttackState::Reloading; }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Combat")
    int32 GetLoadedShells() const { return LoadedShells; }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Animation")
    float GetHealthFraction() const { return FMath::Clamp(Health / 100.f, 0.f, 1.f); }
    UFUNCTION(BlueprintPure, Category="Shotmaker|Animation")
    FVector GetLockedAimPoint() const { return LockedAimPoint; }

    // Designers may bind actual shoulder/weapon animation, muzzle light,
    // audio and haptics; the damage event itself remains fully authoritative.
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnTelegraphStarted();
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnShotFired(int32 ShotIndex);
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnPumpAction(int32 CompletedShotIndex);
    // Pump-action casing extraction, unlike the NONCANONICAL test stun pistol.
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnSpentShellEjected(int32 CompletedShotIndex);
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnReloadStarted();
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnReloadShellInserted(int32 LoadedCount);
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnReloadFinished();
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnBossDefeated();
    // Performance cue only: stage reaction/anger through actual damage, never
    // let a camera/actor animation change survival or mission flags.
    UFUNCTION(BlueprintImplementableEvent, Category="Shotmaker|Presentation")
    void OnBossWounded(float RemainingHealthFraction);
private:
    enum class EAttackState : uint8 { Dormant, Searching, Warning, Volley, Recovery, Reloading, Defeated };

    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* CollisionCapsule;
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* CharacterMesh;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ProxyTorso;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* WeaponBody;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* WeaponBarrel;
    UPROPERTY(VisibleAnywhere, Category="Shotmaker|Weapon")
    UStaticMeshComponent* WeaponPump;
    bool bImportedWeaponGeometry = false;
    float PumpVisualRemainingSeconds = 0.f;

    UPROPERTY(Transient)
    USoundBase* ShotWarningCue = nullptr;
    UPROPERTY(Transient)
    USoundBase* ShotBlastCue = nullptr;
    UPROPERTY(Transient)
    USoundBase* ShotCycleCue = nullptr;

    UPROPERTY(VisibleAnywhere, Category="Shotmaker|Presentation")
    UPointLightComponent* MuzzleFlash = nullptr;
    float MuzzleFlashRemaining = 0.f;
    // Delay the shoulder/pump cycle until after muzzle blast. A pump notify
    // must never be emitted in the same frame as the authoritative shot.
    float PumpCueRemaining = -1.f;
    int32 PendingPumpShotIndex = INDEX_NONE;

    // Modern 3D design hypotheses (NOT measured MSX2 firearm statistics).
    // Editable in a BP child and bounded in the actual gameplay methods.
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="0.5", ClampMax="3.0"))
    float WindupSeconds = .78f;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="0.25", ClampMax="1.5"))
    float IntervalSeconds = .52f;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Animation", meta=(ClampMin="0.05", ClampMax="0.40"))
    float PumpOffsetSeconds = .17f;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="0.8", ClampMax="5.0"))
    float RecoverySeconds = 1.85f;
    // Provisional 3D magazine, NOT an established capacity of the 1987 sprite.
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="3", ClampMax="8"))
    int32 TubeCapacity = 3;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="0.25", ClampMax="1.5"))
    float SecondsPerShellInsert = .58f;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="3", ClampMax="12"))
    float HalfSpreadDegrees = 6.f;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="3", ClampMax="9"))
    int32 PelletCount = 5;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="4", ClampMax="16"))
    float PelletDamage = 8.f;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="12", ClampMax="50"))
    float MaxShotDamage = 32.f;
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="30", ClampMax="110"))
    float PatrolSpeedCmPerSec = 65.f;

    // 100 normalised health: ten accepted standard handgun hits (10 each).
    // 25-damage remote-controlled missiles are a source-matched FUTURE item,
    // not yet an imported or playable player weapon.
    UPROPERTY(EditAnywhere, Category="Shotmaker|Combat", meta=(ClampMin="5", ClampMax="25"))
    float HandgunDamagePerHit = 10.f;

    EAttackState State = EAttackState::Dormant;
    bool bActive = false;
    bool bDefeated = false;
    float Health = 100.f;
    float StateClock = 0.f;
    int32 ShotsFired = 0;
    int32 LoadedShells = 3;
    float ReloadShellClock = 0.f;
    int32 VolleysCompleted = 0;
    float PatrolDirection = 1.f;
    FVector LockedAimPoint = FVector::ZeroVector;
    FVector LastTrackerLocation = FVector::ZeroVector;
    float TrackerPingAge = 99.f;

    bool CanSeePlayer(AMGPlayerCharacter* Player) const;
    FVector GetActualMuzzleLocation() const;
    void FireShot(AMGPlayerCharacter* Player);
    void EnterDefeatedState();
};
