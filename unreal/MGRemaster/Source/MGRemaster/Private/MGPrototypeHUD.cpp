#include "MGPrototypeHUD.h"

#include "MGEquipmentComponent.h"
#include "MGGrayFoxMission.h"
#include "MGGuardActor.h"
#include "MGShootGunnerActor.h"
#include "MGSurveillanceCameraActor.h"
#include "MGPlayerCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
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

UTexture2D* AMGPrototypeHUD::FindPortraitTexture(
    const FString& CharacterId, const FString& Expression)
{
    const FString Filename = FString::Printf(TEXT("T_Codec_%s_%s"),
        *CharacterId, *Expression);
    if (const TObjectPtr<UTexture2D>* Cached = CachedCodecPortraits.Find(Filename))
        return Cached->Get();
    if (MissingCodecPortraits.Contains(Filename)) return nullptr;
    const FString AssetPath = FString::Printf(TEXT("/Game/UI/Codec/%s.%s"),
        *Filename, *Filename);
    if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *AssetPath))
    {
        CachedCodecPortraits.Add(Filename, Texture);
        return Texture;
    }
    MissingCodecPortraits.Add(Filename);
    return nullptr;
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
    const AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
        UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass()));
    if (Mission && Mission->IsStoryCinematicPlaying())
    {
        // A real live in-world shot, not a still image or a fake film overlay.
        // Suppress radar, mission arrows and aiming UI while directing the
        // camera, but retain the actual scene's earned spoken subtitle.
        const float Bar = FMath::Clamp(Height * .084f, 26.f, 89.f);
        DrawRect(FLinearColor(0.008f,.014f,.018f,1.f),0.f,0.f,Width,Bar);
        DrawRect(FLinearColor(0.008f,.014f,.018f,1.f),
                 0.f,Height-Bar,Width,Bar);
        DrawText(FString::Printf(TEXT("OUTER HEAVEN / 1995    %s"),
            *Mission->GetActiveStoryCinematic().ToString().ToUpper()),
            HudGreen, Margin, Bar*.34f,nullptr,.78f*UiScale,false);
        const FString Line = Mission->GetRecentDialogue();
        if (!Line.IsEmpty())
            DrawText(Line,FLinearColor::White,Margin,Height-Bar*.67f,
                     nullptr,.87f*UiScale,false);
        DrawText(TEXT("ESC / B   SKIP"),HudMuted,
                 Width - Margin - 154.f*UiScale,Height-Bar*.67f,
                 nullptr,.65f*UiScale,false);
        return;
    }

    // Authenticated hit feedback, independent of projectile visibility.
    // Edge bars preserve sight of targets, guards and aim reticle. The authored
    // skeletal rig can also consume GetHitReactionAlpha for a local flinch.
    const float Hit = Player->GetHitReactionAlpha();
    if (Hit > 0.f)
    {
        const FLinearColor Edge(0.72f, 0.13f, 0.09f, .43f * Hit);
        const float Rim = FMath::Clamp(Height * .035f, 11.f, 36.f);
        DrawRect(Edge, 0.f, 0.f, Width, Rim);
        DrawRect(Edge, 0.f, Height - Rim, Width, Rim);
        DrawRect(Edge, 0.f, Rim, Rim, Height - 2.f * Rim);
        DrawRect(Edge, Width - Rim, Rim, Rim, Height - 2.f * Rim);
    }

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

    // Transmitter alert is a real signal, not an invisible normal patrol.
    const EMGFacilitySecurityState Facility = Mission ?
        Mission->GetFacilitySecurityState() : EMGFacilitySecurityState::Normal;
    int32 MaxThreat = Mission && Mission->IsTransmitterActive() ? 3 : 0;
    if (Facility == EMGFacilitySecurityState::Alert)
        MaxThreat = FMath::Max(MaxThreat, 3);
    else if (Facility == EMGFacilitySecurityState::Evasion)
        MaxThreat = FMath::Max(MaxThreat, 2);
    else if (Facility == EMGFacilitySecurityState::Caution)
        MaxThreat = FMath::Max(MaxThreat, 1);
    float MaxSuspicion = 0.f;
    for (TActorIterator<AMGGuardActor> Guard(GetWorld()); Guard; ++Guard)
    {
        if (Guard->IsIncapacitated()) continue;
        // State of a distant patrol must not be mislabelled as a confirmed
        // facility-wide enemy contact. Only direct local sight and a
        // transmitted facility report can place the radar in ALERT.
        const bool bDirectThreat = Guard->HasDirectVisualContact()
            && Guard->GetAwarenessState() >= 3;
        if (bDirectThreat) MaxThreat = FMath::Max(MaxThreat, 3);
        else if (Guard->GetAwarenessPhase() ==
                 EMGGuardAwarenessPhase::Searching ||
                 Guard->GetAwarenessPhase() ==
                 EMGGuardAwarenessPhase::Investigating ||
                 Guard->GetAwarenessPhase() ==
                 EMGGuardAwarenessPhase::Suspicious)
            MaxThreat = FMath::Max(MaxThreat, 1);
        MaxSuspicion = FMath::Max(MaxSuspicion, Guard->GetSuspicion());
    }

    // Surveillance exposure is observable feedback from a real CCTV actor,
    // not a guaranteed alert or invisible global detection oracle.
    float CCTVLock = 0.f;
    bool bCCTVWatching = false;
    for (TActorIterator<AMGSurveillanceCameraActor> CCTV(GetWorld()); CCTV; ++CCTV)
    {
        if (!CCTV->IsOperational()) continue;
        if (CCTV->IsTrackingPlayer())
        {
            bCCTVWatching = true;
            CCTVLock = FMath::Max(CCTVLock, CCTV->GetLockFraction());
        }
    }
    if (bCCTVWatching && MaxThreat < 2)
        MaxThreat = FMath::Max(MaxThreat, 1); // CAUTION, not ALERT until report.

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
        // Three separate physical maps. The earlier projection clamped the
        // entire prelude and its guards onto the left prison-radar edge.
        const bool bPrelude = Player->GetActorLocation().X < -1250.f;
        const bool bPostCard3 = Player->GetActorLocation().X > 3800.f;
        const bool bBossWing = !bPostCard3 && Player->GetActorLocation().X > 1350.f;
        const float MinX = bPrelude ? -3900.f :
            (bPostCard3 ? 3750.f : (bBossWing ? 1350.f : -1250.f));
        const float SpanX = bPrelude ? 2700.f :
            (bPostCard3 ? 1850.f : (bBossWing ? 2400.f : 2650.f));
        const float MinY = bBossWing ? -1105.f : -500.f;
        const float SpanY = bBossWing ? 2210.f : 1000.f;
        if (!bBossWing && !bPrelude)
            for (const float DividerWorldX : {-300.f, 470.f})
            {
                const float V = X0 + FMath::Clamp((DividerWorldX - MinX) / SpanX, 0.f, 1.f) * RW;
                DrawRect(HudMuted, V, Y0, 2.f * UiScale, RH * .34f);
                DrawRect(HudMuted, V, Y0 + RH * .66f, 2.f * UiScale, RH * .34f);
            }
        auto MarkPoint = [this, X0, Y0, RW, RH, UiScale, MinX, SpanX, MinY, SpanY](
            const FVector& World, const FLinearColor& Tint, float Size)
        {
            if (World.X < MinX || World.X > MinX + SpanX ||
                World.Y < MinY || World.Y > MinY + SpanY) return;
            const float Px = X0 + FMath::Clamp((World.X - MinX) / SpanX, 0.f, 1.f) * RW;
            const float Py = Y0 + FMath::Clamp((World.Y - MinY) / SpanY, 0.f, 1.f) * RH;
            DrawRect(Tint, Px - Size * .5f * UiScale, Py - Size * .5f * UiScale,
                     Size * UiScale, Size * UiScale);
        };
        MarkPoint(Player->GetActorLocation(), HudGreen, 7.f);
        for (TActorIterator<AMGGuardActor> Guard(GetWorld()); Guard; ++Guard)
            if (!Guard->IsIncapacitated())
                MarkPoint(Guard->GetActorLocation(), MaxThreat == 1 ? HudAmber : HudRed, 5.f);
        for (TActorIterator<AMGSurveillanceCameraActor> CCTV(GetWorld()); CCTV; ++CCTV)
            if (CCTV->IsOperational())
                MarkPoint(CCTV->GetActorLocation(),
                    CCTV->IsTrackingPlayer() ? HudAmber : HudMuted, 5.f);
        if (bBossWing)
            for (TActorIterator<AMGShootGunnerActor> Gunner(GetWorld()); Gunner; ++Gunner)
                if (Gunner->IsEncounterActive() && !Gunner->IsDefeated())
                    MarkPoint(Gunner->GetActorLocation(), HudRed, 8.f);
    }

    // At 720p this short line stays away from the mission log and HUD bars.
    // It is only drawn during a camera's genuine sighting, and disappears
    // when Snake breaks visual contact or the camera is disabled.
    if (bCCTVWatching && MaxThreat < 3)
    {
        const float AlertX = RadarX - 5.f * UiScale;
        const float AlertY = RadarY + RadarSize * .86f;
        DrawText(TEXT("CCTV ACQUIRING"), HudAmber,
            AlertX, AlertY, nullptr, .73f * UiScale, false);
        DrawBar(this, AlertX, AlertY + 19.f * UiScale,
            RadarSize, 5.f * UiScale, CCTVLock, HudAmber);
    }

    // A visual/caption fallback for real security speech. Only a nearby
    // guard's actual transition can produce it; no omniscient global subtitles.
    const AMGGuardActor* AudibleGuard = nullptr;
    float NearestCalloutCm = 950.f;
    for (TActorIterator<AMGGuardActor> Guard(GetWorld()); Guard; ++Guard)
    {
        if (Guard->IsIncapacitated() || Guard->GetActiveGuardCallout().IsEmpty())
            continue;
        const float Distance = FVector::Dist2D(
            Guard->GetActorLocation(), Player->GetActorLocation());
        if (Distance < NearestCalloutCm)
        {
            AudibleGuard = *Guard;
            NearestCalloutCm = Distance;
        }
    }
    if (AudibleGuard)
        DrawText(AudibleGuard->GetActiveGuardCallout(), HudAmber,
            Width * .28f, Height * .70f, nullptr, .83f * UiScale, false);

    if (Mission)
    {
        if (Mission->IsTransmitterActive())
            DrawText(TEXT("TRACKING SIGNAL ACTIVE — DISCARD UNKNOWN ITEM"), HudRed,
                Width * .30f, Margin + 150.f * UiScale,
                nullptr, .9f * UiScale, false);
        DrawRect(HudPanel, Margin, Margin + 113.f * UiScale,
                 FMath::Min(600.f * UiScale, Width - 2.f * Margin), 164.f * UiScale);
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
        // Spatially truthful interaction affordance. The player can approach,
        // turn away or retreat and see the prompt appear/disappear as reach
        // and physical occlusion change. Hidden story flags grant no hints.
        const FString Context = Mission->GetContextActionPrompt(Player);
        if (!Context.IsEmpty() && !Mission->IsBossTelegraphing())
            DrawText(Context, HudAmber, Margin + 12.f * UiScale,
                     Margin + 179.f * UiScale, nullptr, .77f * UiScale, false);
        const FString Hint = Mission->GetPacingHint();
        if (!Hint.IsEmpty())
            DrawText(Hint, HudAmber, Margin + 12.f * UiScale,
                     Margin + 211.f * UiScale, nullptr, 0.75f * UiScale, false);
        // A personal lesson from Gray Fox has a measurable gameplay
        // benefit only when a real guard's footsteps can be heard nearby.
        // This cue respects the ordinary guard hearing/occlusion geometry.
        const FString FoxBoots = Mission->GetFoxPatrolListeningCue();
        if (!FoxBoots.IsEmpty() && !Player->IsCodecOpen())
            DrawText(FoxBoots, HudGreen, Margin + 12.f * UiScale,
                     Margin + 241.f * UiScale, nullptr, .70f * UiScale, false);
        // Never seize the player's camera just to deliver an optional clue.
        if (Mission->HasIncomingTransceiverPrompt() && !Player->IsCodecOpen())
            DrawText(TEXT("OPTIONAL RADIO CONTACT: C / START"), HudGreen,
                Margin + 12.f * UiScale, Margin + 266.f * UiScale,
                nullptr, 0.70f * UiScale, false);
        const FString Intel = Mission->GetMissionLogText();
        if (Player->IsMissionLogVisible() && !Intel.IsEmpty())
        {
            // A two-track dossier: mission plot on the first line, Snake's
            // actually earned character perspective beneath it. This is a
            // player-requested record, never a compulsory expository pause.
            const float LogY = Height - 202.f * UiScale;
            DrawRect(HudPanel, Margin, LogY,
                     Width - 2.f * Margin, 77.f * UiScale);
            DrawText(TEXT("MISSION"), HudAmber,
                     Margin + 12.f * UiScale, LogY + 6.f * UiScale,
                     nullptr, .59f * UiScale, false);
            DrawText(Intel, HudGreen, Margin + 12.f * UiScale,
                     LogY + 22.f * UiScale, nullptr, .72f * UiScale, false);
            DrawText(Mission->GetCharacterStoryText(), HudMuted,
                     Margin + 12.f * UiScale, LogY + 49.f * UiScale,
                     nullptr, .67f * UiScale, false);
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
            // Five-card visual strip, centered on the actual inventory
            // selection. Rendering reads gameplay state but NEVER equips or
            // consumes anything. Inputs are hold / scroll / release in the
            // equipment component. No separate Blueprint inventory state.
            const bool bWeapon = Equip->IsWeaponWindowOpen();
            const TArray<FMGEquipmentEntry>& Entries =
                bWeapon ? Equip->GetWeapons() : Equip->GetItems();
            const int32 Selection = bWeapon ?
                Equip->GetWeaponSelection() : Equip->GetItemSelection();
            const int32 Count = Entries.Num();
            if (Count > 0)
            {
                const float PanelW = FMath::Min(630.f * UiScale, Width - 2.f * Margin);
                const float PanelX = bWeapon ?
                    Width - Margin - PanelW : Margin;
                // Below mission objectives, but well above subtitles.
                const float PanelY = Height * .46f;
                const float PanelH = 158.f * UiScale;
                DrawRect(HudPanel, PanelX, PanelY, PanelW, PanelH);
                DrawRect(HudGreen, PanelX, PanelY, PanelW, 2.f * UiScale);
                DrawText(bWeapon ? TEXT("RB / G    WEAPONS") :
                                  TEXT("LB / TAB    EQUIPMENT"),
                         HudGreen, PanelX + 12.f * UiScale,
                         PanelY + 8.f * UiScale,
                         nullptr, .72f * UiScale, false);
                DrawText(TEXT("HOLD   SCROLL   RELEASE"),
                         HudMuted, PanelX + PanelW - 219.f * UiScale,
                         PanelY + 8.f * UiScale,
                         nullptr, .63f * UiScale, false);
                const float CardStep = (PanelW - 22.f * UiScale) / 5.f;
                for (int32 Offset = -2; Offset <= 2; ++Offset)
                {
                    const int32 Index = ((Selection + Offset) % Count + Count) % Count;
                    // Early game has only three weapon slots. Display no
                    // repeated cards at the two outer offsets; cursor still
                    // wraps across the actual array on input.
                    const int32 Priority[5] = {0, -1, 1, -2, 2};
                    bool bRepeated = false;
                    for (const int32 PriorOffset : Priority)
                    {
                        if (PriorOffset == Offset) break;
                        const int32 PriorIndex =
                            ((Selection + PriorOffset) % Count + Count) % Count;
                        if (PriorIndex == Index) { bRepeated = true; break; }
                    }
                    if (bRepeated) continue;
                    const FMGEquipmentEntry& Entry = Entries[Index];
                    const bool bAvailable = Entry.bOwned &&
                        (!Equip->IsConfiscated() || Entry.Id == FName(TEXT("NONE"))) &&
                        (!Entry.bConsumable || Entry.Quantity > 0);
                    const bool bFocused = Offset == 0;
                    const float CardX = PanelX + 11.f * UiScale +
                        static_cast<float>(Offset + 2) * CardStep;
                    const float CardY = PanelY + (bFocused ? 31.f : 39.f) * UiScale;
                    const float CardH = (bFocused ? 88.f : 73.f) * UiScale;
                    const float CardW = CardStep - 5.f * UiScale;
                    DrawRect(bFocused ?
                        FLinearColor(.11f,.30f,.23f,.98f) :
                        FLinearColor(.055f,.13f,.13f,.88f),
                        CardX, CardY, CardW, CardH);
                    DrawRect(bFocused ? HudAmber : HudMuted, CardX, CardY,
                             CardW, (bFocused ? 3.f : 1.f) * UiScale);
                    const FString Label = bAvailable ?
                        Entry.Label.Left(15) : TEXT("UNAVAILABLE");
                    const FString CountText = Entry.Id == FName(TEXT("HANDGUN")) ?
                        FString::Printf(TEXT("%d RD"), Entry.Quantity) :
                        Entry.bConsumable ?
                        FString::Printf(TEXT("x%d"), Entry.Quantity) :
                        Entry.Id == FName(TEXT("NONE")) ? TEXT("UNEQUIPPED") :
                        bAvailable ? TEXT("EQUIPPED") : TEXT("LOCKED");
                    const FLinearColor Tint = !bAvailable ? HudRed :
                        bFocused ? HudAmber : HudGreen;
                    DrawText(bFocused ? TEXT("[ SELECT ]") : TEXT("  /  "),
                        Tint, CardX + 6.f * UiScale, CardY + 6.f * UiScale,
                        nullptr, .63f * UiScale, false);
                    DrawText(Label, Tint, CardX + 6.f * UiScale,
                        CardY + 28.f * UiScale,
                        nullptr, (bFocused ? .76f : .67f) * UiScale, false);
                    DrawText(CountText, HudMuted, CardX + 6.f * UiScale,
                        CardY + 52.f * UiScale,
                        nullptr, .63f * UiScale, false);
                }
                DrawText(TEXT("STICK / DPAD / WHEEL / [ ]: SCROLL | RELEASE: EQUIP | Y / DOWN: USE"),
                    HudMuted, PanelX + 12.f * UiScale,
                    PanelY + PanelH - 22.f * UiScale,
                    nullptr, .62f * UiScale, false);
            }
        }

        const FString Feedback = Equip->GetEquipmentFeedback();
        if (!Feedback.IsEmpty())
            DrawText(Feedback, HudAmber, Width * 0.5f - 210.f * UiScale,
                     Height - 145.f * UiScale, nullptr, 0.9f * UiScale, false);
    }

    if (Player->IsCodecOpen())
    {
        // Two real identities, NOT two unrelated decorative pictures.
        // When Snake answers, his left portrait animates; otherwise the
        // selected resistance/command contact animates. Every expression
        // frame comes from the SAME source artwork as the browser's portraits.
        const int32 Channel = Player->GetTransceiverChannel();
        const FString ContactId = Channel == 0 ? TEXT("schneider") : TEXT("bigBoss");
        const FString ContactName = Channel == 0 ? TEXT("KYLE SCHNEIDER") : TEXT("BIG BOSS");
        const FString Frequency = Channel == 0 ? TEXT("120.79") : TEXT("120.85");
        const FString Speaker = Player->GetActiveCodecSpeakerId().ToString();
        const float Speech = Player->GetCodecTalkAlpha();
        const bool bSnakeTalking = Speaker == TEXT("snake") && Speech > .25f;
        const bool bContactTalking = !bSnakeTalking && Speech > .25f;
        const float WorldClock = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
        const bool bBlinkSnake = FMath::Fmod(WorldClock, 5.9f) < .12f;
        const bool bBlinkContact = FMath::Fmod(WorldClock + 2.4f, 5.9f) < .12f;
        const FString SnakeExpression = bSnakeTalking ? TEXT("Talk") :
            (bBlinkSnake ? TEXT("Blink") : TEXT("Neutral"));
        const FString ContactExpression = bContactTalking ? TEXT("Talk") :
            (bBlinkContact ? TEXT("Blink") : TEXT("Neutral"));

        const FString Transcript = Mission ?
            Mission->GetTransceiverDialogue(Channel) :
            TEXT("NO SIGNAL — LOCAL CONTACTS UNAVAILABLE");
        TArray<FString> Lines;
        Transcript.ParseIntoArrayLines(Lines, true);
        const float PanelWidth = FMath::Min(600.f * UiScale, Width - 2.f * Margin);
        const float PanelX = Width - Margin - PanelWidth;
        const float PanelY = Height * .30f;
        const int32 VisibleLines = FMath::Min(
            Player->GetVisibleRadioLineCount(Lines.Num()), 8);
        const float PanelHeight = (184.f + 22.f * VisibleLines) * UiScale;
        DrawRect(HudPanel, PanelX, PanelY, PanelWidth, PanelHeight);
        DrawText(FString::Printf(TEXT("TRANSCEIVER  /  %s  /  %s"),
                     *Frequency, *ContactName),
                 HudGreen, PanelX + 12.f * UiScale, PanelY + 8.f * UiScale,
                 nullptr, .80f * UiScale, false);

        const float PortraitY = PanelY + 29.f * UiScale;
        const float PortraitW = 88.f * UiScale;
        const float PortraitH = 105.f * UiScale;
        const float SnakeX = PanelX + 12.f * UiScale;
        const float ContactX = PanelX + PanelWidth - 100.f * UiScale;
        auto RenderPortrait = [&](const FString& Person, const FString& Expression,
                                  float AtX, const FString& Caption)
        {
            DrawRect(FLinearColor(.025f,.12f,.13f,.96f),
                     AtX-1.f, PortraitY-1.f, PortraitW+2.f, PortraitH+2.f);
            UTexture2D* Art = FindPortraitTexture(Person, Expression);
            if (!Art) Art = FindPortraitTexture(Person, TEXT("Neutral"));
            if (Art)
                DrawTexture(Art, AtX, PortraitY, PortraitW, PortraitH,
                            0.f,0.f,1.f,1.f,FLinearColor::White);
            else
                DrawText(TEXT("PORTRAIT ART\nIMPORT PENDING"), HudMuted,
                         AtX + 5.f, PortraitY + 35.f * UiScale, nullptr,
                         .65f * UiScale, false);
            DrawText(Caption, HudGreen, AtX, PortraitY + PortraitH + 5.f * UiScale,
                     nullptr, .64f * UiScale, false);
        };
        RenderPortrait(TEXT("snake"), SnakeExpression, SnakeX, TEXT("SNAKE"));
        RenderPortrait(ContactId, ContactExpression, ContactX, ContactName);
        DrawText(TEXT("FOXHOUND / 1995"), HudMuted,
            PanelX + 124.f * UiScale, PortraitY + 25.f * UiScale,
            nullptr, .74f * UiScale, false);
        DrawText(bSnakeTalking ? TEXT("TX  SNAKE") :
                 bContactTalking ? TEXT("RX  SPEAKING") : TEXT("RX  CONNECTED"),
            HudAmber, PanelX + 124.f * UiScale,
            PortraitY + 65.f * UiScale, nullptr, .69f * UiScale, false);
        for (int32 Index = 0; Index < VisibleLines; ++Index)
            DrawText(Lines[Index], FLinearColor::White,
                PanelX + 12.f * UiScale,
                PanelY + (154.f + 22.f * Index) * UiScale,
                nullptr, .72f * UiScale, false);
        DrawText(TEXT("Q / RS: TUNE    C / VIEW: CLOSE    MOVE: CONTINUE"),
            HudMuted, PanelX + 12.f * UiScale,
            PanelY + (166.f + 22.f * VisibleLines) * UiScale,
            nullptr, .65f * UiScale, false);
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

    // Two consistent instruction rails: the same face buttons are bound in
    // browser and native Unreal. D-pad down/Y keyboard uses item deliberately;
    // selecting an item does not itself consume finite rations.
    const FString CoverHelp = Player->IsInCover() ?
        TEXT("COVER: F / X KNOCK  |  AIM + SLIDE: PEEK  |  MOVE AWAY: RELEASE") :
        TEXT("E / A INTERACT  |  F / X MELEE  |  Z / Y PRONE  |  ESC / B SKIP");
    DrawText(CoverHelp,
             HudMuted, Margin, Height - 46.f * UiScale, nullptr, 0.66f * UiScale, false);
    DrawText(TEXT("Y / DPAD DOWN USE ITEM  |  TAB / LB ITEMS  |  G / RB WEAPONS  |  F9 RETRY CHECKPOINT"),
             HudMuted, Margin, Height - 26.f * UiScale,
             nullptr, 0.63f * UiScale, false);
}
