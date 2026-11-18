#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGRemasterGameMode.generated.h"

class UStaticMesh;

UCLASS()
class MGREMASTER_API AMGRemasterGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AMGRemasterGameMode();
    virtual void StartPlay() override;

private:
    UPROPERTY()
    UStaticMesh* BlockMesh;

    void PlaceBlock(const FVector& Location, const FVector& Scale);
};
