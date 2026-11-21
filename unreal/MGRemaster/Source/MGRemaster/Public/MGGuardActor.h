#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGuardActor.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;

UCLASS()
class MGREMASTER_API AMGGuardActor : public AActor
{
    GENERATED_BODY()
public:
    AMGGuardActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // Route waypoints are placed by authored Mission 1 data, not generated at runtime.
    void ConfigurePatrol(const TArray<FVector>& WorldWaypoints)
    {
        PatrolWaypoints = WorldWaypoints;
        PatrolWaypointIndex = PatrolWaypoints.Num() > 1 ? 1 : 0;
    }
    /** Every deployed sentry has an authored duty, an actual protected object
     * and a local route, rather than a random enemy spawn. */
    void ConfigureDuty(FName DutyId, FName DutyName, FName ProtectedObject,
                       const FVector& AssignmentAnchor, float StopSeconds,
                       const FVector& AssetFocusPoint);
    FName GetDutyId() const { return AssignedDutyId; }
    FName GetProtectedObject() const { return AssignedObject; }
    // A confirmed sighting shared by a local, functioning CCTV system.
    void ReceiveCameraAlert(const FVector& SightLocation, float DurationSeconds);
    bool IsIncapacitated() const { return bIncapacitated; }
    bool IsBeingChoked() const { return bBeingChoked; }
    bool IsDiscoverableBody() const { return bIncapacitated && !bBodyConcealed; }
    bool IsBodyDragged() const { return bBodyDragged; }
    // Hit animation response: the first two strikes stagger, the kick knocks
    // down. The AI cannot shoot or move during a genuine knockdown.
    void ReactToMeleeStrike(bool bFinishingKick, const FVector& StrikeSource);
    UFUNCTION(BlueprintPure, Category="Animation|Combat")
    bool IsMeleeKnockedDown() const { return MeleeKnockdownSeconds > 0.f; }
    UFUNCTION(BlueprintPure, Category="Animation|Combat")
    float GetMeleeImpactSeconds() const { return StaggerSeconds; }
    bool BeginChoke(AActor* Instigator);
    void ReleaseChoke(bool bAlertOnRelease);
    void CompleteChoke();
    void SetBodyDragged(bool bDragging);
    bool MoveDraggedBody(const FVector& DesiredLocation, float DeltaSeconds);
    void ConcealBody(AActor* Container);
    // Optional skeletal actors can consume these authored animation states.
    UFUNCTION(BlueprintPure, Category="Stealth|Body")
    bool IsInStealthChokePose() const { return bBeingChoked; }
    UFUNCTION(BlueprintPure, Category="Stealth|Body")
    bool IsBodyCarriedBySnake() const { return bBodyDragged; }
    uint32 GetAwarenessState() const { return AwarenessState; }
    float GetSuspicion() const { return Suspicion; }
    // Sound stimulus carries a world location, never Snake's position.
    void HearWallKnock(const FVector& SoundLocation, const FVector& WallNormal, float HearingRadius, AActor* SoundInstigator);
    bool IsInvestigatingNoise() const { return DistractionPhase != EDistractionPhase::None; }

private:
    enum class EDistractionPhase : uint8 { None, React, Approach, Search, Return };
    EDistractionPhase DistractionPhase = EDistractionPhase::None;
    FVector DistractionLocation = FVector::ZeroVector;
    FVector ReturnToPatrol = FVector::ZeroVector;
    float DistractionTimer = 0.f;
    float InvestigationStuckTimer = 0.f;
    int32 ConsecutiveKnocks = 0;
    bool AdvanceNoiseInvestigation(float DeltaSeconds);
    void MoveToward(const FVector& Destination, float Speed, float DeltaSeconds);
    void AdvanceAssignedRoutine(float DeltaSeconds);

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UCapsuleComponent* CollisionCapsule;

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* ProxyBody;

    // Contract is shared across the opening, detention, upper floors and
    // future mission encounter authoring.
    FName AssignedDutyId = NAME_None;
    FName AssignedDutyName = NAME_None;
    FName AssignedObject = NAME_None;
    FVector AssignmentAnchor = FVector::ZeroVector;
    FVector DutyFocusPoint = FVector::ZeroVector;
    float DutyPauseSeconds = 0.8f;
    float DutyPauseRemaining = 0.f;
    float DutyBlockedSeconds = 0.f;
    TArray<FVector> PatrolWaypoints;
    int32 PatrolWaypointIndex = 0;
    FVector PatrolOrigin = FVector::ZeroVector;
    FVector LastKnownPlayerPosition = FVector::ZeroVector;
    bool bPatrolForward = true;
    bool bIncapacitated = false;
    bool bBeingChoked = false;
    bool bBodyDragged = false;
    bool bBodyConcealed = false;
    float BodyDiscoveryCooldown = 0.f;
    TWeakObjectPtr<AActor> ChokeInstigator;
    float Suspicion = 0.f;
    float CameraAlertSeconds = 0.f;
    float Health = 100.f;
    float StaggerSeconds = 0.f;
    float MeleeKnockdownSeconds = 0.f;
    float AttackCooldown = 0.f;
    uint32 AwarenessState = 0;
};
