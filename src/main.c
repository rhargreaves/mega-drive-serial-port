#include <genesis.h>

#define PORT2_CTRL 0xA1000B
#define PORT2_SCTRL 0xA10019
#define PORT2_TX 0xA10015
#define PORT2_RX 0xA10017

void serial_init(void)
{
    volatile u8* pb;
    pb = (volatile u8*)PORT2_SCTRL;
    /* S-CTRL is for the status, etc. of each port's mode change, baud rate and
       SERIAL.*/
    *pb = 0xB0; /* 1200bps serial in/out enable */

    volatile u8* pb2;
    pb2 = (volatile u8*)PORT2_CTRL;
    *pb2 = 0x7F; // TH-Int:OFF, PC6..PC0: OUTPUT.
}

u8 _inbyte(void)
{
    volatile u8* pb;
    u8 byte;

    /* wait for RRDY low */
    while (*pb & 0x02) {
        pb = (volatile u8*)PORT2_SCTRL;
    }

    pb = (volatile u8*)PORT2_RX;
    byte = *pb;

    return byte;
}

void _outbyte(u8 c)
{
    volatile u8* pb;
    u8 byte;
    byte = c;

    /* wait for RRDY high */
    while (!(*pb & 0x02)) {
        pb = (volatile u8*)PORT2_SCTRL;
    }

    pb = (volatile u8*)PORT2_TX;
    *pb = byte;
}

int main()
{
    serial_init();
    while (TRUE) {
        for (u8 i = 0; i < 10; i++) {
            _outbyte((u8)'0' + i);
            VDP_waitVSync();
        }
    }
    return 0;
}
