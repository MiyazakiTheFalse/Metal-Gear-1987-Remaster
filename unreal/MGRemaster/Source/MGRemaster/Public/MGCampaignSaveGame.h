#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGCampaignSaveGame.generated.h"

/** Small, versioned cross-level narrative ledger; separate from area checkpoints. */
UCLASS()
class MGREMASTER_API UMGCampaignSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int32 Version = 3;
    // One-based player-facing mission number; world expansion supports 1–5.
    UPROPERTY()
    int32 MissionNumber = 1;
    // Retained solely for migrating v1 saves (old values 0, 1, 2).
    UPROPERTY()
    int32 Chapter = 0;
    UPROPERTY()
    TArray<FName> CompletedBeatIds;
    UPROPERTY()
    TArray<FString> IntelLines;
};
