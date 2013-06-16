#ifndef _USYNTH_INTERNAL_H_
#define _USYNTH_INTERNAL_H_

#include <inttypes.h>
#include <limits.h>
#include "../driver/types.h"

typedef volatile struct env_s{
	// control variables
	uint8_t attack, decay, sustain, release; 

	// output variables
	uint8_t volume; 
	
	// private variables
	uint16_t time; 
	uint16_t a_dx, d_dx, r_dx; 
	uint16_t volume_acc; 
	uint8_t 	pressed; 
}env_t;

typedef volatile struct osc_s{
	// controls
	int8_t			detune; 			// detune ammount in semitones (12 semitones = 1 oct)
	int8_t 		fine_tune; 		// fine tuning in Hz
	int8_t 		phase_offset;
	uint8_t 		lfo; 					// lfo ammount
	int16_t		(*waveform)(uint8_t phase); // waveform 
	
	// outputs
	int16_t 		output; 
	
	// internal variables
	uint16_t		phase_dx;
	uint16_t 	phase_acc; 
	
	//uint16_t		amp_acc; 
}osc_t; 

typedef volatile struct fil_s{
	// control variables 
	uint8_t		cutoff; 
	uint8_t 		resonance; 
	
	// internal variables
	int32_t	 	acc; 
	
	//int16_t 		bounds_min, bounds_max; 
	//int16_t		pos, vel; 
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
	fil_t				filter; 
	env_t				envelope; 
	env_t				filter_env; 
	amp_t				amp; 
	
	osc_t				lfo; 
	uint8_t		lfo_speed; 
	uint8_t 		lfo_osc_amount;
	uint8_t 		lfo_filt_amount; 
	uint8_t 		lfo_amp_amount; 
	
	// knob presets
	uint8_t 		preset_cutoff; 
	uint8_t 		preset_amp_level; 
	
	uint8_t 		sample_rate; 
	uint8_t		increment_per_herz; 
	//eft_t				effect; 
}synth_t; 


void			U_Init(synth_t *synth, uint16_t sample_rate);
void 			U_PlayNote(synth_t *synth, uint8_t note, uint8_t octave, int8_t kind); 
void 			U_PlayNoteRaw(synth_t *s, uint8_t note);
void 			U_ReleaseNote(synth_t *s, uint8_t note, uint8_t octave, uint8_t velocity);
void 			U_ReleaseNoteRaw(synth_t *s, uint8_t note); 
void 			U_SetKnob(synth_t *s, uint8_t knob, int8_t value); 
uint8_t 	U_GenSample(synth_t *synth);

#endif
