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
 UP BUTTON   - change effects
 DOWN BUTTON - on/off RBG backlight
 
 
 todo a function that monitors all intervals; each interval is checked once in the main loop.
			After an interval is checked, the corresponding global flag is set, which is then checked in the interface.
 todo separate brightness for RGB and Clock + dot, all with the ability to disable brightness. All with their own intervals?
 todo add other dot blinking stiles (0.5s, 1s, pwm and on|off).
 todo comments on english
 todo add 12/24
 todo add leading zero off.
 todo i2c delay func to save ROM space
 todo if there is enough ROM - gamma compensation for RGB.
 todo refactor the antipoison algorithm.
 todo simplify the backligt on/off
 todo add backlight bright for night and for normal mode
 todo add disable for the bright for the night mode.
 todo independed antipoisonig intervals for day and for night. 
			Maybe if night bright is off - day interval automatically set to 3 min.
 todo replace nBrightEn with if (nBright < 100)
 todo make a beautiful animation of the transition from night to day interval and vice versa. 
      Now is not beautiful with antipoisonin.
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
