#include "STM8S003F3.h"
#include "rgb.h"
#include "macro.h"
#include "eeprom.h"

/*
Brightness is perceptually linear: the bright 0..100% is the CIE 1976 lightness L, the table is the luminance
  Y = L / 903.3 (L <= 8), ((L + 16) / 116)^3 (L > 8), scaled to 32768.
The color is linear (duty), so the hue does not change with the brightness.
CCR = color(0..255) * Y >> 8, 0..32640 = the period, 490Hz PWM.
*/
#define RGB_RELOAD_VALUE		32640

static const uint16_t cie[101] = {
	0, 36, 73, 109, 145, 181, 218, 254, 290, 328,
	369, 413, 461, 512, 567, 625, 688, 754, 825, 900,
	979, 1063, 1152, 1245, 1344, 1447, 1555, 1669, 1788, 1913,
	2043, 2180, 2322, 2470, 2624, 2785, 2952, 3125, 3306, 3493,
	3687, 3888, 4096, 4312, 4535, 4765, 5003, 5249, 5503, 5765,
	6035, 6314, 6601, 6896, 7201, 7514, 7836, 8167, 8507, 8856,
	9215, 9584, 9962, 10350, 10748, 11157, 11575, 12004, 12443, 12892,
	13353, 13824, 14306, 14799, 15304, 15820, 16347, 16886, 17437, 17999,
	18573, 19160, 19759, 20370, 20993, 21629, 22278, 22940, 23614, 24302,
	25003, 25717, 26445, 27187, 27942, 28711, 29494, 30291, 31102, 31928,
	32768
};

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


static void RGBccr(volatile uint8_t *ccrh, uint8_t color, uint16_t y)
{
	uint16_t v = (uint16_t)hibyte(y) * color + (((uint16_t)lobyte(y) * color) >> 8);	// (y * color) >> 8 without 32 bit math
	
	ccrh[0] = hibyte(v);
	ccrh[1] = lobyte(v);
}

/*
bright -> 0..100%, color is taken from e.r, e.g, e.b
*/
void RGBset ( uint8_t bright )
{
	uint16_t y = cie[bright];
	
	RGBccr(&sfr_TIM1.CCR3H.byte, e.r, y);
	RGBccr(&sfr_TIM1.CCR1H.byte, e.g, y);
	RGBccr(&sfr_TIM1.CCR4H.byte, e.b, y);
}
