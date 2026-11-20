#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGGrayFoxSaveGame.generated.h"

/** Stable KO/corpse state across old corridor returns and checkpoint loads.
 * New additive V4 field; historic V4 saves have an empty body list.
 */
USTRUCT(BlueprintType)
struct FMGPersistedGuardBody
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FName Id = NAME_None;
    UPROPERTY(SaveGame)
    FVector WorldPosition = FVector::ZeroVector;
    UPROPERTY(SaveGame)
    bool bUnconscious = false;
};

/** V5 adds roof encounter progress while preserving all V1–V4 checkpoint fields. */
UCLASS()
class MGREMASTER_API UMGGrayFoxSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    uint32 SchemaVersion = 5;

    UPROPERTY(SaveGame)
    uint32 MissionFlags = 0;

    UPROPERTY(SaveGame)
    FVector CheckpointLocation = FVector(-950.f, 0.f, -11.f);

    // Only actual committed ammunition is saved, not a fresh default each death.
    UPROPERTY(SaveGame)
    int32 HandgunRounds = 0;
    UPROPERTY(SaveGame)
    int32 StunRounds = 0; // Legacy V3 field; no active campaign stun slot.
    UPROPERTY(SaveGame)
    bool bEarnedHandgun = false;
    // Additive V4 source-authentic first-floor supply caches. All default
    // false for older saves; captivity confiscates but never forgets them.
    UPROPERTY(SaveGame)
    bool bEarnedCard1 = false;
    UPROPERTY(SaveGame)
    bool bEarnedTruckRation = false;
    UPROPERTY(SaveGame)
    bool bEarnedBinoculars = false;
    // Additive fields default to the original state in older V4 saves.
    UPROPERTY(SaveGame)
    bool bPrisonCameraOffline = false;
    UPROPERTY(SaveGame)
    bool bKennelCameraOffline = false;
    UPROPERTY(SaveGame)
    bool bHeardFoxFollowup = false;
    UPROPERTY(SaveGame)
    bool bOwnsCarton = false;
    UPROPERTY(SaveGame)
    bool bHeardFreightClerk = false; // Optional scene, no canon mission flag.
    UPROPERTY(SaveGame)
    bool bDispatchCameraOffline = false;
    UPROPERTY(SaveGame)
    bool bServiceChargeCollected = false;
    UPROPERTY(SaveGame)
    bool bServiceWallOpen = false;
    UPROPERTY(SaveGame)
    bool bRelayIntel = false;
    UPROPERTY(SaveGame)
    bool bKidDefeated = false;
    UPROPERTY(SaveGame)
    bool bParachuteTaken = false;
    UPROPERTY(SaveGame) bool bRoofActive = false;
    UPROPERTY(SaveGame) bool bHindDefeated = false;
    UPROPERTY(SaveGame) int32 HindHitsRemaining = 3;
    UPROPERTY(SaveGame) int32 RoofGrenadesRemaining = 6;
    UPROPERTY(SaveGame) int32 HindPhase = 0;
    UPROPERTY(SaveGame) float HindPhaseTime = 2.5f;
    UPROPERTY(SaveGame) float HindTravelTime = 0.f;
    // Retain original guard positions AFTER the player drags them away from
    // patrol lanes. IDs map to placed 1995 actors, never array enumeration.
    UPROPERTY(SaveGame)
    TArray<FMGPersistedGuardBody> GuardBodies;
};
