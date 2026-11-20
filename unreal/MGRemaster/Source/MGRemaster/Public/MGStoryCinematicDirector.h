#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGStoryCinematicDirector.generated.h"

class ACameraActor;
class APlayerController;

/**
 * Physical, real-time, skippable story cameras for the current 1995 source
 * prototype. Gameplay commits all story flags independently BEFORE calls to
 * PlayScene(); this actor can never give items, open doors, damage or save.
 *
 * Do not label this finished MGS-quality performance. Real faces, final
 * voice, authored Sequencer body acting and imported character art are
 * explicit production blockers. The editor may override shots/acting via BP.
 */
USTRUCT(BlueprintType)
struct FMGStoryCameraShot
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 1.0f;
    // Relative to actual actor forward/right in centimetres.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RelativeEye = FVector(-250.f, 165.f, 110.f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FocusHeight = 135.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FieldOfView = 52.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DollyCm = 0.f;
};

UCLASS()
class MGREMASTER_API AMGStoryCinematicDirector : public AActor
{
    GENERATED_BODY()
public:
    AMGStoryCinematicDirector();
    virtual void Tick(float DeltaSeconds) override;

    /** Returns false when no real player camera, world or subject is present. */
    bool PlayScene(FName SceneId, APlayerController* PlayerController,
                   AActor* Subject, bool bLockInput = true);
    bool SkipScene();
    bool IsPlaying() const { return bPlaying; }
    bool IsBlocking() const { return bPlaying && bLocksInput; }
    FName GetSceneId() const { return ActiveSceneId; }
    float GetProgress() const;

    UFUNCTION(BlueprintImplementableEvent, Category="Story|Cinematics")
    void OnCinematicSceneStarted(FName SceneId);
    UFUNCTION(BlueprintImplementableEvent, Category="Story|Cinematics")
    void OnCinematicShotChanged(FName SceneId, int32 ShotIndex);
    UFUNCTION(BlueprintImplementableEvent, Category="Story|Cinematics")
    void OnCinematicSceneEnded(FName SceneId, bool bSkipped);
private:
    TWeakObjectPtr<APlayerController> Controller;
    TWeakObjectPtr<AActor> SubjectActor;
    TWeakObjectPtr<AActor> PreviousViewTarget;
    UPROPERTY(Transient)
    TObjectPtr<ACameraActor> FilmCamera = nullptr;
    TArray<FMGStoryCameraShot> Shots;
    FName ActiveSceneId = NAME_None;
    float TimeInScene = 0.f;
    float TotalTime = 0.f;
    int32 LastShotIndex = INDEX_NONE;
    bool bPlaying = false;
    bool bLocksInput = false;

    void StopScene(bool bSkipped);
    void ApplyShot(int32 ShotIndex, float IntoShot, float DeltaSeconds);
    bool BuildScene(FName Id);
};
