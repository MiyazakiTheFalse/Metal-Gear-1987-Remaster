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
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    bool bCrouched = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    bool bProne = false;
    UPROPERTY(BlueprintReadOnly, Category="MG|Posture")
    bool bAiming = false;
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
};
