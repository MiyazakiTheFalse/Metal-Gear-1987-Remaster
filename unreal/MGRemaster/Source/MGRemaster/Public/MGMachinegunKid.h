#pragma once
#include "CoreMinimal.h"
#include "MGGuardActor.h"
#include "MGMachinegunKid.generated.h"

class AMGPlayerCharacter;

/** Fixed-lane boss prototype. Each burst commits to a marked lane. */
UCLASS()
class MGREMASTER_API AMGMachinegunKid : public AMGGuardActor
{
    GENERATED_BODY()
public:
    AMGMachinegunKid();
    virtual void Tick(float Dt) override;
private:
    UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Receiver;
    UPROPERTY(VisibleAnywhere) class UPointLightComponent* Flash;
    enum class EBeat : uint8 { Waiting, Sight, Burst, Reload };
    EBeat Beat = EBeat::Waiting;
    float Remaining = 0.f;
    float ShotInterval = 0.f;
    float MarkedY = 0.f;
};
