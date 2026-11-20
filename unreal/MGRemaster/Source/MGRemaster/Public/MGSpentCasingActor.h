#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSpentCasingActor.generated.h"

class UStaticMeshComponent;

/** Optional cosmetic *physics* casing/hull, spawned only after a committed
 * live ballistic shot/cycle. Visuals cannot mutate ammunition, AI or damage.
 * If imported art is missing no invisible collision actor is retained.
 */
UENUM()
enum class EMGCasingType : uint8 { HandgunBrass, RiotGunHull };

UCLASS()
class MGREMASTER_API AMGSpentCasingActor : public AActor
{
    GENERATED_BODY()
public:
    AMGSpentCasingActor();
    // Does not spawn anything if the source mesh is not imported.
    static void Eject(UWorld* World, EMGCasingType Type, const FVector& Location,
        const FRotator& WeaponRotation);
private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Body;
    bool InitialiseCase(EMGCasingType Type, const FRotator& WeaponRotation);
};
