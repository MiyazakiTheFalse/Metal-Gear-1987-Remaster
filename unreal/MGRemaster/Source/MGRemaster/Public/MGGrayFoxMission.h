#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGrayFoxMission.generated.h"

class AStaticMeshActor;
class AMGPlayerCharacter;
class AMGShootGunnerActor;
class AMGMachinegunKid;
class AMGSurveillanceCameraActor;
class AMGGuardActor;
class AMGStoryCameraDirector;
class UStaticMesh;
class APointLight;

/**
 * Self-contained, provisional Gray Fox prison greybox.
 * Original MSX2 event structure; J2ME room geometry has NOT been verified.
 */
UENUM(BlueprintType)
enum class EMGFacilitySecurityState : uint8
{
    Normal, Caution, Alert, Evasion, StandDown
};

UCLASS()
class MGREMASTER_API AMGGrayFoxMission : public AActor
{
    GENERATED_BODY()
public:
    AMGGrayFoxMission();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    bool TryInteract(AMGPlayerCharacter* Player);
    bool TryPunchRecoveryDoor(AMGPlayerCharacter* Player);
    bool TryPunchWeakWall(AMGPlayerCharacter* Player);
    bool IsPreludeActive() const { return (Flags & MG_FOX_CAPTURED) == 0; }
    // Shared spatial truth for packaging: real receiving lane or stored cargo
    // may be credible; a lone carton in a detention corridor is not.
    // This is an authored map query, NOT permission to see Snake through walls.
    bool IsExpectedFreightLocation(const FVector& WorldPosition) const;
    bool IsWorkingFreightConveyor(const FVector& WorldPosition) const;
    bool HasDetentionIntel() const { return (Flags & MG_FOX_DETENTION_INTELLIGENCE) != 0; }
    bool IsBossTelegraphing() const;
    bool IsTransmitterActive() const;
    bool DiscardTransmitter(AMGPlayerCharacter* Player);
    bool HasEquipmentConfiscated() const;
    void NotifyGuardContactReported(const FVector& LastSeenPosition);
    void NotifyGuardBodyReported(const FVector& EvidencePosition);
    UFUNCTION(BlueprintPure, Category="Mission|Security")
    EMGFacilitySecurityState GetFacilitySecurityState() const { return FacilitySecurityState; }
    UFUNCTION(BlueprintImplementableEvent, Category="Mission|Security")
    void OnFacilitySecurityStateChanged(EMGFacilitySecurityState Previous,
                                        EMGFacilitySecurityState Current);
    uint32 GetMissionFlags() const { return Flags; }
    FString GetObjectiveText() const;
    FString GetPacingHint() const;
    FString GetRecentDialogue() const;
    // Optional 1995-era transceiver: dialogue is remake-authored, not original transcription.
    FString GetTransceiverDialogue(int32 Channel) const;
    // Stable optional voice asset scene identifier, independent of transient
    // dialogue presentation timing or Blueprint cinematic completion.
    FString GetTransceiverVoiceSceneKey(int32 Channel) const;
    bool HasIncomingTransceiverPrompt() const;
    FString GetMissionLogText() const;
    // Progression of the 1995 rookie's perspective and relationships. Derived
    // from ACTUAL mission flags, never from a cinematic or a secret timer.
    UFUNCTION(BlueprintPure, Category="Mission|Story")
    FString GetCharacterStoryText() const;
    // Veteran knowledge earned by optional listening; uses actual nearby guard movement.
    FString GetFoxPatrolListeningCue() const;
    UFUNCTION(BlueprintPure, Category="Mission|Story")
    FName GetSnakeStoryPhase() const;
    FString GetPhaseText() const;
    UFUNCTION(BlueprintPure, Category="Mission|Pacing")
    float GetSuggestedBeatIntensity() const;
    // Presentation-only callbacks for sound design, lighting and Sequencer;
    // the visual/audio timeline must NEVER write gameplay flags.
    UFUNCTION(BlueprintImplementableEvent, Category="Mission|Pacing")
    void OnPacingBeatChanged(int32 PreviousPhase, int32 NewPhase, float SuggestedIntensity);
    UFUNCTION(BlueprintImplementableEvent, Category="Mission|Environment")
    void OnSurfaceImpact(FName SurfaceTag, int32 ContactIndex, bool bOpened);
    // Visual lip/jaw and expression cue from actual imported voice playback.
    // Mission rescue state is authoritative and never depends on clip length.
    UFUNCTION(BlueprintImplementableEvent, Category="Mission|Voice")
    void OnNarrativeVoiceStarted(FName Performer, FName ClipId, float ClipSeconds);
    void RestartMission();
    void ExhaleSmoke(const AMGPlayerCharacter* Player);
    void SkipStoryCinematic();
    UFUNCTION(BlueprintPure, Category="Mission|Story")
    bool IsStoryCinematicPlaying() const;
    UFUNCTION(BlueprintPure, Category="Mission|Story")
    FName GetActiveStoryCinematic() const;
    void NotifyBodyRelocated(AMGPlayerCharacter* Player);

private:
    UPROPERTY(Transient) AMGStoryCameraDirector* StoryCamera = nullptr;
    // The true actor remains at the real custody post until the short filmed
    // surrender completes or is skipped; mission state authority is HERE.
    bool bCapturePending = false;
    bool bCaptureSkipped = false;
    TWeakObjectPtr<AMGPlayerCharacter> PendingCapturePlayer;
    bool StartStoryCinematic(FName Scene, AMGPlayerCharacter* Player, AActor* Focus);
    TWeakObjectPtr<AMGSurveillanceCameraActor> PrisonCamera;
    TWeakObjectPtr<AMGSurveillanceCameraActor> KennelCamera;
    bool bHeardFoxFollowup = false;
    bool bOwnsCarton = false;
    bool bHeardFreightClerk = false;
    TWeakObjectPtr<AMGSurveillanceCameraActor> DispatchCamera;
    UPROPERTY(Transient) AStaticMeshActor* CartonPickup = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* WornCarton = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* SmokePuff = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* BeamSegment = nullptr;
    FVector SmokeOrigin = FVector::ZeroVector;
    float SmokeAge = 3.f;
    UPROPERTY(Transient) AStaticMeshActor* ServiceWall = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* ChargeParcel = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* ChargeMarker = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* RelayDesk = nullptr;
    bool bServiceChargeCollected = false;
    bool bServiceWallOpen = false;
    bool bRelayIntel = false;
    bool bKidDefeated = false;
    bool bParachuteTaken = false;
    UPROPERTY(Transient) AMGMachinegunKid* MachinegunKid = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* ParachutePack = nullptr;
    float BreachRemaining = -1.f;
    bool TryServiceInteraction(AMGPlayerCharacter* Player);
    void CreateDispatchBay();
    void TickDispatch(AMGPlayerCharacter* Player, float Dt);
    UPROPERTY(Transient)
    UStaticMesh* BlockMesh = nullptr;
    UPROPERTY(Transient)
    AStaticMeshActor* WeakWall = nullptr;
    UPROPERTY(Transient)
    AStaticMeshActor* EscapeWall = nullptr;
    UPROPERTY(Transient)
    AActor* FoxProxy = nullptr;
    UPROPERTY(Transient)
    AStaticMeshActor* ExitMarker = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* POWInformant = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* EarlyHandgunPickup = nullptr;
    bool bEarlyHandgunAcquired = false;
    UPROPERTY(Transient) AStaticMeshActor* CustodyMarker = nullptr;
    UPROPERTY(Transient) AMGGuardActor* CustodyOfficerA = nullptr;
    UPROPERTY(Transient) AMGGuardActor* CustodyOfficerB = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* FirstWallCrack = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* SecondWallCrack = nullptr;
    int32 FirstWallStrikes = 0;
    int32 SecondWallStrikes = 0;
    UPROPERTY(Transient) AStaticMeshActor* GearDoor = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* GearBag = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* PlantedTransmitter = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* Card2Door = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* Card3Pickup = nullptr;
    UPROPERTY(Transient) AStaticMeshActor* ChapterExit = nullptr;
    UPROPERTY(Transient) AMGShootGunnerActor* Shotmaker = nullptr;
    UPROPERTY(Transient) APointLight* BossThreatLight = nullptr;
    int32 RecoveryDoorPunches = 0;
    bool bGearDoorOpen = false;
    bool bCardDoorOpen = false;
    bool bPostCard3ServiceEntered = false;

    // Optional physical evidence: no puzzle authority, no secret reveal.
    struct FOptionalWorldTrace
    {
        TWeakObjectPtr<AStaticMeshActor> Prop;
        FString Observation;
        uint32 RequiredFlags = 0u;
        uint32 ForbiddenFlags = 0u;
    };
    TArray<FOptionalWorldTrace> OptionalWorldTraces;
    void AddWorldTrace(FName Tag, const FVector& Place, const FVector& Size,
        const FString& Observation, uint32 RequiredFlags = 0u,
        uint32 ForbiddenFlags = 0u);
    bool TryInspectWorldTrace(AMGPlayerCharacter* Player);

    uint32 Flags = 0;
    bool bPlayerInitialised = false;
    bool bHasLoadedSave = false;
    bool bUnderAlert = false;
    EMGFacilitySecurityState FacilitySecurityState = EMGFacilitySecurityState::Normal;
    float FacilityAlertRemaining = 0.f;
    float FacilityEvasionRemaining = 0.f;
    float FacilityCautionRemaining = 0.f;
    float FacilityStandDownRemaining = 0.f;
    FVector LastFacilityEvidence = FVector::ZeroVector;
    void SetFacilitySecurityState(EMGFacilitySecurityState NewState);
    float ElapsedSinceBeat = 0.f;
    // Hints are offered when exploration stalls, not simply after entering a room.
    float SecondsSinceExploration = 0.f;
    FVector LastObservedPlayerLocation = FVector::ZeroVector;
    FRotator LastObservedViewRotation = FRotator::ZeroRotator;
    bool bHasObservedPlayer = false;
    float DialogueTimeRemaining = 0.f;
    FVector SavedPosition = FVector::ZeroVector;
    uint32 LoadedSchemaVersion = 0;
    int32 LoadedHandgunRounds = 0;
    bool bLoadedHandgunAcquired = false;
    int32 LoadedStunRounds = 0;
    float BeaconPulseTimer = 0.f;
    int32 LastPresentedPacingPhase = INDEX_NONE;
    FString Dialogue;

    AStaticMeshActor* SpawnBlock(FName Tag, const FVector& Centre,
                                  const FVector& Scale, bool bBlockMovement);
    void CreateGreybox();
    void CreatePrelude();
    void CreateBossWing();
    void CreatePostCard3Service();
    void CreateConnectiveSpaces();
    void CreateFirstPassVerticality();
    void CreateSurveillancePosts();
    void CreateZoneLighting();
    void UpdateZoneLighting(float DeltaSeconds);
    void SpawnPerimeterGuards();
    void BroadcastStructuralImpact(const FVector& Location,
                                   float AudibleRadius, AActor* Source);
    void InitializePlayerIfReady();
    bool ApplyAction(uint32 Action, AMGPlayerCharacter* Player);
    void RestoreVisualState();
    void SaveProgress(AMGPlayerCharacter* Player);
    bool CanReach(const AMGPlayerCharacter* Player, const AActor* Target,
                  float MaxDistanceCm) const;
    uint32 GetPacingPhase() const;
    uint32 GetHintLevel() const;
    void UpdateGuardAlert(float DeltaSeconds);
    void NotifyPacingPhaseIfChanged();
};
