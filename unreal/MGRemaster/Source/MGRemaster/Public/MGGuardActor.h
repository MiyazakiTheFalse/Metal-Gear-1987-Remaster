#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGGuardActor.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;

// Source-only operational role. The custody role is security procedure,
// not a different 'evil/innocent' faction and not scripted awareness.
UENUM(BlueprintType)
enum class EMGGuardRole : uint8
{
    Patrol, Prison, Custody
};

// Acting/animation channel, derived from real observed evidence only.
// No animation or sound writes an authoritative suspicion/mission flag.
UENUM(BlueprintType)
enum class EMGGuardPosture : uint8
{
    Patrol, Notice, Investigate, Search, Challenge, Report, Engage, Strike, Incapacitated
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
    FVector GetEvidenceLocation() const { return LastKnownPlayerPosition; }
    UFUNCTION(BlueprintImplementableEvent, Category="Security|Acting")
    void OnPostureChanged(EMGGuardPosture Previous, EMGGuardPosture Current);
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

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UCapsuleComponent* CollisionCapsule;

    UPROPERTY(VisibleAnywhere, Category="Prototype")
    UStaticMeshComponent* ProxyBody;

    UPROPERTY(VisibleAnywhere, Category="Character|Art")
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(EditAnywhere, Category="Security|Doctrine")
    EMGGuardRole Role = EMGGuardRole::Patrol;
    FVector PatrolOrigin = FVector::ZeroVector;
    bool bCustodyChallenge = false;
    bool bPendingRadioReport = false;
    float ChallengeAge = 0.f;
    float CustodyThreatMemorySeconds = 0.f;
    float ContactEvidenceAge = 99.f;
    float PendingRadioDelay = 0.f;
    float SquadBroadcastCooldown = 0.f;
    float SearchForLastKnownSeconds = 0.f;
    float PursuitBlockedSeconds = 0.f;
    float AttackWindupSeconds = 0.f;
    float VisualConfirmationSeconds = 0.f;
    float BodyScanCountdown = 0.f;
    float BodyReportDelay = -1.f;
    FVector BodyReportLocation = FVector::ZeroVector;
    EMGGuardPosture PresentedPosture = EMGGuardPosture::Patrol;
    TSet<AMGGuardActor*> SeenIncapacitatedColleagues;
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
    float Suspicion = 0.f;
    float Health = 100.f;
    float StaggerSeconds = 0.f;
    float AttackCooldown = 0.f;
    uint32 AwarenessState = 0;
};
