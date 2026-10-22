#include "patching.h"

#define OP_J               2            // J instruction operator
#define SIG_STORE_DELAY MIPS_SH(REG_S1, REG_S0, REG_ZERO)  // the delay slot instruction to look for, sh $s1,0($s0) 0xA6110000u
#define STORE_SEARCH_BYTES 0x200        // how far after the sceCtrlPeekBufferNegative JAL to look

// create the instruction that moves the PS1 controller data into argument 3 for our sceCtrlPeekBufferNegative hook
#define I_MOVE_A2_S0 MIPS_ADDU(REG_A2, REG_S0, REG_ZERO)

#define TRAMP_PEEK_SIZE 16
#define TRAMP_STORE_SIZE  64

#define I_ADDIU_SP_M32 MIPS_ADDIU(REG_SP, REG_SP, -32)
#define I_ADDIU_SP_P32 MIPS_ADDIU(REG_SP, REG_SP,  32)
#define I_SW_RA_28 MIPS_SW(REG_RA, REG_SP, 28)
#define I_LW_RA_28 MIPS_LW(REG_RA, REG_SP, 28)
#define I_SW_V0_24 MIPS_SW(REG_V0, REG_SP, 24)
#define I_LW_V0_24 MIPS_LW(REG_V0, REG_SP, 24)
#define I_SW_V1_20 MIPS_SW(REG_V1, REG_SP, 20)
#define I_LW_V1_20 MIPS_LW(REG_V1, REG_SP, 20)
#define I_MOVE_A0_S0 MIPS_ADDU(REG_A0, REG_S0, REG_ZERO)

// Finds the point to inject our trampoline for setting PS1 button data
// Starts searching from the point we hook the use of sceCtrlPeekBufferNegative (call_addr)
// POPS jumps to an exit addres with a specific "sh $s1,0($s0)" in its delay slot.
// return the address to patch but also the jump address so we can jump once the trampoline patch is done
u32 PATCHING_find_store_jump(SceModule *mod, u32 call_addr, u32 *exit_out)
{
    u32 text_end = mod->text_addr + mod->text_size;
    u32 limit    = call_addr + STORE_SEARCH_BYTES;

    if (limit > text_end - 4)
        limit = text_end - 4;

    for (u32 a = call_addr + 4; a < limit; a += 4) {
        u32 inst = VREAD32(a);

        // verify the instruction is a J and its delay slot matches the instruction we expect
        if ((inst >> 26) == OP_J && VREAD32(a + 4) == SIG_STORE_DELAY) {
            u32 exit_addr = (a & 0xF0000000) | JUMP_TARGET(inst);

            if (exit_addr < mod->text_addr || exit_addr >= text_end)
                return 0;

            *exit_out = exit_addr;

            return a;
        }
    }

    return 0;
}

u32 *PATCHING_make_peekBuff_trampoline(void *kernel_func)
{
    int sc = sceKernelQuerySystemCall(kernel_func);
    if (sc < 0) {
        debugLog("peefBuf - QuerySystemCall failed %x (check exports.exp)", sc);
        return NULL;
    }

    u32 *t = MIPS_alloc_tramp("PopsTrampPeek", TRAMP_PEEK_SIZE);
    if (!t)
        return NULL;

    // $s0 contains POPS pad data. Copy it to $a2 so the hook recieves the pointer to it
    // as its 3rd argument
    t[0] = I_MOVE_A2_S0;

    MAKE_SYSCALL_FUNCTION(&t[1], sc);

    // JAL's delay slot
    t[3] = NOP; 

    MIPS_flush_tramp(t, TRAMP_PEEK_SIZE);

    return t;
}

// Button store trmapoline for manipulating PS1 Pad buttons.
// Replaces a `j <exit>` instruction. Once the patch function is complete it must
// jump to the same exit to continue as normal.
// Aparently $v0 $v1 and $ra get clobbered (i dunno i just work here) from the sys call and JAL
// so they are persisted in stack and restored after
u32 *PATCHING_make_store_trampoline(void *kernel_func, u32 exit_addr)
{
    int sc = sceKernelQuerySystemCall(kernel_func);
    if (sc < 0) {
        debugLog("QuerySystemCall failed %x (check exports)", sc);
        return NULL;
    }

    u32 *t = MIPS_alloc_tramp("PopsTrampStore", TRAMP_STORE_SIZE);
    if (!t)
        return NULL;

    // Create an ammount of stack space to work with
    t[0]  = I_ADDIU_SP_M32;
    // Store the current $ra return address in stack offset 28
    t[1]  = I_SW_RA_28;
    // Store $v0 return value in stack offset 24
    t[2]  = I_SW_V0_24;
    // Store $v1 return value in stack offset 20
    t[3]  = I_SW_V1_20;
    // Create the JAL instruction for our patch function
    // The actual syscall is down further down
    t[4]  = JAL((u32)&t[11]);
    // Move $s0 (PS1 pad data) to $a0 (first argument of patch func)
    t[5]  = I_MOVE_A0_S0;
    // Set back the $v0 from stack overwriting the patch return value
    t[6]  = I_LW_V0_24;
    // Set back the $v1 from stack
    t[7]  = I_LW_V1_20;
    // Set back $ra return address from stack
    t[8]  = I_LW_RA_28;
    // Restore the original jump instruction we replaced
    t[9]  = JUMP(exit_addr);
    // Restore the stack pointer in the jumps delay slot
    t[10] = I_ADDIU_SP_P32;
    
    // Creates two instructions. First is the jump to the return address
    // second is the delay slot which executes the syscall
    // t[11] = jr $ra
    //t[12] = syscall
    MAKE_SYSCALL_FUNCTION(&t[11], sc);

    MIPS_flush_tramp(t, TRAMP_STORE_SIZE);

    return t;
}