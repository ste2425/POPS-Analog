/*
 * main.c - PopsAnalog
 */
#include <pspsdk.h>
#include <pspkernel.h>
#include <psploadcore.h>
#include <pspsysmem_kernel.h>
#include <string.h>
#include <systemctrl.h>
#include <cfwmacros.h>
#include <pspctrl.h>
#include "debug/debug.h"

PSP_MODULE_INFO("PopsAnalog", PSP_MODULE_KERNEL, 1, 0);

#ifndef PSP_MEMORY_PARTITION_USER
#define PSP_MEMORY_PARTITION_USER 2
#endif

#define NID_sceCtrlPeekBufferNegative 0xC152080A

// addu $a2, $s0, $zero - The instruction to move s0 to a2
#define I_MOVE_A2_S0 0x02003021u

#define TRAMP_SIZE 16

/*
 * Kernel-side patch, reached via syscall. Must be non-static and exported.
 *
 * pad is POPS's $s0
 */
int patchPeekBufferNegative(SceCtrlData *data, int numBufs, u8 *pad)
{
    static int logged = 0;
    if (!logged) {
        logged = 1;

        debugLog("PATCH reached, pad=%08x", (u32)pad);
    }

    int ret = sceCtrlPeekBufferNegative(data, numBufs);

    // Tell POPS this is an analog pad (ID 0x73, 6 data bytes).
    pad[0x22] = 0x73;
    pad[0x23] = 6;

    if (ret > 0) {
        // PS1 Right stick X
        // PS1 Right Stick Y
        pad[3] = 0x80;
        // PS1 left Stick X. Get the latest sample, i think POPS only asks for 1 anyway
        pad[4] = data[ret - 1].Lx; 
        // PS1 Left Stick Y
        pad[5] = data[ret - 1].Ly;

        // Dont want POPS to run its own stick to buttons mapping
        // so center the PSP stick.
        for (int i = 0; i < ret; i++) {
            data[i].Lx = 0x80;
            data[i].Ly = 0x80;
        }
    }

    return ret;
}

static u32 *make_trampoline(void *kernel_func)
{
    int sc = sceKernelQuerySystemCall(kernel_func);
    if (sc < 0) {
        debugLog("QuerySystemCall failed %x (check exports.exp)", sc);
        return NULL;
    }

    // Create a bock of user memory to store the MIPS instructions
    SceUID uid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER,
                                               "PopsTramp", PSP_SMEM_Low,
                                               TRAMP_SIZE, NULL);
    if (uid < 0) {
        debugLog("Alloc failed %x", uid);
        return NULL;
    }

    u32 *t = (u32 *)sceKernelGetBlockHeadAddr(uid);
    memset(t, 0, TRAMP_SIZE);

    // $s0 contains POPS pad data. Copy it to $a2 so the hook recieves the pointer to it
    // as its 3rd argument
    t[0] = I_MOVE_A2_S0;

    // t[1] = jr $ra, t[2] = syscall. This is a normal syscall stub: the syscall
    // sits in the delay slot of the jr, so it runs and then control returns
    // straight to POPS (to the instruction after its original JAL) with the
    // patch's return value in $v0.
    // Syscalls are what allow user-mode code to run a kernel function; the
    // function must be exported in exports.exp.
    MAKE_SYSCALL_FUNCTION(&t[1], sc);

    // number of instructions have a delay slot. A JAL will first execute the delay slot then 
    // execute the method.
    t[3] = NOP; 

    sceKernelDcacheWritebackInvalidateRange(t, TRAMP_SIZE);
    sceKernelIcacheInvalidateRange(t, TRAMP_SIZE);

    debugLog("Trampoline at %08x, syscall %x", (u32)t, sc);
    return t;
}

static int on_start_module(SceModule *mod)
{
    if (!mod || strcmp(mod->modname, "pops") != 0)
        return 0;

    debugLog("POPS found");

    // Address of POPS's import stub
    // Use this to build the JAL instruction we modify later
    // sctrlHENFindImportInMod is a ARK5 and bove feature
    u32 stub = sctrlHENFindImportInMod(mod, "sceCtrl", NID_sceCtrlPeekBufferNegative);
    if (stub == 0 || (stub & 0x80000000)) {
        debugLog("import lookup failed: %08x", stub);
        return 0;
    }

    u32 *tramp = make_trampoline((void *)patchPeekBufferNegative);
    if (!tramp)
        return 0;

    // create the original instruction that would execute the real sceCtrlPeekBufferNegative
    const u32 original = JAL(stub);
    // create the JAL instruction to our trampoline to replace it with
    const u32 replaced = JAL((u32)tramp);

    int found = 0;
    // instructions are in 4 byte blocks. So iterate in multiples of four
    for (u32 addr = mod->text_addr; addr < mod->text_addr + mod->text_size; addr += 4) {
        if (VREAD32(addr) == original) {
            MAKE_INSTRUCTION(addr, replaced);

            // CPU has caches for data and instructions. As we have manipulated things we need to clear this cache
            sceKernelDcacheWritebackInvalidateRange((void *)addr, 4);
            sceKernelIcacheInvalidateRange((void *)addr, 4);

            debugLog("Hook installed at %08x", addr);
            found = 1;
            break;
        }
    }

    if (!found)
        debugLog("JAL not found");

    return 0;
}

int module_start(SceSize args, void *argp)
{
    debugInit();
    sctrlHENSetStartModuleHandler(on_start_module);
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    return 0;
}