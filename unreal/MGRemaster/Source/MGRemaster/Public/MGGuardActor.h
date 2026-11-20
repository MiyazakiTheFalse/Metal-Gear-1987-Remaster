#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGuardActor.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;
class UPointLightComponent;
class USoundBase;

// Source-only operational role. The custody role is security procedure,
// not a different 'evil/innocent' faction and not scripted awareness.
UENUM(BlueprintType)
enum class EMGGuardRole : uint8
{
    Patrol, Prison, Custody, LogisticsPost
};

// Acting/animation channel, derived from real observed evidence only.
// No animation or sound writes an authoritative suspicion/mission flag.
UENUM(BlueprintType)
enum class EMGGuardPosture : uint8
{
    Patrol, Notice, Investigate, Search, Challenge, Report, Engage, Strike, Incapacitated,
    SidearmAim, SidearmFire, SidearmReload
};

// Local security belief, distinct from the guard's assigned duty and the
// action/animation it performs. Nothing here grants GPS knowledge of Snake.
UENUM(BlueprintType)
enum class EMGGuardAwarenessPhase : uint8
{
    Routine, Suspicious, Investigating, Confirming, Engaged, Searching, Returning, Incapacitated
};

UENUM(BlueprintType)
enum class EMGGuardEvidenceKind : uint8
{
    None, HeardMovement, StructuralImpact, ObservedIntruder, SquadSighting,
    DiscoveredBody, SquadBodyReport, Wounded
};

// Reuses security suspicion rather than adding a separate 'box alert' AI.
// Distinguish noticing an object from confirming the operative inside it.
UENUM(BlueprintType)
enum class EMGGuardBoxPhase : uint8
{
    None, Notice, Approach, Inspect, Confirmed, Search
};

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

    bool IsIncapacitated() const { return bIncapacitated; }
    UFUNCTION(BlueprintPure, Category="Security|Body")
    bool IsUnconscious() const { return bIncapacitated && !bBodyDead; }
    UFUNCTION(BlueprintPure, Category="Security|Body")
    bool IsBodyBeingDragged() const { return bBodyBeingDragged; }
    bool TryMoveBodyTo(const FVector& ProposedPosition, const AActor* DraggingActor);
    void SetBodyBeingDragged(bool bDragged);
    void SetBodyPersistenceId(FName StableId) { BodyPersistenceId = StableId; }
    FName GetBodyPersistenceId() const { return BodyPersistenceId; }
    void RestoreIncapacitatedBody(const FVector& Position, bool bUnconscious);
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Body")
    void OnBodyDragChanged(bool bIsDragged);
    void SetRole(EMGGuardRole NewRole) { Role = NewRole; }
    void ConfigurePatrol(const FVector& Axis, float HalfLengthCm, float SpeedCmPerSec);
    bool CanAcceptDetention(const class AMGPlayerCharacter* Suspect) const;
    bool IsIssuingChallenge() const { return bCustodyChallenge; }
    UFUNCTION(BlueprintPure, Category="Security|Custody")
    bool IsCustodyChallengeReady() const { return bCustodyChallenge && ChallengeAge >= .75f; }
    // Final guard pose/VO/animation may bind this state. No cinematic controls
    // confiscation or grants arrest merely by playing its animation.
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Custody")
    void OnCustodyChallenge();
    void ReceiveLocalSquadReport(const FVector& SeenAt, const AActor* Sender);
    uint32 GetAwarenessState() const { return AwarenessState; }
    float GetSuspicion() const { return Suspicion; }
    UFUNCTION(BlueprintPure, Category="Security|Acting")
    EMGGuardPosture GetGuardPosture() const { return PresentedPosture; }
    UFUNCTION(BlueprintPure, Category="Security|Acting")
    float GetActualGroundSpeedCmPerSec() const { return ActualGroundSpeedCmPerSec; }
    UFUNCTION(BlueprintPure, Category="Security|State")
    EMGGuardAwarenessPhase GetAwarenessPhase() const { return SecurityPhase; }
    UFUNCTION(BlueprintPure, Category="Security|State")
    EMGGuardEvidenceKind GetEvidenceKind() const { return EvidenceKind; }
    UFUNCTION(BlueprintPure, Category="Security|State")
    bool HasDirectVisualContact() const { return bConfirmedVisualThisFrame; }
    UFUNCTION(BlueprintPure, Category="Security|State")
    float GetTimeInSecurityState() const { return TimeInSecurityState; }
    UFUNCTION(BlueprintPure, Category="Security|State")
    bool IsAlertedByThreat() const { return SecurityPhase == EMGGuardAwarenessPhase::Engaged; }
    UFUNCTION(BlueprintImplementableEvent, Category="Security|State")
    void OnAwarenessPhaseChanged(EMGGuardAwarenessPhase Previous, EMGGuardAwarenessPhase Current,
                                 EMGGuardEvidenceKind CurrentEvidence);
    UFUNCTION(BlueprintPure, Category="Security|Acting")
    FVector GetEvidenceLocation() const { return LastKnownPlayerPosition; }
    FString GetActiveGuardCallout() const { return CalloutRemaining > 0.f
        ? ActiveGuardCallout : FString(); }
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnGuardCallout(FName EventId);
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnPostureChanged(EMGGuardPosture Previous, EMGGuardPosture Current);
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnCartonInspectionConfirmed(); // Animation/prop gesture only; source rules own perception.
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnCartonNotice();
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnCartonInspectStarted();
    UFUNCTION(BlueprintPure, Category="Security|Acting")
    EMGGuardBoxPhase GetBoxPhase() const { return BoxPhase; }
    UFUNCTION(BlueprintPure, Category="Security|Acting")
    float GetBoxInspectionProgress() const { return CartonInspectionRequiredSeconds > 0.f
        ? FMath::Clamp(CartonInspectionSeconds / CartonInspectionRequiredSeconds, 0.f, 1.f)
        : 0.f; }
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnMeleeWarning();
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnMeleeImpact();
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnContactReported(const FVector& EvidenceLocation);
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnBodyDiscovered(const FVector& BodyLocation);
    void ReceiveEnvironmentalReport(const FVector& EvidenceAt, const AActor* Sender);
    // Sound stimulus carries a world location, never Snake's position.
    void HearWallKnock(const FVector& SoundLocation, const FVector& WallNormal, float HearingRadius, AActor* SoundInstigator);
    bool IsInvestigatingNoise() const { return DistractionPhase != EDistractionPhase::None; }
    UFUNCTION(BlueprintPure, Category="Character|Art")
    bool HasAuthoredCharacterArt() const;
    UStaticMeshComponent* GetHeldWeaponVisual() const { return SecuritySidearm; }
    USkeletalMeshComponent* GetRigMesh() const { return CharacterMesh; }
    UFUNCTION(BlueprintPure, Category="Security|Weapon")
    bool IsReloadingSidearm() const { return bSidearmReloading; }
    UFUNCTION(BlueprintPure, Category="Security|Weapon")
    bool IsPreparingSidearmShot() const { return SidearmWindupSeconds > 0.f; }
    UFUNCTION(BlueprintPure, Category="Security|Weapon")
    int32 GetSidearmRounds() const { return SidearmRounds; }
    UFUNCTION(BlueprintPure, Category="Security|Weapon")
    int32 GetRemainingSidearmMagazines() const { return SpareSidearmMagazines; }
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Weapon")
    void OnSidearmAimStarted();
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Weapon")
    void OnSidearmFired(int32 RoundsRemaining, bool bPlayerHit);
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Weapon")
    void OnSidearmReloadStarted();
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Weapon")
    void OnSidearmReloadFinished();
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Weapon")
    void OnSidearmReloadInterrupted();
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Weapon")
    void OnSidearmCasingEjected();

private:
    enum class EDistractionPhase : uint8 { None, React, Approach, Search, Return };
    EDistractionPhase DistractionPhase = EDistractionPhase::None;
    FVector DistractionLocation = FVector::ZeroVector;
    FVector ReturnToPatrol = FVector::ZeroVector;
    float DistractionTimer = 0.f;
    float InvestigationStuckTimer = 0.f;
    int32 ConsecutiveKnocks = 0;
    bool AdvanceNoiseInvestigation(float DeltaSeconds);
    bool AdvanceBoxInvestigation(float DeltaSeconds);
    void MoveToward(const FVector& Destination, float Speed, float DeltaSeconds);

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UCapsuleComponent* CollisionCapsule;

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* ProxyBody;

    UPROPERTY(VisibleAnywhere, Category="Character|Art")
    USkeletalMeshComponent* CharacterMesh;
    UPROPERTY(VisibleAnywhere, Category="Security|Weapon")
    UStaticMeshComponent* SecuritySidearm;
    UPROPERTY(VisibleAnywhere, Category="Security|Weapon")
    UPointLightComponent* SidearmMuzzleLight;
    UPROPERTY(Transient)
    USoundBase* SidearmShotSound = nullptr;
    float PendingSidearmCaseSeconds = -1.f;
    // 1987 guards shoot, but the 3D weapon model, magazine capacity and reload
    // interval are reversible art/balance hypotheses, NOT exact MSX2 stats.
    UPROPERTY(EditAnywhere, Category="Security|Weapon", meta=(ClampMin="4", ClampMax="16"))
    int32 SidearmMagazineCapacity = 8;
    UPROPERTY(EditAnywhere, Category="Security|Weapon", meta=(ClampMin="0.8", ClampMax="3.0"))
    float SidearmReloadDuration = 1.65f;
    int32 SidearmRounds = 8;
    int32 SpareSidearmMagazines = 3;
    bool bSidearmReloading = false;
    float SidearmReloadSeconds = 0.f;
    float SidearmWindupSeconds = 0.f;
    float SidearmShotCooldown = 0.f;
    bool bSidearmAiming = false;
    float SidearmMuzzleFlashRemaining = 0.f;

    UPROPERTY(EditAnywhere, Category="Security|Doctrine")
    EMGGuardRole Role = EMGGuardRole::Patrol;
    FVector PatrolOrigin = FVector::ZeroVector;
    bool bCustodyChallenge = false;
    bool bCartonInspectionAnnounced = false;
    bool bCartonInterestAnnounced = false;
    bool bCartonIdentitySpoken = false;
    float CartonInspectionSeconds = 0.f;
    float CartonInspectionRequiredSeconds = 1.30f;
    float BoxInterestRemainingSeconds = 0.f;
    float BoxReactSeconds = 0.f;
    float BoxSearchSeconds = 0.f;
    float BoxApproachStuckSeconds = 0.f;
    FVector BoxEvidenceWorldPosition = FVector::ZeroVector;
    EMGGuardBoxPhase BoxPhase = EMGGuardBoxPhase::None;
    bool bPendingRadioReport = false;
    float ChallengeAge = 0.f;
    float CustodyThreatMemorySeconds = 0.f;
    // Age since firsthand visual proof, never reset by a footstep or radio rumour.
    float ContactEvidenceAge = 99.f;
    float FootstepLocalizationRefreshSeconds = 0.f;
    void ObserveApproximateFootstep(const FVector& AudibleSource, bool bOccluded);
    float PendingRadioDelay = 0.f;
    FVector PendingConfirmedRadioSighting = FVector::ZeroVector;
    float SquadBroadcastCooldown = 0.f;
    float SearchForLastKnownSeconds = 0.f;
    float PursuitBlockedSeconds = 0.f;
    float AttackWindupSeconds = 0.f;
    float VisualConfirmationSeconds = 0.f;
    float BodyScanCountdown = 0.f;
    float BodyReportDelay = -1.f;
    FVector BodyReportLocation = FVector::ZeroVector;
    FString ActiveGuardCallout;
    float CalloutRemaining = 0.f;
    void EmitGuardCallout(FName EventId, const FString& Caption);
    EMGGuardAwarenessPhase SecurityPhase = EMGGuardAwarenessPhase::Routine;
    EMGGuardEvidenceKind EvidenceKind = EMGGuardEvidenceKind::None;
    float TimeInSecurityState = 0.f;
    float EvidenceAge = 99.f;
    bool bConfirmedVisualThisFrame = false;
    void SetSecurityPhase(EMGGuardAwarenessPhase NewPhase);
    void ObserveEvidence(EMGGuardEvidenceKind Kind, const FVector& Location);
    EMGGuardPosture PresentedPosture = EMGGuardPosture::Patrol;
    TSet<AMGGuardActor*> SeenIncapacitatedColleagues;
    FVector SmoothedGroundVelocity = FVector::ZeroVector;
    float ActualGroundSpeedCmPerSec = 0.f;
    FVector PatrolAxis = FVector::ForwardVector;
    float PatrolHalfLength = 180.f;
    float PatrolSpeed = 90.f;
    FRotator StationFacing = FRotator::ZeroRotator;
    void SetGuardPosture(EMGGuardPosture Desired);
    void CheckVisibleIncapacitatedColleagues();
    int32 FlankSide = 1;
    UPROPERTY(EditAnywhere, Category="Security|Debug")
    bool bShowPerceptionDebug = false;
    FVector LastKnownPlayerPosition = FVector::ZeroVector;
    bool bPatrolForward = true;
    bool bIncapacitated = false;
    bool bBodyDead = false; // Shot lethality != nonlethal melee unconsciousness.
    bool bBodyBeingDragged = false;
    FName BodyPersistenceId = NAME_None; // Assigned by mission, not actor iteration order.
    float Suspicion = 0.f;
    float Health = 100.f;
    float StaggerSeconds = 0.f;
    float AttackCooldown = 0.f;
    uint32 AwarenessState = 0;
};
