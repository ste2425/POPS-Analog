#pragma once

#include <pspkernel.h>
#include <cfwmacros.h>
#include "mips.h"

u32 PATCHING_find_store_jump(SceModule *mod, u32 call_addr, u32 *exit_out);
u32 *PATCHING_make_peekBuff_trampoline(void *kernel_func);
u32 *PATCHING_make_store_trampoline(void *kernel_func, u32 exit_addr);