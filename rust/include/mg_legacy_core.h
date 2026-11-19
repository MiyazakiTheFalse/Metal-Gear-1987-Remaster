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

/* Gray Fox first mission-slice events. Use stable 32-bit bitfield flags. */
#define MG_FOX_CAPTURED 1u
#define MG_FOX_CONFISCATED 2u
#define MG_FOX_FIRST_WALL_BREACHED 4u
#define MG_FOX_RESCUED 8u
#define MG_FOX_EXIT_WALL_BREACHED 16u
#define MG_FOX_LEFT_PRISON 32u
#define MG_FOX_CAPTURE 1u
#define MG_FOX_BREACH_FIRST_WALL 2u
#define MG_FOX_RESCUE 3u
#define MG_FOX_BREACH_EXIT_WALL 4u
#define MG_FOX_ESCAPE 5u
#define MG_FOX_INTEL 64u
#define MG_FOX_RECOVERED 128u
#define MG_FOX_SHOTMAKER_DOWN 256u
#define MG_FOX_COMPLETE 512u
#define MG_FOX_CONTACT 6u
#define MG_FOX_RECLAIM 7u
#define MG_FOX_DEFEAT 8u
#define MG_FOX_DEPART 9u
uint32_t mg_gray_fox_transition(uint32_t flags, uint32_t action, uint32_t* next_flags);

/* Return optional pacing guidance; NO progression is gated on a timer. */
typedef struct MgGrayFoxPacing {
    uint32_t phase;
    float target_intensity;
    uint32_t hint_level;
    uint32_t should_suppress_new_pressure;
} MgGrayFoxPacing;
uint32_t mg_gray_fox_pacing(uint32_t flags, float seconds_without_progress,
                            uint32_t under_alert, MgGrayFoxPacing* output);

uint32_t mg_core_abi_version(void);
uint32_t mg_step_guard(const MgGuardFrame* frame, MgGuardResult* output);
#ifdef __cplusplus
}
#endif
#endif
