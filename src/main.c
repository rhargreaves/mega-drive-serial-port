#include <genesis.h>
#include <vdp.h>

#define PORT2_CTRL 0xA1000B
#define PORT2_SCTRL 0xA10019
#define PORT2_TX 0xA10015
#define PORT2_RX 0xA10017

#define VDP_MODE_REG_3 0xB
#define VDP_IE2 0x08

#define SCTRL_300_BPS 0xF0
#define SCTRL_1200_BPS 0xB0
#define SCTRL_2400_BPS 0x70
#define SCTRL_4800_BPS 0x30

#define SCTRL_RRDY 0x2
#define SCTRL_RINT 0x8

const u16 min_y = 10;
const u16 max_x = 39;
const u16 max_lines = 5;

typedef struct Cursor Cursor;

struct Cursor {
    u16 x;
    u16 y;
};

#define GET_BIT(var, bit) ((var & (1 << bit)) == (1 << bit))

static u16 int_count = 0;

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

static char buffer[256];

static void ext_int_callback(void)
{
    buffer[int_count] = read();
    int_count++;
}

void serial_init(void)
{
    set_sctrl(SCTRL_1200_BPS | SCTRL_RINT);
    set_ctrl(0x7F); // TH-Int:OFF, PC6..PC0: OUTPUT.
    VDP_setReg(VDP_MODE_REG_3, VDP_IE2); // Enable IE2 (enable external interrupts)
    SYS_setExtIntCallback(&ext_int_callback);
    SYS_setInterruptMaskLevel(1);
}

void print_sctrl(void)
{
    vs8* pb;
    pb = (s8*)PORT2_SCTRL;
    s8 data = *pb;

    for (u16 i = 0; i < 8; i++) {
        char text[2];
        sprintf(text, "%d", GET_BIT(data, 7 - i));
        VDP_drawText(text, i * 5, 3);
    }
}

void print_ctrl(void)
{
    vs8* pb;
    pb = (s8*)PORT2_CTRL;
    s8 data = *pb;

    for (u16 i = 0; i < 8; i++) {
        char text[2];
        sprintf(text, "%d", GET_BIT(data, 7 - i));
        VDP_drawText(text, i * 5, 6);
    }
}

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

void _outbyte(u8 c)
{
    vs8* pb = (s8*)PORT2_SCTRL;
    u8 byte;
    byte = c;

    /* wait for TFUL low */
    while (!(*pb & 0x02)) {
        pb = (s8*)PORT2_SCTRL;
    }

    pb = (s8*)PORT2_TX;
    *pb = byte;
}

static void increment_cursor(Cursor* cur)
{
    Cursor *cursor = cur;
    cursor->x++;
    if (cursor->x > max_x) {
        cursor->y++;
        cursor->x = 0;
    }
    if (cursor->y > max_lines) {
        cursor->y = 0;
    }
}


int main()
{
    serial_init();

    VDP_drawText("Mega Drive Serial Port Diagnostics", 3, 0);
    VDP_drawText("BPS1 BPS0 SIN  SOUT RINT RERR RRDY TFUL", 0, 2);
    VDP_drawText("INT  PC6  PC5  PC4  PC3  PC2  PC1  PC0", 0, 5);
    VDP_drawText("Recv:", 0, 9);

    Cursor cur = { 0, 0 };
    while (TRUE) {
        print_sctrl();
        print_ctrl();

        char text[4];
        sprintf(text, "%d", int_count);
        VDP_drawText(text, 0, 20);

        if (can_read()) {
            u8 data = read();
            char buf[2];
            buf[0] = (char)data;
            buf[1] = 0;

            VDP_drawText(buf, cur.x, cur.y + min_y);
            increment_cursor(&cur);
            if(cur.x == 0 && cur.y == 0)
            {
                VDP_clearTextArea(0, min_y, max_x + 1, max_lines + 1);
            }
        } else {
            VDP_waitVSync();
        }
        VDP_drawText(buffer, 0, 22);
    }
    return 0;
}
