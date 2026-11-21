#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGAdaptiveAudioDirector.generated.h"
class UAudioComponent;
class USoundBase;
class AMGPlayerCharacter;

/**
 * 1995 original-score bus shared by all 8 planned Unreal missions.
 * SoundWaves are imported by tools/unreal_import_audio.py into /Game/Audio.
 * Music reacts to actual observed enemies, gameplay mission and player state.
 * It does NOT invent source-game original music or play absent audio files.
 */
UCLASS()
class MGREMASTER_API AMGAdaptiveAudioDirector : public AActor
{
    GENERATED_BODY()
public:
    AMGAdaptiveAudioDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Audio")
    bool PlayCue(FName CueId, FVector WorldOrigin, bool bSpatial=false);
    UFUNCTION(BlueprintCallable, Category="Audio")
    void SetBusLevels(float NewMusic, float NewEffects);
    FString GetCurrentCueName() const { return ActiveMusic.ToString(); }
private:
    UPROPERTY()
    UAudioComponent* MusicA = nullptr;
    UPROPERTY()
    UAudioComponent* MusicB = nullptr;
    FName ActiveMusic = NAME_None;
    TMap<FName,double> CueLastPlayed;
    UPROPERTY(Transient)
    TMap<FName,USoundBase*> Cache;
    float MusicGain = 0.23f;
    float EffectsGain = 0.75f;
    float StepClock = 0.f;
    float LastHealth = 100.f;
    int32 PreviousAwareness = 0;
    bool bPreviousCodec = false;
    bool bPreviousGameOver = false;
    bool bHasHealthSnapshot = false;

    FName DetermineMusic(const AMGPlayerCharacter* Snake) const;
    void TransitionTo(FName Cue);
    USoundBase* FindSound(FName Cue, bool bMusic);
    void ProcessPlayerFoley(float Dt, const AMGPlayerCharacter* Snake);
    FName SurfaceStepCue(const AMGPlayerCharacter* Snake) const;
};