// MIPS instruction builders and helpers

#include "mips.h"

// Allocate user memory for trampoline as POPS runs in user
u32 *MIPS_alloc_tramp(const char *name, int size)
{
    SceUID uid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, name,
                                               PSP_SMEM_Low, size, NULL);
    if (uid < 0) {
        debugLog("Alloc failed %x", uid);
        return NULL;
    }
    u32 *t = (u32 *)sceKernelGetBlockHeadAddr(uid);
    memset(t, 0, size);
    return t;
}

// Flush MIPS cache to make trampolines visible
void MIPS_flush_tramp(u32 *t, int size)
{
    sceKernelDcacheWritebackInvalidateRange(t, size);
    sceKernelIcacheInvalidateRange(t, size);
}
