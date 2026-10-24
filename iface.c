#include <stdint.h>
#include "iface.h"
#include "keyevents.h"
#include "ds3231.h"
#include "display.h"
#include "macro.h"
#include "eeprom.h"
#include "rgb.h"
#include "effects.h"
#include <string.h>

Iface i;

static void iface_display(void);
static void iface_disp2decDigit (uint8_t value, uint8_t *d0, uint8_t *d1);
static void iface_disp3decDigit (uint16_t value, uint8_t *d0, uint8_t *d1, uint8_t *d2);
static void iface_disp2bcdDigit (uint8_t value, uint8_t *d0, uint8_t *d1);


// settings menu: the item number "group item" of every step (BCD)
static const uint8_t menuNum[SETUP_MENU_LAST + 1] = {0x11, 0x12, 0x13, 0x21, 0x22, 0x23, 0x24, 0x31, 0x32, 0x32, 0x33, 0x33,
                                                     0x41, 0x42, 0x43, 0x44, 0x45};
static uint8_t dotOdd;					// colon preview in the menu

static uint16_t nowMinutes;		// current time, minutes from midnight
static uint8_t rgbBlink;			// 100ms ticks of the backlight blinking, see RGBtoggle()

// smooth change of the brightness (day/night, K2): the shown values go to the targets by iface_fade()
static uint8_t brightNow, brightTarget, rgbNow, rgbTarget;
static uint8_t rgbFast;								// backlight fade: 1 - fast (K2) till the target, 0 - as the digits (day/night)


void iface_init( void )
{
	i.display_state = SETUP_NO;
	ds3231_read_time(&i.seconds,&i.minutes,&i.hours);
	iface_apply_mode();
	iface_show(brightTarget, IFACE_RGB_KEEP);		// digits at once, the backlight fades in
}

void iface_flag05sReset(void)
{
	i.counter1s = 0;
	i.flag05s = 0;
}

/*
every 100ms: blinking counters, antipoisoning, key timers, display
*/
void iface_100ms_proc (void)
{
	//counter for blinking digits while time setup, 0.5s on/off
	if (++i.counter1s > 10-1){
		i.counter1s = 0;
	}
	i.flag05s = (i.counter1s > 4);
	if (i.menuIntro) {i.menuIntro--;}
	
	//counter for blinking digits while RGB setup
	i.flag100ms ^= 1;
	
	keyevents_counters();
	iface_display();
}

static uint8_t fade(uint8_t now, uint8_t target, uint8_t step)
{
	if (now + step < target) {return now + step;}
	if (now > target + step) {return now - step;}
	return target;
}

// every 50ms: digits 1%, backlight 1% or 4% after K2 (perceptually linear), full scale in 5s / 1.25s
static void iface_fade(void)
{
	uint8_t b = fade(brightNow, brightTarget, 1);
	
	if (b != brightNow){
		brightNow = b;
		displaySetBright(b);
	}
	b = fade(rgbNow, rgbTarget, rgbFast ? 4 : 1);
	if ((b != rgbNow) && !rgbBlink){
		rgbNow = b;
		RGBset(b);
	}
	if (b == rgbTarget) {rgbFast = 0;}
}

// set the brightness at once (setup modes), rgb = IFACE_RGB_KEEP - do not change the backlight
void iface_show(uint8_t digits, uint8_t rgb)
{
	brightNow = digits;
	displaySetBright(digits);
	if (rgb != IFACE_RGB_KEEP){
		rgbNow = rgb;
		RGBset(rgb);
	}
}

#ifdef AP_TEST
/*
test firmware (python -I tools/build.py -DAP_TEST): the time runs 6 times faster from 09:57,
+1 minute every 10s, antipoisoning at every minute
*/
static uint8_t dec_to_bcd(uint8_t v)
{
	return (uint8_t)((v / 10) << 4) | (v % 10);
}

static void ap_test_time(void)
{
	static uint16_t m = 9*60 + 57;
	static uint8_t sLast;
	
	if (((i.seconds & 0x0F) == 0) && (i.seconds != sLast)) {m = (m + 1) % (24*60);}
	sLast = i.seconds;
	i.minutes = dec_to_bcd(m % 60);
	i.hours = dec_to_bcd(m / 60);
}
#endif

// every 50ms: get time from ds3231 and start the dot pulse at a new second, smooth brightness change
void iface_50ms_proc (void)
{
	static uint8_t secondsLast;
	
	if (i.display_state == SETUP_NO){
		iface_fade();
		ds3231_read_time(&i.seconds,&i.minutes,&i.hours);
#ifdef AP_TEST
		ap_test_time();
#endif
		if (i.seconds != secondsLast){
			secondsLast = i.seconds;
			displayDotPulse(i.seconds & 1);			// BCD, bit 0 is the parity
		}
	}
}


// time digits (BCD time) by the time format and the leading zero setting
void iface_time_digits(uint8_t *d, uint8_t minutes, uint8_t hours)
{
	uint8_t h = bcd_to_decimal(hours);
	
	iface_disp2bcdDigit(minutes, &d[0], &d[1]);
	if (e.f1224){
		h %= 12;
		if (!h) {h = 12;}
	}
	iface_disp2decDigit(h, &d[2], &d[3]);
	if (!d[3] && !e.zeroEn) {d[3] = NIXIE_OFF;}
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
		brightTarget = e.nBright;
		rgbTarget = e.rgbNightEn ? e.rgbNightBright : 0;
	}else{
		brightTarget = e.bright;
		rgbTarget = e.rgbDayEn ? e.rgbBright : 0;
	}
}

/*
settings menu: show the brightness which is being set, the backlight is off for other items
*/
void iface_menu_preview (void)
{
	uint8_t s = i.display_state;
	
	iface_show((s == SETUP_BRIGHT || s == SETUP_NIGHT_BR) ? EE(s) : 100,
	           (s == SETUP_RGB_BR || s == SETUP_RGB_NIGHT_BR) ? EE(s) : 0);
}

/*
settings menu, K2: save the item and go to the next one, after the last one - exit.
The values are changed in e (for the preview), they are saved here or reloaded from EEPROM on exit.
*/
void iface_menu_next (void)
{
	uint8_t s = i.display_state;

	EEPROM_set(s, EE(s));
	if (s == SETUP_MENU_LAST){
		iface_exit_setup();
		return;
	}
	i.display_state = ++s;
	i.menuIntro = ((s == SETUP_NIGHT_START_H) || (s == SETUP_NIGHT_END_H)) ? 8 : 0;
	i.timeSetupCounter = 60*10;
	iface_menu_preview();
}

/*
back to the time display, by the last key or by timeout.
Settings are reloaded from EEPROM to drop not saved changes (the color in RGB setup).
*/
void iface_exit_setup (void)
{
	i.display_state = SETUP_NO;
	rgbBlink = 0;
	EEPROM_dataInit();
	iface_apply_mode();
}

static void iface_display(void)
{
	static uint8_t secondsLast = 0xff;
	static uint8_t minutesLast = 0xff;
	uint8_t interval;
	uint8_t s = i.display_state;
	uint8_t v, a;
	
	switch(s){
		case SETUP_NO:
			if (rgbBlink){
				--rgbBlink;
				RGBset((rgbBlink % 6) >= 3 ? 50 : 0);			// 0.3s on, 0.3s off
			}
			
			if (i.seconds != secondsLast){
				secondsLast = i.seconds;
				iface_apply_mode();
				
				if (minutesLast != i.minutes){
					if (minutesLast != 0xff){		// not at start
						// at night with digits off antipoisoning is not done, it disturbs people
						interval = i.night ? (e.nBright ? e.apNight : 0) : e.apDay;
#ifdef AP_TEST
						interval = 1;
#endif
						if (interval && (nowMinutes % interval == 0)){
							fxApStart();
						}
					}
					minutesLast = i.minutes;
				}
			}
			break;				// the digits are shown by fx_10ms()
		
		case SETUP_HOURS:
		case SETUP_MINUTES:
			iface_time_digits(i.display, i.minutesSetupValue, i.hoursSetupValue);
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
			// 12h format: PM is shown by the fast blinking colon
			displayDot((e.f1224 && (i.hoursSetupValue >= 0x12) && i.flag100ms) ? 0 : 100);
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
			/*
			settings menu: the item number "group item" in the left tubes at full brightness, the colon, the value.
			The night start and end: the item number for 0.8s, then the time HH:MM, the edited part blinks.
			*/
			if (s > SETUP_MENU_LAST) {break;}
			if ((s >= SETUP_NIGHT_START_H) && (s <= SETUP_NIGHT_END_M) && !i.menuIntro){
				a = (s <= SETUP_NIGHT_START_M) ? NIGHT_START_H_ADDR : NIGHT_END_H_ADDR;
				iface_disp2decDigit(EE(a + 1), &i.display[0], &i.display[1]);
				iface_disp2decDigit(EE(a), &i.display[2], &i.display[3]);
				if (i.flag05s){
					a = (s == a) ? 2 : 0;
					i.display[a] = NIXIE_OFF;
					i.display[a + 1] = NIXIE_OFF;
				}
				v = 0;
			}else{
				v = EE(s);
				i.display[0] = v % 10;
				i.display[1] = (eeMax[s] > 9) ? v / 10 : NIXIE_OFF;
				if (i.menuIntro){
					i.display[0] = NIXIE_OFF;
					i.display[1] = NIXIE_OFF;
				}
				i.display[2] = menuNum[s] & 0x0F;
				i.display[3] = menuNum[s] >> 4;
				v = bin(00001100);
			}
			displayNixie(i.display, v);
			if (s != SETUP_COLON){
				displayDot(100);
			}else if (!i.counter1s){
				dotOdd ^= 1;
				displayDotPulse(dotOdd);				// preview of the colon mode
			}
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
K2: switch the backlight on/off for the current interval (day or night).
If the backlight brightness of the interval is 0, blink 3 times at 50% to show
that it is switched off by the settings, the on/off state is not changed.
*/
void RGBtoggle ( void )
{
	uint8_t a = i.night ? RGB_NIGHT_EN_ADDR : RGB_DAY_EN_ADDR;
	
	if (!(i.night ? e.rgbNightBright : e.rgbBright)){
		rgbBlink = 3*6;
		return;
	}
	EEPROM_set(a, !EE(a));
	rgbFast = 1;
	iface_apply_mode();
}
