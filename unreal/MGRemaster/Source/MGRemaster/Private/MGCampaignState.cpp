#include "MGCampaignState.h"
#include "MGCampaignSaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const FString CampaignSlot(TEXT("MG1995CampaignV1"));
}
void UMGCampaignState::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Progress = Cast<UMGCampaignSaveGame>(
        UGameplayStatics::LoadGameFromSlot(CampaignSlot, 0));
    if (Progress && Progress->Version == 1)
    {
        // Zero-based v1 campaign saves are migrated in place: 0->1, 1->2, 2->3.
        // Preserve the rescue/POW/intel ledger; never silently reset progress.
        if (Progress->Chapter >= 0 && Progress->Chapter <= 2)
        {
            Progress->MissionNumber = Progress->Chapter + 1;
            Progress->Version = 2;
            if (!Commit())
                UE_LOG(LogTemp, Warning, TEXT("Could not persist mission-number migration"));
        }
        else Progress = nullptr;
    }
    if (!Progress || Progress->Version != 2 ||
        Progress->MissionNumber < 1 || Progress->MissionNumber > 3)
        Progress = Cast<UMGCampaignSaveGame>(
            UGameplayStatics::CreateSaveGameObject(UMGCampaignSaveGame::StaticClass()));
}
bool UMGCampaignState::Commit()
{
    return Progress &&
        UGameplayStatics::SaveGameToSlot(Progress, CampaignSlot, 0);
}
int32 UMGCampaignState::GetMissionNumber() const
{
    return Progress ? Progress->MissionNumber : 1;
}
bool UMGCampaignState::HasBeat(FName Id) const
{
    return Progress && Progress->CompletedBeatIds.Contains(Id);
}
bool UMGCampaignState::RecordBeat(FName Id, const FString& Intel)
{
    if (!Progress || Id.IsNone() || HasBeat(Id)) return false;
    // Save transactions must roll back in memory on a storage failure.
    const int32 EarlierBeatCount = Progress->CompletedBeatIds.Num();
    const TArray<FString> EarlierIntel = Progress->IntelLines;
    Progress->CompletedBeatIds.Add(Id);
    if (!Intel.IsEmpty())
    {
        // Portable, replayable text, not a fixed subtitle-time window.
        Progress->IntelLines.Add(Intel);
        if (Progress->IntelLines.Num() > 32)
            Progress->IntelLines.RemoveAt(0, Progress->IntelLines.Num() - 32);
    }
    if (!Commit())
    {
        Progress->CompletedBeatIds.SetNum(EarlierBeatCount);
        Progress->IntelLines = EarlierIntel;
        UE_LOG(LogTemp, Warning, TEXT("Failed to save 1995 story beat %s"), *Id.ToString());
        return false;
    }
    return true;
}
bool UMGCampaignState::AdvanceMission(int32 NextMissionNumber)
{
    if (!Progress || NextMissionNumber != Progress->MissionNumber + 1 ||
        NextMissionNumber > 3)
        return false;
    // A prisoner clue, not a radio selection, unlocks the voluntary capture.
    if (NextMissionNumber == 2 && !HasBeat(TEXT("snake.early_pow"))) return false;
    // Only a completed Gray Fox prison escape unlocks returning to Building One.
    if (NextMissionNumber == 3 &&
        (!HasBeat(TEXT("fox.revelation")) ||
         !HasBeat(TEXT("snake.gear_recovery")) ||
         !HasBeat(TEXT("fox.intel_persists"))))
        return false;
    const int32 PreviousMissionNumber = Progress->MissionNumber;
    Progress->MissionNumber = NextMissionNumber;
    if (!Commit())
    {
        Progress->MissionNumber = PreviousMissionNumber;
        UE_LOG(LogTemp, Warning, TEXT("Failed to persist mission %d"), NextMissionNumber);
        return false;
    }
    return true;
}
FString UMGCampaignState::GetIntelLog() const
{
    if (!Progress) return FString();
    FString Lines;
    const int32 First = FMath::Max(0, Progress->IntelLines.Num() - 5);
    for (int32 Index = First; Index < Progress->IntelLines.Num(); ++Index)
    {
        if (!Lines.IsEmpty()) Lines += TEXT("\n");
        Lines += Progress->IntelLines[Index];
    }
    return Lines;
}
FString UMGCampaignState::GetCurrentObjective() const
{
    if (GetMissionNumber() == 2) return
        HasBeat(TEXT("fox.revelation"))
            ? TEXT("ESCAPE THE SECRET PRISON / KEEP FOX'S INTEL")
            : TEXT("FIND GRAY FOX IN THE SECRET CELLS");
    if (GetMissionNumber() == 3)
        return TEXT("GRAY FOX RESCUED / FIND DR. PETTROVICH");
    if (HasBeat(TEXT("snake.early_pow")))
        return TEXT("PRISONER CLUE: LOCATE THE DETENTION TRANSFER");
    return TEXT("INFILTRATE BUILDING ONE / RESCUE PRISONERS");
}
FString UMGCampaignState::GetOpeningRadio(int32 Channel) const
{
    if (Channel == 0)
        return HasBeat(TEXT("snake.early_pow")) ?
            TEXT("SCHNEIDER: I knew the fortress before it changed.\n"
                 "SCHNEIDER: Someone altered the detention passages.\n"
                 "SNAKE: And the missing operative?\n"
                 "SCHNEIDER: I can't see those plans. Stay alert.") :
            TEXT("SCHNEIDER: Outer Heaven has been rebuilt in stages.\n"
                 "SCHNEIDER: Trust the physical layout, not assumptions.\n"
                 "SNAKE: Understood.");
    return GetMissionNumber() == 3 ?
        TEXT("BIG BOSS: You found Gray Fox. Good.\n"
             "BIG BOSS: Find the scientist behind Metal Gear.\n"
             "SNAKE: I'm following the lead.") :
        TEXT("BIG BOSS: Operation Intrude N313.\n"
             "BIG BOSS: Locate Gray Fox and investigate Metal Gear.\n"
             "BIG BOSS: You are on your own. Keep the channel clear.");
}
void UMGCampaignState::ResetForDevelopment()
{
    UGameplayStatics::DeleteGameInSlot(CampaignSlot, 0);
    Progress = Cast<UMGCampaignSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UMGCampaignSaveGame::StaticClass()));
    Commit();
}
