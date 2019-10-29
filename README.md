# Mega Drive Serial Port
Tools and docs for using a SEGA Mega Drive controller port in serial mode

## Controller Port Pinout

| Pin | Parallel Mode * | Serial Mode | I/O Reg Bit |
|-----|-----------|--------|--------------|
| 1 | Up | | UP (D0) |
| 2 | Down | | DOWN (D1) |
| 3 | Gnd / Left | | LEFT (D2) |
| 4 | Gnd / Right | | RIGHT (D3) |
| 5 | +5VDC | +5VDC |
| 6 | Button A / Button B | Tx | TL (D4) |
| 7 | Select | |
| 8 | Gnd | Gnd |
| 9 | Start / Button C | Rx | TR (D5) |

\* Pins in parallel mode can have two interpretations, depending on if `Select` has been set low or high (L / H) from the console.

### Controller Plug
Looking directly at plug (Female 9-pin Type D)

```
-------------
\ 5 4 3 2 1 /
 \ 9 8 7 6 /
  ---------
```

## FTDI USB TTL Serial Cable

From the [datasheet](https://www.ftdichip.com/Support/Documents/DataSheets/Cables/DS_TTL-232RG_CABLES.pdf):

| Colour | Name | Type | Description |
|--------|------|------|-------------|
|Black |GND |GND |Device ground supply pin.|
|Brown |CTS# |Input |Clear to Send Control input / Handshake signal.|
|Red |VCC |Output or input | Power Supply Output except for the TTL-232RG-VIPWE were this is an input and power is supplied by the application interface logic. |
|Orange| TXD |Output |Transmit Asynchronous Data output.|
|Yellow| RXD| Input| Receive Asynchronous Data input.|
|Green |RTS#| Output| Request To Send Control Output / Handshake signal.|

## Mappings

| FTDI Cable | Mega Drive Port Pin |
|------------|-----------------|
| Orange (TXD) | 9 (Rx)        |
| Yellow (RXD) | 6 (Tx)        |
| Black (GND) | 8 (Tx)         |


