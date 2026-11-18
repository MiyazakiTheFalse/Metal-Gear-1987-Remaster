#pragma once
#include "mg_legacy_core.h"

// Ordered prefix validation is shared by the UE fallback and the checkpoint reader.
static inline uint32_t mg_fox_fallback_step(uint32_t flags, uint32_t action)
{
    static const struct { uint32_t action, grant; } steps[] = {
        {MG_FOX_CONTACT, MG_FOX_INTEL},
        {MG_FOX_CAPTURE, MG_FOX_CAPTURED | MG_FOX_CONFISCATED},
        {MG_FOX_BREACH_FIRST_WALL, MG_FOX_FIRST_WALL_BREACHED},
        {MG_FOX_RESCUE, MG_FOX_RESCUED},
        {MG_FOX_BREACH_EXIT_WALL, MG_FOX_EXIT_WALL_BREACHED},
        {MG_FOX_ESCAPE, MG_FOX_LEFT_PRISON},
        {MG_FOX_RECLAIM, MG_FOX_RECOVERED},
        {MG_FOX_DEFEAT, MG_FOX_SHOTMAKER_DOWN},
        {MG_FOX_DEPART, MG_FOX_COMPLETE},
    };
    uint32_t prefix = 0;
    for (const auto& step : steps)
    {
        if (prefix == flags && action == step.action) return prefix | step.grant;
        prefix |= step.grant;
    }
    return flags;
}

static inline bool mg_fox_valid_prefix(uint32_t flags)
{
    if (flags == 0 || flags == 1023u) return true;
    for (uint32_t action = 1; action <= MG_FOX_DEPART; ++action)
        if (mg_fox_fallback_step(flags, action) != flags) return true;
    return false;
}
