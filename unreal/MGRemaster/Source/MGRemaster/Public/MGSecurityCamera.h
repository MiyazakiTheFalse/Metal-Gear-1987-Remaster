#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSecurityCamera.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 * Prototype diegetic CCTV sensor with obstruction-aware, accumulating sight.
 * This is a fictional gameplay interpretation, not exact J2ME camera scripting.
 */
UCLASS()
class MGREMASTER_API AMGSecurityCamera : public AActor
{
    GENERATED_BODY()
public:
    AMGSecurityCamera();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& Event,
                             AController* Instigator, AActor* DamageCauser) override;
    void Configure(FName StableId, int32 DeckNumber, float SweepDegrees = 42.f);
    bool IsAlarmActive() const { return !bDisabled && AlertSeconds > 0.f; }
    float GetDetectionProgress() const { return Suspicion; }
    int32 GetDeck() const { return Deck; }
private:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* SceneRoot;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Housing;
    bool bDisabled = false;
    int32 Deck = 3;
    float BaseYaw = 0.f;
    float SweepAmplitude = 42.f;
    float ScanPhase = 0.f;
    float Suspicion = 0.f;
    float AlertSeconds = 0.f;
    float AlertCooldown = 0.f;
    void ReportConfirmedSighting(const FVector& LastSeenLocation);
};
