#include "MGPrototypeHUD.h"

#include "MGPlayerCharacter.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"

void AMGPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    const AMGPlayerCharacter* Player = Cast<AMGPlayerCharacter>(GetOwningPawn());
    if (!Player) return;
    DrawText(TEXT("PROTOTYPE  |  WASD: Move   Mouse: Look   RMB: Aim   LMB: Stun   Ctrl: Crouch   Shift: Sprint   Q: Shoulder   F6: Graphics"),
        FLinearColor::White, 18.f, 20.f, nullptr, 0.9f, false);
    DrawText(Player->IsAiming() ? TEXT("AIMING  -  NONLETHAL STUN") :
             (Player->IsSprinting() ? TEXT("SPRINTING") : TEXT("INFILTRATION")),
        FLinearColor::White, 18.f, 48.f, nullptr, 1.0f, false);
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
