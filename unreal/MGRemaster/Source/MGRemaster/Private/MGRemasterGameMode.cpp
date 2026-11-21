#include "MGRemasterGameMode.h"

#include "MGGrayFoxMission.h"
#include "MGCampaignState.h"
#include "Engine/GameInstance.h"
#include "MGMissionOne.h"
#include "MGLaterMission.h"
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

    // Mission number survives a reload between mission spaces.
    // Never spawn both prototype worlds: their collisions would overlap.
    UMGCampaignState* Story = GetWorld()->GetGameInstance() ?
        GetWorld()->GetGameInstance()->GetSubsystem<UMGCampaignState>() : nullptr;
    const int32 MissionNumber = Story ? Story->GetMissionNumber() : 1;
    if (Story && MissionNumber == 1)
    {
        Story->RecordBeat(TEXT("snake.mission_start"),
            TEXT("OPERATION INTRUDE N313: Locate the missing operative Gray Fox."));
        Story->RecordBeat(TEXT("fox.missing"),
            TEXT("GRAY FOX: Previous FOXHOUND infiltration ended after a Metal Gear report."));
    }
    // Launch from the insertion unless the player has deliberately entered
    // the secret prison. Reloading after a completed rescue returns to 3F.
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (!PC->GetPawn())
        {
            AMGPlayerCharacter* Pawn = GetWorld()->SpawnActor<AMGPlayerCharacter>(
                FVector(700.f, 700.f, 90.f), FRotator::ZeroRotator);
            if (Pawn) PC->Possess(Pawn);
        }
    }
    if (MissionNumber == 2)
    {
        // The prison actor restores the dedicated capture checkpoint and
        // initializes confiscated inventory. It is never spawned alongside 1F.
        GetWorld()->SpawnActor<AMGGrayFoxMission>(
            AMGGrayFoxMission::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
        return;
    }
    if (MissionNumber >= 4 ||
        (MissionNumber == 3 && Story && Story->IsMissionThreeExteriorActive()))
    {
        if (AMGLaterMission* Later = GetWorld()->SpawnActor<AMGLaterMission>(
            AMGLaterMission::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
        {
            Later->RestorePlayerEquipment(Cast<AMGPlayerCharacter>(
                UGameplayStatics::GetPlayerCharacter(this, 0)));
        }
        return;
    }
    if (AMGMissionOne* Mission = GetWorld()->SpawnActor<AMGMissionOne>(
            AMGMissionOne::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
    {
        if (AMGPlayerCharacter* Snake = Cast<AMGPlayerCharacter>(
                UGameplayStatics::GetPlayerCharacter(this, 0)))
        {
            Snake->SetActorLocation(Mission->ResumeLocation(), false, nullptr,
                                    ETeleportType::TeleportPhysics);
            if (UMGEquipmentComponent* Inventory = Snake->GetEquipment())
                Inventory->InitialiseMissionOneLoadout();
            Mission->RestoreSavedEquipment(Snake);
            Snake->NotifyFeedback(MissionNumber == 3 ?
                TEXT("MISSION 03 — FOX'S INTELLIGENCE / FIND PETTROVICH") :
                TEXT("MISSION 01 — OPERATION INTRUDE N313"),
                                  TEXT("select"));
        }
    }
}
