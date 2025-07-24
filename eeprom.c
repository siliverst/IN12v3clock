#include "STM8S003F3.h"
#include "eeprom.h"

Edata e;

void EEPROM_dataInit(void)
{
	e.rgbGlobalEn = EEPROM_readByte(RGB_EN_ADDR);
	if (e.rgbGlobalEn > 1) {
		e.rgbGlobalEn = 0;
		EEPROM_writeByte(RGB_EN_ADDR,e.rgbGlobalEn);
	}
	
	e.zeroEn = EEPROM_readByte(ZERO_ADDR);
	if (e.zeroEn > 1) {
		e.zeroEn = 0;
		EEPROM_writeByte(ZERO_ADDR,e.zeroEn);
	}
	
	e.f1224 = EEPROM_readByte(F1224_ADDR);
	if (e.f1224) {														//disable, 12h time display is not working yet!
	//if (e.f1224 > 1) {
		e.f1224 = 0;
		EEPROM_writeByte(F1224_ADDR,e.f1224);
	}
	
	e.bright = EEPROM_readByte(BRIGHT_ADDR);
	if ((e.bright > 100)||(e.bright < 5)) {
		e.bright = 100;
		EEPROM_writeByte(BRIGHT_ADDR,e.bright);
	}
	
	e.nBright = EEPROM_readByte(NIGHT_BR_ADDR);
	if (e.nBright > 100) {
		e.nBright = 100;
		EEPROM_writeByte(NIGHT_BR_ADDR,e.nBright);
	}
	
	e.nBrightEn = EEPROM_readByte(NIGHT_BR_EN_ADDR);
	if (e.nBrightEn > 1) {
		e.nBrightEn = 0;
		EEPROM_writeByte(NIGHT_BR_EN_ADDR,e.nBrightEn);
	}
	
	e.nBrightStartH = EEPROM_readByte(NIGHT_BR_START_H_ADDR);
	if (e.f1224){
		if (e.nBrightStartH > 12) {
			e.nBrightStartH = 0;
			EEPROM_writeByte(NIGHT_BR_START_H_ADDR,e.nBrightStartH);
		}
	}else{
		if (e.nBrightStartH > 23) {
			e.nBrightStartH = 0;
			EEPROM_writeByte(NIGHT_BR_START_H_ADDR,e.nBrightStartH);
		}
	}
	
	e.nBrightStartM = EEPROM_readByte(NIGHT_BR_START_M_ADDR);
	if (e.nBrightStartM > 59) {
		e.nBrightStartM = 0;
		EEPROM_writeByte(NIGHT_BR_START_M_ADDR,e.nBrightStartM);
	}
	
	e.nBrightEndH = EEPROM_readByte(NIGHT_BR_STOP_H_ADDR);
	if (e.f1224){
		if (e.nBrightEndH > 12) {
			e.nBrightEndH = 0;
			EEPROM_writeByte(NIGHT_BR_STOP_H_ADDR,e.nBrightEndH);
		}
	}else{
		if (e.nBrightEndH > 23) {
			e.nBrightEndH = 0;
			EEPROM_writeByte(NIGHT_BR_STOP_H_ADDR,e.nBrightEndH);
		}
	}
	
	e.nBrightEndM = EEPROM_readByte(NIGHT_BR_STOP_M_ADDR);
	if (e.nBrightEndM > 59) {
		e.nBrightEndM = 0;
		EEPROM_writeByte(NIGHT_BR_STOP_M_ADDR,e.nBrightEndM);
	}
	
	e.rgbAtNightEn = EEPROM_readByte(NIGHT_RGB_EN_ADDR);
	if (e.rgbAtNightEn > 1){
		e.rgbAtNightEn = 0;
		EEPROM_writeByte(NIGHT_RGB_EN_ADDR,e.rgbAtNightEn);
	}
	
	e.antipoisoningAtNihgtOnly = EEPROM_readByte(ANTIPOISONING_AT_NIGHT_ONLY_ADDR);
	if (e.antipoisoningAtNihgtOnly > 1){
		e.antipoisoningAtNihgtOnly = 0;
		EEPROM_writeByte(ANTIPOISONING_AT_NIGHT_ONLY_ADDR,e.antipoisoningAtNihgtOnly);
	}
	
	e.antipoisoningEffect = EEPROM_readByte(ANTIPOISONING_EFFECT_ADDR);
	if (e.antipoisoningEffect!= 1) {
		e.antipoisoningEffect = 1;	// todo temporally it's only 1
		EEPROM_writeByte(ANTIPOISONING_EFFECT_ADDR, e.antipoisoningEffect);
	}

	e.colonBlinkingType = EEPROM_readByte(COLON_BLINKING_TYPE);
	if (e.colonBlinkingType > 2){
		e.colonBlinkingType = 0;
		EEPROM_writeByte(COLON_BLINKING_TYPE,e.colonBlinkingType);
	}
}

/**
  \fn uint8_t EEPROM_writeByte(uint16_t logAddr, uint8_t data)
  
  \brief write 1B to D-flash / EEPROM
  
  \param[in] logAddr    logical address to write to (starting from EEPROM_ADDR_START)
  \param[in] data       byte to program
  
  \return write successful(=1) or error(=0)

  write single byte to logical address in D-flash / EEPROM
*/
uint8_t EEPROM_writeByte(uint16_t logAddr, uint8_t data) {

  uint16_t   addr = EEPROM_ADDR_START + logAddr;  // physical address 
  uint8_t    countTimeout;                        // timeout counter
  
  // address range check
  if (logAddr > EEPROM_SIZE)
    return(0);
  
  // begin critical cection (disable interrupts)
  DISABLE_INTERRUPTS();
  
  // unlock w/e access to EEPROM
  sfr_FLASH.DUKR.byte = 0xAE;
  sfr_FLASH.DUKR.byte = 0x56;
    
  // wait until access granted
  while(!sfr_FLASH.IAPSR.DUL);
  
  // write byte in 16-bit address range
  *((uint8_t*) addr) = data;

  // wait until done or timeout (normal flash write measured with 0 --> 100 is more than sufficient)
  countTimeout = 100;                                // ~0.95us/inc -> ~0.1ms
  while ((!sfr_FLASH.IAPSR.EOP) && (countTimeout--));
    
  // lock EEPROM again against accidental erase/write
  sfr_FLASH.IAPSR.DUL = 0;
  
  // critical section (enable interrupts)
  ENABLE_INTERRUPTS();

  // write successful -> return 1
  return(countTimeout != 0);

} // EEPROM_writeByte


/**
  \fn uint8_t EEPROM_readByte(uint16_t logAddr)
  
  \brief read 1B from D-flash / EEPROM
  
  \param[in] logAddr    logical address to read from (starting from EEPROM_ADDR_START)
  
  \return read byte (0xFF on error)

  read single byte from logical address in D-flash / EEPROM
*/
uint8_t EEPROM_readByte(uint16_t logAddr) {

  uint16_t   addr = EEPROM_ADDR_START + logAddr;  // physical address 
  
  // address range check
  if (logAddr > EEPROM_SIZE)
    return(0xFF);
  
  // return read data
  return(*((uint8_t*) addr));

} // EEPROM_readByte
