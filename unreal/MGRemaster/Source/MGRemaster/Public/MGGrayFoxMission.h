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

    UPROPERTY() AStaticMeshActor* Informant = nullptr;
    UPROPERTY() AStaticMeshActor* Sentry = nullptr;
    UPROPERTY() AStaticMeshActor* Locker = nullptr;
    UPROPERTY() AStaticMeshActor* Gate = nullptr;
    UPROPERTY() AStaticMeshActor* ArmouryGate = nullptr;
    UPROPERTY() AStaticMeshActor* FinalGate = nullptr;
    UPROPERTY() class AMGGuardActor* Shotmaker = nullptr;
    UPROPERTY() TArray<AStaticMeshActor*> WallScars;
    UPROPERTY() class UMGGrayFoxSaveGame* Loaded = nullptr;
    uint8 FirstHits = 0;
    uint8 ExitHits = 0;
    float NextStrike = 0.f;
    float FoxSpeech = -1.f;
    bool StrikeWall(AMGPlayerCharacter* Player, AStaticMeshActor* Wall, uint8& Hits, uint32 Action);
    void ShowScar(AStaticMeshActor* Wall, uint8 Hit);
    void AddLight(const FVector& Position, FLinearColor Colour, float Radius);
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
    FString Dialogue;

    AStaticMeshActor* SpawnBlock(FName Tag, const FVector& Centre,
                                  const FVector& Scale, bool bBlockMovement);
    void CreateGreybox();
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
