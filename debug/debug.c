#include "debug.h"

// keep this here so we dont get conlfict sceCtrl types
#include <kubridge.h>

int _debugEnabled = 1;

void debugInit() 
{
    if (_debugEnabled)
        kuKernelLoadModule("ms0:/SEPLUGINS/debug_plugin_log.prx", 0, NULL);
}

uint8_t debugEnabed()
{
    return _debugEnabled;
}