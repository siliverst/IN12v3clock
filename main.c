#include "STM8S003F3.h"
#include "display.h"
#include "ds3231.h"
#include "macro.h"
#include "service_timing.h"
#include "keyboard.h"
#include "iface.h"
#include "eeprom.h"
#include "iwdg.h"

/*
Alternative firmware for the GeekStyles IN12 V3 nixie clock (STM8S003F3, 8KB flash).
Modules:
  display.c         - digits PWM by reloading the 74HC595 chain from the TIM2 ISRs, colon PWM
  hc595.c           - shift out to the 595 chain (assembler, the shortest digit pulse depends on it)
  effects.c         - digits in the time display: minute change effects, antipoisoning, glitches
  iface.c           - time display, day/night mode, smooth brightness, setup modes and the settings menu
  keyevents.c       - key actions, keyboard.c - key scanning and debounce
  RGB.c             - backlight PWM (TIM1) with the CIE lightness table
  ds3231.c, i2c.c   - RTC on the software I2C
  eeprom.c          - settings, their limits and defaults
  service_timing.c  - 10/20/50/100ms tasks of the main loop, TIM2 gives the 10ms tick
See README.md.
*/

int main(void) {
	
  DISABLE_INTERRUPTS();
	// switch to 16MHz (default is 2MHz)
  sfr_CLK.CKDIVR.byte = 0x00;
	
	EEPROM_dataInit();
	displayInit();
	ds3231Init();
	keyboard_init();
	iface_init();
	iwdg_init(0xFF);	//1.02s period
	iwdg_service();
  ENABLE_INTERRUPTS(); 

#ifdef DIAG_RESET
	// diagnostics: show the reset cause for 2.5s at start: "8 _ x x", RST_SR:
	// 0 - power on/brown-out, 1 - WWDG, 2 - IWDG, 4 - illegal opcode, 8 - SWIM, 16 - EMC
	{
		uint8_t d[4], n = 0;
		
		d[0] = sfr_ITC.RST_SR.byte;
		sfr_ITC.RST_SR.byte = 0x1F;		// the flags are cleared by writing 1
		d[1] = d[0] / 10;
		d[0] = d[0] % 10;
		d[2] = NIXIE_OFF;
		d[3] = 8;
		displaySetBright(100);
		displayNixie(d, 0);
		while (n < 250){
			if (flag10ms) {flag10ms = 0; n++;}
			iwdg_service();
		}
	}
#endif

	
	while (1) {
		service_timing_proc();
		iwdg_service();
	}
}
