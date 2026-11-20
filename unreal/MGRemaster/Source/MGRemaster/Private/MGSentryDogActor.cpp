#include "MGSentryDogActor.h"
#include "MGPlayerCharacter.h"
#include "MGGuardActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"
#include "EngineUtils.h"
#include "Sound/SoundBase.h"

AMGSentryDogActor::AMGSentryDogActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SentryDogCapsule"));
    Capsule->InitCapsuleSize(37.f, 47.f);
    Capsule->SetCollisionProfileName(TEXT("Pawn"));
    Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    RootComponent = Capsule;
    SentryMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SentryDogSkeletalBody"));
    SentryMesh->SetupAttachment(RootComponent);
    SentryMesh->SetRelativeLocation(FVector(0.f, 0.f, -47.f));
    SentryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SourceProxy = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DOG_DEBUG_NOT_APPROVED"));
    SourceProxy->SetupAttachment(RootComponent);
    SourceProxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SourceProxy->SetRelativeScale3D(FVector(1.25f, .45f, .7f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) SourceProxy->SetStaticMesh(Cube.Object);
}

void AMGSentryDogActor::BeginPlay()
{
    Super::BeginPlay();
    Home = GetActorLocation();
    if (USkeletalMesh* Asset = LoadObject<USkeletalMesh>(nullptr,
        TEXT("/Game/Characters/Animals/SK_OuterHeavenSentryDog_1995.SK_OuterHeavenSentryDog_1995")))
    {
        SentryMesh->SetSkeletalMeshAsset(Asset);
        if (UClass* Animation = LoadClass<UAnimInstance>(nullptr,
            TEXT("/Game/Characters/Animals/ABP_SentryDog_1995.ABP_SentryDog_1995_C")))
            SentryMesh->SetAnimInstanceClass(Animation);
        SourceProxy->SetVisibility(false, true);
        SentryMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SentryMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
        SentryMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
}

void AMGSentryDogActor::ConfigureBeatPatrol(const FVector& TravelAxis, float RadiusCm)
{
    PatrolAxis = TravelAxis.GetSafeNormal2D();
    if (PatrolAxis.IsNearlyZero()) PatrolAxis = FVector::ForwardVector;
    PatrolRadius = FMath::Clamp(RadiusCm, 60.f, 240.f);
}

bool AMGSentryDogActor::CanObserve(const AMGPlayerCharacter* Player) const
{
    if (!IsValid(Player) || !GetWorld()) return false;
    const FVector Delta = Player->GetActorLocation() - GetActorLocation();
    const float MaximumRange = Player->IsProne() ? 300.f : (Player->IsInCover() ? 440.f : 680.f);
    if (Delta.Size2D() > MaximumRange) return false;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MGSentryDogSight), false, this);
    FHitResult Hit;
    const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 25.f);
    const FVector End = Player->GetActorLocation() + FVector(0.f, 0.f, Player->IsProne() ? 9.f : 42.f);
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Query)
        && Hit.GetActor() == Player;
}

void AMGSentryDogActor::MoveDog(const FVector& Target, float Speed, float Dt)
{
    const FVector Delta = Target - GetActorLocation();
    const FVector Dir = Delta.GetSafeNormal2D();
    if (Dir.IsNearlyZero()) return;
    FHitResult Hit;
    SetActorLocation(GetActorLocation() + Dir * FMath::Min(
        Delta.Size2D(), Speed * FMath::Max(0.f, Dt)), true, &Hit);
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), Dir.Rotation(), Dt, 6.f));
}

void AMGSentryDogActor::BarkFromActualLocation()
{
    if (BarkCooldown > 0.f) return;
    BarkCooldown = 4.f;
    OnSentryBark();
    if (USoundBase* Bark = LoadObject<USoundBase>(nullptr,
        TEXT("/Game/Audio/Animals/SentryDog_Bark.SentryDog_Bark")))
        UGameplayStatics::PlaySoundAtLocation(this, Bark, GetActorLocation(), .7f);
    // A nearby guard hears a canine alarm at the DOG, not a supernatural
    // live map marker at the player's transform.
    for (TActorIterator<AMGGuardActor> Guard(GetWorld()); Guard; ++Guard)
        if (FVector::Dist2D(Guard->GetActorLocation(), GetActorLocation()) < 700.f)
            Guard->ReceiveEnvironmentalReport(GetActorLocation(), this);
}

void AMGSentryDogActor::Tick(float Dt)
{
    Super::Tick(Dt);
    if (State == EMGSentryDogState::Incapacitated || !GetWorld()) return;
    const float Step = FMath::Max(0.f, Dt);
    BarkCooldown = FMath::Max(0.f, BarkCooldown - Step);
    AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this, 0));
    const bool bObservedNow = CanObserve(Player);
    if (bObservedNow)
    {
        LastActualEvidence = Player->GetActorLocation();
        ContactAge = 0.f;
        if (State == EMGSentryDogState::Patrol || State == EMGSentryDogState::Notice)
        {
            State = EMGSentryDogState::Notice;
            BarkFromActualLocation();
        }
    }
    else ContactAge += Step;

    if (State == EMGSentryDogState::Lunge)
    {
        LungeRemaining -= Step;
        if (LungeRemaining <= 0.f)
        {
            if (IsValid(Player) && CanObserve(Player) &&
                FVector::Dist2D(GetActorLocation(), Player->GetActorLocation()) < 145.f)
            {
                UGameplayStatics::ApplyDamage(Player, 15.f, nullptr, this,
                    UDamageType::StaticClass());
                OnSentryLungeImpact();
            }
            State = EMGSentryDogState::Recover;
            RecoverRemaining = .85f;
        }
        return;
    }
    if (State == EMGSentryDogState::Recover)
    {
        RecoverRemaining -= Step;
        if (RecoverRemaining <= 0.f) State = EMGSentryDogState::Pursue;
        return;
    }
    if (State == EMGSentryDogState::Notice)
    {
        ThreatElapsed += Step;
        if (ThreatElapsed >= .35f) State = EMGSentryDogState::Pursue;
        return;
    }
    if (State == EMGSentryDogState::Pursue && ContactAge < 4.f)
    {
        const float Distance = FVector::Dist2D(GetActorLocation(), LastActualEvidence);
        if (bObservedNow && Distance < 150.f)
        {
            State = EMGSentryDogState::Lunge;
            LungeRemaining = .36f;
            OnSentryLungeTelegraph();
        }
        else MoveDog(LastActualEvidence, 180.f, Step);
        return;
    }
    State = EMGSentryDogState::Patrol;
    ThreatElapsed = 0.f;
    const FVector ForwardEnd = Home + PatrolAxis * PatrolRadius;
    const FVector BackEnd = Home - PatrolAxis * PatrolRadius;
    const FVector Target = PatrolSign > 0.f ? ForwardEnd : BackEnd;
    if (FVector::Dist2D(GetActorLocation(), Target) < 20.f) PatrolSign *= -1.f;
    MoveDog(PatrolSign > 0.f ? ForwardEnd : BackEnd, 85.f, Step);
}

float AMGSentryDogActor::TakeDamage(float Amount, const FDamageEvent& Event,
    AController* Instigator, AActor* Causer)
{
    if (Amount <= 0.f || State == EMGSentryDogState::Incapacitated) return 0.f;
    Health = FMath::Max(0.f, Health - Amount);
    if (Health <= 0.f)
    {
        State = EMGSentryDogState::Incapacitated;
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        OnSentryIncapacitated();
    }
    return Amount;
}
