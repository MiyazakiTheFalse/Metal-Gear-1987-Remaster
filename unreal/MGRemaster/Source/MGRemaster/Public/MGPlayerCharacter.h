#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MGPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UMGEquipmentComponent;

UCLASS()
class MGREMASTER_API AMGPlayerCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AMGPlayerCharacter();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    bool IsAiming() const { return bAiming; }
    bool IsSprinting() const { return bSprinting && !bIsCrouched && !bAiming; }
    bool IsMissionLogVisible() const { return bShowMissionLog; }
    bool IsCodecOpen() const { return bCodecOpen; }
    FString GetCodecStatus() const { return CodecStatus; }
    int32 GetTransceiverChannel() const { return TransceiverChannel; }
    float GetEquipmentTimeScale() const { return EquipmentTimeScale; }
    float GetNoiseFactor() const;
    float GetHealth() const { return Health; }
    bool IsProne() const { return bIsProne; }
    bool IsInCover() const { return bInCover; }
    // Read by animation blueprints once the finished character rig is imported.
    UFUNCTION(BlueprintPure, Category="Stealth|Wall")
    bool IsCornerPeeking() const { return CoverPeekAlpha > 0.35f; }
    UFUNCTION(BlueprintPure, Category="Stealth|Wall")
    float GetCoverPeekAlpha() const { return CoverPeekAlpha; }
    UFUNCTION(BlueprintPure, Category="Stealth|Wall")
    bool IsWallKnocking() const { return WallKnockVisualTimer > 0.f; }
    UFUNCTION(BlueprintPure, Category="Stealth|Wall")
    float GetCoverShuffleDirection() const { return CoverShuffleDirection; }
    FString GetPostureText() const;
    bool TryEnterCover();
    void LeaveCover();
    UMGEquipmentComponent* GetEquipment() const { return Equipment; }
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    void NotifyFeedback(const FString& Message, FName CueName = NAME_None);

    float GetVisibilityFactor() const;

private:
    UPROPERTY(VisibleAnywhere, Category="Camera")
    USpringArmComponent* CameraBoom;
    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* FollowCamera;
    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* PrototypeBody;
    UPROPERTY(VisibleAnywhere, Category="Equipment")
    UMGEquipmentComponent* Equipment;

    UPROPERTY(EditAnywhere, Category="Controls|Time")
    float EquipmentTimeScale = 0.20f; // Engineering default, not a canonical franchise speed.
    bool bCodecOpen = false;
    bool bIsProne = false;
    bool bPendingProne = false;
    bool bInCover = false;
    FVector CoverNormal = FVector::ZeroVector;
    float CoverReattachCooldown = 0.f;
    float NextWallKnockTime = 0.f;
    float WallKnockVisualTimer = 0.f;
    float CoverPeekAlpha = 0.f;
    float CoverShuffleDirection = 0.f;
    float CoverIntent = 0.f;
    float PreferredShoulderSign = 1.f;
    FString CodecStatus;
    int32 TransceiverChannel = 0; // 0: Kyle Schneider 120.79, 1: Big Boss 120.85
    bool bShowMissionLog = false;
    bool bAiming = false;
    bool bSprinting = false;
    float ShoulderSign = 1.f;
    int32 QualityPreset = 0;
    float Health = 100.f;
    float MeleeNextTime = 0.f;
    float LastMeleeTime = -100.f;
    float MeleeVisualTimer = 0.f;
    float CombatNoiseTimer = 0.f;
    int32 MeleeComboStep = 0;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void TurnRate(float Value);
    void LookUpRate(float Value);
    void AimStart();
    void AimStop();
    void SprintStart();
    void SprintStop();
    void CrouchToggle();
    void ShoulderSwap();
    void FireStun();
    void Interact();
    void RestartRescue();
    void ToggleMissionLog();
    void CycleGraphicsQuality();
    void WeaponWindowStart();
    void WeaponWindowStop();
    void ItemWindowStart();
    void ItemWindowStop();
    void SelectionPrevious();
    void SelectionNext();
    void UseItem();
    void MeleeAttack();
    void ToggleProne();
    void ToggleCover();
    void EnterProneAfterCrouch();
    bool ExitProneSafely();
    void ApplyCoverMovement(float Value, const FVector& WorldDirection);
    bool TryAutoEnterCover(const FVector& WorldDirection);
    bool FindContactWall(const FVector& Direction, float Reach, FHitResult& OutHit) const;
    void KnockOnWall();
    void UpdateCornerPeek(float DeltaSeconds);
    void ToggleCodec();
    void ApplyMenuTimeState();
    bool IsCombatAlertActive() const;
    void CloseEquipmentWindows();

    void UpdateMovementState();
};
