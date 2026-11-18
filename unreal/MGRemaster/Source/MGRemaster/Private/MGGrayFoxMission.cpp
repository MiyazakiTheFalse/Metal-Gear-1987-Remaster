#include "MGGrayFoxMission.h"

#include "MGGuardActor.h"
#include "MGGrayFoxSaveGame.h"
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
    UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(BlockMesh);
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
        if (bValid && (F & MG_FOX_CAPTURED))
        {
            Flags = F;
            SavedPosition = Save->CheckpointLocation;
            bHasLoadedSave = true;
        }
    }
    RestoreVisualState();
    InitializePlayerIfReady();
}

void AMGGrayFoxMission::RestoreVisualState()
{
    if ((Flags & MG_FOX_FIRST_WALL_BREACHED) && IsValid(WeakWall))
        WeakWall->Destroy();
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) && IsValid(EscapeWall))
        EscapeWall->Destroy();
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
    Save->MissionFlags = Flags;
    Save->CheckpointLocation = Player ? Player->GetActorLocation() : CellStart;
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
        Dialogue = TEXT("GRAY FOX: Metal Gear is real. Find the scientist behind it.");
        break;
    case MG_FOX_BREACH_EXIT_WALL:
        if (IsValid(EscapeWall)) EscapeWall->Destroy();
        Dialogue = TEXT("The outer passage is open. Stay low. Guards are nearby.");
        break;
    case MG_FOX_ESCAPE:
        Dialogue = TEXT("PRISON ESCAPED — Gray Fox rescued. Checkpoint secured.");
        break;
    default: break;
    }
    DialogueTimeRemaining = Action == MG_FOX_RESCUE ? 11.f : 5.f;
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
    if (bUnderAlert || Flags & MG_FOX_LEFT_PRISON) return 0;
#if MG_WITH_RUST
    MgGrayFoxPacing Report = {};
    if (mg_core_abi_version() == 1 &&
        mg_gray_fox_pacing(Flags, ElapsedSinceBeat, bUnderAlert ? 1u : 0u, &Report))
        return Report.hint_level;
#endif
    return ElapsedSinceBeat >= 35.f ? 2 : (ElapsedSinceBeat >= 14.f ? 1 : 0);
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
        return TEXT("Find a weakness in the cell wall. Face it and press E.");
    if (!(Flags & MG_FOX_RESCUED))
        return TEXT("Find Gray Fox in the adjacent cell. Face him and press E.");
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        return TEXT("Look for another weak wall and press E to breach it.");
    if (!(Flags & MG_FOX_LEFT_PRISON))
        return TEXT("Get past the guards and reach the far side of the corridor.");
    return TEXT("Gray Fox rescued. You have escaped the prison.");
}

FString AMGGrayFoxMission::GetPacingHint() const
{
    const uint32 Level = GetHintLevel();
    if (!Level) return FString();
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return Level == 1 ? TEXT("Look for a damaged section in the east wall.")
                          : TEXT("Approach the centre of the wall ahead, face it and press E.");
    if (!(Flags & MG_FOX_RESCUED))
        return Level == 1 ? TEXT("Someone is waiting further inside this room.")
                          : TEXT("The prisoner near the middle of the room is Gray Fox.");
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        return Level == 1 ? TEXT("The next barrier may have another weak section.")
                          : TEXT("Approach the centre of the east wall, face it and press E.");
    return Level == 1 ? TEXT("Keep low and use the obstacles to break line of sight.")
                      : TEXT("Move east to the end of the passage, beyond the guards.");
}

FString AMGGrayFoxMission::GetRecentDialogue() const
{
    return DialogueTimeRemaining > 0.f ? Dialogue : FString();
}

void AMGGrayFoxMission::Tick(float Dt)
{
    Super::Tick(Dt);
    InitializePlayerIfReady();
    if (!bPlayerInitialised) return;
    ElapsedSinceBeat += FMath::Max(0.f, Dt);
    DialogueTimeRemaining = FMath::Max(0.f, DialogueTimeRemaining - Dt);
    UpdateGuardAlert();
    AMGPlayerCharacter* Player =
        Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Player && (Flags & MG_FOX_EXIT_WALL_BREACHED) &&
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
