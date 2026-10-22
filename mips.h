// MIPS instruction builders and helpers

#pragma once

#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>
#include "debug/debug.h"

#define PSP_MEMORY_PARTITION_USER 2

#define REG_SP 29
#define REG_RA 31
#define REG_V0 2
#define REG_V1 3
#define REG_ZERO 0
#define REG_A0 4
#define REG_A2 6
#define REG_S0 16 
#define REG_S1 17

// addiu rt, rs, immediate
#define MIPS_ADDIU(rt, rs, imm) \
    (0x24000000u | \
     (((u32)(rs) & 0x1Fu) << 21) | \
     (((u32)(rt) & 0x1Fu) << 16) | \
     ((u32)(imm) & 0xFFFFu))

// sw rt, offset(base)
#define MIPS_SW(rt, base, offset) \
    (0xAC000000u | \
     (((u32)(base) & 0x1Fu) << 21) | \
     (((u32)(rt) & 0x1Fu) << 16) | \
     ((u32)(offset) & 0xFFFFu))

// lw rt, offset(base)
#define MIPS_LW(rt, base, offset) \
    (0x8C000000u | \
     (((u32)(base) & 0x1Fu) << 21) | \
     (((u32)(rt) & 0x1Fu) << 16) | \
     ((u32)(offset) & 0xFFFFu))

// addu rd, rs, rt
#define MIPS_ADDU(rd, rs, rt) \
    (0x00000021u | \
     (((u32)(rs) & 0x1Fu) << 21) | \
     (((u32)(rt) & 0x1Fu) << 16) | \
     (((u32)(rd) & 0x1Fu) << 11))

// sh rt, offset(base)
#define MIPS_SH(rt, base, offset) \
    (0xA4000000u | \
     (((u32)(base) & 0x1Fu) << 21) | \
     (((u32)(rt)   & 0x1Fu) << 16) | \
     ((u32)(offset) & 0xFFFFu))

u32 *MIPS_alloc_tramp(const char *name, int size);
void MIPS_flush_tramp(u32 *t, int size);