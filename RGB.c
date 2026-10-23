#include "STM8S003F3.h"
#include "rgb.h"
#include "macro.h"
#include "eeprom.h"

// color (0..255) * bright (0..100) fits to the period without division, 627Hz PWM
#define RGB_RELOAD_VALUE		25500

void RGBinit ( void )
{
	// configure R_PIN as output (TIM1_CH3)
	sfr_PORTC.DDR.DDR3 = 1;     // input(=0) or output(=1)
	sfr_PORTC.CR1.C13  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTC.CR2.C23  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTC.ODR.ODR3 = 0;
	// configure G_PIN as output (TIM1_CH1)
	sfr_PORTC.DDR.DDR6 = 1;     // input(=0) or output(=1)
	sfr_PORTC.CR1.C16  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTC.CR2.C26  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTC.ODR.ODR6 = 0;
	// configure B_PIN as output (TIM1_CH4)
	sfr_PORTC.DDR.DDR4 = 1;     // input(=0) or output(=1)
	sfr_PORTC.CR1.C14  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTC.CR2.C24  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTC.ODR.ODR4 = 0;
	
	// init TIM1
  sfr_TIM1.CR1.CMS     = 0;                               // PWM Edge-aligned mode
	sfr_TIM1.CR1.DIR     = 0;                               // Counter used as up-counter
  sfr_TIM1.CR1.ARPE    = 1;                               // buffer period register to avoid glitches when changing frequency
  sfr_TIM1.CR1.OPM     = 0;                               // continuous PWM mode
  sfr_TIM1.PSCRH.byte  = 0;                               // set TIM1 prescaler to 1 --> fTIM1=16MHz
  sfr_TIM1.PSCRL.byte  = 0;
  sfr_TIM1.ARRH.byte   = (uint8_t) (RGB_RELOAD_VALUE >> 8);         // set TIM1 auto-reload value = PWM period
  sfr_TIM1.ARRL.byte   = (uint8_t) RGB_RELOAD_VALUE;
	sfr_TIM1.CCR1H.byte  = 0;
  sfr_TIM1.CCR1L.byte  = 0;
	sfr_TIM1.CCR3H.byte  = 0;
  sfr_TIM1.CCR3L.byte  = 0;
	sfr_TIM1.CCR4H.byte  = 0;
  sfr_TIM1.CCR4L.byte  = 0;
  sfr_TIM1.CCER1.CC1P  = 1;                               // set CH1 active low 
  sfr_TIM1.CCER2.CC3P  = 1;                               // set CH3 active low 
	sfr_TIM1.CCER2.CC4P  = 1;                               // set CH4 active low 
  sfr_TIM1.CCER1.CC1E  = 1;                               // enable CH1 output
  sfr_TIM1.CCER2.CC3E  = 1;                               // enable CH3 output
	sfr_TIM1.CCER2.CC4E  = 1;   														// enable CH4 output
  sfr_TIM1.CCMR1.OC1M  = 7;                               // use PWM mode 2
	sfr_TIM1.CCMR3.OC3M  = 7;
  sfr_TIM1.CCMR4.OC4M  = 7;
  sfr_TIM1.CCMR1.OC1PE = 1;                               // buffer compare register to avoid glitches when changing duty cycle
  sfr_TIM1.CCMR3.OC3PE = 1;
	sfr_TIM1.CCMR4.OC4PE = 1;
  sfr_TIM1.BKR.MOE     = 1;                               // main output enable
  sfr_TIM1.CR1.CEN     = 1;                               // start TIM1
}


/*
bright -> 0..100%, color is taken from e.r, e.g, e.b
*/
void RGBset ( uint8_t bright )
{
	uint16_t v;
	
	v = (uint16_t)e.r * bright;
	sfr_TIM1.CCR3H.byte = hibyte(v);
	sfr_TIM1.CCR3L.byte = lobyte(v);
	v = (uint16_t)e.g * bright;
	sfr_TIM1.CCR1H.byte = hibyte(v);
	sfr_TIM1.CCR1L.byte = lobyte(v);
	v = (uint16_t)e.b * bright;
	sfr_TIM1.CCR4H.byte = hibyte(v);
	sfr_TIM1.CCR4L.byte = lobyte(v);
}
