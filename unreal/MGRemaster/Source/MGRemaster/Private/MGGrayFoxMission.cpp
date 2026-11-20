#include "MGGrayFoxMission.h"

#include "MGGuardActor.h"
#include "MGStoryCameraDirector.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "MGShootGunnerActor.h"
#include "MGSentryDogActor.h"
#include "MGMachinegunKid.h"
#include "MGSurveillanceCameraActor.h"
#include "Components/CapsuleComponent.h"
#include "MGGrayFoxSaveGame.h"
#include "MGPlayerCharacter.h"
#include "MGEquipmentComponent.h"
#include "MGPrisonerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "mg_legacy_core.h"

namespace
{
    // Wall and floor positions are illustrative 3D blockout, not original 2D coordinates.
    const FVector CellStart(-950.f, 0.f, -11.f);
    const FVector PreludeStart(-3520.f, -50.f, -11.f);
    const FVector ExitPoint(1150.f, 0.f, -11.f);
    const FVector BossThreshold(1570.f, 0.f, -11.f);
    const FString SaveSlot = TEXT("GrayFoxSliceV1");

    uint32 FallbackStep(uint32 Flags, uint32 Action)
    {
        // Mirror the Rust transaction, fail closed on skipped or repeated steps.
        // 1987 prisoner intelligence, boss, transmitter and Card 3 additions.
        // Old six-bit checkpoints remain valid. Unknown bits fail closed.
        const uint32 Known = 16383u;
        if (Flags & ~Known) return Flags;
        if (Action == MG_FOX_CAPTURE && !(Flags & MG_FOX_CAPTURED))
            return Flags | MG_FOX_CAPTURED | MG_FOX_CONFISCATED;
        if (Action == MG_FOX_BREACH_FIRST_WALL
            && (Flags & MG_FOX_CAPTURED) && !(Flags & MG_FOX_FIRST_WALL_BREACHED))
            return Flags | MG_FOX_FIRST_WALL_BREACHED;
        if (Action == MG_FOX_RESCUE
            && (Flags & MG_FOX_FIRST_WALL_BREACHED) && !(Flags & MG_FOX_RESCUED))
            return Flags | MG_FOX_RESCUED;
        if (Action == MG_FOX_BREACH_EXIT_WALL
            && (Flags & MG_FOX_RESCUED) && !(Flags & MG_FOX_EXIT_WALL_BREACHED))
            return Flags | MG_FOX_EXIT_WALL_BREACHED;
        if (Action == MG_FOX_ESCAPE
            && (Flags & MG_FOX_EXIT_WALL_BREACHED) && !(Flags & MG_FOX_LEFT_PRISON))
            return Flags | MG_FOX_LEFT_PRISON;
        if (Action == MG_FOX_ENTER_SHOTMAKER
            && (Flags & MG_FOX_LEFT_PRISON) && !(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED))
            return Flags | MG_FOX_SHOTMAKER_ENCOUNTERED;
        if (Action == MG_FOX_RECOVER_GEAR
            && (Flags & MG_FOX_SHOTMAKER_ENCOUNTERED) && !(Flags & MG_FOX_GEAR_RECOVERED))
            return Flags | MG_FOX_GEAR_RECOVERED | MG_FOX_TRANSMITTER_PLANTED;
        if (Action == MG_FOX_DEFEAT_SHOTMAKER
            && (Flags & MG_FOX_GEAR_RECOVERED) && !(Flags & MG_FOX_SHOTMAKER_DEFEATED))
            return Flags | MG_FOX_SHOTMAKER_DEFEATED;
        if (Action == MG_FOX_COLLECT_CARD3
            && (Flags & MG_FOX_SHOTMAKER_DEFEATED) && !(Flags & MG_FOX_CARD3_RECOVERED))
            return Flags | MG_FOX_CARD3_RECOVERED;
        if (Action == MG_FOX_REMOVE_TRANSMITTER
            && (Flags & MG_FOX_TRANSMITTER_PLANTED) && !(Flags & MG_FOX_TRANSMITTER_REMOVED))
            return Flags | MG_FOX_TRANSMITTER_REMOVED;
        if (Action == MG_FOX_COMPLETE_SLICE
            && (Flags & MG_FOX_CARD3_RECOVERED) && !(Flags & MG_FOX_SLICE_COMPLETED))
            return Flags | MG_FOX_SLICE_COMPLETED;
        // Distinct from capture, for the expanded playable pre-prison route.
        if (Action == MG_FOX_LEARN_DETENTION_ROUTE
            && !(Flags & MG_FOX_CAPTURED) && !(Flags & MG_FOX_DETENTION_INTELLIGENCE))
            return Flags | MG_FOX_DETENTION_INTELLIGENCE;
        return Flags;
    }
}

AMGGrayFoxMission::AMGGrayFoxMission()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) BlockMesh = Cube.Object;
}

AStaticMeshActor* AMGGrayFoxMission::SpawnBlock(
    FName Tag, const FVector& Centre, const FVector& Scale, bool bBlockMovement)
{
    if (!GetWorld() || !BlockMesh) return nullptr;
    AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(Centre, FRotator::ZeroRotator);
    if (!Actor) return nullptr;
    Actor->Tags.Add(Tag);
    // Breakable mission geometry must remain an interaction, never automatic cover.
    if (Tag.ToString().StartsWith(TEXT("WeakWall_")) || Tag == TEXT("GearDoor"))
        Actor->Tags.Add(TEXT("MG_Breachable"));
    UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(BlockMesh);
    // Optional acoustics metadata on greybox walls; independently replaceable by artists.
    if (Tag.ToString().Contains(TEXT("Boundary")) || Tag == FName(TEXT("Divider")))
        Mesh->ComponentTags.Add(TEXT("MG_Surface_Metal"));
    else if (Tag == FName(TEXT("Bed")))
        Mesh->ComponentTags.Add(TEXT("MG_Surface_Wood"));
    Mesh->SetWorldScale3D(Scale);
    Mesh->SetCollisionProfileName(bBlockMovement ? TEXT("BlockAll") : TEXT("NoCollision"));
    return Actor;
}

void AMGGrayFoxMission::AddWorldTrace(
    FName Tag, const FVector& Place, const FVector& Size,
    const FString& Observation, uint32 RequiredFlags, uint32 ForbiddenFlags)
{
    // This evidence is read by player choice; it is not a mandatory
    // lore terminal, hidden mission flag or source-verified 1987 text.
    AStaticMeshActor* Prop = SpawnBlock(Tag, Place, Size, false);
    if (!IsValid(Prop)) return;
    FOptionalWorldTrace& Trace = OptionalWorldTraces.AddDefaulted_GetRef();
    Trace.Prop = Prop;
    Trace.Observation = Observation;
    Trace.RequiredFlags = RequiredFlags;
    Trace.ForbiddenFlags = ForbiddenFlags;
}

bool AMGGrayFoxMission::TryInspectWorldTrace(AMGPlayerCharacter* Player)
{
    if (!IsValid(Player)) return false;
    for (const FOptionalWorldTrace& Trace : OptionalWorldTraces)
    {
        if ((Flags & Trace.RequiredFlags) != Trace.RequiredFlags ||
            (Flags & Trace.ForbiddenFlags) != 0u ||
            !CanReach(Player, Trace.Prop.Get(), 185.f)) continue;
        Dialogue = Trace.Observation;
        DialogueTimeRemaining = 4.8f;
        // No SaveProgress or ApplyAction: these traces cannot award a key,
        // trigger capture, reveal the phantom or summon omniscient enemies.
        return true;
    }
    return false;
}

void AMGGrayFoxMission::CreatePrelude()
{
    // Spatially isolated from the cell by the west prison wall: the actual
    // change of space happens upon custody/transport, not a magic corridor.
    // Source: full MSX2 opening includes prior infiltration + POW clue;
    // exact J2ME architecture is unverified. This is a deliberate slice lead-in.
    SpawnBlock(TEXT("PreludeFloor"), FVector(-2600.f, 0.f, -150.f),
        FVector(26.f, 10.f, 1.f), true);
    SpawnBlock(TEXT("PreludeWestBoundary"), FVector(-3900.f, 0.f, 90.f),
        FVector(.4f, 10.f, 3.8f), true);
    SpawnBlock(TEXT("PreludeNorthBoundary"), FVector(-2600.f, 500.f, 90.f),
        FVector(26.f, .4f, 3.8f), true);
    SpawnBlock(TEXT("PreludeSouthBoundary"), FVector(-2600.f, -500.f, 90.f),
        FVector(26.f, .4f, 3.8f), true);

    // Alternate northern/southern paths reward scouting rather than a single
    // linear sprint to the objective. The centre route is quicker but exposed.
    SpawnBlock(TEXT("PreludeUtility"), FVector(-3100.f, 60.f, -5.f),
        FVector(2.9f, 1.3f, 2.1f), true);
    SpawnBlock(TEXT("PreludeCargoA"), FVector(-2880.f, -250.f, -20.f),
        FVector(1.8f, 1.5f, 1.5f), true);
    SpawnBlock(TEXT("PreludeCargoB"), FVector(-2340.f, 30.f, -15.f),
        FVector(2.4f, .85f, 1.7f), true);
    AddWorldTrace(TEXT("CargoTransferManifest"),
        FVector(-2870.f, -168.f, -5.f), FVector(.45f, .06f, .50f),
        TEXT("CARGO: FILTERS / DRESSINGS / RATIONS. SIGNATURE MISSING."),
        0u, MG_FOX_CAPTURED);
    // Source-ranked MSX2 handgun acquisition, with NEW 3D room placement.
    // Earned by proximity/use, NOT a default loadout or magical reward.
    EarlyHandgunPickup = SpawnBlock(TEXT("Building1EarlyHandgunCache"),
        FVector(-3670.f, 180.f, -56.f), FVector(.68f, .50f, .45f), false);
    SpawnBlock(TEXT("Building1HandgunStorageShelf"),
        FVector(-3670.f, 230.f, -62.f), FVector(1.45f, .42f, .70f), true);
    SpawnBlock(TEXT("PreludeSecurityBooth"), FVector(-1920.f, 225.f, -15.f),
        FVector(2.6f, 1.8f, 2.1f), true);
    AddWorldTrace(TEXT("DutyRotationSlate"),
        FVector(-2056.f, 260.f, 15.f), FVector(.06f, .50f, .50f),
        TEXT("DUTY SLATE: TRANSPORT 02. SHIFT OVERDUE. NO REASON GIVEN."),
        0u, MG_FOX_CAPTURED);
    // The POW cannot be accidentally mistaken for Fox; bespoke POW human art
    // is a separate import target; this is a non-colliding readable proxy.
    POWInformant = SpawnBlock(TEXT("InformantPOW"),
        FVector(-2590.f, 330.f, -33.f), FVector(.65f, .45f, 1.45f), false);

    // Detention transport: its silhouette, not a quest teleport button, is the
    // landmark. Two separated components for body/cab; artist substitutes later.
    SpawnBlock(TEXT("CustodyTruckChassis"), FVector(-1590.f, -310.f, -28.f),
        FVector(3.9f, 1.7f, 1.25f), true);
    SpawnBlock(TEXT("CustodyTruckCab"), FVector(-1360.f, -310.f, 35.f),
        FVector(1.65f, 1.7f, 2.15f), true);
    CustodyMarker = SpawnBlock(TEXT("CustodyPoint"),
        FVector(-1590.f, -15.f, -25.f), FVector(.25f, .3f, .55f), false);

    // A quiet outer perimeter is not the prison's guarded corridor. Patrols
    // are hand-positioned; hostile sight and hearing obey guard perception.
    if (GetWorld())
    {
        if (AMGGuardActor* PatrolA = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(-3060.f, 280.f, -11.f), FRotator(0.f, -90.f, 0.f)))
        {
            PatrolA->SetRole(EMGGuardRole::Patrol);
            PatrolA->SetBodyPersistenceId(TEXT("P01_YardGuardA"));
            PatrolA->ConfigurePatrol(FVector(0.f, 1.f, 0.f), 130.f, 82.f);
        }
        if (AMGGuardActor* PatrolB = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(-2100.f, -285.f, -11.f), FRotator(0.f, 135.f, 0.f)))
        {
            PatrolB->SetRole(EMGGuardRole::Patrol);
            PatrolB->SetBodyPersistenceId(TEXT("P01_YardGuardB"));
            PatrolB->ConfigurePatrol(FVector(1.f, 0.f, 0.f), 145.f, 92.f);
        }
        // Source-informed duty: the receiving station has a posted officer
        // watching cargo transfers, not a free-roaming NPC whose duty makes
        // no sense. Stationary sector-scanning is a GAME abstraction, not
        // direct access to current military facility operations.
        if (AMGGuardActor* Receiver = GetWorld()->SpawnActor<AMGGuardActor>(
                FVector(-2410.f, 295.f, -11.f), FRotator(0.f, -95.f, 0.f)))
        {
            Receiver->SetRole(EMGGuardRole::LogisticsPost);
            Receiver->SetBodyPersistenceId(TEXT("P01_ReceivingOfficer"));
        }
        CustodyOfficerA = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(-1570.f, 315.f, -11.f), FRotator(0.f, -90.f, 0.f));
        CustodyOfficerB = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(-1470.f, 170.f, -11.f), FRotator(0.f, -90.f, 0.f));
        if (IsValid(CustodyOfficerA))
            { CustodyOfficerA->SetRole(EMGGuardRole::Custody);
              CustodyOfficerA->SetBodyPersistenceId(TEXT("P01_CustodyA")); }
        if (IsValid(CustodyOfficerB))
            { CustodyOfficerB->SetRole(EMGGuardRole::Custody);
              CustodyOfficerB->SetBodyPersistenceId(TEXT("P01_CustodyB")); }
    }
}

namespace
{
    constexpr float DispatchBeltStart = -3350.f;
    constexpr float DispatchBeltEnd = -2250.f;
    constexpr float DispatchLaneY = -370.f;
    constexpr float DispatchSensorX = -2700.f;
}

bool AMGGrayFoxMission::IsWorkingFreightConveyor(const FVector& P) const
{
    return IsPreludeActive() && P.X >= DispatchBeltStart && P.X <= DispatchBeltEnd
        && FMath::Abs(P.Y - DispatchLaneY) <= 55.f;
}

bool AMGGrayFoxMission::IsExpectedFreightLocation(const FVector& P) const
{
    if (!IsPreludeActive()) return false;
    // A crate may be either on the WORKING conveyor or legitimately staged
    // beside authored stack geometry. Movement is only normal on the belt.
    const bool bInStoredCargo =
        FVector::Dist2D(P, FVector(-2880.f, -250.f, P.Z)) <= 135.f ||
        FVector::Dist2D(P, FVector(-2340.f, 30.f, P.Z)) <= 155.f;
    return IsWorkingFreightConveyor(P) || bInStoredCargo;
}

void AMGGrayFoxMission::CreateDispatchBay()
{
    CartonPickup = SpawnBlock(TEXT("DispatchCarton"), FVector(-3410.f, -268.f, -55.f),
        FVector(1.14f, .95f, .90f), false);
    WornCarton = SpawnBlock(TEXT("CarriedCarton"), FVector::ZeroVector, FVector(1.2f, 1.05f, 1.18f), false);
    for (AStaticMeshActor* Box : {CartonPickup, WornCarton})
        if (IsValid(Box))
            if (UMaterialInstanceDynamic* Paper = Box->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
                Paper->SetVectorParameterValue(TEXT("Color"),FLinearColor(.48f,.31f,.16f));
    SmokePuff = SpawnBlock(TEXT("ExhaledSmoke"), FVector::ZeroVector, FVector(.12f), false);
    if (IsValid(SmokePuff))
    {
        if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
            SmokePuff->GetStaticMeshComponent()->SetStaticMesh(Sphere);
        SmokePuff->SetActorHiddenInGame(true);
    }
    BeamSegment = SpawnBlock(TEXT("IlluminatedSmoke"), FVector::ZeroVector, FVector(.025f,.65f,.025f), false);
    if (IsValid(BeamSegment)) BeamSegment->SetActorHiddenInGame(true);
    if (IsValid(WornCarton)) WornCarton->SetActorHiddenInGame(true);
    SpawnBlock(TEXT("FreightBelt"), FVector((DispatchBeltStart + DispatchBeltEnd) * .5f, DispatchLaneY, -97.f),
        FVector((DispatchBeltEnd - DispatchBeltStart)/100.f, 1.05f, .06f), false);
    for (float X = DispatchBeltStart; X <= DispatchBeltEnd; X += 75.f)
        SpawnBlock(TEXT("BeltCleat"), FVector(X, DispatchLaneY, -93.f), FVector(.04f, 1.05f, .03f), false);
    for (const float Y : {-475.f, -175.f})
        SpawnBlock(TEXT("OpticalGate"), FVector(DispatchSensorX, Y, 34.f), FVector(.15f,.16f,.22f), false);
    DispatchCamera = GetWorld()->SpawnActor<AMGSurveillanceCameraActor>(
        FVector(-2120.f, -435.f, 180.f), FRotator(0.f, 148.f, 0.f));
    if (DispatchCamera.IsValid()) DispatchCamera->ConfigureSector(38.f, 700.f);
}

void AMGGrayFoxMission::ExhaleSmoke(const AMGPlayerCharacter* Player)
{
    if (!IsValid(Player) || !Player->GetEquipment() ||
        !Player->GetEquipment()->IsItemEquipped(TEXT("CIGARETTES"))) return;
    SmokeOrigin = Player->GetActorLocation() + Player->GetActorForwardVector()*28.f;
    SmokeOrigin.Z += Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*.62f;
    SmokeAge = 0.f;
}

void AMGGrayFoxMission::TickDispatch(AMGPlayerCharacter* Player, float Dt)
{
    const FVector P = Player->GetActorLocation();
    const bool bBox = Player->IsWearingCarton();
    const bool bRidingCarton = IsPreludeActive() && bBox
        && P.X >= DispatchBeltStart && P.X < DispatchBeltEnd
        && FMath::Abs(P.Y - DispatchLaneY) < 52.f;
    if (bRidingCarton)
    {
        Player->AddActorWorldOffset(
            FVector(FMath::Min(165.f*Dt, DispatchBeltEnd - P.X), 0.f, 0.f), true);
        // Remake-authored incidental life: quiet, optional dialogue while
        // a REAL moving freight belt carries Snake forward. Zero loss of
        // player input, no new faction knowledge, enemies or progression gate.
        if (!bHeardFreightClerk && P.X >= -2570.f)
        {
            bHeardFreightClerk = true;
            Dialogue = TEXT("RECEIVING CLERK (OFF): Another load for the duty desk. Check the seals and keep the line moving.");
            DialogueTimeRemaining = 4.2f;
            SaveProgress(Player);
        }
    }
    if (IsValid(WornCarton))
    {
        WornCarton->SetActorHiddenInGame(!bBox);
        if (bBox) WornCarton->SetActorLocationAndRotation(
            Player->GetActorLocation() + FVector(0.f,0.f,59.f-Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
            Player->GetActorRotation());
    }
    if (IsPreludeActive() && bHasObservedPlayer && (LastObservedPlayerLocation.X < DispatchSensorX) != (P.X < DispatchSensorX)
        && P.Y >= -475.f && P.Y <= -175.f && !Player->bIsCrouched && !Player->IsProne())
    {
        NotifyGuardContactReported(FVector(DispatchSensorX, P.Y, P.Z));
        Player->NotifyFeedback(TEXT("DISPATCH GATE — ALARM"), TEXT("alert"));
    }
    SmokeAge = FMath::Min(3.f, SmokeAge + Dt);
    const FVector Puff = SmokeOrigin + FVector(16.f*SmokeAge,0.f,12.f*SmokeAge);
    if (IsValid(SmokePuff))
    {
        SmokePuff->SetActorHiddenInGame(SmokeAge >= 3.f);
        SmokePuff->SetActorLocation(Puff);
        SmokePuff->SetActorScale3D(FVector(.12f + .12f*SmokeAge)*(1.f-SmokeAge/3.f));
    }
    if (IsValid(BeamSegment))
    {
        const bool bReveal = SmokeAge < 3.f && FMath::Abs(Puff.X-DispatchSensorX) < 95.f
            && Puff.Y > -475.f && Puff.Y < -175.f && FMath::Abs(Puff.Z-34.f)<65.f;
        BeamSegment->SetActorHiddenInGame(!bReveal);
        BeamSegment->SetActorLocation(FVector(DispatchSensorX,FMath::Clamp(Puff.Y,-442.f,-208.f),34.f));
    }
}

void AMGGrayFoxMission::CreateGreybox()
{
    // Unreal's basic cube is 100cm at scale 1. Floor top is -100cm.
    // Three functional areas: prison cell, Fox cell, guarded escape corridor.
    SpawnBlock(TEXT("Floor"), FVector(75.f, 0.f, -150.f), FVector(26.5f, 10.f, 1.f), true);
    SpawnBlock(TEXT("NorthBoundary"), FVector(75.f, 500.f, 90.f), FVector(26.5f, 0.4f, 3.8f), true);
    SpawnBlock(TEXT("SouthBoundary"), FVector(75.f, -500.f, 90.f), FVector(26.5f, 0.4f, 3.8f), true);
    SpawnBlock(TEXT("WestBoundary"), FVector(-1250.f, 0.f, 90.f), FVector(0.4f, 10.f, 3.8f), true);
    // Connected opening: 400cm wide. Never teleport to the boss.
    SpawnBlock(TEXT("EastBoundaryNorth"), FVector(1400.f, 375.f, 90.f), FVector(.4f, 2.5f, 3.8f), true);
    SpawnBlock(TEXT("EastBoundarySouth"), FVector(1400.f, -375.f, 90.f), FVector(.4f, 2.5f, 3.8f), true);

    for (const float X : {-300.f, 470.f})
    {
        SpawnBlock(TEXT("Divider"), FVector(X, -322.5f, 90.f), FVector(0.4f, 3.55f, 3.8f), true);
        SpawnBlock(TEXT("Divider"), FVector(X, 322.5f, 90.f), FVector(0.4f, 3.55f, 3.8f), true);
    }
    WeakWall = SpawnBlock(TEXT("WeakWall_A"), FVector(-300.f, 0.f, 90.f),
                          FVector(0.4f, 2.9f, 3.8f), true);
    EscapeWall = SpawnBlock(TEXT("WeakWall_B"), FVector(470.f, 0.f, 90.f),
                            FVector(0.4f, 2.9f, 3.8f), true);
    // Restrained 3D blockout: not a claim about exact 1995 room measurements.
    FoxProxy = GetWorld()->SpawnActor<AMGPrisonerCharacter>(
        FVector(130.f, 0.f, -11.f), FRotator::ZeroRotator);
    // The actor contains its own visible fallback and optional imported rig.
    SpawnBlock(TEXT("Bed"), FVector(-900.f, 240.f, -62.f),
               FVector(1.8f, 0.9f, 0.75f), true);
    AddWorldTrace(TEXT("CellRepairReceipt"),
        FVector(-865.f, -474.f, 25.f), FVector(.44f, .06f, .50f),
        TEXT("REPAIR SLIP: LOWER WALL PATCHED. INSPECTION DEFERRED."),
        MG_FOX_CAPTURED, 0u);
    SpawnBlock(TEXT("Bed"), FVector(110.f, 240.f, -62.f),
               FVector(1.8f, 0.9f, 0.75f), true);
    SpawnBlock(TEXT("GuardCover"), FVector(950.f, 215.f, -25.f),
               FVector(1.8f, 0.7f, 1.5f), true);
    SpawnBlock(TEXT("GuardCover"), FVector(1200.f, -200.f, -25.f),
               FVector(1.8f, 0.7f, 1.5f), true);
    ExitMarker = SpawnBlock(TEXT("EscapeMarker"), ExitPoint, FVector(.5f,.5f,1.2f), false);
    // Cracks are simple light-catching surface slivers. They are pre-authored
    // at the weak seams but reveal more damage on each actual punch.
    FirstWallCrack = SpawnBlock(TEXT("FirstWallDamage"),
        FVector(-323.f, 0.f, 90.f), FVector(.05f, .10f, 1.6f), false);
    SecondWallCrack = SpawnBlock(TEXT("SecondWallDamage"),
        FVector(447.f, 0.f, 90.f), FVector(.05f, .10f, 1.6f), false);
    if (IsValid(FirstWallCrack)) FirstWallCrack->SetActorHiddenInGame(true);
    if (IsValid(SecondWallCrack)) SecondWallCrack->SetActorHiddenInGame(true);
}

void AMGGrayFoxMission::CreateBossWing()
{
    // Provisional geometry, NOT J2ME original room dimensions. A connected
    // boss arena plus a physical recovery side room and Card-2 storeroom.
    SpawnBlock(TEXT("BossFloor"), FVector(2550.f, 0.f, -150.f), FVector(24.f, 10.f, 1.f), true);
    // Physically connect Card 3's eastern passage to the next 1987 beat.
    // An interactive gate at x=3610 still blocks passage until the slice's
    // already-authoritative chapter completion transaction commits.
    SpawnBlock(TEXT("BossEastBoundaryNorth"), FVector(3750.f, 365.f, 90.f), FVector(.4f, 2.7f, 3.8f), true);
    SpawnBlock(TEXT("BossEastBoundarySouth"), FVector(3750.f, -365.f, 90.f), FVector(.4f, 2.7f, 3.8f), true);
    SpawnBlock(TEXT("BossNorthWallWest"), FVector(1650.f, 500.f, 90.f), FVector(5.f, .4f, 3.8f), true);
    SpawnBlock(TEXT("BossNorthWallEast"), FVector(3025.f, 500.f, 90.f), FVector(14.5f, .4f, 3.8f), true);
    Card2Door = SpawnBlock(TEXT("Card2Door"), FVector(2100.f, 500.f, 90.f), FVector(4.f, .4f, 3.8f), true);
    SpawnBlock(TEXT("CardRoomFloor"), FVector(2100.f, 780.f, -150.f), FVector(9.f, 5.6f, 1.f), true);
    SpawnBlock(TEXT("CardRoomWest"), FVector(1640.f, 780.f, 90.f), FVector(.4f, 5.6f, 3.8f), true);
    SpawnBlock(TEXT("CardRoomEast"), FVector(2560.f, 780.f, 90.f), FVector(.4f, 5.6f, 3.8f), true);
    SpawnBlock(TEXT("CardRoomNorth"), FVector(2100.f, 1080.f, 90.f), FVector(9.2f, .4f, 3.8f), true);
    Card3Pickup = SpawnBlock(TEXT("Card3Pickup"), FVector(2100.f, 845.f, -45.f), FVector(.35f, .14f, .9f), false);

    SpawnBlock(TEXT("BossSouthWallWest"), FVector(2015.f, -500.f, 90.f), FVector(12.3f, .4f, 3.8f), true);
    SpawnBlock(TEXT("BossSouthWallEast"), FVector(3335.f, -500.f, 90.f), FVector(8.3f, .4f, 3.8f), true);
    GearDoor = SpawnBlock(TEXT("GearDoor"), FVector(2775.f, -500.f, 90.f), FVector(2.9f, .4f, 3.8f), true);
    SpawnBlock(TEXT("GearRoomFloor"), FVector(2775.f, -785.f, -150.f), FVector(10.2f, 5.8f, 1.f), true);
    SpawnBlock(TEXT("GearRoomWest"), FVector(2265.f, -785.f, 90.f), FVector(.4f, 5.8f, 3.8f), true);
    SpawnBlock(TEXT("GearRoomEast"), FVector(3285.f, -785.f, 90.f), FVector(.4f, 5.8f, 3.8f), true);
    SpawnBlock(TEXT("GearRoomSouth"), FVector(2775.f, -1085.f, 90.f), FVector(10.2f, .4f, 3.8f), true);
    GearBag = SpawnBlock(TEXT("GearBag"), FVector(2700.f, -830.f, -68.f), FVector(.65f, .7f, .55f), false);
    AddWorldTrace(TEXT("PropertyCustodyReceipt"),
        FVector(2290.f, -850.f, 25.f), FVector(.06f, .44f, .50f),
        TEXT("PROPERTY REGISTER: SEALED GEAR BAG. CLAIMANT FIELD EMPTY."),
        MG_FOX_CAPTURED, 0u);
    PlantedTransmitter = SpawnBlock(TEXT("PlantedTransmitter"), FVector(2790.f, -833.f, -30.f), FVector(.2f, .2f, .16f), false);
    if (IsValid(PlantedTransmitter)) PlantedTransmitter->SetActorHiddenInGame(true);

    SpawnBlock(TEXT("BossLowCoverA"), FVector(1960.f, 170.f, -25.f), FVector(2.1f, 1.f, 1.5f), true);
    SpawnBlock(TEXT("BossLowCoverB"), FVector(2610.f, -50.f, -25.f), FVector(1.1f, 1.6f, 1.5f), true);
    SpawnBlock(TEXT("BossLowCoverC"), FVector(3330.f, 230.f, -25.f), FVector(1.4f, .8f, 1.5f), true);
    // Full corridor width: the later dog wing must never be reachable by
    // slipping around this door before Card 3 and the exit transaction.
    ChapterExit = SpawnBlock(TEXT("ChapterExit"), FVector(3610.f, 0.f, 90.f),
        FVector(.45f, 10.f, 3.8f), true);
    if (GetWorld())
        Shotmaker = GetWorld()->SpawnActor<AMGShootGunnerActor>(
            FVector(3100.f, 55.f, -11.f), FRotator(0.f, 180.f, 0.f));
}

void AMGGrayFoxMission::CreatePostCard3Service()
{
    // Adaptation of the original MSX2's Card 3 -> guard-dog -> suspicious
    // plastic-explosive wall sequence. Layout/metres are explicitly NEW,
    // reversible 3D interpretation, not an extracted J2ME or MSX2 map.
    // The single contextual charge opens the relay passage to the lift hall.
    SpawnBlock(TEXT("PostCard3Floor"), FVector(4650.f, 0.f, -150.f),
        FVector(18.f, 10.f, 1.f), true);
    SpawnBlock(TEXT("PostCard3NorthBoundary"), FVector(4650.f, 500.f, 90.f),
        FVector(18.f, .4f, 3.8f), true);
    SpawnBlock(TEXT("PostCard3SouthBoundary"), FVector(4650.f, -500.f, 90.f),
        FVector(18.f, .4f, 3.8f), true);
    ServiceWall = SpawnBlock(TEXT("PostCard3FutureSectorWall"), FVector(5550.f, 0.f, 90.f),
        FVector(.4f, 10.f, 3.8f), true);

    ChargeParcel = SpawnBlock(TEXT("ServiceChargeParcel"), FVector(4670.f, 155.f, -70.f),
        FVector(.65f,.48f,.4f), false);
    SpawnBlock(TEXT("RelayFloor"), FVector(6050.f,0.f,-150.f), FVector(10.f,10.f,1.f), true);
    for (const float Y : {-500.f,500.f})
        SpawnBlock(TEXT("RelaySideWall"), FVector(6050.f,Y,90.f), FVector(10.f,.4f,3.8f), true);
    SpawnBlock(TEXT("LiftHallFloor"), FVector(7210.f,0.f,-150.f), FVector(13.4f,10.f,1.f), true);
    for (float Y : {-500.f,500.f})
        SpawnBlock(TEXT("LiftHallSide"), FVector(7210.f,Y,90.f), FVector(13.4f,.4f,3.8f), true);
    SpawnBlock(TEXT("RoofLift"), FVector(7880.f,0.f,90.f), FVector(.4f,10.f,3.8f), true);
    SpawnBlock(TEXT("LiftMotorWest"), FVector(6790.f,140.f,-5.f), FVector(.9f,1.5f,1.9f), true);
    SpawnBlock(TEXT("LiftMotorEast"), FVector(7230.f,-100.f,-5.f), FVector(.9f,1.5f,1.9f), true);
    MachinegunKid = GetWorld()->SpawnActor<AMGMachinegunKid>(FVector(7560.f,80.f,-11.f),FRotator(0.f,180.f,0.f));
    ParachutePack = SpawnBlock(TEXT("LiftParachute"),FVector(7700.f,-200.f,-68.f),FVector(.72f,.59f,.50f),false);
    SpawnBlock(TEXT("RelaySwitchbank"), FVector(5900.f,150.f,-12.f), FVector(1.8f,.8f,1.76f), true);
    RelayDesk = SpawnBlock(TEXT("RelayHeadset"), FVector(6320.f,-145.f,-7.f), FVector(.48f,.42f,.24f), false);
    SpawnBlock(TEXT("RelayDesk"), FVector(6320.f,-170.f,-63.f), FVector(1.5f,.7f,.7f), true);
    // Kennel architecture: evidence of a working security service, not
    // arbitrarily scattered enemies or an invisible dog spawn trigger.
    SpawnBlock(TEXT("KennelConcreteNorth"), FVector(4140.f, 362.f, -25.f),
        FVector(2.9f, 1.1f, 1.5f), true);
    SpawnBlock(TEXT("KennelGateFrame"), FVector(4210.f, 242.f, 91.f),
        FVector(.18f, .16f, 3.f), true);
    SpawnBlock(TEXT("DogCareSupply"), FVector(4720.f, 280.f, -30.f),
        FVector(1.8f, 1.2f, 1.4f), true);
    AddWorldTrace(TEXT("KennelDutyBoard"),
        FVector(4720.f, 213.f, 20.f), FVector(.55f, .08f, .48f),
        TEXT("KENNEL: FEED / WATER / NIGHT WALK. SECOND HANDLER ABSENT."),
        MG_FOX_SLICE_COMPLETED, 0u);
    SpawnBlock(TEXT("ServiceJunctionCover"), FVector(4980.f, -105.f, -25.f),
        FVector(1.6f, 1.0f, 1.5f), true);

    // Physically real crawl route south of the dog's main sightline.
    // Floor top = -100 cm, overhead underside = -19 cm: ~81 cm gap.
    // Actual prone capsule height = 76 cm; crouched = 110 cm, so only
    // a clearance-validated prone body can traverse. The side rails stop
    // the player from clipping around the low roof once committed to it.
    // This is an optional 3D stealth route; the main north lane stays open.
    SpawnBlock(TEXT("ServiceDuctRoof"), FVector(4430.f, -300.f, 10.f),
        FVector(3.3f, 2.4f, .58f), true);
    SpawnBlock(TEXT("ServiceDuctSouthRail"), FVector(4430.f, -458.f, -25.f),
        FVector(3.5f, .46f, 1.5f), true);
    SpawnBlock(TEXT("ServiceDuctNorthRail"), FVector(4430.f, -142.f, -25.f),
        FVector(3.5f, .46f, 1.5f), true);
    // Low exit lip is set below Snake's prone clearance; never hard-lock
    // the capsule behind an arbitrary "press crouch" prompt.
    SpawnBlock(TEXT("ServiceDuctExitGrate"), FVector(4770.f, -300.f, -69.f),
        FVector(.16f, 1.1f, .35f), false);
    // The next destructible seam is represented as visible evidence but
    // marks the placement point for the contextual service charge.
    ChargeMarker = SpawnBlock(TEXT("PostCard3DamagedSeam"), FVector(5521.f, 0.f, 30.f),
        FVector(.06f, .14f, 2.1f), false);

    if (!GetWorld()) return;
    if (AMGSentryDogActor* DogA = GetWorld()->SpawnActor<AMGSentryDogActor>(
        FVector(4090.f, 180.f, -53.f), FRotator(0.f, 180.f, 0.f)))
        DogA->ConfigureBeatPatrol(FVector(1.f, 0.f, 0.f), 130.f);
    if (AMGSentryDogActor* DogB = GetWorld()->SpawnActor<AMGSentryDogActor>(
        FVector(4900.f, 190.f, -53.f), FRotator(0.f, 180.f, 0.f)))
        DogB->ConfigureBeatPatrol(FVector(0.f, 1.f, 0.f), 115.f);
}

void AMGGrayFoxMission::CreateConnectiveSpaces()
{
    // Real-world reference logic: Cold War command bunkers, service corridors,
    // vehicle logistics and staff support spaces. 1995 Outer Heaven's actual
    // architecture is NOT depicted in MGSV. These are reversible 3D inventions,
    // NOT canonical MSX2 room dimensions or copied real-world blueprints.
    //
    // 1 Unreal BasicShapes Cube = 100 cm. Existing floor surface is Z=-100.
    // Functional ducts and cable runs are above standing-player clearance.
    // Keep the original north/south perimeter and canonical gates untouched.

    // A. Perimeter: a partly roofed service walk and a sheltered tool alcove.
    // Main route and two existing POW/custody approaches remain continuously
    // traversable; the new shelf is not a fake blocking mission door.
    SpawnBlock(TEXT("OuterServiceCableTray"), FVector(-3450.f, -460.f, 240.f),
        FVector(7.1f, .40f, .18f), false);
    SpawnBlock(TEXT("OuterServiceDrainRun"), FVector(-3390.f, -428.f, -97.f),
        FVector(5.0f, .56f, .05f), false);
    SpawnBlock(TEXT("UtilityAlcovePartialWall"), FVector(-3450.f, 390.f, 25.f),
        FVector(2.2f, .22f, 2.5f), true);
    SpawnBlock(TEXT("UtilityToolCart"), FVector(-3280.f, 425.f, -40.f),
        FVector(.85f, .55f, 1.1f), true);
    AddWorldTrace(TEXT("ServiceMaintenanceLedger"),
        FVector(-3330.f, 376.f, 8.f), FVector(.47f, .06f, .43f),
        TEXT("1991: PURIFIER REBUILT. ORIGINAL FITTING DATE UNKNOWN."),
        0u, MG_FOX_CAPTURED);

    // A second, clearly distinct visual landmark before the custody truck.
    // Freight shelving has a reason to exist: equipment goes in/out through
    // this service yard, and the receiving clerk's position is readable.
    SpawnBlock(TEXT("FreightReceivingRack"), FVector(-2450.f, 422.f, 52.f),
        FVector(2.6f, .75f, 2.5f), true);
    SpawnBlock(TEXT("FreightRoofCanopy"), FVector(-2500.f, 378.f, 222.f),
        FVector(5.3f, 1.9f, .22f), false);
    AddWorldTrace(TEXT("FreightReceivingSlip"),
        FVector(-2315.f, 420.f, 9.f), FVector(.33f, .06f, .46f),
        TEXT("RECEIVED: LINEN, RATIONS, SPARES. ONE CART MISSING."),
        0u, MG_FOX_CAPTURED);
    // Keep custody visible, rather than hiding the canonical transport
    // transaction behind maintenance-room keys or a second forced arrest.
    SpawnBlock(TEXT("TransportRouteFloorStripes"), FVector(-1720.f, 90.f, -98.f),
        FVector(4.6f, .06f, .03f), false);
    SpawnBlock(TEXT("CustodyLightingBracket"), FVector(-1730.f, 465.f, 260.f),
        FVector(1.6f, .28f, .22f), false);
    AddWorldTrace(TEXT("CustodyTransferDutyCard"),
        FVector(-1875.f, -405.f, 16.f), FVector(.44f, .06f, .43f),
        TEXT("CHECK SEALS BEFORE UNLOADING. HANDOVER SIGNATURE MISSING."),
        MG_FOX_DETENTION_INTELLIGENCE, MG_FOX_CAPTURED);

    // B. Prison-to-arena threshold: recognizable building services and
    // an intelligible staging vestibule, never a new gate after rescuing Fox.
    // The prison's x=1400 opening and Shoot Gunner's existing arena remain free.
    SpawnBlock(TEXT("PrisonHallCableTray"), FVector(1160.f, -445.f, 235.f),
        FVector(4.7f, .32f, .24f), false);
    SpawnBlock(TEXT("SecurityThresholdNorthPier"), FVector(1525.f, 346.f, 45.f),
        FVector(.63f, .82f, 2.9f), true);
    SpawnBlock(TEXT("SecurityThresholdSouthPier"), FVector(1525.f, -346.f, 45.f),
        FVector(.63f, .82f, 2.9f), true);
    SpawnBlock(TEXT("SecurityThresholdHeader"), FVector(1525.f, 0.f, 245.f),
        FVector(.68f, 7.8f, .27f), false);
    // The empty gap between the two piers exceeds 6 metres. Neither pier
    // covers an authentic keycard, boss-entry flag or crouch-only shortcut.
    AddWorldTrace(TEXT("SecurityVestibuleInspectionCard"),
        FVector(1610.f, -410.f, 18.f), FVector(.48f, .07f, .46f),
        TEXT("SECURITY: QUARTERLY SEAL CHECK OVERDUE. SHIFT SUPERVISOR TO INSPECT."),
        MG_FOX_LEFT_PRISON, 0u);

    // C. Post-Card-3 kennel: operational service access, not a decorative
    // tunnel that adds extra enemy placements to meet an arbitrary quota.
    // The existing real prone crawl and both dogs remain unchanged.
    SpawnBlock(TEXT("KennelSupplyCableRun"), FVector(5000.f, 443.f, 238.f),
        FVector(6.8f, .32f, .23f), false);
    SpawnBlock(TEXT("KennelDrainGrating"), FVector(5105.f, 345.f, -97.f),
        FVector(2.8f, .44f, .05f), false);
    SpawnBlock(TEXT("KennelHoseLocker"), FVector(5270.f, 424.f, 20.f),
        FVector(.70f, .65f, 2.4f), true);
    AddWorldTrace(TEXT("KennelServiceLog"),
        FVector(5175.f, 447.f, 10.f), FVector(.46f, .07f, .42f),
        TEXT("KENNEL WATER TEST FILED. DRAIN REPAIR STILL UNSCHEDULED."),
        MG_FOX_SLICE_COMPLETED, 0u);

    // Status: service architecture is original source GREYBOX, not an
    // authored 1995 prefab kit, complete 1987 map or hands-on tested level.
}

void AMGGrayFoxMission::CreateFirstPassVerticality()
{
    // NEW 1995-credible freight/service elevation. No 1987 room map has been
    // claimed for this optional bypass. 80cm high cart-access maintenance run,
    // reached with real walkable ramps, no mantle/teleport or mission flag.
    //
    // Existing ground top: Z=-100cm; deck top: Z=-20cm. 100cm cube source.
    // Ascending pitch 16.5 degrees, descending pitch -15.6 degrees. Both
    // are below Unreal's default walkable floor angle (~44 degrees).
    // Capsule centre traverses roughly -11 -> +69, still within save bounds.
    const float DeckY = -405.f;
    const FVector RampScale(2.75f, 1.5f, .16f);
    if (AStaticMeshActor* Up = SpawnBlock(
            TEXT("FreightMaintenanceRampUp"), FVector(-3665.f, DeckY, -68.f),
            RampScale, true))
        Up->SetActorRotation(FRotator(16.5f, 0.f, 0.f));

    SpawnBlock(TEXT("FreightElevatedMaintenanceDeck"),
        FVector(-3390.f, DeckY, -30.f), FVector(3.0f, 1.5f, .20f), true);

    if (AStaticMeshActor* Down = SpawnBlock(
            TEXT("FreightMaintenanceRampDown"), FVector(-3100.f, DeckY, -68.f),
            FVector(3.05f, 1.5f, .16f), true))
        Down->SetActorRotation(FRotator(-15.0f, 0.f, 0.f));

    // Structural beam and accessible observation rail. 80cm elevation is
    // not an impenetrable sniper nest; guards can still detect an exposed
    // player from grounded LOS when physically unobstructed.
    SpawnBlock(TEXT("FreightDeckSouthHandrail"),
        FVector(-3390.f, -470.f, 15.f), FVector(3.1f, .06f, .72f), true);
    SpawnBlock(TEXT("FreightDeckNorthVisualRail"),
        FVector(-3390.f, -330.f, 20.f), FVector(3.0f, .05f, .42f), false);
    SpawnBlock(TEXT("FreightDeckCrossBraceA"),
        FVector(-3470.f, DeckY, -72.f), FVector(.17f, 1.48f, .38f), false);
    SpawnBlock(TEXT("FreightDeckCrossBraceB"),
        FVector(-3310.f, DeckY, -72.f), FVector(.17f, 1.48f, .38f), false);
    AddWorldTrace(TEXT("OverlookStructuralRepairSlip"),
        FVector(-3390.f, -390.f, -14.f), FVector(.40f, .08f, .43f),
        TEXT("WALKWAY: SUPPORT CORROSION REPAIRED. LOAD LIMIT TO BE RECHECKED."),
        0u, MG_FOX_CAPTURED);

    // Accessible across BOTH ramps. The 1987 POW/transport intelligence
    // still has to be earned via the existing ground-floor interactions.
    // Never place keys, essential ammunition or route gates on the overlook.

    // A non-climbable service gantry above Shoot Gunner's battle area.
    // This gives the building industrial scale but deliberately avoids a
    // vantage from which players could defeat the boss with zero counterplay.
    // Its underside is Z=225, 325cm above the arena floor.
    SpawnBlock(TEXT("BossOverheadMaintenanceGantry"),
        FVector(2415.f, 0.f, 245.f), FVector(1.35f, 9.0f, .40f), true);
    SpawnBlock(TEXT("BossGantryNorthSupport"),
        FVector(2415.f, 472.f, 62.5f), FVector(.32f, .32f, 3.25f), true);
    SpawnBlock(TEXT("BossGantrySouthSupport"),
        FVector(2415.f, -472.f, 62.5f), FVector(.32f, .32f, 3.25f), true);
    SpawnBlock(TEXT("BossGantryCableLoop"),
        FVector(2470.f, 0.f, 195.f), FVector(.18f, 8.6f, .18f), false);
    // Guards never spawn on this unreachable overhead surface.
}

void AMGGrayFoxMission::CreateSurveillancePosts()
{
    // CCTV is a visible, lens-based 1995 security adaptation. These posts
    // observe specific existing routes, never the original hidden prisoner
    // cell, and neither post awards a new lore flag or key item.
    // Their access/isolator panels are mounted at hand height below the optic.
    if (!GetWorld()) return;

    // Prison escape corridor: watches the original corridor rather than the
    // cell where the player has no means to counter surveillance.
    if (AMGSurveillanceCameraActor* CCTV = GetWorld()->SpawnActor<AMGSurveillanceCameraActor>(
            FVector(1180.f, 424.f, 235.f), FRotator(0.f, 180.f, 0.f)))
    {
        PrisonCamera = CCTV;
        CCTV->ConfigureSector(38.f, 710.f);
    }

    // Kennel maintenance transfer: visible as the player approaches Card 3's
    // next guard-dog hazard. It cannot see through the duct walls.
    if (AMGSurveillanceCameraActor* CCTV = GetWorld()->SpawnActor<AMGSurveillanceCameraActor>(
            FVector(5170.f, 427.f, 235.f), FRotator(0.f, 180.f, 0.f)))
    {
        KennelCamera = CCTV;
        CCTV->ConfigureSector(33.f, 760.f);
    }
}

void AMGGrayFoxMission::CreateZoneLighting()
{
    if (!GetWorld()) return;
    // Source-authored 1995 lighting composition, not a finished art pass.
    // Each light cues a spatial landmark without being treated as a stealth
    // visibility multiplier; AI perception currently uses collision/posture.
    struct FZoneLightingCue
    {
        FVector Position;
        FLinearColor Tint;
        float Lumens;
        float Radius;
        bool bBossReactive;
    };
    const FZoneLightingCue Lights[] = {
        {FVector(-3200.f, -35.f, 240.f), FLinearColor(.41f,.54f,.72f), 950.f, 820.f, false}, // exterior service
        {FVector(-1710.f, 190.f, 220.f), FLinearColor(.95f,.72f,.43f), 1250.f, 610.f, false}, // active custody point
        {FVector(-930.f, 55.f, 230.f), FLinearColor(.77f,.79f,.72f), 520.f, 570.f, false}, // weak cell bulb
        {FVector(120.f, -55.f, 225.f), FLinearColor(.59f,.73f,.91f), 680.f, 600.f, false}, // Fox's cell, cold
        {FVector(1060.f, -20.f, 255.f), FLinearColor(.60f,.72f,.81f), 840.f, 840.f, false}, // guard corridor
        {FVector(2575.f, -810.f, 240.f), FLinearColor(.95f,.72f,.49f), 750.f, 700.f, false}, // confiscated gear
        {FVector(2080.f, 795.f, 235.f), FLinearColor(.61f,.75f,.85f), 670.f, 640.f, false}, // aftermath card room
        {FVector(2990.f, 45.f, 280.f), FLinearColor(.99f,.50f,.31f), 650.f, 940.f, true} // boss threat
    };
    for (const FZoneLightingCue& Cue : Lights)
    {
        APointLight* Actor = GetWorld()->SpawnActor<APointLight>(
            Cue.Position, FRotator::ZeroRotator);
        if (!Actor) continue;
        UPointLightComponent* Light = Actor->GetPointLightComponent();
        if (!Light) continue;
        Light->SetMobility(EComponentMobility::Movable);
        Light->SetIntensity(Cue.Lumens);
        Light->SetAttenuationRadius(Cue.Radius);
        Light->SetLightColor(Cue.Tint);
        // Avoid eight dynamic shadow-casting lights until actual performance
        // profiling gives budget; geometry can still receive surface shading.
        Light->SetCastShadows(false);
        if (Cue.bBossReactive) BossThreatLight = Actor;
    }
}

void AMGGrayFoxMission::UpdateZoneLighting(float DeltaSeconds)
{
    if (!IsValid(BossThreatLight)) return;
    UPointLightComponent* Light = BossThreatLight->GetPointLightComponent();
    if (!Light) return;
    // Visual storytelling tied to *committed* event: the large firing-space
    // warning light intensifies while the gunner controls the wing, then
    // recedes after defeat. No invented damage or lock from a lighting cue.
    const float Target = (Flags & MG_FOX_SHOTMAKER_DEFEATED) ? 240.f :
        ((Flags & MG_FOX_SHOTMAKER_ENCOUNTERED) ? 1550.f : 650.f);
    Light->SetIntensity(FMath::FInterpTo(Light->Intensity, Target,
        FMath::Max(0.f, DeltaSeconds), 1.2f));
}

void AMGGrayFoxMission::SpawnPerimeterGuards()
{
    // Existing hostile patrols outside confinement. No surprise wave on rescue.
    if (!GetWorld()) return;
    if (AMGGuardActor* G = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(840.f, 360.f, -11.f), FRotator(0.f, 180.f, 0.f)))
    {
        G->SetRole(EMGGuardRole::Prison);
        G->ConfigurePatrol(FVector(0.f, -1.f, 0.f), 180.f, 76.f);
        G->SetBodyPersistenceId(TEXT("P03_PrisonNorth"));
    }
    if (AMGGuardActor* G = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(1120.f, -340.f, -11.f), FRotator(0.f, 180.f, 0.f)))
    {
        G->SetRole(EMGGuardRole::Prison);
        G->ConfigurePatrol(FVector(0.f, 1.f, 0.f), 170.f, 70.f);
        G->SetBodyPersistenceId(TEXT("P03_PrisonSouth"));
    }
}

void AMGGrayFoxMission::BeginPlay()
{
    Super::BeginPlay();
    CreatePrelude();
    CreateDispatchBay();
    CreateGreybox();
    CreateBossWing();
    CreatePostCard3Service();
    CreateConnectiveSpaces();
    CreateFirstPassVerticality();
    CreateSurveillancePosts();
    CreateZoneLighting();
    SpawnPerimeterGuards();
    StoryCamera = GetWorld()->SpawnActor<AMGStoryCameraDirector>(
        PreludeStart, FRotator::ZeroRotator);
    if (const UMGGrayFoxSaveGame* Save = Cast<UMGGrayFoxSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlot, 0)))
    {
        // Reject unknown bits and structurally impossible checkpoints.
        const uint32 F = Save->MissionFlags;
        const bool bLegacy = Save->SchemaVersion == 1;
        const bool bRecognizedSchema = bLegacy || Save->SchemaVersion == 2
            || Save->SchemaVersion == 3 || Save->SchemaVersion == 4;
        const bool bValid = bRecognizedSchema
            && (F & ~(bLegacy ? 63u : 16383u)) == 0
            && (!(F & MG_FOX_CAPTURED) || (F & MG_FOX_CONFISCATED))
            && (!(F & MG_FOX_FIRST_WALL_BREACHED) || (F & MG_FOX_CAPTURED))
            && (!(F & MG_FOX_RESCUED) || (F & MG_FOX_FIRST_WALL_BREACHED))
            && (!(F & MG_FOX_EXIT_WALL_BREACHED) || (F & MG_FOX_RESCUED))
            && (!(F & MG_FOX_LEFT_PRISON) || (F & MG_FOX_EXIT_WALL_BREACHED))
            && (!(F & MG_FOX_SHOTMAKER_ENCOUNTERED) || (F & MG_FOX_LEFT_PRISON))
            && (!(F & MG_FOX_GEAR_RECOVERED) || (F & MG_FOX_SHOTMAKER_ENCOUNTERED))
            && (!(F & MG_FOX_TRANSMITTER_PLANTED) || (F & MG_FOX_GEAR_RECOVERED))
            && (!(F & MG_FOX_TRANSMITTER_REMOVED) || (F & MG_FOX_TRANSMITTER_PLANTED))
            && (!(F & MG_FOX_SHOTMAKER_DEFEATED) || (F & MG_FOX_GEAR_RECOVERED))
            && (!(F & MG_FOX_CARD3_RECOVERED) || (F & MG_FOX_SHOTMAKER_DEFEATED))
            && (!(F & MG_FOX_SLICE_COMPLETED) || (F & MG_FOX_CARD3_RECOVERED));
        const FVector P = Save->CheckpointLocation;
        const bool bPositionSafe = !P.ContainsNaN()
            && P.X >= ((F & MG_FOX_CAPTURED) ? -1160.f : -3790.f)
            && P.X <= ((F & MG_FOX_CAPTURED)
                ? ((F & MG_FOX_SLICE_COMPLETED) ? (Save->bServiceWallOpen && Save->bServiceChargeCollected ? 7830.f : 5480.f)
                    : ((F & MG_FOX_SHOTMAKER_ENCOUNTERED) ? 3650.f : 1310.f))
                : -1300.f)
            && FMath::Abs(P.Y) < ((F & MG_FOX_SHOTMAKER_ENCOUNTERED) ? 1020.f : 410.f)
            && P.Z >= -95.f && P.Z <= 180.f
            // A stale save must not place the player beyond a solid, unopened wall.
            && (!(F & MG_FOX_CAPTURED)
                || ((F & MG_FOX_FIRST_WALL_BREACHED) || P.X < -350.f))
            && (!(F & MG_FOX_CAPTURED)
                || ((F & MG_FOX_EXIT_WALL_BREACHED) || P.X < 420.f))
            // Later boss checkpoints cannot appear before the prison exit.
            && ((F & MG_FOX_LEFT_PRISON) || P.X < 1300.f);
        if (bValid && bPositionSafe
            && ((F & MG_FOX_CAPTURED) || (F & MG_FOX_DETENTION_INTELLIGENCE)
                || (Save->SchemaVersion == 4 && (Save->bEarnedHandgun || Save->bOwnsCarton || Save->bDispatchCameraOffline))))
        {
            Flags = F;
            SavedPosition = Save->CheckpointLocation;
            LoadedSchemaVersion = Save->SchemaVersion;
            LoadedHandgunRounds = Save->HandgunRounds;
            LoadedStunRounds = Save->StunRounds;
            // V1–V3 were prison-slice builds with the handgun pre-owned.
            bLoadedHandgunAcquired = Save->SchemaVersion < 4 || Save->bEarnedHandgun;
            bEarlyHandgunAcquired = bLoadedHandgunAcquired;
            bServiceChargeCollected = (F & MG_FOX_SLICE_COMPLETED) && Save->bServiceChargeCollected;
            bServiceWallOpen = bServiceChargeCollected && Save->bServiceWallOpen;
            bRelayIntel = bServiceWallOpen && Save->bRelayIntel;
            bKidDefeated = bServiceWallOpen && Save->bKidDefeated;
            bParachuteTaken = bKidDefeated && Save->bParachuteTaken;
            if (bKidDefeated && IsValid(MachinegunKid)) MachinegunKid->Destroy();
            if (bParachuteTaken && IsValid(ParachutePack)) ParachutePack->SetActorHiddenInGame(true);
            if (IsValid(ChargeParcel)) ChargeParcel->SetActorHiddenInGame(bServiceChargeCollected);
            if (bServiceWallOpen)
            {
                if (IsValid(ServiceWall)) ServiceWall->Destroy();
                if (IsValid(ChargeMarker)) ChargeMarker->Destroy();
            }
            bOwnsCarton = Save->bOwnsCarton;
            bHeardFreightClerk = Save->bHeardFreightClerk;
            if (bOwnsCarton && IsValid(CartonPickup)) CartonPickup->SetActorHiddenInGame(true);
            if (Save->bDispatchCameraOffline && DispatchCamera.IsValid())
                DispatchCamera->RestoreOfflineState();
            bHeardFoxFollowup = Save->bHeardFoxFollowup;
            if (Save->bPrisonCameraOffline && PrisonCamera.IsValid())
                PrisonCamera->RestoreOfflineState();
            if (Save->bKennelCameraOffline && KennelCamera.IsValid())
                KennelCamera->RestoreOfflineState();
            // Additive V4 guard body world snapshots. Bodies are restored
            // only AFTER the save itself and all story flags pass validation.
            // Unknown IDs and out-of-world coordinates fail closed; never
            // spawn new actor identities from untrusted save data.
            for (const FMGPersistedGuardBody& Body : Save->GuardBodies)
            {
                if (Body.Id.IsNone() || Body.WorldPosition.ContainsNaN() ||
                    Body.WorldPosition.X < -3900.f ||
                    Body.WorldPosition.X > 7950.f ||
                    FMath::Abs(Body.WorldPosition.Y) > 980.f ||
                    Body.WorldPosition.Z < -100.f || Body.WorldPosition.Z > 190.f)
                    continue;
                for (TActorIterator<AMGGuardActor> It(GetWorld()); It; ++It)
                {
                    if (It->GetBodyPersistenceId() != Body.Id) continue;
                    It->RestoreIncapacitatedBody(Body.WorldPosition, Body.bUnconscious);
                    break;
                }
            }
            bHasLoadedSave = true;
        }
    }
    RestoreVisualState();
    InitializePlayerIfReady();
}

void AMGGrayFoxMission::RestoreVisualState()
{
    if (bEarlyHandgunAcquired && IsValid(EarlyHandgunPickup))
        EarlyHandgunPickup->SetActorHiddenInGame(true);
    if (((Flags & MG_FOX_DETENTION_INTELLIGENCE) || (Flags & MG_FOX_CAPTURED))
        && IsValid(POWInformant))
        POWInformant->SetActorHiddenInGame(true);
    if ((Flags & MG_FOX_FIRST_WALL_BREACHED) && IsValid(FirstWallCrack))
        FirstWallCrack->Destroy();
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) && IsValid(SecondWallCrack))
        SecondWallCrack->Destroy();
    if ((Flags & MG_FOX_FIRST_WALL_BREACHED) && IsValid(WeakWall))
        WeakWall->Destroy();
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) && IsValid(EscapeWall))
        EscapeWall->Destroy();
    if (AMGPrisonerCharacter* Fox = Cast<AMGPrisonerCharacter>(FoxProxy))
        Fox->SetRescued((Flags & MG_FOX_RESCUED) != 0);
    if (Flags & MG_FOX_GEAR_RECOVERED)
    {
        bGearDoorOpen = true;
        if (IsValid(GearDoor)) GearDoor->Destroy();
    }
    if (Flags & MG_FOX_CARD3_RECOVERED)
    {
        bCardDoorOpen = true;
        if (IsValid(Card2Door)) Card2Door->Destroy();
        if (IsValid(Card3Pickup)) Card3Pickup->SetActorHiddenInGame(true);
    }
    if (Flags & MG_FOX_SLICE_COMPLETED)
        if (IsValid(ChapterExit)) ChapterExit->Destroy();
    if (IsValid(PlantedTransmitter))
        PlantedTransmitter->SetActorHiddenInGame(
            !(Flags & MG_FOX_TRANSMITTER_PLANTED)
            || (Flags & MG_FOX_TRANSMITTER_REMOVED));
    if (IsValid(Shotmaker))
    {
        if (Flags & MG_FOX_SHOTMAKER_DEFEATED) Shotmaker->RestoreDefeated();
        else if (Flags & MG_FOX_SHOTMAKER_ENCOUNTERED) Shotmaker->SetEncounterActive(true);
    }
}

bool AMGGrayFoxMission::StartStoryCinematic(
    FName Scene, AMGPlayerCharacter* Player, AActor* Focus)
{
    return IsValid(StoryCamera) && StoryCamera->PlayScene(Scene, Player, Focus);
}

void AMGGrayFoxMission::SkipStoryCinematic()
{
    if (IsValid(StoryCamera)) StoryCamera->SkipScene();
}

void AMGGrayFoxMission::NotifyBodyRelocated(AMGPlayerCharacter* Player)
{
    // Persist the actual placement, not the cinematic performance or E key.
    if (bPlayerInitialised && IsValid(Player)) SaveProgress(Player);
}

void AMGGrayFoxMission::InitializePlayerIfReady()
{
    if (bPlayerInitialised) return;
    AMGPlayerCharacter* Player =
        Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player || !Player->GetEquipment() ||
        !Player->GetEquipment()->HasInitializedInventory()) return;
    // Never restore inventory before UMGEquipmentComponent::BeginPlay has
    // populated canonical slots, regardless of actor BeginPlay ordering.
    Player->SetActorLocation(bHasLoadedSave ? SavedPosition : PreludeStart,
                             false, nullptr, ETeleportType::TeleportPhysics);
    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FRotator(-8.f, 0.f, 0.f));
    bPlayerInitialised = true;
    // Post-Card3 encounter checkpoint is a subsequent physical area, not a
    // second rescue or repeated boss-introduction flag after reload.
    bPostCard3ServiceEntered = bHasLoadedSave
        && (Flags & MG_FOX_SLICE_COMPLETED) && SavedPosition.X > 3850.f;
    // The historic confiscation flag is retained for canon and save fidelity.
    // Gear usability depends on the distinct actual recovery transaction.
    if (UMGEquipmentComponent* Inventory = Player->GetEquipment())
    {
        if (bParachuteTaken) Inventory->AwardItem(TEXT("PARACHUTE"),1);
        if (bOwnsCarton && !(Flags & MG_FOX_CAPTURED))
            Inventory->AwardItem(TEXT("FREIGHT_CARTON"), 1);
        // Capture removes the borrowed carton. The 1987 permanent box
        // remains unowned until its original B1-F3 Card-2 gate is authored.
        if (bHasLoadedSave && bLoadedHandgunAcquired)
        {
            // Ownership and remaining rounds are different facts. Older
            // V1/V2 checkpoint fields never carried ammunition totals.
            Inventory->AwardWeapon(TEXT("HANDGUN"),
                LoadedSchemaVersion >= 3 ? 0 : 24);
            if (LoadedSchemaVersion >= 3)
                Inventory->RestoreHandgunRounds(LoadedHandgunRounds);
        }
        if (bHasLoadedSave && LoadedSchemaVersion >= 3)
            Inventory->RestoreStunRounds(LoadedStunRounds);
        Inventory->SetConfiscated((Flags & MG_FOX_CAPTURED)
            && !(Flags & MG_FOX_GEAR_RECOVERED));
        if (IsTransmitterActive())
            Inventory->AwardItem(TEXT("TRANSMITTER"), 1);
        // Rebuild a non-consumable from a completed mission transaction after
        // a checkpoint load. Never award it merely from a boss defeat.
        if (Flags & MG_FOX_CARD3_RECOVERED)
            Inventory->AwardItem(TEXT("CARD3"), 1);
    }

    NotifyPacingPhaseIfChanged();
    if (!bHasLoadedSave)
    {
        // Camera reveal and codec are concurrent; no movement freeze or
        // forced press-to-continue exposition. A reload never replays this.
        StartStoryCinematic(TEXT("opening"), Player, Player);
        // Open with the authentic transceiver paradigm, without stealing
        // movement. Players can close it immediately or keep walking.
        Player->OpenIncomingMissionBriefing();
        Dialogue = TEXT("OPERATION INTRUDE N313 — BIG BOSS / 120.85.");
        DialogueTimeRemaining = 7.f;
    }
    else
    {
        Dialogue = (Flags & MG_FOX_CAPTURED)
            ? TEXT("OPERATIVE CHECKPOINT RESTORED.")
            : TEXT("INFILTRATION CHECKPOINT RESTORED.");
        DialogueTimeRemaining = 3.f;
    }
}

void AMGGrayFoxMission::SaveProgress(AMGPlayerCharacter* Player)
{
    UMGGrayFoxSaveGame* Save = Cast<UMGGrayFoxSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UMGGrayFoxSaveGame::StaticClass()));
    if (!Save) return;
    Save->SchemaVersion = 4;
    Save->MissionFlags = Flags;
    Save->bServiceChargeCollected = bServiceChargeCollected;
    Save->bServiceWallOpen = bServiceWallOpen;
    Save->bRelayIntel = bRelayIntel;
    Save->bKidDefeated = bKidDefeated;
    Save->bParachuteTaken = bParachuteTaken;
    Save->bOwnsCarton = bOwnsCarton;
    Save->bHeardFreightClerk = bHeardFreightClerk;
    Save->bDispatchCameraOffline = DispatchCamera.IsValid() && !DispatchCamera->IsOperational();
    Save->bHeardFoxFollowup = bHeardFoxFollowup;
    Save->bPrisonCameraOffline = PrisonCamera.IsValid() && !PrisonCamera->IsOperational();
    Save->bKennelCameraOffline = KennelCamera.IsValid() && !KennelCamera->IsOperational();
    Save->bEarnedHandgun = bEarlyHandgunAcquired;
    for (TActorIterator<AMGGuardActor> It(GetWorld()); It; ++It)
    {
        if (!It->IsIncapacitated() || It->GetBodyPersistenceId().IsNone())
            continue;
        FMGPersistedGuardBody Body;
        Body.Id = It->GetBodyPersistenceId();
        Body.WorldPosition = It->GetActorLocation();
        Body.bUnconscious = It->IsUnconscious();
        Save->GuardBodies.Add(Body);
    }
    Save->CheckpointLocation = Player ? Player->GetActorLocation() : PreludeStart;
    if (Player && Player->GetEquipment())
    {
        Save->HandgunRounds = Player->GetEquipment()->GetHandgunRounds();
        Save->StunRounds = Player->GetEquipment()->GetStunRounds();
    }
    if (!UGameplayStatics::SaveGameToSlot(Save, SaveSlot, 0))
        UE_LOG(LogTemp, Warning, TEXT("Gray Fox: could not save mission checkpoint"));
}

bool AMGGrayFoxMission::ApplyAction(uint32 Action, AMGPlayerCharacter* Player)
{
    uint32 Next = Flags;
#if MG_WITH_RUST
    if (mg_core_abi_version() != 1 || !mg_gray_fox_transition(Flags, Action, &Next))
        return false;
#else
    Next = FallbackStep(Flags, Action);
    if (Next == Flags) return false;
#endif
    Flags = Next;
    ElapsedSinceBeat = 0.f;
    SecondsSinceExploration = 0.f;
    bHasObservedPlayer = false;
    switch (Action)
    {
    case MG_FOX_CAPTURE:
        if (Player)
        {
            // The exact camera/guard arrest and transportation performance is
            // a Sequencer requirement; this commit is an honest functional
            // handoff. Confiscation state commits BEFORE transfer.
            if (Player->GetEquipment())
            {
                // The freight carton belongs to receiving, not Snake's
                // permanent inventory. Do not smuggle it through custody.
                Player->GetEquipment()->RevokeTemporaryItem(TEXT("FREIGHT_CARTON"));
                Player->GetEquipment()->SetConfiscated(true);
            }
            Player->SetActorLocation(CellStart, false, nullptr,
                ETeleportType::TeleportPhysics);
            // Skip-safe fall-back for the still-unmade custody -> unconscious
            // transport Sequencer: game state and checkpoint handoff occur
            // immediately, then the view recovers from black in the cell.
            // No VO, animation or camera callback can change the capture flag.
            if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
            {
                if (APlayerCameraManager* Camera = PC->PlayerCameraManager)
                    Camera->StartCameraFade(1.f, 0.f, .82f,
                        FLinearColor::Black, true, false);
            }
        }
        if (IsValid(Player))
            StartStoryCinematic(TEXT("capture"),Player,Player);
        Dialogue = TEXT("DETENTION — WEAPONS CONFISCATED. ASSESS THE CELL.");
        break;
    case MG_FOX_LEARN_DETENTION_ROUTE:
        if (IsValid(POWInformant)) POWInformant->SetActorHiddenInGame(true);
        Dialogue = TEXT("POW: THEY KEEP FOX IN SECRET DETENTION. THE TRUCK TAKES PRISONERS INSIDE.");
        break;
    case MG_FOX_BREACH_FIRST_WALL:
        if (IsValid(WeakWall)) WeakWall->Destroy();
        Dialogue = TEXT("The weakened wall gives way. Someone is in the adjacent cell.");
        break;
    case MG_FOX_RESCUE:
        if (AMGPrisonerCharacter* Fox = Cast<AMGPrisonerCharacter>(FoxProxy))
            Fox->SetRescued(true);
        // Placeholder original prose, not a published game-script transcription.
        Dialogue = TEXT("GRAY FOX: I saw it walk, Snake. Metal Gear can launch a nuclear missile. Find Dr. Pettrovich.");
        if (IsValid(Player) && IsValid(FoxProxy))
            StartStoryCinematic(TEXT("fox"),Player,FoxProxy);
        // Optional dry recorded human voice, positioned at the actual cell.
        // The player receives the rescue + intelligence even with no VO asset.
        if (IsValid(FoxProxy))
        {
            if (USoundBase* Cue = LoadObject<USoundBase>(nullptr,
                    TEXT("/Game/Audio/VO/gray_fox/VO_fox_rescue_01.VO_fox_rescue_01")))
            {
                UGameplayStatics::PlaySoundAtLocation(this, Cue,
                    FoxProxy->GetActorLocation(), .78f);
                OnNarrativeVoiceStarted(FName(TEXT("gray_fox")),
                    FName(TEXT("VO_fox_rescue_01")), Cue->GetDuration());
            }
        }
        break;
    case MG_FOX_BREACH_EXIT_WALL:
        if (IsValid(EscapeWall)) EscapeWall->Destroy();
        Dialogue = TEXT("The outer passage is open. Stay low. Guards are nearby.");
        break;
    case MG_FOX_ESCAPE:
        Dialogue = TEXT("PRISON EXIT REACHED — THE INFILTRATION CONTINUES.");
        break;
    case MG_FOX_ENTER_SHOTMAKER:
        if (IsValid(Shotmaker)) Shotmaker->SetEncounterActive(true);
        if (IsValid(Player) && IsValid(Shotmaker))
            StartStoryCinematic(TEXT("shotmaker"),Player,Shotmaker);
        // Remake-authored prison-owner threat. It is a performance cue,
        // never a magic boss activation event or an uncaptioned voice.
        Dialogue = TEXT("SHOOT GUNNER: You made it out of your cell. Not out of my prison.");
        if (IsValid(Shotmaker))
        {
            if (USoundBase* Cue = LoadObject<USoundBase>(nullptr,
                    TEXT("/Game/Audio/VO/shotmaker/VO_shotmaker_intro_01.VO_shotmaker_intro_01")))
            {
                UGameplayStatics::PlaySoundAtLocation(this, Cue,
                    Shotmaker->GetActorLocation(), .76f);
                OnNarrativeVoiceStarted(FName(TEXT("shotmaker")),
                    FName(TEXT("VO_shotmaker_intro_01")), Cue->GetDuration());
            }
        }
        break;
    case MG_FOX_RECOVER_GEAR:
        if (Player && Player->GetEquipment())
        {
            Player->GetEquipment()->SetConfiscated(false);
            Player->GetEquipment()->AwardItem(TEXT("TRANSMITTER"), 1);
        }
        if (IsValid(PlantedTransmitter)) PlantedTransmitter->SetActorHiddenInGame(false);
        Dialogue = TEXT("GEAR RECOVERED — EQUIP THE HANDGUN. WHAT IS THAT DEVICE?");
        break;
    case MG_FOX_REMOVE_TRANSMITTER:
        if (Player && Player->GetEquipment())
            Player->GetEquipment()->DiscardPlantedTransmitter();
        if (IsValid(PlantedTransmitter)) PlantedTransmitter->SetActorHiddenInGame(true);
        Dialogue = TEXT("ENEMY TRANSMITTER REMOVED.");
        break;
    case MG_FOX_DEFEAT_SHOTMAKER:
        Dialogue = TEXT("SHOOT GUNNER DOWN — SEARCH FOR THE ACCESS CARD.");
        break;
    case MG_FOX_COLLECT_CARD3:
        if (Player && Player->GetEquipment())
            Player->GetEquipment()->AwardItem(TEXT("CARD3"), 1);
        if (IsValid(Card3Pickup)) Card3Pickup->SetActorHiddenInGame(true);
        Dialogue = TEXT("CARD 3 RECOVERED — ROUTE FORWARD OPEN.");
        if (IsValid(Player))
            StartStoryCinematic(TEXT("card3"),Player,Player);
        break;
    case MG_FOX_COMPLETE_SLICE:
        if (IsValid(ChapterExit)) ChapterExit->Destroy();
        Dialogue = TEXT("OUTER HEAVEN CONTINUES — CHAPTER COMPLETE.");
        break;
    default: break;
    }
    // Save AFTER item and weapon transactions. A reload must match the
    // currently visible inventory, not the previous frame's values.
    SaveProgress(Player);
    NotifyPacingPhaseIfChanged();
    // Short subtitle holds are placeholder reading windows; dialogue never locks control.
    DialogueTimeRemaining = Action == MG_FOX_RESCUE ? 7.f : 4.5f;
    return true;
}

bool AMGGrayFoxMission::CanReach(
    const AMGPlayerCharacter* Player, const AActor* Target, float MaxDistanceCm) const
{
    if (!Player || !IsValid(Target) || !GetWorld()) return false;
    FVector Delta = Target->GetActorLocation() - Player->GetActorLocation();
    if (Delta.Size2D() > MaxDistanceCm) return false;

    const APlayerController* PC = Cast<APlayerController>(Player->GetController());
    const FVector Facing = PC ?
        PC->GetControlRotation().Vector().GetSafeNormal2D() :
        Player->GetActorForwardVector().GetSafeNormal2D();
    if (FVector::DotProduct(Facing, Delta.GetSafeNormal2D()) < 0.20f)
        return false;

    // Prevent interaction through intact security walls.
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(FoxInteractionLOS), true, Player);
    const FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, 45.f);
    const FVector End = Target->GetActorLocation() + FVector(0.f, 0.f, 45.f);
    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, ECC_Visibility, Query);
    return !bBlocked || Hit.GetActor() == Target;
}

bool AMGGrayFoxMission::TryServiceInteraction(AMGPlayerCharacter* Player)
{
    if (!(Flags & MG_FOX_SLICE_COMPLETED)) return false;
    if (!bServiceChargeCollected && CanReach(Player, ChargeParcel, 185.f))
    {
        bServiceChargeCollected = true;
        ChargeParcel->SetActorHiddenInGame(true);
        Dialogue = TEXT("Plastic explosive recovered. One charge.");
    }
    else if (!bServiceWallOpen && CanReach(Player, ChargeMarker, 200.f))
    {
        if (bServiceChargeCollected && BreachRemaining < 0.f)
        {
            BreachRemaining = 2.4f;
            ChargeMarker->SetActorScale3D(FVector(.25f,.42f,.48f));
        }
        Dialogue = bServiceChargeCollected ? TEXT("Charge ticking.") : TEXT("A patched joint. A fist will do nothing here.");
    }
    else if (bKidDefeated && !bParachuteTaken && CanReach(Player, ParachutePack, 165.f))
    {
        if (!Player->GetEquipment() || !Player->GetEquipment()->AwardItem(TEXT("PARACHUTE"),1)) return false;
        bParachuteTaken = true;
        ParachutePack->SetActorHiddenInGame(true);
        Dialogue = TEXT("Parachute recovered. The lift goes to the roof.");
    }
    else if (bServiceWallOpen && CanReach(Player, RelayDesk, 180.f))
    {
        bRelayIntel = true;
        Dialogue = TEXT("DUTY LOG: Asked for the west lift back. Machinegun Kid sent the key. Kept the lift.");
    }
    else return false;
    DialogueTimeRemaining = 5.f;
    SaveProgress(Player);
    return true;
}

bool AMGGrayFoxMission::TryInteract(AMGPlayerCharacter* Player)
{
    if (!bPlayerInitialised || !Player) return false;
    if (TryServiceInteraction(Player)) return true;
    if (!(Flags & MG_FOX_CAPTURED))
    {
        if (!bOwnsCarton && CanReach(Player, CartonPickup, 160.f) && Player->GetEquipment())
        {
            if (!Player->GetEquipment()->AwardItem(TEXT("FREIGHT_CARTON"), 1)) return false;
            bOwnsCarton = true;
            CartonPickup->SetActorHiddenInGame(true);
            Dialogue = TEXT("BORROWED FREIGHT CARTON. TEMPORARY DISGUISE FOR RECEIVING.");
            DialogueTimeRemaining = 3.f;
            SaveProgress(Player);
            return true;
        }
        if (!bEarlyHandgunAcquired &&
            CanReach(Player, EarlyHandgunPickup, 185.f))
        {
            if (UMGEquipmentComponent* Inventory = Player->GetEquipment())
                if (Inventory->AwardWeapon(TEXT("HANDGUN"), 24))
                {
                    bEarlyHandgunAcquired = true;
                    if (IsValid(EarlyHandgunPickup))
                        EarlyHandgunPickup->SetActorHiddenInGame(true);
                    Dialogue = TEXT("HANDGUN RECOVERED — EQUIP IT IN WEAPONS.");
                    DialogueTimeRemaining = 3.2f;
                    SaveProgress(Player); // Independent V4 earned-gun state.
                    return true;
                }
        }
        if (!(Flags & MG_FOX_DETENTION_INTELLIGENCE)
            && CanReach(Player, POWInformant, 225.f))
            return ApplyAction(MG_FOX_LEARN_DETENTION_ROUTE, Player);
        if (Flags & MG_FOX_DETENTION_INTELLIGENCE)
        {
            const FVector At = Player->GetActorLocation();
            const bool bAtCustody = IsValid(CustodyMarker)
                && FVector::Dist2D(At, CustodyMarker->GetActorLocation()) < 270.f;
            if (bAtCustody)
            {
                // An actual custody guard must have observed and challenged
                // Snake. The security system does NOT know this is his plan.
                const bool bChallenged =
                    (IsValid(CustodyOfficerA)
                     && CustodyOfficerA->CanAcceptDetention(Player))
                    || (IsValid(CustodyOfficerB)
                        && CustodyOfficerB->CanAcceptDetention(Player));
                if (!bChallenged)
                {
                    // The player has to be physically observed and allowed to
                    // comply. This brief challenge is an actual guard state,
                    // not an arbitrary cinematic wait-to-unlock timer.
                    const bool bBeingChallenged =
                        (IsValid(CustodyOfficerA) && CustodyOfficerA->IsIssuingChallenge())
                        || (IsValid(CustodyOfficerB) && CustodyOfficerB->IsIssuingChallenge());
                    Dialogue = bBeingChallenged
                        ? TEXT("SECURITY: STOP. HANDS WHERE I CAN SEE THEM.")
                        : TEXT("STAY VISIBLE. LET SECURITY IDENTIFY YOU.");
                    DialogueTimeRemaining = 2.8f;
                    return true;
                }
                if (Player->IsAiming() || Player->IsSprinting())
                {
                    Dialogue = TEXT("SECURITY: HANDS VISIBLE! STOP MOVING!");
                    DialogueTimeRemaining = 2.5f;
                    return true;
                }
                // The player deliberately steps into a real guard-controlled
                // prisoner transport point; the guard has no reason to infer
                // Snake's plan. A cinematic capture performance is pending.
                return ApplyAction(MG_FOX_CAPTURE, Player);
            }
        }
        return TryInspectWorldTrace(Player);
    }
    if ((Flags & MG_FOX_FIRST_WALL_BREACHED) == 0
        && CanReach(Player, WeakWall, 240.f))
    {
        Dialogue = TEXT("THE REPAIRED CONCRETE SOUNDS HOLLOW. STRIKE IT.");
        DialogueTimeRemaining = 3.f;
        return true;
    }
    if ((Flags & MG_FOX_RESCUED) == 0
        && CanReach(Player, FoxProxy, 210.f))
        return ApplyAction(MG_FOX_RESCUE, Player);
    if ((Flags & MG_FOX_RESCUED) && !(Flags & MG_FOX_EXIT_WALL_BREACHED)
        && !bHeardFoxFollowup && DialogueTimeRemaining <= 0.f
        && CanReach(Player, FoxProxy, 175.f))
    {
        bHeardFoxFollowup = true;
        Dialogue = TEXT("GRAY FOX: I counted the steps between shifts. After a while I could tell who was bringing water.");
        DialogueTimeRemaining = 5.f;
        SaveProgress(Player);
        return true;
    }
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) == 0
        && CanReach(Player, EscapeWall, 240.f))
    {
        Dialogue = TEXT("ANOTHER FRESH REPAIR. TRY A PHYSICAL BREACH.");
        DialogueTimeRemaining = 3.f;
        return true;
    }
    if (!(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED))
        return TryInspectWorldTrace(Player);

    if (!(Flags & MG_FOX_GEAR_RECOVERED) && bGearDoorOpen
        && CanReach(Player, GearBag, 200.f))
        return ApplyAction(MG_FOX_RECOVER_GEAR, Player);
    // The original game places the enemy transmitter IN Snake's inventory.
    // E on the visible evidence is an accessible physical alternative; the
    // primary 1987 solution is equip the item and discard it with item Use.
    if (IsTransmitterActive() && CanReach(Player, PlantedTransmitter, 185.f))
        return DiscardTransmitter(Player);

    if ((Flags & MG_FOX_SHOTMAKER_DEFEATED)
        && !(Flags & MG_FOX_CARD3_RECOVERED))
    {
        if (!bCardDoorOpen && CanReach(Player, Card2Door, 235.f))
        {
            bool bHasCard2 = false;
            if (const UMGEquipmentComponent* Inventory = Player->GetEquipment())
                for (const FMGEquipmentEntry& Entry : Inventory->GetItems())
                    if (Entry.Id == TEXT("CARD2") && Entry.bOwned)
                        bHasCard2 = true;
            if (bHasCard2)
            {
                bCardDoorOpen = true;
                if (IsValid(Card2Door)) Card2Door->Destroy();
                Dialogue = TEXT("CARD 2 VERIFIED — STOREROOM UNLOCKED.");
            }
            else Dialogue = TEXT("CARD 2 REQUIRED.");
            DialogueTimeRemaining = 3.f;
            return true;
        }
        if (bCardDoorOpen && CanReach(Player, Card3Pickup, 195.f))
            return ApplyAction(MG_FOX_COLLECT_CARD3, Player);
    }
    if ((Flags & MG_FOX_CARD3_RECOVERED) && !(Flags & MG_FOX_SLICE_COMPLETED)
        && CanReach(Player, ChapterExit, 235.f))
        return ApplyAction(MG_FOX_COMPLETE_SLICE, Player);
    return TryInspectWorldTrace(Player);
}

void AMGGrayFoxMission::BroadcastStructuralImpact(const FVector& Location,
                                                     float AudibleRadius, AActor* Source)
{
    if (!GetWorld()) return;
    // The wall itself is the evidence source. Guards hear that impact through
    // distance/occlusion modelling and never gain a direct visual lock or
    // Snake's transform merely because an interaction was performed.
    for (TActorIterator<AMGGuardActor> Guard(GetWorld()); Guard; ++Guard)
        if (!Guard->IsIncapacitated())
            Guard->HearWallKnock(Location, FVector(1,0,0),
                                AudibleRadius, Source);
}

bool AMGGrayFoxMission::TryPunchWeakWall(AMGPlayerCharacter* Player)
{
    if (!Player || !bPlayerInitialised || !(Flags & MG_FOX_CAPTURED))
        return false;
    const bool bFirst = !(Flags & MG_FOX_FIRST_WALL_BREACHED);
    const bool bSecond = !bFirst && (Flags & MG_FOX_RESCUED)
        && !(Flags & MG_FOX_EXIT_WALL_BREACHED);
    if (!bFirst && !bSecond) return false;
    AStaticMeshActor* Surface = bFirst ? WeakWall : EscapeWall;
    if (!CanReach(Player, Surface, 190.f)) return false;
    int32& Impacts = bFirst ? FirstWallStrikes : SecondWallStrikes;
    ++Impacts;
    const int32 Needed = bFirst ? 3 : 4; // tunable; never a timed waiting gate
    BroadcastStructuralImpact(Surface->GetActorLocation(),
        bFirst ? 790.f : 1100.f, Player);
    OnSurfaceImpact(bFirst ? FName(TEXT("PrisonWallA"))
                           : FName(TEXT("PrisonWallB")),
                    Impacts, Impacts >= Needed);
    AStaticMeshActor* Crack = bFirst ? FirstWallCrack : SecondWallCrack;
    if (IsValid(Crack))
    {
        Crack->SetActorHiddenInGame(false);
        // Accumulating light-catching fracture: physical player blows, not
        // a timer-driven wall deletion or a ready-made explosion animation.
        Crack->SetActorScale3D(FVector(1.f, FMath::Min(Impacts * .60f, 2.2f), 1.f));
    }
    if (Impacts < Needed)
    {
        Dialogue = Impacts == 1 ? TEXT("THE WALL SHIFTS. KEEP STRIKING THE SEAM.")
                                : TEXT("THE CRACK IS DEEPENING.");
        DialogueTimeRemaining = 2.5f;
        return true;
    }
    return ApplyAction(bFirst ? MG_FOX_BREACH_FIRST_WALL
                              : MG_FOX_BREACH_EXIT_WALL, Player);
}

bool AMGGrayFoxMission::TryPunchRecoveryDoor(AMGPlayerCharacter* Player)
{
    if (!Player || !bPlayerInitialised || bGearDoorOpen ||
        !(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED) ||
        (Flags & MG_FOX_GEAR_RECOVERED) ||
        !CanReach(Player, GearDoor, 210.f))
        return false;
    ++RecoveryDoorPunches;
    BroadcastStructuralImpact(GearDoor->GetActorLocation(), 950.f, Player);
    OnSurfaceImpact(FName(TEXT("RecoveryDoor")), RecoveryDoorPunches,
                    RecoveryDoorPunches >= 2);
    // Physical repeated impacts; count is a playtest hypothesis, not a
    // transcribed 1987 number. Never use a single 'E' door disappearance.
    if (RecoveryDoorPunches < 2)
        Dialogue = TEXT("THE RECOVERY DOOR SHUDDERS. STRIKE AGAIN.");
    else
    {
        bGearDoorOpen = true;
        if (IsValid(GearDoor)) GearDoor->Destroy();
        Dialogue = TEXT("RECOVERY ROOM OPEN — FIND THE CONFISCATED EQUIPMENT.");
    }
    DialogueTimeRemaining = 3.5f;
    return true;
}

bool AMGGrayFoxMission::IsBossTelegraphing() const
{
    return IsValid(Shotmaker) && Shotmaker->IsTelegraphing();
}

bool AMGGrayFoxMission::IsTransmitterActive() const
{
    return (Flags & MG_FOX_TRANSMITTER_PLANTED) != 0
        && (Flags & MG_FOX_TRANSMITTER_REMOVED) == 0;
}

bool AMGGrayFoxMission::DiscardTransmitter(AMGPlayerCharacter* Player)
{
    return Player && Player->GetEquipment()
        && Player->GetEquipment()->HasPlantedTransmitter()
        && IsTransmitterActive()
        && ApplyAction(MG_FOX_REMOVE_TRANSMITTER, Player);
}

bool AMGGrayFoxMission::HasEquipmentConfiscated() const
{
    return (Flags & MG_FOX_CONFISCATED) != 0
        && (Flags & MG_FOX_GEAR_RECOVERED) == 0;
}

void AMGGrayFoxMission::SetFacilitySecurityState(EMGFacilitySecurityState NewState)
{
    if (FacilitySecurityState == NewState) return;
    const EMGFacilitySecurityState Previous = FacilitySecurityState;
    FacilitySecurityState = NewState;
    OnFacilitySecurityStateChanged(Previous, NewState);
}

void AMGGrayFoxMission::NotifyGuardContactReported(const FVector& LastSeenPosition)
{
    // Only an actual transmitted guard sighting creates installation alert.
    LastFacilityEvidence = LastSeenPosition;
    FacilityAlertRemaining = FMath::Max(FacilityAlertRemaining, 10.f);
    FacilityEvasionRemaining = 7.f;
    SetFacilitySecurityState(EMGFacilitySecurityState::Alert);
}

void AMGGrayFoxMission::NotifyGuardBodyReported(const FVector& EvidencePosition)
{
    // A discovered body raises *caution*, never a perfect enemy location.
    LastFacilityEvidence = EvidencePosition;
    FacilityCautionRemaining = FMath::Max(FacilityCautionRemaining, 9.f);
    if (FacilitySecurityState != EMGFacilitySecurityState::Alert &&
        FacilitySecurityState != EMGFacilitySecurityState::Evasion)
        SetFacilitySecurityState(EMGFacilitySecurityState::Caution);
}

void AMGGrayFoxMission::UpdateGuardAlert(float Dt)
{
    const float Step = FMath::Max(Dt, 0.f);
    FacilityAlertRemaining = FMath::Max(0.f, FacilityAlertRemaining - Step);
    FacilityCautionRemaining = FMath::Max(0.f, FacilityCautionRemaining - Step);
    bool bLocalConfirmedContact = false;
    if (GetWorld())
        for (TActorIterator<AMGGuardActor> G(GetWorld()); G; ++G)
        {
            if (G->IsIncapacitated()) continue;
            bLocalConfirmedContact |= G->HasDirectVisualContact()
                && G->GetAwarenessState() >= 3;
            // Crucial: the facility has NOT automatically heard what one
            // uncommunicative sentry privately suspects or investigates.
            // Only explicit radio/body notifications and the enemy beacon
            // are allowed to raise installation-wide condition.
        }

    if (FacilityAlertRemaining > 0.f)
        SetFacilitySecurityState(EMGFacilitySecurityState::Alert);
    else if (FacilitySecurityState == EMGFacilitySecurityState::Alert)
    {
        FacilityEvasionRemaining = FMath::Max(FacilityEvasionRemaining, 7.f);
        SetFacilitySecurityState(EMGFacilitySecurityState::Evasion);
    }
    else if (FacilitySecurityState == EMGFacilitySecurityState::Evasion)
    {
        FacilityEvasionRemaining = FMath::Max(0.f, FacilityEvasionRemaining - Step);
        if (FacilityEvasionRemaining <= 0.f)
        {
            FacilityCautionRemaining = FMath::Max(FacilityCautionRemaining, 5.f);
            SetFacilitySecurityState(EMGFacilitySecurityState::Caution);
        }
    }
    else if (FacilityCautionRemaining > 0.f || IsTransmitterActive())
        SetFacilitySecurityState(EMGFacilitySecurityState::Caution);
    else if (FacilitySecurityState == EMGFacilitySecurityState::Caution)
    {
        FacilityStandDownRemaining = 3.f;
        SetFacilitySecurityState(EMGFacilitySecurityState::StandDown);
    }
    else if (FacilitySecurityState == EMGFacilitySecurityState::StandDown)
    {
        FacilityStandDownRemaining = FMath::Max(0.f, FacilityStandDownRemaining - Step);
        if (FacilityStandDownRemaining <= 0.f)
            SetFacilitySecurityState(EMGFacilitySecurityState::Normal);
    }

    // A local direct sighting or planted transmitter still suppresses hints,
    // even before a guard has successfully transmitted a facility alarm.
    bUnderAlert = bLocalConfirmedContact || IsTransmitterActive() ||
        FacilitySecurityState == EMGFacilitySecurityState::Alert ||
        FacilitySecurityState == EMGFacilitySecurityState::Evasion;
}

uint32 AMGGrayFoxMission::GetPacingPhase() const
{
#if MG_WITH_RUST
    MgGrayFoxPacing Report = {};
    if (mg_core_abi_version() == 1 &&
        mg_gray_fox_pacing(Flags, ElapsedSinceBeat, bUnderAlert ? 1u : 0u, &Report))
        return Report.phase;
#endif
    if (!(Flags & MG_FOX_CAPTURED))
        return (Flags & MG_FOX_DETENTION_INTELLIGENCE) ? 9 : 0;
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED)) return 1;
    if (!(Flags & MG_FOX_RESCUED)) return 2;
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED)) return 3;
    if (!(Flags & MG_FOX_LEFT_PRISON)) return 4;
    if (Flags & MG_FOX_SLICE_COMPLETED) return 5;
    if (Flags & MG_FOX_CARD3_RECOVERED) return 11;
    if (Flags & MG_FOX_SHOTMAKER_DEFEATED) return 10;
    if (Flags & MG_FOX_GEAR_RECOVERED) return 8;
    if (Flags & MG_FOX_SHOTMAKER_ENCOUNTERED) return 7;
    return 6;
}

float AMGGrayFoxMission::GetSuggestedBeatIntensity() const
{
#if MG_WITH_RUST
    MgGrayFoxPacing Report = {};
    if (mg_core_abi_version() == 1 &&
        mg_gray_fox_pacing(Flags, ElapsedSinceBeat,
            bUnderAlert ? 1u : 0u, &Report))
        return FMath::Clamp(Report.target_intensity, 0.f, 1.f);
#endif
    switch (GetPacingPhase())
    {
    case 0: return .22f;
    case 9: return .52f;
    case 1: return .12f;
    case 2: return .42f;
    case 3: return .18f;
    case 4: return .76f;
    case 6: return .49f;
    case 7: return .86f;
    case 8: return .93f;
    case 10: return .10f;
    case 11: return .28f;
    case 5: return .03f;
    default: return .25f;
    }
}

void AMGGrayFoxMission::NotifyPacingPhaseIfChanged()
{
    const int32 NewPhase = static_cast<int32>(GetPacingPhase());
    if (LastPresentedPacingPhase == NewPhase) return;
    const int32 Previous = LastPresentedPacingPhase;
    LastPresentedPacingPhase = NewPhase;
    // The Sequencer/lighting/audio can subscribe and blend in the new mood.
    // Presentation never owns a game-state transaction.
    OnPacingBeatChanged(Previous, NewPhase, GetSuggestedBeatIntensity());
}

uint32 AMGGrayFoxMission::GetHintLevel() const
{
    // Genuine detection and active volleys take precedence over optional
    // navigation hints. Card search after victory is deliberately quiet.
    const uint32 CurrentPhase = GetPacingPhase();
    if (bUnderAlert || CurrentPhase == 8 || CurrentPhase == 5 ||
        (IsValid(Shotmaker) &&
         (Shotmaker->IsTelegraphing() || Shotmaker->IsFiringVolley())))
        return 0;
    // Engine and Rust use matching candidates. These are hypotheses pending
    // first-time player tests, not canonical 'correct' hint delays.
    uint32 Level = ElapsedSinceBeat >= 75.f ? 2u : (ElapsedSinceBeat >= 28.f ? 1u : 0u);
#if MG_WITH_RUST
    MgGrayFoxPacing Report = {};
    if (mg_core_abi_version() == 1 &&
        mg_gray_fox_pacing(Flags, ElapsedSinceBeat, bUnderAlert ? 1u : 0u, &Report))
        Level = Report.hint_level;
#endif
    // Sustained navigation / looking around means the player is still testing
    // hypotheses. Delay help until they pause; hard caps avoid indefinite silence.
    if (Level >= 2 && SecondsSinceExploration < 10.f && ElapsedSinceBeat < 150.f)
        Level = 1;
    if (Level >= 1 && SecondsSinceExploration < 6.f && ElapsedSinceBeat < 100.f)
        Level = 0;
    return Level;
}

FString AMGGrayFoxMission::GetPhaseText() const
{
    switch (GetPacingPhase())
    {
    case 0: return TEXT("INFILTRATION — N313");
    case 9: return TEXT("DETENTION APPROACH");
    case 10: return TEXT("AFTERMATH — SEARCH");
    case 11: return TEXT("INNER ROUTE");
    case 1: return TEXT("CELL DISCOVERY");
    case 2: return TEXT("BREAKTHROUGH");
    case 3: return TEXT("REVELATION");
    case 4: return TEXT("ESCAPE");
    case 6: return TEXT("APPROACH");
    case 7: return TEXT("SHOOT GUNNER — UNARMED");
    case 8: return TEXT("SHOOT GUNNER — ARMED");
    default: return TEXT("RECOVERY");
    }
}

FString AMGGrayFoxMission::GetObjectiveText() const
{
    if (!(Flags & MG_FOX_CAPTURED))
        return (Flags & MG_FOX_DETENTION_INTELLIGENCE)
            ? TEXT("Investigate the transport security. Keep your weapon lowered.")
            : TEXT("Trace Gray Fox's disappearance. Look for prisoners.");
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return TEXT("Search your cell for a way out.");
    if (!(Flags & MG_FOX_RESCUED))
        return TEXT("Find and speak to Gray Fox.");
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        return TEXT("Find a route into the outer passage.");
    if (!(Flags & MG_FOX_LEFT_PRISON))
        return TEXT("Get past the guards and reach the far side of the corridor.");
    if (!(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED))
        return TEXT("Continue east through the security checkpoint.");
    if (!(Flags & MG_FOX_GEAR_RECOVERED))
        return TEXT("Survive the gunner. Find your confiscated equipment.");
    if (IsTransmitterActive())
        return TEXT("Your equipment has been tampered with. Inspect your items.");
    if (!(Flags & MG_FOX_SHOTMAKER_DEFEATED))
        return TEXT("Outmanoeuvre the gunner. Use cover between volleys.");
    if (!(Flags & MG_FOX_CARD3_RECOVERED))
        return TEXT("Investigate the secured storeroom for a way forward.");
    if (!(Flags & MG_FOX_SLICE_COMPLETED))
        return TEXT("Find the route deeper into Outer Heaven.");
    if (bParachuteTaken) return TEXT("Parachute secured. Roof lift reached. End of current build.");
    if (bKidDefeated) return TEXT("Collect the parachute at the west lift.");
    if (bRelayIntel) return TEXT("Follow the relay passage to the west lift.");
    if (bServiceWallOpen) return TEXT("Investigate the relay room beyond the kennels.");
    return TEXT("Continue through the Card 3 service passage. Scout the kennel and damaged masonry.");
}

FString AMGGrayFoxMission::GetPacingHint() const
{
    const uint32 Level = GetHintLevel();
    if (!Level) return FString();
    if (!(Flags & MG_FOX_CAPTURED))
    {
        if (!(Flags & MG_FOX_DETENTION_INTELLIGENCE))
            return Level == 1 ? TEXT("The northern cover has a captive nearby.")
                              : TEXT("Check the guarded utility area for a detained informant.");
        return Level == 1 ? TEXT("The transport vehicle is the route inside.")
                          : TEXT("Approach the truck without aiming. Comply with the challenge.");
    }
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return Level == 1 ? TEXT("The concrete has not all aged the same way.")
                          : TEXT("Look for an altered join in the wall. Examine it up close.");
    if (!(Flags & MG_FOX_RESCUED))
        return Level == 1 ? TEXT("Someone is waiting further inside this room.")
                          : TEXT("The prisoner near the middle of the room is Gray Fox.");
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        return Level == 1 ? TEXT("Old work and new repairs might meet again.")
                          : TEXT("Investigate the unusual section of the next barrier.");
    if (!(Flags & MG_FOX_LEFT_PRISON))
        return Level == 1 ? TEXT("Keep low and use the obstacles to break line of sight.")
                          : TEXT("Move east to the end of the passage, beyond the guards.");
    if (!(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED))
        return Level == 1 ? TEXT("The east passage is still open.")
                          : TEXT("Continue east past the security checkpoint.");
    if (!(Flags & MG_FOX_GEAR_RECOVERED))
        return Level == 1 ? TEXT("Storage is separated from the open firing lane.")
                          : TEXT("The southern recovery door yields to repeated strikes.");
    if (!(Flags & MG_FOX_SHOTMAKER_DEFEATED))
        return Level == 1 ? TEXT("A complete three-blast volley has a recovery window.")
                          : TEXT("Return fire from cover after his third shot.");
    if (!(Flags & MG_FOX_CARD3_RECOVERED))
        return Level == 1 ? TEXT("Look for a door controlled by an earlier access card.")
                          : TEXT("Use Card 2 at the northern storeroom, then search inside.");
    if (!(Flags & MG_FOX_SLICE_COMPLETED))
        return Level == 1 ? TEXT("The defeated boss guarded a route inward.")
                          : TEXT("Bring Card 3 to the eastern security passage.");
    return FString();
}

FString AMGGrayFoxMission::GetMissionLogText() const
{
    if (!(Flags & MG_FOX_RESCUED))
    {
        if (Flags & MG_FOX_DETENTION_INTELLIGENCE)
            return TEXT("POW INTEL: FOX IS DETAINED. A GUARDED TRANSPORT ENTERS THE SECRET CELLS.");
        if (!(Flags & MG_FOX_CAPTURED))
            return TEXT("OPERATION N313: INVESTIGATE FOX'S DISAPPEARANCE. BIG BOSS: 120.85.");
        return FString();
    }
    if (Flags & MG_FOX_CARD3_RECOVERED)
        return TEXT("GRAY FOX: METAL GEAR / MADNAR. CARD 3 ACQUIRED. CONTINUE INFILTRATION.");
    if (Flags & MG_FOX_SHOTMAKER_DEFEATED)
        return TEXT("GRAY FOX: METAL GEAR / MADNAR. SEARCH THE BOSS AREA FOR CARD 3.");
    if (Flags & MG_FOX_GEAR_RECOVERED)
    {
        if (!(Flags & MG_FOX_TRANSMITTER_REMOVED))
            return TEXT("GRAY FOX: METAL GEAR / MADNAR. RECOVERED GEAR MAY BE COMPROMISED.");
        return TEXT("GRAY FOX: METAL GEAR / MADNAR. ENEMY TRANSMITTER REMOVED.");
    }
    // Temporary paraphrase; not a verbatim line from the licensed J2ME script.
    return TEXT("GRAY FOX INTEL: Metal Gear is real. Seek the scientist who designed it. "
                "Your equipment is still missing. Continue the infiltration.");
}

FString AMGGrayFoxMission::GetRecentDialogue() const
{
    return DialogueTimeRemaining > 0.f ? Dialogue : FString();
}


FString AMGGrayFoxMission::GetTransceiverVoiceSceneKey(int32 Channel) const
{
    if (!(Flags & MG_FOX_CAPTURED))
        return Channel == 0 ? TEXT("schneider_prelude")
            : ((Flags & MG_FOX_DETENTION_INTELLIGENCE)
                ? TEXT("boss_prelude_intel") : TEXT("boss_n313_opening"));
    if (Channel == 0)
    {
        if (!(Flags & MG_FOX_FIRST_WALL_BREACHED)) return TEXT("schneider_cell");
        if (!(Flags & MG_FOX_RESCUED)) return TEXT("schneider_fox");
        if (!(Flags & MG_FOX_EXIT_WALL_BREACHED)) return TEXT("schneider_escape");
        return TEXT("schneider_unknown");
    }
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED)) return TEXT("boss_cell");
    if (!(Flags & MG_FOX_RESCUED)) return TEXT("boss_fox");
    if ((Flags & MG_FOX_LEFT_PRISON) && !(Flags & MG_FOX_GEAR_RECOVERED))
        return TEXT("boss_equipment");
    if ((Flags & MG_FOX_GEAR_RECOVERED) && !(Flags & MG_FOX_TRANSMITTER_REMOVED))
        return TEXT("boss_transmitter");
    return TEXT("boss_continue");
}

FString AMGGrayFoxMission::GetTransceiverDialogue(int32 Channel) const
{
    // Remake-authored dialogue. Opening radio uses the original 120.85
    // mission structure, but all lines below are ORIGINAL adaptation prose.
    if (!(Flags & MG_FOX_CAPTURED))
    {
        if (Channel == 1)
            return (Flags & MG_FOX_DETENTION_INTELLIGENCE)
                ? TEXT("BIG BOSS: Gray Fox is inside. Use what you learned.\n"
                       "BIG BOSS: Do not turn this into a firefight.\n"
                       "SNAKE: Understood.")
                : TEXT("BIG BOSS: Operation Intrude N313.\n"
                       "BIG BOSS: Gray Fox disappeared inside Outer Heaven.\n"
                       "BIG BOSS: Establish contact. Verify the weapon report.\n"
                       "SNAKE: Metal Gear?\n"
                       "BIG BOSS: Intelligence is incomplete. Find evidence.");
        return TEXT("SCHNEIDER: Prisoners are being moved through this sector.\n"
                    "SCHNEIDER: I don't have the inner-cell plans.\n"
                    "SNAKE: I'll investigate.");
    }
    if (Channel == 0)
    {
        if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
            return TEXT("SCHNEIDER: I helped design parts of Outer Heaven.\n"
                        "SCHNEIDER: Before they altered the plans.\n"
                        "SNAKE: Altered how?\n"
                        "SCHNEIDER: They sealed old passages with new work.\n"
                        "SCHNEIDER: Some of it was rushed. I never saw\n"
                        "SCHNEIDER: the prison plans, Snake.\n"
                        "SNAKE: Understood.");
        if (!(Flags & MG_FOX_RESCUED))
            return TEXT("SCHNEIDER: You found a way through?\n"
                        "SCHNEIDER: Keep your bearings. I never saw the\n"
                        "SCHNEIDER: prison plans. I can't map your route.");
        if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
            return TEXT("SCHNEIDER: If they've modified this area,\n"
                        "SCHNEIDER: the original structure might show it.\n"
                        "SCHNEIDER: I can't tell you where.");
        return TEXT("SCHNEIDER: Stay alert. I don't know this area.\n"
                    "SCHNEIDER: Use what the building tells you.");
    }

    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return TEXT("BIG BOSS: Snake, you're in a tight spot.\n"
                    "BIG BOSS: Check the wall. There may be a way through.\n"
                    "SNAKE: How do you know?\n"
                    "BIG BOSS: You're on site. See what you can find.");
    if (!(Flags & MG_FOX_RESCUED))
        return TEXT("BIG BOSS: Find Gray Fox. Don't lose sight of\n"
                    "BIG BOSS: the objective.");
    if ((Flags & MG_FOX_LEFT_PRISON) && !(Flags & MG_FOX_GEAR_RECOVERED))
        return TEXT("BIG BOSS: Recover the equipment they took.\n"
                    "BIG BOSS: Search the nearby storage doors.");
    if ((Flags & MG_FOX_GEAR_RECOVERED) && !(Flags & MG_FOX_TRANSMITTER_REMOVED))
        return TEXT("BIG BOSS: Inspect the gear you recovered.\n"
                    "BIG BOSS: They may have planted a transmitter.\n"
                    "SNAKE: A transmitter?");
    return TEXT("BIG BOSS: Continue the mission, Snake.\n"
                "BIG BOSS: Stay alert.");
}

bool AMGGrayFoxMission::HasIncomingTransceiverPrompt() const
{
    // Optional prompt, never an automatic/interruption-triggered radio call.
    return bPlayerInitialised && !bUnderAlert && (Flags & MG_FOX_CAPTURED) &&
           !(Flags & MG_FOX_FIRST_WALL_BREACHED) && GetHintLevel() >= 1;
}


void AMGGrayFoxMission::Tick(float Dt)
{
    Super::Tick(Dt);
    InitializePlayerIfReady();
    if (!bPlayerInitialised) return;
    ElapsedSinceBeat += FMath::Max(0.f, Dt);
    DialogueTimeRemaining = FMath::Max(0.f, DialogueTimeRemaining - Dt);
    UpdateZoneLighting(Dt);
    UpdateGuardAlert(Dt);
    AMGPlayerCharacter* Player =
        Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Player)
    {
        if (BreachRemaining >= 0.f)
        {
            BreachRemaining -= Dt;
            if (BreachRemaining <= 0.f)
            {
                BreachRemaining = -1.f;
                bServiceWallOpen = true;
                if (IsValid(ServiceWall)) ServiceWall->Destroy();
                if (IsValid(ChargeMarker)) ChargeMarker->Destroy();
                BroadcastStructuralImpact(FVector(5550.f,0.f,0.f), 1100.f, this);
                Dialogue = TEXT("Beyond the dust, a headset swings against a desk.");
                DialogueTimeRemaining = 3.f;
                SaveProgress(Player);
            }
        }
        if (bServiceWallOpen && !bKidDefeated && IsValid(MachinegunKid) && MachinegunKid->IsIncapacitated())
        {
            bKidDefeated = true;
            Dialogue = TEXT("The feed belt settles. A parachute hangs by the lift.");
            DialogueTimeRemaining = 4.f;
            SaveProgress(Player);
        }
        TickDispatch(Player, Dt);
        const FVector CurrentPosition = Player->GetActorLocation();
        const APlayerController* PC = Cast<APlayerController>(Player->GetController());
        const FRotator View = PC ? PC->GetControlRotation() : Player->GetActorRotation();
        // Count purposeful movement or camera scanning as engagement. No hint
        // is given just because a curious player spends 28 seconds exploring.
        const bool bExploring = bHasObservedPlayer &&
            (FVector::Dist2D(CurrentPosition, LastObservedPlayerLocation) > 2.5f ||
             FMath::Abs(FMath::FindDeltaAngleDegrees(View.Yaw, LastObservedViewRotation.Yaw)) > 0.35f ||
             FMath::Abs(View.Pitch - LastObservedViewRotation.Pitch) > 0.35f);
        SecondsSinceExploration = bExploring ? 0.f :
            SecondsSinceExploration + FMath::Max(0.f, Dt);
        LastObservedPlayerLocation = CurrentPosition;
        LastObservedViewRotation = View;
        bHasObservedPlayer = true;
    }
    // Crossing from the boss wing into the dog-guarded passage establishes a
    // new physical checkpoint, without fabricating MSX2 story flags or an
    // automatic cutscene. After a death, the original boss remains resolved.
    if (Player && (Flags & MG_FOX_SLICE_COMPLETED) &&
        !bPostCard3ServiceEntered && Player->GetActorLocation().X > 3850.f)
    {
        bPostCard3ServiceEntered = true;
        Dialogue = TEXT("SECURITY KENNEL AHEAD. COVER OR A LOW SERVICE ROUTE.");
        DialogueTimeRemaining = 4.2f;
        SaveProgress(Player);
    }
    if (Player && (Flags & MG_FOX_EXIT_WALL_BREACHED) &&
        !(Flags & MG_FOX_LEFT_PRISON) &&
        FVector::Dist2D(Player->GetActorLocation(), ExitPoint) < 190.f)
    {
        ApplyAction(MG_FOX_ESCAPE, Player);
    }
    if (Player && (Flags & MG_FOX_LEFT_PRISON)
        && !(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED)
        && Player->GetActorLocation().X > BossThreshold.X)
        ApplyAction(MG_FOX_ENTER_SHOTMAKER, Player);
    if (Player && IsTransmitterActive() && IsValid(Shotmaker)
        && (Flags & MG_FOX_SHOTMAKER_ENCOUNTERED)
        && (Flags & MG_FOX_GEAR_RECOVERED))
    {
        BeaconPulseTimer += FMath::Max(0.f, Dt);
        if (BeaconPulseTimer >= 3.f)
        {
            Shotmaker->ReceiveTrackerPing(Player->GetActorLocation());
            BeaconPulseTimer = 0.f;
        }
    }
    else BeaconPulseTimer = 0.f;
    if (Player && (Flags & MG_FOX_SHOTMAKER_ENCOUNTERED)
        && !(Flags & MG_FOX_SHOTMAKER_DEFEATED)
        && IsValid(Shotmaker) && Shotmaker->IsDefeated())
        ApplyAction(MG_FOX_DEFEAT_SHOTMAKER, Player);
    // A reliable once-per-beat signal for score, lighting and camera design;
    // no timeline lock or fabricated pressure if a player moves quickly.
    NotifyPacingPhaseIfChanged();
}

void AMGGrayFoxMission::RestartMission()
{
    UGameplayStatics::DeleteGameInSlot(SaveSlot, 0);
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}
