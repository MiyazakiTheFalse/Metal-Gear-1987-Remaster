#include "MGGrayFoxMission.h"

#include "MGGuardActor.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "MGShootGunnerActor.h"
#include "MGGrayFoxSaveGame.h"
#include "MGPlayerCharacter.h"
#include "MGEquipmentComponent.h"
#include "MGPrisonerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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
    SpawnBlock(TEXT("PreludeSecurityBooth"), FVector(-1920.f, 225.f, -15.f),
        FVector(2.6f, 1.8f, 2.1f), true);
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
            PatrolA->ConfigurePatrol(FVector(0.f, 1.f, 0.f), 130.f, 82.f);
        }
        if (AMGGuardActor* PatrolB = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(-2100.f, -285.f, -11.f), FRotator(0.f, 135.f, 0.f)))
        {
            PatrolB->SetRole(EMGGuardRole::Patrol);
            PatrolB->ConfigurePatrol(FVector(1.f, 0.f, 0.f), 145.f, 92.f);
        }
        CustodyOfficerA = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(-1570.f, 315.f, -11.f), FRotator(0.f, -90.f, 0.f));
        CustodyOfficerB = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(-1470.f, 170.f, -11.f), FRotator(0.f, -90.f, 0.f));
        if (IsValid(CustodyOfficerA))
            CustodyOfficerA->SetRole(EMGGuardRole::Custody);
        if (IsValid(CustodyOfficerB))
            CustodyOfficerB->SetRole(EMGGuardRole::Custody);
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
    SpawnBlock(TEXT("BossEastBoundary"), FVector(3750.f, 0.f, 90.f), FVector(.4f, 10.f, 3.8f), true);
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
    PlantedTransmitter = SpawnBlock(TEXT("PlantedTransmitter"), FVector(2790.f, -833.f, -30.f), FVector(.2f, .2f, .16f), false);
    if (IsValid(PlantedTransmitter)) PlantedTransmitter->SetActorHiddenInGame(true);

    SpawnBlock(TEXT("BossLowCoverA"), FVector(1960.f, 170.f, -25.f), FVector(2.1f, 1.f, 1.5f), true);
    SpawnBlock(TEXT("BossLowCoverB"), FVector(2610.f, -50.f, -25.f), FVector(1.1f, 1.6f, 1.5f), true);
    SpawnBlock(TEXT("BossLowCoverC"), FVector(3330.f, 230.f, -25.f), FVector(1.4f, .8f, 1.5f), true);
    ChapterExit = SpawnBlock(TEXT("ChapterExit"), FVector(3610.f, 0.f, 90.f), FVector(.45f, 2.2f, 3.8f), true);
    if (GetWorld())
        Shotmaker = GetWorld()->SpawnActor<AMGShootGunnerActor>(
            FVector(3100.f, 55.f, -11.f), FRotator(0.f, 180.f, 0.f));
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
    }
    if (AMGGuardActor* G = GetWorld()->SpawnActor<AMGGuardActor>(
            FVector(1120.f, -340.f, -11.f), FRotator(0.f, 180.f, 0.f)))
    {
        G->SetRole(EMGGuardRole::Prison);
        G->ConfigurePatrol(FVector(0.f, 1.f, 0.f), 170.f, 70.f);
    }
}

void AMGGrayFoxMission::BeginPlay()
{
    Super::BeginPlay();
    CreatePrelude();
    CreateGreybox();
    CreateBossWing();
    CreateZoneLighting();
    SpawnPerimeterGuards();
    if (const UMGGrayFoxSaveGame* Save = Cast<UMGGrayFoxSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlot, 0)))
    {
        // Reject unknown bits and structurally impossible checkpoints.
        const uint32 F = Save->MissionFlags;
        const bool bLegacy = Save->SchemaVersion == 1;
        const bool bRecognizedSchema = bLegacy || Save->SchemaVersion == 2 || Save->SchemaVersion == 3;
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
                ? ((F & MG_FOX_SHOTMAKER_ENCOUNTERED) ? 3650.f : 1310.f)
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
            && ((F & MG_FOX_CAPTURED) || (F & MG_FOX_DETENTION_INTELLIGENCE)))
        {
            Flags = F;
            SavedPosition = Save->CheckpointLocation;
            LoadedSchemaVersion = Save->SchemaVersion;
            LoadedHandgunRounds = Save->HandgunRounds;
            LoadedStunRounds = Save->StunRounds;
            bHasLoadedSave = true;
        }
    }
    RestoreVisualState();
    InitializePlayerIfReady();
}

void AMGGrayFoxMission::RestoreVisualState()
{
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

void AMGGrayFoxMission::InitializePlayerIfReady()
{
    if (bPlayerInitialised) return;
    AMGPlayerCharacter* Player =
        Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;
    Player->SetActorLocation(bHasLoadedSave ? SavedPosition : PreludeStart,
                             false, nullptr, ETeleportType::TeleportPhysics);
    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FRotator(-8.f, 0.f, 0.f));
    bPlayerInitialised = true;
    // The historic confiscation flag is retained for canon and save fidelity.
    // Gear usability depends on the distinct actual recovery transaction.
    if (UMGEquipmentComponent* Inventory = Player->GetEquipment())
    {
        if (bHasLoadedSave && LoadedSchemaVersion >= 3)
        {
            Inventory->RestoreHandgunRounds(LoadedHandgunRounds);
            Inventory->RestoreStunRounds(LoadedStunRounds);
        }
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
    Save->SchemaVersion = 3;
    Save->MissionFlags = Flags;
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
                Player->GetEquipment()->SetConfiscated(true);
            Player->SetActorLocation(CellStart, false, nullptr,
                ETeleportType::TeleportPhysics);
        }
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
        Dialogue = TEXT("GRAY FOX: Metal Gear is real. Find the scientist behind it.");
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
        Dialogue = TEXT("SHOOT GUNNER — THREE SPREAD SHOTS. FIND COVER.");
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

bool AMGGrayFoxMission::TryInteract(AMGPlayerCharacter* Player)
{
    if (!bPlayerInitialised || !Player) return false;
    if (!(Flags & MG_FOX_CAPTURED))
    {
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
        return false;
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
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) == 0
        && CanReach(Player, EscapeWall, 240.f))
    {
        Dialogue = TEXT("ANOTHER FRESH REPAIR. TRY A PHYSICAL BREACH.");
        DialogueTimeRemaining = 3.f;
        return true;
    }
    if (!(Flags & MG_FOX_SHOTMAKER_ENCOUNTERED)) return false;

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
    return false;
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
        return TEXT("Examine the detention cell for an escape route.");
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
    return TEXT("Outer Heaven chapter complete.");
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
