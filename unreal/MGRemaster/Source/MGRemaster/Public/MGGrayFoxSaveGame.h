#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGGrayFoxSaveGame.generated.h"

/** V4 persists first earned firearm; V1–V3 source checkpoints migrate. */
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
};
