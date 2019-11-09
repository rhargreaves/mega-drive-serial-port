#include "serial.h"
#include <genesis.h>
#include <vdp.h>

#define BUFFER_LEN 2048

const u16 min_y = 6;
const u16 max_x = 39;
const u16 max_lines = 10;

typedef struct Cursor Cursor;

struct Cursor {
    u16 x;
    u16 y;
};

static u16 read_head = 0;
static u16 write_head = 0;
static u8 ui_dirty = FALSE;

static void increment_cursor(Cursor* cur);

static char buffer[BUFFER_LEN];

static u8 can_read_buffer(void)
{
    return write_head != read_head;
}

static u8 read_buffer(void)
{
    u8 data = buffer[read_head++];
    if (read_head == BUFFER_LEN) {
        read_head = 0;
    }
    return data;
}

static void write_buffer(u8 data)
{
    buffer[write_head++] = data;
    if (write_head == BUFFER_LEN) {
        write_head = 0;
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

void print_sctrl(void)
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
    write_buffer(serial_read());
    ui_dirty = TRUE;
}

static void increment_cursor(Cursor* cur)
{
    Cursor* cursor = cur;
    cursor->x++;
    if (cursor->x > max_x) {
        cursor->y++;
        cursor->x = 0;
    }
    if (cursor->y > max_lines) {
        cursor->y = 0;
    }
}

static void read_direct(Cursor* cur)
{
    while (serial_readReady()) {
        write_buffer(read());
        ui_dirty = TRUE;
    }
}

static void read_from_buffer(Cursor* cur)
{
    if (can_read_buffer()) {
        u8 data = read_buffer();
        char buf[2] = { (char)data, 0 };
        VDP_drawText(buf, cur->x, cur->y + min_y);
        increment_cursor(cur);
        if (cur->x == 0 && cur->y == 0) {
            VDP_clearTextArea(0, min_y, max_x + 1, max_lines + 1);
        }
        ui_dirty = TRUE;
    }
}

static u16 buffer_free(void)
{
    /*
    ----R--------W-----
    xxxxx        xxxxxx

    ----W--------R-----
        xxxxxxxxx
    */
    if (write_head >= read_head) {
        return BUFFER_LEN - (write_head - read_head);
    } else {
        return read_head - write_head;
    }
}

int main()
{
    VDP_setPaletteColor((PAL1 * 16) + 15, RGB24_TO_VDPCOLOR(0x444444));

    VDP_drawText("Mega Drive Serial Port Diagnostics", 3, 0);
    VDP_drawText("Read Buffer:", 0, 4);

    serial_init(SCTRL_4800_BPS | SCTRL_RINT | SCTRL_SIN | SCTRL_SOUT);
    serial_setReadReadyCallback(&ui_callback);

    Cursor cur = { 0, 0 };
    while (TRUE) {
        print_sctrl();
        // read_direct(&cur);
        read_from_buffer(&cur);
        if (ui_dirty) {
            char text[32];
            sprintf(text, "%-4d Free", buffer_free());
            VDP_drawText(text, 28, 4);
            ui_dirty = FALSE;
        }
        VDP_waitVSync();
    }
    return 0;
}
