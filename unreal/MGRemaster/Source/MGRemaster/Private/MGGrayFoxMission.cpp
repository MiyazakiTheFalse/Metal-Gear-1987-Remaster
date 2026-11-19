#include "MGGrayFoxMission.h"

#include "MGGuardActor.h"
#include "MGEquipmentComponent.h"
#include "MGPrisonAudio.h"
#include "mg_fox_steps.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
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

    const FVector ApproachStart(-2750.f, 0.f, -11.f);
    const FVector LastDoor(3050.f, 0.f, -11.f);
    const TCHAR* FoxLines[] = {
        TEXT("GRAY FOX: Metal Gear is a walking battle tank."),
        TEXT("GRAY FOX: It can launch a nuclear missile. They mean to use it."),
        TEXT("GRAY FOX: Find Dr. Pettrovich. He designed it. He knows how to stop it.")
    };

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
    SpawnBlock(TEXT("NorthBoundary"), FVector(-775,500,90), FVector(9.5,.4,3.8), true);
    SpawnBlock(TEXT("NorthBoundary"), FVector(-195,500,90), FVector(2.1,.4,3.8), true);
    SpawnBlock(TEXT("NorthBoundary"), FVector(515,500,90), FVector(7.1,.4,3.8), true);
    SpawnBlock(TEXT("NorthBoundary"), FVector(1290,500,90), FVector(2.2,.4,3.8), true);
    SpawnBlock(TEXT("ServiceFloor"), FVector(550,750,-150), FVector(17,5,1), true);
    SpawnBlock(TEXT("ServiceBoundary"), FVector(550,1000,90), FVector(17,.4,3.8), true);
    for (float X : {-300.f,1400.f})
        SpawnBlock(TEXT("ServiceBoundary"), FVector(X,750,90), FVector(.4,5,3.8), true);
    SpawnBlock(TEXT("SouthBoundary"), FVector(75.f, -500.f, 90.f), FVector(26.5f, 0.4f, 3.8f), true);
    SpawnBlock(TEXT("WestBoundary"), FVector(-1250.f, 0.f, 90.f), FVector(0.4f, 10.f, 3.8f), true);
    for (float Y : {-325.f,325.f})
    {
        SpawnBlock(TEXT("Divider"), FVector(-300,Y,90), FVector(.4,3.6,3.8), true);
        SpawnBlock(TEXT("SecurityBoundary"), FVector(1400,Y,90), FVector(.4,3.6,3.8), true);
    }
    SpawnBlock(TEXT("Divider"), FVector(470,0,90), FVector(.4,10,3.8), true);
    WeakWall = SpawnBlock(TEXT("WeakWall_A"), FVector(-300.f, 0.f, 90.f),
                          FVector(0.4f, 2.9f, 3.8f), true);
    EscapeWall = SpawnBlock(TEXT("WeakWall_B"), FVector(35.f, 500.f, 90.f),
                            FVector(2.5f, 0.4f, 3.8f), true);
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
    // Entry yard and eastern property room use the same floor elevation as the prison.
    for (float X : {-2150.f, 2300.f})
    {
        SpawnBlock(TEXT("Floor"), FVector(X,0,-150), FVector(18,10,1), true);
        for (float Y : {-500.f,500.f})
            SpawnBlock(TEXT("Boundary"), FVector(X,Y,90), FVector(18,.4,3.8), true);
    }
    SpawnBlock(TEXT("Boundary"), FVector(-3050,0,90), FVector(.4,10,3.8), true);
    SpawnBlock(TEXT("Boundary"), FVector(3200,0,90), FVector(.4,10,3.8), true);
    Informant = SpawnBlock(TEXT("Prisoner"), FVector(-2540,90,-26), FVector(.52,.55,1.48), false);
    Sentry = SpawnBlock(TEXT("ArrestPost"), FVector(-1460,0,-11), FVector(.65,.55,1.78), true);
    SpawnBlock(TEXT("ProcessingCounter"), FVector(-1500,96,-50), FVector(.9,1.2,.95), true);
    SpawnBlock(TEXT("YardCover"), FVector(-2010,80,-25), FVector(1.1,2,1.5), true);
    Gate = SpawnBlock(TEXT("PropertyDoor"), FVector(1400,0,90), FVector(.4,2.9,3.8), true);
    Locker = SpawnBlock(TEXT("PropertyLocker"), FVector(1610,-260,-12), FVector(.7,1.4,1.76), true);
    for (float Y : {-315.f,315.f})
        SpawnBlock(TEXT("PropertyBoundary"), FVector(1930,Y,90), FVector(.5,3.7,3.8), true);
    ArmouryGate = SpawnBlock(TEXT("PropertyShutter"), FVector(1930,0,90), FVector(.5,2.6,3.8), true);
    SpawnBlock(TEXT("ArenaCover"), FVector(2250,70,-25), FVector(1,1.8,1.5), true);
    SpawnBlock(TEXT("ArenaCover"), FVector(2510,-245,-25), FVector(.8,1.5,1.5), true);
    FinalGate = SpawnBlock(TEXT("ExitShutter"), FVector(2870,0,90), FVector(.45,3,3.8), true);
    for (float Y : {-325.f,325.f})
        SpawnBlock(TEXT("ExitFrame"), FVector(2870,Y,90), FVector(.45,3.5,3.8), true);
    for (AStaticMeshActor* Figure : {Informant, FoxProxy, Sentry})
    {
        if (!Figure) continue;
        const FVector P = Figure->GetActorLocation();
        Figure->SetActorScale3D(FVector(.43,.52,.72));
        SpawnBlock(TEXT("Head"), P + FVector(1,0,51), FVector(.30,.31,.35), false);
        for (float Side : {-1.f,1.f})
        {
            SpawnBlock(TEXT("Sleeve"), P + FVector(0,Side*31,-2), FVector(.18,.19,.53), false);
            SpawnBlock(TEXT("Leg"), P + FVector(0,Side*13,-56), FVector(.22,.21,.64), false);
        }
    }
    AddLight(FVector(-2440,60,210), FLinearColor(1,.73,.46), 900);
    AddLight(FVector(-790,-110,175), FLinearColor(.58,.73,.9), 850);
    AddLight(FVector(190,140,180), FLinearColor(1,.73,.51), 680);
    AddLight(FVector(460,840,195), FLinearColor(.58,.75,.87), 980);
    AddLight(FVector(1590,-180,170), FLinearColor(1,.76,.48), 700);
    AddLight(FVector(2610,120,225), FLinearColor(.74,.83,1), 1150);
    AddLight(FVector(3070,0,170), FLinearColor(1,.81,.55), 440);

}

void AMGGrayFoxMission::SpawnPerimeterGuards()
{
    // Existing hostile patrols outside confinement. No surprise wave on rescue.
    if (!GetWorld()) return;
    GetWorld()->SpawnActor<AMGGuardActor>(FVector(-1890.f,-265.f,-11.f), FRotator(0,180,0));
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
    if (UMGGrayFoxSaveGame* Save = Cast<UMGGrayFoxSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlot, 0)))
    {
        const bool Legacy = Save->SchemaVersion == 1 && (Save->MissionFlags & MG_FOX_CAPTURED)
            && !(Save->MissionFlags & ~63u);
        const uint32 Candidate = Save->MissionFlags | (Legacy ? MG_FOX_INTEL : 0u);
        if ((Legacy || Save->SchemaVersion == 2) && mg_fox_valid_prefix(Candidate))
        {
            Flags = Candidate;
            // V1 used an east-facing second breach. Migrate to a safe landmark.
            SavedPosition = Legacy ? ((Flags & MG_FOX_LEFT_PRISON) ? ExitPoint :
                ((Flags & MG_FOX_RESCUED) ? FVector(100,0,-11) : CellStart)) : Save->CheckpointLocation;
            const bool InYard = SavedPosition.X > -3050 && SavedPosition.X < -1250 &&
                FMath::Abs(SavedPosition.Y) < 500;
            const bool InPrison = SavedPosition.X > -1250 && SavedPosition.X < 3200 &&
                FMath::Abs(SavedPosition.Y) < 500;
            const bool InPassage = SavedPosition.X > -300 && SavedPosition.X < 1400 &&
                SavedPosition.Y >= 500 && SavedPosition.Y < 1000;
            const bool ThroughFirst = Flags & MG_FOX_FIRST_WALL_BREACHED;
            const bool ThroughSecond = Flags & MG_FOX_EXIT_WALL_BREACHED;
            bHasLoadedSave = !SavedPosition.ContainsNaN() && SavedPosition.Z >= -100 &&
                SavedPosition.Z < 280 && ((Flags & MG_FOX_CAPTURED) ?
                ((InPrison || (InPassage && ThroughSecond)) &&
                 (ThroughFirst || SavedPosition.X < -300) &&
                 (ThroughSecond || SavedPosition.X < 470) &&
                 ((Flags & MG_FOX_LEFT_PRISON) || SavedPosition.X < 1400) &&
                 ((Flags & MG_FOX_RECOVERED) || SavedPosition.X < 1930) &&
                 ((Flags & MG_FOX_SHOTMAKER_DOWN) || SavedPosition.X < 2870)) : InYard);
            if (bHasLoadedSave)
            {
                Loaded = Save;
                FirstHits = FMath::Min<uint8>(Save->FirstHits, 2);
                ExitHits = FMath::Min<uint8>(Save->ExitHits, 2);
            }
            else Flags = 0;
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
    if ((Flags & MG_FOX_LEFT_PRISON) && IsValid(Gate))
    {
        Gate->SetActorEnableCollision(false);
        Gate->SetActorHiddenInGame(true);
    }
    if ((Flags & MG_FOX_RECOVERED) && IsValid(ArmouryGate))
    {
        ArmouryGate->SetActorHiddenInGame(true);
        ArmouryGate->SetActorEnableCollision(false);
        Locker->SetActorRotation(FRotator(0,18,0));
    }
    if ((Flags & MG_FOX_SHOTMAKER_DOWN) && IsValid(FinalGate))
    {
        FinalGate->SetActorEnableCollision(false);
        FinalGate->SetActorHiddenInGame(true);
    }
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        for (uint8 I = 1; I <= FirstHits; ++I) ShowScar(WeakWall, I);
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        for (uint8 I = 1; I <= ExitHits; ++I) ShowScar(EscapeWall, I);

}

void AMGGrayFoxMission::InitializePlayerIfReady()
{
    if (bPlayerInitialised) return;
    AMGPlayerCharacter* Player =
        Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player || !Player->GetEquipment() || !Player->GetEquipment()->HasBegunPlay()) return;
    Player->SetActorLocation(bHasLoadedSave ? SavedPosition : ApproachStart,
                             false, nullptr, ETeleportType::TeleportPhysics);
    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
        PC->SetControlRotation(FRotator(-8.f, 0.f, 0.f));
    bPlayerInitialised = true;

    if (HasEquipmentConfiscated()) Player->TuneCellReceiver();
    if (Loaded) Player->GetEquipment()->RestoreLoadout(*Loaded);
    Player->GetEquipment()->SetConfiscated(HasEquipmentConfiscated());
    if (!bHasLoadedSave && FParse::Param(FCommandLine::Get(), TEXT("GrayFoxCellStart")))
    {
        ApplyAction(MG_FOX_CONTACT, Player);
        ApplyAction(MG_FOX_CAPTURE, Player);
    }
    if (bHasLoadedSave)
    {
        Dialogue = TEXT("Checkpoint restored.");
        DialogueTimeRemaining = 2.f;
    }
}

void AMGGrayFoxMission::SaveProgress(AMGPlayerCharacter* Player)
{
    UMGGrayFoxSaveGame* Save = Cast<UMGGrayFoxSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UMGGrayFoxSaveGame::StaticClass()));
    if (!Save) return;
    Save->SchemaVersion = 2;
    Save->FirstHits = FirstHits;
    Save->ExitHits = ExitHits;
    if (Player && Player->GetEquipment())
    {
        const auto* Equipment = Player->GetEquipment();
        Save->Weapons = Equipment->GetWeapons();
        Save->Items = Equipment->GetItems();
        Save->EquippedWeapon = Equipment->GetEquippedWeaponIndex();
        Save->EquippedItem = Equipment->GetEquippedItemIndex();
    }
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
    Next = mg_fox_fallback_step(Flags, Action);
    if (Next == Flags) return false;
#endif
    UE_LOG(LogTemp, Display, TEXT("GrayFox beat=%u previous=%u elapsed=%.2f"), Action, Flags, ElapsedSinceBeat);
    Flags = Next;
    if (Action == MG_FOX_CAPTURE)
    {
        Player->TuneCellReceiver();
        Player->LeaveCover();
        Player->SetActorLocation(CellStart, false, nullptr, ETeleportType::TeleportPhysics);
        if (auto* Controller = Player->GetController()) Controller->SetControlRotation(FRotator(-6,0,0));
    }
    Player->GetEquipment()->SetConfiscated(HasEquipmentConfiscated());
    ElapsedSinceBeat = 0.f;
    SecondsSinceExploration = 0.f;
    bHasObservedPlayer = false;
    SaveProgress(Player);

    switch (Action)
    {
    case MG_FOX_CONTACT:
        Dialogue = TEXT("PRISONER: Fox is in solitary. The guards at the east post take prisoners there.");
        break;
    case MG_FOX_CAPTURE:
        MGPrisonAudio::Play(this, CellStart, EMGPrisonCue::Latch);
        Dialogue = TEXT("You are unarmed. Observe the cell and look for an exit.");
        break;
    case MG_FOX_BREACH_FIRST_WALL:
        if (IsValid(WeakWall)) WeakWall->Destroy();
        Dialogue = TEXT("The weakened wall gives way. Someone is in the adjacent cell.");
        break;
    case MG_FOX_RESCUE:
        // Placeholder original prose, not a published game-script transcription.
        FoxSpeech = 0.f;
        Dialogue = FoxLines[0];
        break;
    case MG_FOX_BREACH_EXIT_WALL:
        if (IsValid(EscapeWall)) EscapeWall->Destroy();
        Dialogue = TEXT("The outer passage is open. Stay low. Guards are nearby.");
        break;
    case MG_FOX_ESCAPE:
        Gate->SetActorHiddenInGame(true);
        Gate->SetActorEnableCollision(false);
        Dialogue = TEXT("Property office ahead. Your equipment is inside.");
        break;
    case MG_FOX_RECLAIM:
        ArmouryGate->SetActorEnableCollision(false);
        ArmouryGate->SetActorHiddenInGame(true);
        Locker->SetActorRotation(FRotator(0,18,0));
        MGPrisonAudio::Play(this, Locker->GetActorLocation(), EMGPrisonCue::Latch);
        Dialogue = TEXT("Equipment recovered. Check your weapon selection.");
        break;
    case MG_FOX_DEFEAT:
        FinalGate->SetActorEnableCollision(false);
        FinalGate->SetActorHiddenInGame(true);
        MGPrisonAudio::Play(this, FinalGate->GetActorLocation(), EMGPrisonCue::Latch);
        Dialogue = TEXT("The shutter is open.");
        break;
    case MG_FOX_DEPART:
        Dialogue = TEXT("Checkpoint secured. Next objective: locate Dr. Pettrovich.");
        break;
    default: break;
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
    if (FMath::Abs(Delta.Z) > 220.f || Delta.Size2D() > MaxDistanceCm) return false;

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
        if (!(Flags & MG_FOX_INTEL) && CanReach(Player, Informant, 220.f))
            return ApplyAction(MG_FOX_CONTACT, Player);
        if ((Flags & MG_FOX_INTEL) && CanReach(Player, Sentry, 210.f) && !Player->IsAiming())
            return ApplyAction(MG_FOX_CAPTURE, Player);
        return false;
    }
    if ((Flags & MG_FOX_LEFT_PRISON) && !(Flags & MG_FOX_RECOVERED) && CanReach(Player, Locker, 210.f))
        return ApplyAction(MG_FOX_RECLAIM, Player);
    if ((Flags & MG_FOX_FIRST_WALL_BREACHED) == 0
        && CanReach(Player, WeakWall, 240.f))
        return StrikeWall(Player, WeakWall, FirstHits, MG_FOX_BREACH_FIRST_WALL);
    if ((Flags & MG_FOX_RESCUED) == 0
        && CanReach(Player, FoxProxy, 210.f))
        return ApplyAction(MG_FOX_RESCUE, Player);
    if ((Flags & MG_FOX_EXIT_WALL_BREACHED) == 0
        && CanReach(Player, EscapeWall, 240.f))
        return (Flags & MG_FOX_RESCUED) && StrikeWall(Player, EscapeWall, ExitHits, MG_FOX_BREACH_EXIT_WALL);
    return false;
}

bool AMGGrayFoxMission::HasEquipmentConfiscated() const
{
    return (Flags & MG_FOX_CONFISCATED) && !(Flags & MG_FOX_RECOVERED);
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
    if (!(Flags & MG_FOX_CAPTURED) || bUnderAlert || (Flags & MG_FOX_LEFT_PRISON)) return 0;
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
    if (!(Flags & MG_FOX_CAPTURED)) return (Flags & MG_FOX_INTEL) ? TEXT("APPROACH") : TEXT("INFILTRATION");
    if (Flags & MG_FOX_COMPLETE) return TEXT("SLICE COMPLETE");
    if (Flags & MG_FOX_SHOTMAKER_DOWN) return TEXT("RESOLUTION");
    if (Flags & MG_FOX_RECOVERED) return TEXT("SHOOT GUNNER");
    if (Flags & MG_FOX_LEFT_PRISON) return TEXT("PROPERTY OFFICE");
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
    if (!(Flags & MG_FOX_INTEL)) return TEXT("Speak to the prisoner beside the yard light. E / X");
    if (!(Flags & MG_FOX_CAPTURED)) return TEXT("Reach the east guard post. Lower your weapon and press E / X to surrender.");
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return TEXT("Search your cell for a way out.");
    if (!(Flags & MG_FOX_RESCUED))
        return TEXT("Find Gray Fox in the adjacent cell. Face him and press E.");
    if (!(Flags & MG_FOX_EXIT_WALL_BREACHED))
        return TEXT("Find a route into the outer passage.");
    if (!(Flags & MG_FOX_LEFT_PRISON))
        return TEXT("Get past the guards and reach the far side of the corridor.");
    if (!(Flags & MG_FOX_RECOVERED)) return TEXT("Retrieve your equipment from the property locker. E / X");
    if (!(Flags & MG_FOX_SHOTMAKER_DOWN)) return TEXT("Get past Shoot Gunner. Cover breaks his line of fire.");
    if (!(Flags & MG_FOX_COMPLETE)) return TEXT("Leave through the open shutter.");
    return TEXT("Infiltration continues. Locate Dr. Pettrovich.");
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
    return Level == 1 ? TEXT("Keep low and use the obstacles to break line of sight.")
                      : TEXT("Move east to the end of the passage, beyond the guards.");
}

FString AMGGrayFoxMission::GetMissionLogText() const
{
    if (!(Flags & MG_FOX_RESCUED)) return FString();
    // Temporary paraphrase; not a verbatim line from the licensed J2ME script.
    return FString(FoxLines[0]) + TEXT("\n") + FoxLines[1] + TEXT("\n") + FoxLines[2];
}

FString AMGGrayFoxMission::GetRecentDialogue() const
{
    if (FoxSpeech >= 0.f && FoxSpeech < 11.f)
        return FoxLines[FoxSpeech < 3.f ? 0 : (FoxSpeech < 7.f ? 1 : 2)];
    return DialogueTimeRemaining > 0.f ? Dialogue : FString();
}


FString AMGGrayFoxMission::GetTransceiverDialogue(int32 Channel) const
{
    // Remake-authored dialogue; Schneider knows architecture, not this prison layout.
    if (Channel == 0)
    {
        if (!(Flags & MG_FOX_CAPTURED))
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

    if (!(Flags & MG_FOX_CAPTURED)) return TEXT("BIG BOSS: Find Gray Fox, Snake.");
    if (!(Flags & MG_FOX_FIRST_WALL_BREACHED))
        return TEXT("BIG BOSS: Snake, you're in a tight spot.\n"
                    "BIG BOSS: Check the wall. There may be a way through.\n"
                    "SNAKE: Understood.");
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
    ElapsedSinceBeat += FMath::Max(0.f, Dt);
    DialogueTimeRemaining = FMath::Max(0.f, DialogueTimeRemaining - Dt);
    if (FoxSpeech >= 0.f) FoxSpeech += Dt;
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
    if (Player && (Flags & MG_FOX_EXIT_WALL_BREACHED) &&
        !(Flags & MG_FOX_LEFT_PRISON) &&
        FVector::Dist2D(Player->GetActorLocation(), ExitPoint) < 190.f)
    {
        ApplyAction(MG_FOX_ESCAPE, Player);
    }
    if (Player && (Flags & MG_FOX_RECOVERED) && !(Flags & MG_FOX_SHOTMAKER_DOWN))
    {
        if (!IsValid(Shotmaker))
        {
            Shotmaker = GetWorld()->SpawnActor<AMGGuardActor>(FVector(2660,100,-11), FRotator(0,180,0));
            if (Shotmaker) Shotmaker->ConfigureShotmaker();
        }
        if (Shotmaker && Shotmaker->IsIncapacitated()) ApplyAction(MG_FOX_DEFEAT, Player);
    }
    if (Player && (Flags & MG_FOX_SHOTMAKER_DOWN) && !(Flags & MG_FOX_COMPLETE)
        && FVector::Dist2D(Player->GetActorLocation(), LastDoor) < 105.f)
        ApplyAction(MG_FOX_DEPART, Player);

}

void AMGGrayFoxMission::RestartMission()
{
    UGameplayStatics::DeleteGameInSlot(SaveSlot, 0);
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void AMGGrayFoxMission::AddLight(const FVector& Position, FLinearColor Colour, float Radius)
{
    if (auto* Lamp = GetWorld()->SpawnActor<APointLight>(Position, FRotator::ZeroRotator))
    {
        auto* Light = CastChecked<UPointLightComponent>(Lamp->GetLightComponent());
        Light->SetMobility(EComponentMobility::Movable);
        Light->SetLightColor(Colour);
        Light->SetAttenuationRadius(Radius);
        Light->SetIntensity(2400.f);
        Light->SetCastShadows(false);
    }
}

void AMGGrayFoxMission::ShowScar(AStaticMeshActor* Wall, uint8 Hit)
{
    if (!IsValid(Wall)) return;
    const bool Side = Wall == WeakWall;
    const float Offset = Hit == 1 ? -11.f : 7.f;
    const FVector At = Wall->GetActorLocation() +
        (Side ? FVector(-22,Offset,-67 + Hit*13) : FVector(Offset,-22,-67 + Hit*13));
    auto* Chip = SpawnBlock(TEXT("SplitMasonry"), At, FVector(.06,.10,.46), false);
    if (Chip)
    {
        Chip->SetActorRotation(FRotator(0,Side ? 0 : 90,Hit == 1 ? 29 : -35));
        WallScars.Add(Chip);
    }
}

bool AMGGrayFoxMission::StrikeWall(AMGPlayerCharacter* Player, AStaticMeshActor* Wall, uint8& Hits, uint32 Action)
{
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now < NextStrike) return true;
    NextStrike = Now + .30f;
    Player->PlayBreachStrike();
    const FVector At = Wall->GetActorLocation();
    ++Hits;
    const bool Open = Hits == 3;
    const auto Cue = Open ? EMGPrisonCue::Collapse : (Hits == 1 ? EMGPrisonCue::Knock : EMGPrisonCue::Fracture);
    MGPrisonAudio::Play(this, At, Cue);
    UE_LOG(LogTemp, Display, TEXT("GrayFox wall=%u impact=%d elapsed=%.2f"), Action, Hits, ElapsedSinceBeat);
    if (!Open)
    {
        ShowScar(Wall, Hits);
        Player->NotifyFeedback(Hits == 1 ? TEXT("HOLLOW IMPACT — A CRACK OPENS") : TEXT("MORTAR BREAKING"));
        SaveProgress(Player);
        return true;
    }
    for (auto* Scar : WallScars) if (IsValid(Scar)) Scar->Destroy();
    WallScars.Reset();
    for (int32 I = 0; I < 8; ++I)
    {
        const float Side = I % 2 ? 1.f : -1.f;
        const FVector Offset = Wall == WeakWall ? FVector((I-3)*12,Side*68,0) : FVector(Side*68,(I-3)*12,0);
        auto* Rubble = SpawnBlock(TEXT("Masonry"), FVector(At.X, At.Y, -92) + Offset,
                                 FVector(.16 + .02*I,.24,.11), false);
        if (Rubble) Rubble->SetActorRotation(FRotator(8,I*41,13));
    }
    return ApplyAction(Action, Player);
}
