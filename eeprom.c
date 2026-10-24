#include "STM8S003F3.h"
#include "eeprom.h"
#include "display.h"

Edata e;

// limits and defaults, in the order of enum eeprom_adreses
const uint8_t eeMax[EE_COUNT] = {1, 1, 100, 100, 100, 100, 1, 23, 59, 23, 59, AP_INTERVAL_MAX, AP_INTERVAL_MAX, 3, 1, 1, 255, 255, 255};
static const uint8_t eeDef[EE_COUNT] = {0, 0, 100, 30, 100, 30, 0, 22, 0, 7, 0, 10, 2, 0, 1, 0, 12, 5, 81};

/*
read all settings from EEPROM to e, wrong values are replaced by defaults.
If the layout magic does not match (empty EEPROM or old layout), all settings are set to defaults.
*/
void EEPROM_dataInit(void)
{
	uint8_t a, v;
	uint8_t reset = (EEPROM_readByte(EE_MAGIC_ADDR) != EE_MAGIC);
	
	for (a = 0; a < EE_COUNT; a++){
		v = EEPROM_readByte(a);
		if (reset || (v > eeMax[a]) || ((a == BRIGHT_ADDR) && (v < BRIGHT_MIN))){
			v = eeDef[a];
			EEPROM_writeByte(a, v);
		}
		EE(a) = v;
	}
	EEPROM_writeByte(EE_MAGIC_ADDR, EE_MAGIC);
}

// set the field of e and save it to EEPROM
void EEPROM_set(uint8_t addr, uint8_t data)
{
	EE(addr) = data;
	EEPROM_writeByte(addr, data);
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
  uint16_t   countTimeout;                        // timeout counter
  
  // address range check
  if (logAddr >= EEPROM_SIZE)
    return(0);
  
  // the same value is already there, do not wear EEPROM
  if (*((uint8_t*) addr) == data)
    return(1);
  
  // interrupts are not touched: the unlock does not need it, and at start they must stay disabled
  displayWaitLongPhase();
  
  // unlock w/e access to EEPROM
  sfr_FLASH.DUKR.byte = 0xAE;
  sfr_FLASH.DUKR.byte = 0x56;
    
  // wait until access granted
  while(!sfr_FLASH.IAPSR.DUL);
  
  // write byte in 16-bit address range
  *((uint8_t*) addr) = data;

  // wait until done or timeout: the byte write takes up to 6ms (with erase), the loop is ~0.5us
  countTimeout = 20000;
  while ((!sfr_FLASH.IAPSR.EOP) && (countTimeout--));
    
  // lock EEPROM again against accidental erase/write
  sfr_FLASH.IAPSR.DUL = 0;
  
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
  if (logAddr >= EEPROM_SIZE)
    return(0xFF);
  
  // return read data
  return(*((uint8_t*) addr));

} // EEPROM_readByte
