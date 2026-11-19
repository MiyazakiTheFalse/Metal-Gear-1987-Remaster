#include "MGPrototypeHUD.h"

#include "MGEquipmentComponent.h"
#include "MGGrayFoxMission.h"
#include "MGGuardActor.h"
#include "MGPlayerCharacter.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const FLinearColor HudGreen(0.60f, 0.89f, 0.68f, 1.f);
    const FLinearColor HudMuted(0.48f, 0.66f, 0.57f, 1.f);
    const FLinearColor HudAmber(0.95f, 0.77f, 0.41f, 1.f);
    const FLinearColor HudRed(0.94f, 0.28f, 0.29f, 1.f);
    const FLinearColor HudPanel(0.018f, 0.060f, 0.061f, 0.90f);

    void DrawBar(AHUD* HUD, float X, float Y, float W, float H, float Pct,
                 const FLinearColor& Foreground)
    {
        HUD->DrawRect(FLinearColor(0.12f, 0.18f, 0.16f, 0.95f), X, Y, W, H);
        HUD->DrawRect(Foreground, X, Y, W * FMath::Clamp(Pct, 0.f, 1.f), H);
    }

    FString SlotLabel(const TArray<FMGEquipmentEntry>& Entries, int32 Index)
    {
        if (!Entries.IsValidIndex(Index)) return TEXT("NONE");
        return Entries[Index].Label;
    }
}

void AMGPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(GetOwningPawn());
    if (!Player) return;

    const float Width = Canvas->ClipX;
    const float Height = Canvas->ClipY;
    const float Margin = FMath::Clamp(Width * 0.018f, 12.f, 28.f);
    const float UiScale = FMath::Clamp(Height / 900.f, 0.76f, 1.20f);
    const UMGEquipmentComponent* Equip = Player->GetEquipment();

    // Safe margins and relative placement matter on 720p and widescreen.
    DrawRect(HudPanel, Margin, Margin, 310.f * UiScale, 98.f * UiScale);
    DrawText(TEXT("SOLID SNAKE / FOXHOUND"), HudGreen,
             Margin + 12.f * UiScale, Margin + 8.f * UiScale,
             nullptr, 0.91f * UiScale, false);
    DrawText(TEXT("LIFE"), FLinearColor::White,
             Margin + 12.f * UiScale, Margin + 37.f * UiScale,
             nullptr, 0.84f * UiScale, false);
    DrawBar(this, Margin + 65.f * UiScale, Margin + 39.f * UiScale,
            224.f * UiScale, 10.f * UiScale,
            Player->GetHealth() / 100.f,
            Player->GetHealth() <= 28.f ? HudRed : HudGreen);
    DrawText(FString::Printf(TEXT("%3.0f / 100    %s"),
             Player->GetHealth(), *Player->GetPostureText()), HudMuted,
             Margin + 12.f * UiScale, Margin + 62.f * UiScale,
             nullptr, 0.78f * UiScale, false);

    // Actual guard awareness drives radar vs threat overlay.
    int32 MaxThreat = 0;
    float MaxSuspicion = 0.f;
    for (TActorIterator<AMGGuardActor> Guard(GetWorld()); Guard; ++Guard)
    {
        if (Guard->IsIncapacitated()) continue;
        MaxThreat = FMath::Max(MaxThreat, static_cast<int32>(Guard->GetAwarenessState()));
        MaxSuspicion = FMath::Max(MaxSuspicion, Guard->GetSuspicion());
    }

    const float RadarSize = 170.f * UiScale;
    const float RadarX = Width - Margin - RadarSize;
    const float RadarY = Margin;
    DrawRect(HudPanel, RadarX, RadarY, RadarSize, RadarSize * 0.79f);
    if (MaxThreat >= 2)
    {
        const FLinearColor ThreatColor = MaxThreat >= 3 ? HudRed : HudAmber;
        DrawText(MaxThreat >= 3 ? TEXT("ALERT") : TEXT("EVASION"), ThreatColor,
                 RadarX + 22.f * UiScale, RadarY + 14.f * UiScale,
                 nullptr, 1.12f * UiScale, false);
        DrawText(TEXT("RADAR OFFLINE"), HudMuted,
                 RadarX + 15.f * UiScale, RadarY + 55.f * UiScale,
                 nullptr, 0.74f * UiScale, false);
        DrawBar(this, RadarX + 12.f * UiScale, RadarY + 99.f * UiScale,
                RadarSize - 24.f * UiScale, 8.f * UiScale,
                MaxSuspicion, ThreatColor);
    }
    else
    {
        DrawText(MaxThreat == 1 ? TEXT("RADAR / CAUTION") : TEXT("RADAR / NORMAL"),
                 MaxThreat == 1 ? HudAmber : HudGreen,
                 RadarX + 8.f * UiScale, RadarY + 8.f * UiScale,
                 nullptr, 0.75f * UiScale, false);

        const float X0 = RadarX + 13.f * UiScale;
        const float Y0 = RadarY + 29.f * UiScale;
        const float RW = RadarSize - 26.f * UiScale;
        const float RH = 91.f * UiScale;
        DrawRect(FLinearColor(0.05f, 0.13f, 0.13f, 1.f), X0, Y0, RW, RH);
        // Provisional prison topology; replace with verified original map data.
        const float SpanX = 26.6f * 100.f;
        const float SpanY = 10.f * 100.f;
        for (const float DividerWorldX : {-410.f, 450.f})
        {
            const float V = X0 + FMath::Clamp((DividerWorldX + 1250.f) / SpanX, 0.f, 1.f) * RW;
            DrawRect(HudMuted, V, Y0, 2.f * UiScale, RH * 0.34f);
            DrawRect(HudMuted, V, Y0 + RH * 0.66f, 2.f * UiScale, RH * 0.34f);
        }
        auto MarkPoint = [this, X0, Y0, RW, RH, UiScale](const FVector& World,
                        const FLinearColor& Tint, float Size)
        {
            const float Px = X0 + FMath::Clamp((World.X + 1250.f) / 2660.f, 0.f, 1.f) * RW;
            const float Py = Y0 + FMath::Clamp((World.Y + 500.f) / 1000.f, 0.f, 1.f) * RH;
            DrawRect(Tint, Px - Size * 0.5f * UiScale, Py - Size * 0.5f * UiScale,
                     Size * UiScale, Size * UiScale);
        };
        MarkPoint(Player->GetActorLocation(), HudGreen, 7.f);
        for (TActorIterator<AMGGuardActor> Guard(GetWorld()); Guard; ++Guard)
            if (!Guard->IsIncapacitated())
                MarkPoint(Guard->GetActorLocation(), MaxThreat == 1 ? HudAmber : HudRed, 5.f);
    }

    const AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
        UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass()));
    if (Mission)
    {
        if (Mission->IsTransmitterActive())
            DrawText(TEXT("TRACKING SIGNAL ACTIVE — DISCARD UNKNOWN ITEM"), HudRed,
                Width * .30f, Margin + 150.f * UiScale,
                nullptr, .9f * UiScale, false);
        DrawRect(HudPanel, Margin, Margin + 113.f * UiScale,
                 FMath::Min(540.f * UiScale, Width - 2.f * Margin), 112.f * UiScale);
        DrawText(FString::Printf(TEXT("MISSION / %s"), *Mission->GetPhaseText()),
                 HudGreen, Margin + 12.f * UiScale,
                 Margin + 120.f * UiScale, nullptr, 0.75f * UiScale, false);
        // A subtitle-only telegraph proxy until real audio/animation/VFX exist.
        // Indicates an actually observed hostile wind-up, not unseen enemy AI.
        if (Mission->IsBossTelegraphing())
            DrawText(TEXT("SHOTGUN AIMING — BREAK LINE OF SIGHT"), HudAmber,
                Width * .37f, Height * .78f, nullptr, 1.03f * UiScale, false);
        DrawText(Mission->GetObjectiveText(), FLinearColor::White,
                 Margin + 12.f * UiScale, Margin + 149.f * UiScale,
                 nullptr, 0.88f * UiScale, false);
        const FString Hint = Mission->GetPacingHint();
        if (!Hint.IsEmpty())
            DrawText(Hint, HudAmber, Margin + 12.f * UiScale,
                     Margin + 187.f * UiScale, nullptr, 0.78f * UiScale, false);
        // Never seize the player's camera just to deliver an optional clue.
        if (Mission->HasIncomingTransceiverPrompt() && !Player->IsCodecOpen())
            DrawText(TEXT("OPTIONAL RADIO CONTACT: C / START"), HudGreen,
                Margin + 12.f * UiScale, Margin + 210.f * UiScale,
                nullptr, 0.70f * UiScale, false);
        const FString Intel = Mission->GetMissionLogText();
        if (Player->IsMissionLogVisible() && !Intel.IsEmpty())
        {
            DrawRect(HudPanel, Margin, Height - 167.f * UiScale,
                     Width - 2.f * Margin, 41.f * UiScale);
            DrawText(Intel, HudGreen, Margin + 12.f * UiScale,
                     Height - 156.f * UiScale, nullptr, 0.78f * UiScale, false);
        }
        const FString Cue = Mission->GetRecentDialogue();
        if (!Cue.IsEmpty())
        {
            const float SubtitleY = Height - 104.f * UiScale;
            DrawRect(HudPanel, Margin, SubtitleY - 6.f * UiScale,
                     Width - 2.f * Margin, 44.f * UiScale);
            DrawText(Cue, FLinearColor::White, Margin + 15.f * UiScale,
                     SubtitleY, nullptr, 0.92f * UiScale, false);
        }
    }

    // Show actual selected equipment. The inventory lives in the component,
    // and unavailable slots cannot become equipped just because selected.
    if (Equip)
    {
        const float BoxWidth = 230.f * UiScale;
        const float BoxY = Height - 215.f * UiScale;
        DrawRect(HudPanel, Margin, BoxY, BoxWidth, 52.f * UiScale);
        DrawText(TEXT("LB / ITEMS"), HudMuted, Margin + 8.f * UiScale,
                 BoxY + 5.f * UiScale, nullptr, 0.70f * UiScale, false);
        DrawText(SlotLabel(Equip->GetItems(), Equip->GetEquippedItemIndex()), HudGreen,
                 Margin + 8.f * UiScale, BoxY + 27.f * UiScale,
                 nullptr, 0.82f * UiScale, false);

        const float WeaponX = Width - Margin - BoxWidth;
        DrawRect(HudPanel, WeaponX, BoxY, BoxWidth, 52.f * UiScale);
        DrawText(TEXT("RB / WEAPONS"), HudMuted, WeaponX + 8.f * UiScale,
                 BoxY + 5.f * UiScale, nullptr, 0.70f * UiScale, false);
        DrawText(SlotLabel(Equip->GetWeapons(), Equip->GetEquippedWeaponIndex()), HudGreen,
                 WeaponX + 8.f * UiScale, BoxY + 27.f * UiScale,
                 nullptr, 0.82f * UiScale, false);

        if (Equip->IsItemWindowOpen() || Equip->IsWeaponWindowOpen())
        {
            const bool bWeapon = Equip->IsWeaponWindowOpen();
            const TArray<FMGEquipmentEntry>& Entries =
                bWeapon ? Equip->GetWeapons() : Equip->GetItems();
            const int32 Selection = bWeapon ?
                Equip->GetWeaponSelection() : Equip->GetItemSelection();
            const float X = bWeapon ? WeaponX : Margin;
            const float Y = Height - (270.f + 38.f * Entries.Num()) * UiScale;
            DrawRect(HudPanel, X, Y, BoxWidth, (33.f + 31.f * Entries.Num()) * UiScale);
            DrawText(bWeapon ? TEXT("HOLD RB / SELECT WEAPON")
                             : TEXT("HOLD LB / SELECT ITEM"), HudGreen,
                     X + 8.f * UiScale, Y + 8.f * UiScale,
                     nullptr, 0.72f * UiScale, false);
            for (int32 Index = 0; Index < Entries.Num(); ++Index)
            {
                const FMGEquipmentEntry& Entry = Entries[Index];
                const bool bAvailable = Entry.bOwned &&
                    (!Equip->IsConfiscated() || Entry.Id == FName(TEXT("NONE"))) &&
                    (!Entry.bConsumable || Entry.Quantity > 0);
                const FString Text = FString::Printf(TEXT("%s %s%s"),
                    Index == Selection ? TEXT(">") : TEXT(" "),
                    *Entry.Label,
                    bAvailable ? TEXT("") : TEXT(" [LOCKED]"));
                DrawText(Text, !bAvailable ? HudRed :
                         (Index == Selection ? HudAmber : FLinearColor::White),
                         X + 8.f * UiScale,
                         Y + (40.f + Index * 31.f) * UiScale, nullptr, 0.75f * UiScale, false);
            }
        }

        const FString Feedback = Equip->GetEquipmentFeedback();
        if (!Feedback.IsEmpty())
            DrawText(Feedback, HudAmber, Width * 0.5f - 210.f * UiScale,
                     Height - 145.f * UiScale, nullptr, 0.9f * UiScale, false);
    }

    if (Player->IsCodecOpen())
    {
        // Non-pausing 1995 transceiver: readable at 720p, no camera seizure,
        // player can keep walking; real VO/spectrum treatment is still missing.
        const int32 Channel = Player->GetTransceiverChannel();
        const FString Transcript = Mission ?
            Mission->GetTransceiverDialogue(Channel) :
            TEXT("NO SIGNAL — LOCAL CONTACTS UNAVAILABLE");
        TArray<FString> Lines;
        Transcript.ParseIntoArrayLines(Lines, true);
        const float PanelWidth = FMath::Min(580.f * UiScale, Width - 2.f * Margin);
        const float PanelX = Width - Margin - PanelWidth;
        const float PanelY = Height * 0.31f;
        const int32 VisibleLines = FMath::Min(Lines.Num(), 8);
        const float PanelHeight = (71.f + 23.f * VisibleLines) * UiScale;
        DrawRect(HudPanel, PanelX, PanelY, PanelWidth, PanelHeight);
        DrawText(Channel == 0 ? TEXT("TRANSCEIVER / 120.79 / KYLE SCHNEIDER")
                              : TEXT("TRANSCEIVER / 120.85 / BIG BOSS"),
                 HudGreen, PanelX + 12.f * UiScale, PanelY + 8.f * UiScale,
                 nullptr, 0.82f * UiScale, false);
        for (int32 Index = 0; Index < VisibleLines; ++Index)
            DrawText(Lines[Index], FLinearColor::White,
                     PanelX + 12.f * UiScale,
                     PanelY + (34.f + 23.f * Index) * UiScale,
                     nullptr, 0.74f * UiScale, false);
        DrawText(TEXT("Q / RS: TUNE     C / START: CLOSE     MOVE: CONTINUE"),
                 HudMuted, PanelX + 12.f * UiScale,
                 PanelY + (49.f + 23.f * VisibleLines) * UiScale,
                 nullptr, 0.66f * UiScale, false);
    }

    if (Player->IsAiming())
    {
        const float X = Width * 0.5f;
        const float Y = Height * 0.5f;
        DrawLine(X - 11.f, Y, X - 3.f, Y, FLinearColor::White, 1.f);
        DrawLine(X + 3.f, Y, X + 11.f, Y, FLinearColor::White, 1.f);
        DrawLine(X, Y - 11.f, X, Y - 3.f, FLinearColor::White, 1.f);
        DrawLine(X, Y + 3.f, X, Y + 11.f, FLinearColor::White, 1.f);
    }

    const FString CoverHelp = Player->IsInCover() ?
        TEXT("WALL: F / A KNOCK   AIM + SLIDE: CORNER PEEK   MOVE AWAY: RELEASE") :
        TEXT("E / X: BREACH / INTERACT   MOVE INTO WALL: COVER   F / A: MELEE");
    DrawText(CoverHelp,
             HudMuted, Margin, Height - 27.f * UiScale, nullptr, 0.69f * UiScale, false);
}
