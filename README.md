# POPS-Analog

A PSP plugin for POPS that adds *real* analog control, not just mapping the PSP's stick to the PS1's DPAD.

---

## Installation

* Download `pops_analog.prx` from the [Latest Release](https://github.com/ste2425/POPS-Analog/releases/latest) and place it in `/SEPLUGINS`
* Update the `PLUGINS.TXT` file and add the following line to the bottom
  * `pops, ms0:/seplugins/pops_analog.prx, 1`
* Save then safely unmount the PSP.

## Requirements

This plugin does *not* have a specific CFW requirement. However for 6.61 (not sure if 6.60 had this too) different motherboard revisions had their own version of POPS. I have coalted a table of CFW, OFW and hardware variant's i have been able to test. 

If you run this please contact me with the details from the `Known Working Environments` table and i will update it.

## Known Working Environments

ℹ️ - *Over the next few days i will try various combinations of CFW and OFW with the two PSP's i have and update the table below*

| CFW | OFW | Motherboard Version |
| -- | -- | -- |
| LME-2.3 | 6.61 | TA-081v2 (01g) |
| ARK-4 | 6.61 | TA-081v2 (01g) |
| ARK-5 | 6.61 | TA-081v2 (01g) |
| ARK-5 | 6.61 | TA-088v3 (02g) |

## PS1 Controller Configuration

It is still possible to set custom controller configurations in POPS. However the PSP's analog stick will no longer be visible to POPS and is reserved solely for the PS1's analog stick.

## How It Works

POPS makes a call to `sceCtrlPeekBufferNegative` in a single place to get its data.

It finds, by NID, the specific MIPS `JAL` instruction that executes this and replace it with our own trampoline. This means the hook is reliable across multiple POPS versions.

The PS1's controller pad is stored in register `$s0` so the trampoline copies it to `$a2` meaning the patched `sceCtrlPeekBufferNegative` receives a pointer to it as its third argument.

# Acknowledgements

* PSP Homebrew Discord community. Without their help i would never have been able to achieve anything on the PSP.
* ARK-5 developers. Without Ark5 non of this would work
* Special thanks to GrayJack. Spent allot of time in the Homebrew Discord explaining techniques for hooking the `sceCtrlPeekBufferNegative` call and showed me a way far better than i was attempting.
