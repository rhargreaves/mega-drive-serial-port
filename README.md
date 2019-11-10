# Mega Drive Serial Port [![CircleCI](https://circleci.com/gh/rhargreaves/mega-drive-serial-port.svg?style=svg)](https://circleci.com/gh/rhargreaves/mega-drive-serial-port) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/rhargreaves/mega-drive-serial-port?style=plastic)](https://github.com/rhargreaves/mega-drive-serial-port/releases)

Tools and docs for using a SEGA Mega Drive controller port in serial mode

## Diagnostics Tool

When the ROM starts, it:

- Enables Serial Mode on Port 2
- Sets speed to [4800 bps](https://github.com/rhargreaves/mega-drive-serial-port/blob/8b287d3986b7dcea66f2a9463d649aaceb826bf1/src/main.c#L137)
- Receives data into a 2K buffer on hardware interrupt ([can be configured to poll instead](https://github.com/rhargreaves/mega-drive-serial-port/blob/8b287d3986b7dcea66f2a9463d649aaceb826bf1/src/main.c#L7), but is not reliable at speeds greater than 1200 bps)
- Send a continuous stream of `0123456789\n`
- Prints any received data to the screen.

<p align="center">
    <img src="https://github.com/rhargreaves/mega-drive-serial-port/raw/master/docs/screen.jpg" width="500" />
</p>

### Build

Docker:

```sh
./docker-make
```

Linux (requires `cmake` & [gendev](https://github.com/kubilus1/gendev)):

```sh
make
```

## Hardware

### Controller Port Pinout

| Pin | Parallel Mode \*    | Serial Mode | I/O Reg Bit |
| --- | ------------------- | ----------- | ----------- |
| 1   | Up                  |             | UP (D0)     |
| 2   | Down                |             | DOWN (D1)   |
| 3   | Gnd / Left          |             | LEFT (D2)   |
| 4   | Gnd / Right         |             | RIGHT (D3)  |
| 5   | +5VDC               | +5VDC       |
| 6   | Button A / Button B | Tx          | TL (D4)     |
| 7   | Select              |             |
| 8   | Gnd                 | Gnd         |
| 9   | Start / Button C    | Rx          | TR (D5)     |

\* Pins in parallel mode can have two interpretations, depending on if `Select` has been set low or high (L / H) from the console.

### Controller Plug

Looking directly at plug (Female 9-pin Type D)

```
-------------
\ 5 4 3 2 1 /
 \ 9 8 7 6 /
  ---------
```

### FTDI USB TTL Serial Cable

From the [datasheet](https://www.ftdichip.com/Support/Documents/DataSheets/Cables/DS_TTL-232RG_CABLES.pdf):

| Colour | Name | Type            | Description                                                                                                                        |
| ------ | ---- | --------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| Black  | GND  | GND             | Device ground supply pin.                                                                                                          |
| Brown  | CTS# | Input           | Clear to Send Control input / Handshake signal.                                                                                    |
| Red    | VCC  | Output or input | Power Supply Output except for the TTL-232RG-VIPWE were this is an input and power is supplied by the application interface logic. |
| Orange | TXD  | Output          | Transmit Asynchronous Data output.                                                                                                 |
| Yellow | RXD  | Input           | Receive Asynchronous Data input.                                                                                                   |
| Green  | RTS# | Output          | Request To Send Control Output / Handshake signal.                                                                                 |

### Mappings

| FTDI Cable   | Mega Drive Port Pin |
| ------------ | ------------------- |
| Orange (TXD) | 9 (Rx)              |
| Yellow (RXD) | 6 (Tx)              |
| Black (GND)  | 8 (Gnd)             |
