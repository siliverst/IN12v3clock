#include <stdint.h>
#include "keyevents.h"
#include "macro.h"
#include "keyboard.h"
#include "display.h"
#include "iface.h"
#include "ds3231.h"
#include "eeprom.h"
#include "rgb.h"

/*
longpress_keyX		-	in keyivents_counters
fastrelease_keyX 	-	in processing of button release
ivent_single_keyX - this is what the button does in longpress or fastrelease
*/

void event_single_key1(void);
void event_single_key2(void);
void longpress_key1(void);
void longpress_key2(void);
void longpress_key1_2(void);
void fastReleaseKey2(void);
void fastReleaseKey1(void);


Keyivents ki;

void keyevents_proc ( void )
{
	ki.keys_mirror = keyboard_get_key();

	if(ki.keys_mirror != 0xFF){
		//---------------------------------------------------------------------------------------------------------------
		// Button 1 is pressed
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == S1_DOWN)
		{
			bitmaskset(ki.bf,S1_M);
			if (ki.bf == S1_M){
				if (i.display_state != SETUP_NO){ 
					event_single_key1();
				}
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 2 is pressed
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == S2_DOWN)
		{
			bitmaskset(ki.bf,S2_M);
			if (ki.bf == S2_M){
				event_single_key2();
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 1 is released
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == S1_UP)
		{
			if (bitchk(ki.bf,S1)){
				fastReleaseKey1();
				bitmaskclr(ki.bf,S1_M);
			}
		}
		//---------------------------------------------------------------------------------------------------------------
		// Button 2 is released
		//---------------------------------------------------------------------------------------------------------------
		if (ki.keys_mirror == S2_UP)
		{
			if (bitchk(ki.bf,S2)){
				fastReleaseKey2();
				bitmaskclr(ki.bf,S2_M);
			}
		}
	}
}

void keyevents_counters ( void )
{
	uint8_t t;
	
	//The counters count the time the button is pressed, tick 1/10s.
	for(t=0; t<sizeof(ki.lp_counter); t++){
		if (bitchk(ki.bf,t)){
			if (ki.lp_counter[t]<255) ki.lp_counter[t]++;
			if (ki.kr_counter[t]<255) ki.kr_counter[t]++;
		} else {
			ki.lp_counter[t]=0;
			ki.kr_counter[t]=0;
		}
	}
	if (i.timeSetupCounter){
		i.timeSetupCounter--;
		if (!i.timeSetupCounter){
			iface_exit_setup();
		}
	}
	
	longpress_key1();
	longpress_key2();
	longpress_key1_2();
}

void event_single_key1(void)
{
	uint8_t s = i.display_state;
	uint8_t v = i.setupValue;
	
	switch(s){
		case SETUP_NO:
			apStart();
			break;
		
		case SETUP_HOURS:
			iface_flag05sReset();
			++i.hoursSetupValue;
			if ((i.hoursSetupValue&0x0F) > 0x09){i.hoursSetupValue +=0x06;}
			if (i.hoursSetupValue > 0x23) 			{i.hoursSetupValue = 0;}
			break;
		
		case SETUP_MINUTES:
			iface_flag05sReset();
			++i.minutesSetupValue;
			if ((i.minutesSetupValue&0x0F) > 0x09){i.minutesSetupValue +=0x06;}
			if (i.minutesSetupValue > 0x59) 			{i.minutesSetupValue = 0;}
			break;
		
		case SETUP_R:
		case SETUP_G:
		case SETUP_B:
			// the color is changed in e only, it is saved by K2 or reloaded from EEPROM by timeout
			++EE(R_ADDR + s - SETUP_R);
			RGBset(100);
			break;
		
		case SETUP_BRIGHT:
		case SETUP_NIGHT_BR:
		case SETUP_RGB_BR:
		case SETUP_RGB_NIGHT_BR:
			// brightness goes down: 100..0 (digits: 100..5 day, 100..5,0 night) and again from 100
			if (--v > 100){
				v = 100;
			}else if ((v < BRIGHT_MIN) && (s <= SETUP_NIGHT_BR)){
				v = (s == SETUP_NIGHT_BR) ? 0 : 100;
			}
			i.setupValue = v;
			iface_menu_preview();
			break;
		
		default:
			// other settings menu items: 0..max
			if (s <= SETUP_MENU_LAST){
				if (++v > eeMax[s]) {v = 0;}
				i.setupValue = v;
			}
			break;
	}
	if (i.display_state!=SETUP_NO){
		i.timeSetupCounter = 60*10;
	}
}


void event_single_key2(void)
{
	uint8_t seconds = 0;
	uint8_t s = i.display_state;
	
	switch (s){
		case SETUP_NO:
			break;
		
		case SETUP_HOURS:
			i.display_state = SETUP_MINUTES;
			break;
			
		case SETUP_MINUTES:
			ds3231_write_time(&seconds,&i.minutesSetupValue,&i.hoursSetupValue);
			i.minutes = i.minutesSetupValue;
			i.hours = i.hoursSetupValue;
			iface_exit_setup();
			bitmaskclr(ki.bf,S2_M);	//disable fast release key2 to prevent toggle RGB
			break;
		
		case SETUP_R:
		case SETUP_G:
			EEPROM_set(R_ADDR + s - SETUP_R, EE(R_ADDR + s - SETUP_R));
			i.display_state++;
			break;
		
		case SETUP_B:
			EEPROM_set(B_ADDR, e.b);
			iface_exit_setup();
			bitmaskclr(ki.bf,S2_M);	//disable fast release key2 to prevent toggle RGB
			break;
		
		default:
			// settings menu: save the value and go to the next item
			if (s > SETUP_MENU_LAST){
				iface_exit_setup();
				break;
			}
			EEPROM_set(s, i.setupValue);
			if (s == SETUP_MENU_LAST){
				iface_exit_setup();
				bitmaskclr(ki.bf,S2_M);	//disable fast release key2 to prevent toggle RGB
			}else{
				i.display_state = ++s;
				i.setupValue = EE(s);
				iface_menu_preview();
			}
			break;
	}
}

void longpress_key1(void)
{
	if (ki.bf == S1_M){
		if (i.display_state == SETUP_NO){
			if (ki.lp_counter[S1] >= 2*10){
				i.display_state = SETUP_HOURS;
				i.timeSetupCounter = 60*10;
				i.hoursSetupValue = i.hours;
				i.minutesSetupValue = i.minutes;
				iface_show(100, IFACE_RGB_KEEP);
				bitmaskclr(ki.bf,S1_M);
			}
		}else if (i.display_state > SETUP_MENU_LAST || eeMax[i.display_state] > 9){
			// key repeat for values, not for on/off items
			if (ki.lp_counter[S1] >= 18){
				if(ki.kr_counter[S1] >= 1){
					ki.kr_counter[S1] = 0;
					event_single_key1 ();
				}
			}else if (ki.lp_counter[S1] >= 8){
				if(ki.kr_counter[S1] >= 2){
					ki.kr_counter[S1] = 0;
					event_single_key1 ();
				}
			}
		}
	}
}

void longpress_key2(void)
{
	if (ki.bf == S2_M){
		if (i.display_state == SETUP_NO){
			if (ki.lp_counter[S2] >= 2*10){
				i.display_state = SETUP_R;
				i.timeSetupCounter = 60*10;
				iface_show(100, 100);
				bitmaskclr(ki.bf,S2_M);
			}
		}
	}
}

void longpress_key1_2(void){
	if ((ki.lp_counter[S1] >= 2*10)&&(ki.lp_counter[S2] >= 2*10)){
		if (i.display_state == SETUP_NO){
			i.display_state = SETUP_ZERO;
			i.setupValue = e.zeroEn;
			i.timeSetupCounter = 60*10;
			iface_menu_preview();
		}
		bitmaskclr(ki.bf,(S1_M+S2_M));
	}
}

void fastReleaseKey1(void)
{
	if (ki.bf == S1_M){
		if (ki.lp_counter[S1] < 1*10){
			if (i.display_state == SETUP_NO){
				event_single_key1();
			}
		}
	}
}

void fastReleaseKey2(void)
{
	if (ki.bf == S2_M){
		if (ki.lp_counter[S2] < 1*10){
			if (i.display_state == SETUP_NO){
				RGBtoggle();
			}
		}
	}
}
