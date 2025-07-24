#include "STM8S003F3.h"
#include "hc595.h"
#include "display.h"
#include "rgb.h"
#include "macro.h"
#include "eeprom.h"
#include <string.h>

static volatile	uint8_t zero_data[5] 							= {0,0,0,0,0};
static volatile uint8_t disp_data[5]							= {0,0,0,0,0};
static 					uint8_t dotPulseCounter;
			 volatile uint8_t flag10ms;
static 					uint16_t displayBright;
static volatile uint8_t	bright0;

static uint8_t *displayNixieBuffPrepare(uint8_t *inbuff, uint8_t dmask);
static void displaySetDotBright( uint8_t bright);

void displayInit ( void )
{
	// configure DOT_PIN as output
	sfr_PORTD.DDR.DDR6 = 1;     // input(=0) or output(=1)
	sfr_PORTD.CR1.C16  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTD.CR2.C26  = 1;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	DOT_PIN = 0;
	
	// configure TIM2 with 100Hz frequency, count to 10000;
  sfr_TIM2.CR1.ARPE = 1;								// use buffered period register to avoid glitches
  sfr_TIM2.PSCR.PSC = 4;								// set TIM2 prescaler fTim = fcpu/2^pre with pre in [0..15]

  
  sfr_TIM2.ARRH.byte = hibyte(10000);		// set reload period = (ARR+1)/fTim
  sfr_TIM2.ARRL.byte = lobyte(10000);
	
	sfr_TIM2.CCR1H.byte = hibyte(1000);		// set PWM channel 1 duty period
	sfr_TIM2.CCR1L.byte = lobyte(1000);
	sfr_TIM2.CCR2H.byte = hibyte(0);			// set PWM channel 2 duty period
	sfr_TIM2.CCR2L.byte = lobyte(0);
	
	sfr_TIM2.CCMR1.OC1PE 	= 1;						// buffer compare register to avoid glitches when changing duty cycle
	sfr_TIM2.CCER1.CC1E 	= 0;						// TIM2 channel 1 output disable
	sfr_TIM2.CCER1.CC2E 	= 0;						// TIM2 channel 2 output disable
	sfr_TIM2.IER.CC1IE 		= 1;						// TIM2 channel 1 compare interrupt enable
	//sfr_TIM2.IER.CC2IE 		= 1;						// TIM2 channel 2 compare interrupt enable
	sfr_TIM2.IER.UIE 			= 1;						// TIM2 update interrupt enable
	sfr_TIM2.CR1.CEN     	= 1;						// start TIM2
	
	hc595Init();
	displaySetBright(100);
	displayDot(0);
	hc595ChainShiftOut(zero_data,sizeof(zero_data));
	
	
	RGBinit();
	displayRGBset(e.rgbGlobalEn);
}

//inbuff -> {minutes,tens of minutes,hours,tens of hours}
//dmask  -> bitmask of digits that have been prepared
static uint8_t *displayNixieBuffPrepare(uint8_t *inbuff, uint8_t dmask)
{
	static uint8_t data[5];
	
	memset(data,0,sizeof(data));
	
	if (bitchk(dmask,0)){
		// minutes
		switch(inbuff[0]){
			case 0:
				bitset(data[4],0);
				break;
			case 1:
				bitset(data[4],1);
				break;
			case 2:
				bitset(data[4],2);
				break;
			case 3:
				bitset(data[4],3);
				break;
			case 4:
				bitset(data[4],4);
				break;
			case 5:
				bitset(data[4],5);
				break;
			case 6:
				bitset(data[4],6);
				break;
			case 7:
				bitset(data[4],7);
				break;
			case 8:
				bitset(data[3],0);
				break;
			case 9:
				bitset(data[3],1);
				break;
			default:
				break;
		}
	}
	if (bitchk(dmask,1)){
		// tens of minutes
		switch(inbuff[1]){
			case 0:
				bitset(data[3],2);
				break;
			case 1:
				bitset(data[3],3);
				break;
			case 2:
				bitset(data[3],4);
				break;
			case 3:
				bitset(data[3],5);
				break;
			case 4:
				bitset(data[3],6);
				break;
			case 5:
				bitset(data[3],7);
				break;
			case 6:
				bitset(data[2],0);
				break;
			case 7:
				bitset(data[2],1);
				break;
			case 8:
				bitset(data[2],2);
				break;
			case 9:
				bitset(data[2],3);
				break;
			default:
			break;
		}
	}
	if (bitchk(dmask,2)){
		// hours
		switch(inbuff[2]){
			case 0:
				bitset(data[2],4);
				break;
			case 1:
				bitset(data[2],5);
				break;
			case 2:
				bitset(data[2],6);
				break;
			case 3:
				bitset(data[2],7);
				break;
			case 4:
				bitset(data[1],0);
				break;
			case 5:
				bitset(data[1],1);
				break;
			case 6:
				bitset(data[1],2);
				break;
			case 7:
				bitset(data[1],3);
				break;
			case 8:
				bitset(data[1],4);
				break;
			case 9:
				bitset(data[1],5);
				break;
			default:
			break;
		}
	}
	if (bitchk(dmask,3)){
		// tens of hours
		switch(inbuff[3]){
			case 0:
				bitset(data[1],6);
				break;
			case 1:
				bitset(data[1],7);
				break;
			case 2:
				bitset(data[0],0);
				break;
			case 3:
				bitset(data[0],1);
				break;
			case 4:
				bitset(data[0],2);
				break;
			case 5:
				bitset(data[0],3);
				break;
			case 6:
				bitset(data[0],4);
				break;
			case 7:
				bitset(data[0],5);
				break;
			case 8:
				bitset(data[0],6);
				break;
			case 9:
				bitset(data[0],7);
				break;
			default:
			break;
		}
	}
	return data;
}

/*uint8_t bright -> 0-100%
*/
void displaySetBright(uint8_t bright)
{
	bright0 = bright;
	if (bright == 0){
		memset(disp_data,0,sizeof(disp_data));
	}else{
		displayBright = scale(bright,100,10000);
		if (displayBright < NIXIE_MIN_BRIGHT){
			displayBright = NIXIE_MIN_BRIGHT;
		}
		sfr_TIM2.CCR1H.byte = hibyte(displayBright);		// set PWM channel 1 duty period
		sfr_TIM2.CCR1L.byte = lobyte(displayBright);
	}
}

/*input: 
uint8_t data[4] -> {minutes,tens of minutes,hours,tens of hours}
*/
void displayNixie(uint8_t *data, uint8_t full_bright_bitmask)
{
	if (bright0){
		memcpy(disp_data,displayNixieBuffPrepare(data,bin(00001111)),sizeof(disp_data));
	}else{
		memcpy(disp_data,displayNixieBuffPrepare(data,full_bright_bitmask),sizeof(disp_data));
	}
	memcpy(zero_data,displayNixieBuffPrepare(data,full_bright_bitmask),sizeof(zero_data));
}

void displayDot (uint8_t state) 
{
	uint16_t bright = displayBright;
	
	if (state == 0) {
		sfr_TIM2.IER.CC2IE 		= 0;							// TIM2 channel 2 compare interrupt disable
		DOT_PIN = 0;
	}else {
		if (bright < DOT_MIN_BRIGHT) {
			bright = DOT_MIN_BRIGHT;
		}
		sfr_TIM2.CCR2H.byte = hibyte(bright);		// set PWM channel 2 duty period
		sfr_TIM2.CCR2L.byte = lobyte(bright);
		sfr_TIM2.IER.CC2IE 		= 1;							// TIM2 channel 2 compare interrupt enable
	}
}

void displayDotPulse (void) 
{
	dotPulseCounter = 0;
}

void displayDotPulseProc (void)
{
	switch (e.colonBlinkingType) {
		case 1:
			if (dotPulseCounter < 50) { 
				displaySetDotBright(50 * 4);
				dotPulseCounter++;
			}
			break;
		case 2:
			displaySetDotBright(0);
			break;
		default:
			if (dotPulseCounter < 25) {
				displaySetDotBright(dotPulseCounter * 4);
			} else if (dotPulseCounter < 50) {
				displaySetDotBright((49 - dotPulseCounter) * 4);
			}
			if (dotPulseCounter < 50) {
				dotPulseCounter++;
			}
			break;
	}
}

static void displaySetDotBright( uint8_t bright)
{
	uint16_t b;
	
	if (bright == 0) {
		sfr_TIM2.IER.CC2IE 		= 0;						// TIM2 channel 2 compare interrupt disable
		DOT_PIN = 0;
	}else {
		b = scale(bright,100,displayBright);
		if (b < DOT_MIN_BRIGHT) {b = DOT_MIN_BRIGHT;}
		sfr_TIM2.CCR2H.byte = hibyte(b);		// set PWM channel 2 duty period
		sfr_TIM2.CCR2L.byte = lobyte(b);
		sfr_TIM2.IER.CC2IE 		= 1;					// TIM2 channel 2 compare interrupt enable
	}
}

ISR_HANDLER (TIM2_UPD_ISR, _TIM2_OVR_UIF_VECTOR_)
{
	hc595ChainShiftOut(disp_data,sizeof(disp_data));
	if (sfr_TIM2.IER.CC2IE)
	{
		DOT_PIN = 1;
	}
	flag10ms = 1;
	sfr_TIM2.SR1.UIF = 0;
  return;
} // TIM2_UPD_ISR


ISR_HANDLER (TIM2_CAP_ISR, _TIM2_CAPCOM_CC1IF_VECTOR_)
{
	//for digits
	if(sfr_TIM2.SR1.CC1IF){
		hc595ChainShiftOut(zero_data,sizeof(zero_data));
		sfr_TIM2.SR1.CC1IF = 0;
	}
	
	//for dot
	if(sfr_TIM2.SR1.CC2IF)
	{
		DOT_PIN = 0;
		sfr_TIM2.SR1.CC2IF = 0;
	}
  return;
} // TIM2_CAP_ISR


void displayRGBset (uint8_t state)
{
	if (state){
		RGBsetR((uint16_t)scale(EEPROM_readByte(R_ADDR),255,displayBright));
		RGBsetG((uint16_t)scale(EEPROM_readByte(G_ADDR),255,displayBright));
		RGBsetB((uint16_t)scale(EEPROM_readByte(B_ADDR),255,displayBright));
	}else{
		RGBsetR(0);
		RGBsetG(0);
		RGBsetB(0);
	}
}

void displayRset(uint8_t value)
{
	RGBsetR((uint16_t)scale(value,255,displayBright));
}

void displayGset(uint8_t value)
{
	RGBsetG((uint16_t)scale(value,255,displayBright));
}

void displayBset(uint8_t value)
{
	RGBsetB((uint16_t)scale(value,255,displayBright));
}