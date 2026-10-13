TARGET = pops_analog
OBJS   = main.o exports.o debug/debug_plugin_log.o debug/debug.o

PSP_FW_VERSION = 660

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

INCDIR  = $(SYSCTRL_DIR)
LIBDIR  = $(SYSCTRL_DIR)

CFLAGS   = -Os -G0 -Wall -std=gnu99 -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  = $(CFLAGS)

LDFLAGS  = -nostartfiles
LIBS     = -lpspsystemctrl_kernel -lpspctrl -lpspkubridge

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak