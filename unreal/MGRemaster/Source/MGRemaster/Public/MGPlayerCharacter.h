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
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    bool IsAiming() const { return bAiming; }
    bool IsSprinting() const { return bSprinting && !bIsCrouched && !bAiming; }
    bool IsMissionLogVisible() const { return bShowMissionLog; }
    float GetNoiseFactor() const;
    float GetHealth() const { return Health; }
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

    void UpdateMovementState();
};
