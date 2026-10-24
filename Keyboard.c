#include "STM8S003F3.h"
#include "keyboard.h"
#include "macro.h"

#define S1_PIN				sfr_PORTD.IDR.IDR4
#define S2_PIN				sfr_PORTD.IDR.IDR5

Keyboard kbd;
static void insert_button(uint8_t b);


void keyboard_init(void)
{
	// configure S1_PIN as ID input
	sfr_PORTD.DDR.DDR4 = 0;     // input(=0) or output(=1)
	sfr_PORTD.CR1.C14  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTD.CR2.C24  = 0;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTD.ODR.ODR4 = 1;
	// configure S2_PIN as ID input
	sfr_PORTD.DDR.DDR5 = 0;     // input(=0) or output(=1)
	sfr_PORTD.CR1.C15  = 1;     // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
	sfr_PORTD.CR2.C25  = 0;     // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
	sfr_PORTD.ODR.ODR5 = 1;
	
	kbd.S1_reg = 0xFE;
	kbd.S2_reg = 0xFE;
	kbd.HeadPtr = 0;
	kbd.TailPtr = 0;
}

void keyboard_refresh( void )
{
	kbd.S1_reg <<= 1;
	if(!S1_PIN) kbd.S1_reg |= 0x01;
	if(kbd.S1_reg == 0x7F) {insert_button(S1_DOWN);}
	if(kbd.S1_reg == 0x80) {insert_button(S1_UP);}
	
	kbd.S2_reg <<= 1;
	if(!S2_PIN) kbd.S2_reg |= 0x01;
	if(kbd.S2_reg == 0x7F) {insert_button(S2_DOWN);}
	if(kbd.S2_reg == 0x80) {insert_button(S2_UP);}
}


uint8_t keyboard_get_key(void)
{
	uint8_t result;
	if(kbd.HeadPtr == kbd.TailPtr) return 255;
	result = kbd.Buffer[kbd.TailPtr];
	if(++kbd.TailPtr == 8) kbd.TailPtr = 0;
	return(result);
}

static void insert_button(uint8_t b )
{
	kbd.Buffer[kbd.HeadPtr] = b;              
	if(++kbd.HeadPtr == 8) kbd.HeadPtr = 0; 
}
