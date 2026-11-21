#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGBodyHidingSpot.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class AMGGuardActor;

/** Physical-looking maintenance locker for the later-series MGS body-hiding
 * interaction. Aimed at utility spaces: not a teleporting arbitrary hide zone.
 */
UCLASS()
class MGREMASTER_API AMGBodyHidingSpot : public AActor
{
    GENERATED_BODY()
public:
    AMGBodyHidingSpot();
    virtual void Tick(float DeltaSeconds) override;
    bool IsOccupied() const { return bOccupied; }
    bool IsAccessible() const { return !bOccupied && !bOpening; }
    bool TryStow(AMGGuardActor* Body);
    void SetFacilityLabel(const FString& Text);
private:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ShellRear;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ShellLeft;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ShellRight;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ShellTop;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* BasePlate;
    UPROPERTY(VisibleAnywhere)
    USceneComponent* DoorHinge;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* DoorPlate;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* DoorHandle;
    UPROPERTY(VisibleAnywhere)
    UTextRenderComponent* FacilitySign;
    UPROPERTY()
    AMGGuardActor* StoredBody = nullptr;
    bool bOccupied = false;
    bool bOpening = false;
    bool bClosing = false;
    float DoorSwing = 0.f;
    float LoadingHold = 0.f;
};
