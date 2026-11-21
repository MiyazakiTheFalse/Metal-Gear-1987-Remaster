#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGMissionOne.generated.h"

class AMGPlayerCharacter;
class AStaticMeshActor;
class UStaticMesh;

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
    // Called after ammunition use/consumable use to persist spent stock.
    void PersistInventory(AMGPlayerCharacter* Player) { SaveProgress(Player); }
    void RestoreSavedEquipment(AMGPlayerCharacter* Player);
    FString GetObjectiveText() const;
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
    int32 SavedRationsRemaining = -1;
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
    FVector SavedCheckpoint = FVector(700.f, 700.f, 90.f);
    void SaveProgress(AMGPlayerCharacter* Player);
    AStaticMeshActor* Block(FName Tag, FVector Position, FVector Scale, bool bCollision = true);
    void Build();
    void SpawnGuards();
    void AddPickup(FName Id, FName Item, FVector Metres, bool bWeapon);
};
