#include "MGCharacterAnimInstance.h"
#include "MGPlayerCharacter.h"
#include "MGGuardActor.h"
#include "MGPrisonerCharacter.h"
#include "MGShootGunnerActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"

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
    const float CurrentYaw = Owner->GetActorRotation().Yaw;
    bStartedMoving = bStoppedMoving = false;
    AccelerationCmPerSecSquared = 0.f;
    YawVelocityDegreesPerSec = 0.f;
    if (bHaveKinematicSample && DeltaSeconds > KINDA_SMALL_NUMBER)
    {
        AccelerationCmPerSecSquared = FMath::Clamp(
            (GroundSpeed - PreviousSpeed) / DeltaSeconds, -6000.f, 6000.f);
        YawVelocityDegreesPerSec = FMath::Clamp(
            FMath::FindDeltaAngleDegrees(PreviousYaw, CurrentYaw) / DeltaSeconds,
            -1440.f, 1440.f);
        bStartedMoving = PreviousSpeed < 18.f && GroundSpeed >= 18.f;
        bStoppedMoving = PreviousSpeed >= 18.f && GroundSpeed < 18.f;
    }
    PreviousSpeed = GroundSpeed;
    PreviousYaw = CurrentYaw;
    bHaveKinematicSample = true;
    BrakingBlend = FMath::FInterpTo(BrakingBlend,
        bMoving && AccelerationCmPerSecSquared < -180.f ? 1.f : 0.f,
        DeltaSeconds, 9.f);
    const FVector Local = Owner->GetActorTransform().InverseTransformVectorNoScale(Velocity);
    TravelDirectionDegrees = bMoving ?
        FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X)) : 0.f;

    bCrouched = bProne = bAiming = bSprinting = bInCover = false;
    bWeaponReady = false;
    FireRecoilAlpha = 0.f;
    bIsFiring = false;
    IntentStrength = 0.f;
    IntentDirectionDegrees = 0.f;
    bReversalRequested = false;
    bCornerPeeking = bKnocking = bInvestigating = bIncapacitated = bRescued = false;
    CoverShuffleDirection = 0.f;
    AwarenessState = 0;
    if (const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(Owner))
    {
        const FVector Input = Player->GetLastMovementInputVector().GetSafeNormal2D();
        IntentStrength = FMath::Clamp(Input.Size(), 0.f, 1.f);
        if (IntentStrength > 0.05f)
        {
            const FVector LocalInput = Owner->GetActorTransform().InverseTransformVectorNoScale(Input);
            IntentDirectionDegrees = FMath::RadiansToDegrees(
                FMath::Atan2(LocalInput.Y, LocalInput.X));
            bReversalRequested = bMoving && GroundSpeed > 60.f &&
                FVector::DotProduct(Input, Velocity.GetSafeNormal2D()) < -0.5f;
        }
        bCrouched = Player->GetCharacterMovement()->IsCrouching();
        bProne = Player->IsProne();
        bAiming = Player->IsAiming();
        bWeaponReady = Player->HasReadyWeapon();
        FireRecoilAlpha = Player->GetFiringPoseAlpha();
        bIsFiring = FireRecoilAlpha > 0.f && bWeaponReady;
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
    // Strict presentation priority: damage/death overrides all; committed
    // player actions override locomotion; communication is additive during
    // movement, so an incoming radio call never freezes Snake's legs.
    EMGPerformanceAction Desired = bMoving
        ? EMGPerformanceAction::Travel : EMGPerformanceAction::Idle;
    bRadioListeningOverlay = false;
    GunnerHealthFraction = 1.f;
    bWeaponReloading = false;
    RemainingVisibleRounds = -1;
    float TargetLeftGrip = 0.f;
    float TargetRightGrip = 0.f;
    if (const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(Owner))
    {
        bRadioListeningOverlay = Player->IsCodecOpen();
        // Never have the weapon grip on a captured/unarmed Snake.
        TargetRightGrip = bWeaponReady ? 1.f : 0.f;
        TargetLeftGrip = bWeaponReady && bAiming ? .7f : 0.f;
        if (Player->IsMeleeVisualActive()) Desired = EMGPerformanceAction::Melee;
        else if (bKnocking) Desired = EMGPerformanceAction::WallKnock;
        else if (bIsFiring) Desired = EMGPerformanceAction::Fire;
        else if (Player->HasPhysicalInteractionContact() &&
                 Player->GetPhysicalInteractionKind() == TEXT("CameraIsolator"))
            Desired = EMGPerformanceAction::EnvironmentInteract;
        else if (bProne) Desired = EMGPerformanceAction::Prone;
        else if (bCornerPeeking) Desired = EMGPerformanceAction::CoverPeek;
        else if (bInCover) Desired = EMGPerformanceAction::Cover;
        else if (bAiming) Desired = EMGPerformanceAction::Aim;
        else if (bCrouched) Desired = EMGPerformanceAction::Crouch;
        else if (bRadioListeningOverlay && !bMoving)
            Desired = EMGPerformanceAction::RadioIdle;
    }
    else if (const AMGGuardActor* Guard = Cast<AMGGuardActor>(Owner))
    {
        bWeaponReloading = Guard->IsReloadingSidearm();
        RemainingVisibleRounds = Guard->GetSidearmRounds();
        TargetRightGrip = 1.f;
        TargetLeftGrip = bWeaponReloading ? .8f : 0.f;
        switch (Guard->GetGuardPosture())
        {
        case EMGGuardPosture::SidearmAim: Desired = EMGPerformanceAction::GuardSidearmAim; break;
        case EMGGuardPosture::SidearmFire: Desired = EMGPerformanceAction::GuardSidearmFire; break;
        case EMGGuardPosture::SidearmReload: Desired = EMGPerformanceAction::GuardSidearmReload; break;
        case EMGGuardPosture::Notice: Desired = EMGPerformanceAction::GuardNotice; break;
        case EMGGuardPosture::Investigate: Desired = EMGPerformanceAction::GuardInvestigate; break;
        case EMGGuardPosture::Search: Desired = EMGPerformanceAction::GuardSearch; break;
        case EMGGuardPosture::Challenge: Desired = EMGPerformanceAction::GuardChallenge; break;
        case EMGGuardPosture::Report: Desired = EMGPerformanceAction::GuardReport; break;
        case EMGGuardPosture::Engage: Desired = EMGPerformanceAction::GuardEngage; break;
        case EMGGuardPosture::Strike: Desired = EMGPerformanceAction::GuardStrike; break;
        case EMGGuardPosture::Incapacitated: Desired = EMGPerformanceAction::Incapacitated; break;
        default: break; // Existing ground speed still drives locomotion clips.
        }
        if (Guard->IsIncapacitated()) Desired = EMGPerformanceAction::Incapacitated;
    }
    else if (const AMGPrisonerCharacter* Fox = Cast<AMGPrisonerCharacter>(Owner))
        Desired = Fox->IsRescued() ? EMGPerformanceAction::FoxRescued
                                  : EMGPerformanceAction::FoxPrisoner;
    else if (const AMGShootGunnerActor* Gunner = Cast<AMGShootGunnerActor>(Owner))
    {
        GunnerHealthFraction = Gunner->GetHealthFraction();
        bWeaponReloading = Gunner->IsReloading();
        RemainingVisibleRounds = Gunner->GetLoadedShells();
        TargetRightGrip = Gunner->IsDefeated() ? 0.f : 1.f;
        TargetLeftGrip = Gunner->IsDefeated() ? 0.f :
            (Gunner->IsReloading() ? .2f : 1.f);
        if (Gunner->IsDefeated()) Desired = EMGPerformanceAction::GunnerDefeated;
        else if (Gunner->IsFiringVolley()) Desired = EMGPerformanceAction::GunnerVolley;
        else if (Gunner->IsTelegraphing()) Desired = EMGPerformanceAction::GunnerAim;
        else if (Gunner->IsReloading()) Desired = EMGPerformanceAction::GunnerReload;
        else if (Gunner->IsRecovering()) Desired = EMGPerformanceAction::GunnerRecovery;
        else Desired = EMGPerformanceAction::GunnerSearch;
    }

    bPerformanceActionChanged = Desired != PerformanceAction;
    bPresentationInterrupted = false;
    if (bPerformanceActionChanged)
    {
        const EMGPerformanceAction Old = PerformanceAction;
        PreviousPerformanceAction = Old;
        bPresentationInterrupted =
            (Old == EMGPerformanceAction::GuardStrike &&
             Desired != EMGPerformanceAction::GuardStrike) ||
            (Old == EMGPerformanceAction::GunnerAim &&
             Desired != EMGPerformanceAction::GunnerVolley) ||
            (Old == EMGPerformanceAction::GunnerVolley &&
             Desired != EMGPerformanceAction::GunnerRecovery &&
             Desired != EMGPerformanceAction::GunnerDefeated);
        PerformanceAction = Desired;
        PerformanceActionElapsedSeconds = 0.f;
        OnPerformanceActionTransition(Old, Desired, bPresentationInterrupted);
    }
    else PerformanceActionElapsedSeconds += FMath::Max(0.f, DeltaSeconds);
    // Base transition duration exposes an actual, framerate-independent blend
    // parameter to the AnimBP. Never delay the authoritative guard/shot state.
    const float BlendSeconds =
        (Desired == EMGPerformanceAction::Prone || Desired == EMGPerformanceAction::Cover)
        ? .26f : ((Desired == EMGPerformanceAction::Fire ||
                   Desired == EMGPerformanceAction::WallKnock ||
                   Desired == EMGPerformanceAction::GuardStrike) ? .08f : .17f);
    PerformanceTransitionAlpha = FMath::Clamp(
        PerformanceActionElapsedSeconds / BlendSeconds, 0.f, 1.f);
    // Secondary channels let authored blend graphs interpolate into stances
    // without ever replacing the movement capsule's real clearance checks.
    CrouchTransitionAlpha = FMath::FInterpTo(
        CrouchTransitionAlpha, bCrouched ? 1.f : 0.f, DeltaSeconds, 8.f);
    ProneTransitionAlpha = FMath::FInterpTo(
        ProneTransitionAlpha, bProne ? 1.f : 0.f, DeltaSeconds, 5.5f);
    CoverTransitionAlpha = FMath::FInterpTo(
        CoverTransitionAlpha, bInCover ? 1.f : 0.f, DeltaSeconds, 10.f);
    RadioListenOverlayAlpha = FMath::FInterpTo(
        RadioListenOverlayAlpha, bRadioListeningOverlay ? 1.f : 0.f, DeltaSeconds, 8.f);
    // A hand placement is only approved when the real rig has a weapon_r
    // joint and the visually-held imported weapon supplies its own
    // support_hand socket. Never infer IK success from an equipment ID.
    const UStaticMeshComponent* ActualWeapon = nullptr;
    const USkeletalMeshComponent* ActualRig = nullptr;
    if (const AMGPlayerCharacter* Snake = Cast<AMGPlayerCharacter>(Owner))
    {
        ActualWeapon = Snake->GetHeldWeaponVisual();
        ActualRig = Snake->GetMesh();
    }
    else if (const AMGGuardActor* Guard = Cast<AMGGuardActor>(Owner))
    {
        ActualWeapon = Guard->GetHeldWeaponVisual();
        ActualRig = Guard->GetRigMesh();
    }
    else if (const AMGShootGunnerActor* Gunner = Cast<AMGShootGunnerActor>(Owner))
    {
        ActualWeapon = Gunner->GetHeldWeaponVisual();
        ActualRig = Gunner->GetRigMesh();
    }
    bHasValidRightWeaponSocket = ActualRig && ActualRig->GetSkeletalMeshAsset()
        && ActualRig->DoesSocketExist(TEXT("weapon_r"))
        && ActualWeapon && ActualWeapon->IsVisible()
        && ActualWeapon->GetStaticMesh() != nullptr;
    bHasValidSupportHandSocket = bHasValidRightWeaponSocket
        && ActualWeapon->DoesSocketExist(TEXT("support_hand"));
    if (bHasValidSupportHandSocket)
        SupportHandTargetWS = ActualWeapon->GetSocketTransform(
            TEXT("support_hand"), RTS_World);
    TargetRightGrip = bHasValidRightWeaponSocket ? TargetRightGrip : 0.f;
    TargetLeftGrip = bHasValidSupportHandSocket ? TargetLeftGrip : 0.f;
    WeaponReloadBlendAlpha = FMath::FInterpTo(WeaponReloadBlendAlpha,
        bWeaponReloading ? 1.f : 0.f, DeltaSeconds, 9.f);
    RightHandGripAlpha = FMath::FInterpTo(RightHandGripAlpha,
        TargetRightGrip, DeltaSeconds, 13.f);
    LeftHandGripAlpha = FMath::FInterpTo(LeftHandGripAlpha,
        TargetLeftGrip, DeltaSeconds, 11.f);
    ImpactPoseAlpha = FMath::FInterpTo(
        ImpactPoseAlpha, (PerformanceAction == EMGPerformanceAction::Melee ||
                          PerformanceAction == EMGPerformanceAction::GuardStrike)
                         ? 1.f : 0.f, DeltaSeconds, 13.f);

    AimBlendAlpha = FMath::FInterpTo(AimBlendAlpha,
        bAiming ? 1.f : 0.f, DeltaSeconds, 12.f);
    if (!bAiming)
    {
        AimPitchDegrees = FMath::FInterpTo(AimPitchDegrees, 0.f, DeltaSeconds, 12.f);
        AimYawDegrees = FMath::FInterpTo(AimYawDegrees, 0.f, DeltaSeconds, 12.f);
    }
    // PHYSICAL ARTICULATION / CONTACT PASS
    // Not a procedural animation replacement: exact-bone offsets and planted
    // foot normals are input to authored animation layers and IK solvers.
    // The native collision/movement controller remains the sole authority.
    const USkeletalMeshComponent* SkeletalBody = GetSkelMeshComponent();
    const UWorld* GroundWorld = SkeletalBody ? SkeletalBody->GetWorld() : nullptr;
    const bool bHasFeet = SkeletalBody && SkeletalBody->GetSkeletalMeshAsset()
        && SkeletalBody->DoesSocketExist(TEXT("foot_l"))
        && SkeletalBody->DoesSocketExist(TEXT("foot_r"));
    const bool bFeetCanPlant = bHasFeet && GroundWorld
        && !bProne && !bIncapacitated && GroundSpeed < 420.f;
    float LeftDesired = 0.f, RightDesired = 0.f;
    bool bLeftGrounded = false, bRightGrounded = false;
    FVector LeftNormal = FVector::UpVector;
    FVector RightNormal = FVector::UpVector;
    if (bFeetCanPlant)
    {
        auto ProbeSole = [GroundWorld, SkeletalBody, Owner](
            const FName Bone, float& Offset, FVector& Normal) -> bool
        {
            const FVector Sole = SkeletalBody->GetSocketLocation(Bone);
            FHitResult GroundHit;
            FCollisionQueryParams Params(SCENE_QUERY_STAT(MGGroundedFoot), false, Owner);
            const bool bHit = GroundWorld->LineTraceSingleByChannel(
                GroundHit, Sole + FVector(0.f, 0.f, 32.f),
                Sole - FVector(0.f, 0.f, 76.f), ECC_Visibility, Params);
            if (!bHit || GroundHit.ImpactNormal.Z < .66f) return false;
            // Small limits stop impossible leg stretching on stairs and
            // mean we do not hide an unreachable ledge with a joint offset.
            Offset = FMath::Clamp(GroundHit.ImpactPoint.Z - Sole.Z + 2.f,
                -14.f, 14.f);
            Normal = GroundHit.ImpactNormal;
            return true;
        };
        bLeftGrounded = ProbeSole(TEXT("foot_l"), LeftDesired, LeftNormal);
        bRightGrounded = ProbeSole(TEXT("foot_r"), RightDesired, RightNormal);
    }
    bValidGroundFootProbes = bLeftGrounded && bRightGrounded;
    LeftGroundNormalWS = bLeftGrounded ? LeftNormal : FVector::UpVector;
    RightGroundNormalWS = bRightGrounded ? RightNormal : FVector::UpVector;
    // No phantom foot IK while falling, sprinting or missing rig/sockets.
    const float PlantWeight = bValidGroundFootProbes
        ? FMath::Clamp((420.f - GroundSpeed) / 290.f, .0f, 1.f) : 0.f;
    FootGroundingAlpha = FMath::FInterpTo(
        FootGroundingAlpha, PlantWeight, DeltaSeconds, 12.f);
    LeftFootGroundOffsetCm = FMath::FInterpTo(
        LeftFootGroundOffsetCm, bLeftGrounded ? LeftDesired : 0.f, DeltaSeconds, 13.f);
    RightFootGroundOffsetCm = FMath::FInterpTo(
        RightFootGroundOffsetCm, bRightGrounded ? RightDesired : 0.f, DeltaSeconds, 13.f);
    PelvisGroundOffsetCm = FMath::FInterpTo(
        PelvisGroundOffsetCm, bValidGroundFootProbes
            ? FMath::Clamp(FMath::Min(LeftDesired, RightDesired), -12.f, 0.f) : 0.f,
        DeltaSeconds, 11.f);

    bValidSurfaceHandTarget = false;
    FVector DesiredSurfaceTarget = FVector::ZeroVector;
    // Character actors with no compatible left hand should not be advertised
    // to the AnimBP as having valid environmental hand contact.
    const bool bCanReachWithLeftHand = SkeletalBody &&
        SkeletalBody->GetSkeletalMeshAsset() &&
        SkeletalBody->DoesSocketExist(TEXT("hand_l"));
    if (bCanReachWithLeftHand)
    {
        if (const AMGPlayerCharacter* Snake = Cast<AMGPlayerCharacter>(Owner))
        {
            if (Snake->HasPhysicalInteractionContact())
            {
                DesiredSurfaceTarget = Snake->GetPhysicalInteractionWorldTarget();
                bValidSurfaceHandTarget = true;
            }
            else if (Snake->IsInCover() && !Snake->IsAiming() && GroundWorld)
            {
                const FVector Normal = Snake->GetCoverSurfaceNormal();
                if (!Normal.IsNearlyZero())
                {
                    const FVector Chest = Snake->GetActorLocation() +
                        FVector(0.f, 0.f, bCrouched ? 38.f : 70.f);
                    FCollisionQueryParams Params(
                        SCENE_QUERY_STAT(MGPhysicalWallHandContact), false, Owner);
                    FHitResult Wall;
                    if (GroundWorld->LineTraceSingleByChannel(
                            Wall, Chest, Chest - Normal * 110.f,
                            ECC_Visibility, Params) &&
                        FVector::DotProduct(Wall.ImpactNormal.GetSafeNormal2D(),
                            Normal.GetSafeNormal2D()) > .60f)
                    {
                        DesiredSurfaceTarget = Wall.ImpactPoint + Normal * 3.f;
                        bValidSurfaceHandTarget = true;
                    }
                }
            }
        }
    }
    if (bValidSurfaceHandTarget)
        SurfaceHandTargetWS = DesiredSurfaceTarget;
    SurfaceHandContactAlpha = FMath::FInterpTo(SurfaceHandContactAlpha,
        bValidSurfaceHandTarget ? 1.f : 0.f, DeltaSeconds, 12.f);

}
