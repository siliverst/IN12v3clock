/*
 Wrapper for STM8_headers/include/STM8S003F3.h (git submodule, not changed).
 The original header declares the peripheral registers without "volatile", so with the
 Cosmic optimizer enabled register accesses can be removed: e.g. the first of the two
 EEPROM unlock keys was dropped, the unlock never happened and the IWDG reset the MCU.
 Here all sfr_xxx macros are redefined as volatile. All sources include "STM8S003F3.h",
 quoted includes are searched in the source directory first, so this file is used.
*/
#ifndef _STM8S003F3_VOLATILE_H_
#define _STM8S003F3_VOLATILE_H_

#include "STM8_headers/include/STM8S003F3.h"

#undef  sfr_ADC1
#define sfr_ADC1   (*((volatile ADC1_t*) 0x53e0))
#undef  sfr_AWU
#define sfr_AWU   (*((volatile AWU_t*) 0x50f0))
#undef  sfr_BEEP
#define sfr_BEEP   (*((volatile BEEP_t*) 0x50f3))
#undef  sfr_CLK
#define sfr_CLK   (*((volatile CLK_t*) 0x50c0))
#undef  sfr_CPU
#define sfr_CPU   (*((volatile CPU_t*) 0x7f00))
#undef  sfr_DM
#define sfr_DM   (*((volatile DM_t*) 0x7f90))
#undef  sfr_FLASH
#define sfr_FLASH   (*((volatile FLASH_t*) 0x505a))
#undef  sfr_I2C
#define sfr_I2C   (*((volatile I2C_t*) 0x5210))
#undef  sfr_ITC
#define sfr_ITC   (*((volatile ITC_t*) 0x50a0))
#undef  sfr_IWDG
#define sfr_IWDG   (*((volatile IWDG_t*) 0x50e0))
#undef  sfr_OPT
#define sfr_OPT   (*((volatile OPT_t*) 0x4800))
#undef  sfr_PORTA
#define sfr_PORTA   (*((volatile PORT_t*) 0x5000))
#undef  sfr_PORTB
#define sfr_PORTB   (*((volatile PORT_t*) 0x5005))
#undef  sfr_PORTC
#define sfr_PORTC   (*((volatile PORT_t*) 0x500a))
#undef  sfr_PORTD
#define sfr_PORTD   (*((volatile PORT_t*) 0x500f))
#undef  sfr_PORTE
#define sfr_PORTE   (*((volatile PORT_t*) 0x5014))
#undef  sfr_PORTF
#define sfr_PORTF   (*((volatile PORT_t*) 0x5019))
#undef  sfr_SPI
#define sfr_SPI   (*((volatile SPI_t*) 0x5200))
#undef  sfr_SWIM
#define sfr_SWIM   (*((volatile SWIM_t*) 0x7f80))
#undef  sfr_TIM1
#define sfr_TIM1   (*((volatile TIM1_t*) 0x5250))
#undef  sfr_TIM2
#define sfr_TIM2   (*((volatile TIM2_t*) 0x5300))
#undef  sfr_TIM4
#define sfr_TIM4   (*((volatile TIM4_t*) 0x5340))
#undef  sfr_UART1
#define sfr_UART1   (*((volatile UART1_t*) 0x5230))
#undef  sfr_WWDG
#define sfr_WWDG   (*((volatile WWDG_t*) 0x50d1))

#endif  // _STM8S003F3_VOLATILE_H_
