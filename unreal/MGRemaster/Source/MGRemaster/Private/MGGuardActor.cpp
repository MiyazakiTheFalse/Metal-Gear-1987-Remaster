#include "MGGuardActor.h"

#include "MGPlayerCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "mg_legacy_core.h"

AMGGuardActor::AMGGuardActor()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    ProxyBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GuardBody"));
    ProxyBody->SetupAttachment(RootComponent);
    ProxyBody->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.6f));
    ProxyBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProxyBody->SetCollisionResponseToAllChannels(ECR_Ignore);
    ProxyBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (Cylinder.Succeeded()) ProxyBody->SetStaticMesh(Cylinder.Object);
}

void AMGGuardActor::BeginPlay()
{
    Super::BeginPlay();
    PatrolOrigin = GetActorLocation();
}

float AMGGuardActor::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                                AController* EventInstigator, AActor* DamageCauser)
{
    const float Accepted = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (DamageAmount > 0.f && !bIncapacitated)
    {
        // This first vertical slice has a single nonlethal stun weapon.
        bIncapacitated = true;
        Suspicion = 0.f;
        AwarenessState = 0;
        ProxyBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ProxyBody->SetRelativeRotation(FRotator(0.f, 0.f, 80.f));
        SetActorTickEnabled(false);
    }
    return Accepted;
}

void AMGGuardActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bIncapacitated || !GetWorld()) return;
    const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    const float Distance = ToPlayer.Size2D();

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MGGuardVisibility), false, this);
    const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, 55.f);
    const FVector Target = Player->GetActorLocation() + FVector(0.f, 0.f, 55.f);
    const bool bTraceHit = GetWorld()->LineTraceSingleByChannel(Hit, Eye, Target, ECC_Visibility, Params);
    const bool bHasLineOfSight = !bTraceHit || Hit.GetActor() == Player;
    const FVector Forward = GetActorForwardVector();

    MgGuardFrame Frame = {};
    Frame.guard_pos = {GetActorLocation().X / 100.f, GetActorLocation().Y / 100.f};
    Frame.guard_forward = {Forward.X, Forward.Y};
    Frame.player_pos = {Player->GetActorLocation().X / 100.f, Player->GetActorLocation().Y / 100.f};
    Frame.vision_range_m = 12.f;
    Frame.vision_half_angle_deg = 55.f;
    Frame.hearing_range_m = 11.f;
    Frame.visibility = Player->GetVisibilityFactor();
    Frame.noise = Player->GetNoiseFactor();
    Frame.suspicion = Suspicion;
    Frame.delta_seconds = DeltaSeconds;
    Frame.line_of_sight = bHasLineOfSight ? 1u : 0u;

    MgGuardResult Result = {};
#if MG_WITH_RUST
    if (mg_core_abi_version() == 1 && mg_step_guard(&Frame, &Result))
    {
        Suspicion = Result.suspicion;
        AwarenessState = Result.state;
        if (Result.sees_player || Result.hears_player)
            LastKnownPlayerPosition = Player->GetActorLocation();
    }
#else
    // Reduced fallback for editor prototyping only; use the Rust native ABI
    // for the full independently tested perception implementation.
    const bool bFacing = FVector::DotProduct(Forward.GetSafeNormal2D(), ToPlayer.GetSafeNormal2D()) > 0.57f;
    const bool bVisible = bHasLineOfSight && bFacing && Distance < 1200.f;
    const bool bAudible = Distance < 1100.f && Frame.noise > 0.1f;
    Suspicion = FMath::Clamp(Suspicion + DeltaSeconds *
        ((bVisible ? Frame.visibility * 0.95f : 0.f) +
         (bAudible ? Frame.noise * 0.40f : 0.f) -
         ((!bVisible && !bAudible) ? 0.12f : 0.f)), 0.f, 1.f);
    AwarenessState = Suspicion >= 0.99f ? 3 : Suspicion >= 0.68f ? 2 : Suspicion >= 0.22f ? 1 : 0;
    if (bVisible || bAudible) LastKnownPlayerPosition = Player->GetActorLocation();
#endif

    if (AwarenessState == 3)
    {
        const FVector Direction = (LastKnownPlayerPosition - GetActorLocation()).GetSafeNormal2D();
        if (!Direction.IsNearlyZero())
        {
            SetActorLocation(GetActorLocation() + Direction * 185.f * DeltaSeconds, true);
            SetActorRotation(Direction.Rotation());
        }
    }
    else if (AwarenessState > 0)
    {
        const FVector ToSearch = (LastKnownPlayerPosition - GetActorLocation()).GetSafeNormal2D();
        if (!ToSearch.IsNearlyZero())
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), ToSearch.Rotation(), DeltaSeconds, 3.f));
    }
    else
    {
        const FVector Point = PatrolOrigin + FVector(bPatrolForward ? 260.f : -260.f, 0.f, 0.f);
        const FVector Diff = Point - GetActorLocation();
        if (Diff.Size2D() < 20.f) bPatrolForward = !bPatrolForward;
        else
        {
            const FVector Step = Diff.GetSafeNormal2D();
            SetActorLocation(GetActorLocation() + Step * 90.f * DeltaSeconds, true);
            SetActorRotation(Step.Rotation());
        }
    }

#if !UE_BUILD_SHIPPING
    const FColor DebugColor = AwarenessState == 3 ? FColor::Red :
        (AwarenessState == 2 ? FColor::Orange : (AwarenessState == 1 ? FColor::Yellow : FColor::Green));
    DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 180.f),
        22.f, 12, DebugColor, false, 0.f, 0, 2.f);
    DrawDebugLine(GetWorld(), Eye, Eye + GetActorForwardVector() * 300.f,
        DebugColor, false, 0.f, 0, 2.f);
#endif
}
