#include "MGRemasterGameMode.h"

#include "MGGuardActor.h"
#include "MGPlayerCharacter.h"
#include "MGPrototypeHUD.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AMGRemasterGameMode::AMGRemasterGameMode()
{
    DefaultPawnClass = AMGPlayerCharacter::StaticClass();
    HUDClass = AMGPrototypeHUD::StaticClass();
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    BlockMesh = Cube.Succeeded() ? Cube.Object : nullptr;
}

void AMGRemasterGameMode::PlaceBlock(const FVector& Location, const FVector& Scale)
{
    AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!Block) return;
    UStaticMeshComponent* Mesh = Block->GetStaticMeshComponent();
    Mesh->SetMobility(EComponentMobility::Movable);
    if (BlockMesh) Mesh->SetStaticMesh(BlockMesh);
    Mesh->SetWorldScale3D(Scale);
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AMGRemasterGameMode::StartPlay()
{
    Super::StartPlay();
    if (!GetWorld()) return;
    // Deliberately an original test chamber, NOT reconstructed MSX/J2ME geometry.
    // Cube meshes are 100cm; floor top at z=-100, pawn capsule bottom at -88.
    PlaceBlock(FVector(0.f, 0.f, -150.f), FVector(24.f, 18.f, 1.f));
    PlaceBlock(FVector(-1200.f, 0.f, 125.f), FVector(1.f, 18.f, 4.5f));
    PlaceBlock(FVector(1200.f, 0.f, 125.f), FVector(1.f, 18.f, 4.5f));
    PlaceBlock(FVector(0.f, -900.f, 125.f), FVector(24.f, 1.f, 4.5f));
    PlaceBlock(FVector(0.f, 900.f, 125.f), FVector(24.f, 1.f, 4.5f));
    PlaceBlock(FVector(200.f, 120.f, -20.f), FVector(1.8f, 3.f, 1.6f));
    PlaceBlock(FVector(-350.f, -200.f, 0.f), FVector(2.2f, 2.f, 2.f));

    GetWorld()->SpawnActor<AMGGuardActor>(AMGGuardActor::StaticClass(),
        FVector(550.f, 350.f, -12.f), FRotator(0.f, 180.f, 0.f));
    GetWorld()->SpawnActor<AMGGuardActor>(AMGGuardActor::StaticClass(),
        FVector(-650.f, 300.f, -12.f), FRotator::ZeroRotator);
}
