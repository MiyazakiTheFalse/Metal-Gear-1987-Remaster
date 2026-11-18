#include "MGPrisonAudio.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"
#include "Sound/SoundAttenuation.h"

void MGPrisonAudio::Play(UObject* Context, const FVector& Position, EMGPrisonCue Cue)
{
    struct FVoice { float Seconds, Resonance, Grit, Tail; };
    static constexpr FVoice Voices[] = {
        {.17f, 138, .16f, 25}, {.27f, 96, .48f, 17}, {.62f, 63, .67f, 8},
        {.28f, 730, .26f, 24}, {.38f, 620, .61f, 16}, {.55f, 79, .9f, 12}
    };
    const auto& Voice = Voices[static_cast<int32>(Cue)];
    constexpr int32 Rate = 22050;
    const int32 Frames = FMath::CeilToInt(Voice.Seconds * Rate);
    TArray<int16> Samples;
    Samples.SetNumUninitialized(Frames);
    FRandomStream Random(73 + static_cast<int32>(Cue));
    float LowNoise = 0;
    for (int32 I = 0; I < Frames; ++I)
    {
        const float T = float(I) / Rate;
        const float Noise = Random.FRandRange(-1.f, 1.f);
        LowNoise += .24f * (Noise - LowNoise);
        const float Resonance = FMath::Sin(2.f * PI * Voice.Resonance * T)
            + .32f * FMath::Sin(2.f * PI * Voice.Resonance * 1.53f * T);
        const float Body = (.48f * Resonance + Voice.Grit * LowNoise) * FMath::Exp(-T * Voice.Tail);
        const float Scatter = T > .07f ? Voice.Grit * Noise * FMath::Exp(-12.f * (T - .07f)) * .36f : 0;
        const float Edge = FMath::Min(1.f, FMath::Min(T, Voice.Seconds - T) * 400.f);
        Samples[I] = int16(FMath::Clamp((Body + Scatter) * Edge, -1.f, 1.f) * 23000.f);
    }
    auto* Wave = NewObject<USoundWaveProcedural>(Context);
    Wave->SetSampleRate(Rate);
    Wave->NumChannels = 1;
    Wave->Duration = Voice.Seconds;
    Wave->bLooping = false;
    auto* Attenuation = NewObject<USoundAttenuation>(Wave);
    Attenuation->Attenuation.bSpatialize = true;
    Attenuation->Attenuation.AttenuationShapeExtents.X = 180.f;
    Attenuation->Attenuation.FalloffDistance = 1600.f;
    Wave->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Frames * sizeof(int16));
    if (auto* Audio = UGameplayStatics::SpawnSoundAtLocation(Context, Wave, Position, FRotator::ZeroRotator, 1.f, 1.f, 0.f, Attenuation))
        Audio->StopDelayed(Voice.Seconds + .05f);
}
