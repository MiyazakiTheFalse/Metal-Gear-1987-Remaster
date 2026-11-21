#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGMissionOne.generated.h"

class AMGPlayerCharacter;
class AStaticMeshActor;
class UStaticMesh;
class UMGCampaignState;

/**
 * Mission 1 authored greybox. Coordinates are provisional remake metres.
 * This is not a verified reproduction of J2ME rooms or a final art pass.
 */
UCLASS()
class MGREMASTER_API AMGMissionOne : public AActor
{
    GENERATED_BODY()
public:
    AMGMissionOne();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    bool TryInteract(AMGPlayerCharacter* Player);
    /** Called only when a physical remote missile strikes the actual panel. */
    bool DisableElectricalSupply();
    // Called after ammunition use/consumable use to persist spent stock.
    void PersistInventory(AMGPlayerCharacter* Player) { SaveProgress(Player); }
    void RestoreSavedEquipment(AMGPlayerCharacter* Player);
    FString GetObjectiveText() const;
    FString GetRecentDialogue() const { return BeatSubtitleTime > 0.f ? BeatSubtitle : FString(); }
    bool CycleLiftDestination(AMGPlayerCharacter* Player);
    int32 GetCurrentFloor() const { return CurrentFloor; }
    int32 GetSelectedFloor() const { return SelectedFloor; }
    bool IsLiftTravelling() const { return bLiftTravelling; }
    bool HasReachedElevator() const { return bElevatorReached; }
    FVector EntryLocation() const { return FVector(700.f, 700.f, 90.f); }
    FVector ResumeLocation() const { return CurrentFloor == 1 ? EntryLocation() : FloorLiftLocation(CurrentFloor); }
private:
    struct FPickup
    {
        FName Id;
        FName Item;
        FVector Location;
        bool bWeapon;
        bool bTaken = false;
        AStaticMeshActor* Marker = nullptr;
    };
    TArray<FPickup> Pickups;
    UPROPERTY()
    UStaticMesh* BlockMesh = nullptr;
    UPROPERTY()
    AStaticMeshActor* CardDoor = nullptr;
    bool bCardDoorOpened = false;
    bool bElevatorReached = false;
    bool bPOWRescued = false;
    bool bDianePOWRescued = false;
    bool bFoxLocationPOWRescued = false;
    bool bCaptureMethodPOWRescued = false;
    bool bRCStoreDoorOpened = false;
    bool bCardTwoDoorOpened = false;
    bool bFoxLocationDoorOpened = false;
    bool bCaptureMethodDoorOpened = false;
    bool bDianeDoorOpened = false;
    bool bElectricPanelDisabled = false;
    float ElectricDamageAccum = 0.f;
    float ElectricWarningCooldown = 0.f;
    UPROPERTY()
    AStaticMeshActor* CardTwoDoor = nullptr;
    UPROPERTY()
    AStaticMeshActor* RCStoreDoor = nullptr;
    UPROPERTY()
    AStaticMeshActor* FoxLocationDoor = nullptr;
    UPROPERTY()
    AStaticMeshActor* CaptureMethodDoor = nullptr;
    UPROPERTY()
    AStaticMeshActor* DianeDoor = nullptr;
    UPROPERTY()
    AStaticMeshActor* ElectricPanel = nullptr;
    UPROPERTY()
    AStaticMeshActor* DianePrisoner = nullptr;
    UPROPERTY()
    AStaticMeshActor* FoxLocationPrisoner = nullptr;
    UPROPERTY()
    AStaticMeshActor* CaptureMethodPrisoner = nullptr;
    float BeatSubtitleTime = 0.f;
    FString BeatSubtitle;
    bool bPrisonerDoorOpened = false;
    bool bGasDoorOpened = false;
    UPROPERTY()
    AStaticMeshActor* PrisonerDoor = nullptr;
    UPROPERTY()
    AStaticMeshActor* CaptureMarker = nullptr;
    UPROPERTY()
    AStaticMeshActor* GasDoor = nullptr;
    float GasWarningCooldown = 0.f;
    float GasDamageAccum = 0.f;
    int32 SavedHandgunRounds = -1;
    int32 SavedRCRounds = -1;
    int32 SavedMineCount = -1;
    int32 SavedRationsRemaining = -1;
    bool bMissionThreeCacheClaimed = false;
    UPROPERTY()
    AStaticMeshActor* M03Manifest = nullptr;
    UPROPERTY()
    AStaticMeshActor* M03Survey = nullptr;
    UPROPERTY()
    AStaticMeshActor* M03Archive = nullptr;
    UPROPERTY()
    AStaticMeshActor* M03Cache = nullptr;
    UPROPERTY()
    AStaticMeshActor* M03Breaker = nullptr;
    UPROPERTY()
    AStaticMeshActor* M03ExteriorConnector = nullptr;
    bool bLiftTravelling = false;
    float LiftTravelRemaining = 0.f;
    int32 CurrentFloor = 1;
    int32 SelectedFloor = 3;
    int32 DestinationFloor = 1;
    TWeakObjectPtr<AMGPlayerCharacter> LiftPassenger;
    static FVector FloorLiftLocation(int32 Floor);
    bool StartLift(AMGPlayerCharacter* Player);
    void FinishLift();
    bool IsNearLift(const AMGPlayerCharacter* Player) const;
    void BuildUpperFloors();
    void BuildMissionThreeInvestigation();
    bool TryMissionThreeInvestigation(AMGPlayerCharacter* Player, UMGCampaignState* Story);
    FVector SavedCheckpoint = FVector(700.f, 700.f, 90.f);
    bool SaveProgress(AMGPlayerCharacter* Player);
    AStaticMeshActor* Block(FName Tag, FVector Position, FVector Scale, bool bCollision = true);
    void Build();
    void SpawnGuards();
    void AddPickup(FName Id, FName Item, FVector Metres, bool bWeapon);
};
