#pragma once

/*
    Displays a total of 5 messages in a ringe buffer.
    Messages will persist untill pushed out the buffer.

    EG:
    debugLog("Im a little teapot");
    debugLog("Message #%u (tick 0x%x)", tick / 120, tick);
*/
extern void dbg_log(const char *fmt, ...);