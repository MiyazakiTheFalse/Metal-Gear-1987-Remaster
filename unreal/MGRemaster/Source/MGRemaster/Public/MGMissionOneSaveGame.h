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
    int32 Version = 7;
    UPROPERTY()
    TArray<FName> CollectedPickupIds;
    // Added pickups across the same physical Outer Heaven inventory; nothing
    // is silently regenerated on crossing into Building Two.
    UPROPERTY()
    TArray<FName> LaterCollectedPickupIds;
    UPROPERTY()
    TArray<FName> ExtraOwnedItems;
    UPROPERTY()
    TArray<FName> ExtraOwnedWeapons;
    UPROPERTY()
    bool bCardDoorOpened = false;
    UPROPERTY()
    bool bElevatorReached = false;
    UPROPERTY()
    bool bPOWRescued = false;
    // v6 source-fidelity split: early 3F POW is not Diane, and the two later
    // 1F witnesses separately explain Fox's location and the capture method.
    UPROPERTY()
    bool bDianePOWRescued = false;
    UPROPERTY()
    bool bFoxLocationPOWRescued = false;
    UPROPERTY()
    bool bCaptureMethodPOWRescued = false;
    UPROPERTY()
    bool bRCStoreDoorOpened = false;
    UPROPERTY()
    bool bCardTwoDoorOpened = false;
    UPROPERTY()
    bool bDianeDoorOpened = false;
    UPROPERTY()
    bool bFoxLocationDoorOpened = false;
    UPROPERTY()
    bool bCaptureMethodDoorOpened = false;
    UPROPERTY()
    bool bElectricPanelDisabled = false;
    UPROPERTY()
    bool bPrisonerDoorOpened = false;
    UPROPERTY()
    bool bGasDoorOpened = false;
    // v5: one-time cross-mission cache reward, separate from optional story intel.
    UPROPERTY()
    bool bMissionThreeCacheClaimed = false;
    UPROPERTY()
    int32 CurrentFloor = 1;
    UPROPERTY()
    int32 HandgunRounds = -1;
    UPROPERTY()
    int32 RationsRemaining = -1;
    UPROPERTY()
    TMap<FName, int32> WeaponRounds;
    UPROPERTY()
    TMap<FName, int32> ItemQuantities;
    UPROPERTY()
    FVector LastCheckpoint = FVector(700.f, 700.f, 90.f);
};
