
# IN12v3clock

Alternate firmware for the GeekStyles IN12 V3 Nixie nixieclock PCB.

## Version
- **1.00**: Initial release.

## Description
This firmware is designed for the IN12 V3 clock PCB, 
with frontside inscription: **GeekStyles IN12 V3**  
[![Frontside Preview](Photo/pcb_front_preview.JPG)](Photo/pcb_front_fullsize.JPG)
and the backside inscription: **JM NixieClock IN12 V4.2 20190514**.   
[![Backside Preview](Photo/pcb_back_preview.JPG)](Photo/pcb_back_fullsize.JPG)
The STM8S003F3P6 microcontroller features 8 KB of Flash memory, 1 KB of RAM, and 128 bytes of EEPROM.

---

## Requirements
1. **Cosmic STM8 compiller + STVD**
2. Alternate is the **SDCC compiller** (the binary file is slightly larger compared to COSMIC).
3. **External Libraries**:
- Added submodule: [STM8_headers](https://github.com/gicking/STM8_headers).
  
---

## Important Configuration
Before flashing the firmware, set the option byte `AFR0` in the ST Visual Programmer:
- **Port C5, C6 & C7**: Configure to **Alternate Function**.

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
 - Compile the firmware using Cosmic STM8 v4.6 toolchain. 
 - Or use precompiled firmware file: `Release\in12v3clock.s19` 

 **When using SDCC:**
 - Install Docker.
 - Open Windows Terminal.
 - Run the `run.cmd` file.
 - Run command `make clean` in the terminal window.
 - Run command `make` in the terminal window.
 - Or use the precompiled files (`main.s19` or `main.ihx`) located in the `SDCC` directory.

 **Command line build (no STVD, no Docker):**
 - Requires Python 3 and installed Cosmic CXSTM8 and/or SDCC (default install paths, or set `COSMIC_DIR` / `SDCC_DIR`).
 - `python tools/build.py` builds with both compilers, `python tools/build.py cosmic` or `... sdcc` builds with one.
 - Results go to `build/cosmic/in12v3clock.s19` and `build/sdcc/main.s19`; the script prints ROM/RAM usage and fails if the firmware does not fit into 8 KB.
 - Note: in the STVD Release configuration the Cosmic optimizer is enabled (option `-no` removed), it saves about 540 bytes.
 - `python tools/flash.py [data]` flashes `build/cosmic/in12v3clock.s19` through ST-LINK (SWIM) with `STVP_CmdLine`, option bytes are not touched; `data` also writes the default EEPROM file.
2. Flash the firmware to the STM8S003F3P6 microcontroller.
- The `in12v3clock_Programmer` directory contains a project for the STVP programmer with a default EEPROM file and a default OPTION BYTE file.

 - SWD pinout:
 
 ![SWD pinout](Photo/swd_pinout.jpg)


 Here you can find the pinout of your ST-Link programmer:  
 [Wiki: ST-Link Pinout](https://wiki.cuvoodoo.info/doku.php?id=jtag)

3. Verify the board's functionality.

---

# **Clock Control and Setup Guide**

<details>
<summary>Show Full Instruction</summary>

## **Key Definitions:**
- **K1** — Upper button.
- **K2** — Lower button.

---

## **Basic Control:**
1. **Single press of K1** — Switches to the next minute change effect (menu item 4.1): its number is shown in the left tube,
   1.5 s after the last press the time appears with this effect.
   **Double press of K1** — Switches to the next cathode poisoning prevention effect (menu item 4.2), its number is shown
   in the right tube, then it runs the same way. Press again during the 1.5 s to go on without the demo.
   The single press reacts 0.4 s after the release (waiting for the second press).
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
