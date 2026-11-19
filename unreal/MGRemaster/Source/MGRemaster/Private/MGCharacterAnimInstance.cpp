#include "MGCharacterAnimInstance.h"
#include "MGPlayerCharacter.h"
#include "MGGuardActor.h"
#include "MGPrisonerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

void UMGCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    AActor* Owner = GetOwningActor();
    if (!Owner) return;

    const FVector Location = Owner->GetActorLocation();
    FVector Velocity = FVector::ZeroVector;
    // Guard patrols use swept actor movement rather than a MovementComponent.
    // Capture their actual displacement, not a guessed animation speed.
    if (bHavePreviousLocation && DeltaSeconds > KINDA_SMALL_NUMBER)
        Velocity = (Location - PreviousLocation) / DeltaSeconds;
    PreviousLocation = Location;
    bHavePreviousLocation = true;
    if (const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(Owner))
        Velocity = Player->GetVelocity();
    Velocity.Z = 0.f;
    GroundSpeed = FMath::Clamp(Velocity.Size(), 0.f, 750.f);
    bMoving = GroundSpeed > 8.f;
    const FVector Local = Owner->GetActorTransform().InverseTransformVectorNoScale(Velocity);
    TravelDirectionDegrees = bMoving ?
        FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X)) : 0.f;

    bCrouched = bProne = bAiming = bSprinting = bInCover = false;
    bWeaponReady = false;
    FireRecoilAlpha = 0.f;
    bCornerPeeking = bKnocking = bInvestigating = bIncapacitated = bRescued = false;
    CoverShuffleDirection = 0.f;
    AwarenessState = 0;
    if (const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(Owner))
    {
        bCrouched = Player->GetCharacterMovement()->IsCrouching();
        bProne = Player->IsProne();
        bAiming = Player->IsAiming();
        bWeaponReady = Player->HasReadyWeapon();
        FireRecoilAlpha = Player->GetFiringPoseAlpha();
        if (Player->GetController())
        {
            const FRotator Control = Player->GetController()->GetControlRotation();
            AimPitchDegrees = FMath::FInterpTo(AimPitchDegrees,
                bAiming ? FMath::Clamp(FRotator::NormalizeAxis(Control.Pitch), -60.f, 55.f) : 0.f,
                DeltaSeconds, 14.f);
            AimYawDegrees = FMath::FInterpTo(AimYawDegrees,
                bAiming ? FMath::Clamp(FMath::FindDeltaAngleDegrees(Owner->GetActorRotation().Yaw, Control.Yaw), -85.f, 85.f) : 0.f,
                DeltaSeconds, 14.f);
        }
        bSprinting = Player->IsSprinting();
        bInCover = Player->IsInCover();
        bCornerPeeking = Player->IsCornerPeeking();
        bKnocking = Player->IsWallKnocking();
        CoverShuffleDirection = Player->GetCoverShuffleDirection();
    }
    else if (const AMGGuardActor* Guard = Cast<AMGGuardActor>(Owner))
    {
        AwarenessState = static_cast<int32>(Guard->GetAwarenessState());
        bInvestigating = Guard->IsInvestigatingNoise();
        bIncapacitated = Guard->IsIncapacitated();
    }
    else if (const AMGPrisonerCharacter* Fox = Cast<AMGPrisonerCharacter>(Owner))
    {
        bRescued = Fox->IsRescued();
    }
    AimBlendAlpha = FMath::FInterpTo(AimBlendAlpha,
        bAiming ? 1.f : 0.f, DeltaSeconds, 12.f);
    if (!bAiming)
    {
        AimPitchDegrees = FMath::FInterpTo(AimPitchDegrees, 0.f, DeltaSeconds, 12.f);
        AimYawDegrees = FMath::FInterpTo(AimYawDegrees, 0.f, DeltaSeconds, 12.f);
    }
}
