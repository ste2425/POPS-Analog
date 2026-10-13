	.set noreorder

#include "pspstub.s"

	STUB_START "debug_plugin_log",0x40090000,0x00010005
	STUB_FUNC  0xD82140BF,dbg_log
	STUB_END
