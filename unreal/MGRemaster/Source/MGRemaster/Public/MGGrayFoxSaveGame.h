#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGEquipmentComponent.h"
#include "MGGrayFoxSaveGame.generated.h"

/** Durable slice state. A production save will extend this with mission inventory. */
UCLASS()
class MGREMASTER_API UMGGrayFoxSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    uint32 SchemaVersion = 1;

    UPROPERTY(SaveGame)
    uint32 MissionFlags = 0;

    UPROPERTY(SaveGame)
    FVector CheckpointLocation = FVector(-950.f, 0.f, -11.f);
    UPROPERTY(SaveGame) uint8 FirstHits = 0;
    UPROPERTY(SaveGame) uint8 ExitHits = 0;
    UPROPERTY(SaveGame) TArray<FMGEquipmentEntry> Weapons;
    UPROPERTY(SaveGame) TArray<FMGEquipmentEntry> Items;
    UPROPERTY(SaveGame) int32 EquippedWeapon = 0;
    UPROPERTY(SaveGame) int32 EquippedItem = 0;
};
