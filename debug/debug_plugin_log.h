#pragma once

/*
    Displays a total of 5 messages in a ringe buffer.
    Messages will persist untill pushed out the buffer.

    EG:
    dbg_log("Hello from a PSP plugin!");
    dbg_log("Message #%u (tick 0x%x)", tick / 120, tick);
*/
extern void dbg_log(const char *fmt, ...);