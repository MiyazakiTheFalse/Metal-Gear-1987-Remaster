#include "MGMachinegunKid.h"
#include "MGPlayerCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    // Original short mechanical cues. Generate only on an actual beat.
    void PlayFeedCue(AActor* Source, bool bShot)
    {
        constexpr int32 Rate = 24000;
        const int32 Count = bShot ? 1920 : 3600;
        TArray<int16> Samples;
        Samples.SetNumUninitialized(Count);
        FRandomStream Noise(bShot ? 1541 : 3229);
        for (int32 I=0; I<Count; ++I)
        {
            const float T = float(I)/Rate;
            const float Envelope = FMath::Min(T/.003f,1.f)*FMath::Exp(-T*(bShot?62.f:27.f));
            Samples[I] = int16(Envelope * (Noise.FRandRange(-4600.f,4600.f)
                + 3700.f*FMath::Sin(2.f*PI*T*(bShot?110.f:610.f))));
        }
        USoundWaveProcedural* Sound = NewObject<USoundWaveProcedural>(Source);
        Sound->SetSampleRate(Rate);
        Sound->NumChannels = 1;
        Sound->Duration = float(Count)/Rate;
        Sound->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()),Count*sizeof(int16));
        // Stop the component after its buffer drains (procedural waves can otherwise wait for more data).
        if (UAudioComponent* Audio = UGameplayStatics::SpawnSoundAtLocation(Source,Sound,Source->GetActorLocation()))
            Audio->FadeOut(Sound->Duration,0.f);
    }
}

AMGMachinegunKid::AMGMachinegunKid()
{
    Receiver = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SupportWeapon"));
    Receiver->SetupAttachment(RootComponent);
    Receiver->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Receiver->SetRelativeLocation(FVector(40.f,14.f,37.f));
    Receiver->SetRelativeScale3D(FVector(.88f,.12f,.16f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Receiver->SetStaticMesh(Cube.Object);
    Flash = CreateDefaultSubobject<UPointLightComponent>(TEXT("BurstFlash"));
    Flash->SetupAttachment(RootComponent);
    Flash->SetRelativeLocation(FVector(84.f,14.f,37.f));
    Flash->SetLightColor(FLinearColor(1.f,.57f,.20f));
    Flash->SetAttenuationRadius(260.f);
    Flash->SetCastShadows(false);
    Flash->SetIntensity(0.f);
}

void AMGMachinegunKid::BeginPlay()
{
    Super::BeginPlay();
    // This boss must have a UNIQUE identity and weapons rig. If the approved
    // 1995 model has not been assembled/imported, prototype gameplay remains
    // visible but cannot count as production-ready character art.
    const bool bLoaded = TryUseBespokeSkeletalArt(
        TEXT("/Game/Characters/Bosses/MachinegunKid/SK_MachinegunKid_1995.SK_MachinegunKid_1995"),
        TEXT("/Game/Characters/Bosses/MachinegunKid/ABP_MachinegunKid_1995.ABP_MachinegunKid_1995_C"));
    if (UStaticMeshComponent* GuardPistol = GetHeldWeaponVisual())
        GuardPistol->SetVisibility(false);
    if (UStaticMesh* AuthoredWeapon = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Weapons/MachinegunKid/SM_SupportMachinegun_1995.SM_SupportMachinegun_1995")))
    {
        Receiver->SetStaticMesh(AuthoredWeapon);
        Receiver->SetRelativeScale3D(FVector::OneVector);
    }
    if (bLoaded && GetRigMesh() &&
        GetRigMesh()->DoesSocketExist(TEXT("weapon_r")))
    {
        Receiver->AttachToComponent(GetRigMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("weapon_r"));
        Receiver->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
        Flash->AttachToComponent(Receiver,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        Flash->SetRelativeLocation(FVector(80.f, 0.f, 0.f));
    }
}

void AMGMachinegunKid::Tick(float Dt)
{
    // This actor uses its own encounter controller. Patrol logic would run a second weapon.
    AActor::Tick(Dt);
    AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player || IsIncapacitated()) return;
    const FVector P = Player->GetActorLocation();
    const FVector Muzzle = GetActorLocation() + FVector(0.f,0.f,48.f);
    const FVector Target = P + FVector(0.f,0.f,Player->IsProne() ? 0.f : 30.f);
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(MGKidLane), false, this);
    const bool bInRoom = P.X > 6480.f;
    const bool bSeen = bInRoom && (!GetWorld()->LineTraceSingleByChannel(
        Hit,Muzzle,Target,ECC_Visibility,Query) || Hit.GetActor() == Player);
    Flash->SetIntensity(0.f);
    Receiver->SetRelativeRotation(FRotator(Beat == EBeat::Reload ? -28.f : 0.f,0.f,0.f));
    Remaining -= Dt;
    if (Beat == EBeat::Waiting)
    {
        if (!bSeen) return;
        MarkedY = P.Y;
        SetActorRotation(FRotator(0.f,(Target-Muzzle).Rotation().Yaw,0.f));
        Player->NotifyFeedback(TEXT("MACHINEGUN KID — weapon rising"),TEXT("select"));
        Remaining = 1.f;
        Beat = EBeat::Sight;
        OnBossFiringPhase(TEXT("RaiseWeapon"),Remaining);
    }
    else if (Beat == EBeat::Sight && Remaining <= 0.f)
    {
        Beat = EBeat::Burst;
        Remaining = 1.15f;
        OnBossFiringPhase(TEXT("Burst"),Remaining);
        ShotInterval = 0.f;
    }
    else if (Beat == EBeat::Burst)
    {
        ShotInterval -= Dt;
        if (Remaining <= 0.f)
        {
            Beat = EBeat::Reload;
            Remaining = 1.8f;
            OnBossFiringPhase(TEXT("Reload"),Remaining);
            PlayFeedCue(this,false);
            if (bInRoom) Player->NotifyFeedback(TEXT("Feed cover open"),TEXT("select"));
        }
        else if (ShotInterval <= 0.f)
        {
            ShotInterval = .23f;
            Flash->SetIntensity(1200.f);
            PlayFeedCue(this,true);
            if (bSeen && FMath::Abs(P.Y-MarkedY)<75.f)
                UGameplayStatics::ApplyDamage(Player,8.f,nullptr,this,UDamageType::StaticClass());
        }
    }
    else if (Beat == EBeat::Reload && Remaining <= 0.f)
    {
        Beat = EBeat::Waiting;
        OnBossFiringPhase(TEXT("Waiting"),0.f);
    }
}
