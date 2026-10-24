#ifndef _EFFECTS_H_
#define	_EFFECTS_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

// fxDemo() argument
#define FX_FLIP_DEMO	0
#define FX_AP_DEMO		1

void fx_10ms(void);
void fxApStart(void);
void fxDemo(uint8_t next);

#ifdef	__cplusplus
}
#endif

#endif	/* _EFFECTS_H_ */
