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
    int32 Version = 1;
    // 0: Building One opening; 1: captured / Gray Fox prison; 2: escaped.
    UPROPERTY()
    int32 Chapter = 0;
    UPROPERTY()
    TArray<FName> CompletedBeatIds;
    UPROPERTY()
    TArray<FString> IntelLines;
};
