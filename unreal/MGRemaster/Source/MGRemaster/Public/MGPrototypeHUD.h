#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MGPrototypeHUD.generated.h"

UCLASS()
class MGREMASTER_API AMGPrototypeHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
