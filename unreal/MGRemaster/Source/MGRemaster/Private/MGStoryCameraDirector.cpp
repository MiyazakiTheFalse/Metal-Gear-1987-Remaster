#include "MGStoryCameraDirector.h"

#include "MGPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AMGStoryCameraDirector::AMGStoryCameraDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("CinematicRoot"));
    SetRootComponent(Root);
    Lens = CreateDefaultSubobject<UCameraComponent>(TEXT("CinematicCamera"));
    Lens->SetupAttachment(Root);
    Lens->FieldOfView = 49.f;
    SetActorTickEnabled(false);
}

bool AMGStoryCameraDirector::ConfigureScene(FName Scene)
{
    // These are removable rough in-engine blocked shots. Realistic actors,
    // scripted body performances, lens continuity, sound mix and actual
    // Sequencer assets remain a separate, required production deliverable.
    if (Scene == TEXT("opening"))
    {
        MaxSeconds=3.15f; RadiusCm=520.f; YawDegrees=136.f;
        EyeRaiseCm=145.f; SubjectEyeCm=85.f; PanDegreesPerSecond=5.f;
    }
    else if (Scene == TEXT("capture"))
    {
        MaxSeconds=2.55f; RadiusCm=260.f; YawDegrees=85.f;
        EyeRaiseCm=85.f; SubjectEyeCm=72.f; PanDegreesPerSecond=-4.f;
    }
    else if (Scene == TEXT("fox"))
    {
        MaxSeconds=2.75f; RadiusCm=190.f; YawDegrees=190.f;
        EyeRaiseCm=65.f; SubjectEyeCm=100.f; PanDegreesPerSecond=-7.f;
    }
    else if (Scene == TEXT("shotmaker"))
    {
        MaxSeconds=1.35f; RadiusCm=245.f; YawDegrees=135.f;
        EyeRaiseCm=115.f; SubjectEyeCm=95.f; PanDegreesPerSecond=7.f;
    }
    else if (Scene == TEXT("card3"))
    {
        MaxSeconds=1.20f; RadiusCm=240.f; YawDegrees=185.f;
        EyeRaiseCm=115.f; SubjectEyeCm=85.f; PanDegreesPerSecond=-4.f;
    }
    else return false;
    return true;
}

bool AMGStoryCameraDirector::PlayScene(
    FName SceneId, AMGPlayerCharacter* Player, AActor* Subject)
{
    if (!IsValid(Player) || !IsValid(Subject) || bPlaying ||
        Player->IsAiming() || Player->GetHitReactionAlpha() > .08f ||
        ShownSceneIds.Contains(SceneId) || !ConfigureScene(SceneId))
        return false;
    APlayerController* PC = Cast<APlayerController>(Player->GetController());
    if (!PC || !PC->IsLocalController()) return false;
    ActingPlayer = Player;
    FocusActor = Subject;
    ActiveScene = SceneId;
    ElapsedSeconds = 0.f;
    bPlaying = true;
    ShownSceneIds.Add(SceneId);
    UpdateShot(0.f, true);
    // Gameplay remains unpaused. A shot only captures the camera and cannot
    // run mission transitions, change enemy suspicion or spawn extra guards.
    PC->SetViewTargetWithBlend(this, .26f);
    SetActorTickEnabled(true);
    OnStorySceneStarted(SceneId);
    return true;
}

void AMGStoryCameraDirector::UpdateShot(float Dt, bool bSnap)
{
    AActor* Subject = FocusActor.Get();
    if (!IsValid(Subject) || !GetWorld()) return;
    const FVector Focus = Subject->GetActorLocation() +
        FVector(0.f, 0.f, SubjectEyeCm);
    const float Yaw = FMath::DegreesToRadians(
        YawDegrees + PanDegreesPerSecond * ElapsedSeconds);
    const FVector Proposed = Focus + FVector(
        FMath::Cos(Yaw)*RadiusCm,FMath::Sin(Yaw)*RadiusCm,EyeRaiseCm);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MGStoryShotLOS),false,this);
    Params.AddIgnoredActor(Subject);
    FHitResult Impact;
    FVector Safe = Proposed;
    if (GetWorld()->SweepSingleByChannel(Impact,Focus,Proposed,
        FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(19.f),Params)
        && Impact.bBlockingHit)
        Safe = Impact.Location - (Proposed-Focus).GetSafeNormal()*24.f;
    FVector Next = bSnap ? Safe :
        FMath::VInterpTo(GetActorLocation(),Safe,Dt,10.f);
    // Smoothing between independently safe points may itself cross a wall.
    FHitResult SmoothHit;
    if (!bSnap && GetWorld()->SweepSingleByChannel(
        SmoothHit,Focus,Next,FQuat::Identity,ECC_Visibility,
        FCollisionShape::MakeSphere(19.f),Params) && SmoothHit.bBlockingHit)
        Next = SmoothHit.Location - (Next-Focus).GetSafeNormal()*24.f;
    SetActorLocation(Next);
    SetActorRotation((Focus-Next).Rotation());
}

void AMGStoryCameraDirector::FinishScene(bool bSkipped)
{
    if (!bPlaying) return;
    const FName Finished = ActiveScene;
    bPlaying = false;
    ActiveScene = NAME_None;
    if (AMGPlayerCharacter* Player = ActingPlayer.Get())
        if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
            PC->SetViewTargetWithBlend(Player, .28f);
    ActingPlayer.Reset();
    FocusActor.Reset();
    SetActorTickEnabled(false);
    OnStorySceneEnded(Finished,bSkipped);
}

void AMGStoryCameraDirector::SkipScene() { FinishScene(true); }

void AMGStoryCameraDirector::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!bPlaying) return;
    AMGPlayerCharacter* Player = ActingPlayer.Get();
    if (!IsValid(Player) || !FocusActor.IsValid() || Player->IsAiming() ||
        Player->GetHitReactionAlpha() > .08f)
    {
        FinishScene(true);
        return;
    }
    ElapsedSeconds += FMath::Max(0.f,Dt);
    if (ElapsedSeconds >= MaxSeconds){ FinishScene(false);return; }
    UpdateShot(Dt,false);
}
