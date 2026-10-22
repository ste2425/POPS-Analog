#pragma once

#include <stdint.h>

#include "debug_plugin_log.h"

void debugInit();

uint8_t debugEnabed();

/*
    Displays a total of 5 messages in a ringe buffer.
    Messages will persist untill pushed out the buffer.

    EG:
    debugLog("Im a little teapot");
    debugLog("Message #%u (tick 0x%x)", tick / 120, tick);
*/
#define debugLog(...) \
    do { if (debugEnabed()) dbg_log(__VA_ARGS__); } while (0)