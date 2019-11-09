#include "serial.h"
#include <genesis.h>
#include <stdbool.h>
#include <vdp.h>

const bool DO_READ = TRUE;
const bool DO_WRITE = TRUE;
const bool USE_RINT = TRUE;

const u16 BUFFER_MIN_Y = 6;
const u16 BUFFER_MAX_X = 39;
const u16 BUFFER_MAX_LINES = 6;

#define BUFFER_LEN 2048

typedef struct Cursor Cursor;

struct Cursor {
    u16 x;
    u16 y;
};

static u16 readHead = 0;
static u16 writeHead = 0;
static u8 ui_dirty = FALSE;

static void incrementCursor(Cursor* cur);

static char buffer[BUFFER_LEN];

static u8 canReadBuffer(void)
{
    return writeHead != readHead;
}

static u8 readBuffer(void)
{
    u8 data = buffer[readHead++];
    if (readHead == BUFFER_LEN) {
        readHead = 0;
    }
    return data;
}

static void writeToBuffer(u8 data)
{
    buffer[writeHead++] = data;
    if (writeHead == BUFFER_LEN) {
        writeHead = 0;
    }
}

static u16 get_baud_rate(u16 sctrl)
{
    u16 baudRate;
    switch (sctrl & 0xC0) {
    case SCTRL_300_BPS:
        baudRate = 300;
        break;
    case SCTRL_1200_BPS:
        baudRate = 1200;
        break;
    case SCTRL_2400_BPS:
        baudRate = 2400;
        break;
    default:
        baudRate = 4800;
        break;
    }
    return baudRate;
}

void printSCtrl(void)
{
    s8 sctrl = serial_sctrl();

    char baudRateText[9];
    sprintf(baudRateText, "%d bps", get_baud_rate(sctrl));
    VDP_drawText(baudRateText, 0, 2);

    if ((sctrl & SCTRL_RERR) == SCTRL_RERR) {
        VDP_setTextPalette(PAL0);
    } else {
        VDP_setTextPalette(PAL1);
    }
    VDP_drawText("RERR", 10, 2);

    if ((sctrl & SCTRL_RRDY) == SCTRL_RRDY) {
        VDP_setTextPalette(PAL0);
    } else {
        VDP_setTextPalette(PAL1);
    }
    VDP_drawText("RRDY", 15, 2);

    if ((sctrl & SCTRL_TFUL) == SCTRL_TFUL) {
        VDP_setTextPalette(PAL0);
    } else {
        VDP_setTextPalette(PAL1);
    }
    VDP_drawText("TFUL", 20, 2);
    VDP_setTextPalette(PAL0);
}

static void ui_callback(void)
{
    writeToBuffer(serial_read());
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

static void readSerialIntoBuffer(Cursor* cur)
{
    while (serial_readReady()) {
        writeToBuffer(serial_read());
        ui_dirty = TRUE;
    }
}

static void readFromBuffer(Cursor* cur)
{
    VDP_setTextPalette(PAL1);
    if (canReadBuffer()) {
        u8 data = readBuffer();
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

static u16 bufferFree(void)
{
    /*
    ----R--------W-----
    xxxxx        xxxxxx

    ----W--------R-----
        xxxxxxxxx
    */
    if (writeHead >= readHead) {
        return BUFFER_LEN - (writeHead - readHead);
    } else {
        return readHead - writeHead;
    }
}

static void printBufferFree(void)
{
    if (ui_dirty) {
        char text[32];
        sprintf(text, "%4d Free", bufferFree());
        VDP_drawText(text, 28, 4);
        ui_dirty = FALSE;
    }
}

int main()
{
    VDP_setPaletteColor((PAL1 * 16) + 15, RGB24_TO_VDPCOLOR(0x444444));

    VDP_drawText("Mega Drive Serial Port Diagnostics", 3, 0);
    VDP_drawText("Read Buffer:", 0, 4);

    u8 sctrlFlags = SCTRL_4800_BPS | SCTRL_SIN | SCTRL_SOUT;
    if (USE_RINT) {
        sctrlFlags |= SCTRL_RINT;
    }
    serial_init(sctrlFlags);
    serial_setReadReadyCallback(&ui_callback);

    Cursor cur = { 0, 0 };
    while (TRUE) {
        printSCtrl();

        if (DO_READ) {
            if (!USE_RINT) {
                readSerialIntoBuffer(&cur);
            }
            readFromBuffer(&cur);
            printBufferFree();
        }
        if (DO_WRITE) {
            for (u8 i = '0'; i <= 'z'; i++) {
                serial_sendWhenReady(i);
            }
            serial_sendWhenReady('\n');
        }

        VDP_waitVSync();
    }
    return 0;
}
