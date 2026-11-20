#include "MGShootGunnerActor.h"

#include "MGPlayerCharacter.h"
#include "MGEquipmentComponent.h"
#include "MGSpentCasingActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AMGShootGunnerActor::AMGShootGunnerActor()
{
    PrimaryActorTick.bCanEverTick = true;
    CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
    CollisionCapsule->InitCapsuleSize(39.f, 89.f);
    CollisionCapsule->SetCollisionProfileName(TEXT("Pawn"));
    CollisionCapsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    CollisionCapsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    RootComponent = CollisionCapsule;

    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShotmakerSkeletalBody"));
    CharacterMesh->SetupAttachment(RootComponent);
    CharacterMesh->SetRelativeLocation(FVector(0.f, 0.f, -89.f));
    CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    ProxyTorso = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PROXY_ActorShape"));
    ProxyTorso->SetupAttachment(RootComponent);
    ProxyTorso->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProxyTorso->SetRelativeScale3D(FVector(.62f, .74f, 1.5f));
    if (Cube.Succeeded()) ProxyTorso->SetStaticMesh(Cube.Object);

    // Explicitly shaped, replaceable source-only proxy parts, NOT a production
    // shotgun or accurate military reference. No copyrighted mesh imported.
    WeaponBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PROXY_RiotGunReceiver"));
    WeaponBody->SetupAttachment(RootComponent);
    WeaponBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBody->SetRelativeLocation(FVector(35.f, 25.f, 34.f));
    WeaponBody->SetRelativeScale3D(FVector(.8f, .12f, .15f));
    if (Cube.Succeeded()) WeaponBody->SetStaticMesh(Cube.Object);
    WeaponBarrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PROXY_RiotGunBarrel"));
    WeaponBarrel->SetupAttachment(RootComponent);
    WeaponBarrel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBarrel->SetRelativeLocation(FVector(83.f, 25.f, 34.f));
    WeaponBarrel->SetRelativeScale3D(FVector(.7f, .055f, .065f));
    if (Cube.Succeeded()) WeaponBarrel->SetStaticMesh(Cube.Object);
    WeaponPump = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShotmakerIndependentPump"));
    WeaponPump->SetupAttachment(WeaponBody);
    WeaponPump->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponPump->SetVisibility(false);

    // Physical muzzle-origin visual tell. Final VFX/materials need UE test.
    MuzzleFlash = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShotmakerMuzzleFlash"));
    MuzzleFlash->SetupAttachment(RootComponent);
    MuzzleFlash->SetRelativeLocation(FVector(117.f, 23.f, 34.f));
    MuzzleFlash->SetIntensity(9000.f);
    MuzzleFlash->SetAttenuationRadius(280.f);
    MuzzleFlash->SetLightColor(FLinearColor(1.f, .67f, .29f));
    MuzzleFlash->SetVisibility(false);
}

void AMGShootGunnerActor::BeginPlay()
{
    Super::BeginPlay();
    if (USkeletalMesh* Model = LoadObject<USkeletalMesh>(nullptr,
            TEXT("/Game/Characters/Bosses/Shotmaker/SK_Shotmaker_1995.SK_Shotmaker_1995")))
    {
        CharacterMesh->SetSkeletalMeshAsset(Model);
        if (UClass* Anim = LoadClass<UAnimInstance>(nullptr,
                TEXT("/Game/Characters/Bosses/Shotmaker/ABP_Shotmaker_1995.ABP_Shotmaker_1995_C")))
            CharacterMesh->SetAnimInstanceClass(Anim);
        ProxyTorso->SetVisibility(false);
    }
    // A validated final shotgun mesh will replace these primitive shapes. A
    // missing weapon should not silently cause an invisible physical attack.
    if (UStaticMesh* Gun = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/Weapons/Shotmaker/SM_RiotShotgun_1995.SM_RiotShotgun_1995")))
    {
        WeaponBody->SetStaticMesh(Gun);
        WeaponBody->SetRelativeScale3D(FVector::OneVector);
        WeaponBarrel->SetVisibility(false);
        bImportedWeaponGeometry = true;
        // The new source mesh deliberately keeps an independent moving fore-end.
        if (UStaticMesh* Pump = LoadObject<UStaticMesh>(nullptr,
                TEXT("/Game/Weapons/Shotmaker/SM_RiotShotgunPump_1995.SM_RiotShotgunPump_1995")))
        {
            WeaponPump->SetStaticMesh(Pump);
            WeaponPump->SetVisibility(true);
        }
        // When a proper weapon grip socket exists, parent weapon presentation
        // to the hand. Source-offset is provisional and must be fitted in UE.
        // With no authored rig the weapon continues following the actor, not
        // pretending a primitive body has articulated hands.
        if (CharacterMesh->GetSkeletalMeshAsset()
            && CharacterMesh->DoesSocketExist(TEXT("weapon_r")))
        {
            WeaponBody->AttachToComponent(CharacterMesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("weapon_r"));
            WeaponBody->SetRelativeLocation(FVector(-4.f, 0.f, 9.f));
        }
        MuzzleFlash->AttachToComponent(WeaponBody,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        MuzzleFlash->SetRelativeLocation(FVector(84.f, 0.f, 0.f));
    }
    // Authored original source audio files can be imported by
    // Content/Python/import_shotmaker_source_assets.py. These are optional
    // assets, not hidden copies of copyrighted Metal Gear recordings.
    ShotWarningCue = LoadObject<USoundBase>(nullptr,
        TEXT("/Game/Audio/Shotmaker/Shotmaker_AimCue.Shotmaker_AimCue"));
    ShotBlastCue = LoadObject<USoundBase>(nullptr,
        TEXT("/Game/Audio/Shotmaker/Shotmaker_Blast.Shotmaker_Blast"));
    ShotCycleCue = LoadObject<USoundBase>(nullptr,
        TEXT("/Game/Audio/Shotmaker/Shotmaker_Cycle.Shotmaker_Cycle"));
}

void AMGShootGunnerActor::SetEncounterActive(bool bEnabled)
{
    if (bDefeated) return;
    bActive = bEnabled;
    State = bEnabled ? EAttackState::Searching : EAttackState::Dormant;
    MuzzleFlashRemaining = 0.f;
    if (MuzzleFlash) MuzzleFlash->SetVisibility(false);
    StateClock = 0.f;
    ShotsFired = 0;
    LoadedShells = FMath::Clamp(TubeCapacity, 3, 8);
    ReloadShellClock = 0.f;
    PumpCueRemaining = -1.f;
    PendingPumpShotIndex = INDEX_NONE;
    PumpVisualRemainingSeconds = 0.f;
    if (WeaponPump) WeaponPump->SetRelativeLocation(FVector::ZeroVector);
}

void AMGShootGunnerActor::ReceiveTrackerPing(const FVector& ApproximateLocation)
{
    if (!bActive || bDefeated) return;
    // A compromised inventory provides a **radio beacon**, not wall-penetrating
    // bullets or a perfect real-time crosshair. Last known position decays.
    LastTrackerLocation = ApproximateLocation;
    TrackerPingAge = 0.f;
}

bool AMGShootGunnerActor::CanSeePlayer(AMGPlayerCharacter* Player) const
{
    if (!IsValid(Player) || !GetWorld()) return false;
    const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, 55.f);
    const FVector Target = Player->GetActorLocation() + FVector(0.f, 0.f, 40.f);
    if (FVector::Dist2D(Eye, Target) > 1250.f) return false;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MGShotmakerSight), true, this);
    FHitResult Hit;
    return GetWorld()->LineTraceSingleByChannel(
        Hit, Eye, Target, ECC_Visibility, Query) && Hit.GetActor() == Player;
}

void AMGShootGunnerActor::Tick(float Dt)
{
    Super::Tick(Dt);
    MuzzleFlashRemaining = FMath::Max(0.f, MuzzleFlashRemaining - FMath::Max(Dt, 0.f));
    if (MuzzleFlash) MuzzleFlash->SetVisibility(MuzzleFlashRemaining > 0.f);
    if (!bActive || bDefeated || !GetWorld()) return;
    AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    StateClock += FMath::Max(0.f, Dt);
    TrackerPingAge += FMath::Max(0.f, Dt);
    PumpVisualRemainingSeconds = FMath::Max(
        0.f, PumpVisualRemainingSeconds - FMath::Max(0.f, Dt));
    if (WeaponPump && WeaponPump->IsVisible())
    {
        // Actual transform changes only on the moving fore-end mesh. Native
        // health, shot rays, magazine counts and actor/hand grip never move.
        const float Phase = 1.f - PumpVisualRemainingSeconds / .24f;
        const float Pull = (Phase < .42f)
            ? FMath::Clamp(Phase / .42f, 0.f, 1.f)
            : FMath::Clamp((1.f - Phase) / .58f, 0.f, 1.f);
        WeaponPump->SetRelativeLocation(FVector(-9.f * Pull, 0.f, 0.f));
    }
    if (PendingPumpShotIndex != INDEX_NONE)
    {
        PumpCueRemaining -= FMath::Max(0.f, Dt);
        if (PumpCueRemaining <= 0.f)
        {
            const int32 PumpedShot = PendingPumpShotIndex;
            PendingPumpShotIndex = INDEX_NONE;
            PumpCueRemaining = -1.f;
            PumpVisualRemainingSeconds = .24f;
            OnPumpAction(PumpedShot);
            OnSpentShellEjected(PumpedShot);
            if (bImportedWeaponGeometry && IsValid(WeaponBody))
                AMGSpentCasingActor::Eject(GetWorld(), EMGCasingType::RiotGunHull,
                    WeaponBody->GetComponentTransform().TransformPosition(
                        FVector(9.f, 5.1f, 3.f)), WeaponBody->GetComponentRotation());
            if (ShotCycleCue)
                UGameplayStatics::PlaySoundAtLocation(this, ShotCycleCue,
                    GetActorLocation() + FVector(0.f, 0.f, 55.f), .64f);
        }
    }

    if (State == EAttackState::Searching)
    {
        // Limited guard of his own arena rather than GPS-tracking the player.
        // Avoid translating when aiming/firing; no instant 180-degree turn.
        if (StateClock > .45f)
        {
            const FVector Here = GetActorLocation();
            if (TrackerPingAge < 3.f && FMath::Abs(Here.Y - LastTrackerLocation.Y) > 55.f)
                PatrolDirection = FMath::Sign(LastTrackerLocation.Y - Here.Y);
            else
            {
                if (Here.Y > 235.f) PatrolDirection = -1.f;
                if (Here.Y < -205.f) PatrolDirection = 1.f;
            }
            SetActorLocation(Here + FVector(0.f, PatrolDirection * FMath::Clamp(PatrolSpeedCmPerSec, 30.f, 110.f) * Dt, 0.f), true);
        }
        if (CanSeePlayer(Player))
        {
            // Snapshot the target. He does not cheat by following Snake through
            // a wall or instantaneously realigning during the three-shot burst.
            LockedAimPoint = Player->GetActorLocation() + FVector(0.f, 0.f, 38.f);
            State = EAttackState::Warning;
            StateClock = 0.f;
            ShotsFired = 0;
            OnTelegraphStarted();
            if (ShotWarningCue)
                UGameplayStatics::PlaySoundAtLocation(this, ShotWarningCue,
                    GetActorLocation() + FVector(0.f, 0.f, 50.f), .68f);
        }
        return;
    }
    if (State == EAttackState::Warning)
    {
        const FRotator Desired = (LockedAimPoint - GetActorLocation()).Rotation();
        const FRotator Rot = FMath::RInterpTo(GetActorRotation(),
            FRotator(0.f, Desired.Yaw, 0.f), Dt, 3.f);
        SetActorRotation(Rot);
        // Aim hold is visible. If cover is reached before the volley, the
        // locked aim remains at the observed position, rewarding player agency.
        // No sideways/backwards muzzle fire: the body and gun must actually
        // finish turning before their projectile traces become authoritative.
        const FVector Facing = GetActorForwardVector().GetSafeNormal2D();
        const FVector DesiredFacing = (LockedAimPoint - GetActorLocation()).GetSafeNormal2D();
        const bool bAligned = FVector::DotProduct(Facing, DesiredFacing) >= 0.985f;
        if (StateClock >= FMath::Clamp(WindupSeconds, .5f, 3.f) && bAligned)
        {
            State = EAttackState::Volley;
            StateClock = 0.f;
            ShotsFired = 0;
            FireShot(Player);
        }
        return;
    }
    if (State == EAttackState::Volley)
    {
        // The MSX2 attack has THREE large spread shots. Deliberate individual
        // spacing and longer burst recovery are 3D tuning hypotheses.
        if (StateClock >= FMath::Clamp(IntervalSeconds, .25f, 1.5f))
        {
            StateClock = 0.f;
            FireShot(Player);
        }
        return;
    }
    if (State == EAttackState::Recovery && StateClock >= FMath::Clamp(RecoverySeconds, .8f, 5.f))
    {
        // Three visible blasts are the source rule, not proof that the
        // firearm has a historically three-round tube. Reload ONLY if the
        // provisional actual magazine is empty. No magically reset counter.
        if (LoadedShells <= 0)
        {
            State = EAttackState::Reloading;
            ReloadShellClock = 0.f;
            OnReloadStarted();
        }
        else State = EAttackState::Searching;
        StateClock = 0.f;
    }
    if (State == EAttackState::Reloading)
    {
        // A real reload is a sequence of individual shell-in-hand inserts,
        // not instant ammunition restoration. Combat and pursuit resume only
        // after the actual final insert. Event callbacks are visual-only.
        ReloadShellClock += FMath::Max(0.f, Dt);
        const float SecondsPerInsert = FMath::Clamp(SecondsPerShellInsert, .25f, 1.5f);
        if (ReloadShellClock >= SecondsPerInsert)
        {
            ReloadShellClock -= SecondsPerInsert;
            LoadedShells = FMath::Min(LoadedShells + 1, FMath::Clamp(TubeCapacity, 3, 8));
            OnReloadShellInserted(LoadedShells);
            if (LoadedShells >= FMath::Clamp(TubeCapacity, 3, 8))
            {
                State = EAttackState::Searching;
                StateClock = 0.f;
                ReloadShellClock = 0.f;
                OnReloadFinished();
            }
        }
    }
}

FVector AMGShootGunnerActor::GetActualMuzzleLocation() const
{
    // Keep the authoritative projectile origin on the physical imported
    // weapon, even if its hand moves during a real animation.
    return bImportedWeaponGeometry && IsValid(WeaponBody)
        ? WeaponBody->GetComponentTransform().TransformPosition(FVector(84.f, 0.f, 0.f))
        : GetActorLocation() + GetActorRotation().RotateVector(FVector(112.f, 23.f, 32.f));
}

void AMGShootGunnerActor::FireShot(AMGPlayerCharacter* Player)
{
    if (State != EAttackState::Volley || ShotsFired >= 3 ||
        LoadedShells <= 0 || !GetWorld())
        return;
    // Authored 3D spread, with a bounded angular cone. Original MSX2
    // documents three broad blasts, NOT a canonical pellet count or timing.
    // Never use camera-space traces through a different physical obstruction.
    const FVector Muzzle = GetActualMuzzleLocation();
    MuzzleFlashRemaining = .085f;
    if (MuzzleFlash) MuzzleFlash->SetVisibility(true);
    if (ShotBlastCue)
        UGameplayStatics::PlaySoundAtLocation(this, ShotBlastCue, Muzzle, .70f);
    const FVector ToAim = (LockedAimPoint - Muzzle).GetSafeNormal();
    const FRotator Base = ToAim.Rotation();
    const int32 Count = FMath::Clamp(PelletCount, 3, 9);
    const float Spread = FMath::Clamp(HalfSpreadDegrees, 3.f, 12.f);
    int32 Hits = 0;
    for (int32 Pellet = 0; Pellet < Count; ++Pellet)
    {
        const float Yaw = -Spread + (2.f * Spread * Pellet / (Count - 1))
            + (ShotsFired == 1 ? 1.2f : (ShotsFired == 2 ? -1.2f : 0.f));
        const FVector Dir = FRotator(Base.Pitch, Base.Yaw + Yaw, 0.f).Vector();
        FCollisionQueryParams Query(SCENE_QUERY_STAT(MGShotmakerPellets), true, this);
        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, Muzzle,
                Muzzle + Dir * 1250.f, ECC_Visibility, Query)
            && Hit.GetActor() == Player)
            ++Hits;
    }
    if (Hits > 0 && IsValid(Player))
    {
        // One bounded damage transaction for a burst, not five independent
        // death callbacks or damage through corners from a camera trace.
        UGameplayStatics::ApplyDamage(Player,
            FMath::Min(Hits * FMath::Clamp(PelletDamage, 4.f, 16.f),
                       FMath::Clamp(MaxShotDamage, 12.f, 50.f)),
            nullptr, this, UDamageType::StaticClass());
    }
    const int32 CompletedShotIndex = ShotsFired;
    --LoadedShells; // One actual chambered round leaves the shotgun.
    OnShotFired(CompletedShotIndex);
    PendingPumpShotIndex = CompletedShotIndex;
    PumpCueRemaining = FMath::Min(
        FMath::Clamp(PumpOffsetSeconds, .05f, .40f),
        FMath::Clamp(IntervalSeconds, .25f, 1.5f) * .75f);
    ++ShotsFired;
    if (ShotsFired >= 3 || LoadedShells <= 0)
    {
        ++VolleysCompleted;
        State = EAttackState::Recovery;
        StateClock = 0.f;
    }
}

float AMGShootGunnerActor::TakeDamage(float Amount, const FDamageEvent& DamageEvent,
                                   AController* EventInstigator, AActor* Causer)
{
    if (!bActive || bDefeated || Amount <= 0.f) return 0.f;
    // A stun-only TEST prop cannot erase the original puzzle by defeating
    // Shotmaker with one non-lethal hit. Reclaimed conventional weapons work.
    if (const AMGPlayerCharacter* Snake = Cast<AMGPlayerCharacter>(Causer))
        if (Snake->GetEquipment() && Snake->GetEquipment()->IsStunEquipped())
            return 0.f;
    const float Accepted = Super::TakeDamage(Amount, DamageEvent, EventInstigator, Causer);
    Health = FMath::Max(0.f, Health - Amount);
    OnBossWounded(GetHealthFraction());
    if (Health <= 0.f) EnterDefeatedState();
    return FMath::Max(Amount, Accepted);
}

void AMGShootGunnerActor::EnterDefeatedState()
{
    if (bDefeated) return;
    bDefeated = true;
    bActive = false;
    PendingPumpShotIndex = INDEX_NONE;
    PumpCueRemaining = -1.f;
    ReloadShellClock = 0.f;
    PumpVisualRemainingSeconds = 0.f;
    if (WeaponPump) WeaponPump->SetRelativeLocation(FVector::ZeroVector);
    MuzzleFlashRemaining = 0.f;
    if (MuzzleFlash) MuzzleFlash->SetVisibility(false);
    State = EAttackState::Defeated;
    CollisionCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    // Production must use an authored physical death performance/ragdoll.
    // Keep source proxy visibly down, not instantly destroyed.
    ProxyTorso->SetRelativeRotation(FRotator(0.f, 0.f, 72.f));
    OnBossDefeated();
}

void AMGShootGunnerActor::RestoreDefeated()
{
    Health = 0.f;
    EnterDefeatedState();
}
