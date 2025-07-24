#include <stdint.h>
#include "keyevents.h"
#include "macro.h"
#include "keyboard.h"
#include "display.h"
#include "iface.h"
#include "ds3231.h"
#include "eeprom.h"

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
extern Iface i;

void keyevents_init ( void )
{

}

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
			i.display_state = SETUP_NO;
		}
	}
	
	longpress_key1();
	longpress_key2();
	longpress_key1_2();
}

void event_single_key1(void)
{
	switch(i.display_state){
		case SETUP_NO:
				//if (!nixie_antipoison_is_active(&ap)) {
				apStart();
				//}
			break;
		case SETUP_HOURS:
			iface_flag05sReset();
			++i.hoursSetupValue;
			if ((i.hoursSetupValue&0x0F) > 0x09){i.hoursSetupValue +=0x06;}
			if(e.f1224){
				if (i.hoursSetupValue > 0x12) 			{i.hoursSetupValue = 0;}
			}else{
				if (i.hoursSetupValue > 0x23) 			{i.hoursSetupValue = 0;}
			}
			break;
		
		case SETUP_MINUTES:
			iface_flag05sReset();
			++i.minutesSetupValue;
			if ((i.minutesSetupValue&0x0F) > 0x09){i.minutesSetupValue +=0x06;}
			if (i.minutesSetupValue > 0x59) 			{i.minutesSetupValue = 0;}
			break;
		
		case SETUP_R:
			++i.setupValue;
			displayRset(i.setupValue);
			break;
		
		case SETUP_G:
			++i.setupValue;
			displayGset(i.setupValue);
			break;
		
		case SETUP_B:
			++i.setupValue;
			displayBset(i.setupValue);
			break;
			
		case	SETUP_F1224:
			i.setupValue = 0;	//todo disable, 12h time display is not working yet!
			break;
			
		case	SETUP_ZERO:
		case  SETUP_NIGHT_BR_EN:
		case	SETUP_NIGHT_RGB_EN:
		case  SETUP_ANTIPOISONING_AT_NIGHT_ONLY:
			if (i.setupValue){
				i.setupValue = 0;
			}else{
				i.setupValue = 1;
			}
			break;
		
		case SETUP_COLON_BLINKING_TYPE:
			i.setupValue++;
			if (i.setupValue > 2){
				i.setupValue = 0;
			}
			break;

		case SETUP_BRIGHT:
		case SETUP_NIGHT_BR:
			i.setupValue--;
			if (i.setupValue > 100){
				i.setupValue = 100;
			}
			if (i.setupValue < 5){
				i.setupValue = 0;
			}
			
			if (i.setupValue > 4){
				displaySetBright(i.setupValue);
			}
			break;
		
		case SETUP_NIGHT_BR_START_H:
		case SETUP_NIGHT_BR_END_H:
			++i.setupValue;
			if(e.f1224){
				if (i.setupValue > 12)			{i.setupValue=0;}
			}else{
				if (i.setupValue > 23)			{i.setupValue=0;}
			}
			break;
		
		case SETUP_NIGHT_BR_START_M:
		case SETUP_NIGHT_BR_END_M:
			++i.setupValue;
			if (i.setupValue > 59) 				{i.setupValue = 0;}
			break;
		
		default:
			break;
	}
	if (i.display_state!=SETUP_NO){
		i.timeSetupCounter = 60*10;
	}
}


void event_single_key2(void)
{
	uint8_t seconds = 0;
	
	switch (i.display_state){
		case SETUP_HOURS:
			i.display_state = SETUP_MINUTES;
			break;
			
		case SETUP_MINUTES:
			ds3231_write_time(&seconds,&i.minutesSetupValue,&i.hoursSetupValue);
			i.display_state = SETUP_NO;
			bitmaskclr(ki.bf,S2_M);	//disable fast release key2 to prevent toggle RGB
			break;
		
		case SETUP_R:
			i.display_state++;
			EEPROM_writeByte(R_ADDR,i.setupValue);
			i.setupValue = EEPROM_readByte(G_ADDR);
			break;
		
		case SETUP_G:
			i.display_state++;
			EEPROM_writeByte(G_ADDR,i.setupValue);
			i.setupValue = EEPROM_readByte(B_ADDR);
			break;
		
		case SETUP_B:
			i.display_state = SETUP_NO;
			EEPROM_writeByte(B_ADDR,i.setupValue);
			bitmaskclr(ki.bf,S2_M);	//disable fast release key2 to prevent toggle RGB
			break;
		
		case SETUP_ZERO:
			e.zeroEn = i.setupValue;
			EEPROM_writeByte(ZERO_ADDR,e.zeroEn);
			i.display_state++;
			i.setupValue = e.f1224;
			break;
		
		case SETUP_F1224:
			e.f1224 = i.setupValue;
			EEPROM_writeByte(F1224_ADDR,e.f1224);
			i.display_state++;
			i.setupValue = e.bright;
			displaySetBright(i.setupValue);
			break;
		
		case SETUP_BRIGHT:
			i.display_state++;
			e.bright = i.setupValue;
			EEPROM_writeByte(BRIGHT_ADDR,e.bright);
			i.setupValue = e.nBright;
			displaySetBright(i.setupValue);
			break;
		
		case SETUP_NIGHT_BR:
			e.nBright = i.setupValue;
			EEPROM_writeByte(NIGHT_BR_ADDR,e.nBright);
			i.display_state++;
			i.setupValue = e.nBrightEn;
			displaySetBright(100);
			break;
		
		case SETUP_NIGHT_BR_EN:
			e.nBrightEn = i.setupValue;
			EEPROM_writeByte(NIGHT_BR_EN_ADDR,e.nBrightEn);
			i.display_state++;
			i.setupValue = e.nBrightStartH;
			break;
		
		case SETUP_NIGHT_BR_START_H:
			e.nBrightStartH = i.setupValue;
			EEPROM_writeByte(NIGHT_BR_START_H_ADDR,e.nBrightStartH);
			i.display_state++;
			i.setupValue = e.nBrightStartM;
			break;
		
		case SETUP_NIGHT_BR_START_M:
			e.nBrightStartM = i.setupValue;
			EEPROM_writeByte(NIGHT_BR_START_M_ADDR,e.nBrightStartM);
			i.display_state++;
			i.setupValue = e.nBrightEndH;
			break;
		
		case SETUP_NIGHT_BR_END_H:
			e.nBrightEndH = i.setupValue;
			EEPROM_writeByte(NIGHT_BR_STOP_H_ADDR,e.nBrightEndH);
			i.display_state++;
			i.setupValue = e.nBrightEndM;
			break;
		
		case SETUP_NIGHT_BR_END_M:
			e.nBrightEndM = i.setupValue;
			EEPROM_writeByte(NIGHT_BR_STOP_M_ADDR,e.nBrightEndM);
			i.display_state++;
			i.setupValue = e.rgbAtNightEn;
			break;
		
		case SETUP_NIGHT_RGB_EN:
			e.rgbAtNightEn = i.setupValue;
			EEPROM_writeByte(NIGHT_RGB_EN_ADDR,e.rgbAtNightEn);
			i.display_state++;
			i.setupValue = e.antipoisoningAtNihgtOnly;
			break;
		
		case SETUP_ANTIPOISONING_AT_NIGHT_ONLY:
			e.antipoisoningAtNihgtOnly = i.setupValue;
			EEPROM_writeByte(ANTIPOISONING_AT_NIGHT_ONLY_ADDR,e.antipoisoningAtNihgtOnly);
			i.display_state++;
			i.setupValue = e.colonBlinkingType;
			break;

		case SETUP_COLON_BLINKING_TYPE:
			e.colonBlinkingType = i.setupValue;
			EEPROM_writeByte(COLON_BLINKING_TYPE,e.colonBlinkingType);
			i.display_state = SETUP_NO;
			bitmaskclr(ki.bf,S2_M);	//disable fast release key2 to prevent toggle RGB
			break;

		default:
			i.display_state = SETUP_NO;
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
				displaySetBright(100);
				bitmaskclr(ki.bf,S1_M);
			}
		}else if (i.display_state > SETUP_ZERO && i.display_state < SETUP_NO){
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
				i.setupValue = EEPROM_readByte(R_ADDR);
				displayRset(EEPROM_readByte(R_ADDR));
				displayGset(EEPROM_readByte(G_ADDR));
				displayBset(EEPROM_readByte(B_ADDR));
				i.timeSetupCounter = 60*10;
				displaySetBright(100);
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
			displayRGBset(0);
			displaySetBright(100);
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
