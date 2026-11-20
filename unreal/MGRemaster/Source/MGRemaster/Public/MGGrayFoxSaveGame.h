#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGGrayFoxSaveGame.generated.h"

/** V3 records firearm ammunition; V1 and V2 prison checkpoints are migrated. */
UCLASS()
class MGREMASTER_API UMGGrayFoxSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    uint32 SchemaVersion = 3;

    UPROPERTY(SaveGame)
    uint32 MissionFlags = 0;

    UPROPERTY(SaveGame)
    FVector CheckpointLocation = FVector(-950.f, 0.f, -11.f);

    // Only actual committed ammunition is saved, not a fresh default each death.
    UPROPERTY(SaveGame)
    int32 HandgunRounds = 24;
    UPROPERTY(SaveGame)
    int32 StunRounds = 0; // Legacy V3 field; no active campaign stun slot.
};
