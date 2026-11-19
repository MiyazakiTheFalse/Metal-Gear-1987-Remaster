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
/* Shoot Gunner/Shotmaker encounter. Historical confiscation remains immutable. */
#define MG_FOX_SHOTMAKER_ENCOUNTERED 64u
#define MG_FOX_GEAR_RECOVERED 128u
#define MG_FOX_SHOTMAKER_DEFEATED 256u
#define MG_FOX_SLICE_COMPLETED 512u
#define MG_FOX_TRANSMITTER_PLANTED 1024u
#define MG_FOX_TRANSMITTER_REMOVED 2048u
#define MG_FOX_CARD3_RECOVERED 4096u
#define MG_FOX_DETENTION_INTELLIGENCE 8192u
#define MG_FOX_CAPTURE 1u
#define MG_FOX_BREACH_FIRST_WALL 2u
#define MG_FOX_RESCUE 3u
#define MG_FOX_BREACH_EXIT_WALL 4u
#define MG_FOX_ESCAPE 5u
#define MG_FOX_ENTER_SHOTMAKER 6u
#define MG_FOX_RECOVER_GEAR 7u
#define MG_FOX_DEFEAT_SHOTMAKER 8u
#define MG_FOX_COMPLETE_SLICE 9u
#define MG_FOX_COLLECT_CARD3 10u
#define MG_FOX_REMOVE_TRANSMITTER 11u
#define MG_FOX_LEARN_DETENTION_ROUTE 12u
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

/* Backward-compatible ABI-v1 addition. TX-55 logic is game-engine neutral.
   On a valid placement the output is written exactly once; stale or
   incorrect animation notifies leave input, output and ammo unchanged. */
#define MG_TX55_LEFT 0u
#define MG_TX55_RIGHT 1u
#define MG_TX55_INVALID 0u
#define MG_TX55_ACCEPTED 1u
#define MG_TX55_WRONG_LEG 2u
#define MG_TX55_NO_CHARGE 3u
#define MG_TX55_STALE_EVENT 4u
#define MG_TX55_ALREADY_DESTROYED 5u
typedef struct MgTx55State {
    uint32_t accepted_placements;
    uint32_t charges_available;
    uint32_t destroyed;
} MgTx55State;
uint32_t mg_tx55_place_charge(const MgTx55State* input, uint32_t leg,
                             uint32_t expected_progress, MgTx55State* output);

uint32_t mg_core_abi_version(void);
uint32_t mg_step_guard(const MgGuardFrame* frame, MgGuardResult* output);
#ifdef __cplusplus
}
#endif
#endif
