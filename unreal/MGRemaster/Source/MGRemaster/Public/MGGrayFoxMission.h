#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGrayFoxMission.generated.h"

class AStaticMeshActor;
class AMGPlayerCharacter;
class AMGShootGunnerActor;
class UStaticMesh;

/**
 * Self-contained, provisional Gray Fox prison greybox.
 * Original MSX2 event structure; J2ME room geometry has NOT been verified.
 */
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
    bool HasDetentionIntel() const { return (Flags & MG_FOX_DETENTION_INTELLIGENCE) != 0; }
    bool IsBossTelegraphing() const;
    bool IsTransmitterActive() const;
    bool DiscardTransmitter(AMGPlayerCharacter* Player);
    bool HasEquipmentConfiscated() const;
    uint32 GetMissionFlags() const { return Flags; }
    FString GetObjectiveText() const;
    FString GetPacingHint() const;
    FString GetRecentDialogue() const;
    // Optional 1995-era transceiver: dialogue is remake-authored, not original transcription.
    FString GetTransceiverDialogue(int32 Channel) const;
    bool HasIncomingTransceiverPrompt() const;
    FString GetMissionLogText() const;
    FString GetPhaseText() const;
    void RestartMission();

private:
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
    UPROPERTY(Transient) AStaticMeshActor* CustodyMarker = nullptr;
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
    int32 RecoveryDoorPunches = 0;
    bool bGearDoorOpen = false;
    bool bCardDoorOpen = false;

    uint32 Flags = 0;
    bool bPlayerInitialised = false;
    bool bHasLoadedSave = false;
    bool bUnderAlert = false;
    float ElapsedSinceBeat = 0.f;
    // Hints are offered when exploration stalls, not simply after entering a room.
    float SecondsSinceExploration = 0.f;
    FVector LastObservedPlayerLocation = FVector::ZeroVector;
    FRotator LastObservedViewRotation = FRotator::ZeroRotator;
    bool bHasObservedPlayer = false;
    float DialogueTimeRemaining = 0.f;
    FVector SavedPosition = FVector::ZeroVector;
    uint32 LoadedSchemaVersion = 0;
    int32 LoadedHandgunRounds = 24;
    int32 LoadedStunRounds = 8;
    float BeaconPulseTimer = 0.f;
    FString Dialogue;

    AStaticMeshActor* SpawnBlock(FName Tag, const FVector& Centre,
                                  const FVector& Scale, bool bBlockMovement);
    void CreateGreybox();
    void CreatePrelude();
    void CreateBossWing();
    void SpawnPerimeterGuards();
    void InitializePlayerIfReady();
    bool ApplyAction(uint32 Action, AMGPlayerCharacter* Player);
    void RestoreVisualState();
    void SaveProgress(AMGPlayerCharacter* Player);
    bool CanReach(const AMGPlayerCharacter* Player, const AActor* Target,
                  float MaxDistanceCm) const;
    uint32 GetPacingPhase() const;
    uint32 GetHintLevel() const;
    void UpdateGuardAlert();
};
