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

#define SCTRL_RRDY 0x2
#define SCTRL_RINT 0x8

#define INT_MASK_LEVEL_ENABLE_ALL 1

#define BUFFER_LEN 2048

const u16 min_y = 11;
const u16 max_x = 39;
const u16 max_lines = 5;

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
        buffer[write_head++] = read();
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

int main()
{
    serial_init();

    VDP_drawText("Mega Drive Serial Port Diagnostics", 3, 0);
    VDP_drawText("BPS1 BPS0 SIN  SOUT RINT RERR RRDY TFUL", 0, 2);
    VDP_drawText("INT  PC6  PC5  PC4  PC3  PC2  PC1  PC0", 0, 5);
    VDP_drawText("Read Buffer:", 0, 10);

    print_ctrl();

    Cursor cur = { 0, 0 };
    while (TRUE) {
        print_sctrl();
        //  read_direct(&cur);
        read_from_buffer(&cur);

        if (ui_dirty) {
            /*
            ----R--------W-----
            xxxxx        xxxxxx

            ----W--------R-----
                xxxxxxxxx
            */
            u16 bufferLeft;
            if (write_head >= read_head) {
                bufferLeft = BUFFER_LEN - (write_head - read_head);
            } else {
                bufferLeft = read_head - write_head;
            }
            char text[32];
            sprintf(text, "Bytes Free: %-4d", bufferLeft);
            VDP_drawText(text, 0, 25);
            ui_dirty = FALSE;
        }
    }
    return 0;
}
