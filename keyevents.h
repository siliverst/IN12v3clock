#ifndef _KEYEVENTS_H_
#define _KEYEVENTS_H_

#ifdef	__cplusplus
extern "C" {
#endif

#define KEYS_COUNT				2
#define CLICK2_TICKS			4				// 100ms: max pause before the 2nd click of a double click
#ifndef K1_DOUBLE_CLICK
#define K1_DOUBLE_CLICK		1				// 1: K1 double click selects the antipoisoning effect, 0: off (single click at once)
#endif
// bits of the pressed keys (bf)
#define S1               	0
#define S2               	1
#define S1_M               _BV(S1)
#define S2_M               _BV(S2)


typedef struct {
	uint8_t	keys_mirror;							// the last key event from the keyboard buffer
	uint8_t	bf;												// pressed keys, a key is cleared here when its release must be ignored
	uint8_t lp_counter[KEYS_COUNT];		// 100ms ticks of the key press (long press)
	uint8_t kr_counter[KEYS_COUNT];		// 100ms ticks of the key repeat
	uint8_t	click1;										// K1 click in the time display: 100ms ticks left to wait for the 2nd click
}Keyevents;



void keyevents_proc ( void );
void keyevents_counters ( void );

#ifdef	__cplusplus
}
#endif

#endif	/* _KEYEVENTS_H_ */