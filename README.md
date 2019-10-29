# Mega Drive Serial Port
Tools and docs for using a SEGA Mega Drive controller port in serial mode

## Controller Port Pinout

| Pin | Parallel* | Serial |
|-----|-------------------------|--------|
| 1 | Up | |
| 2 | Down | |
| 3 | Gnd / Left | |
| 4 | Gnd / Right | |
| 5 | +5VDC | +5VDC |
| 6 | Button A / Button B | Tx |
| 7 | Select | |
| 8 | Gnd | Gnd |
| 9 | Start / Button C | Rx |

\* Pins in parallel mode can have two interpretations, depending on if `Select` is low or high (L / H)
