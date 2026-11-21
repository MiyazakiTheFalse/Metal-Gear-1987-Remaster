#include "MGAdaptiveAudioDirector.h"
#include "MGCampaignState.h"
#include "MGPlayerCharacter.h"
#include "MGSetpieceBoss.h"
#include "MGGuardActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AMGAdaptiveAudioDirector::AMGAdaptiveAudioDirector()
{
    PrimaryActorTick.bCanEverTick=true;
    PrimaryActorTick.TickInterval=.075f;
}
void AMGAdaptiveAudioDirector::BeginPlay()
{
    Super::BeginPlay();
    TransitionTo(DetermineMusic(Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this,0))));
}
void AMGAdaptiveAudioDirector::EndPlay(const EEndPlayReason::Type Reason)
{
    if (IsValid(MusicA)) MusicA->Stop();
    if (IsValid(MusicB)) MusicB->Stop();
    Super::EndPlay(Reason);
}
void AMGAdaptiveAudioDirector::SetBusLevels(float Music,float Effects)
{
    MusicGain=FMath::Clamp(Music,0.f,1.f)*.23f;
    EffectsGain=FMath::Clamp(Effects,0.f,1.f);
    // Mixer update is applied to the current music component in Tick.
}
USoundBase* AMGAdaptiveAudioDirector::FindSound(FName Cue,bool bMusic)
{
    if (Cue.IsNone()) return nullptr;
    if (USoundBase** Existing=Cache.Find(Cue)) return *Existing;
    const FString N=Cue.ToString();
    const TCHAR* Folder=bMusic?TEXT("Music"):TEXT("FX");
    const FString Asset=FString::Printf(
        TEXT("/Game/Audio/%s/%s.%s"),Folder,*N,*N);
    USoundBase* Loaded=LoadObject<USoundBase>(nullptr,*Asset);
    Cache.Add(Cue,Loaded);
    if (!Loaded && !IsRunningDedicatedServer())
        UE_LOG(LogTemp,Warning,TEXT("Audio asset missing: %s"),*Asset);
    return Loaded;
}
bool AMGAdaptiveAudioDirector::PlayCue(FName CueId,FVector Origin,bool bSpatial)
{
    static const TMap<FName,FName> Aliases={
        {TEXT("hit"),TEXT("punch1")}, {TEXT("step"),TEXT("step_concrete")},
        {TEXT("bossStep"),TEXT("boss_step")},{TEXT("alert"),TEXT("guard_alert")},
        {TEXT("rack"),TEXT("gear")},{TEXT("seal"),TEXT("door")},
        {TEXT("propertyPing"),TEXT("pickup")},{TEXT("kennel"),TEXT("boss_windup")},
        {TEXT("rescue"),TEXT("win")},{TEXT("impact"),TEXT("kick")},
        {TEXT("hurt"),TEXT("hit")},{TEXT("game_over_radio"),TEXT("radio_lost")},
        {TEXT("menu"),TEXT("menu")},{TEXT("whoosh"),TEXT("whoosh")}};
    const FName Resolved=Aliases.Contains(CueId)?Aliases[CueId]:CueId;
    USoundBase* Sound=FindSound(Resolved,false);
    if (!Sound) return false;
    const double Now=GetWorld()->GetTimeSeconds();
    const double* Prev=CueLastPlayed.Find(Resolved);
    // Prevent machine-gun repetition of UI notifications or repeated guard
    // queries turning into clipping. Mission/boss cues get their own cooldown.
    const double MinSpacing=
        Resolved==TEXT("step_concrete")||Resolved==TEXT("step_metal")||
        Resolved==TEXT("step_soil")?.14:
        Resolved==TEXT("alarm")?.85:.045;
    if (Prev && Now-*Prev<MinSpacing) return true;
    CueLastPlayed.Add(Resolved,Now);
    if (bSpatial)
    {
        float Gain=EffectsGain;
        if (const AActor* Listener=UGameplayStatics::GetPlayerPawn(this,0))
        {
            const float Dist=FVector::Dist(Listener->GetActorLocation(),Origin);
            Gain*=FMath::Clamp(1.f/(1.f+FMath::Square(Dist/850.f)),.025f,1.f);
        }
        UGameplayStatics::PlaySoundAtLocation(this,Sound,Origin,Gain);
    }
    else UGameplayStatics::PlaySound2D(this,Sound,EffectsGain*.64f);
    return true;
}
FName AMGAdaptiveAudioDirector::DetermineMusic(
    const AMGPlayerCharacter* Snake) const
{
    const UMGCampaignState* Story=GetWorld()&&GetWorld()->GetGameInstance()?
        GetWorld()->GetGameInstance()->GetSubsystem<UMGCampaignState>():nullptr;
    if (!Story) return TEXT("music_title");
    if (Story->IsTitleScreenActive()) return TEXT("music_title");
    if (Snake && Snake->IsGameOver()) return TEXT("music_game_over");
    const int32 Mission=Story->GetMissionNumber();
    if (Mission==8)
    {
        if (Story->HasBeat(TEXT("tx55.destruction"))) return TEXT("music_escape");
        return TEXT("music_metalgear");
    }
    if (Mission==7) return TEXT("music_betrayal");
    if (Mission==6) return TEXT("music_desert");
    if (Mission==5) return TEXT("music_revelation");
    int32 Awareness=0;
    if (Snake && !Snake->IsTitleScreenActive())
        for (TActorIterator<AMGGuardActor> Guard(GetWorld());Guard;++Guard)
            if (!Guard->IsIncapacitated() &&
                FVector::Dist(Guard->GetActorLocation(),
                              Snake->GetActorLocation())<2400.f &&
                FMath::Abs(Guard->GetActorLocation().Z-
                           Snake->GetActorLocation().Z)<260.f)
                Awareness=FMath::Max(Awareness,(int32)Guard->GetAwarenessState());
    // Gameplay-observed boss range is a meaningful encounter trigger.
    if (Snake)
        for (TActorIterator<AMGSetpieceBoss> Boss(GetWorld());Boss;++Boss)
            if (!Boss->IsDefeated() &&
                FVector::Dist(Boss->GetActorLocation(),
                              Snake->GetActorLocation())<1450.f)
                return TEXT("music_boss");
    if (Awareness>=3) return TEXT("music_alert");
    if (Awareness>0) return TEXT("music_caution");
    if (Mission==2) return TEXT("music_prison");
    if (Mission==4) return TEXT("music_research");
    if (Mission==3 && Story->IsMissionThreeExteriorActive())
        return TEXT("music_infiltration");
    return TEXT("music_infiltration");
}
void AMGAdaptiveAudioDirector::TransitionTo(FName Cue)
{
    if (Cue==ActiveMusic) return;
    USoundBase* Sound=FindSound(Cue,true);
    if (!Sound) return; // retain existing cue on missing assets
    UAudioComponent*& NewSlot=(MusicA==nullptr || !MusicA->IsPlaying())?
        MusicA:MusicB;
    if (NewSlot && NewSlot->IsPlaying())
    {
        // if both slots are already active, recycle the quieter fading slot.
        NewSlot->FadeOut(.16f,0.f);
        NewSlot->Stop();
    }
    NewSlot=UGameplayStatics::SpawnSound2D(
        this,Sound,0.f,1.f,0.f,nullptr,false,false);
    if (!NewSlot) return;
    NewSlot->bAutoDestroy=false;
    // Ambient->danger changes are fast; dramatic story cues breathe longer.
    const float Fade=Cue==TEXT("music_alert")? .38f:
                     Cue==TEXT("music_game_over")? .25f:1.25f;
    if (IsValid(MusicA)&&MusicA!=NewSlot&&MusicA->IsPlaying())
        MusicA->FadeOut(Fade,0.f);
    if (IsValid(MusicB)&&MusicB!=NewSlot&&MusicB->IsPlaying())
        MusicB->FadeOut(Fade,0.f);
    NewSlot->FadeIn(Fade,MusicGain);
    ActiveMusic=Cue;
}
FName AMGAdaptiveAudioDirector::SurfaceStepCue(
    const AMGPlayerCharacter* Snake) const
{
    if (!Snake || !GetWorld()) return TEXT("step_concrete");
    FHitResult H;
    FCollisionQueryParams Q(SCENE_QUERY_STAT(MGStepSurface),false,Snake);
    if (GetWorld()->LineTraceSingleByChannel(H,Snake->GetActorLocation(),
        Snake->GetActorLocation()-FVector(0,0,150.f),ECC_Visibility,Q))
    {
        if (AActor* Floor=H.GetActor())
        {
            if (Floor->ActorHasTag(TEXT("MG_Surface_Metal")) ||
                Floor->GetName().Contains(TEXT("Metal")) ||
                Floor->GetName().Contains(TEXT("Service")))
                return TEXT("step_metal");
            if (Floor->GetName().Contains(TEXT("Desert")) ||
                Floor->GetName().Contains(TEXT("ExteriorRoad")))
                return TEXT("step_soil");
        }
    }
    return TEXT("step_concrete");
}
void AMGAdaptiveAudioDirector::ProcessPlayerFoley(
    float Dt,const AMGPlayerCharacter* Snake)
{
    if (!Snake || Snake->IsGameOver()) return;
    const float Speed=Snake->GetVelocity().Size2D();
    if (Speed<70.f) {StepClock=0.f;return;}
    const float StepPeriod=Snake->IsProne() ? .95f :
        Snake->IsSprinting() ? .31f : Snake->IsInCover() ? .63f : .48f;
    StepClock+=Dt;
    if (StepClock>StepPeriod)
    {
        StepClock=FMath::Fmod(StepClock,StepPeriod);
        PlayCue(SurfaceStepCue(Snake),
                Snake->GetActorLocation()-FVector(0,0,75.f),true);
    }
}
void AMGAdaptiveAudioDirector::Tick(float Dt)
{
    Super::Tick(Dt);
    const AMGPlayerCharacter* Snake=Cast<AMGPlayerCharacter>(
        UGameplayStatics::GetPlayerCharacter(this,0));
    const FName Desired=DetermineMusic(Snake);
    if (Desired!=ActiveMusic) TransitionTo(Desired);
    if (Snake)
    {
        int32 MaxAwareness=0;
        for (TActorIterator<AMGGuardActor> Guard(GetWorld());Guard;++Guard)
            if (!Guard->IsIncapacitated() &&
                FMath::Abs(Guard->GetActorLocation().Z-
                           Snake->GetActorLocation().Z)<260.f)
                MaxAwareness=FMath::Max(MaxAwareness,(int32)Guard->GetAwarenessState());
        if (MaxAwareness>=3 && PreviousAwareness<3)
            PlayCue(TEXT("alarm"),Snake->GetActorLocation(),false);
        PreviousAwareness=MaxAwareness;
        if (bHasHealthSnapshot && Snake->GetHealth()<LastHealth-.01f &&
            !Snake->IsGameOver())
            PlayCue(TEXT("hit"),Snake->GetActorLocation(),true);
        LastHealth=Snake->GetHealth();
        bHasHealthSnapshot=true;
        if (Snake->IsCodecOpen()!=bPreviousCodec)
        {
            PlayCue(Snake->IsCodecOpen()?
                    TEXT("radio"):TEXT("radio_lost"),FVector::ZeroVector,false);
            bPreviousCodec=Snake->IsCodecOpen();
        }
        if (Snake->IsGameOver()&&!bPreviousGameOver)
            PlayCue(TEXT("game_over_sting"),FVector::ZeroVector,false);
        bPreviousGameOver=Snake->IsGameOver();
        // Actual mix duck when opening the non-pausing codec: readable
        // dialogue takes precedence without restarting the music loop.
        const float Target=Snake->IsCodecOpen()?MusicGain*.29f:MusicGain;
        if (IsValid(MusicA) && MusicA->IsPlaying())
            MusicA->SetVolumeMultiplier(FMath::FInterpTo(
                MusicA->VolumeMultiplier,Target,Dt,5.f));
        if (IsValid(MusicB) && MusicB->IsPlaying())
            MusicB->SetVolumeMultiplier(FMath::FInterpTo(
                MusicB->VolumeMultiplier,Target,Dt,5.f));
        ProcessPlayerFoley(Dt,Snake);
    }
}
