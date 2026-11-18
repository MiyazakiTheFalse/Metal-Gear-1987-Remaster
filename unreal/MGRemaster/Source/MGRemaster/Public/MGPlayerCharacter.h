#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MGPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;

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
    float GetVisibilityFactor() const;

private:
    UPROPERTY(VisibleAnywhere, Category="Camera")
    USpringArmComponent* CameraBoom;
    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* FollowCamera;
    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* PrototypeBody;

    bool bShowMissionLog = false;
    bool bAiming = false;
    bool bSprinting = false;
    float ShoulderSign = 1.f;
    int32 QualityPreset = 0;

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
    void UpdateMovementState();
};
