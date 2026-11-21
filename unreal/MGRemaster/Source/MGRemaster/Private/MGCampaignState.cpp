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
FString UMGCampaignState::GetMissionTitle() const
{
    switch (GetMissionNumber())
    {
    case 1: return TEXT("MISSION 01 / OPERATION INTRUDE N313");
    case 2: return TEXT("MISSION 02 / THE MISSING FOX");
    case 3: return TEXT("MISSION 03 / THE SCIENTIST TRAIL");
    default: return TEXT("MISSION / OUTER HEAVEN");
    }
}

FString UMGCampaignState::GetCharacterThread() const
{
    // A quiet story-state cue rather than a cutscene interrupt. Do not disclose
    // Big Boss's secret or the Venom retrospective before earned story events.
    if (GetMissionNumber() == 3)
        return TEXT("SNAKE / FOX'S WARNING CHANGES THE MISSION");
    if (GetMissionNumber() == 2)
        return HasBeat(TEXT("fox.revelation"))
            ? TEXT("FOX / SEEK THE ENGINEER WHO KNOWS THE MACHINE")
            : TEXT("FOX / MISSING OPERATIVE, POSSIBLY IMPRISONED");
    return HasBeat(TEXT("snake.early_pow"))
        ? TEXT("PRISONER / AN OPERATIVE IS HELD BEHIND THE WALLS")
        : TEXT("BIG BOSS / INITIAL ORDERS: FIND GRAY FOX");
}

int32 UMGCampaignState::GetRadioContactCount() const
{
    // Diane must first be introduced by a rescued prisoner. Her voice is
    // optional character texture, not a required unlock for prison capture.
    return HasBeat(TEXT("diane.contact")) ? 3 : 2;
}

FString UMGCampaignState::GetRadioContactLabel(int32 Channel) const
{
    if (Channel == 0) return TEXT("SCHNEIDER / 120.79");
    if (Channel == 1) return TEXT("BIG BOSS / 120.85");
    if (Channel == 2 && GetRadioContactCount() > 2)
        return TEXT("DIANE / 120.33");
    return TEXT("NO SIGNAL");
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
    // Original 1995 characters, remake-authored paraphrase. Dialogue responds
    // to earned first-hand events, not a prewritten mission-completion timer.
    if (Channel == 2)
        return HasBeat(TEXT("diane.contact")) ?
            (GetMissionNumber() == 3 ?
                TEXT("DIANE: So Fox was alive.\n"
                     "DIANE: Then the scientist is worth finding.\n"
                     "SNAKE: Any leads?\n"
                     "DIANE: Not yet. I'll ask the others.") :
                TEXT("DIANE: A prisoner gave you this channel?\n"
                     "DIANE: I know some of the fighters guarding this place.\n"
                     "SNAKE: Can you help me with them?\n"
                     "DIANE: When you find one, tell me what you see.")) :
            TEXT("CHANNEL NOT YET AVAILABLE");
    if (Channel == 0)
    {
        if (GetMissionNumber() == 3)
            return TEXT("SCHNEIDER: The scientist will be in the secured areas.\n"
                        "SCHNEIDER: Those routes have changed since I drew them.\n"
                        "SNAKE: Then I'll verify them in person.\n"
                        "SCHNEIDER: Good. I won't pretend I know every door.");
        return HasBeat(TEXT("snake.early_pow")) ?
            TEXT("SCHNEIDER: They changed the detention passages.\n"
                 "SNAKE: One of the prisoners saw an operative taken away.\n"
                 "SCHNEIDER: Follow what he saw, not my old drawings.") :
            TEXT("SCHNEIDER: I knew the fortress before it changed.\n"
                 "SCHNEIDER: Look for repairs that don't fit the structure.\n"
                 "SNAKE: Understood. I'll check the walls.");
    }
    if (Channel == 1)
        return GetMissionNumber() == 3 ?
            TEXT("BIG BOSS: Gray Fox gave you a lead. Follow it.\n"
                 "SNAKE: Pettrovich. I need to find him.\n"
                 "BIG BOSS: Correct. Do not lose the initiative.") :
            (HasBeat(TEXT("snake.early_pow")) ?
                TEXT("BIG BOSS: You've found evidence of detention.\n"
                     "SNAKE: They took a FOXHOUND operative inside.\n"
                     "BIG BOSS: Investigate, but don't waste time.") :
                TEXT("BIG BOSS: Operation Intrude N313.\n"
                     "BIG BOSS: Find Gray Fox. Investigate Metal Gear.\n"
                     "SNAKE: Understood. Beginning infiltration."));
    return TEXT("NO SIGNAL");
}
void UMGCampaignState::ResetForDevelopment()
{
    UGameplayStatics::DeleteGameInSlot(CampaignSlot, 0);
    Progress = Cast<UMGCampaignSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UMGCampaignSaveGame::StaticClass()));
    Commit();
}
