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
