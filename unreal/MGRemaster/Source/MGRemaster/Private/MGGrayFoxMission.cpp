#include "MGGrayFoxMission.h"

#include "MGGuardActor.h"
#include "MGGrayFoxSaveGame.h"
#include "MGMissionOneSaveGame.h"
#include "MGEquipmentComponent.h"
#include "MGCampaignState.h"
#include "Engine/GameInstance.h"
#include "MGPlayerCharacter.h"
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
    const FVector ExitPoint(1150.f, 0.f, -11.f);
    const FString SaveSlot = TEXT("GrayFoxSliceV1");

    uint32 FallbackStep(uint32 Flags, uint32 Action)
    {
        // Mirror the Rust transaction, fail closed on skipped or repeated steps.
        const uint32 Known = 63u;
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
    if (Tag.ToString().StartsWith(TEXT("WeakWall_")))
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

void AMGGrayFoxMission::CreateGreybox()
{
    // Unreal's basic cube is 100cm at scale 1. Floor top is -100cm.
    // Three functional areas: prison cell, Fox cell, guarded escape corridor.
    SpawnBlock(TEXT("Floor"), FVector(75.f, 0.f, -150.f), FVector(26.5f, 10.f, 1.f), true);
    SpawnBlock(TEXT("NorthBoundary"), FVector(75.f, 500.f, 90.f), FVector(26.5f, 0.4f, 3.8f), true);
    SpawnBlock(TEXT("SouthBoundary"), FVector(75.f, -500.f, 90.f), FVector(26.5f, 0.4f, 3.8f), true);
    SpawnBlock(TEXT("WestBoundary"), FVector(-1250.f, 0.f, 90.f), FVector(0.4f, 10.f, 3.8f), true);
    SpawnBlock(TEXT("EastBoundary"), FVector(1400.f, 0.f, 90.f), FVector(0.4f, 10.f, 3.8f), true);

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
    FoxProxy = SpawnBlock(TEXT("GrayFoxProxy"), FVector(130.f, 0.f, -11.f),
                          FVector(0.5f, 0.5f, 1.78f), false);
    // Proxy is visible, not an actual authored character rig or animation.
    SpawnBlock(TEXT("Bed"), FVector(-900.f, 240.f, -62.f),
               FVector(1.8f, 0.9f, 0.75f), true);
    SpawnBlock(TEXT("Bed"), FVector(110.f, 240.f, -62.f),
               FVector(1.8f, 0.9f, 0.75f), true);
    SpawnBlock(TEXT("GuardCover"), FVector(950.f, 215.f, -40.f),
               FVector(1.8f, 0.7f, 1.2f), true);
    SpawnBlock(TEXT("GuardCover"), FVector(1200.f, -200.f, -40.f),
               FVector(1.8f, 0.7f, 1.2f), true);
    ExitMarker = SpawnBlock(TEXT("EscapeMarker"), ExitPoint, FVector(0.5f,0.5f,1.2f), false);
    GearLocker = SpawnBlock(TEXT("ConfiscatedGearLocker"),
        FVector(725.f, -220.f, -11.f), FVector(1.2f,.65f,1.3f), false);
}

void AMGGrayFoxMission::SpawnPerimeterGuards()
{
    // Existing hostile patrols outside confinement. No surprise wave on rescue.
    if (!GetWorld()) return;
    GetWorld()->SpawnActor<AMGGuardActor>(FVector(840.f, 360.f, -11.f),
                                           FRotator(0.f, 180.f, 0.f));
    GetWorld()->SpawnActor<AMGGuardActor>(FVector(1120.f, -340.f, -11.f),
                                           FRotator(0.f, 180.f, 0.f));
}

void AMGGrayFoxMission::BeginPlay()
{
    Super::BeginPlay();
    CreateGreybox();
    SpawnPerimeterGuards();
    if (const UMGGrayFoxSaveGame* Save = Cast<UMGGrayFoxSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlot, 0)))
    {
        // Reject unknown bits and structurally impossible checkpoints.
        const uint32 F = Save->MissionFlags;
        const bool bValid = (F & ~63u) == 0
            && (!(F & MG_FOX_CAPTURED) || (F & MG_FOX_CONFISCATED))
            && (!(F & MG_FOX_FIRST_WALL_BREACHED) || (F & MG_FOX_CAPTURED))
            && (!(F & MG_FOX_RESCUED) || (F & MG_FOX_FIRST_WALL_BREACHED))
            && (!(F & MG_FOX_EXIT_WALL_BREACHED) || (F & MG_FOX_RESCUED))
            && (!(F & MG_FOX_LEFT_PRISON) || (F & MG_FOX_EXIT_WALL_BREACHED));
        const FVector P = Save->CheckpointLocation;
        const bool bPositionSafe = !P.ContainsNaN()
            && P.X >= -1160.f && P.X <= 1310.f
            && FMath::Abs(P.Y) < 410.f
            && P.Z >= -95.f && P.Z <= 180.f
            // A stale save must not place the player beyond a solid, unopened wall.
            && ((F & MG_FOX_FIRST_WALL_BREACHED) || P.X < -350.f)
            && ((F & MG_FOX_EXIT_WALL_BREACHED) || P.X < 420.f);
        if (bValid && bPositionSafe && (Save->SchemaVersion == 1 || Save->SchemaVersion == 2)
            && (F & MG_FOX_CAPTURED))
        {
            Flags = F;
            SavedPosition = Save->CheckpointLocation;
            bHasLoadedSave = true;
            bGearRecovered = Save->SchemaVersion >= 2 && Save->bGearRecovered;
        }
    }
    RestoreVisualState();
    InitializePlayerIfReady();
    // Recover from a save taken after the exit flag was committed but before
    // the return transition. The narrative transaction must not repeat.
    if ((Flags & MG_FOX_LEFT_PRISON) && GetWorld() && GetWorld()->GetGameInstance())
    {
        if (UMGCampaignState* Story =
            GetWorld()->GetGameInstance()->GetSubsystem<UMGCampaignState>())
        {
            Story->RecordBeat(TEXT("fox.revelation"),
                TEXT("GRAY FOX: Metal Gear is nuclear-capable. Find Dr. Pettrovich."));
            if (bGearRecovered)
                Story->RecordBeat(TEXT("snake.gear_recovery"),
                    TEXT("GEAR RECLAIMED: Confiscated equipment was recovered."));
            Story->RecordBeat(TEXT("fox.intel_persists"),
                TEXT("PRISON ESCAPED: Gray Fox's report remains in the mission log."));
            Story->AdvanceMission(3);
        }
        ReturnToOuterHeavenSeconds = 0.1f;
    }
}

void AMGGrayFoxMission::RestoreVisualState()
{
    if ((Flags & MG_FOX_FIRST_WALL_BREACHED) && IsValid(WeakWall))
        WeakWall->Destroy();
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) && IsValid(EscapeWall))
        EscapeWall->Destroy();
    if (bGearRecovered && IsValid(GearLocker))
        GearLocker->Destroy();
}

void AMGGrayFoxMission::InitializePlayerIfReady()
{
    if (bPlayerInitialised) return;
    AMGPlayerCharacter* Player =
        Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;
    Player->SetActorLocation(bHasLoadedSave ? SavedPosition : CellStart,
                             false, nullptr, ETeleportType::TeleportPhysics);
    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FRotator(-8.f, 0.f, 0.f));
    bPlayerInitialised = true;
    if (bGearRecovered)
        RestoreCapturedGear(Player);

    if (!bHasLoadedSave)
    {
        ApplyAction(MG_FOX_CAPTURE, Player);
        Dialogue = TEXT("Equipment confiscated. Find a way out.");
        DialogueTimeRemaining = 6.f;
    }
    else
    {
        Dialogue = TEXT("Prison checkpoint restored.");
        DialogueTimeRemaining = 3.f;
    }
}

void AMGGrayFoxMission::SaveProgress(AMGPlayerCharacter* Player)
{
    UMGGrayFoxSaveGame* Save = Cast<UMGGrayFoxSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UMGGrayFoxSaveGame::StaticClass()));
    if (!Save) return;
    Save->SchemaVersion = 2;
    Save->bGearRecovered = bGearRecovered;
    Save->MissionFlags = Flags;
    Save->CheckpointLocation = Player ? Player->GetActorLocation() : CellStart;
    if (!UGameplayStatics::SaveGameToSlot(Save, SaveSlot, 0))
        UE_LOG(LogTemp, Warning, TEXT("Gray Fox: could not save mission checkpoint"));
}

void AMGGrayFoxMission::PersistRecoveredInventory(AMGPlayerCharacter* Player)
{
    if (!bGearRecovered || !Player || !Player->GetEquipment()) return;
    UMGMissionOneSaveGame* OpeningSave = Cast<UMGMissionOneSaveGame>(
        UGameplayStatics::LoadGameFromSlot(TEXT("MissionOneV1"), 0));
    if (!OpeningSave) return;
    OpeningSave->Version = FMath::Max(4, OpeningSave->Version);
    OpeningSave->HandgunRounds = Player->GetEquipment()->GetHandgunRounds();
    OpeningSave->RationsRemaining = Player->GetEquipment()->GetItemQuantity(TEXT("RATION"));
    if (!UGameplayStatics::SaveGameToSlot(OpeningSave, TEXT("MissionOneV1"), 0))
        UE_LOG(LogTemp, Warning, TEXT("Prison gear usage was not persisted to Building One"));
}

void AMGGrayFoxMission::RestoreCapturedGear(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment()) return;
    UMGEquipmentComponent* Equipment = Player->GetEquipment();
    Equipment->InitialiseMissionOneLoadout();
    const UMGMissionOneSaveGame* OpeningSave = Cast<UMGMissionOneSaveGame>(
        UGameplayStatics::LoadGameFromSlot(TEXT("MissionOneV1"), 0));
    if (!OpeningSave) return;
    for (const FName Id : OpeningSave->CollectedPickupIds)
    {
        if (Id == TEXT("P01")) Equipment->AwardItem(TEXT("RATION"), 1);
        else if (Id == TEXT("P02")) Equipment->AwardItem(TEXT("CARD1"), 1);
        else if (Id == TEXT("P03")) Equipment->AwardItem(TEXT("BINOCULARS"), 1);
        else if (Id == TEXT("P04")) Equipment->AwardItem(TEXT("GASMASK"), 1);
        else if (Id == TEXT("P05")) Equipment->AwardWeapon(TEXT("HANDGUN"), 0);
        else if (Id == TEXT("P06")) Equipment->AwardItem(TEXT("MINES"), 1);
        else if (Id == TEXT("P07")) Equipment->AwardAmmunition(TEXT("HANDGUN"), 12);
    }
    if (OpeningSave->Version >= 4)
    {
        Equipment->RestoreHandgunRounds(FMath::Max(0, OpeningSave->HandgunRounds));
        Equipment->RestoreItemQuantity(TEXT("RATION"), FMath::Max(0, OpeningSave->RationsRemaining));
    }
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
    SaveProgress(Player);

    switch (Action)
    {
    case MG_FOX_CAPTURE:
        Dialogue = TEXT("You are unarmed. Observe the cell and look for an exit.");
        break;
    case MG_FOX_BREACH_FIRST_WALL:
        if (IsValid(WeakWall)) WeakWall->Destroy();
        Dialogue = TEXT("The weakened wall gives way. Someone is in the adjacent cell.");
        break;
    case MG_FOX_RESCUE:
        // Placeholder original prose, not a published game-script transcription.
        Dialogue = TEXT("GRAY FOX: Metal Gear is nuclear-armed. Find Pettrovich.");
        // The recorded objective does not depend on watching this dialogue.
        // Fox remains a vulnerable captive, not an all-knowing AI companion.
        break;
    case MG_FOX_BREACH_EXIT_WALL:
        if (IsValid(EscapeWall)) EscapeWall->Destroy();
        Dialogue = TEXT("The outer passage is open. Stay low. Guards are nearby.");
        break;
    case MG_FOX_ESCAPE:
        Dialogue = TEXT("GRAY FOX RESCUED — FIND THE SCIENTIST");
        break;
    default: break;
    }
    // Commit narrative knowledge at the same moment as the durable gameplay
    // flag, not when text/voice playback happens to finish.
    if (GetWorld() && GetWorld()->GetGameInstance())
    {
        if (UMGCampaignState* Story =
            GetWorld()->GetGameInstance()->GetSubsystem<UMGCampaignState>())
        {
            if (Action == MG_FOX_CAPTURE)
                Story->RecordBeat(TEXT("snake.capture"),
                    TEXT("CAPTURED: Snake enters the secret detention wing without his equipment."));
            else if (Action == MG_FOX_RESCUE)
            {
                Story->RecordBeat(TEXT("snake.rescue_fox"),
                    TEXT("GRAY FOX RESCUED: The missing FOXHOUND operative is alive."));
                Story->RecordBeat(TEXT("fox.revelation"),
                    TEXT("GRAY FOX: Metal Gear is a nuclear-armed walking weapon. Find Dr. Pettrovich."));
                Story->RecordBeat(TEXT("madnar.named"),
                    TEXT("NEW LEAD: Dr. Pettrovich, designer of Metal Gear."));
            }
            else if (Action == MG_FOX_ESCAPE)
            {
                Story->RecordBeat(TEXT("fox.intel_persists"),
                    TEXT("PRISON ESCAPED: Gray Fox's report remains in the mission log."));
                if (Story->AdvanceChapter(2))
                    ReturnToOuterHeavenSeconds = 1.35f;
            }
        }
    }
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
    if ((Flags & MG_FOX_FIRST_WALL_BREACHED) == 0
        && CanReach(Player, WeakWall, 240.f))
        return ApplyAction(MG_FOX_BREACH_FIRST_WALL, Player);
    if ((Flags & MG_FOX_RESCUED) == 0
        && CanReach(Player, FoxProxy, 210.f))
        return ApplyAction(MG_FOX_RESCUE, Player);
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) == 0
        && CanReach(Player, EscapeWall, 240.f))
        return ApplyAction(MG_FOX_BREACH_EXIT_WALL, Player);
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) && !bGearRecovered &&
        CanReach(Player, GearLocker, 220.f))
    {
        bGearRecovered = true;
        RestoreCapturedGear(Player);
        if (IsValid(GearLocker)) GearLocker->Destroy();
        SaveProgress(Player);
        if (GetWorld() && GetWorld()->GetGameInstance())
            if (UMGCampaignState* Story =
                GetWorld()->GetGameInstance()->GetSubsystem<UMGCampaignState>())
                Story->RecordBeat(TEXT("snake.gear_recovery"),
                    TEXT("EQUIPMENT RECOVERED: The prison locker held Snake's confiscated supplies."));
        Dialogue = TEXT("GEAR RECLAIMED — CHECK EQUIPMENT BEFORE ESCAPING");
        DialogueTimeRemaining = 4.5f;
        return true;
    }
    return false;
}

bool AMGGrayFoxMission::HasEquipmentConfiscated() const
{
    return (Flags & MG_FOX_CONFISCATED) != 0;
}

void AMGGrayFoxMission::UpdateGuardAlert()
{
    bUnderAlert = false;
    if (!GetWorld()) return;
    for (TActorIterator<AMGGuardActor> It(GetWorld()); It; ++It)
        if (It->GetAwarenessState() >= 3 && !It->IsIncapacitated())
        {
            bUnderAlert = true;
            break;
        }
}

uint32 AMGGrayFoxMission::GetPacingPhase() const
{
#if MG_WITH_RUST
    MgGrayFoxPacing Report = {};
    if (mg_core_abi_version() == 1 &&
        mg_gray_fox_pacing(Flags, ElapsedSinceBeat, bUnderAlert ? 1u : 0u, &Report))
        return Report.phase;
#endif
    if (!(Flags & MG_FOX_CAPTURED)) return 0;
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED)) return 1;
    if (!(Flags & MG_FOX_RESCUED)) return 2;
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED)) return 3;
    if (!(Flags & MG_FOX_LEFT_PRISON)) return 4;
    return 5;
}

uint32 AMGGrayFoxMission::GetHintLevel() const
{
    if (bUnderAlert || (Flags & MG_FOX_LEFT_PRISON)) return 0;
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
    case 0: return TEXT("CAPTURE");
    case 1: return TEXT("CELL DISCOVERY");
    case 2: return TEXT("BREAKTHROUGH");
    case 3: return TEXT("REVELATION");
    case 4: return TEXT("ESCAPE");
    default: return TEXT("RECOVERY");
    }
}

FString AMGGrayFoxMission::GetObjectiveText() const
{
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return TEXT("Search your cell for a way out.");
    if (!(Flags & MG_FOX_RESCUED))
        return TEXT("Find Gray Fox in the adjacent cell. Face him and press E.");
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        return TEXT("Find a route into the outer passage.");
    if (!(Flags & MG_FOX_LEFT_PRISON))
        return bGearRecovered ?
            TEXT("Get past the guards and reach the far side of the corridor.") :
            TEXT("Recover confiscated equipment from the corridor locker.");
    return TEXT("Gray Fox rescued. You have escaped the prison.");
}

FString AMGGrayFoxMission::GetPacingHint() const
{
    const uint32 Level = GetHintLevel();
    if (!Level) return FString();
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return Level == 1 ? TEXT("The concrete has not all aged the same way.")
                          : TEXT("Look for an altered join in the wall. Examine it up close.");
    if (!(Flags & MG_FOX_RESCUED))
        return Level == 1 ? TEXT("Someone is waiting further inside this room.")
                          : TEXT("The prisoner near the middle of the room is Gray Fox.");
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        return Level == 1 ? TEXT("Old work and new repairs might meet again.")
                          : TEXT("Investigate the unusual section of the next barrier.");
    if (!bGearRecovered)
        return Level == 1 ? TEXT("Equipment may be stored near the guard route.")
                          : TEXT("Inspect the marked locker along the south wall.");
    return Level == 1 ? TEXT("Keep low and use the obstacles to break line of sight.")
                      : TEXT("Move east to the end of the passage, beyond the guards.");
}

FString AMGGrayFoxMission::GetMissionLogText() const
{
    if (!(Flags & MG_FOX_RESCUED)) return FString();
    // Temporary paraphrase; not a verbatim line from the licensed J2ME script.
    return bGearRecovered ?
        TEXT("GRAY FOX INTEL: Metal Gear is real. Seek its scientist. Equipment recovered.") :
        TEXT("GRAY FOX INTEL: Metal Gear is real. Seek its scientist. Recover equipment.");
}

FString AMGGrayFoxMission::GetRecentDialogue() const
{
    return DialogueTimeRemaining > 0.f ? Dialogue : FString();
}


FString AMGGrayFoxMission::GetTransceiverDialogue(int32 Channel) const
{
    // Remake-authored dialogue; Diane can be reached if her contact was
    // learned from the rescued POW, but she has no magical prison blueprint.
    if (Channel == 2)
        return TEXT("DIANE: I heard one of your people may be inside.\n"
                    "DIANE: I don't know this detention wing.\n"
                    "SNAKE: Then I'll have to find him myself.\n"
                    "DIANE: Be careful.");
    // Schneider knows architecture, not this prison layout.
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
    if (ReturnToOuterHeavenSeconds >= 0.f)
    {
        ReturnToOuterHeavenSeconds -= FMath::Max(0.f, Dt);
        if (ReturnToOuterHeavenSeconds <= 0.f)
            UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
        return;
    }
    ElapsedSinceBeat += FMath::Max(0.f, Dt);
    DialogueTimeRemaining = FMath::Max(0.f, DialogueTimeRemaining - Dt);
    UpdateGuardAlert();
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
    if (Player && bGearRecovered && (Flags & MG_FOX_EXIT_WALL_BREACHED) &&
        !(Flags & MG_FOX_LEFT_PRISON) &&
        FVector::Dist2D(Player->GetActorLocation(), ExitPoint) < 190.f)
    {
        ApplyAction(MG_FOX_ESCAPE, Player);
    }
}

void AMGGrayFoxMission::RestartMission()
{
    UGameplayStatics::DeleteGameInSlot(SaveSlot, 0);
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}
