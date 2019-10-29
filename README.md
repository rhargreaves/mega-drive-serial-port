# Mega Drive Serial Port
Tools and docs for using a SEGA Mega Drive controller port in serial mode

## Controller Port Pinout

| Pin | Parallel Mode * | Serial Mode | Ctrl Bit |
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

\* Pins in parallel mode can have two interpretations, depending on if `Select` is low or high (L / H)
