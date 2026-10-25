
# IN12v3clock

Alternate firmware for the GeekStyles IN12 V3 Nixie nixieclock PCB.

## Version
- **2.00**: Rework of the firmware:
  - day and night modes with smooth brightness change, separate digits and backlight brightness, the night digits can be off;
  - perceptually linear backlight brightness (CIE lightness), backlight on/off by K2 for day and night separately;
  - digits PWM 100/200/400 Hz with flicker free updates, fast 595 shift out (assembler);
  - minute change effects (fade, roll, train, elastic), antipoisoning effects, glitches
    (ideas from [NixieClock v2](https://github.com/AlexGyver/NixieClock_v2) by AlexGyver);
  - 6 colon modes, leading zero, 12/24-hour format;
  - settings menu by groups, the night start and end set as a time;
  - DS3231: ~100 kHz I2C, the oscillator is enabled, the lost time (empty battery) is reset to 00:00;
  - command line build and flashing (`tools/`), many bug fixes. Cosmic: ~6.6 KB of 8 KB.
- **1.00**: Initial release.

## Description
This firmware is designed for the IN12 V3 clock PCB, 
with frontside inscription: **GeekStyles IN12 V3**  
[![Frontside Preview](Photo/pcb_front_preview.JPG)](Photo/pcb_front_fullsize.JPG)
and the backside inscription: **JM NixieClock IN12 V4.2 20190514**.   
[![Backside Preview](Photo/pcb_back_preview.JPG)](Photo/pcb_back_fullsize.JPG)
The STM8S003F3P6 microcontroller features 8 KB of Flash memory, 1 KB of RAM, and 128 bytes of EEPROM.

---

## Quick Start: flash the precompiled firmware (no build)
The repository has the ready firmware of the current version, no compiler is needed to flash it:
`Release/in12v3clock.s19` (Cosmic, recommended), `SDCC/main.s19` and `SDCC/main.ihx` (the same firmware built with SDCC).

1. **Get the firmware:** `git clone https://github.com/siliverst/IN12v3clock.git`, or on GitHub **Code > Download ZIP**
   (the submodule is not needed for flashing), or only the file:
   [in12v3clock.s19](https://github.com/siliverst/IN12v3clock/raw/master/Release/in12v3clock.s19).
2. **Install** [ST Visual Programmer (STVP)](https://www.st.com/en/development-tools/stvp-stm8.html) (it installs the ST-LINK driver too).
3. **Connect** an ST-LINK V2 to the debug connector: `SWIM` -> pin 2, `RST` -> pin 3, `GND` -> pin 4, power the clock
   from its own supply (see [Flashing](#flashing)).
4. **Flash** in one of the ways (in the project folder):
   - Python 3: `python tools/flash.py release`
   - VS Code: `Terminal > Run Task...` > `Flash` > `Precompiled firmware from the repository (Release/, no compiler needed)`
   - PowerShell, no Python:
     ```
     & "C:\Program Files (x86)\STMicroelectronics\st_toolset\stvp\STVP_CmdLine.exe" -BoardName=ST-LINK -Port=USB -ProgMode=SWIM -Device=STM8S003F3 -no_loop -no_log -no_progress -verif -no_progOption -no_progData -FileProg=Release\in12v3clock.s19
     ```
   - STVP window: open the project `in12v3clock_Programmer/in12v3clock_STVP.stp`, tab **PROGRAM MEMORY**, `Program > Current tab`
   - Linux / macOS: `stm8flash -c stlinkv2 -p stm8s003f3 -s flash -w Release/in12v3clock.s19`

   Success: `<<< Programming PROGRAM MEMORY succeeds` and `<<< Verifying PROGRAM MEMORY succeeds`
   (the lines about DATA MEMORY and OPTION BYTE in the STVP_CmdLine output can be ignored, they are not written).
5. **Once for a new chip:** the option byte `AFR0` (the green backlight channel), see [Flashing](#flashing).
   The settings: the firmware writes the defaults itself if the EEPROM is empty or has an old layout.

---

## Requirements
What to install (Windows):

| Tool | Needed for | Where | Default path (used if no path is set) |
|------|-----------|-------|----------------------------------------|
| **Cosmic CXSTM8** (free license, registration) | build, the main compiler | [cosmicsoftware.com](https://www.cosmicsoftware.com/download_stm8_free.php) | `C:\Program Files (x86)\COSMIC\FSE_Compilers\CXSTM8` |
| **SDCC** 4.x (4.5 is tested) | build, alternative (the binary is ~0.8 KB larger) | [sdcc.sourceforge.net](https://sdcc.sourceforge.net) | `C:\Program Files\SDCC\bin` |
| **Python 3** | build and flash scripts in `tools/` | [python.org](https://www.python.org) | in `PATH` |
| **ST Visual Programmer (STVP)** + ST-LINK driver | flashing (`STVP_CmdLine.exe`) | [st.com](https://www.st.com/en/development-tools/stvp-stm8.html) | `C:\Program Files (x86)\STMicroelectronics\st_toolset\stvp\STVP_CmdLine.exe` |
| **VS Code** + C/C++ extension | editor (optional) | [code.visualstudio.com](https://code.visualstudio.com) | |

Other paths are set in one of the ways (an empty value means the default path):
- VS Code: `.vscode/settings.json`: `in12v3clock.cosmicDir`, `in12v3clock.sdccDir`, `in12v3clock.stvpCmdLine`;
- command line: the environment variables `COSMIC_DIR`, `SDCC_DIR`, `STVP_CMDLINE`.

External libraries: the submodule [STM8_headers](https://github.com/gicking/STM8_headers) (`git clone --recurse-submodules`).

## Working in VS Code
1. Install the tools from the table above. If a tool is not in its default path, set the path in `.vscode/settings.json`.
2. Open the project folder: `File > Open Folder`. VS Code offers the recommended extensions (C/C++, Python), install them.
3. **Build:** `Ctrl+Shift+B`, choose the compiler (`cosmic`, `sdcc` or both). The terminal at the bottom shows the result:
   `COSMIC: ROM 6611 / 8192 (1581 free) ...`. The errors are in the Problems panel (`Ctrl+Shift+M`), a click opens the line.
4. **Other tasks** (flashing etc.): `Terminal > Run Task...` (or `Ctrl+Shift+P` > `Tasks: Run Task`), choose the task:

   | Task | What it does |
   |------|--------------|
   | `Build` | build, asks for the compiler (the same as `Ctrl+Shift+B`) |
   | `Build Cosmic`, `Build SDCC` | build with one compiler, no question |
   | `Flash` | flash through ST-LINK, asks: Cosmic or SDCC firmware or the precompiled one (`Release/`, no build needed), with or without the default EEPROM (the settings reset). Option bytes are not touched |
   | `Build and flash Cosmic` | build and flash at once — the usual one |
   | `Special build Cosmic` | test builds: `AP_TEST` (the time runs 6 times faster, antipoisoning every minute), `DIAG_RESET` (the reset cause at start), `K1_DOUBLE_CLICK=0` |
   | `Default EEPROM file` | regenerate the default EEPROM file after a change of the settings layout |

5. **Flashing:** connect the ST-LINK to the debug connector, power the clock, run `Flash` or `Build and flash Cosmic`.
   Success is two lines in the terminal:
   ```
   <<< Programming PROGRAM MEMORY succeeds
   <<< Verifying PROGRAM MEMORY succeeds
   ```
   `ERROR : Cannot communicate with the device` — the ST-LINK is not connected or the clock has no power.
   The option byte `AFR0` must be set once (see [Flashing](#flashing)), the scripts do not touch the option bytes.
6. **Repeat the last task:** `Ctrl+Shift+P` > `Tasks: Rerun Last Task`. A key can be assigned to it:
   `File > Preferences > Keyboard Shortcuts`, search `Rerun Last Task`.
- Output files: `build/cosmic/in12v3clock.s19`, `build/sdcc/main.s19`. The build fails if the firmware does not fit into 8 KB.

The old STVD project (`in12v3clock.stp`) still works, but VS Code with the scripts is the main way now.

---

## Pinout
### Microcontroller Pin Assignment
| Pin  | Signal                          | Description                                                        |
|------|---------------------------------|--------------------------------------------------------------------|
| 1    | PD4 (HS) UART1_CK/TIM2_CH1/BEEP | Input: Button UP with 10k pull-up resistor. Grounded when pressed.     |
| 2    | PD5 (HS) UART1_TX/AIN5          | Input: Button DOWN with 10k pull-up resistor. Grounded when pressed.   |
| 3    | PD6 (HS) UART1_RX               | Output: Drives colon neons (DS1 & DS2) through 1k resistor.       |
| 4    | NRST                            | Debug connector pin 3.                                             |
| 5    | PA1 OSCIN                       | Clock input.                                                       |
| 6    | PA2 OSCOUT                      | Clock output.                                                      |
| 7    | Vss                             | Ground.                                                            |
| 8    | Vcap                            | Decoupling capacitor.                                              |
| 9    | Vdd                             | +3.3V supply.                                                      |
| 10   | PA3 (HS) SPI_NSS/TIM2_CH3       | Reserved.                                                          |
| 11   | PB5 (T) I2C_SDA/TIM1_BKIN       | I2C SDA for DS3231 RTC with 10k pull-up resistor.                  |
| 12   | PB4 (T) I2C_SCL/ADC_ETR         | I2C SCL for DS3231 RTC with 10k pull-up resistor.                  |
| 13   | PC3 (HS) TIM1_CH3               | Drives LEDs (red) through 1k resistor.                             |
| 14   | PC4 (HS) TIM1_CH4               | Drives LEDs (blue) through 1k resistor.                            |
| 15   | PC5 (HS) SPI_SCK/TIM2_CH1       | Drives 595 shift register data input with 10k pull-up resistor.    |
| 16   | PC6 (HS) SPI_MOSI/TIM1_CH1      | Drives LEDs (green) through 1k resistor.                           |
| 17   | PC7 (HS) SPI_MISO/TIM1_CH2      | Drives 595 RCLK (latch) with 10k pull-up resistor.                 |
| 18   | PD1 (HS) SWIM                   | Debug connector pin 2.                                             |
| 19   | PD2 (HS) AIN3/TIM2_CH3          | Drives 595 SRCLK with 10k pull-up resistor.                        |
| 20   | PD3 (HS) AIN4/TIM2_CH2          | Drives 595 U4 OE.                                                  |

### Debug Connector
| Pin | Signal | Description |
|-----|--------|-------------|
| 1   | 3.3V   | Power supply. |
| 2   | SWIM   | Programming interface. |
| 3   | NRST   | Reset.       |
| 4   | GND    | Ground.      |

---

## Shift Register Wiring
The project uses a series of 74HC595 shift registers, driving 2003A darlington arrays to control the Nixie tubes. Here is the wiring layout:

- **Shift Registers (595)**: Connected in series to control multiple outputs.
- **Darlington Arrays (2003A)**: Drive the Nixie tubes from shift register outputs.

```
PC5
|
V
595 U4
Q0 -> 2003A U5  I1, O1-> N4 0
Q1 -> 2003A U5  I2, O2-> N4 1
Q2 -> 2003A U5  I3, O3-> N4 2
Q3 -> 2003A U5  I4, O4-> N4 3
Q4 -> 2003A U5  I5, O5-> N4 4
Q5 -> 2003A U5  I6, O6-> N4 5
Q6 -> 2003A U5  I7, O7-> N4 6
Q7 -> 2003A U6  I1, O1-> N4 7
|
V
595 U7
Q0 -> 2003A U6  I2, O2-> N4 8
Q1 -> 2003A U6  I3, O3-> N4 9
Q2 -> 2003A U6  I4, O4-> N3 0
Q3 -> 2003A U6  I5, O5-> N3 1
Q4 -> 2003A U6  I6, O6-> N3 2
Q5 -> 2003A U6  I7, O7-> N3 3
Q6 -> 2003A U8  I1, O1-> N3 4
Q7 -> 2003A U8  I2, O2-> N3 5
|
V
595 U9
Q0 -> 2003A U8  I3, O3-> N3 6
Q1 -> 2003A U8  I4, O4-> N3 7
Q2 -> 2003A U8  I5, O5-> N3 8
Q3 -> 2003A U8  I6, O6-> N3 9
Q4 -> 2003A U8  I7, O7-> N2 0
Q5 -> 2003A U10 I1, O1-> N2 1
Q6 -> 2003A U10 I2, O2-> N2 2
Q7 -> 2003A U10 I3, O3-> N2 3
|
V
595 U11
Q0 -> 2003A U10 I4, O4-> N2 4
Q1 -> 2003A U10 I5, O5-> N2 5
Q2 -> 2003A U10 I6, O6-> N2 6
Q3 -> 2003A U10 I7, O7-> N2 7
Q4 -> 2003A U12 I1, O1-> N2 8
Q5 -> 2003A U12 I2, O2-> N2 9
Q6 -> 2003A U12 I3, O3-> N1 0
Q7 -> 2003A U12 I4, O4-> N1 1
|
V
595 U13
Q0 -> 2003A U12 I5, O5-> N1 2
Q1 -> 2003A U12 I6, O6-> N1 3
Q2 -> 2003A U12 I7, O7-> N1 4
Q3 -> 2003A U14 I1, O1-> N1 5
Q4 -> 2003A U14 I2, O2-> N1 6
Q5 -> 2003A U14 I3, O3-> N1 7
Q6 -> 2003A U14 I4, O4-> N1 8
Q7 -> 2003A U14 I5, O5-> N1 9
```

---

## Usage
1. Clone the repository with submodules:
   ```bash
   git clone --recurse-submodules <repository_url>
   ```

 **When using COSMIC:**
 - Compile the firmware using Cosmic STM8 v4.6 toolchain (STVD project `in12v3clock.stp`, configuration Release).
 - Or use precompiled firmware file: `Release\in12v3clock.s19` 

 **When using SDCC:**
 - Windows: VS Code task `Build SDCC` or `python tools/build.py sdcc` (see below).
 - Linux / macOS: install `sdcc` (4.5 is tested, older versions may not fit into 8 KB), `make`, `binutils`, then in the project folder:
   - `make` — build to `build/sdcc/main.ihx` and `main.s19`, `make clean` — remove the build;
   - `make flash` — flash through ST-LINK V2 with [stm8flash](https://github.com/vdudouyt/stm8flash) (option bytes and EEPROM are not touched).
 - Or use the precompiled files (`main.s19` or `main.ihx`) located in the `SDCC` directory.

 **Command line build on Windows (no STVD):**
 - Requires Python 3 and installed Cosmic CXSTM8 and/or SDCC (default install paths, or set `COSMIC_DIR` / `SDCC_DIR`).
 - `python tools/build.py` builds with both compilers, `python tools/build.py cosmic` or `... sdcc` builds with one.
 - Results go to `build/cosmic/in12v3clock.s19` and `build/sdcc/main.s19`; the script prints ROM/RAM usage and fails if the firmware does not fit into 8 KB.
 - Note: in the STVD Release configuration the Cosmic optimizer is enabled (option `-no` removed), it saves about 540 bytes.
 - Build options: `python tools/build.py cosmic -DK1_DOUBLE_CLICK=0` (no double click of K1), `-DDIAG_RESET`
   (shows the reset cause at start), `-DAP_TEST` (test: the time runs 6 times faster, antipoisoning every minute).
 - `python tools/eedata.py` regenerates the default EEPROM file from the sources (after a change of the settings layout).
 - `python tools/flash.py [release] [sdcc] [data]` flashes `build/cosmic/in12v3clock.s19` through ST-LINK (SWIM) with `STVP_CmdLine`, option bytes are not touched; `data` also writes the default EEPROM file; `release` flashes the precompiled `Release/in12v3clock.s19` (no compiler needed).
2. Flash the firmware, see [Flashing](#flashing).
3. Verify the board's functionality.

## Flashing
Only flashing the precompiled firmware: see [Quick Start](#quick-start-flash-the-precompiled-firmware-no-build).

The STM8S003F3 has no UART bootloader, it is flashed only through **SWIM** with an **ST-LINK V2** (an original or a cheap clone).

**Connection** — the debug connector of the board (see [Debug Connector](#debug-connector)):
- ST-LINK `SWIM` -> pin 2, `RST` -> pin 3, `GND` -> pin 4;
- power the clock from its own power supply. Pin 1 (3.3V): an original ST-LINK only measures the target voltage there,
  a clone outputs 3.3V — with the clock powered by its supply it is safer not to connect it.

![SWD pinout](Photo/swd_pinout.jpg)

The pinout of your ST-LINK: [Wiki: ST-Link Pinout](https://wiki.cuvoodoo.info/doku.php?id=jtag)

**Option byte `AFR0` (once for a new chip):** the backlight green channel is TIM1_CH1 on PC6, it is available only
with the alternate function remapping `AFR0` (Port C5, C6, C7 — alternate function). Without it the green LEDs do not work.
The file `in12v3clock_Programmer/in12v3clock_STVP_OPTION BYTE.hex` has this setting, the scripts below never write the option bytes.

**Files:**
- firmware: `Release/in12v3clock.s19` (precompiled, Cosmic) or your build `build/cosmic/in12v3clock.s19`;
  SDCC: `SDCC/main.s19` / `build/sdcc/main.s19`;
- default settings (EEPROM): `in12v3clock_Programmer/in12v3clock_STVP_data.s19` (`.hex`). Not needed: the firmware writes
  the default settings itself if the EEPROM is empty or has an old layout.

### Windows: ST Visual Programmer (STVP), GUI
Install [STVP](https://www.st.com/en/development-tools/stvp-stm8.html) (it also installs the ST-LINK driver).
1. Open the project `in12v3clock_Programmer/in12v3clock_STVP.stp` (`File > Open Project`). It is set for ST-LINK, SWIM,
   STM8S003F3 and loads the files: PROGRAM MEMORY — `Release/in12v3clock.s19`, DATA MEMORY — the default settings,
   OPTION BYTE — `AFR0`. If a file is not found, load it by `File > Open...` on its tab.
   Without the project: `Configure > Configure ST Visual Programmer`: Hardware `ST-LINK`, Port `USB`, Programming mode `SWIM`,
   Device `STM8S003F3`; then load the files on the tabs.
2. A new chip: tab **OPTION BYTE**, check that `AFR0` is `Port C5... Alternate Active`, then `Program > Current tab`.
3. Tab **PROGRAM MEMORY**: `Program > Current tab` (Ctrl+P). The verify runs after the programming.
4. Tab **DATA MEMORY** (optional, resets the settings to the defaults): `Program > Current tab`.

`Program > All tabs` writes all three at once (the settings are reset).

### Windows: command line or VS Code
`python tools/flash.py [release] [sdcc] [data]` or the VS Code task `Flash` (see [Working in VS Code](#working-in-vs-code)).
It uses `STVP_CmdLine.exe` from STVP, writes the program memory (and the default EEPROM with `data`), verifies it,
never touches the option bytes. Set `AFR0` once with the STVP GUI.

### Linux / macOS: stm8flash
[stm8flash](https://github.com/vdudouyt/stm8flash) works with ST-LINK V2 (on Linux add the udev rule from its README
or run with `sudo`):
```bash
make flash                                                   # build/sdcc/main.ihx, the same as:
stm8flash -c stlinkv2 -p stm8s003f3 -s flash -w build/sdcc/main.ihx
stm8flash -c stlinkv2 -p stm8s003f3 -s flash -w Release/in12v3clock.s19          # the precompiled Cosmic firmware
stm8flash -c stlinkv2 -p stm8s003f3 -s eeprom -w in12v3clock_Programmer/in12v3clock_STVP_data.hex   # settings reset (optional)
# option bytes, once for a new chip: OPT0..NOPT5 with AFR0 = 1 (OPT2 = 0x01, NOPT2 = 0xFE)
printf '\x00\x00\xff\x01\xfe\x00\xff\x00\xff\x00\xff' > opt.bin
stm8flash -c stlinkv2 -p stm8s003f3 -s opt -w opt.bin
```
The stm8flash commands are not tested by the author on this board (the Windows way is).

---

# **Clock Control and Setup Guide**

<details>
<summary>Show Full Instruction</summary>

## **Key Definitions:**
- **K1** — Upper button.
- **K2** — Lower button.

---

## **Basic Control:**
1. **Single press of K1** — Shows the number of the current minute change effect (menu item 4.1) in the left tube.
   Press again while the number is shown to switch to the next effect. 1.5 s after the last press the time appears with this effect.
   **Double press of K1** — The same for the cathode poisoning prevention effect (menu item 4.2), the number is in the right tube.
   The single press reacts 0.4 s after the release (waiting for the second press). The double press can be switched off
   in the firmware: `K1_DOUBLE_CLICK 0` in `keyevents.h` (then the single press reacts at once).
2. **Single press of K2** — Switches the RGB backlight on/off for the current interval (day or night).  
   Day and night have their own on/off state and brightness (menu items 2.3 and 2.4).  
   If the backlight brightness of the current interval is 0, the backlight blinks 3 times and stays off.
3. **Hold K1 (> 2 seconds)** — Activates time setup mode.
4. **Hold K2 (> 2 seconds)** — Activates backlight color adjustment mode.
5. **Hold K1 and K2 simultaneously (> 2 seconds)** — Activates the settings menu.

---

## **Time Setup Mode:**
1. **Entering the mode:** Hold K1 for more than 2 seconds.
2. **Hour adjustment:**
   - The hour digits start blinking.
   - **K1:** Increases the hour value by 1.
     - Holding K1 (> 0.8 seconds): Continuously increases the hour value.
   - **K2:** Switches to minute adjustment.
3. **Minute adjustment:**
   - The minute digits start blinking.
   - **K1:** Increases the minute value by 1.
     - Holding K1 (> 0.8 seconds): Continuously increases the minute value.
   - **K2:** Saves the time (seconds reset to zero) and exits the setup mode.

In the 12-hour format the hours are set as 12, 1…11 (AM, the colon is on), then 12, 1…11 (PM, the colon blinks fast).

If the RTC battery was empty or removed while the clock was off (DS3231 oscillator stop flag),
the clock starts at 00:00 — set the time again.

---

## **Backlight Color Adjustment Mode:**
1. **Entering the mode:** Hold K2 for more than 2 seconds.
2. **Adjustment sequence:**
   - **Red (digit "1" blinks).**
   - **Green (digit "2" blinks).**
   - **Blue (digit "3" blinks).**
3. **Brightness adjustment:**
   - **K1:** Increases brightness by 1 (range: 0–255).
     - Holding K1 (> 0.8 seconds): Continuously increases brightness.
   - **K2:** Saves the current value and proceeds to the next color.
4. **Exiting the mode:** After adjusting blue brightness, the mode exits automatically.

---

## **Settings Menu:**
1. **Entering the menu:** Hold K1 and K2 simultaneously for more than 2 seconds.
2. **Display:** the item number "group item" in the two left tubes (at full brightness), the colon, the value in the two right tubes.
   The night start and end: the item number for 0.8 s, then the time `HH:MM`, the edited part blinks.
3. **Control:**
   - **K1:** Changes the value (hold for auto repeat for the values above 9).
   - **K2 (short):** Saves the item and moves to the next one, after the last one — exit.
   - **K2 (hold 1 s):** Saves the item and exits the menu.
   - Without key presses the menu (and the time and color setup) is left after 60 seconds, not saved values are dropped.

   In the time display the brightness changes smoothly (day/night, K2): digits and backlight 1% every 50 ms, the backlight switched by K2 4% every 50 ms.
4. ## Menu Items:  

    **1. Display**  
    **1.1** Leading zero of the hours: `0` — blank (` 7:05`), `1` — shown (`07:05`)  
    **1.2** Time format: `0` — 24-hour, `1` — 12-hour (no AM/PM on the time display; in the time setup PM is shown by the fast blinking colon; the night interval is always in 24-hour format)  
    **1.3** Colon (shown while the item is selected):  
         `0` — Pulse every second  
         `1` — Always on (at the digits brightness)  
         `2` — Always off  
         `3` — Slow pulse: lights up smoothly (0.5 s), stays on (0.5 s), goes out smoothly (0.5 s), stays off (0.5 s)  
         `4` — Blinking: 0.5 s on, 0.5 s off  
         `5` — Slow blinking: 1 s on, 1 s off  

    **2. Brightness** (counts down 99 → 0 and shows the brightness being set; `99` is the full brightness, 100%)  
    **2.1** Digits, day: `5–99%`  
    **2.2** Digits, night: `0, 5–99%` (`0` — digits and colon are off at night)  
    **2.3** Backlight, day: `0–99%`, perceptually linear (CIE lightness)  
    **2.4** Backlight, night: `0–99%`  
    **2.5** Digits PWM frequency: `0` — 100 Hz (default), `1` — 200 Hz, `2` — 400 Hz (experimental: at a low brightness some cathodes may not glow fully); shown at the night brightness to see the flicker  

    **3. Night**  
    **3.1** Night interval: `0` — Off (day settings all the time), `1` — On  
    **3.2** Night start `HH:MM` (24-hour)  
    **3.3** Night end `HH:MM`  

    **4. Effects**  
    **4.1** Minute change effect (only the changed digits, except train and elastic):  
         `0` — None  
         `1` — Fade out and in  
         `2` — Roll down by numbers  
         `3` — Roll through the cathodes in their order in the tube (front to back)  
         `4` — Train: the digits go out to the right, the new ones come from the left  
         `5` — Elastic: the digits fly out to the right one by one, the new ones fly in  
    **4.2** Cathode poisoning prevention effect (every digit goes through all 10 cathodes):  
         `0` — Wave: the digits roll down one after another from the shown digits to the time  
         `1` — Wave through the cathodes in their order in the tube  
         `2` — Noise: all digits roll fast together (0.5 s)  
    **4.3** Cathode poisoning prevention interval, day: `0` — Off, `1–30` minutes  
    **4.4** Cathode poisoning prevention interval, night: `0` — Off, `1–30` minutes (not done if the night digits brightness is 0)  
    **4.5** Glitches: `0` — Off, `1` — On (every 30–120 s a random tube flickers a few times)  

</details>

---

## License
This project is distributed under the **MIT License**.
