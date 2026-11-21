#include "MGMissionOne.h"
#include "MGMissionOneSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "MGGuardActor.h"
#include "MGSecurityCamera.h"
#include "MGPlayerCharacter.h"
#include "MGEquipmentComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    FVector CM(float X, float Y, float Z = 90.f) { return FVector(X * 100.f, Y * 100.f, Z); }
}

AMGMissionOne::AMGMissionOne()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) BlockMesh = Cube.Object;
}

AStaticMeshActor* AMGMissionOne::Block(FName Tag, FVector Position, FVector Scale, bool bCollision)
{
    if (!BlockMesh || !GetWorld()) return nullptr;
    AStaticMeshActor* A = GetWorld()->SpawnActor<AStaticMeshActor>(Position, FRotator::ZeroRotator);
    if (!A) return nullptr;
    A->Tags.Add(Tag);
    UStaticMeshComponent* M = A->GetStaticMeshComponent();
    M->SetMobility(EComponentMobility::Movable);
    M->SetStaticMesh(BlockMesh);
    M->SetWorldScale3D(Scale);
    M->SetCollisionProfileName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
    return A;
}

void AMGMissionOne::AddPickup(FName Id, FName Item, FVector Metres, bool bWeapon)
{
    FPickup P;
    P.Id = Id;
    P.Item = Item;
    P.Location = CM(Metres.X, Metres.Y, 65.f + Metres.Z * 100.f);
    P.bWeapon = bWeapon;
    P.Marker = Block(Id, P.Location, FVector(.65f, .65f, .45f), false);
    Pickups.Add(P);
}

void AMGMissionOne::Build()
{
    // BasicShapes cube is 100 cm: contract area rectangles are expressed in metres.
    // Floors provide physically continuous traversal, not proof of source geometry.
    const struct FRoom { const TCHAR* Id; float X,Y,W,H; } Rooms[] = {
        {TEXT("M01_Insertion"),0,0,24,18},
        {TEXT("M01_OptionalDrainage"),24,0,18,18},
        {TEXT("M01_FirstTruckYard"),0,18,24,18},
        {TEXT("M01_CardOneStore"),24,18,24,18},
        {TEXT("M01_SecondTruckYard"),48,18,24,24},
        {TEXT("M01_OptionalMaintenance"),48,42,30,12},
        {TEXT("M01_Elevator"),72,18,28,24}
    };
    for (const FRoom& R : Rooms)
        Block(FName(R.Id), CM(R.X + R.W/2.f, R.Y + R.H/2.f, -75.f),
              FVector(R.W, R.H, 1.5f));

    // Authored circulation: wall sections stop shortcuts across key-card
    // dependencies. Door-sized gaps remain at the actual crossing points.
    // These boundaries are provisional and must be replaced after J2ME tracing.
    auto WallX = [this](float X, float FromY, float ToY)
    {
        Block(TEXT("Boundary"), CM(X, (FromY + ToY)/2.f, 165.f),
              FVector(.35f, ToY - FromY, 3.3f));
    };
    auto WallY = [this](float Y, float FromX, float ToX)
    {
        Block(TEXT("Boundary"), CM((FromX + ToX)/2.f, Y, 165.f),
              FVector(ToX - FromX, .35f, 3.3f));
    };
    // Main playable route: insertion -> three trucks -> Card 1 store ->
    // later trucks -> elevator. Optional unverified connectors are sealed.
    WallY(0.f, 0.f, 42.f);
    WallX(0.f, 0.f, 36.f);
    WallY(18.f, 0.f, 9.5f); WallY(18.f, 14.5f, 42.f);
    WallX(24.f, 0.f, 18.f);
    WallX(42.f, 0.f, 18.f);
    WallY(36.f, 0.f, 48.f);
    WallX(24.f, 18.f, 25.4f); WallX(24.f, 28.6f, 36.f);
    WallX(48.f, 18.f, 26.5f); WallX(48.f, 29.5f, 36.f);
    WallY(18.f, 24.f, 100.f);
    WallY(42.f, 48.f, 78.f);
    WallX(48.f, 36.f, 54.f);
    WallY(54.f, 48.f, 78.f);
    WallX(78.f, 42.f, 54.f);
    WallX(72.f, 18.f, 28.f); WallX(72.f, 32.f, 42.f);
    WallY(42.f, 78.f, 100.f);
    WallX(100.f, 18.f, 42.f);

    // Original-comparative truck groups. Separate volumes and approach vectors
    // make discovery readable. These are placeholder solid props, not final trucks.
    for (float X : {5.f, 12.f, 20.f})
        Block(TEXT("FirstTruckProxy"), CM(X, 23.f, 105.f), FVector(3.3f, 5.5f, 2.1f));
    for (float X : {55.f, 61.f, 67.f})
        Block(TEXT("SecondTruckProxy"), CM(X, 33.f, 105.f), FVector(3.3f, 5.5f, 2.1f));
    for (const FVector& P : {CM(11.f, 9.f), CM(17.f, 9.f), CM(9.f, 32.f),
                             CM(34.f, 22.f), CM(41.f, 31.f), CM(60.f, 23.f)})
        Block(TEXT("CrateCover"), P, FVector(1.5f, 1.3f, 1.5f));

    // A visible removable card door; other transitions remain open pending
    // verified original connectivity. Never make up a critical source shortcut.
    CardDoor = Block(TEXT("CardOneDoor"), CM(24.f, 27.f, 135.f),
                     FVector(.40f, 3.2f, 2.7f));
    Block(TEXT("ElevatorMarker"), CM(92.f, 26.f, 90.f), FVector(1.f, 1.f, 1.8f), false);

    // Interior locations are represented by pickup markers near accessible sides
    // of truck proxies, never inside a blocking mesh.
    AddPickup(TEXT("P01"), TEXT("RATION"), FVector(5.f, 19.5f, 0.f), false);
    AddPickup(TEXT("P02"), TEXT("CARD1"), FVector(12.f, 19.5f, 0.f), false);
    AddPickup(TEXT("P03"), TEXT("BINOCULARS"), FVector(20.f, 19.5f, 0.f), false);
    AddPickup(TEXT("P04"), TEXT("GASMASK"), FVector(31.f, 25.f, 0.f), false);
    AddPickup(TEXT("P05"), TEXT("HANDGUN"), FVector(55.f, 29.5f, 0.f), true);
    AddPickup(TEXT("P06"), TEXT("MINES"), FVector(67.f, 29.5f, 0.f), false);
    BuildUpperFloors();
}

void AMGMissionOne::SpawnGuards()
{
    struct FGuardRoute
    {
        const TCHAR* Name;
        TArray<FVector> Nodes;
    };
    // Waypoints authored in provisional remake metres, all within their zones.
    const TArray<FGuardRoute> Routes = {
        {TEXT("G01_Approach"), {CM(11,12),CM(17,12),CM(17,15),CM(11,15)}},
        {TEXT("G02_Trucks"), {CM(8,27),CM(19,27),CM(19,31),CM(8,31)}},
        {TEXT("G03_TruckDeparture"), {CM(18,33),CM(13,33),CM(13,29)}},
        {TEXT("G04_SecondYard"), {CM(53,29),CM(65,29),CM(65,34),CM(53,34)}},
        {TEXT("G05_Lift"), {CM(70,24),CM(70,36)}},
        {TEXT("G07_SecondFloorCorridor"), {CM(58,29,690),CM(69,29,690),CM(69,35,690)}},
        {TEXT("G08_ThirdFloorCameraGallery"), {CM(65,28,1290),CM(76,28,1290),CM(76,30,1290),CM(65,30,1290)}}
        // G06's optional maintenance wing is sealed in source-fidelity mode.
        // It remains authored in the JSON contract but must not spawn here.
    };
    for (const FGuardRoute& Route : Routes)
    {
        if (Route.Nodes.IsEmpty()) continue;
        AMGGuardActor* Guard = GetWorld()->SpawnActor<AMGGuardActor>(
            Route.Nodes[0], FRotator::ZeroRotator);
        if (Guard)
        {
            Guard->Tags.Add(FName(Route.Name));
            Guard->ConfigurePatrol(Route.Nodes);
        }
    }
}

void AMGMissionOne::BeginPlay()
{
    Super::BeginPlay();
    Build();
    SpawnGuards();
    const UMGMissionOneSaveGame* Save = Cast<UMGMissionOneSaveGame>(
        UGameplayStatics::LoadGameFromSlot(TEXT("MissionOneV1"), 0));
    if (Save && (Save->Version == 1 || Save->Version == 2 || Save->Version == 3))
    {
        for (FPickup& P : Pickups)
            if (Save->CollectedPickupIds.Contains(P.Id))
            {
                P.bTaken = true;
                if (IsValid(P.Marker)) P.Marker->Destroy();
            }
        bCardDoorOpened = Save->bCardDoorOpened;
        if (bCardDoorOpened && IsValid(CardDoor)) CardDoor->Destroy();
        bElevatorReached = Save->bElevatorReached;
        bPOWRescued = Save->Version >= 2 && Save->bPOWRescued;
        bPrisonerDoorOpened = Save->Version >= 3 && Save->bPrisonerDoorOpened;
        bGasDoorOpened = Save->Version >= 3 && Save->bGasDoorOpened;
        if (bPrisonerDoorOpened && IsValid(PrisonerDoor)) PrisonerDoor->Destroy();
        if (bGasDoorOpened && IsValid(GasDoor)) GasDoor->Destroy();
        CurrentFloor = Save->Version >= 2 ? FMath::Clamp(Save->CurrentFloor, 1, 3) : 1;
        SelectedFloor = CurrentFloor == 1 ? 3 : (CurrentFloor == 3 ? 2 : 1);
        // Checkpoint is kept at the fixed start until source-verified safe
        // positions are authored; no restart into a solid door or enemy.
    }
}

void AMGMissionOne::RestoreSavedEquipment(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment()) return;
    UMGEquipmentComponent* Equipment = Player->GetEquipment();
    for (const FPickup& P : Pickups)
        if (P.bTaken)
        {
            if (P.Item == TEXT("HANDGUN_AMMO"))
                Equipment->AwardAmmunition(TEXT("HANDGUN"), 12);
            else if (P.bWeapon) Equipment->AwardWeapon(P.Item, 0);
            else Equipment->AwardItem(P.Item, 1);
        }
}

void AMGMissionOne::SaveProgress(AMGPlayerCharacter* Player)
{
    UMGMissionOneSaveGame* Save = Cast<UMGMissionOneSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UMGMissionOneSaveGame::StaticClass()));
    if (!Save) return;
    for (const FPickup& P : Pickups)
        if (P.bTaken) Save->CollectedPickupIds.Add(P.Id);
    Save->bCardDoorOpened = bCardDoorOpened;
    Save->bElevatorReached = bElevatorReached;
    Save->bPOWRescued = bPOWRescued;
    Save->bPrisonerDoorOpened = bPrisonerDoorOpened;
    Save->bGasDoorOpened = bGasDoorOpened;
    Save->CurrentFloor = CurrentFloor;
    Save->Version = 3;
    if (!UGameplayStatics::SaveGameToSlot(Save, TEXT("MissionOneV1"), 0))
        UE_LOG(LogTemp, Warning, TEXT("Mission One checkpoint save failed"));
}

bool AMGMissionOne::TryInteract(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment()) return false;
    UMGEquipmentComponent* Equipment = Player->GetEquipment();
    for (FPickup& P : Pickups)
    {
        if (P.bTaken || FVector::Dist(Player->GetActorLocation(), P.Location) > 190.f)
            continue;
        if (P.Item == TEXT("GASMASK") && !Equipment->HasItem(TEXT("CARD1")))
        {
            Player->NotifyFeedback(TEXT("CARD 1 REQUIRED"), TEXT("denied"));
            return true;
        }
        const bool bAwarded = P.Item == TEXT("HANDGUN_AMMO") ?
            Equipment->AwardAmmunition(TEXT("HANDGUN"), 12) :
            (P.bWeapon ? Equipment->AwardWeapon(P.Item, 0) :
            Equipment->AwardItem(P.Item, 1));
        if (!bAwarded) return false;
        P.bTaken = true;
        if (IsValid(P.Marker)) P.Marker->Destroy();
        SaveProgress(Player);
        Player->NotifyFeedback(FString::Printf(TEXT("%s ACQUIRED"), *P.Item.ToString()), TEXT("select"));
        return true;
    }
    if (!bCardDoorOpened && IsValid(CardDoor) &&
        FVector::Dist2D(Player->GetActorLocation(), CardDoor->GetActorLocation()) < 240.f)
    {
        if (!Equipment->HasItem(TEXT("CARD1")))
            Player->NotifyFeedback(TEXT("LOCKED — CARD 1"), TEXT("denied"));
        else
        {
            bCardDoorOpened = true;
            CardDoor->Destroy();
            SaveProgress(Player);
            Player->NotifyFeedback(TEXT("CARD 1 DOOR OPEN"), TEXT("select"));
        }
        return true;
    }
    if (CurrentFloor == 3 && !bPOWRescued &&
        FVector::Dist(Player->GetActorLocation(), CM(54.f, 33.f, 1290.f)) < 240.f)
    {
        bPOWRescued = true;
        SaveProgress(Player);
        Player->NotifyFeedback(TEXT("PRISONER CONTACT — SOURCE DIALOGUE PENDING"), TEXT("rescue"));
        return true;
    }
    if (IsNearLift(Player)) return StartLift(Player);
    return false;
}

FString AMGMissionOne::GetObjectiveText() const
{
    if (bLiftTravelling)
        return FString::Printf(TEXT("ELEVATOR / EN ROUTE TO %dF"), DestinationFloor);
    if (CurrentFloor == 3)
        return bPOWRescued ? TEXT("3F — SEARCH CAMERA GALLERY / RETURN VIA ELEVATOR")
            : TEXT("3F — FIND HANDGUN AMMO AND THE PRISONER");
    if (CurrentFloor == 2)
        return TEXT("2F — RECONNOITRE CORRIDORS / RETURN TO ELEVATOR");
    bool bCardCollected = false;
    for (const FPickup& P : Pickups)
        if (P.Item == FName(TEXT("CARD1")) && P.bTaken)
            bCardCollected = true;
    if (!bCardCollected) return TEXT("INFILTRATE THE TRUCK YARD — FIND ACCESS CARD 1");
    if (!bCardDoorOpened) return TEXT("ACCESS THE CARD 1 ROOM — INVESTIGATE ITS CONTENTS");
    return FString::Printf(TEXT("1F — REACH ELEVATOR / Q SELECTS %dF"), SelectedFloor);
}

void AMGMissionOne::BuildUpperFloors()
{
    // Separate persistent decks share a vertical shaft; they are not detached
    // unrelated maps. 600 cm deck separation is an authored remake hypothesis.
    for (int32 Floor = 2; Floor <= 3; ++Floor)
    {
        const float DeckZ = (Floor - 1) * 600.f;
        Block(FName(*FString::Printf(TEXT("B1_%dF_Deck"), Floor)),
              CM(71.f, 30.f, DeckZ - 75.f), FVector(58.f, 24.f, 1.5f));
        auto Perimeter = [this, DeckZ](float X, float Y, float SX, float SY)
        {
            Block(TEXT("UpperBoundary"), CM(X, Y, DeckZ + 165.f),
                  FVector(SX, SY, 3.3f));
        };
        Perimeter(42.f,30.f,.35f,24.f);
        Perimeter(100.f,30.f,.35f,24.f);
        Perimeter(71.f,18.f,58.f,.35f);
        Perimeter(71.f,42.f,58.f,.35f);
        // Adjacent elevator foyer and recognizable cover anchor. 3F cameras
        // are independent, rotating LOS sensors; not part of guard patrol AI.
        if (Floor == 2)
        {
            Block(TEXT("2F_ServiceCrates"), CM(61.f, 34.f, DeckZ + 85.f),
                  FVector(2.2f, 1.4f, 1.7f));
            Block(TEXT("2F_PatrolCover"), CM(76.f, 23.f, DeckZ + 65.f),
                  FVector(2.0f, 1.1f, 1.3f));
            if (AMGSecurityCamera* Cam = GetWorld()->SpawnActor<AMGSecurityCamera>(
                    CM(77.f,39.f,DeckZ+265.f), FRotator(0.f,-90.f,0.f)))
                Cam->Configure(TEXT("CCTV_2F_SECURITY"),2,40.f);
        }
        else
        {
            // Original-comparative 3F: north cameras, western opening, Card 1
            // prisoner cell, and a gas-filled connecting room. The 3D meters
            // and exact shapes remain provisional pending J2ME map recovery.
            Block(TEXT("3F_PrisonerContact"), CM(54.f, 33.f, DeckZ + 90.f),
                  FVector(.6f,.6f,1.8f), false);
            Block(TEXT("3F_CrateCover"), CM(83.f, 27.f, DeckZ + 70.f),
                  FVector(1.8f,1.3f,1.4f));
            Block(TEXT("3F_CameraBlindCover"), CM(73.f, 23.f, DeckZ + 70.f),
                  FVector(1.8f,1.3f,1.4f));
            auto WallX = [this,DeckZ](float X,float Y1,float Y2)
            {
                Block(TEXT("3F_Partition"), CM(X, (Y1+Y2)/2.f, DeckZ+165.f),
                      FVector(.30f,Y2-Y1,3.3f));
            };
            auto WallY = [this,DeckZ](float Y,float X1,float X2)
            {
                Block(TEXT("3F_Partition"), CM((X1+X2)/2.f, Y, DeckZ+165.f),
                      FVector(X2-X1,.30f,3.3f));
            };
            // Contained prisoner room: the player must approach its 3m east door.
            WallX(49.f,29.f,39.f);
            WallY(29.f,49.f,60.f);
            WallY(39.f,49.f,60.f);
            WallX(60.f,29.f,31.5f); WallX(60.f,34.5f,39.f);
            PrisonerDoor = Block(TEXT("3F_PrisonerDoor_Card1"),
                CM(60.f,33.f,DeckZ+135.f), FVector(.35f,3.f,2.7f));
            // The optional hazardous side passage is physically bounded.
            WallX(62.f,33.f,35.f); WallX(62.f,38.f,41.f);
            WallY(41.f,62.f,80.f);
            WallY(33.f,62.f,68.f); WallY(33.f,72.f,80.f);
            WallX(80.f,33.f,35.f); WallX(80.f,38.f,41.f);
            GasDoor = Block(TEXT("3F_GasDoor_Card1"),
                CM(62.f,36.5f,DeckZ+135.f), FVector(.35f,3.f,2.7f));
            // 2.7m-high mounted cameras sweep toward the main 3F corridor.
            for (float X : {68.f,79.f})
            {
                if (AMGSecurityCamera* Cam = GetWorld()->SpawnActor<AMGSecurityCamera>(
                        CM(X,39.f,DeckZ+265.f), FRotator(0.f,-90.f,0.f)))
                    Cam->Configure(X<70.f?TEXT("CCTV_3F_WEST"):TEXT("CCTV_3F_EAST"),3,50.f);
            }
        }
        Block(FName(*FString::Printf(TEXT("B1_%dF_Lift"), Floor)),
              CM(92.f, 26.f, DeckZ + 90.f), FVector(1.f,1.f,1.8f), false);
    }
    AddPickup(TEXT("P07"), TEXT("HANDGUN_AMMO"), FVector(78.f, 28.f, 12.f), false);
}

FVector AMGMissionOne::FloorLiftLocation(int32 Floor)
{
    return CM(92.f, 26.f, 90.f + (Floor - 1) * 600.f);
}

bool AMGMissionOne::IsNearLift(const AMGPlayerCharacter* Player) const
{
    return Player && FVector::Dist(Player->GetActorLocation(), FloorLiftLocation(CurrentFloor)) <= 320.f;
}

bool AMGMissionOne::CycleLiftDestination(AMGPlayerCharacter* Player)
{
    if (!IsNearLift(Player) || bLiftTravelling) return false;
    // Q cycles only among *other* floors, giving players deliberate control
    // without adding a menu that seizes the camera.
    do { SelectedFloor = SelectedFloor == 3 ? 1 : SelectedFloor + 1; }
    while (SelectedFloor == CurrentFloor);
    Player->NotifyFeedback(FString::Printf(
        TEXT("ELEVATOR DESTINATION %dF — PRESS E TO TRAVEL"), SelectedFloor),
        TEXT("select"));
    return true;
}

bool AMGMissionOne::StartLift(AMGPlayerCharacter* Player)
{
    if (!IsNearLift(Player) || bLiftTravelling || SelectedFloor == CurrentFloor)
        return false;
    Player->LeaveCover();
    LiftPassenger = Player;
    bLiftTravelling = true;
    DestinationFloor = SelectedFloor;
    // Temporary simulated closed-door phase. When streamed floor sublevels
    // exist, start an async load here and complete only after readiness.
    LiftTravelRemaining = 0.85f;
    if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
        Movement->DisableMovement();
    Player->NotifyFeedback(FString::Printf(
        TEXT("ELEVATOR / %dF — TRANSIT"), DestinationFloor), TEXT("select"));
    return true;
}

void AMGMissionOne::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bLiftTravelling) return;
    LiftTravelRemaining -= FMath::Max(0.f, DeltaSeconds);
    if (LiftTravelRemaining <= 0.f) FinishLift();
}

void AMGMissionOne::FinishLift()
{
    AMGPlayerCharacter* Player = LiftPassenger.Get();
    bLiftTravelling = false;
    LiftPassenger.Reset();
    if (!IsValid(Player)) return;
    const FVector Arrival = FloorLiftLocation(DestinationFloor);
    // All three arrival platforms are authored floors. Teleport once only
    // after the closed-door phase; don't alter other mission state.
    Player->SetActorLocation(Arrival, false, nullptr, ETeleportType::TeleportPhysics);
    if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
        Movement->SetMovementMode(MOVE_Walking);
    CurrentFloor = DestinationFloor;
    SelectedFloor = CurrentFloor == 1 ? 3 : (CurrentFloor == 3 ? 2 : 1);
    bElevatorReached = true;
    SaveProgress(Player);
    Player->NotifyFeedback(FString::Printf(
        TEXT("BUILDING ONE / %dF — Q CHANGE LIFT DESTINATION"), CurrentFloor),
        TEXT("select"));
}
