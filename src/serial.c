#include "serial.h"

#define VDP_MODE_REG_3 0xB
#define VDP_IE2 0x08
#define INT_MASK_LEVEL_ENABLE_ALL 1

_voidCallback* readReadyCallback;

static void extIntCallback(void)
{
    readReadyCallback();
}

static void set_sctrl(u16 value)
{
    vs8* pb;
    pb = (s8*)PORT2_SCTRL;
    *pb = value;
}

static void set_ctrl(u16 value)
{
    vs8* pb;
    pb = (s8*)PORT2_CTRL;
    *pb = value;
}

u8 serial_sctrl(void)
{
    vs8* pb;
    pb = (s8*)PORT2_SCTRL;
    return *pb;
}

bool serial_readReady(void)
{
    vs8* pb = (s8*)PORT2_SCTRL;
    return *pb & SCTRL_RRDY;
}

u8 serial_read(void)
{
    vs8* pb = (s8*)PORT2_RX;
    return *pb;
}

void serial_setReadReadyCallback(_voidCallback* cb)
{
    readReadyCallback = cb;
}

void serial_init(u8 sctrlFlags)
{
    set_sctrl(sctrlFlags);
    set_ctrl(CTRL_PCS_OUT);
    if (sctrlFlags & SCTRL_RINT) {
        VDP_setReg(VDP_MODE_REG_3, VDP_IE2);
        SYS_setExtIntCallback(&extIntCallback);
        SYS_setInterruptMaskLevel(INT_MASK_LEVEL_ENABLE_ALL);
    }
}

void serial_send(u8 data)
{
    vu8* pb = (vu8*)PORT2_TX;
    *pb = data;
}

bool serial_sendReady(void)
{
    vu8* pb = (vu8*)PORT2_SCTRL;
    return !((*pb & SCTRL_TFUL) == SCTRL_TFUL);
}
