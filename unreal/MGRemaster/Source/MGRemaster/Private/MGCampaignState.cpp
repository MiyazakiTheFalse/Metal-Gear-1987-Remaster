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
    if (Progress && Progress->Version == 2)
    {
        Progress->Version = 3;
        Commit(); // backward-compatible, same ledger; no old saves discarded
    }
    if (!Progress || Progress->Version != 3 ||
        Progress->MissionNumber < 1 || Progress->MissionNumber > 8)
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
    case 4: return TEXT("MISSION 04 / THE OUTER DEFENCES");
    case 5: return TEXT("MISSION 05 / THE PRICE OF METAL GEAR");
    case 6: return TEXT("MISSION 06 / THE COST OF LOYALTY");
    case 7: return TEXT("MISSION 07 / ORDERS AND EVIDENCE");
    case 8: return TEXT("MISSION 08 / OUTER HEAVEN");
    default: return TEXT("MISSION / OUTER HEAVEN");
    }
}

FString UMGCampaignState::GetCharacterThread() const
{
    // A quiet story-state cue rather than a cutscene interrupt. Do not disclose
    // Big Boss's secret or the Venom retrospective before earned story events.
    if (GetMissionNumber() == 8)
        return HasBeat(TEXT("snake.escaped_outer_heaven")) ?
            TEXT("SNAKE / HE SURVIVED ON HIS OWN TERMS") :
            TEXT("SNAKE / METAL GEAR AND HIS COMMANDER ARE THE FINAL TEST");
    if (GetMissionNumber() == 7)
        return TEXT("BIG BOSS / SNAKE MUST CHOOSE EVIDENCE OVER ORDERS");
    if (GetMissionNumber() == 6)
        return HasBeat(TEXT("brother.escape_ladder")) ?
            TEXT("JENNIFER / HER BROTHER'S LEFT-LADDER CLUE MATTERS") :
            TEXT("SNAKE / FALSE COMMAND INTELLIGENCE IS NOW A THREAT");
    if (GetMissionNumber() == 8)
        return HasBeat(TEXT("snake.escaped_outer_heaven")) ?
            TEXT("OUTER HEAVEN ESCAPED / OPERATION INTRUDE N313 COMPLETE") :
            HasBeat(TEXT("tx55.destruction")) ?
            TEXT("ESCAPE BEFORE DETONATION / LEFT LADDER CLUE") :
            TEXT("PLANT THE 16-CHARGE TX-55 FOOT DEMOLITION SEQUENCE");
    if (GetMissionNumber() == 7)
        return TEXT("FIND THE WITNESS / CHALLENGE BIG BOSS / CLEAR SECURITY");
    if (GetMissionNumber() == 6)
        return TEXT("CROSS THE DESERT / RESCUE JENNIFER'S BROTHER");
    if (GetMissionNumber() == 5)
        return HasBeat(TEXT("ellen.rescue")) ?
            TEXT("ELLEN / RESCUED — RETURN TO HER FATHER") :
            TEXT("MADNAR / SCIENTIST REFUSES TO COOPERATE UNTIL ELLEN IS SAFE");
    if (GetMissionNumber() == 4)
        return HasBeat(TEXT("jennifer.contact")) ?
            TEXT("JENNIFER / HELP IS EARNED THROUGH RESCUING PRISONERS") :
            TEXT("FOX / FOLLOW THE LEAD THROUGH THE DEFENDED RESEARCH COMPLEX");
    if (GetMissionNumber() == 3)
        return FString::Printf(
            TEXT("SNAKE / FOX'S LEAD VERIFIED BY %d OF 3 SOURCES"),
            GetMissionThreeIntelCount());
    if (GetMissionNumber() == 2)
        return HasBeat(TEXT("fox.revelation"))
            ? TEXT("FOX / SEEK THE ENGINEER WHO KNOWS THE MACHINE")
            : TEXT("FOX / MISSING OPERATIVE, POSSIBLY IMPRISONED");
    if (HasBeat(TEXT("fox.location_learned")) &&
        HasBeat(TEXT("fox.capture_method_learned")))
        return TEXT("FOX / 1F WEST DEAD END — SECRET CELL ROUTE");
    if (HasBeat(TEXT("diane.contact")))
        return TEXT("DIANE / A NEW RESISTANCE RADIO CONTACT");
    return HasBeat(TEXT("snake.early_pow"))
        ? TEXT("PRISONER / AN OPERATIVE IS HELD BEHIND THE WALLS")
        : TEXT("BIG BOSS / INITIAL ORDERS: FIND GRAY FOX");
}

int32 UMGCampaignState::GetRadioContactCount() const
{
    // Diane must first be introduced by a rescued prisoner. Her voice is
    // optional character texture, not a required unlock for prison capture.
    if (HasBeat(TEXT("jennifer.contact"))) return 4;
    return HasBeat(TEXT("diane.contact")) ? 3 : 2;
}

FString UMGCampaignState::GetRadioContactLabel(int32 Channel) const
{
    if (Channel == 0) return TEXT("SCHNEIDER / 120.79");
    if (Channel == 1) return TEXT("BIG BOSS / 120.85");
    if (Channel == 2 && GetRadioContactCount() > 2)
        return TEXT("DIANE / 120.33");
    if (Channel == 3 && GetRadioContactCount() > 3)
        return TEXT("JENNIFER / 120.48");
    return TEXT("NO SIGNAL");
}

int32 UMGCampaignState::GetMissionThreeIntelCount() const
{
    int32 Leads = 0;
    if (HasBeat(TEXT("m03.route.manifest"))) ++Leads;
    if (HasBeat(TEXT("m03.route.survey"))) ++Leads;
    if (HasBeat(TEXT("m03.route.archive"))) ++Leads;
    return Leads;
}
bool UMGCampaignState::IsMissionThreeExteriorActive() const
{
    return Progress && Progress->MissionNumber == 3 &&
        Progress->bMissionThreeExteriorActive;
}
bool UMGCampaignState::SetMissionThreeExteriorActive(bool bExterior)
{
    if (!Progress || Progress->MissionNumber != 3) return false;
    if (bExterior && !HasBeat(TEXT("m03.route.confirmed"))) return false;
    const bool Previous=Progress->bMissionThreeExteriorActive;
    Progress->bMissionThreeExteriorActive=bExterior;
    if (!Commit())
    {
        Progress->bMissionThreeExteriorActive=Previous;
        return false;
    }
    return true;
}
int32 UMGCampaignState::GetTX55CorrectCharges() const
{
    return Progress ? Progress->TX55CorrectCharges : 0;
}
bool UMGCampaignState::RecordTX55Charge(bool bRightFoot)
{
    if (!Progress || Progress->MissionNumber != 8 ||
        !HasBeat(TEXT("madnar.tx55_sequence"))) return false;
    // MSX2 source comparative: R R L R L L R L L R R L R L R R.
    static const bool RightFootSequence[]={
        true,true,false,true,false,false,true,false,
        false,true,true,false,true,false,true,true};
    if (Progress->TX55CorrectCharges >= UE_ARRAY_COUNT(RightFootSequence))
        return false;
    if (bRightFoot != RightFootSequence[Progress->TX55CorrectCharges])
        return false; // Wrong-side input is not an invisible softlock.
    ++Progress->TX55CorrectCharges;
    if (!Commit())
    {
        --Progress->TX55CorrectCharges;
        return false;
    }
    return true;
}
float UMGCampaignState::GetEscapeTimeRemaining() const
{
    return Progress ? Progress->EscapeSecondsRemaining : 120.f;
}
bool UMGCampaignState::SetEscapeTimeRemaining(float Seconds)
{
    if (!Progress || Progress->MissionNumber != 8 ||
        !HasBeat(TEXT("tx55.destruction"))) return false;
    const float Old=Progress->EscapeSecondsRemaining;
    Progress->EscapeSecondsRemaining=FMath::Clamp(Seconds,0.f,120.f);
    if (!Commit())
    {
        Progress->EscapeSecondsRemaining=Old;
        return false;
    }
    return true;
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
void UMGCampaignState::RevokeMisplacedDianeContact()
{
    if (!Progress || !HasBeat(TEXT("diane.contact"))) return;
    const TArray<FName> PreviousBeats = Progress->CompletedBeatIds;
    const TArray<FString> PreviousIntel = Progress->IntelLines;
    Progress->CompletedBeatIds.Remove(TEXT("diane.contact"));
    Progress->IntelLines.RemoveAll([](const FString& Entry)
    {
        return Entry.Contains(TEXT("Resistance contact Diane can be reached"));
    });
    if (!Commit())
    {
        Progress->CompletedBeatIds = PreviousBeats;
        Progress->IntelLines = PreviousIntel;
    }
}
bool UMGCampaignState::AdvanceMission(int32 NextMissionNumber)
{
    if (!Progress || NextMissionNumber != Progress->MissionNumber + 1 ||
        NextMissionNumber > 8)
        return false;
    // The source route is back to 1F: one prisoner identifies Fox's hidden
    // cell; a second explains the capture route. Neither an optional radio
    // conversation nor the earlier 3F POW alone is a valid substitute.
    if (NextMissionNumber == 2 &&
        (!HasBeat(TEXT("fox.location_learned")) ||
         !HasBeat(TEXT("fox.capture_method_learned"))))
        return false;
    // Only a completed Gray Fox prison escape unlocks returning to Building One.
    if (NextMissionNumber == 3 &&
        (!HasBeat(TEXT("fox.revelation")) ||
         !HasBeat(TEXT("snake.gear_recovery")) ||
         !HasBeat(TEXT("bosses.shoot_gunner")) ||
         !HasBeat(TEXT("m02.card3_acquired")) ||
         !HasBeat(TEXT("fox.intel_persists"))))
        return false;
    // Boss and map gates are physical encounters, not completed by merely
    // listening to an optional codec transmission.
    if (NextMissionNumber == 4 &&
        (!HasBeat(TEXT("m03.route.confirmed")) ||
         !HasBeat(TEXT("m03.machinegun_kid_defeated")) ||
         !HasBeat(TEXT("m03.hind_d_defeated")) ||
         !HasBeat(TEXT("m03.tank_defeated")))) return false;
    if (NextMissionNumber == 5 &&
        (!HasBeat(TEXT("m04.bulldozer_defeated")) ||
         !HasBeat(TEXT("m04.fire_trooper_defeated")) ||
         !HasBeat(TEXT("tx11.rocket_defeat")) ||
         !HasBeat(TEXT("m04.card7_claimed")))) return false;
    if (NextMissionNumber == 6 &&
        (!HasBeat(TEXT("ellen.rescue")) ||
         !HasBeat(TEXT("madnar.weakness")))) return false;
    if (NextMissionNumber == 7 &&
        (!HasBeat(TEXT("m06.coward_duck_defeated")) ||
         !HasBeat(TEXT("brother.escape_ladder")) ||
         !HasBeat(TEXT("m06.card8_acquired")))) return false;
    if (NextMissionNumber == 8 &&
        (!HasBeat(TEXT("pow.final_reveal")) ||
         !HasBeat(TEXT("snake.disobeys_abort")) ||
         !HasBeat(TEXT("m07.final_security_cleared")))) return false;
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
    if (GetMissionNumber() == 5)
        return HasBeat(TEXT("ellen.rescue")) ?
            TEXT("RETURN FROM BUILDING ONE BASEMENT / MADNAR AWAITS") :
            TEXT("FIND ELLEN IN BUILDING ONE BASEMENT / SAVE HER");
    if (GetMissionNumber() == 4)
        return HasBeat(TEXT("jennifer.rocket_help")) ?
            TEXT("RECOVER ROCKETS / DEFEAT THE TX-11 DEFENDERS") :
            TEXT("SEARCH BUILDING TWO / FIND JENNIFER AND THE ROCKETS");
    if (GetMissionNumber() == 3 && HasBeat(TEXT("m03.exterior_started")))
        return TEXT("EXTERIOR / DEFEAT MACHINEGUN KID, HIND D AND TANK");
    if (GetMissionNumber() == 2) return
        HasBeat(TEXT("fox.revelation"))
            ? TEXT("ESCAPE THE SECRET PRISON / KEEP FOX'S INTEL")
            : TEXT("FIND GRAY FOX IN THE SECRET CELLS");
    if (GetMissionNumber() == 3)
        return GetMissionThreeIntelCount() > 0 ?
            TEXT("SERVICE ROUTE ESTABLISHED / SEEK BUILDING TWO") :
            TEXT("FOLLOW FOX: INVESTIGATE THREE BUILDING ONE LEADS");
    if (HasBeat(TEXT("fox.location_learned")) &&
        HasBeat(TEXT("fox.capture_method_learned")))
        return TEXT("RETURN TO 1F WESTERN DEAD END — CAPTURE ROUTE");
    if (HasBeat(TEXT("diane.contact")))
        return TEXT("RETURN TO 1F / FIND THE TWO FOX-RELATED PRISONERS");
    if (HasBeat(TEXT("m01.electric.discovered")))
        return TEXT("STEER REMOTE MISSILE INTO THE ELECTRIC CONTROL PANEL");
    if (HasBeat(TEXT("snake.early_pow")))
        return TEXT("FIND CARD 2 AND THE WESTERN REMOTE MISSILE ROOM");
    return TEXT("INFILTRATE BUILDING ONE / RESCUE PRISONERS");
}
FString UMGCampaignState::GetOpeningRadio(int32 Channel) const
{
    // Original 1995 characters, remake-authored paraphrase. Dialogue responds
    // to earned first-hand events, not a prewritten mission-completion timer.
    if (Channel == 3 && HasBeat(TEXT("jennifer.contact")))
        return HasBeat(TEXT("jennifer.rocket_help")) ?
            TEXT("JENNIFER: I've put the rocket launcher in the next store.\n"
                 "JENNIFER: Take the rockets before you face the machines.\n"
                 "SNAKE: Understood. Thank you.") :
            TEXT("JENNIFER: Those machines are armoured. I can help.\n"
                 "JENNIFER: Stay close to the research supply room.\n"
                 "SNAKE: I'll collect what you've arranged.");
    if (Channel == 2 && GetMissionNumber() >= 4)
        return HasBeat(TEXT("jennifer.contact")) ?
            TEXT("DIANE: Jennifer knows the research facility.\n"
                 "DIANE: Help the prisoners first. Then call her.") :
            TEXT("DIANE: The research wing has its own security.\n"
                 "DIANE: Observe each guard before acting.");
    if (Channel == 2 && GetMissionNumber() == 3 &&
        GetMissionThreeIntelCount() >= 2)
        return TEXT("DIANE: You've confirmed that route twice.\n"
                    "DIANE: You may have missed what the service store kept.\n"
                    "SNAKE: Where?\n"
                    "DIANE: The second-floor supply bay is worth another look.");
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
        if (GetMissionNumber() == 1 &&
            HasBeat(TEXT("m01.electric.discovered")) &&
            !HasBeat(TEXT("m01.electric.disabled")))
            return TEXT("SNAKE: The floor is electrified. I can see its controller.\n"
                        "SCHNEIDER: A remote missile can reach it.\n"
                        "SNAKE: Where are the missiles?\n"
                        "SCHNEIDER: Check the western Card Two store.");
        if (GetMissionNumber() == 3 && HasBeat(TEXT("m03.route.survey")))
            return TEXT("SNAKE: The service road is visible from the upper floor.\n"
                        "SCHNEIDER: Then it survived the rebuilding.\n"
                        "SNAKE: You weren't certain?\n"
                        "SCHNEIDER: Not until you saw it yourself.");
        if (GetMissionNumber() == 3 && HasBeat(TEXT("m03.route.archive")))
            return TEXT("SNAKE: An old gas-wing diagram shows a service approach.\n"
                        "SCHNEIDER: That's a dangerous place to leave records.\n"
                        "SNAKE: But it confirms a route.\n"
                        "SCHNEIDER: Check the road itself before trusting it.");
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
    if (Channel == 1 && GetMissionNumber() == 3 &&
        GetMissionThreeIntelCount() > 0)
        return TEXT("SNAKE: The research service approach is marked.\n"
                    "BIG BOSS: Good. Confirm the building before entering.\n"
                    "SNAKE: The report checks out against Fox's lead.");
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
