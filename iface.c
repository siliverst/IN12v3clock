#include <stdint.h>
#include "iface.h"
#include "keyevents.h"
#include "service_timing.h"
#include "ds3231.h"
#include "display.h"
#include "macro.h"
#include "eeprom.h"
#include "rgb.h"
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

static uint16_t nowMinutes;		// current time, minutes from midnight

void iface_init( void )
{
	i.display_state = SETUP_NO;
	ds3231_read_time(&i.seconds,&i.minutes,&i.hours);
	memset(ap.flagEn,0,sizeof(ap.flagEn));
	apUpdateTime();	// two calls toupdate the digsNew array;
	apUpdateTime();
	iface_apply_mode();
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
			displayDotPulse();
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

/*
day or night mode by the current time: digits and backlight brightness
*/
void iface_apply_mode (void)
{
	uint16_t start_minutes;
	uint16_t end_minutes;
	
	nowMinutes		= time_to_minutes(bcd_to_decimal(i.hours), bcd_to_decimal(i.minutes));
	start_minutes	= time_to_minutes(e.nightStartH, e.nightStartM);
	end_minutes		= time_to_minutes(e.nightEndH, e.nightEndM);
	i.night = e.nightEn && is_time_in_interval(nowMinutes, start_minutes, end_minutes);
	
	if (i.night){
		displaySetBright(e.nBright);
		RGBset(e.rgbNightEn ? e.rgbNightBright : 0);
	}else{
		displaySetBright(e.bright);
		RGBset(e.rgbDayEn ? e.rgbBright : 0);
	}
}

/*
settings menu: show the brightness which is being set, the backlight is off for other items
*/
void iface_menu_preview (void)
{
	uint8_t s = i.display_state;
	
	displaySetBright((s == SETUP_BRIGHT || s == SETUP_NIGHT_BR) ? i.setupValue : 100);
	RGBset((s == SETUP_RGB_BR || s == SETUP_RGB_NIGHT_BR) ? i.setupValue : 0);
}

/*
back to the time display, by the last key or by timeout.
Settings are reloaded from EEPROM to drop not saved changes (the color in RGB setup).
*/
void iface_exit_setup (void)
{
	i.display_state = SETUP_NO;
	EEPROM_dataInit();
	iface_apply_mode();
}

static void iface_display(void)
{
	static uint8_t secondsLast = 0xff;
	static uint8_t minutesLast = 0xff;
	uint8_t interval;
	uint8_t s = i.display_state;
	uint8_t v = i.setupValue;
	
	switch(s){
		case SETUP_NO:
			if (i.seconds != secondsLast){
				secondsLast = i.seconds;
				iface_apply_mode();
				
				if (minutesLast != i.minutes){
					if (minutesLast != 0xff){		// not at start
						interval = i.night ? e.apNight : e.apDay;
						if (interval && (nowMinutes % interval == 0)){
							apStart();
						}
					}
					minutesLast = i.minutes;
				}
			}
			
			if (!apInWork()){
				iface_disp2bcdDigit(i.minutes, &i.display[0], &i.display[1]);
				iface_disp2bcdDigit(i.hours, &i.display[2], &i.display[3]);
				displayNixie(&i.display[0],0);
			}
			break;
		
		case SETUP_HOURS:
		case SETUP_MINUTES:
			iface_disp2bcdDigit(i.minutesSetupValue, &i.display[0], &i.display[1]);
			iface_disp2bcdDigit(i.hoursSetupValue, &i.display[2], &i.display[3]);
			if(i.flag05s){
				if (s == SETUP_HOURS){
					i.display[2] = NIXIE_OFF;
					i.display[3] = NIXIE_OFF;
				}else{
					i.display[0] = NIXIE_OFF;
					i.display[1] = NIXIE_OFF;
				}
			}
			displayNixie(&i.display[0],0);
			displayDot(100);
			break;
		
		case SETUP_R:
		case SETUP_G:
		case SETUP_B:
			iface_disp3decDigit (EE(R_ADDR + s - SETUP_R), &i.display[0], &i.display[1], &i.display[2]);
			if (i.flag100ms){
				i.display[3] = NIXIE_OFF;
			}else{
				i.display[3] = s - SETUP_R + 1;
			}
			displayNixie(&i.display[0],0);
			displayDot(0);
			break;
		
		default:
			// settings menu: item number in the left digits (at full brightness), value in the right ones
			if (s > SETUP_MENU_LAST) {break;}
			i.display[0] = v % 10;
			i.display[1] = (eeMax[s] > 9) ? (v / 10) % 10 : NIXIE_OFF;
			i.display[2] = (v > 99) ? v / 100 : NIXIE_OFF;
			if (s > 9){
				iface_disp2decDigit(s, &i.display[2], &i.display[3]);
				displayNixie(&i.display[0],bin(00001100));
			}else{
				i.display[3] = s;
				displayNixie(&i.display[0],bin(00001000));
			}
			displayDot(0);
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
K2: switch the backlight on/off for the current interval (day or night)
*/
void RGBtoggle ( void )
{
	uint8_t a = i.night ? RGB_NIGHT_EN_ADDR : RGB_DAY_EN_ADDR;
	
	EEPROM_set(a, !EE(a));
	iface_apply_mode();
}
