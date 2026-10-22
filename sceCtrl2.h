// Definitions for various sceCtrl 2 variants taken from 
// https://github.com/uofw/uofw/blob/28306bb5bd4fcdcc0c1d469bf1f3bdb885dde3e9/include/ctrl.h#L1

#pragma once

// NID of sceCtrlPeekBufferNegative which POPS uses for controller data
// Allows us to find out injection point
#define NID_sceCtrlPeekBufferNegative 0xC152080A
#define NID_sceCtrlPeekBufferNegative2 0x239A6BA7

typedef struct {
    /** 
     * The time stamp of the time during which sampling was performed. Time unit is microseconds. 
     * Can be used to get the time period of a button pressing event.
     */ 
    u32 timeStamp;
    /** The currently pressed button. Bitwise OR'ed values of ::SceCtrlPadButtons. */
    u32 buttons;
    /** Analog Stick X-axis offset (0 - 0xFF). Left = 0, Right = 0xFF. */
    u8 aX;
    /** Analog Stick Y-axis offset (0 - 0xFF). Up = 0, Down = 0xFF. */
    u8 aY;
    /** DS3 right analog x-axis. Filled with 0 if input source doesn't allow second analog input. */
    u8 rX;
    /** DS3 right analog y-axis. Filled with 0 if input source doesn't allow second analog input. */
    u8 rY;
    /** Reserved. */
    u8 rsrv[4];
    /** D-pad pressure sensitivity.
    * Byte 1: D-Pad right.
    * Byte 3: D-Pad left.    
    */
    s32 DPadSenseA;
    /** D-pad pressure sensitivity.
    * Byte 1: D-Pad up.
    * Byte 3: D-Pad down.    
    */
    s32 DPadSenseB;
    /** Gamepad pressure sensitivity.
    * Byte 1: Triangle.
    * Byte 3: Circle.    
    */
    s32 GPadSenseA;
    /** Gamepad pressure sensitivity.
    * Byte 1: Cross.
    * Byte 3: Square.    
    */
    s32 GPadSenseB;
    /** Axis pressure sensitivity.
    * Byte 1: L1.
    * Byte 3: R1.    
    */
    s32 AxisSenseA;
    /** Axis pressure sensitivity.
    * Byte 1: L2.
    * Byte 3: R2.    
    */
    s32 AxisSenseB;
    /** DS3 sixaxis. This is the return value for tilting the x-axis. */
    s32 TiltA;
    /** DS3 sixaxis. This is the return value for tilting the y-axis. */
    s32 TiltB;
} SceCtrlData2;

/**
 * Enumeration for the digital controller buttons in positive logic.
 *
 * @note SCE_CTRL_INTERCEPTED, SCE_CTRL_WLAN_UP, SCE_CTRL_REMOTE, SCE_CTRL_VOLUP, SCE_CTRL_VOLDOWN, 
 *       SCE_CTRL_SCREEN, SCE_CTRL_NOTE, SCE_CTRL_DISC, SCE_CTRL_MS can only be read in kernel mode.
 */
enum SceCtrlPadButtons {
    /** Select button. */
    SCE_CTRL_SELECT         = 0x00000001,
    /** DS3 L3 button. */
    SCE_CTRL_L3             = 0x00000002,
    /** DS3 R3 button. */
    SCE_CTRL_R3             = 0x00000004,
    /** Start button. */
    SCE_CTRL_START          = 0x00000008,
    /** Up D-Pad button. */
    SCE_CTRL_UP             = 0x00000010,
    /** Right D-Pad button. */
    SCE_CTRL_RIGHT          = 0x00000020,
    /** Down D-Pad button. */
    SCE_CTRL_DOWN           = 0x00000040,
    /** Left D-Pad button. */
    SCE_CTRL_LEFT           = 0x00000080,
    /** Left trigger. This accounts for the DS3 L2 trigger as well. */
    SCE_CTRL_LTRIGGER       = 0x00000100,
    /** Right trigger. This accounts for the DS3 R2 trigger as well. */
    SCE_CTRL_RTRIGGER       = 0x00000200,
    /** DS3 L1 trigger. */
    SCE_CTRL_L1TRIGGER      = 0x00000400,
    /** DS3 R1 trigger. */
    SCE_CTRL_R1TRIGGER      = 0x00000800,
    /** Triangle button. */
    SCE_CTRL_TRIANGLE       = 0x00001000,
    /** Circle button. */
    SCE_CTRL_CIRCLE         = 0x00002000,
    /** Cross button. */
    SCE_CTRL_CROSS          = 0x00004000,
    /** Square button. */
    SCE_CTRL_SQUARE         = 0x00008000,
    /** 
     * If this bit is set, then controller input is being intercepted by the 
     * system software or another application.  For example, this is the case 
     * when the PSP's HOME menu is being shown. 
     */
    SCE_CTRL_INTERCEPTED    = 0x00010000,
    /** Hold button. */
    SCE_CTRL_HOLD           = 0x00020000,
    /** W-LAN switch up. */
    SCE_CTRL_WLAN_UP        = 0x00040000,
    /** Remote hold position. */
    SCE_CTRL_REMOTE         = 0x00080000,
    /** Volume up button. */
    SCE_CTRL_VOLUP          = 0x00100000,
    /** Volume down button. */
    SCE_CTRL_VOLDOWN        = 0x00200000,
    /** Screen button. */
    SCE_CTRL_SCREEN         = 0x00400000,
    /** Music Note button. */
    SCE_CTRL_NOTE           = 0x00800000,   	
    /** Disc present. */
    SCE_CTRL_DISC           = 0x01000000,
    /** Memory stick present. */
    SCE_CTRL_MS             = 0x02000000,
    SCE_CTRL_UNK_10000000   = 0x10000000,
    SCE_CTRL_UNK_20000000   = 0x20000000,
};

/** Specifies the type of input data to be obtained. */
enum SceCtrlPort {
    /* Input is only obtained from the PSP. */
    SCE_CRTL_PORT_PSP = 0,
    /* Input is obtained from the PSP and a connected DualShock3 controller. */
    SCE_CTRL_PORT_DS3 = 1,
    /* Input is obtained from the PSP and an unknown connected external device. */
    SCE_CTRL_PORT_UNKNOWN_2 = 2
};

// Function pointer type matching sceCtrlPeekBufferNegative2 so we can manually import it
typedef int (*sceCtrlPeekBufferNegative2_t)(int port, SceCtrlData2 *pad_data, int count);