#ifndef _HC595_H_
#define	_HC595_H_

#ifdef	__cplusplus
extern "C" {
#endif

void hc595Init (void);
#define HC595_CHAIN		5						// bytes in the chain (5 x 74HC595)
void hc595ChainShiftOut (uint8_t *data);
#ifdef	__cplusplus
}
#endif

#endif	/* _HC595_H_ */