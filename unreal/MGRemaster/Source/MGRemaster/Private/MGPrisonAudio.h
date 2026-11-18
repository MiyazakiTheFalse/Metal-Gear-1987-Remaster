#pragma once
#include "CoreMinimal.h"

enum class EMGPrisonCue { Knock, Fracture, Collapse, Latch, Rack, Report };

// Original mono foley. All sources are finite and positioned at the interaction.
namespace MGPrisonAudio
{
    void Play(UObject* Context, const FVector& Position, EMGPrisonCue Cue);
}
