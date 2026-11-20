#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSurveillanceCameraActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class AMGPlayerCharacter;

/** Remake-specific 1995 analogue CCTV: an optional diegetic security system.
 * Optical evidence only. A camera has no authority to update mission state
 * until it has physically seen the player for a readable lock period.
 */
UCLASS(Blueprintable)
class MGREMASTER_API AMGSurveillanceCameraActor : public AActor
{
    GENERATED_BODY()
public:
    AMGSurveillanceCameraActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float Amount, const FDamageEvent& Event,
        AController* Instigator, AActor* Causer) override;

    /** Root yaw faces surveillance sector. Sweeps around that initial heading. */
    void ConfigureSector(float HalfAngleDegrees, float RangeCm);
    void RestoreOfflineState();
    bool TryDisableAtControlPanel(const AMGPlayerCharacter* Player);
    FVector GetControlPanelWorldLocation() const;
    UFUNCTION(BlueprintPure, Category="MG|Security|Camera")
    bool IsOperational() const { return bOperational; }
    UFUNCTION(BlueprintPure, Category="MG|Security|Camera")
    bool IsTrackingPlayer() const { return bTrackingPlayer && bOperational; }
    UFUNCTION(BlueprintPure, Category="MG|Security|Camera")
    bool HasReportedIntruder() const { return bReportedIntruder && bOperational; }
    UFUNCTION(BlueprintPure, Category="MG|Security|Camera")
    float GetLockFraction() const { return FMath::Clamp(VisualContactSeconds / 1.25f, 0.f, 1.f); }
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Security|Camera")
    void OnVisualContactAcquired();
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Security|Camera")
    void OnVisualContactLost();
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Security|Camera")
    void OnAlarmTransmitted(const FVector& EvidenceLocation);
    UFUNCTION(BlueprintImplementableEvent, Category="MG|Security|Camera")
    void OnCameraDisabled(bool bByInteraction);

private:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* Pivot;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mount;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Housing;
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ControlPanel;
    UPROPERTY(VisibleAnywhere)
    UPointLightComponent* StatusLamp;
    FRotator InitialWorldRotation = FRotator::ZeroRotator;
    float SweepHalfDegrees = 42.f;
    float DetectionRangeCm = 850.f;
    float VisualContactSeconds = 0.f;
    float ReacquirePauseSeconds = 0.f;
    float ReportCooldownSeconds = 0.f;
    FVector LastObservedWorldPosition = FVector::ZeroVector;
    bool bOperational = true;
    bool bTrackingPlayer = false;
    bool bReportedIntruder = false;
    bool CanActuallySee(const AMGPlayerCharacter* Player) const;
    void Disable(bool bByInteraction);
};
