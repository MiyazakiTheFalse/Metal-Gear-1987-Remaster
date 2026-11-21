#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGPrisonerCharacter.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;

/** Gray Fox's prisoner body is a visual actor, never the owner of rescue flags. */
UCLASS()
class MGREMASTER_API AMGPrisonerCharacter : public AActor
{
    GENERATED_BODY()
public:
    AMGPrisonerCharacter();
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="Character|Art")
    void SetRescued(bool bInRescued);
    UFUNCTION(BlueprintPure, Category="Character|Art")
    bool IsRescued() const { return bRescued; }
    UFUNCTION(BlueprintPure, Category="Character|Art")
    bool HasAuthoredCharacterArt() const;

private:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* SceneRoot;
    UPROPERTY(VisibleAnywhere, Category="Character|Art")
    USkeletalMeshComponent* CharacterMesh;
    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* PlaceholderBody;
    bool bRescued = false;
    FName PrototypeAnimationRequest = NAME_None;
    void UpdatePrototypeFallbackAnimation();
};
