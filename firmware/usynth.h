#ifndef _USYNTH_INTERNAL_H_
#define _USYNTH_INTERNAL_H_

#include <inttypes.h>
#include <limits.h>

typedef volatile struct env_s{
	// control variables
	uint8_t attack, decay, sustain, release; 

	// output variables
	int16_t volume; 
	
	// private variables
	uint16_t time; 
	uint16_t a_dx, d_dx, r_dx; 
}env_t;

typedef volatile struct osc_s{
	// controls
	uint8_t		detune; // detune ammount 
	int16_t 		pitch; // pitch offset in semitones
	uint8_t 		fade; // amount of fading from reset
	uint8_t 		lfo; // lfo ammount
	int16_t		(*waveform)(uint8_t phase); // waveform 
	
	// outputs
	int16_t 		output; 
	
	// internal variables
	uint16_t		phase_dx;
	uint16_t 	phase_acc; 
	uint8_t 		phase_offset;
	
	uint16_t		amp_acc; 
}osc_t; 

typedef volatile struct fil_s{
	// control variables 
	uint8_t		cutoff; 
	uint8_t 		resonance; 
	uint8_t		decay; 
	uint8_t		lfo; // lfo amount
	
	// internal variables
	int32_t	 	acc; 
}fil_t; 

typedef volatile struct ins_s{
	
}ins_t; 

typedef volatile struct amp_s {
	uint8_t 		level; 
} amp_t; 

/*
typedef volatile struct eft_s {
	int16_t 		buf[33];
	uint16_t 	phase; 
	uint16_t 	phase_dx; 
	int16_t 		(*run)(volatile struct eft_s *eft, int16_t sample); 
	void 				(*reset)(volatile struct eft_s *eft); 
} eft_t; 

void EFT_KarplusInit(eft_t *eft);
*/

typedef volatile struct synth_s{
	osc_t 			osc1, osc2; 
	int8_t			mix; // mix amount
	osc_t				lfo; 
	fil_t				lowpass; 
	env_t				envelope; 
	env_t				filter_env; 
	amp_t				amp; 
	//eft_t				effect; 
}synth_t; 

void			U_Init(synth_t *synth);
void 			U_PlayNote(synth_t *synth, uint8_t note, uint8_t octave, int8_t kind); 
void 			U_PlayNoteRaw(synth_t *s, uint8_t note);
uint8_t 	U_GenSample(synth_t *synth);

#endif
