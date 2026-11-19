#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGGrayFoxSaveGame.generated.h"

/** Durable slice state. V2 adds encounter flags; v1 prison checkpoints load safely. */
UCLASS()
class MGREMASTER_API UMGGrayFoxSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    uint32 SchemaVersion = 2;

    UPROPERTY(SaveGame)
    uint32 MissionFlags = 0;

    UPROPERTY(SaveGame)
    FVector CheckpointLocation = FVector(-950.f, 0.f, -11.f);
};
