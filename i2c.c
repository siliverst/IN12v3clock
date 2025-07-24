#include "STM8S003F3.h"
#include "i2c.h"
#include "macro.h"

#define SCL_PIN						sfr_PORTB.ODR.ODR4
#define SDA_PIN						sfr_PORTB.ODR.ODR5
#define SDA_IN_PIN				sfr_PORTB.IDR.IDR5

void i2c_init(void)
{
	// configure SCL as OD output
	sfr_PORTB.DDR.DDR4 = 1;     // input(=0) or output(=1)
	sfr_PORTB.CR1.C14  = 0;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTB.CR2.C24  = 0;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTB.ODR.ODR4 = 1;
	// configure SDA as OD output
	sfr_PORTB.DDR.DDR5 = 1;     // input(=0) or output(=1)
	sfr_PORTB.CR1.C15  = 0;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTB.CR2.C25  = 0;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTB.ODR.ODR5 = 1;
}


uint8_t i2c_busy_check ( void )
{
	if (SDA_IN_PIN == OK){
		return OK;
	}else{
		return BUSY;
	}
}

// if the bus is busy, then 99% of the slave is stuck in ACK, this function is designed to eliminate this.
void i2c_slave_unlock ( void )
{
	uint8_t a,i;
	
	if (!SDA_IN_PIN){
		SCL_PIN = 0;
		i=0;
		while(i++<1) {NOP();}
			
		a = 0;
		do{
			i=0;
			while(i++<1) {NOP();}
			SCL_PIN = 1;
			i=0;
			while(i++<1) {NOP();}
			SCL_PIN = 0;
			a++;
		}while((a<9)&&(!SDA_IN_PIN));
	}
	i=0;
	while(i++<1) {NOP();}
	i2c_init();
	i=0;
	while(i++<1) {NOP();}
}


uint8_t i2c_shift(uint8_t b)
{
	uint8_t i;
	for(i = 0;i < 8; i++)
	{
			if(b & 0x80){
		SDA_PIN = 1;
	}else {
		SDA_PIN = 0;
	}
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	SCL_PIN = 1;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	b <<= 1;
	if(SDA_IN_PIN) b |= 1;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	SCL_PIN = 0;
	}
	return(b);
}

uint8_t i2c_ack(uint8_t b)
{
	if(b) SDA_PIN = 1;
	else  SDA_PIN = 0;
	NOP();
	NOP();
	NOP();
	SCL_PIN = 1;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	b = SDA_IN_PIN;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	SCL_PIN = 0;
	NOP();
	NOP();
	SDA_PIN = 1;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
    return(b);
}

void i2c_start (void)
{
	NOP();
	NOP();
	NOP();
	NOP();
	SDA_PIN = 1;
	SCL_PIN = 1;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	SDA_PIN = 0;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	SCL_PIN = 0;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
}

void i2c_stop (void)
{
	SCL_PIN = 0;
	SDA_PIN = 0;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	SCL_PIN = 1;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	SDA_PIN = 1;
}
