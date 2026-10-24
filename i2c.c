#include "STM8S003F3.h"
#include "i2c.h"
#include "macro.h"

/*
Software I2C master on PB4 (SCL) and PB5 (SDA), true open-drain pins with external pull-ups.
SCL is about 100kHz: every half period is one i2c_delay().
The ISRs may stretch a half period, that is allowed by I2C (DS3231 has no bus timeout).
*/
#define SCL_PIN						sfr_PORTB.ODR.ODR4
#define SDA_PIN						sfr_PORTB.ODR.ODR5
#define SDA_IN_PIN				sfr_PORTB.IDR.IDR5
#define I2C_DELAY					20			// loop count, ~5us at 16MHz with the call

static void i2c_delay(void)
{
	uint8_t i = I2C_DELAY;
	
	while (--i) {NOP();}
}

void i2c_init(void)
{
	// release the lines first, then outputs (open-drain: CR1 = 0 after reset)
	SCL_PIN = 1;
	SDA_PIN = 1;
	sfr_PORTB.DDR.DDR4 = 1;
	sfr_PORTB.DDR.DDR5 = 1;
}

/*
return 1 if the bus is free.
SDA held low means the slave is stuck (MCU reset in the middle of a read): clock SCL until it releases SDA
(9 clocks at most), stop and return 0.
*/
uint8_t i2c_bus_free(void)
{
	uint8_t a = 9;
	
	if (SDA_IN_PIN) {return 1;}
	while (!SDA_IN_PIN && a--){
		SCL_PIN = 0;
		i2c_delay();
		SCL_PIN = 1;
		i2c_delay();
	}
	i2c_stop();
	return 0;
}

// send b and read the bus at the same time (send 0xFF to read), SCL is low at the end
uint8_t i2c_shift(uint8_t b)
{
	uint8_t i;
	
	for (i = 0; i < 8; i++){
		SDA_PIN = (b & 0x80) ? 1 : 0;
		i2c_delay();
		SCL_PIN = 1;
		i2c_delay();
		b <<= 1;
		if (SDA_IN_PIN) {b |= 1;}
		SCL_PIN = 0;
	}
	return b;
}

// send the ACK bit b (READ_ANSWER to read the slave ACK), return the bus state, SDA is released at the end
uint8_t i2c_ack(uint8_t b)
{
	SDA_PIN = b;
	i2c_delay();
	SCL_PIN = 1;
	i2c_delay();
	b = SDA_IN_PIN;
	SCL_PIN = 0;
	SDA_PIN = 1;
	return b;
}

// start or repeated start, SCL is low at the end
void i2c_start(void)
{
	SDA_PIN = 1;
	i2c_delay();
	SCL_PIN = 1;
	i2c_delay();
	SDA_PIN = 0;
	i2c_delay();
	SCL_PIN = 0;
}

void i2c_stop(void)
{
	SCL_PIN = 0;
	SDA_PIN = 0;
	i2c_delay();
	SCL_PIN = 1;
	i2c_delay();
	SDA_PIN = 1;
	i2c_delay();
}
