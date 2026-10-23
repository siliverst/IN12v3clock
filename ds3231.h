#ifndef _DS3231_H_
#define	_DS3231_H_

#ifdef	__cplusplus
extern "C" {
#endif


// DS3231 I2C address
#define DS3231_ADDRESS	0xD0
// hours register bits
#define DS3231_12H			0x40
#define DS3231_PM				0x20

enum ds3231_reg_addresses{
	SECONDS_ADDR,
	MINUTES_ADDR,
	HOURS_ADDR,
	DAY_ADDR,
	DATE_ADDR,
	MONTH_ADDR,
	YEAR_ADDR,
	ALARM1_SECONDS_ADDR,
	ALARM1_MINUTES_ADDR,
	ALARM1_HOURS_ADDR,
	ALARM1_DAY_DATE_ADDR,
	ALARM2_MINUTES_ADDR,
	ALARM2_HOUR_ADDR,
	ALARM2_DAY_DATE_ADDR,
	CONTROL_ADDR,
	CONTROL_STATUS_ADDR,
	AGING_OFFSET_ADDR,
	MSB_OF_TEMP_ADDR,
	LSB_OF_TEMP_ADDR
};



void ds3231Init(void);
void ds3231_read_time(uint8_t *seconds, uint8_t *minutes, uint8_t *hours);
void ds3231_write_time(uint8_t *seconds, uint8_t *minutes, uint8_t *hours);

#ifdef	__cplusplus
}
#endif

#endif	/* _DS3231_H_ */