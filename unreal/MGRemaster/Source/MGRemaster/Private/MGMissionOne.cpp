#include "MGMissionOne.h"
#include "MGGuardActor.h"
#include "MGPlayerCharacter.h"
#include "MGEquipmentComponent.h"
#include "Components/SceneComponent.h"
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
    PrimaryActorTick.bCanEverTick = false;
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
    P.Location = CM(Metres.X, Metres.Y, 65.f);
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
        {TEXT("G02_Trucks"), {CM(8,26),CM(19,26),CM(19,31),CM(8,31)}},
        {TEXT("G03_TruckDeparture"), {CM(18,33),CM(13,33),CM(13,29)}},
        {TEXT("G04_SecondYard"), {CM(53,29),CM(65,29),CM(65,34),CM(53,34)}},
        {TEXT("G05_Lift"), {CM(67,24),CM(67,36)}},
        {TEXT("G06_OptionalMaintenance"), {CM(59,46),CM(66,46),CM(66,51)}}
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
}

bool AMGMissionOne::TryInteract(AMGPlayerCharacter* Player)
{
    if (!Player || !Player->GetEquipment()) return false;
    UMGEquipmentComponent* Equipment = Player->GetEquipment();
    for (FPickup& P : Pickups)
    {
        if (P.bTaken || FVector::Dist2D(Player->GetActorLocation(), P.Location) > 190.f)
            continue;
        if (P.Item == TEXT("GASMASK") && !Equipment->HasItem(TEXT("CARD1")))
        {
            Player->NotifyFeedback(TEXT("CARD 1 REQUIRED"), TEXT("denied"));
            return true;
        }
        const bool bAwarded = P.bWeapon ?
            Equipment->AwardWeapon(P.Item, 0) :
            Equipment->AwardItem(P.Item, 1);
        if (!bAwarded) return false;
        P.bTaken = true;
        if (IsValid(P.Marker)) P.Marker->Destroy();
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
            Player->NotifyFeedback(TEXT("CARD 1 DOOR OPEN"), TEXT("select"));
        }
        return true;
    }
    if (!bElevatorReached &&
        FVector::Dist2D(Player->GetActorLocation(), CM(92.f, 26.f)) < 230.f)
    {
        bElevatorReached = true;
        Player->NotifyFeedback(TEXT("BUILDING ONE — UPPER FLOORS NOT YET AUTHOR-VERIFIED"), TEXT("select"));
        return true;
    }
    return false;
}
