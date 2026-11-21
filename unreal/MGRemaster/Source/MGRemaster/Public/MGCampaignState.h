#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCampaignState.generated.h"

class UMGCampaignSaveGame;

/**
 * Persists source-comparative narrative transactions between level reloads.
 * These scene IDs are also defined in data/character_arc_registry.json.
 * No optional radio call is permitted to block the Gray Fox path.
 */
UCLASS()
class MGREMASTER_API UMGCampaignState : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    int32 GetMissionNumber() const;
    bool IsMissionThreeExteriorActive() const;
    bool SetMissionThreeExteriorActive(bool bExterior);
    int32 GetTX55CorrectCharges() const;
    bool RecordTX55Charge(bool bRightFoot);
    float GetEscapeTimeRemaining() const;
    bool SetEscapeTimeRemaining(float Seconds);
    FString GetMissionTitle() const;
    FString GetCharacterThread() const;
    int32 GetRadioContactCount() const;
    FString GetRadioContactLabel(int32 Channel) const;
    bool HasBeat(FName Id) const;
    int32 GetMissionThreeIntelCount() const;
    bool RecordBeat(FName Id, const FString& Intel);
    /** v5->v6 correction: old prototype gave Diane's frequency too early. */
    void RevokeMisplacedDianeContact();
    bool AdvanceMission(int32 NextMissionNumber);
    FString GetIntelLog() const;
    FString GetCurrentObjective() const;
    FString GetOpeningRadio(int32 Channel) const;
    void ResetForDevelopment();
private:
    UPROPERTY()
    UMGCampaignSaveGame* Progress = nullptr;
    bool Commit();
};
