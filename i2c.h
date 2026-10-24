#ifndef _I2C_H_
#define	_I2C_H_

#ifdef	__cplusplus
extern "C" {
#endif

#define ACK						0
#define	NAK						1
#define READ_ANSWER		1
#define SET_ACK				0
#define SET_NAK				1

void i2c_init ( void );
uint8_t i2c_bus_free(void);


uint8_t i2c_shift(uint8_t b);
uint8_t i2c_ack(uint8_t b);
void i2c_start (void);
void i2c_stop (void);


#ifdef	__cplusplus
}
#endif

#endif	/* _I2C_H_ */