#include "STM8S003F3.h"
#include "ds3231.h"
#include "i2c.h"

static uint8_t ds3231_read(uint8_t reg, uint8_t *buf, uint8_t n);
static void ds3231_write(uint8_t reg, uint8_t *buf, uint8_t n);

/*
the clock works in 24h format only: if the RTC is in 12h mode (set by another firmware),
convert the hours register to 24h.
*/
void ds3231Init(void)
{
	uint8_t h, h12;

	i2c_init();
	if (ds3231_read(HOURS_ADDR, &h, 1) && (h & DS3231_12H)){
		h12 = h & 0x1F;																		// BCD 1..12
		if (h12 == 0x12) {h12 = 0;}
		if (h & DS3231_PM) {
			h12 += 0x12;
			if ((h12 & 0x0F) > 0x09) {h12 += 0x06;}					// BCD correction
		}
		ds3231_write(HOURS_ADDR, &h12, 1);
	}
}

// time is in BCD, the values are not changed if the RTC does not answer
void ds3231_read_time(uint8_t *seconds, uint8_t *minutes, uint8_t *hours)
{
	uint8_t t[3];

	if (ds3231_read(SECONDS_ADDR, t, sizeof(t))){
		*seconds = t[0] & 0x7F;
		*minutes = t[1] & 0x7F;
		*hours = t[2] & 0x3F;		// 24h format, bit 6 = 0
	}
}

void ds3231_write_time(uint8_t *seconds, uint8_t *minutes, uint8_t *hours)
{
	uint8_t t[3];

	t[0] = *seconds;
	t[1] = *minutes;
	t[2] = *hours;
	ds3231_write(SECONDS_ADDR, t, sizeof(t));
}

// start and send the address of the chip and of the register, return ACK
static uint8_t ds3231_select(uint8_t reg)
{
	if (i2c_busy_check() != OK){
		i2c_slave_unlock();
		return 0;
	}
	i2c_start();
	i2c_shift(DS3231_ADDRESS);
	if (i2c_ack(READ_ANSWER) != ACK) {return 0;}
	i2c_shift(reg);
	return (i2c_ack(READ_ANSWER) == ACK);
}

// return 1 if ok
static uint8_t ds3231_read(uint8_t reg, uint8_t *buf, uint8_t n)
{
	uint8_t res = 0;

	if (ds3231_select(reg)){
		i2c_start();
		i2c_shift(DS3231_ADDRESS | 0x01);
		if (i2c_ack(READ_ANSWER) == ACK){
			while (n--){
				*buf++ = i2c_shift(0xFF);
				i2c_ack(n ? SET_ACK : SET_NAK);
			}
			res = 1;
		}
	}
	i2c_stop();
	return res;
}

static void ds3231_write(uint8_t reg, uint8_t *buf, uint8_t n)
{
	if (ds3231_select(reg)){
		while (n--){
			i2c_shift(*buf++);
			if (i2c_ack(READ_ANSWER) != ACK) {break;}
		}
	}
	i2c_stop();
}
