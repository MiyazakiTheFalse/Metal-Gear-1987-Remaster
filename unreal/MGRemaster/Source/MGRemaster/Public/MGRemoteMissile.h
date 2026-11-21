#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGRemoteMissile.generated.h"

class UStaticMeshComponent;

/** Player-steered, expendable 1987 remote missile prototype.
 * Snake stays in place until the missile hits something or runs out of fuel.
 */
UCLASS()
class MGREMASTER_API AMGRemoteMissile : public AActor
{
    GENERATED_BODY()
public:
    AMGRemoteMissile();
    virtual void Tick(float DeltaSeconds) override;
    void SetSteering(float YawAxis, float PitchAxis);
private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Body = nullptr;
    float YawSteer = 0.f;
    float PitchSteer = 0.f;
    float FuelRemaining = 5.f;
    float MoveSpeed = 1250.f;
};