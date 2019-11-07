#include <genesis.h>

#define PORT2_CTRL 0xA1000B
#define PORT2_SCTRL 0xA10019
#define PORT2_TX 0xA10015
#define PORT2_RX 0xA10017

#define SCTRL_300_BPS 0xF0
#define SCTRL_1200_BPS 0xB0
#define SCTRL_2400_BPS 0x70
#define SCTRL_4800_BPS 0x30

const u16 min_y = 10;
const u16 max_x = 39;
const u16 max_lines = 5;

typedef struct Cursor Cursor;

struct Cursor {
    u16 x;
    u16 y;
};

#define GET_BIT(var, bit) ((var & (1 << bit)) == (1 << bit))

static void increment_cursor(Cursor* cur);

void serial_init(void)
{
    vs8* pb;
    pb = (s8*)PORT2_SCTRL;
    /* S-CTRL is for the status, etc. of each port's mode change, baud rate and
       SERIAL.*/
    *pb = SCTRL_1200_BPS;

    vs8* pb2;
    pb2 = (s8*)PORT2_CTRL;
    *pb2 = 0x7F; // TH-Int:OFF, PC6..PC0: OUTPUT.
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
    return *pb & 0x02;
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

        if (can_read()) {
            u8 data = read();
            char buffer[2];
            buffer[0] = (char)data;

            VDP_drawText(buffer, cur.x, cur.y + min_y);
            increment_cursor(&cur);
            if(cur.x == 0 && cur.y == 0)
            {
                VDP_clearTextArea(0, min_y, max_x + 1, max_lines + 1);
            }
        } else {
            VDP_waitVSync();
        }
    }
    return 0;
}
