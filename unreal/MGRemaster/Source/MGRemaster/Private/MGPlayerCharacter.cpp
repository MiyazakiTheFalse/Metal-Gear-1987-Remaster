#include "MGPlayerCharacter.h"
#include "MGGrayFoxMission.h"
#include "MGGuardActor.h"
#include "MGEquipmentComponent.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CollisionShape.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AMGPlayerCharacter::AMGPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(35.f, 89.f);
    Equipment = CreateDefaultSubobject<UMGEquipmentComponent>(TEXT("Equipment"));
    // Reference height: 178 cm (official MG2 profile; provisional for 1995).
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    GetCharacterMovement()->CrouchedHalfHeight = 55.f;
    GetCharacterMovement()->MaxWalkSpeed = 350.f;
    GetCharacterMovement()->MaxAcceleration = 1800.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2200.f;
    GetCharacterMovement()->GroundFriction = 7.f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 650.f, 0.f);
    bUseControllerRotationYaw = false;

    PrototypeBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBody"));
    PrototypeBody->SetupAttachment(GetCapsuleComponent());
    PrototypeBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // UE BasicShapes Cylinder: 100 cm high at scale 1; Snake proxy = 178 cm.
    PrototypeBody->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.78f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (Cylinder.Succeeded()) PrototypeBody->SetStaticMesh(Cylinder.Object);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 420.f;
    CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 65.f));
    CameraBoom->SocketOffset = FVector(0.f, 65.f, 10.f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.f;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 18.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);
    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->FieldOfView = 85.f;
}

void AMGPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (GEngine && GEngine->GetGameUserSettings())
    {
        QualityPreset = FMath::Clamp(GEngine->GetGameUserSettings()->GetOverallScalabilityLevel(), 0, 3);
    }
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
        {
            CameraManager->ViewPitchMin = -65.f;
            CameraManager->ViewPitchMax = 55.f;
        }
    }
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        // A paused Codec must still accept its close button.
        PC->PrimaryActorTick.bTickEvenWhenPaused = true;
    }
    ApplyMenuTimeState();
    UpdateMovementState();
}

void AMGPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Ensure a checkpoint restart cannot inherit a slowed or paused world.
    if (GetWorld())
    {
        UGameplayStatics::SetGamePaused(this, false);
        UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
    }
    Super::EndPlay(EndPlayReason);
}

void AMGPlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    CombatNoiseTimer = FMath::Max(0.f, CombatNoiseTimer - DeltaSeconds);
    MeleeVisualTimer = FMath::Max(0.f, MeleeVisualTimer - DeltaSeconds);
    // Start prone only once CharacterMovement has established a valid crouch
    // base position. Never fake a prone pose with a standing collision capsule.
    if (bPendingProne && bIsCrouched && !bIsProne)
        EnterProneAfterCrouch();

    if (GetMesh())
        PrototypeBody->SetVisibility(GetMesh()->GetSkeletalMeshAsset() == nullptr, true);

    // A temporary prone/cover camera uses real collision and interpolated
    // pivot/shoulder geometry. Artists can replace the cylinder with a rig.
    const float TargetLength = bIsProne ? (bAiming ? 170.f : 275.f) :
        (bAiming ? 190.f : (bInCover ? 320.f : 420.f));
    const float TargetFOV = bAiming ? 65.f : 85.f;
    const float ShoulderWidth = bIsProne ? 24.f : (bInCover ? 46.f : (bAiming ? 58.f : 65.f));
    const FVector PivotLocal(0.f, 0.f, bIsProne ? 18.f : (bIsCrouched ? 40.f : 65.f));
    FVector TargetSocket(0.f, ShoulderSign * ShoulderWidth, bIsProne ? 8.f : (bAiming ? 18.f : 10.f));

    // Shoulder offset must also respect nearby cover. Spring-arm probe alone
    // handles camera distance, but the sideways socket needs its own sweep.
    if (GetWorld())
    {
        const FVector Pivot = CameraBoom->GetComponentLocation();
        const FVector SideWorld = CameraBoom->GetComponentRotation().RotateVector(
            FVector(0.f, TargetSocket.Y, 0.f));
        FCollisionQueryParams Params(SCENE_QUERY_STAT(MGCameraShoulderSweep), false, this);
        FHitResult Hit;
        if (GetWorld()->SweepSingleByChannel(Hit, Pivot, Pivot + SideWorld,
                FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(12.f), Params))
        {
            TargetSocket.Y *= FMath::Clamp(Hit.Time - 0.10f, 0.f, 1.f);
        }
    }

    const FVector TargetPivot = PivotLocal;
    CameraBoom->SetRelativeLocation(FMath::VInterpTo(CameraBoom->GetRelativeLocation(), TargetPivot, DeltaSeconds, 12.f));
    const FVector TargetProxyScale = bIsProne ?
        FVector(0.50f, 0.50f, 1.48f) :
        FVector(0.7f, 0.7f, bIsCrouched ? 1.10f : 1.78f);
    const FRotator ProxyRotation = bIsProne ? FRotator(88.f, 0.f, 0.f) : FRotator::ZeroRotator;
    PrototypeBody->SetRelativeRotation(FMath::RInterpTo(
        PrototypeBody->GetRelativeRotation(), ProxyRotation, DeltaSeconds, 12.f));
    PrototypeBody->SetRelativeScale3D(FMath::VInterpTo(PrototypeBody->GetRelativeScale3D(), TargetProxyScale, DeltaSeconds, 12.f));
    // Clear visual windup/recovery cue until a source-accurate animation rig exists.
    const float AttackOffset = MeleeVisualTimer > 0.f ?
        17.f * FMath::Sin(FMath::Clamp(MeleeVisualTimer / 0.22f, 0.f, 1.f) * PI) : 0.f;
    PrototypeBody->SetRelativeLocation(FMath::VInterpTo(
        PrototypeBody->GetRelativeLocation(), FVector(AttackOffset, 0.f, 0.f),
        DeltaSeconds, 19.f));
    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetLength, DeltaSeconds, 10.f);
    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocket, DeltaSeconds, 10.f);
    FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, 11.f));

    // Dedicated cover needs to remain attached to a real surface. Do not
    // magnetise through a wall or continue covering after turning away.
    if (bInCover && GetWorld())
    {
        FHitResult Check;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(MGCoverContinuity), false, this);
        const FVector From = GetActorLocation() + FVector(0.f, 0.f, 34.f);
        const bool bWall = GetWorld()->LineTraceSingleByChannel(Check,
            From, From - CoverNormal * 115.f, ECC_Visibility, Query);
        if (!bWall || FMath::Abs(Check.ImpactNormal.Z) > 0.22f ||
            FVector::DotProduct(Check.ImpactNormal.GetSafeNormal2D(),
                                CoverNormal.GetSafeNormal2D()) < 0.70f)
            LeaveCover();
        else
            SetActorRotation(FMath::RInterpTo(GetActorRotation(),
                (-CoverNormal).Rotation(), DeltaSeconds, 14.f));
    }
}

void AMGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis("MoveForward", this, &AMGPlayerCharacter::MoveForward);
    Input->BindAxis("MoveRight", this, &AMGPlayerCharacter::MoveRight);
    Input->BindAxis("Turn", this, &AMGPlayerCharacter::Turn);
    Input->BindAxis("LookUp", this, &AMGPlayerCharacter::LookUp);
    Input->BindAxis("TurnRate", this, &AMGPlayerCharacter::TurnRate);
    Input->BindAxis("LookUpRate", this, &AMGPlayerCharacter::LookUpRate);
    Input->BindAction("Aim", IE_Pressed, this, &AMGPlayerCharacter::AimStart);
    Input->BindAction("Aim", IE_Released, this, &AMGPlayerCharacter::AimStop);
    Input->BindAction("Sprint", IE_Pressed, this, &AMGPlayerCharacter::SprintStart);
    Input->BindAction("Sprint", IE_Released, this, &AMGPlayerCharacter::SprintStop);
    Input->BindAction("Crouch", IE_Pressed, this, &AMGPlayerCharacter::CrouchToggle);
    Input->BindAction("ShoulderSwap", IE_Pressed, this, &AMGPlayerCharacter::ShoulderSwap);
    Input->BindAction("Fire", IE_Pressed, this, &AMGPlayerCharacter::FireStun);
    Input->BindAction("GraphicsPreset", IE_Pressed, this, &AMGPlayerCharacter::CycleGraphicsQuality);
    Input->BindAction("Interact", IE_Pressed, this, &AMGPlayerCharacter::Interact);
    Input->BindAction("RestartRescue", IE_Pressed, this, &AMGPlayerCharacter::RestartRescue);
    Input->BindAction("MissionLog", IE_Pressed, this, &AMGPlayerCharacter::ToggleMissionLog);
    Input->BindAction("WeaponWindow", IE_Pressed, this, &AMGPlayerCharacter::WeaponWindowStart);
    Input->BindAction("WeaponWindow", IE_Released, this, &AMGPlayerCharacter::WeaponWindowStop);
    Input->BindAction("ItemWindow", IE_Pressed, this, &AMGPlayerCharacter::ItemWindowStart);
    Input->BindAction("ItemWindow", IE_Released, this, &AMGPlayerCharacter::ItemWindowStop);
    Input->BindAction("SelectPrevious", IE_Pressed, this, &AMGPlayerCharacter::SelectionPrevious);
    Input->BindAction("SelectNext", IE_Pressed, this, &AMGPlayerCharacter::SelectionNext);
    Input->BindAction("UseItem", IE_Pressed, this, &AMGPlayerCharacter::UseItem);
    Input->BindAction("Melee", IE_Pressed, this, &AMGPlayerCharacter::MeleeAttack);
    Input->BindAction("Prone", IE_Pressed, this, &AMGPlayerCharacter::ToggleProne);
    Input->BindAction("Cover", IE_Pressed, this, &AMGPlayerCharacter::ToggleCover);
    FInputActionBinding& CodecBinding =
        Input->BindAction("Codec", IE_Pressed, this, &AMGPlayerCharacter::ToggleCodec);
    CodecBinding.bExecuteWhenPaused = true;
}
void AMGPlayerCharacter::MoveForward(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
        if (bInCover) ApplyCoverMovement(Value, Direction);
        else if (bIsProne)
        {
            // The upright controller capsule alone does not cover a 1.5m body.
            // Require a forward-aligned prone clearance box at the next step.
            FCollisionQueryParams Query(SCENE_QUERY_STAT(MGProneMovement), false, this);
            FHitResult Hit;
            const FVector Delta = Direction * Value * 95.f *
                (GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f);
            const bool bHit = GetWorld() && GetWorld()->SweepSingleByChannel(
                Hit, GetActorLocation(), GetActorLocation()+Delta, GetActorQuat(),
                ECC_Pawn, FCollisionShape::MakeBox(FVector(74.f, 29.f, 27.f)), Query);
            if (!bHit) AddMovementInput(Direction, Value);
        }
        else AddMovementInput(Direction, Value);
    }
}
void AMGPlayerCharacter::MoveRight(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
        if (bInCover) ApplyCoverMovement(Value, Direction);
        else if (bIsProne)
        {
            FCollisionQueryParams Query(SCENE_QUERY_STAT(MGProneMovement), false, this);
            FHitResult Hit;
            const FVector Delta = Direction * Value * 95.f *
                (GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f);
            const bool bHit = GetWorld() && GetWorld()->SweepSingleByChannel(
                Hit, GetActorLocation(), GetActorLocation()+Delta, GetActorQuat(),
                ECC_Pawn, FCollisionShape::MakeBox(FVector(74.f, 29.f, 27.f)), Query);
            if (!bHit) AddMovementInput(Direction, Value);
        }
        else AddMovementInput(Direction, Value);
    }
}
void AMGPlayerCharacter::Turn(float Value) { AddControllerYawInput(Value); }
void AMGPlayerCharacter::LookUp(float Value) { AddControllerPitchInput(Value); }
// Unlike raw mouse deltas, gamepad sticks represent velocity and require delta time.
void AMGPlayerCharacter::TurnRate(float Value)
{
    if (GetWorld()) AddControllerYawInput(Value * 100.f * GetWorld()->GetDeltaSeconds());
}
void AMGPlayerCharacter::LookUpRate(float Value)
{
    if (GetWorld()) AddControllerPitchInput(Value * 80.f * GetWorld()->GetDeltaSeconds());
}

void AMGPlayerCharacter::AimStart()
{
    bAiming = true;
    bSprinting = false;
    UpdateMovementState();
}
void AMGPlayerCharacter::AimStop() { bAiming = false; UpdateMovementState(); }
void AMGPlayerCharacter::SprintStart()
{
    if (!bAiming && !bIsCrouched && !bIsProne)
    {
        if (bInCover) LeaveCover();
        bSprinting = true;
        UpdateMovementState();
    }
}
void AMGPlayerCharacter::SprintStop() { bSprinting = false; UpdateMovementState(); }
void AMGPlayerCharacter::CrouchToggle()
{
    bSprinting = false;
    if (bIsProne)
    {
        ExitProneSafely(); // Prohibited under a low ceiling until space exists.
    }
    else if (bIsCrouched) UnCrouch();
    else Crouch();
    UpdateMovementState();
}
void AMGPlayerCharacter::ShoulderSwap() { ShoulderSign = -ShoulderSign; }

void AMGPlayerCharacter::UpdateMovementState()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = !bAiming && !bInCover;
    Movement->bUseControllerDesiredRotation = bAiming && !bInCover;
    Movement->MaxWalkSpeed = bIsProne ? 95.f :
        (bInCover ? 120.f : (bIsCrouched ? 170.f :
            (bAiming ? 210.f : (bSprinting ? 600.f : 350.f))));
    Movement->MaxWalkSpeedCrouched = 170.f;
}
float AMGPlayerCharacter::GetNoiseFactor() const
{
    const float Speed = GetVelocity().Size2D();
    if (CombatNoiseTimer > 0.f) return 0.95f;
    if (Speed < 15.f) return 0.f;
    if (bIsProne) return 0.035f;
    if (bInCover) return 0.10f;
    if (bIsCrouched) return 0.08f;
    if (IsSprinting()) return 0.95f;
    return bAiming ? 0.14f : 0.30f;
}
float AMGPlayerCharacter::GetVisibilityFactor() const
{
    if (bIsProne) return 0.21f;
    if (bInCover) return bIsCrouched ? 0.31f : 0.65f;
    return bIsCrouched ? 0.36f : 1.f;
}

void AMGPlayerCharacter::Interact()
{
    if (AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
        UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass())))
    {
        const uint32 PreviousFlags = Mission->GetMissionFlags();
        if (Mission->TryInteract(this))
        {
            const uint32 Changed = PreviousFlags ^ Mission->GetMissionFlags();
            if (Changed & 4u) NotifyFeedback(TEXT("WALL BREACHED — NEW ROUTE"), TEXT("breach"));
            else if (Changed & 8u) NotifyFeedback(TEXT("GRAY FOX RESCUED — INTEL LOG UPDATED"), TEXT("rescue"));
            else if (Changed & 16u) NotifyFeedback(TEXT("ESCAPE ROUTE OPEN"), TEXT("breach"));
        }
        else if (bInCover)
        {
            LeaveCover();
            NotifyFeedback(TEXT("LEFT COVER"), TEXT("select"));
        }
        else if (!TryEnterCover())
            NotifyFeedback(TEXT("NO ACTION AVAILABLE — MOVE CLOSER AND FACE TARGET"), TEXT("denied"));
    }
}

void AMGPlayerCharacter::ToggleMissionLog()
{
    if (!bCodecOpen) bShowMissionLog = !bShowMissionLog;
}

void AMGPlayerCharacter::RestartRescue()
{
    if (AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
        UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass())))
    {
        Mission->RestartMission();
    }
}


FString AMGPlayerCharacter::GetPostureText() const
{
    if (bIsProne) return TEXT("PRONE");
    if (bInCover) return TEXT("WALL COVER");
    if (bIsCrouched) return TEXT("CROUCHED");
    return TEXT("STANDING");
}
void AMGPlayerCharacter::ApplyCoverMovement(float Value, const FVector& Direction)
{
    if (FMath::IsNearlyZero(Value)) return;
    const FVector Tangent = FVector::CrossProduct(FVector::UpVector, CoverNormal).GetSafeNormal2D();
    const float AlongWall = FVector::DotProduct(Direction.GetSafeNormal2D(), Tangent);
    AddMovementInput(Tangent, AlongWall * Value);
}
void AMGPlayerCharacter::EnterProneAfterCrouch()
{
    bPendingProne = false;
    if (!bIsCrouched || bIsProne) return;
    // Maintain capsule base height as its upper half shrinks.
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    const float Before = Capsule->GetUnscaledCapsuleHalfHeight();
    const float ProneHalf = 38.f;
    if (Before <= ProneHalf) return;
    const float Delta = Before - ProneHalf;
    const FVector FutureCenter = GetActorLocation() - FVector(0.f, 0.f, Delta);
    FCollisionQueryParams Clearance(SCENE_QUERY_STAT(MGProneBodyClearance), false, this);
    if (GetWorld() && GetWorld()->OverlapBlockingTestByChannel(
        FutureCenter, GetActorQuat(), ECC_Pawn,
        FCollisionShape::MakeBox(FVector(74.f, 29.f, 27.f)), Clearance))
    {
        NotifyFeedback(TEXT("NOT ENOUGH ROOM TO LIE FLAT"), TEXT("denied"));
        return;
    }
    Capsule->SetCapsuleHalfHeight(ProneHalf, true);
    FHitResult Hit;
    const bool bMoved = SetActorLocation(
        GetActorLocation() - FVector(0.f, 0.f, Delta), true, &Hit);
    if (!bMoved || FVector::Dist(GetActorLocation(), FutureCenter) > 2.f)
    {
        // Preserve capsule height if the floor move was obstructed; no
        // teleporting into geometry just to complete a stance.
        Capsule->SetCapsuleHalfHeight(Before, true);
        NotifyFeedback(TEXT("CANNOT GO PRONE HERE"), TEXT("denied"));
        return;
    }
    bIsProne = true;
    if (bInCover) LeaveCover();
    bSprinting = false;
    UpdateMovementState();
    NotifyFeedback(TEXT("PRONE — LOW PROFILE / SLOW MOVEMENT"), TEXT("select"));
}
bool AMGPlayerCharacter::ExitProneSafely()
{
    if (!bIsProne || !GetWorld()) return false;
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    const float CurrentHalf = Capsule->GetUnscaledCapsuleHalfHeight();
    const float CrouchedHalf = 55.f;
    if (CurrentHalf >= CrouchedHalf - 0.5f)
    {
        bIsProne = false;
        UpdateMovementState();
        return true;
    }
    const float Delta = CrouchedHalf - CurrentHalf;
    const FVector NewCenter = GetActorLocation() + FVector(0.f, 0.f, Delta);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MGProneRiseClearance), false, this);
    if (GetWorld()->OverlapBlockingTestByChannel(NewCenter, FQuat::Identity,
        ECC_Pawn, FCollisionShape::MakeCapsule(Capsule->GetUnscaledCapsuleRadius(), CrouchedHalf), Query))
    {
        NotifyFeedback(TEXT("NOT ENOUGH ROOM TO RISE"), TEXT("denied"));
        return false;
    }
    FHitResult Hit;
    if (!SetActorLocation(NewCenter, true, &Hit))
    {
        NotifyFeedback(TEXT("CANNOT RISE HERE"), TEXT("denied"));
        return false;
    }
    Capsule->SetCapsuleHalfHeight(CrouchedHalf, true);
    bIsProne = false;
    UpdateMovementState();
    NotifyFeedback(TEXT("CROUCHED — STANCE RESTORED"), TEXT("select"));
    return true;
}
void AMGPlayerCharacter::ToggleProne()
{
    if (bIsProne) { ExitProneSafely(); return; }
    if (bPendingProne) return;
    bSprinting = false;
    if (!bIsCrouched)
    {
        bPendingProne = true;
        Crouch();
    }
    else EnterProneAfterCrouch();
    UpdateMovementState();
}
bool AMGPlayerCharacter::TryEnterCover()
{
    if (!GetWorld() || bIsProne || !Controller) return false;
    const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, bIsCrouched ? 20.f : 36.f);
    const FVector TowardWall = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f).Vector();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MGTakeCover), false, this);
    FHitResult Hit;
    if (!GetWorld()->SweepSingleByChannel(Hit, Eye, Eye + TowardWall * 98.f,
        FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(12.f), Query)
        || FMath::Abs(Hit.ImpactNormal.Z) > 0.22f)
        return false;
    CoverNormal = Hit.ImpactNormal.GetSafeNormal2D();
    bInCover = !CoverNormal.IsNearlyZero();
    bSprinting = false;
    UpdateMovementState();
    if (bInCover) NotifyFeedback(TEXT("COVER TAKEN — MOVE ALONG WALL"), TEXT("select"));
    return bInCover;
}
void AMGPlayerCharacter::LeaveCover()
{
    bInCover = false;
    CoverNormal = FVector::ZeroVector;
    UpdateMovementState();
}
void AMGPlayerCharacter::ToggleCover()
{
    if (bInCover)
    {
        LeaveCover();
        NotifyFeedback(TEXT("LEFT COVER"), TEXT("select"));
    }
    else if (!TryEnterCover())
    {
        NotifyFeedback(TEXT("NO SUITABLE WALL WITHIN REACH"), TEXT("denied"));
    }
}

void AMGPlayerCharacter::FireStun()
{
    if (!GetWorld() || !Controller) return;
    if (!bAiming)
    {
        NotifyFeedback(TEXT("HOLD AIM BEFORE FIRING"), TEXT("denied"));
        return;
    }
    if (!Equipment || !Equipment->IsStunEquipped())
    {
        NotifyFeedback(Equipment && Equipment->IsConfiscated() ?
            TEXT("WEAPONS CONFISCATED") : TEXT("NO WEAPON EQUIPPED"), TEXT("denied"));
        return;
    }
    if (!Equipment->ConsumeStunRound())
    {
        NotifyFeedback(TEXT("OUT OF AMMUNITION"), TEXT("denied"));
        return;
    }
    NotifyFeedback(TEXT("STUN ROUND FIRED"), TEXT("stun"));
    CombatNoiseTimer = 0.55f;
    // Camera ray determines crosshair target; a second ray enforces obstruction
    // between the character and target. Prototype nonlethal incapacitation only.
    const FVector CameraStart = FollowCamera->GetComponentLocation();
    const FVector CameraEnd = CameraStart + FollowCamera->GetForwardVector() * 14000.f;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MGStunAim), true, this);
    FHitResult CameraHit;
    const bool bCameraHit = GetWorld()->LineTraceSingleByChannel(CameraHit, CameraStart, CameraEnd, ECC_Visibility, Params);
    const FVector Target = bCameraHit ? CameraHit.ImpactPoint : CameraEnd;
    const FVector Muzzle = GetActorLocation() + FVector(0.f, 0.f, 45.f);
    FHitResult MuzzleHit;
    if (GetWorld()->LineTraceSingleByChannel(MuzzleHit, Muzzle, Target, ECC_Visibility, Params) && MuzzleHit.GetActor())
    {
        UGameplayStatics::ApplyDamage(MuzzleHit.GetActor(), 100.f, Controller, this, UDamageType::StaticClass());
    }
}

void AMGPlayerCharacter::NotifyFeedback(const FString& Message, FName CueName)
{
    if (Equipment) Equipment->SetFeedback(Message);
    if (CueName.IsNone()) return;
    // Asset pipeline imports original placeholder WAVs to /Game/Feedback.
    // Absence is safe: visible feedback still works. No extracted licensed SFX
    // are included in the public repository.
    const FString Name = CueName.ToString();
    const FString ObjectRef = FString::Printf(
        TEXT("/Game/Feedback/%s.%s"), *Name, *Name);
    if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, *ObjectRef))
        UGameplayStatics::PlaySound2D(this, Sound, 0.65f);
}
void AMGPlayerCharacter::WeaponWindowStart()
{
    if (bCodecOpen) return;
    if (Equipment)
    {
        Equipment->OpenWeaponWindow();
        ApplyMenuTimeState();
        NotifyFeedback(TEXT("WEAPON SELECTION — TIME SLOWED"), TEXT("menu"));
    }
}
void AMGPlayerCharacter::WeaponWindowStop()
{
    if (Equipment)
    {
        const int32 Selected = Equipment->GetWeaponSelection();
        Equipment->CloseWeaponWindow();
        ApplyMenuTimeState();
        NotifyFeedback(Equipment->GetEquippedWeaponIndex() == Selected ?
            TEXT("WEAPON CONFIRMED") : TEXT("WEAPON UNAVAILABLE"), 
            Equipment->GetEquippedWeaponIndex() == Selected ? TEXT("select") : TEXT("denied"));
    }
}
void AMGPlayerCharacter::ItemWindowStart()
{
    if (bCodecOpen) return;
    if (Equipment)
    {
        Equipment->OpenItemWindow();
        ApplyMenuTimeState();
        NotifyFeedback(TEXT("ITEM SELECTION — TIME SLOWED"), TEXT("menu"));
    }
}
void AMGPlayerCharacter::ItemWindowStop()
{
    if (Equipment)
    {
        const int32 Selected = Equipment->GetItemSelection();
        Equipment->CloseItemWindow();
        ApplyMenuTimeState();
        NotifyFeedback(Equipment->GetEquippedItemIndex() == Selected ?
            TEXT("ITEM CONFIRMED") : TEXT("ITEM UNAVAILABLE"),
            Equipment->GetEquippedItemIndex() == Selected ? TEXT("select") : TEXT("denied"));
    }
}
void AMGPlayerCharacter::SelectionPrevious()
{
    if (Equipment && (Equipment->IsItemWindowOpen() || Equipment->IsWeaponWindowOpen()))
    { Equipment->SelectionPrevious(); NotifyFeedback(TEXT("SELECTION MOVED"), TEXT("menu")); }
}
void AMGPlayerCharacter::SelectionNext()
{
    if (Equipment && (Equipment->IsItemWindowOpen() || Equipment->IsWeaponWindowOpen()))
    { Equipment->SelectionNext(); NotifyFeedback(TEXT("SELECTION MOVED"), TEXT("menu")); }
}
void AMGPlayerCharacter::UseItem()
{
    if (!Equipment || Equipment->IsConfiscated())
    { NotifyFeedback(TEXT("ITEMS CONFISCATED"), TEXT("denied")); return; }
    if (Health >= 100.f)
    { NotifyFeedback(TEXT("HEALTH FULL"), TEXT("denied")); return; }
    if (Equipment->ConsumeRation())
    {
        Health = FMath::Min(100.f, Health + 45.f);
        NotifyFeedback(TEXT("RATION USED — HEALTH RESTORED"), TEXT("select"));
    }
    else NotifyFeedback(TEXT("EQUIP A RATION FIRST"), TEXT("denied"));
}
void AMGPlayerCharacter::MeleeAttack()
{
    if (!GetWorld() || !Controller) return;
    if (bIsProne)
    {
        NotifyFeedback(TEXT("UNABLE TO STRIKE WHILE PRONE"), TEXT("denied"));
        return;
    }
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now < MeleeNextTime) return;
    MeleeNextTime = Now + 0.35f;
    MeleeComboStep = (Now - LastMeleeTime > 1.0f) ? 1 : (MeleeComboStep % 3) + 1;
    LastMeleeTime = Now;
    MeleeVisualTimer = 0.22f;
    CombatNoiseTimer = 0.45f;
    const FVector Forward = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f).Vector();
    const FVector Origin = GetActorLocation();
    AMGGuardActor* Target = nullptr;
    float Best = 165.f;
    for (TActorIterator<AMGGuardActor> It(GetWorld()); It; ++It)
    {
        AMGGuardActor* Guard = *It;
        if (!IsValid(Guard) || Guard->IsIncapacitated()) continue;
        const FVector Diff = Guard->GetActorLocation() - Origin;
        const float Distance = Diff.Size2D();
        if (Distance >= Best || Distance < 20.f) continue;
        if (FVector::DotProduct(Forward.GetSafeNormal2D(), Diff.GetSafeNormal2D()) < 0.58f)
            continue;
        FHitResult Hit;
        FCollisionQueryParams Query(SCENE_QUERY_STAT(MGUnarmedLOS), false, this);
        const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit,
            Origin + FVector(0.f, 0.f, 44.f),
            Guard->GetActorLocation() + FVector(0.f, 0.f, 44.f),
            ECC_Visibility, Query);
        if (bBlocked && Hit.GetActor() != Guard) continue;
        Target = Guard;
        Best = Distance;
    }
    if (Target)
    {
        const float ComboDamage[3] = {35.f, 35.f, 40.f};
        UGameplayStatics::ApplyDamage(Target, ComboDamage[MeleeComboStep - 1],
            Controller, this, UDamageType::StaticClass());
        NotifyFeedback(Target->IsIncapacitated() ?
            TEXT("GUARD INCAPACITATED") : TEXT("MELEE IMPACT"), TEXT("impact"));
    }
    else NotifyFeedback(TEXT("UNARMED STRIKE — NO TARGET"), TEXT("whoosh"));
}
float AMGPlayerCharacter::TakeDamage(float Amount, const FDamageEvent& Event,
                                     AController* Instigator, AActor* Causer)
{
    const float Accepted = Super::TakeDamage(Amount, Event, Instigator, Causer);
    if (Amount <= 0.f || Health <= 0.f) return Accepted;
    Health = FMath::Max(0.f, Health - Amount);
    NotifyFeedback(Health <= 0.f ? TEXT("SNAKE DOWN — RELOADING CHECKPOINT")
                                  : TEXT("INJURED — TAKE COVER"), TEXT("hurt"));
    if (Health <= 0.f && GetWorld())
    {
        // A full failure flow will add death animation/AI reset; checkpoint
        // reload retains the versioned mission save.
        UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
    }
    return Accepted;
}


bool AMGPlayerCharacter::IsCombatAlertActive() const
{
    if (!GetWorld()) return false;
    for (TActorIterator<AMGGuardActor> It(GetWorld()); It; ++It)
        if (!It->IsIncapacitated() && It->GetAwarenessState() >= 3)
            return true;
    return false;
}

void AMGPlayerCharacter::CloseEquipmentWindows()
{
    if (!Equipment) return;
    if (Equipment->IsItemWindowOpen()) Equipment->CloseItemWindow();
    if (Equipment->IsWeaponWindowOpen()) Equipment->CloseWeaponWindow();
}

void AMGPlayerCharacter::ApplyMenuTimeState()
{
    if (!GetWorld()) return;
    const bool bSelecting = Equipment &&
        (Equipment->IsWeaponWindowOpen() || Equipment->IsItemWindowOpen());
    const float Desired = (!bCodecOpen && bSelecting) ?
        FMath::Clamp(EquipmentTimeScale, 0.10f, 1.0f) : 1.0f;
    // Global time dilation affects AI, movement and physics together.
    // Equipment menu rendering/input remains live. A real pause is separate.
    UGameplayStatics::SetGlobalTimeDilation(this, Desired);
}

void AMGPlayerCharacter::ToggleCodec()
{
    if (!GetWorld()) return;
    if (bCodecOpen)
    {
        bCodecOpen = false;
        CodecStatus.Empty();
        UGameplayStatics::SetGamePaused(this, false);
        ApplyMenuTimeState();
        NotifyFeedback(TEXT("CODEC CLOSED — INFILTRATION RESUMED"), TEXT("select"));
        return;
    }

    // The initial prison slice is an equipment-loss checkpoint. It has no
    // verified working 1995 transceiver until a later source-confirmed event.
    const AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
        UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass()));
    if (Mission && (Mission->GetMissionFlags() & 8u) == 0)
    {
        NotifyFeedback(TEXT("CODEC OFFLINE — TRANSMITTER CONFISCATED"), TEXT("denied"));
        return;
    }
    if (IsCombatAlertActive())
    {
        NotifyFeedback(TEXT("TRANSMISSION DELAYED — UNDER FIRE"), TEXT("denied"));
        return;
    }
    // For the provisional slice this is a read-only after-rescue intelligence
    // playback, NOT a fabricated new Gray Fox radio contact.
    CloseEquipmentWindows();
    ApplyMenuTimeState();
    CodecStatus = Mission ? TEXT("FOXHOUND INTELLIGENCE FILE — PLAYBACK")
                          : TEXT("RADIO CONTACTS — NONE VERIFIED");
    bCodecOpen = true;
    const bool bPaused = UGameplayStatics::SetGamePaused(this, true);
    if (!bPaused)
    {
        bCodecOpen = false;
        CodecStatus.Empty();
        NotifyFeedback(TEXT("CODEC PAUSE FAILED — TRY AGAIN"), TEXT("denied"));
    }
    else NotifyFeedback(TEXT("CODEC OPEN — WORLD PAUSED"), TEXT("menu"));
}

void AMGPlayerCharacter::CycleGraphicsQuality()
{
    if (!GEngine) return;
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (!Settings) return;
    QualityPreset = (QualityPreset + 1) % 4;
    static constexpr float Scales[] = {70.f, 85.f, 100.f, 100.f};
    Settings->SetOverallScalabilityLevel(QualityPreset);
    Settings->SetResolutionScaleValueEx(Scales[QualityPreset]);
    Settings->ApplySettings(false);
    Settings->SaveSettings();
    if (GEngine)
    {
        const TCHAR* Labels[] = {TEXT("Performance"), TEXT("Balanced"), TEXT("High"), TEXT("Ultra")};
        GEngine->AddOnScreenDebugMessage(91, 4.f, FColor::White, FString::Printf(TEXT("Graphics: %s"), Labels[QualityPreset]));
    }
}
