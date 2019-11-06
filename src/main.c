#include <genesis.h>

#define PORT2_CTRL 0xA1000B
#define PORT2_SCTRL 0xA10019
#define PORT2_TX 0xA10015
#define PORT2_RX 0xA10017

#define SCTRL_300_BPS 0xF0
#define SCTRL_1200_BPS 0xB0
#define SCTRL_2400_BPS 0x70
#define SCTRL_4800_BPS 0x30

#define GET_BIT(var, bit) ((var & (1 << bit)) == (1 << bit))

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
    VDP_drawText("BPS1 BPS0 SIN  SOUT RINT RERR RRDY TFUL", 0, 2);
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
    VDP_drawText("INT  PC6  PC5  PC4  PC3  PC2  PC1  PC0", 0, 5);
    for (u16 i = 0; i < 8; i++) {
        char text[2];
        sprintf(text, "%d", GET_BIT(data, 7 - i));
        VDP_drawText(text, i * 5, 6);
    }
}

u8 _inbyte(void)
{
    vs8* pb = (s8*)PORT2_SCTRL;
    u8 byte;

    /* wait for RRDY high */
    // while (*pb & 0x02) {
    //     pb = (s8*)PORT2_SCTRL;
    // }

    pb = (s8*)PORT2_RX;
    byte = *pb;

    return byte;
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

int main()
{
    serial_init();
    while (TRUE) {
        print_sctrl();
        print_ctrl();

        for (u8 i = 0; i < 10; i++) {
            _outbyte((u8)'0' + i);
        }
        // VDP_waitVSync();

        u8 data = _inbyte();
        char buffer[5] = "    ";
        buffer[0] = (char)data;
        VDP_drawText(buffer, 1, 1);
        // VDP_waitVSync();
    }
    return 0;
}
