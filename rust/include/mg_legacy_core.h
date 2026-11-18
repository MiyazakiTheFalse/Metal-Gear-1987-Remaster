#ifndef MG_LEGACY_CORE_H
#define MG_LEGACY_CORE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MgVec2 { float x; float y; } MgVec2;
typedef struct MgGuardFrame {
    MgVec2 guard_pos;
    MgVec2 guard_forward;
    MgVec2 player_pos;
    float vision_range_m;
    float vision_half_angle_deg;
    float hearing_range_m;
    float visibility;
    float noise;
    float suspicion;
    float delta_seconds;
    uint32_t line_of_sight;
} MgGuardFrame;
typedef struct MgGuardResult {
    float suspicion;
    uint32_t state; /* 0 patrol, 1 investigate, 2 search, 3 alert */
    uint32_t sees_player;
    uint32_t hears_player;
    float visual_threat;
    float sound_threat;
} MgGuardResult;

uint32_t mg_core_abi_version(void);
uint32_t mg_step_guard(const MgGuardFrame* frame, MgGuardResult* output);
#ifdef __cplusplus
}
#endif
#endif
