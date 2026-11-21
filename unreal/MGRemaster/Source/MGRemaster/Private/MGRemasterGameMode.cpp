#include "MGRemasterGameMode.h"

#include "MGGrayFoxMission.h"
#include "MGMissionOne.h"
#include "MGEquipmentComponent.h"
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

    // Start with the Outer Heaven arrival. The Gray Fox prison prototype remains
    // available as a separate source actor and is not erased by this pass.
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (!PC->GetPawn())
        {
            AMGPlayerCharacter* Pawn = GetWorld()->SpawnActor<AMGPlayerCharacter>(
                FVector(700.f, 700.f, 90.f), FRotator::ZeroRotator);
            if (Pawn) PC->Possess(Pawn);
        }
    }
    if (AMGMissionOne* Mission = GetWorld()->SpawnActor<AMGMissionOne>(
            AMGMissionOne::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
    {
        if (AMGPlayerCharacter* Snake = Cast<AMGPlayerCharacter>(
                UGameplayStatics::GetPlayerCharacter(this, 0)))
        {
            Snake->SetActorLocation(Mission->EntryLocation(), false, nullptr,
                                    ETeleportType::TeleportPhysics);
            if (UMGEquipmentComponent* Inventory = Snake->GetEquipment())
                Inventory->InitialiseMissionOneLoadout();
            Snake->NotifyFeedback(TEXT("OPERATION INTRUDE N313 — INFILTRATE OUTER HEAVEN"),
                                  TEXT("select"));
        }
    }
}
