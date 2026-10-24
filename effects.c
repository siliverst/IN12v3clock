#include "STM8S003F3.h"
#include "effects.h"
#include "display.h"
#include "iface.h"
#include "eeprom.h"
#include <string.h>

/*
Digits in the time display: every 10ms fx_10ms() moves the shown digits to the time digits
by the minute change effect (e.flip) or by antipoisoning (e.apType), and makes the glitches.
Effects are taken from NixieClock v2 by AlexGyver (flip effects, burn, glitches), rewritten for the static
indication of this board (a blank tube is NIXIE_OFF, the dim digits are the CC3 frame of the display).
In the setup modes fx does nothing, after them the time is shown at once.
*/
enum {FX_NONE, FX_FLIP, FX_AP, FX_HOLD};

#define HOLD_TICKS			150		// 10ms ticks: the effect number is shown before the demo, a new click restarts it
#define FADE_STEPS			25		// fade effect: steps down and up (the square below needs 25)
#define FADE_MIN_US			30		// fade effect: the shortest pulse, us: the neon does not glow steadily with shorter ones
#define AP_DELAY				2			// antipoisoning: steps between the starts of the digits
#define AP_STEP_TICKS		10		// antipoisoning: 10ms ticks in a step (the waves)
#define AP_NOISE_STEPS	50		// antipoisoning noise: 10ms steps, all cathodes 5 times

static uint8_t shown[NIXIE_COUNT];		// digits on the tubes
static uint8_t target[NIXIE_COUNT];		// time digits
static uint8_t fxMode;
static uint8_t fxNext;								// FX_HOLD: what to run after it
static uint8_t fxTick;								// 10ms ticks in the step
static uint8_t fxStep;
static uint8_t fxMask;								// flip: digits to change
static uint8_t fxFull;								// digits at full brightness
static uint8_t fxK, fxP;							// elastic: moving digit, its position
static uint8_t fxInSetup = 1;					// the time is shown at once after setup and at start

// 10ms ticks of a step of the flip effects 1..5: fade, roll by numbers, roll by cathodes, train, elastic
static const uint8_t flipTicks[FLIP_MAX + 1] = {0, 2, 5, 4, 8, 7};
// cathodes of IN-12 from the front to the back
static const uint8_t cath[10] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3};

static uint8_t apSteps[NIXIE_COUNT];	// antipoisoning: steps left
static uint8_t apTick;								// antipoisoning: steps from the start

static uint16_t rnd = 0xACE1;
static uint16_t glitchWait = 3000;		// 10ms ticks to the next glitch
static uint8_t glitchCount;						// toggles left
static uint8_t glitchTimer;						// 10ms ticks to the next toggle
static uint8_t glitchDig;
static uint8_t glitchOff;

static void flipStart(void);

// xorshift16, 0..n-1
static uint8_t random(uint8_t n)
{
	rnd ^= rnd << 7;
	rnd ^= rnd >> 9;
	rnd ^= rnd << 8;
	return (uint8_t)(rnd % n);
}

static uint8_t cathPos(uint8_t d)
{
	uint8_t p = 0;

	while (cath[p] != d) {p++;}
	return p;
}

static uint8_t dec10(uint8_t v)
{
	return (v ? v : 10) - 1;
}

// blank (leading zero) rolls as 0
static uint8_t digit(uint8_t d)
{
	return (d > 9) ? 0 : d;
}

// the rolling value of the digit: its number or its cathode position
static uint8_t apMap(uint8_t d)
{
	return (e.apType == 1) ? cathPos(d) : d;
}

static uint8_t apUnmap(uint8_t v)
{
	return (e.apType == 1) ? cath[v] : v;
}

/*
Antipoisoning from the shown digits.
0, 1 - waves: every digit rolls down (by numbers / by cathodes from the back to the front) from the shown
       digit to the time digit through all 10 cathodes. All digits make the same number of steps (9 or 10),
       so they finish one after another, as they start:
         - the time digit is skipped on the way and is shown at the last step;
         - old != new: the 8 other digits + new = 9 steps (3 -> 2 1 0 9 8 7 6 4 5);
         - old == new: the 9 other digits + new = 10 steps (7 -> 6 5 4 3 2 1 0 9 8 7).
       The time digit is taken at every step, so the end is right if the minute changes during the run.
2    - noise: all digits roll together every 10ms, 5 rounds.
The rolling digits are at full brightness.
*/
void fxApStart(void)
{
	uint8_t d, t;

	for (d = 0; d < NIXIE_COUNT; d++){
		t = digit(target[d]);
		if (shown[d] > 9) {shown[d] = t;}																	// blank tube starts from its digit
		apSteps[d] = (e.apType == 2) ? AP_NOISE_STEPS : (shown[d] == t) ? 10 : 9;
	}
	apTick = 0;
	fxTick = 0;
	fxMode = FX_AP;
	displayDim(0, 0);
}

static void apProc(void)
{
	uint8_t d, v;
	uint8_t noise = (e.apType == 2);

	if (!noise && (++fxTick < AP_STEP_TICKS)) {return;}
	fxTick = 0;
	fxFull = 0;
	for (d = 0; d < NIXIE_COUNT; d++){
		if (apSteps[d] && (noise || (apTick >= d * AP_DELAY))){
			if (--apSteps[d]){
				v = dec10(apMap(shown[d]));
				if (!noise && (v == apMap(digit(target[d])))) {v = dec10(v);}
				shown[d] = apUnmap(v);
				fxFull |= (uint8_t)(1 << d);
			}else{
				shown[d] = target[d];
			}
		}
	}
	apTick++;
	if (!(apSteps[0] | apSteps[1] | apSteps[2] | apSteps[3])) {fxMode = FX_NONE;}
}

// minute change effect from the shown digits to the time digits
static void flipStart(void)
{
	uint8_t d;

	fxMask = 0;
	for (d = 0; d < NIXIE_COUNT; d++){
		if (shown[d] != target[d]) {fxMask |= (uint8_t)(1 << d);}
	}
	fxTick = 0;
	fxStep = 0;
	fxK = 0;
	fxP = 0;
	fxMode = FX_FLIP;
	if (!e.flip) {
		memcpy(shown, target, sizeof(shown));
		fxMode = FX_NONE;
	}
}

// elastic: the digits fly out to the right one by one, then the new ones fly in from the left
static uint8_t elasticStep(void)
{
	if (fxStep < 10){																// out: the digit fxK goes to position 0 and disappears
		if (fxP == 0){
			shown[0] = NIXIE_OFF;
			fxP = ++fxK;
		}else{
			shown[fxP - 1] = shown[fxP];
			shown[fxP--] = NIXIE_OFF;
		}
		if (fxK == NIXIE_COUNT){
			fxK = 0;
			fxP = NIXIE_COUNT;
		}
	}else{																					// in: the digit fxK comes from position 3 to fxK
		if (fxP == NIXIE_COUNT){
			shown[--fxP] = target[fxK];
		}else{
			shown[fxP - 1] = shown[fxP];
			shown[fxP--] = NIXIE_OFF;
		}
		if (fxP == fxK){
			fxK++;
			fxP = NIXIE_COUNT;
		}
	}
	return (fxK == NIXIE_COUNT) && (fxStep >= 10);
}

/*
fade: the on time of the changing digits, timer ticks, k = 0..25: 0 - off, 1..25 - from FADE_MIN_US to the
digits brightness by the square (smooth for the eye). Ticks: 1, 2, 4 in 1us at 100, 200, 400Hz.
*/
static uint16_t fadeLevel(uint8_t k)
{
	uint16_t full = (uint16_t)displayGetBright() * 100;
	uint16_t min = FADE_MIN_US << e.pwm;
	
	if (!k) {return 0;}
	if (full <= min) {return full;}
	return min + (full - min) / 25 * k / 25 * k;	// (full - min) * k^2 / 625 in 16 bits
}

static void flipProc(void)
{
	uint8_t d, a, b, k, end = 0;

	if (++fxTick < flipTicks[e.flip]) {return;}
	fxTick = 0;
	switch (e.flip){
		case 1:																				// fade out, change, fade in (square: smooth for the eye)
			k = (fxStep < FADE_STEPS) ? FADE_STEPS - 1 - fxStep : fxStep - FADE_STEPS + 1;
			if (fxStep == FADE_STEPS - 1) {memcpy(shown, target, sizeof(shown));}
			end = (fxStep == 2*FADE_STEPS - 1);
			displayDim(end ? 0 : fxMask, fadeLevel(k));
			break;

		case 2:																				// roll down by numbers
		case 3:																				// roll by cathodes
			for (d = 0; d < NIXIE_COUNT; d++){
				if (fxMask & (1 << d)){
					a = digit(shown[d]);
					b = digit(target[d]);
					if (e.flip == 2){
						if (a != b) {a = dec10(a);}
					}else{
						k = cathPos(a);
						if (k < cathPos(b)) {k++;}
						else if (k > cathPos(b)) {k--;}
						a = cath[k];
					}
					shown[d] = a;
					if (a == b){
						shown[d] = target[d];
						fxMask &= (uint8_t)~(1 << d);
					}
				}
			}
			end = !fxMask;
			break;

		case 4:																				// train: goes out to the right, the new one comes from the left
			for (d = 0; d < NIXIE_COUNT - 1; d++) {shown[d] = shown[d + 1];}
			shown[NIXIE_COUNT - 1] = (fxStep < NIXIE_COUNT) ? NIXIE_OFF : target[fxStep - NIXIE_COUNT];
			end = (fxStep == 2*NIXIE_COUNT - 1);
			break;

		default:
			end = elasticStep();
			break;
	}
	fxStep++;
	if (end) {fxMode = FX_NONE;}
}

/*
glitch: a random tube blinks 3..6 times with 20..100ms, every 30..120s, between the seconds 7 and 55
*/
static void glitchProc(void)
{
	if (glitchCount){
		if (--glitchTimer) {return;}
		glitchOff ^= 1;
		if (--glitchCount) {glitchTimer = 2 + 2*random(5);}
		else {glitchOff = 0;}
	}else if (glitchWait){
		glitchWait--;
	}else if (e.glitchEn && (i.seconds > 0x07) && (i.seconds < 0x55)){		// BCD
		glitchWait = 3000 + random(91) * 100;
		glitchDig = random(NIXIE_COUNT);
		glitchCount = 3 + random(4);
		glitchTimer = 2 + 2*random(5);
		glitchOff = 1;
	}
}

/*
show the effect number for 1.5s, then run it: FX_FLIP_DEMO - minute change effect, the number in the left tube,
FX_AP_DEMO - antipoisoning, the number in the right tube. Clicks during the 1.5s just change the number.
*/
void fxDemo(uint8_t next)
{
	memset(shown, NIXIE_OFF, sizeof(shown));
	if (next == FX_AP_DEMO) {shown[0] = e.apType;}
	else {shown[NIXIE_COUNT - 1] = e.flip;}
	fxNext = next;
	fxMode = FX_HOLD;
	fxTick = 0;
	fxFull = 0;
	displayDim(0, 0);
}

// the number of the effect kind (FX_FLIP_DEMO, FX_AP_DEMO) is shown now
uint8_t fxHolding(uint8_t kind)
{
	return (fxMode == FX_HOLD) && (fxNext == kind);
}

void fx_10ms(void)
{
	uint8_t out[NIXIE_COUNT];

	if (i.display_state != SETUP_NO){
		if (!fxInSetup){
			fxInSetup = 1;
			fxMode = FX_NONE;
			fxFull = 0;
			glitchCount = 0;
			glitchOff = 0;
			displayDim(0, 0);
		}
		return;
	}
	iface_time_digits(target, i.minutes, i.hours);
	if (fxInSetup){
		fxInSetup = 0;
		memcpy(shown, target, sizeof(shown));
		rnd ^= (uint16_t)(i.minutes << 8) | i.seconds;	// a different glitch sequence every time
		rnd |= 1;
	}

	switch (fxMode){
		case FX_NONE:
			if (memcmp(shown, target, sizeof(shown))) {flipStart();}
			else {glitchProc();}
			break;
		case FX_HOLD:
			if (++fxTick >= HOLD_TICKS){
				if (fxNext == FX_AP_DEMO) {fxApStart();}
				else {flipStart();}
			}
			break;
		case FX_FLIP:
			flipProc();
			break;
		default:
			apProc();
			break;
	}

	memcpy(out, shown, sizeof(out));
	if ((fxMode == FX_NONE) && glitchOff) {out[glitchDig] = NIXIE_OFF;}
	displayNixie(out, fxFull);
}
