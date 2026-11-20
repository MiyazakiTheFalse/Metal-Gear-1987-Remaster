#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MGCharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EMGPerformanceAction : uint8
{
    Idle, Travel, Crouch, Prone, Aim, Fire, Melee, WallKnock,
    Cover, CoverPeek, RadioIdle,
    GuardNotice, GuardInvestigate, GuardSearch, GuardChallenge,
    GuardReport, GuardEngage, GuardStrike, Incapacitated,
    FoxPrisoner, FoxRescued, GunnerSearch, GunnerAim,
    GunnerVolley, GunnerRecovery, GunnerDefeated
};

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
    float CrouchTransitionAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    float ProneTransitionAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    float CoverTransitionAlpha = 0.f;
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

    // Action arbitration is presentation-only. Montages/BlendSpaces may
    // respond, but never call the native damage/mission transaction from here.
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    EMGPerformanceAction PerformanceAction = EMGPerformanceAction::Idle;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    EMGPerformanceAction PreviousPerformanceAction = EMGPerformanceAction::Idle;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    bool bPerformanceActionChanged = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    bool bPresentationInterrupted = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    float PerformanceActionElapsedSeconds = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    float PerformanceTransitionAlpha = 1.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    bool bRadioListeningOverlay = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    float RadioListenOverlayAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Transitions")
    float ImpactPoseAlpha = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="MG|Gunner")
    float GunnerHealthFraction = 1.f;
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Transitions")
    void OnPerformanceActionTransition(EMGPerformanceAction From, EMGPerformanceAction To, bool bInterrupted);

private:
    FVector PreviousLocation = FVector::ZeroVector;
    bool bHavePreviousLocation = false;
    bool bHaveKinematicSample = false;
    float PreviousSpeed = 0.f;
    float PreviousYaw = 0.f;
};
