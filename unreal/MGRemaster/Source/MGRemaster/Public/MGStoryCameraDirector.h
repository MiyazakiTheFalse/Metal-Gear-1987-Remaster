#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGStoryCameraDirector.generated.h"

class UCameraComponent;
class AMGPlayerCharacter;

/**
 * Real, short in-world cinematic camera with no authority over gameplay.
 * Story beats are driven by MGGrayFoxMission or the source campaign state
 * machine; the camera may ONLY present a successful event. If its subject is
 * missing, camera setup fails safely and gameplay proceeds without it.
 *
 * Authored 1995 shots are replaceable by approved UE Sequencer LevelSequences.
 * This class is a running procedural bridge, NOT production facial acting.
 */
UCLASS()
class MGREMASTER_API AMGStoryCameraDirector : public AActor
{
    GENERATED_BODY()
public:
    AMGStoryCameraDirector();
    virtual void Tick(float DeltaSeconds) override;

    bool PlayScene(FName SceneId, AMGPlayerCharacter* Player, AActor* Subject);
    void SkipScene();
    UFUNCTION(BlueprintPure, Category="Cinematic")
    bool IsPlaying() const { return bPlaying; }
    UFUNCTION(BlueprintPure, Category="Cinematic")
    FName GetActiveScene() const { return ActiveScene; }
    UFUNCTION(BlueprintPure, Category="Cinematic")
    float GetSceneProgress() const { return MaxSeconds > 0.f
        ? FMath::Clamp(ElapsedSeconds/MaxSeconds, 0.f, 1.f) : 1.f; }
    UFUNCTION(BlueprintImplementableEvent, Category="Cinematic")
    void OnStorySceneStarted(FName SceneId);
    UFUNCTION(BlueprintImplementableEvent, Category="Cinematic")
    void OnStorySceneEnded(FName SceneId, bool bSkipped);
private:
    UPROPERTY(VisibleAnywhere, Category="Cinematic")
    UCameraComponent* Lens = nullptr;
    TWeakObjectPtr<AMGPlayerCharacter> ActingPlayer;
    TWeakObjectPtr<AActor> FocusActor;
    FName ActiveScene = NAME_None;
    TSet<FName> ShownSceneIds;
    float ElapsedSeconds = 0.f;
    float MaxSeconds = 0.f;
    float RadiusCm = 300.f;
    float YawDegrees = 170.f;
    float EyeRaiseCm = 90.f;
    float SubjectEyeCm = 100.f;
    float PanDegreesPerSecond = 4.f;
    bool bPlaying = false;
    bool ConfigureScene(FName SceneId);
    void UpdateShot(float DeltaSeconds, bool bSnap);
    void FinishScene(bool bSkipped);
};
