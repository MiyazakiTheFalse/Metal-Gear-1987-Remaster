#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
class UTexture2D;
#include "MGPrototypeHUD.generated.h"

UCLASS()
class MGREMASTER_API AMGPrototypeHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
private:
    UTexture2D* FindPortraitTexture(const FString& CharacterId, const FString& Expression);
    // Cached 8 x 3 lightweight expression frames: no per-tick asset discovery.
    UPROPERTY(Transient)
    TMap<FString, TObjectPtr<UTexture2D>> CachedCodecPortraits;
    TSet<FString> MissingCodecPortraits;
};
