#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGLaterMission.generated.h"

class AStaticMeshActor;
class AMGPlayerCharacter;
class AMGSetpieceBoss;
class UStaticMesh;
class UMGCampaignState;

/**
 * Source-comparative greybox for the physically distinct post-Fox arenas:
 * Mission 3 exterior (Machinegun Kid / Hind D / tank);
 * Mission 4 Building Two approach (bulldozer / Fire Trooper / TX-11 pair);
 * Mission 5 Madnar -> Building One return for Ellen -> Madnar.
 *
 * The source-faithful room/tile geography from J2ME has not been decoded;
 * milestones are proper player-triggered actors, not cutscene skip toggles.
 */
UCLASS()
class MGREMASTER_API AMGLaterMission : public AActor
{
    GENERATED_BODY()
public:
    AMGLaterMission();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void RestorePlayerEquipment(AMGPlayerCharacter* Player);
    void PersistPlayerEquipment(AMGPlayerCharacter* Player);
    bool TryInteract(AMGPlayerCharacter* Player);
    void OnBossDefeated(FName BossId);
    void NotifyWrongWeapon(FName BossId, FName RequiredWeapon);
    FString GetObjectiveText() const;
    FString GetRecentDialogue() const { return DialogueSeconds > 0.f ? Dialogue : FString(); }
    int32 GetActiveMission() const { return Mission; }
private:
    struct FWorldPickup
    {
        FName Id;
        FName Kind;
        int32 Quantity = 1;
        AStaticMeshActor* Marker = nullptr;
        FName Prerequisite = NAME_None;
        bool bTaken = false;
    };
    UPROPERTY()
    UStaticMesh* Cube = nullptr;
    UPROPERTY()
    AStaticMeshActor* TransitionMarker = nullptr;
    UPROPERTY()
    AStaticMeshActor* ResearchPrisoner = nullptr;
    UPROPERTY()
    AStaticMeshActor* JenniferPrisoner = nullptr;
    UPROPERTY()
    AStaticMeshActor* RocketSupply = nullptr;
    UPROPERTY()
    AStaticMeshActor* EllenProxy = nullptr;
    UPROPERTY()
    AStaticMeshActor* WeakWall = nullptr;
    UPROPERTY()
    AStaticMeshActor* MadnarProxy = nullptr;
    UPROPERTY()
    AStaticMeshActor* PassageGateA = nullptr;
    UPROPERTY()
    AStaticMeshActor* PassageGateB = nullptr;
    UPROPERTY()
    AStaticMeshActor* PassageGateC = nullptr;
    UPROPERTY()
    AStaticMeshActor* PassageGateD = nullptr;
    UPROPERTY()
    AStaticMeshActor* RocketRoomDoor = nullptr;
    UPROPERTY()
    AStaticMeshActor* JenniferBrother = nullptr;
    UPROPERTY()
    AStaticMeshActor* HostageLeft = nullptr;
    UPROPERTY()
    AStaticMeshActor* HostageRight = nullptr;
    UPROPERTY()
    AStaticMeshActor* FinalWitness = nullptr;
    UPROPERTY()
    AStaticMeshActor* DecisionPost = nullptr;
    UPROPERTY()
    AStaticMeshActor* TX55LeftFoot = nullptr;
    UPROPERTY()
    AStaticMeshActor* TX55RightFoot = nullptr;
    UPROPERTY()
    AStaticMeshActor* LeftEscapeLadder = nullptr;
    UPROPERTY()
    AStaticMeshActor* RightEscapeLadder = nullptr;
    float ChargePlacementRecovery = 0.f;
    float EscapeSaveAccumulator = 0.f;
    TArray<FWorldPickup> Pickups;
    int32 Mission = 3;
    float DialogueSeconds = 0.f;
    float WrongWeaponCooldown = 0.f;
    FString Dialogue;
    FVector SpawnPoint = FVector(650.f,1100.f,90.f);
    FString SavedSlot = TEXT("MissionOneV1");
    UMGCampaignState* Story() const;
    AStaticMeshActor* Block(FName Id, FVector Center, FVector Scale,
                            bool bSolid = true);
    void AddPickup(FName Id, FName Type, FVector Location, int32 Count,
                   FName Prerequisite = NAME_None);
    void SpawnBoss(FName Id, FVector Location, FName Weakness, float Health,
                   float Radius, float Telegraph, float Power);
    void BuildExterior();
    void BuildBuildingTwo();
    void BuildEllenReturn();
    void BuildDesertAndCowardDuck();
    void BuildDeepBunker();
    void BuildTX55Finale();
    void RemoveDefeatedGates();
    void SetSubtitle(const FString& Text, float Seconds = 7.f);
    bool RescuePOW(AStaticMeshActor*& Marker, FName Beat, const FString& Intel,
                   AMGPlayerCharacter* Player);
    bool ClaimPickup(FWorldPickup& Pickup, AMGPlayerCharacter* Player);
    bool IsWithin(const AMGPlayerCharacter* Player, const AActor* Actor,
                  float Distance = 210.f) const;
    bool SavePickupState(FName Id, AMGPlayerCharacter* Player);
    void AdvanceAndReload(int32 NextMission);
};