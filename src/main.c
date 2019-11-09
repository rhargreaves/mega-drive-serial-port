#include <genesis.h>
#include <vdp.h>

#define PORT2_CTRL 0xA1000B
#define PORT2_SCTRL 0xA10019
#define PORT2_TX 0xA10015
#define PORT2_RX 0xA10017

#define VDP_MODE_REG_3 0xB
#define VDP_IE2 0x08

#define CTRL_PCS_OUT 0x7F

#define SCTRL_SIN 0x20
#define SCTRL_SOUT 0x10
#define SCTRL_300_BPS 0xC0
#define SCTRL_1200_BPS 0x80
#define SCTRL_2400_BPS 0x40
#define SCTRL_4800_BPS 0x00

#define SCTRL_TFUL 0x1
#define SCTRL_RRDY 0x2
#define SCTRL_RERR 0x4
#define SCTRL_RINT 0x8

#define INT_MASK_LEVEL_ENABLE_ALL 1

#define BUFFER_LEN 2048

const u16 min_y = 6;
const u16 max_x = 39;
const u16 max_lines = 10;

typedef struct Cursor Cursor;

struct Cursor {
    u16 x;
    u16 y;
};

#define GET_BIT(var, bit) ((var & (1 << bit)) == (1 << bit))

static u16 read_head = 0;
static u16 write_head = 0;
static u8 ui_dirty = FALSE;

u8 read(void);
static void increment_cursor(Cursor* cur);

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

static char buffer[BUFFER_LEN];

u8 can_read(void)
{
    vs8* pb = (s8*)PORT2_SCTRL;
    return *pb & SCTRL_RRDY;
}

u8 read(void)
{
    vs8* pb = (s8*)PORT2_RX;
    return *pb;
}

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

static void ext_int_callback(void)
{
    write_buffer(read());
    ui_dirty = TRUE;
}

void serial_init(void)
{
    set_sctrl(SCTRL_4800_BPS | SCTRL_RINT | SCTRL_SIN | SCTRL_SOUT);
    set_ctrl(CTRL_PCS_OUT);
    VDP_setReg(VDP_MODE_REG_3, VDP_IE2);
    SYS_setExtIntCallback(&ext_int_callback);
    SYS_setInterruptMaskLevel(INT_MASK_LEVEL_ENABLE_ALL);
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
    vs8* pb;
    pb = (s8*)PORT2_SCTRL;
    s8 sctrl = *pb;

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
    while (can_read()) {
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
    VDP_drawText("Mega Drive Serial Port Diagnostics", 3, 0);
    VDP_drawText("Read Buffer:", 0, 4);

    serial_init();
    Cursor cur = { 0, 0 };
    while (TRUE) {
        print_sctrl();
        // read_direct(&cur);
        read_from_buffer(&cur);
        if (ui_dirty) {
            char text[32];
            sprintf(text, "Bytes Free: %-4d", buffer_free());
            VDP_drawText(text, 18, 4);
            ui_dirty = FALSE;
        }
        VDP_waitVSync();
    }
    return 0;
}
