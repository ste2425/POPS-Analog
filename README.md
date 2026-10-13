# POPS-Analog

A PSP plugin for POPS that adds *real* analog control, not just mapping the PSP's stick to the PS1's DPAD.

---

## Installation

* Download `pops_analog.prx` from the [Latest Release](https://github.com/ste2425/POPS-Analog/releases/latest) and place it in `/SEPLUGINS`
* Update the `PLUGINS.TXT` file and add the following line to the bottom
  * `pops, ms0:/seplugins/pops_analog.prx, 1`
* Save then safely unmount the PSP.

## Requirements

This plugin requires `ARK-5` or higher. Behavior is unknown if running on a different CFW.

It *should* support all POPS versions. However i have only tested on a 1K and 2K.

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