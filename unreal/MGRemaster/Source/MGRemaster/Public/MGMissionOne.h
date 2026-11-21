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
    bool TryInteract(AMGPlayerCharacter* Player);
    FVector EntryLocation() const { return FVector(700.f, 700.f, 90.f); }
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
    AStaticMeshActor* Block(FName Tag, FVector Position, FVector Scale, bool bCollision = true);
    void Build();
    void SpawnGuards();
    void AddPickup(FName Id, FName Item, FVector Metres, bool bWeapon);
};
