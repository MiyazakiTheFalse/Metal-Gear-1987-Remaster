#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGrayFoxMission.generated.h"

class AStaticMeshActor;
class AMGPlayerCharacter;
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
    AStaticMeshActor* FoxProxy = nullptr;
    UPROPERTY(Transient)
    AStaticMeshActor* ExitMarker = nullptr;
    UPROPERTY(Transient)
    AStaticMeshActor* GearLocker = nullptr;

    uint32 Flags = 0;
    bool bGearRecovered = false;
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
    float ReturnToOuterHeavenSeconds = -1.f;
    FVector SavedPosition = FVector::ZeroVector;
    FString Dialogue;

    AStaticMeshActor* SpawnBlock(FName Tag, const FVector& Centre,
                                  const FVector& Scale, bool bBlockMovement);
    void CreateGreybox();
    void SpawnPerimeterGuards();
    void InitializePlayerIfReady();
    bool ApplyAction(uint32 Action, AMGPlayerCharacter* Player);
    void RestoreVisualState();
    void SaveProgress(AMGPlayerCharacter* Player);
    void RestoreCapturedGear(AMGPlayerCharacter* Player);
    bool CanReach(const AMGPlayerCharacter* Player, const AActor* Target,
                  float MaxDistanceCm) const;
    uint32 GetPacingPhase() const;
    uint32 GetHintLevel() const;
    void UpdateGuardAlert();
};
