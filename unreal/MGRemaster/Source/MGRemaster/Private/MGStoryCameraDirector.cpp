#include "MGStoryCameraDirector.h"

#include "MGPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
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

void AMGStoryCameraDirector::AddShot(
    float Duration, float Radius, float Yaw, float Height, float Focus,
    float Pan, float Fov, bool bSnake)
{
    FAuthoredShot S;
    S.EndSeconds = MaxSeconds + FMath::Max(.15f, Duration);
    S.Radius = Radius;
    S.Yaw = Yaw;
    S.Height = Height;
    S.Focus = Focus;
    S.Pan = Pan;
    S.Fov = Fov;
    S.bUseSnake = bSnake;
    ShotPlan.Add(S);
    MaxSeconds = S.EndSeconds;
}

bool AMGStoryCameraDirector::ConfigureScene(FName Scene)
{
    ShotPlan.Reset();
    MaxSeconds = 0.f;
    // Original *authored* shots of live actors and playable level geometry.
    // These are blocking/composition first-pass values, not captured mocap,
    // facial rig performance, final VO, light rigs or imported Sequencer.
    if (Scene == TEXT("opening"))
    {
        AddShot(1.55f, 570.f, 142.f, 235.f, 120.f, -5.f, 57.f);
        AddShot(1.20f, 300.f, -136.f, 178.f, 130.f, 6.f, 43.f);
        AddShot(1.00f, 430.f, 150.f, 190.f, 125.f, 4.f, 53.f);
    }
    else if (Scene == TEXT("capture"))
    {
        // This scene happens at the REAL custody post before transport,
        // not mysteriously after Snake has already woken up in a cell.
        AddShot(1.22f, 290.f, 105.f, 175.f, 110.f, -4.f, 50.f);
        AddShot(1.33f, 205.f, -110.f, 160.f, 120.f, 2.f, 40.f);
    }
    else if (Scene == TEXT("wake"))
    {
        AddShot(1.20f, 310.f, 145.f, 260.f, 95.f, -3.f, 56.f);
        AddShot(1.18f, 190.f, -130.f, 150.f, 115.f, 4.f, 42.f);
        AddShot(.85f, 330.f, 168.f, 170.f, 122.f, 5.f, 52.f);
    }
    else if (Scene == TEXT("fox"))
    {
        AddShot(1.32f, 350.f, 160.f, 195.f, 125.f, -4.f, 56.f);
        AddShot(1.55f, 210.f, -112.f, 160.f, 132.f, 2.f, 42.f);
        AddShot(1.25f, 275.f, 135.f, 170.f, 127.f, -2.f, 48.f, true);
        AddShot(1.08f, 330.f, 170.f, 180.f, 124.f, 4.f, 51.f);
    }
    else if (Scene == TEXT("shotmaker"))
    {
        AddShot(.68f, 355.f, 125.f, 182.f, 120.f, -5.f, 52.f);
        AddShot(.67f, 240.f, -120.f, 152.f, 132.f, 3.f, 42.f);
    }
    else if (Scene == TEXT("down"))
    {
        AddShot(.94f, 300.f, -145.f, 170.f, 90.f, 5.f, 46.f);
        AddShot(1.18f, 350.f, 135.f, 185.f, 117.f, 3.f, 53.f, true);
    }
    else if (Scene == TEXT("card3"))
    {
        AddShot(1.20f, 235.f, 155.f, 167.f, 102.f, -3.f, 42.f);
    }
    else return false;
    return ShotPlan.Num() > 0 && MaxSeconds > 0.f;
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
    CurrentShot = INDEX_NONE;
    InitialSubjectYaw = Subject->GetActorRotation().Yaw;
    bPlaying = true;
    // Only very short committed story moments block player input. Freeform
    // camera accents give control back immediately when the player moves.
    bLocksInput = SceneId == TEXT("capture") ||
        SceneId == TEXT("wake") || SceneId == TEXT("fox");
    ShownSceneIds.Add(SceneId);
    UpdateShot(0.f, true);
    PC->SetViewTargetWithBlend(this, .18f);
    if (bLocksInput)
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }
    SetActorTickEnabled(true);
    OnStorySceneStarted(SceneId);
    return true;
}

void AMGStoryCameraDirector::UpdateShot(float Dt, bool bSnap)
{
    AActor* MainSubject = FocusActor.Get();
    if (!IsValid(MainSubject) || !GetWorld() || !ShotPlan.Num()) return;
    int32 Index = 0;
    while (Index < ShotPlan.Num()-1 &&
        ElapsedSeconds >= ShotPlan[Index].EndSeconds)
        ++Index;
    const FAuthoredShot& Shot = ShotPlan[Index];
    const bool bFirstFrameOfShot = CurrentShot != Index;
    if (bFirstFrameOfShot)
    {
        CurrentShot = Index;
        const float PriorEnd = Index > 0 ? ShotPlan[Index-1].EndSeconds : 0.f;
        OnStoryShotChanged(ActiveScene, Index, Shot.EndSeconds - PriorEnd);
    }
    AActor* Subject = Shot.bUseSnake && ActingPlayer.IsValid()
        ? Cast<AActor>(ActingPlayer.Get()) : MainSubject;
    if (!IsValid(Subject)) return;
    const float ShotStart = Index > 0 ? ShotPlan[Index-1].EndSeconds : 0.f;
    const float ShotTime = FMath::Max(0.f, ElapsedSeconds - ShotStart);
    const FVector Focus = Subject->GetActorLocation() +
        FVector(0.f, 0.f, Shot.Focus);
    const float Yaw = FMath::DegreesToRadians(
        InitialSubjectYaw + Shot.Yaw + Shot.Pan * ShotTime);
    const FVector Proposed = Focus + FVector(
        FMath::Cos(Yaw)*Shot.Radius,
        FMath::Sin(Yaw)*Shot.Radius,
        Shot.Height);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MGStoryShotLOS),
        false, this);
    Params.AddIgnoredActor(Subject);
    if (ActingPlayer.IsValid()) Params.AddIgnoredActor(ActingPlayer.Get());
    FHitResult Impact;
    FVector Safe = Proposed;
    if (GetWorld()->SweepSingleByChannel(Impact, Focus, Proposed,
        FQuat::Identity, ECC_Visibility,
        FCollisionShape::MakeSphere(19.f), Params) && Impact.bBlockingHit)
        Safe = Impact.Location -
            (Proposed-Focus).GetSafeNormal()*24.f;
    // Hard-cut on a new authored shot. A blend between two camera eyes
    // may slice through the walls even if both final positions are safe.
    FVector Next = (bSnap || bFirstFrameOfShot) ?
        Safe : FMath::VInterpTo(GetActorLocation(), Safe, Dt, 10.f);
    FHitResult SmoothHit;
    if (!bSnap && GetWorld()->SweepSingleByChannel(
        SmoothHit, Focus, Next, FQuat::Identity, ECC_Visibility,
        FCollisionShape::MakeSphere(19.f), Params) && SmoothHit.bBlockingHit)
        Next = SmoothHit.Location -
            (Next-Focus).GetSafeNormal()*24.f;
    SetActorLocation(Next);
    SetActorRotation((Focus-Next).Rotation());
    if (Lens) Lens->SetFieldOfView(Shot.Fov);
}

void AMGStoryCameraDirector::FinishScene(bool bSkipped)
{
    if (!bPlaying) return;
    const FName Finished = ActiveScene;
    bPlaying = false;
    ActiveScene = NAME_None;
    if (AMGPlayerCharacter* Player = ActingPlayer.Get())
        if (APlayerController* PC =
            Cast<APlayerController>(Player->GetController()))
        {
            if (bLocksInput)
            {
                PC->SetIgnoreMoveInput(false);
                PC->SetIgnoreLookInput(false);
            }
            PC->SetViewTargetWithBlend(Player, .20f);
        }
    ActingPlayer.Reset();
    FocusActor.Reset();
    bLocksInput = false;
    CurrentShot = INDEX_NONE;
    SetActorTickEnabled(false);
    OnStorySceneEnded(Finished, bSkipped);
}

void AMGStoryCameraDirector::SkipScene() { FinishScene(true); }

void AMGStoryCameraDirector::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!bPlaying) return;
    AMGPlayerCharacter* Player = ActingPlayer.Get();
    if (!IsValid(Player) || !FocusActor.IsValid() ||
        Player->GetHitReactionAlpha() > .08f ||
        (!bLocksInput && (Player->IsAiming() ||
            Player->GetVelocity().Size2D() > 45.f)))
    {
        FinishScene(true);
        return;
    }
    ElapsedSeconds += FMath::Max(0.f, Dt);
    if (ElapsedSeconds >= MaxSeconds)
    {
        FinishScene(false);
        return;
    }
    UpdateShot(Dt, false);
}
