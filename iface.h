#ifndef _IFACE_H_
#define	_IFACE_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "display.h"
#include "eeprom.h"

// settings menu items are the EEPROM addresses
enum setup_states{
	SETUP_ZERO						= ZERO_ADDR,
	SETUP_F1224						= F1224_ADDR,
	SETUP_BRIGHT					= BRIGHT_ADDR,
	SETUP_NIGHT_BR				= NIGHT_BR_ADDR,
	SETUP_RGB_BR					= RGB_BR_ADDR,
	SETUP_RGB_NIGHT_BR		= RGB_NIGHT_BR_ADDR,
	SETUP_NIGHT_EN				= NIGHT_EN_ADDR,
	SETUP_NIGHT_START_H		= NIGHT_START_H_ADDR,
	SETUP_NIGHT_START_M		= NIGHT_START_M_ADDR,
	SETUP_NIGHT_END_H			= NIGHT_END_H_ADDR,
	SETUP_NIGHT_END_M			= NIGHT_END_M_ADDR,
	SETUP_AP_DAY					= AP_DAY_ADDR,
	SETUP_AP_NIGHT				= AP_NIGHT_ADDR,
	SETUP_COLON						= COLON_ADDR,
	SETUP_R,
	SETUP_G,
	SETUP_B,
	SETUP_HOURS,
	SETUP_MINUTES,
	SETUP_NO = 255
};
#define SETUP_MENU_LAST		SETUP_COLON

typedef struct  
{
	uint16_t 	timeSetupCounter;
	uint8_t		display[NIXIE_COUNT];
	uint8_t		display_state;
	uint8_t		seconds;
	uint8_t		minutes;
	uint8_t		hours;
	uint8_t		setupValue;
	uint8_t 	minutesSetupValue;
	uint8_t 	hoursSetupValue;
	uint8_t 	counter1s;
	uint8_t		flag05s;
	uint8_t		flag100ms;
	uint8_t		night;							// 1 - night interval is active now
}Iface;

extern Iface i;



void iface_init( void );
void iface_50ms_proc (void);
void iface_100ms_proc (void);
void iface_flag05sReset(void);
uint8_t bcd_to_decimal(uint8_t bcd);
uint16_t time_to_minutes(uint8_t hour, uint8_t minute);
uint8_t is_time_in_interval(uint16_t current_minutes, uint16_t start_minutes, uint16_t end_minutes);
void apStart (void);
void RGBtoggle ( void );
void iface_apply_mode (void);
void iface_menu_preview (void);
#define IFACE_RGB_KEEP	255
void iface_show(uint8_t digits, uint8_t rgb);
void iface_exit_setup (void);

#ifdef	__cplusplus
}
#endif

#endif	/* _IFACE_H_ */