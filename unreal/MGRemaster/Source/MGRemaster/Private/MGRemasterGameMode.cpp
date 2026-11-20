#include "MGRemasterGameMode.h"

#include "MGGrayFoxMission.h"
#include "MGPlayerCharacter.h"
#include "MGPrototypeHUD.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
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

    // The rescue is the current runnable mission slice; other greybox
    // environments are added only after original J2ME map verification.
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (!PC->GetPawn())
        {
            AMGPlayerCharacter* Pawn = GetWorld()->SpawnActor<AMGPlayerCharacter>(
                FVector(-950.f, 0.f, -11.f), FRotator::ZeroRotator);
            if (Pawn) PC->Possess(Pawn);
        }
    }
    // Editor-authored levels may already contain this mission controller.
    // A duplicate would spawn overlapping collision walls, duplicate guards
    // and race on the same SaveGame slot. Spawn only in the Entry fallback.
    if (!UGameplayStatics::GetActorOfClass(
            this, AMGGrayFoxMission::StaticClass()))
    {
        GetWorld()->SpawnActor<AMGGrayFoxMission>(
            AMGGrayFoxMission::StaticClass(), FVector::ZeroVector,
            FRotator::ZeroRotator);
    }
}
