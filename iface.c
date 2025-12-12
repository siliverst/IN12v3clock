#include <stdint.h>
#include "iface.h"
#include "keyevents.h"
#include "service_timing.h"
#include "ds3231.h"
#include "display.h"
#include "macro.h"
#include "eeprom.h"
#include <string.h>

Iface i;
antipoisoning ap;

static void timer50msProc(void);
static void iface_display(void);
static void iface_disp2decDigit (uint8_t value, uint8_t *d0, uint8_t *d1);
static void iface_disp3decDigit (uint16_t value, uint8_t *d0, uint8_t *d1, uint8_t *d2);
static void iface_disp2bcdDigit (uint8_t value, uint8_t *d0, uint8_t *d1);

static void apCounterProc(void);
static uint8_t apLoop(uint8_t dig);
static void apProc (void);
static void apUpdateTime (void);
static uint8_t apInWork (void);

static uint8_t dot_dis;	//dirty hack

void iface_init( void )
{
	i.display_state = SETUP_NO;
	ds3231_read_time(&i.seconds,&i.minutes,&i.hours);
	memset(ap.flagEn,0,sizeof(ap.flagEn));
	apUpdateTime();	// two calls toupdate the digsNew array;
	apUpdateTime();
	//displayRGBset(0);
	dot_dis = 0;
}

void iface_proc ( void ) 
{
	static uint8_t proc_fase = 0;
	static uint8_t counter50ms = 0;
	static uint8_t counter100ms = 0;
	static uint8_t localFlag100ms = 0;
	
	keyevents_proc();
	
	if (i.flag10ms) {
		set_service_timing_mutex();
		proc_fase++;
		switch(proc_fase){
			case 1: 
				// counter for time get time from ds3231 and start blinking dot if needed
				if (++counter50ms> 5-1){
					counter50ms = 0;
					timer50msProc();
				}
				break;
			
			case 2:
				if (++counter100ms> 10-1){
					counter100ms = 0;
					localFlag100ms = 1;	//flag for pocess 100ms tasks
					
					//counter for blinking digits while time setup
					if (++i.counter1s > 10-1){
						i.counter1s = 0;
						i.flag05s = 0;
					}
					if (i.counter1s > 4){
						i.flag05s = 1;
					}
					
					//counter for blinking digits while RGB setup
					if (i.flag100ms){
						i.flag100ms = 0;
					} else {
						i.flag100ms = 1;
					}
				}
				break;
			
			case 3:
				//counter for antipoisoning
				apCounterProc();
				break;
			
			case 4:
				if (localFlag100ms){
					keyevents_counters();
				}
				break;
			
			case 5:
				if (localFlag100ms) {
					iface_display();
					localFlag100ms = 0;
				}
				proc_fase = 0;
				i.flag10ms = 0;
				reset_service_timing_mutex();
				break;
			
			default:
				proc_fase = 0;
				i.flag10ms = 0;
				reset_service_timing_mutex();
				break;
		}
	}
}

void iface_flag05sReset(void)
{
	i.counter1s = 0;
	i.flag05s = 0;
}

void iface_10ms_proc_en (void)
{
	i.flag10ms = 1;
}

static void timer50msProc(void)
{
	static uint8_t secondsLast;
	
	if (i.display_state == SETUP_NO){
		ds3231_read_time(&i.seconds,&i.minutes,&i.hours);
		if (i.seconds != secondsLast){
			secondsLast = i.seconds;
			if (dot_dis){
				displayDot(0);
			}else{
				displayDotPulse();
			}
		}
	}
}


static void apCounterProc(void)
{
	if (++ap.counter > 10-1){
		ap.counter = 0;
		apProc();
	}
}

//0,9,8,7,6,5,4,3,2,1,0,9,8,7,6,5,4,3,2,1,0
// border cases:
// 5->0;543210    432198760
// 3->0;3210      219876540
// 2->0;210       198765430
// if new lower than old -> counting to 9, and skip new, but last (9th) should be new.
static uint8_t apLoop(uint8_t dig) 
{
	if ((ap.digsNew[dig] >= ap.digsOld[dig])||(ap.digsNew[dig] == 0 && ap.digsOld[dig] == 9)){
		if (--ap.digsCurrent[dig] > 9) {
			ap.digsCurrent[dig] = 9;
		}
		if (ap.digsCurrent[dig] == ap.digsNew[dig]){
			return 1;
		}else{
			return 0;
		}
	}
	else{
		if (ap.digsCurrent[dig] == ap.digsOld[dig]) {ap.digsCounter[dig] = 8;}
		if (ap.digsCounter[dig]--){
			--ap.digsCurrent[dig];
			if (ap.digsCurrent[dig] == ap.digsNew[dig]) {--ap.digsCurrent[dig];};
			if (ap.digsCurrent[dig] > 9) {ap.digsCurrent[dig] = 9;}
			return 0;
		}else{
			ap.digsCurrent[dig] = ap.digsNew[dig];
			return 1;
		}
	}
}

void apStart (void)
{
	memset(ap.flagEn,1,sizeof(ap.flagEn));
	ap.procCounter = AP_CYCLES;
	ap.counter = 10-1;
	apUpdateTime();
}

static uint8_t apInWork (void)
{
	if (ap.flagEn[0]||ap.flagEn[1]||ap.flagEn[2]||ap.flagEn[3]){
		return 1;
	}else{
		return 0;
	}
}

//once in 100ms
static void apProc (void)
{
	uint8_t d;
	uint8_t brighMaxBitMask = 0;
	uint8_t limit;

	if (apInWork()){
		
		limit = ap.procCounter/AP_CYCLES;
		
		for (d=0;d<limit;d++){
			if (ap.flagEn[d] == 1){
				if (apLoop(d)){
					ap.flagEn[d] = 2;	//one more cycle
				}
				i.display[d] = ap.digsCurrent[d];
				displayNixie(&i.display[0],bitset(brighMaxBitMask,d));
			}else if (ap.flagEn[d] == 2){
				ap.flagEn[d] = 0;
			}
		}
		if (ap.procCounter < NIXIE_COUNT*AP_CYCLES){
			++ap.procCounter;
		}
	}
}

static void apUpdateTime (void)
{
	memcpy(ap.digsOld, ap.digsNew, sizeof(ap.digsOld));
	memcpy(ap.digsCurrent, ap.digsOld, sizeof(ap.digsCurrent));
	iface_disp2bcdDigit(i.minutes,	&ap.digsNew[0],&ap.digsNew[1]);
	iface_disp2bcdDigit(i.hours,		&ap.digsNew[2],&ap.digsNew[3]);
}

static void iface_display(void)
{
	static uint8_t secondsLast = 0xff;
	static uint8_t minutesLast = 0xff;
	
	uint16_t current_minutes;
	uint16_t start_minutes;
	uint16_t end_minutes;
	
	switch(i.display_state){
		case SETUP_NO:
			current_minutes = time_to_minutes(bcd_to_decimal(i.hours), bcd_to_decimal(i.minutes));
			start_minutes 	=	time_to_minutes(e.nBrightStartH, e.nBrightStartM);
			end_minutes 		= time_to_minutes(e.nBrightEndH, e.nBrightEndM);
			
			if (minutesLast == 0xff) {
				minutesLast = i.minutes;
			}
			else if (minutesLast != i.minutes){
				minutesLast = i.minutes;
				if (!is_time_in_interval(current_minutes, start_minutes, end_minutes)) {
					if (i.minutes%4 == 0)
					{
						apStart();
					}
				}
			}
			
			if (!apInWork()){
				iface_disp2bcdDigit(i.minutes, &i.display[0], &i.display[1]);
				iface_disp2bcdDigit(i.hours, &i.display[2], &i.display[3]);
				displayNixie(&i.display[0],0);
			
				if (i.seconds != secondsLast){
					secondsLast = i.seconds;
					
					if (e.nBrightEn){
						if (is_time_in_interval(current_minutes, start_minutes, end_minutes)) {
							displaySetBright(e.nBright);
							if (e.nBright == 0){
								displayRGBset(0);
								dot_dis = 1;
							} else {
								dot_dis = 0;
								if (e.rgbGlobalEn){
									if (e.rgbAtNightEn){
										displayRGBset(1);
									} else {
										displayRGBset(0);
									}
								}
							}
						} else {
							displaySetBright(e.bright);
							dot_dis = 0;
							if (e.rgbGlobalEn){
								displayRGBset(1);
							}
						}
					} else {
						displaySetBright(e.bright);
						dot_dis = 0;
						if (e.rgbGlobalEn){
							displayRGBset(1);
						}
					}
				}
			}
			break;
		
		case SETUP_HOURS:
			iface_disp2bcdDigit(i.minutesSetupValue, &i.display[0], &i.display[1]);
			iface_disp2bcdDigit(i.hoursSetupValue, &i.display[2], &i.display[3]);
			if(i.flag05s){
				i.display[2] = NIXIE_OFF;
				i.display[3] = NIXIE_OFF;
			}
			displayNixie(&i.display[0],0);
			displayDot(1);
			break;
		
		case SETUP_MINUTES:
			iface_disp2bcdDigit(i.minutesSetupValue, &i.display[0], &i.display[1]);
			iface_disp2bcdDigit(i.hoursSetupValue, &i.display[2], &i.display[3]);
			if(i.flag05s){
				i.display[0] = NIXIE_OFF;
				i.display[1] = NIXIE_OFF;
			}
			displayNixie(&i.display[0],0);
			displayDot(1);
			break;
		
		case SETUP_R:
		case SETUP_G:
		case SETUP_B:
			iface_disp3decDigit (i.setupValue, &i.display[0], &i.display[1], &i.display[2]);
			if (i.flag100ms){
				i.display[3] = NIXIE_OFF;
			}else{
				i.display[3] = i.display_state-11;
			}
			displayNixie(&i.display[0],0);
			displayDot(0);
			break;
		
		case SETUP_BRIGHT:
		case SETUP_NIGHT_BR:
			i.display[3] = i.display_state;
			if (i.setupValue) {
				iface_disp3decDigit (i.setupValue, &i.display[0], &i.display[1], &i.display[2]);
			}else{
				i.display[0] = NIXIE_OFF;
				i.display[1] = NIXIE_OFF;
				i.display[2] = NIXIE_OFF;
			}
			displayNixie(&i.display[0],bin(00001000));
			displayDot(0);
		break;
		
		case SETUP_ZERO:
		case SETUP_F1224:
		case SETUP_NIGHT_BR_EN:
		case SETUP_NIGHT_RGB_EN:
		case SETUP_ANTIPOISONING_AT_NIGHT_ONLY:
			if (i.display_state > 9){
				iface_disp2decDigit(i.display_state,&i.display[2],&i.display[3]);
			}else{
				i.display[3] = i.display_state;
				i.display[2] = NIXIE_OFF;	
			}
			i.display[1] = NIXIE_OFF;
			i.display[0] = i.setupValue;
			displayNixie(&i.display[0],0);
			displayDot(0);
			break;
		
		case SETUP_COLON_BLINKING_TYPE:
			if (i.display_state > 10){
				iface_disp2decDigit(i.display_state,&i.display[2],&i.display[3]);
			}else{
				i.display[3] = i.display_state;
				i.display[2] = NIXIE_OFF;	
			}
			i.display[1] = NIXIE_OFF;
			i.display[0] = i.setupValue;
			displayNixie(&i.display[0],0);
			displayDot(0);
			break;
		
		case SETUP_NIGHT_BR_START_H:
		case SETUP_NIGHT_BR_START_M:
		case SETUP_NIGHT_BR_END_H:
		case SETUP_NIGHT_BR_END_M:
			iface_disp2decDigit(i.setupValue, &i.display[0], &i.display[1]);
			i.display[2] = NIXIE_OFF;
			i.display[3] = i.display_state;
			displayNixie(&i.display[0],0);
			displayDot(0);
			break;
		
		default:
			break;
	}
}

static void iface_disp2decDigit (uint8_t value, uint8_t *d0, uint8_t *d1)
{
	if (value > 99) {value = 99;}
	*d0 = value%10;
	*d1 = value/10;
}

static void iface_disp3decDigit (uint16_t value, uint8_t *d0, uint8_t *d1, uint8_t *d2)
{
	if (value > 999) {value = 999;}
	*d0 = value%10;
	*d1 = (value/10)%10;
	*d2 = value/100;
}

static void iface_disp2bcdDigit (uint8_t value, uint8_t *d0, uint8_t *d1)
{
	if (value > 0x99) {value = 0x99;}
	*d0 = value&0x0F;
	*d1 = (value&0xF0)>>4;
}

uint8_t bcd_to_decimal(uint8_t bcd)
{
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t decimal_to_bcd(uint8_t decimal) 
{
	return ((decimal / 10) << 4) | (decimal % 10);
}

uint16_t time_to_minutes(uint8_t hours, uint8_t minutes) 
{
	return (uint16_t)hours * 60 + (uint16_t)minutes;
}

uint8_t is_time_in_interval(uint16_t current_minutes, uint16_t start_minutes, uint16_t end_minutes) 
{
	if (start_minutes <= end_minutes) {
			// Interval without crossing midnight
			return current_minutes >= start_minutes && current_minutes < end_minutes;
	} else {
			// Interval with crossing midnight
			return current_minutes >= start_minutes || current_minutes < end_minutes;
	}
}

/*
switch state of rgb
if time is in night interval - control nigt rgb enable flag
if time in in normal interval - control global rgb enable flag
*/
void RGBtoggle ( void )
{
	uint16_t current_minutes;
	uint16_t start_minutes;
	uint16_t end_minutes;
	
	current_minutes = time_to_minutes(bcd_to_decimal(i.hours), bcd_to_decimal(i.minutes));
	start_minutes 	=	time_to_minutes(e.nBrightStartH, e.nBrightStartM);
	end_minutes 		= time_to_minutes(e.nBrightEndH, e.nBrightEndM);
		
	if (is_time_in_interval(current_minutes, start_minutes, end_minutes)&&e.nBrightEn) {
		if (e.rgbAtNightEn){
			e.rgbAtNightEn = 0;
		} else {
			e.rgbAtNightEn = 1;
		}
		displayRGBset(e.rgbAtNightEn);
		EEPROM_writeByte(NIGHT_RGB_EN_ADDR,e.rgbGlobalEn);
	} 
	else {
		if (e.rgbGlobalEn){
			e.rgbGlobalEn = 0;
		} else {
			e.rgbGlobalEn = 1;
		}
		displayRGBset(e.rgbGlobalEn);
		EEPROM_writeByte(RGB_EN_ADDR,e.rgbGlobalEn);
	}
}