#ifndef _EEPROM_H_
#define	_EEPROM_H_

#ifdef	__cplusplus
extern "C" {
#endif

// EEPROM layout. Addresses 0..SETUP_MENU_LAST are also the settings menu steps in their order (see iface.h),
// the order of the fields in Edata must be the same.
enum eeprom_adreses{
	ZERO_ADDR,								// 1.1 leading zero of the hours
	F1224_ADDR,								// 1.2 12/24h
	COLON_ADDR,								// 1.3 colon: 0 - pulse, 1 - on, 2 - off, 3 - slow pulse, 4, 5 - blinking
	BRIGHT_ADDR,							// 2.1 digits brightness, day, 5..99%
	NIGHT_BR_ADDR,						// 2.2 digits brightness, night, 0, 5..99%
	RGB_BR_ADDR,							// 2.3 backlight brightness, day, 0..99%
	RGB_NIGHT_BR_ADDR,				// 2.4 backlight brightness, night, 0..99%
	PWM_ADDR,									// 2.5 digits PWM: 0 - 100Hz, 1 - 200Hz, 2 - 400Hz
	NIGHT_EN_ADDR,						// 3.1 night interval enable
	NIGHT_START_H_ADDR,				// 3.2 night start, hours and minutes (one menu item)
	NIGHT_START_M_ADDR,
	NIGHT_END_H_ADDR,					// 3.3 night end
	NIGHT_END_M_ADDR,
	FLIP_ADDR,								// 4.1 minute change effect, 0 - none
	AP_TYPE_ADDR,							// 4.2 antipoisoning effect
	AP_DAY_ADDR,							// 4.3 antipoisoning interval, day, minutes, 0 - off
	AP_NIGHT_ADDR,						// 4.4 antipoisoning interval, night, minutes, 0 - off
	GLITCH_ADDR,							// 4.5 glitches on/off
	RGB_DAY_EN_ADDR,					// backlight on/off (K2), day
	RGB_NIGHT_EN_ADDR,				// backlight on/off (K2), night
	R_ADDR,
	G_ADDR,
	B_ADDR,
	EE_COUNT
};

#define EE_MAGIC_ADDR			EE_COUNT
#define EE_MAGIC					0x5D		// change it when the layout is changed
#define FLIP_MAX					5
#define PWM_MAX						2
#define AP_TYPE_MAX				2
#define AP_INTERVAL_MAX		30			// minutes
#define BRIGHT_MIN				5				// %, minimal digits brightness (except 0 at night)
#define BRIGHT_MAX				99			// %, the menu shows 2 digits of the value

typedef struct {
	uint8_t		zeroEn;
	uint8_t		f1224;
	uint8_t		colonBlinkingType;
	uint8_t		bright;
	uint8_t		nBright;
	uint8_t		rgbBright;
	uint8_t		rgbNightBright;
	uint8_t		pwm;
	uint8_t		nightEn;
	uint8_t		nightStartH;
	uint8_t		nightStartM;
	uint8_t		nightEndH;
	uint8_t		nightEndM;
	uint8_t		flip;
	uint8_t		apType;
	uint8_t		apDay;
	uint8_t		apNight;
	uint8_t		glitchEn;
	uint8_t		rgbDayEn;
	uint8_t		rgbNightEn;
	uint8_t		r;
	uint8_t		g;
	uint8_t		b;
}Edata;

extern Edata e;
extern const uint8_t eeMax[EE_COUNT];

// access to the field of e by its EEPROM address
#define EE(addr)		(((uint8_t *)&e)[addr])

void EEPROM_dataInit(void);
void EEPROM_set(uint8_t addr, uint8_t data);
uint8_t EEPROM_writeByte(uint16_t logAddr, uint8_t data);
uint8_t EEPROM_readByte(uint16_t logAddr);


#ifdef	__cplusplus
}
#endif

#endif	/* _EEPROM_H_ */
