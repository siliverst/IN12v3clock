#ifndef _DISPLAY_H_
#define	_DISPLAY_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "STM8S003F3.h"

#define DOT_PIN						sfr_PORTD.ODR.ODR6
#define NIXIE_COUNT				4
#define NIXIE_OFF					10

extern volatile uint8_t flag10ms;	// set by TIM2 every 10ms


void displayInit ( void );
void displaySetBright(uint8_t bright);
void displayDim(uint8_t mask, uint16_t level);
uint8_t displayGetBright(void);
void displayNixie(uint8_t *data, uint8_t full_bright_bitmask);
void displayDot (uint8_t bright);
void displayDotPulse (uint8_t odd);
void displayWaitLongPhase(void);
void displayDotPulseProc (void);


ISR_HANDLER (TIM2_UPD_ISR, _TIM2_OVR_UIF_VECTOR_);
ISR_HANDLER (TIM2_CAP_ISR, _TIM2_CAPCOM_CC1IF_VECTOR_);

#ifdef	__cplusplus
}
#endif

#endif	/* _DISPLAY_H_ */