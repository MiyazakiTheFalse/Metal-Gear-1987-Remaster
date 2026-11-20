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

/** V4 persists first earned firearm and additive physical guard-body state. */
UCLASS()
class MGREMASTER_API UMGGrayFoxSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    uint32 SchemaVersion = 4;

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
    // Retain original guard positions AFTER the player drags them away from
    // patrol lanes. IDs map to placed 1995 actors, never array enumeration.
    UPROPERTY(SaveGame)
    TArray<FMGPersistedGuardBody> GuardBodies;
};
