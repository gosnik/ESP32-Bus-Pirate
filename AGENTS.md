# AGENTS

This repository contains a `qtbits` board variant for `ESP32-Bus-Pirate`. Use this file when Codex needs to use a physical QtBits device as a bench instrument to test, debug, or diagnose another embedded target.

## Scope

Use the QtBits-flashed Bus Pirate as:

- A passive observer for unknown digital buses.
- A simple active probe for UART, I2C, SPI, 1-Wire, GPIO, and JTAG/SWD discovery.
- A repeatable serial-controlled test fixture for external devices.

Do not treat it as a generic dev board first. Treat it as an instrument with a CLI.

## Safety Rules

- Prefer non-destructive operations first: `scan`, `sniff`, `read`, `measure`, `wizard`, `logic`, `analogic`, `ping`, `identify`.
- Do not use `write`, `erase`, `glitch`, `jam`, `flood`, `slave`, `emulator`, or continuous drive commands unless the user explicitly wants active manipulation.
- The upstream docs warn that connected devices should only operate at `3.3V` or `5V`. If the target voltage is unknown, assume `3.3V` until confirmed.
- Never attach QtBits display, touch, or encoder pins to a DUT.
- For high-volume output or timing-sensitive work, prefer USB serial over the web UI.

## QtBits Board Assumptions

The local `qtbits` environment is defined in [platformio.ini](./platformio.ini) under `[env:qtbits]`.

Reserved board pins:

- Display/touch/board control: `0,1,2,3,4,5,33,34,35,36,37,38,39,40,41,42`
- Protected set in this repo also excludes: `6,7,8,9,10,11,12,13,17,18`

Preferred DUT-facing pins from the current repo configuration:

- `UART`: `RX=44`, `TX=43`
- `HDUART`: `IO=44`
- `I2C`: `SDA=15`, `SCL=16`
- `SPI`: `CS=14`, `CLK=15`, `MISO=16`, `MOSI=21`
- `1WIRE`: `DQ=14`
- `2WIRE`: `CLK=15`, `IO=16`, `RST=21`
- `3WIRE`: `CS=14`, `SK=15`, `DI=16`, `DO=21`
- `JTAG` scan set: `14,15,16,21,43,44`

If a task needs different pins, use the mode’s `config` command or adjust the firmware profile explicitly.

## Connect to the Device

Prefer USB serial.

On Linux, find the port with:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Open the session at `115200 8N1`. The upstream docs recommend `screen`:

```bash
screen /dev/ttyACM0 115200
```

If no banner appears, send any key or a newline.

## How Codex Should Work

When using the device to diagnose a DUT:

1. Start in a safe mode.
   Use `mode hiz` first if the target state is unknown.
2. Identify candidate pins and activity.
   Use `mode dio`, then `pins`, `scan`, `wizard <gpio>`, `measure <gpio>`, `sniff <gpio>`, `logic <gpio>`, or `analogic <gpio>`.
3. Switch to the protocol mode only after you have a concrete hypothesis.
4. Prefer passive inspection before active transactions.
5. If active transactions are required, keep them minimal and explain the risk.
6. For repeatable tests, use direct commands or instruction syntax instead of interactive menus.

Always send an explicit mode command before protocol actions:

```text
mode i2c
mode spi
mode uart
mode 1wire
mode jtag
mode dio
```

## Recommended Debug Playbooks

### Unknown Target

Start with:

```text
mode hiz
mode dio
pins
scan
wizard 14
wizard 15
wizard 16
wizard 21
wizard 43
wizard 44
measure 14
sniff 14
```

Use this to determine whether the signal looks idle-high, clocked, bursty, analog, or UART-like.

### UART

Use when the DUT likely has TX/RX serial.

Safe-first sequence:

```text
mode uart
config
scan
autobaud
ping
read
raw
```

Active sequence:

```text
write AT
write Hello\n
['AT' r:64]
bridge
```

Use `bridge` only when interactive passthrough is the goal.

### Half-Duplex UART

Use for single-wire serial links:

```text
mode hduart
config
bridge
[0x01 D:10 r:255]
```

### I2C

Default QtBits bus is `SDA=15`, `SCL=16`.

Safe-first sequence:

```text
mode i2c
config
scan
ping 0x50
identify 0x50
discovery
sniff
```

Minimal active reads:

```text
read 0x50 0x00
dump 0x50 64
monitor 0x50 500
[0x50 0x00 r:4]
```

Do not use `glitch`, `jam`, `flood`, or `slave` unless explicitly requested.

### SPI

Default QtBits bus is `CS=14`, `CLK=15`, `MISO=16`, `MOSI=21`.

Safe-first sequence:

```text
mode spi
config
sniff
[0x9F r:3]
```

Use `[0x9F r:3]` as a low-risk JEDEC-ID probe for common SPI flash parts.

Use `flash` or `eeprom` shells only when the target is known and reads are intended.

### 1-Wire

Default QtBits pin is `14`.

Safe-first sequence:

```text
mode 1wire
scan
ping
read
temp
sniff
[0x33 r:8]
```

Prefer serial CLI for timing-sensitive 1-Wire work.

### GPIO / DIO

Use for simple line stimulation or observation:

```text
mode dio
read 14
pullup 14
pulldown 14
pulse 14 50
measure 14 1000
sniff 14
pwm 14 1000 50
```

Avoid `jam` unless fault injection is explicitly requested.

### JTAG / SWD Discovery

Use this only after passive inspection suggests a debug port.

```text
mode jtag
config
scan swd
scan jtag
```

The current QtBits scan pool is `14,15,16,21,43,44`.

## General Commands Worth Using

These are useful across modes:

```text
help
system
mode
profile
wizard 14
logic 14
analogic 14
repeat 5 scan
P
p
```

Use `repeat` for quick polling loops. Use `P` and `p` only when a pull-up is appropriate for the active bus.

## Instruction Syntax

The firmware supports compact Bus Pirate-like instruction sequences. Useful examples from the upstream docs:

```text
[0xAA 0xBB]
[r:4]
[0xA1 r]
['A']
["ABC"]
[d:10]
[D:1]
```

Use instruction syntax when you need deterministic read/write/delay sequences without interactive prompts.

## Automation Guidance

Prefer serial automation for repeatable diagnostics. The upstream project documents a Python helper in the separate `ESP32-Bus-Pirate-Scripts` repository.

If you script directly, keep the host workflow simple:

1. Open the serial port at `115200`.
2. Send a newline to wake the prompt.
3. Send `mode <protocol>`.
4. Send explicit commands.
5. Read until quiet or until the expected marker appears.

For heavy sniffers, long dumps, or repeated tests, do not use the web terminal.

## When to Stop and Ask

Stop and ask the user before:

- Driving lines that may be connected to powered outputs.
- Writing memory or registers on an unknown target.
- Injecting glitches, jams, floods, or slave/emulator traffic.
- Erasing EEPROM or flash.
- Using RF, infrared, or network attack features outside an explicitly authorized test.

## Sources

This file is based on the local `qtbits` configuration in this repo and the upstream wiki pages:

- `Home`
- `00-Terminal`
- `00-Quick`
- `25-General`
- `99-Serial`
- `99-Instructions`
- `99-Python`
- `01-HiZ`
- `02-1WIRE`
- `03-UART`
- `04-HDUART`
- `05-I2C`
- `06-SPI`
- `09-DIO`
- `15-JTAG`
