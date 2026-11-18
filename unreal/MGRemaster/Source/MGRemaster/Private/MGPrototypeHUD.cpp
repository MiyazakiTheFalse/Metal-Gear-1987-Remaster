#include "MGPrototypeHUD.h"

#include "MGGrayFoxMission.h"
#include "MGPlayerCharacter.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AMGPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(GetOwningPawn());
    if (!Player) return;
    DrawText(TEXT("GRAY FOX RESCUE — PROVISIONAL GREYBOX  |  WASD: Move   Mouse: Camera   E: Interact   Ctrl: Crouch   Shift: Sprint   F9: Restart"),
        FLinearColor::White, 18.f, 18.f, nullptr, 0.78f, false);
    DrawText(TEXT("RMB: Aim   Q: Swap shoulder   J / D-pad Up: Intel log   F6: Graphics   |  Captivity: unarmed"),
        FLinearColor::White, 18.f, 38.f, nullptr, 0.80f, false);

    const AMGGrayFoxMission* Mission = Cast<AMGGrayFoxMission>(
        UGameplayStatics::GetActorOfClass(this, AMGGrayFoxMission::StaticClass()));
    if (Mission)
    {
        DrawText(FString::Printf(TEXT("PHASE: %s"), *Mission->GetPhaseText()),
                 FLinearColor(0.7f, 0.9f, 1.f, 1.f), 20.f, 74.f, nullptr, 1.f, false);
        DrawText(Mission->GetObjectiveText(), FLinearColor::White,
                 20.f, 103.f, nullptr, 1.0f, false);
        const FString Hint = Mission->GetPacingHint();
        if (!Hint.IsEmpty())
            DrawText(Hint, FLinearColor(0.88f, 0.85f, 0.6f, 1.f),
                     20.f, 131.f, nullptr, 0.93f, false);
        if (Player->IsMissionLogVisible())
        {
            const FString Intel = Mission->GetMissionLogText();
            if (!Intel.IsEmpty())
                DrawText(Intel, FLinearColor(0.65f, 0.92f, 1.f, 1.f),
                    20.f, Canvas->ClipY - 143.f, nullptr, 0.92f, false);
        }
        const FString Cue = Mission->GetRecentDialogue();
        if (!Cue.IsEmpty())
            DrawText(Cue, FLinearColor::White, 24.f,
                     Canvas->ClipY - 90.f, nullptr, 1.15f, false);
    }

    if (Player->IsAiming())
    {
        const float X = Canvas->ClipX * 0.5f;
        const float Y = Canvas->ClipY * 0.5f;
        DrawLine(X - 10.f, Y, X - 3.f, Y, FLinearColor::White, 1.f);
        DrawLine(X + 3.f, Y, X + 10.f, Y, FLinearColor::White, 1.f);
        DrawLine(X, Y - 10.f, X, Y - 3.f, FLinearColor::White, 1.f);
        DrawLine(X, Y + 3.f, X, Y + 10.f, FLinearColor::White, 1.f);
    }
}
