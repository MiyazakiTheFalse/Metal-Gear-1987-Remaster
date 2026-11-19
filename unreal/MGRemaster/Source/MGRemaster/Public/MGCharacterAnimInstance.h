#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MGCharacterAnimInstance.generated.h"

/**
 * Shared data bridge for animation blueprints. Visual state does not modify
 * authoritative AI, health, collision, mission or equipment transactions.
 */
UCLASS(Blueprintable, Transient)
class MGREMASTER_API UMGCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    float GroundSpeed = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    float TravelDirectionDegrees = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    bool bMoving = false;
    // Unfiltered owner displacement is the truth for capsule-driven animation.
    // Pose search/BlendSpaces can choose a clip from these, never drive the AI.
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    float AccelerationCmPerSecSquared = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    float YawVelocityDegreesPerSec = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    float IntentDirectionDegrees = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    float IntentStrength = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    bool bStartedMoving = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    bool bStoppedMoving = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    bool bReversalRequested = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Locomotion")
    float BrakingBlend = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    bool bCrouched = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    bool bProne = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    bool bAiming = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Aim")
    float AimBlendAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Aim")
    float AimPitchDegrees = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Aim")
    float AimYawDegrees = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Aim")
    bool bWeaponReady = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Aim")
    float FireRecoilAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Aim")
    bool bIsFiring = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    bool bSprinting = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Cover")
    bool bInCover = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Cover")
    bool bCornerPeeking = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Cover")
    bool bKnocking = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Cover")
    float CoverShuffleDirection = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Guard")
    int32 AwarenessState = 0;
    UPROPERTY(BlueprintReadOnly, Category="MG|Guard")
    bool bInvestigating = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Guard")
    bool bIncapacitated = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Fox")
    bool bRescued = false;

private:
    FVector PreviousLocation = FVector::ZeroVector;
    bool bHavePreviousLocation = false;
    bool bHaveKinematicSample = false;
    float PreviousSpeed = 0.f;
    float PreviousYaw = 0.f;
};
