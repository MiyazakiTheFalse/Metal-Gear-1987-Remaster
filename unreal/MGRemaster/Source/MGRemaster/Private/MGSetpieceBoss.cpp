#include "MGSetpieceBoss.h"
#include "MGLaterMission.h"
#include "MGOrdnance.h"
#include "MGPlayerCharacter.h"
#include "MGCampaignState.h"
#include "Engine/GameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

AMGSetpieceBoss::AMGSetpieceBoss()
{
    PrimaryActorTick.bCanEverTick = true;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossVisualCollision"));
    SetRootComponent(Body);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
    Body->SetCollisionProfileName(TEXT("BlockAll"));
    Body->SetWorldScale3D(FVector(2.2f,1.4f,2.2f));
}
void AMGSetpieceBoss::ConfigureEncounter(
    AMGLaterMission* Mission, FName EncounterId, FName RequiredOrdnance,
    float HitPoints, float AttackRadius, float Telegraph, float AttackDamage,
    const FVector& Focus)
{
    Director = Mission;
    Id = EncounterId;
    WeaponRequirement = RequiredOrdnance;
    Health = FMath::Max(1.f, HitPoints);
    AlertRange = FMath::Max(280.f, AttackRadius);
    TelegraphTime = FMath::Clamp(Telegraph,.5f,3.5f);
    AttackPower = FMath::Max(1.f, AttackDamage);
    DefensiveFocus = Focus;
    Tags.AddUnique(Id);
    Tags.AddUnique(TEXT("AUTHORED_BOSS_POST"));
    // Distinct readable silhouettes even until licensed art is mounted.
    if (Id == TEXT("m03.hind_d_defeated"))
        Body->SetWorldScale3D(FVector(3.7f,5.2f,2.f));
    else if (Id == TEXT("m03.tank_defeated") ||
             Id == TEXT("m04.bulldozer_defeated"))
        Body->SetWorldScale3D(FVector(5.f,3.5f,2.6f));
    else if (Id.ToString().Contains(TEXT("TX11")))
        Body->SetWorldScale3D(FVector(1.8f,1.3f,2.7f));
    else if (Id == TEXT("bosses.shoot_gunner"))
        Body->SetWorldScale3D(FVector(1.6f,1.2f,2.2f));
    else
        Body->SetWorldScale3D(FVector(1.3f,1.3f,2.1f));
    // Keep every cover collision above the floor rather than inserting the
    // root cube through a platform and letting Unreal change spawn placement.
    FVector At=GetActorLocation();
    At.Z=10.f+Body->GetComponentScale().Z*50.f;
    SetActorLocation(At,false);
}
float AMGSetpieceBoss::TakeDamage(float Damage, const FDamageEvent& Event,
                                  AController* Instigator, AActor* Causer)
{
    if (bDefeated || Damage <= 0.f) return 0.f;
    FName Source = TEXT("HANDGUN");
    if (const AMGOrdnance* Ordnance = Cast<AMGOrdnance>(Causer))
        Source = Ordnance->GetOrdnanceId();
    // Enemy-specific counterplay, not a generic 100-health-hit sponge.
    // Air vehicle -> grenades, tank -> mines, TX-11 -> rockets.
    if (Source != WeaponRequirement)
    {
        if (Director.IsValid()) Director->NotifyWrongWeapon(Id, WeaponRequirement);
        return 0.f;
    }
    // Snake's existing hitscan prototype emits 100 damage for incapacitating
    // guards. Human bosses need a separate multi-shot endurance profile,
    // including Coward Duck's ten careful pistol shots.
    if (Id==TEXT("m06.coward_duck_defeated") && IsValid(Causer) &&
        GetWorld())
    {
        // He hides behind THREE actual hostages. Shooting straight through
        // one cannot magically damage the boss: Snake must seek an angle.
        // Full POW harm/rank simulation is a separate unverified system.
        const FVector From=Causer->GetActorLocation();
        const FVector Delta=GetActorLocation()-From;
        const float D2=Delta.SizeSquared2D();
        if (D2>1.f)
        {
            for (TActorIterator<AStaticMeshActor> Pawn(GetWorld());Pawn;++Pawn)
            {
                AStaticMeshActor* Captive=*Pawn;
                if (!IsValid(Captive) ||
                    (!Captive->ActorHasTag(TEXT("M06_JenniferBrother_Hostage")) &&
                     !Captive->ActorHasTag(TEXT("M06_HostageWest")) &&
                     !Captive->ActorHasTag(TEXT("M06_HostageEast")))) continue;
                const FVector ToHostage=Captive->GetActorLocation()-From;
                const float U=FVector::DotProduct(
                    FVector(Delta.X,Delta.Y,0.f),
                    FVector(ToHostage.X,ToHostage.Y,0.f))/D2;
                if (U<=0.f || U>=1.f) continue;
                const FVector Closest=From+Delta*U;
                if (FVector::Dist2D(Closest,Captive->GetActorLocation())<125.f)
                {
                    if (Director.IsValid()) Director->NotifyHostageShield();
                    return 0.f;
                }
            }
        }
    }
    const float EffectiveDamage=Source==TEXT("HANDGUN") ?
        FMath::Min(Damage,18.f) : Damage;
    Health = FMath::Max(0.f, Health - EffectiveDamage);
    if (Health <= 0.f)
    {
        bDefeated = true;
        if (Director.IsValid()) Director->OnBossDefeated(Id);
        else if (Id == TEXT("bosses.shoot_gunner") &&
                 GetWorld() && GetWorld()->GetGameInstance())
        {
            if (UMGCampaignState* Story =
                GetWorld()->GetGameInstance()->GetSubsystem<UMGCampaignState>())
                Story->RecordBeat(Id,
                    TEXT("SHOOT GUNNER DEFEATED: The prison corridor's shotgun guard is down."));
        }
        Destroy();
    }
    else Windup = -1.f;
    return EffectiveDamage;
}
void AMGSetpieceBoss::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bDefeated || !GetWorld()) return;
    AMGPlayerCharacter* Snake = Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this,0));
    if (!Snake) return;
    // Mobile machines follow an authored roadway. Their movement is not
    // driven by Snake's position; prepare mines along the actual tank route.
    if (Id == TEXT("m03.tank_defeated") ||
        Id == TEXT("m04.bulldozer_defeated"))
    {
        MoveClock += DeltaSeconds;
        const float Amplitude = Id == TEXT("m03.tank_defeated") ? 790.f : 470.f;
        FVector Next=GetActorLocation();
        Next.X=DefensiveFocus.X + FMath::Sin(MoveClock*.58f)*Amplitude;
        FHitResult MovementHit;
        SetActorLocation(Next,true,&MovementHit);
    }
    const FVector Offset = Snake->GetActorLocation() - GetActorLocation();
    if (FMath::Abs(Offset.Z) > 340.f || Offset.Size2D() > AlertRange)
    {
        Windup = -1.f;
        Cooldown = FMath::Max(0.f,Cooldown - DeltaSeconds);
        return;
    }
    FHitResult Sight;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MGBossSight),false,this);
    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        Sight, GetActorLocation()+FVector(0.f,0.f,120.f),
        Snake->GetActorLocation()+FVector(0.f,0.f,55.f),
        ECC_Visibility,Params) && Sight.GetActor()!=Snake;
    if (bBlocked)
    {
        Windup = -1.f;
        Cooldown = FMath::Max(0.f,Cooldown - DeltaSeconds);
        return;
    }
    Cooldown = FMath::Max(0.f,Cooldown - DeltaSeconds);
    if (Windup < 0.f && Cooldown <= 0.f)
    {
        Windup = TelegraphTime;
        Snake->NotifyFeedback(
            FString::Printf(TEXT("%s / ATTACK WINDUP — MOVE INTO COVER"),*Id.ToString()),
            TEXT("boss_windup"));
    }
    if (Windup >= 0.f)
    {
        // Fixed facing/tracking with a visible turn; attack does not warp to Snake.
        const FRotator Target = Offset.GetSafeNormal2D().Rotation();
        SetActorRotation(FMath::RInterpTo(GetActorRotation(),Target,
                                          DeltaSeconds,1.8f));
        Windup -= DeltaSeconds;
        if (Windup <= 0.f)
        {
            // Check after telegraph: hiding behind physically blocking scenery
            // must work and distance can be exploited.
            const FVector AtRelease = Snake->GetActorLocation()-GetActorLocation();
            FHitResult Hit;
            const bool Occluded = GetWorld()->LineTraceSingleByChannel(
                Hit,GetActorLocation()+FVector(0.f,0.f,120.f),
                Snake->GetActorLocation()+FVector(0.f,0.f,55.f),
                ECC_Visibility,Params) && Hit.GetActor()!=Snake;
            Snake->NotifyFeedback(TEXT("ENEMY ATTACK / CLEAR THE FIRE LINE"),
                                  TEXT("boss_attack"));
            if (!Occluded && AtRelease.Size2D() <= AlertRange &&
                FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(),
                                    AtRelease.GetSafeNormal2D()) > .55f)
                UGameplayStatics::ApplyDamage(Snake,AttackPower,nullptr,this,
                    UDamageType::StaticClass());
            Windup = -1.f;
            Cooldown = 2.15f;
        }
    }
}
