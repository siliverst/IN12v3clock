#include "STM8S003F3.h"
#include "service_timing.h"
#include "display.h"
#include "keyboard.h"
#include "keyevents.h"
#include "iface.h"

/*
all periodic tasks, called from the main loop.
flag10ms is set by the display timer (TIM2 update, 100Hz). If the loop is late
for more than 10ms (EEPROM write), the tick is lost, the clock itself is in the RTC.
*/
void service_timing_proc (void)
{
	static uint8_t counter20ms;
	static uint8_t counter50ms;
	static uint8_t counter100ms;
	
	if (!flag10ms) {return;}
	flag10ms = 0;
	
	if (i.display_state == SETUP_NO) {displayDotPulseProc();}		// setup modes drive the dot themselves
	if (++counter20ms >= 2){
		counter20ms = 0;
		keyboard_refresh();
	}
	keyevents_proc();
	if (++counter50ms >= 5){
		counter50ms = 0;
		iface_50ms_proc();
	}
	if (++counter100ms >= 10){
		counter100ms = 0;
		iface_100ms_proc();
	}
}
