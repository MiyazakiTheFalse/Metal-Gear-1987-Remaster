#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGMissionOneSaveGame.generated.h"

/** Checkpoint persists only committed, finite story-world interactions. */
UCLASS()
class MGREMASTER_API UMGMissionOneSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int32 Version = 3;
    UPROPERTY()
    TArray<FName> CollectedPickupIds;
    UPROPERTY()
    bool bCardDoorOpened = false;
    UPROPERTY()
    bool bElevatorReached = false;
    UPROPERTY()
    bool bPOWRescued = false;
    UPROPERTY()
    bool bPrisonerDoorOpened = false;
    UPROPERTY()
    bool bGasDoorOpened = false;
    UPROPERTY()
    int32 CurrentFloor = 1;
    UPROPERTY()
    FVector LastCheckpoint = FVector(700.f, 700.f, 90.f);
};
