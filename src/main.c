#include "buffer.h"
#include "serial.h"
#include <genesis.h>
#include <stdbool.h>
#include <vdp.h>

const bool USE_RECV_INT = TRUE;

const u16 BUFFER_MIN_Y = 6;
const u16 BUFFER_MAX_X = 39;
const u16 BUFFER_MAX_LINES = 6;

typedef struct Cursor Cursor;

struct Cursor {
    u16 x;
    u16 y;
};

static u8 ui_dirty = FALSE;

static u16 baudRate(u8 sctrl)
{
    switch (sctrl & 0xC0) {
    case SCTRL_300_BPS:
        return 300;
    case SCTRL_1200_BPS:
        return 1200;
    case SCTRL_2400_BPS:
        return 2400;
    default:
        return 4800;
    }
}

static void printBaudRate(void)
{
    char baudRateText[9];
    sprintf(baudRateText, "%d bps", baudRate(serial_sctrl()));
    VDP_drawText(baudRateText, 0, 2);
}

static void printSCtrlFlags(void)
{
    const u8 flags[] = { SCTRL_RERR, SCTRL_RRDY, SCTRL_TFUL };
    const char* names[] = { "RERR", "RRDY", "TFUL" };

    s8 sctrl = serial_sctrl();
    for (u16 i = 0; i < 3; i++) {
        if (sctrl & flags[i]) {
            VDP_setTextPalette(PAL0);
        } else {
            VDP_setTextPalette(PAL1);
        }
        VDP_drawText(names[i], 10 + (i * 5), 2);
    }
    VDP_setTextPalette(PAL0);
}

static void ui_callback(void)
{
    buffer_write(serial_receive());
    ui_dirty = TRUE;
}

static void incrementCursor(Cursor* cur)
{
    Cursor* cursor = cur;
    cursor->x++;
    if (cursor->x > BUFFER_MAX_X) {
        cursor->y++;
        cursor->x = 0;
    }
    if (cursor->y > BUFFER_MAX_LINES) {
        cursor->y = 0;
    }
}

static void receiveSerialIntoBuffer(Cursor* cur)
{
    while (serial_readyToReceive()) {
        buffer_write(serial_receive());
        ui_dirty = TRUE;
    }
}

static void readFromBuffer(Cursor* cur)
{
    VDP_setTextPalette(PAL1);
    if (buffer_canRead()) {
        u8 data = buffer_read();
        char buf[2] = { (char)data, 0 };
        VDP_drawText(buf, cur->x, cur->y + BUFFER_MIN_Y);
        incrementCursor(cur);
        if (cur->x == 0 && cur->y == 0) {
            VDP_clearTextArea(
                0, BUFFER_MIN_Y, BUFFER_MAX_X + 1, BUFFER_MAX_LINES + 1);
        }
        ui_dirty = TRUE;
    }
    VDP_setTextPalette(PAL0);
}

static void printBufferFree(void)
{
    if (ui_dirty) {
        char text[32];
        sprintf(text, "%4d Free", buffer_available());
        VDP_drawText(text, 28, 4);
        ui_dirty = FALSE;
    }
}

static void receive(Cursor* cur)
{
    if (!USE_RECV_INT) {
        receiveSerialIntoBuffer(cur);
    }
    readFromBuffer(cur);
    printBufferFree();
}

static void send(void)
{
    for (u8 i = '0'; i <= '9'; i++) {
        serial_sendWhenReady(i);
    }
    serial_sendWhenReady('\n');
}

static void init(void)
{
    VDP_setPaletteColor((PAL1 * 16) + 15, RGB24_TO_VDPCOLOR(0x444444));
    VDP_drawText("Mega Drive Serial Port Diagnostics", 3, 0);
    VDP_drawText("Recv Buffer:", 0, 4);

    u8 sctrlFlags = SCTRL_4800_BPS | SCTRL_SIN | SCTRL_SOUT;
    if (USE_RECV_INT) {
        sctrlFlags |= SCTRL_RINT;
    }
    serial_init(sctrlFlags);
    serial_setReadyToReceiveCallback(&ui_callback);

    printBaudRate();
}

static void sendAndReceiveLoop(void)
{
    const bool DO_RECEIVE = TRUE;
    const bool DO_SEND = TRUE;

    Cursor cur = { 0, 0 };
    while (TRUE) {
        printSCtrlFlags();
        if (DO_RECEIVE) {
            receive(&cur);
        }
        if (DO_SEND) {
            send();
        }
    }
}

int main()
{
    init();
    sendAndReceiveLoop();
    return 0;
}
