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
#include "mips.h"
#include "sceCtrl2.h"
#include "ark5.h"
#include "patching.h"

PSP_MODULE_INFO("PopsAnalog", PSP_MODULE_KERNEL, 1, 0);

sceCtrlPeekBufferNegative2_t _sceCtrlPeekBufferNegative2 = NULL;

#define PS1_L2_BIT 0x0100
#define PS1_R2_BIT 0x0200
#define PS1_R1_BIT 0x0800
#define PS1_L1_BIT 0x0400

// patchStoreButtons hook seems to run for all controller ports
// whereas patchPeekBufferNegative only runs on port 1
// so we keep track to ensure both hooks only run for port 1
static u8 *volatile g_pad = NULL;

// Runs right after POPS stores the PS1 btton data, after "sh $s1,0($s0)" has already executed
// POPS overwites rather than merge so we can't do button assignment in patchPeekBufferNegative
// The return value is ignored; the trampoline restores POPS's own $v0.
// Must be non static and exported
int patchStoreButtons(u8 *pad)
{
    // we run in kernel but the register will still be what it previously was
    // withut we get -2147483613 is 0x80000023, which is SCE_ERROR_PRIV_REQUIRED errors
    u32 k1 = pspSdkSetK1(0);

    if (_sceCtrlPeekBufferNegative2 == 0 || pad != g_pad) {
        pspSdkSetK1(k1);
        return 0;
    }

    // DS3 mapps R2 and L2 to the PSP's R and L
    // So we need to check both the PSP's buffer and the DS3's buffer
    // To allow PSP to still trigger PS1 R1/L1 but prevent the DS3 triggering them
    // with R2 and L2
    SceCtrlData2 psp, ds3;

    // TODO - Should i set Right analog stick values here from DS3? 
    _sceCtrlPeekBufferNegative2(SCE_CRTL_PORT_PSP, &psp, 1);
    int ds3Resp = _sceCtrlPeekBufferNegative2(SCE_CTRL_PORT_DS3, &ds3, 1);

    // Returns a error or number of read buffers.
    // Note if no DS3 datasource there will be no buffer so SCE_ERROR_NOT_SUPPORTED will be returned
    // so this handles no read data allong with errors.
    // https://github.com/uofw/uofw/blob/28306bb5bd4fcdcc0c1d469bf1f3bdb885dde3e9/src/kd/ctrl/ctrl.c#L2075
    if (ds3Resp < 1) {
        pspSdkSetK1(k1);
        return 0;
    }

    u8 pspRPressed = !(psp.buttons & SCE_CTRL_RTRIGGER);
    u8 pspLPressed = !(psp.buttons & SCE_CTRL_LTRIGGER);

    u8 ds3R1Pressed = !(ds3.buttons & SCE_CTRL_R1TRIGGER);
    u8 ds3R2Pressed = !(ds3.buttons & SCE_CTRL_RTRIGGER);

    u8 ds3L1Pressed = !(ds3.buttons & SCE_CTRL_L1TRIGGER);
    u8 ds3L2Pressed = !(ds3.buttons & SCE_CTRL_LTRIGGER);

    u16 buttons = *(volatile u16 *)pad;

    // mark all as released
    buttons |= (PS1_L2_BIT | PS1_R2_BIT | PS1_L1_BIT | PS1_R1_BIT);

    if (ds3L2Pressed)
        buttons &= ~PS1_L2_BIT;
    if (ds3R2Pressed)
        buttons &= ~PS1_R2_BIT;

    if ((pspRPressed && !ds3R2Pressed) || ds3R1Pressed)
        buttons &= ~PS1_R1_BIT;
    if ((pspLPressed && !ds3L2Pressed) || ds3L1Pressed)
        buttons &= ~PS1_L1_BIT;

    *(volatile u16 *)pad = buttons;

    pspSdkSetK1(k1);

    return 0;
}

// Replaces POPS call to PeekBufferNegative
// Must be non static and exported
int patchPeekBufferNegative(SceCtrlData *data, int numBufs, u8 *pad)
{
    int ret = sceCtrlPeekBufferNegative(data, numBufs);

    g_pad = pad;

    // Tell POPS this is an analog pad (ID 0x73, 6 data bytes).
    pad[0x22] = 0x73;
    pad[0x23] = 6;

    if (ret > 0) {
        pad[2] = data[ret - 1].Rx == 0 ? 0x80 : data[ret - 1].Rx;
        pad[3] = data[ret - 1].Ry == 0 ? 0x80 : data[ret - 1].Ry;

        pad[4] = data[ret - 1].Lx; 
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

static int on_start_module(SceModule *mod)
{
    if (!mod || strcmp(mod->modname, "pops") != 0)
        return 0;

    _sceCtrlPeekBufferNegative2 = (sceCtrlPeekBufferNegative2_t)sctrlHENFindFunction(
        "sceController_Service",
        "sceCtrl",
        NID_sceCtrlPeekBufferNegative2
    );

    if (_sceCtrlPeekBufferNegative2 == 0) {
        debugLog("sceCtrlPeekBufferNegative2 not found");
        return 0;
    }

    // Address of POPS's import stub
    // Use this to build the JAL instruction we modify later
    u32 stub = _sctrlHENFindImportInMod(mod, "sceCtrl", NID_sceCtrlPeekBufferNegative);
    if (stub == 0) {
        debugLog("sceCtrlPeekBufferNegative not found");
        return 0;
    }

    u32 *tramp = PATCHING_make_peekBuff_trampoline((void *)patchPeekBufferNegative);
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
            found = 1;

            // replace the real NID call instruction at this address with the reampoline
            MAKE_INSTRUCTION(addr, replaced);

            MIPS_flush_tramp((void *)addr, 4);

            // Now hook POPs storing the buttons
            // so we can safely inject L2/R2
            // the store is searched relative to the NID call address so we dont acidentally find false positives.
            u32 exit_addr  = 0;
            u32 store_addr = PATCHING_find_store_jump(mod, addr, &exit_addr);
            if (!store_addr)
                debugLog("store jump not found; hook 2 skipped");
            
            u32 *tStore = NULL;
            if (store_addr) {
                tStore = PATCHING_make_store_trampoline((void *)patchStoreButtons, exit_addr);

                if (tStore) {
                    MAKE_INSTRUCTION(store_addr, JUMP((u32)tStore));

                    MIPS_flush_tramp((void *)store_addr, 4);
                }
            }
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
    g_pad = NULL;
    return 0;
}