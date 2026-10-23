#ifndef _KEYEVENTS_H_
#define _KEYEVENTS_H_

#ifdef	__cplusplus
extern "C" {
#endif

#define KEYS_COUNT				2
// биты текущего состояния кнопок (bf)
#define S1               	0
#define S2               	1
#define S1_M               _BV(S1)
#define S2_M               _BV(S2)


typedef struct {
	uint8_t	keys_mirror;
	uint8_t	bf;												//buttons flags
	uint8_t lp_counter[KEYS_COUNT];
	uint8_t kr_counter[KEYS_COUNT];		//key repeat counters
}Keyivents;



void keyevents_proc ( void );
void keyevents_counters ( void );

#ifdef	__cplusplus
}
#endif

#endif	/* _KEYEVENTS_H_ */