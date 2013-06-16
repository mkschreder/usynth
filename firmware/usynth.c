
#include "usynth.h"

#define BUFSIZE 1024

#define INCREMENT_FROM_FREQ(q, rate) (uint16_t)((COUNTER_MAX / rate) * (q)) 


#define MIN(x, y) (((x) < (y))?(x):(y))
#ifndef MAX
#define MAX(x, y) (((x) > (y))?(x):(y))
#endif

int32_t LIMIT(int32_t bottom, int32_t top, int32_t variable){
	return MAX(bottom, MIN(top, variable));
}

int16_t VOLUME(int16_t sample, uint8_t vol){
	return LIMIT(CHAR_MIN, CHAR_MAX, (sample * vol) / 256); 
}

int16_t SCALE(int16_t a, uint8_t parts){
	int32_t mul = (int32_t)a * parts; 
	return mul / 256;
}

int16_t MIX(int16_t a, int16_t b){
	return LIMIT(-128, 127, a + b);
}

#ifdef __linux__
#define PROGMEM PSTORE
#define PSTORE
#else
//#define PROGMEM
#define PSTORE 
//#define U_AVR
#include <avr/pgmspace.h>
#endif 

static const int8_t sineTable[256] PSTORE = {0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116, 117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118, 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92, 90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51, 49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3, 0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46, -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88, -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116, -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118, -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100, -98, -96, -94, -92, -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51, -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12, -9, -6, -3 };

/*
static const uint8_t expTable[64] PSTORE = {255, 235, 217, 201, 186, 171, 158, 146, 135, 125, 115, 107, 98, 91, 84, 77, 71, 66, 61, 56, 52, 48, 44, 41, 37, 35, 32, 29, 27, 25, 23, 21, 19, 18, 16, 15, 14, 13, 11, 11, 10, 9, 8, 7, 7, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0};

// linear approximation between the missing points
// input value 0..255 - output = value 0..255
static const uint8_t EXP(uint8_t x){
	//return exp(-x/40.f) * 255; 
	uint8_t i = LIMIT(0, 63, (x >> 2));
	uint8_t dy = (expTable[i] - expTable[i + 1]);
	uint8_t i2 = ((x >> 1) & 1)?(dy >> 1):0;
	uint8_t i1 = (x & 1)?(dy >> 2):0;
	return expTable[i] - i2 - i1; 
}*/

static const int note_freq[] PROGMEM = {
// c    c#   d   d#    e    f   f#    g    g#   a   a#     b
	16 , 17,  18 , 20,  21 , 22 , 23,  25 , 26,  28 , 29,  31 , //0	
	33 , 34,  37 , 39,  41 , 44 , 46,  49 , 52,  55 , 58,  62 ,	//1
	65 , 68,  73 , 78,  82 , 87 , 93,  98 , 104, 110, 117, 124,	//2
	131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, //3
	262, 278, 294, 312, 330, 350, 370, 392, 416, 440, 466, 494, //4
	524, 556, 588, 624, 660, 700, 740, 784, 832, 880, 932, 988, //5
	1048,1112,1176,1248,1320,1400,1480,1568,1664,1760,1865,1976 //6
};

static const uint16_t NOTE_INDEX(int note){
	return (((note) >= 0 && (note) < 2)?((note) * 2):(((note) >= 2 && (note) < 5)?((note) * 2 - 1):((note) * 2 - 2)));
}

static const uint16_t S_FrequencyFromNote(uint8_t note, uint8_t octave, int8_t detune, int16_t fine_tune){
	int n = note;
	int d = octave * 12 + ((NOTE_INDEX(n) + 9) % 12);
#ifndef __linux__
	uint16_t fq = (pgm_read_word_near(&note_freq[d + detune]) + fine_tune);
#else
	uint16_t fq = note_freq[d + detune] + fine_tune;
#endif
	return fq;
}

static const uint16_t S_FrequencyFromIndex(uint8_t idx){
#ifndef __linux__
	return (pgm_read_word_near(&note_freq[idx]));
#else
	return note_freq[idx];
#endif
}

static inline int16_t SIN(uint8_t phase) {
#ifdef U_AVR
	return (int16_t)pgm_read_byte_near(&sineTable[phase & 0xff]);
#else
	return sineTable[phase & 0xff];
#endif
}

static inline int16_t SQUARE(uint8_t phase){
	return ((phase < 0x80)?127:-127);
}

static inline int16_t SAWR(uint8_t phase){
	return phase - 128;
}

static inline int16_t SAWL(uint8_t phase){
	return 127 - phase;
}

static inline int16_t TRIANGLE(uint8_t phase){
	int p = phase; 
	p -= 127;
	int w = (p < 0)?-p:p;
	// need this cap
	w = (w > 127)?127:w;
	w = (w < -127)?-127:w;
	return (w * 2) - 127; // - 127
}

static inline int16_t NOISE(uint8_t phase){
	static uint32_t g_seed; 
	g_seed = (214013*g_seed+2531011); 
  int8_t r = (g_seed>>16)&0x7FFF; 
  return r; 
}

static int16_t FI_SimpleLP_Fast(fil_t *fi, int16_t sample){
	int8_t s = ((int8_t)sample - ((fi->acc / 256) * (uint16_t)fi->resonance) / 256); // feedback
	int32_t dy = ((s * 128) - fi->acc);
	fi->acc += ((dy * fi->cutoff) / 256); //(uint16_t)fi->cutoff); 
	return (fi->acc / 128);
}

#define MIN(x, y) (((x) < (y))?(x):(y))
#define MAX(x, y) (((x) > (y))?(x):(y))


// the Schroder filter - a fast, enhanced lowpass filter
/*static int16_t FI_Schroder(fil_t *fi, int16_t sample) {
	 if(sample - fi->pos < 0){
		fi->vel -= (255 - fi->cutoff) >> 1; 
		fi->pos += fi->vel / 128; 
		sample = fi->pos; 
	} else {
		fi->vel = 0; 
		fi->pos = sample; 
	}
	
	// lowpass
	sample = FI_SimpleLP_Fast(fi, sample); 
	
	static long min = -127; 
	static long max = 127; 
	min = MIN(min, sample * 128); 
	max = MAX(max, sample * 128); 
	min++; 
	max--; 
	long zero = ((min+max) / 2) / 128;
	sample -= zero; 
	
	return sample; 
}*/

/*
static int8_t FI_SimpleHP_Fast(fil_t *fi, int8_t sample){
	uint8_t s = sample + 127;
	return ((s - (FI_SimpleLP_Fast(fi, sample) + 127)) / 2) - 128;
}
*/

static int8_t FI_Distortion(fil_t *fi, int8_t x){
	float amount = 0.60; 
	float in = ((float)x)/127.f; 
	float k = 2*amount/(1-amount);

	return 127 * (1+k)*in/(1+k*((in >= 0)?in:-in));
}


static void OSC_Process(osc_t *osc) {
	if(osc->waveform){
		osc->output = osc->waveform(((osc->phase_acc >> 8) + (uint16_t)osc->phase_offset) & 0xff); 
		//osc->output = ((osc->amp_acc >> 9) * osc->output) / 128; 
	}
	osc->phase_acc += osc->phase_dx; 
	
	// fade the oscillator 
	// osc->amp_acc -= osc->amp_acc >> osc->fade; 
}

static void OSC_Reset(osc_t *osc){
	osc->phase_acc = 0; 
}

static void ENV_Reset(env_t *env){
	//env->time = 0; 
	env->pressed = 0; 
	env->volume = 0; 
	env->state = 0; 
	
	env->a_acc = 0; //UINT16_MAX - (env->volume << 8); //UINT16_MAX;
	env->d_acc = 0; //UINT16_MAX;
	env->r_acc = 0; //UINT16_MAX; 
}

static void ENV_Recalculate(env_t *env){
	env->a_dx = (!env->attack)?SAMPLES_PER_SECOND:SAMPLES_PER_SECOND / env->attack;
	env->d_dx = (!env->decay)?SAMPLES_PER_SECOND:SAMPLES_PER_SECOND / env->decay;
	env->d_dx = SCALE(env->d_dx, 255 - env->sustain); 
	env->r_dx = (!env->release)?SAMPLES_PER_SECOND:SAMPLES_PER_SECOND / env->release;
	env->r_dx = SCALE(env->r_dx, env->sustain); 
}

static void ENV_SetAttackTime(env_t *env, uint8_t a){
	env->attack = a;
	ENV_Recalculate(env); 
}

static void ENV_SetDecayTime(env_t *env, uint8_t d){
	env->decay = d;
	ENV_Recalculate(env); 
}

static void ENV_SetSustainLevel(env_t *env, uint8_t s){
	env->sustain = s; 
	ENV_Recalculate(env); 
}

static void ENV_SetReleaseTime(env_t *env, uint8_t r){
	env->release = r;
	ENV_Recalculate(env); 
}

/*
static void ENV_Setup(env_t *env, uint8_t a, uint8_t d, uint8_t s, uint8_t r){
	env->attack = a; env->decay = d; env->release = r;
	env->a_dx = UINT16_MAX/a;
	env->d_dx = UINT16_MAX/d;
	env->r_dx = UINT16_MAX/r;
	env->sustain = s; 
}
*/

// should be called at 64 times per second relative to sample rate
static void ENV_Process(env_t *env, uint8_t pressed){
	if(env->state == 0){
		if(UINT16_MAX - env->a_dx < env->a_acc) env->a_acc = UINT16_MAX; 
		else env->a_acc = env->a_acc + env->a_dx; 
		env->volume = (env->a_acc >> 8); 
		if(env->a_acc == UINT16_MAX) env->state = 1; 
	} 
	else if(env->state == 1){ // decay
		if(UINT16_MAX - env->d_dx < env->d_acc) env->d_acc = UINT16_MAX; 
		else env->d_acc = env->d_acc + env->d_dx; 
		//env->d_acc = LIMIT(0, UINT16_MAX, env->d_acc + env->d_dx); 
		env->volume = 255 - (env->d_acc >> 8); 
		if(env->volume < env->sustain){
			env->state = 2; 
			env->volume = env->sustain; 
		}
	}
	else if(env->state == 2){ // sustain
		if(!env->pressed) env->state = 3; 
	}
	else if(env->state == 3){
		if(UINT16_MAX - env->r_dx < env->r_acc) env->r_acc = UINT16_MAX; 
		else env->r_acc = env->r_acc + env->r_dx; 
		//env->r_acc = LIMIT(0, UINT16_MAX, env->r_acc + env->r_dx); 
		env->volume = LIMIT(0, 255, env->sustain - (env->r_acc >> 8)); 
		if(env->volume == 0){
			env->state = 4; 
		}
	}
}

void U_Init(synth_t *s, uint16_t sample_rate){
	// first oscillator
	s->osc1.detune = 0; 
	s->osc1.fine_tune = 0; 
	s->osc1.waveform = NOISE; 
	s->osc1.phase_dx = 0; 
	s->osc1.phase_offset = 0; 
	
	s->osc2.detune = 0; 
	s->osc2.fine_tune = 0; 
	s->osc2.waveform = NOISE; 
	s->osc2.phase_dx = 0; 
	s->osc2.phase_offset = 0; 
	
	// sinusoidal lfo
	s->lfo_speed = 0; 
	s->lfo_osc_amount = 0; 
	s->lfo_filt_amount = 0; 
	s->lfo_amp_amount = 0; 
	s->lfo.waveform = SIN; 
	s->lfo.phase_dx = 0; 
	s->lfo.phase_offset = 0; 
	
	s->mix = 128; 
	
	s->filter.cutoff = 255; 
	
	s->sample_rate = sample_rate;
	s->increment_per_herz = UINT_MAX / sample_rate; 
	
	s->amp.level = 255; 
	
}

void U_PlayNoteRaw(synth_t *s, uint8_t note){
	ENV_Reset(&s->envelope); 
	ENV_Reset(&s->filter_env); 
	
	OSC_Reset(&s->osc1);
	OSC_Reset(&s->osc2); 
	OSC_Reset(&s->lfo); 
	
	uint16_t fq = S_FrequencyFromIndex(note + s->osc1.detune);
	s->osc1.phase_dx = (fq + s->osc1.fine_tune) * s->increment_per_herz; 
	
	fq = S_FrequencyFromIndex(note + s->osc2.detune);
	s->osc2.phase_dx = (fq + s->osc2.fine_tune) * s->increment_per_herz; 
	
	s->envelope.pressed = 1; 
}

void U_PlayNote(synth_t *s, uint8_t note, uint8_t octave, int8_t kind){
	ENV_Reset(&s->envelope); 
	ENV_Reset(&s->filter_env); 
	
	OSC_Reset(&s->osc1);
	OSC_Reset(&s->osc2); 
	OSC_Reset(&s->lfo); 
	
	//s->effect.reset(&s->effect); 
	
	uint16_t fq = S_FrequencyFromNote(note, octave, kind + s->osc1.detune, s->osc1.fine_tune);
	s->osc1.phase_dx = s->increment_per_herz * fq; 
	
	fq = S_FrequencyFromNote(note, octave, kind + s->osc2.detune, s->osc2.fine_tune);
	s->osc2.phase_dx = fq * s->increment_per_herz; 
	
	s->envelope.pressed = 1; 
}

void U_ReleaseNoteRaw(synth_t *s, uint8_t note){
	s->envelope.pressed = 0; 
}

void U_ReleaseNote(synth_t *s, uint8_t note, uint8_t octave, uint8_t velocity){
	s->envelope.pressed = 0; 
}

void U_SetKnob(synth_t *synth, uint8_t knob, int8_t value){
	switch(knob){
		case KB_OSC1_WAVEFORM: 
			if(value == WAVE_SIN)
				synth->osc1.waveform = SIN; 
			else if(value == WAVE_SQUARE)
				synth->osc1.waveform = SQUARE;
			else if(value == WAVE_SAWL)
				synth->osc1.waveform = SAWL;
			else if(value == WAVE_SAWR)
				synth->osc1.waveform = SAWR;
			else if(value == WAVE_TRIANGLE)
				synth->osc1.waveform = TRIANGLE;
			else if(value == WAVE_NOISE)
				synth->osc1.waveform = NOISE; 
			break; 
		case KB_OSC1_DETUNE:
			synth->osc1.detune = value;
			break; 
		case KB_OSC1_FINE_TUNE: 
			synth->osc1.fine_tune = value; 
			break; 
		case KB_OSC1_PHASE_OFFSET: 
			synth->osc1.phase_offset = value; 
			break;
		case KB_OSC2_WAVEFORM: 
			if(value == WAVE_SIN)
				synth->osc2.waveform = SIN; 
			else if(value == WAVE_SQUARE)
				synth->osc2.waveform = SQUARE;
			else if(value == WAVE_SAWL)
				synth->osc2.waveform = SAWL;
			else if(value == WAVE_SAWR)
				synth->osc2.waveform = SAWR;
			else if(value == WAVE_TRIANGLE)
				synth->osc2.waveform = TRIANGLE;
			else if(value == WAVE_NOISE)
				synth->osc2.waveform = NOISE; 
			break; 
		case KB_OSC2_DETUNE: 
			synth->osc2.detune = value; 
			break; 
		case KB_OSC2_FINE_TUNE: 
			synth->osc2.fine_tune = value; 
			break; 
		case KB_OSC2_PHASE_OFFSET: 
			synth->osc2.phase_offset = value; 
			break;
		case KB_OSC_MIX_AMOUNT: 
			synth->mix = value; 
			break; 
		case KB_LFO_SPEED: 
			synth->lfo_speed = LIMIT(0, 127, value); 
			break; 
		case KB_LFO_TO_OSC: 
			synth->lfo_osc_amount = LIMIT(0, 255, value * 2); 
			break; 
		case KB_LFO_TO_FILTER: 
			synth->lfo_filt_amount = LIMIT(0, 255, value * 2); 
			break; 
		case KB_LFO_TO_AMP: 
			synth->lfo_amp_amount = LIMIT(0, 255, value * 2); 
			break; 
		case KB_AMP_ENV_ATTACK: 
			ENV_SetAttackTime(&synth->envelope, LIMIT(0, 255, value * 2));
			break; 
		case KB_AMP_ENV_DECAY: 
			ENV_SetDecayTime(&synth->envelope, LIMIT(0, 255, value * 2));
			break; 
		case KB_AMP_ENV_SUSTAIN: 
			ENV_SetSustainLevel(&synth->envelope, LIMIT(0, 255, value * 2));
			break; 
		case KB_AMP_ENV_RELEASE: 
			ENV_SetReleaseTime(&synth->envelope, LIMIT(0, 255, value * 2));
			break; 
		case KB_FILTER_CUTOFF: 
			synth->preset_cutoff = LIMIT(0, 255, value * 2); 
			break; 
		case KB_FILTER_ENV_AMOUNT: 
			break; 
		case KB_FILTER_ATTACK: 
		case KB_FILTER_DECAY: 
		case KB_FILTER_SUSTAIN: 
		case KB_FILTER_RELEASE: 
			break; 
		case KB_AMP_VOLUME: 
			synth->preset_amp_level = LIMIT(0, 255, value * 2); 
			break; 
	}
}

// should be called once for each sample at the rate of samplerate
uint8_t U_GenSample(synth_t *synth){
	static uint16_t counter = 0; 
	
	// run envelope update every sample_rate/16 times per second
	if((counter & 0xf) == 0){
		ENV_Process(&synth->envelope, 1);
		ENV_Process(&synth->filter_env, 0); 
	}
	counter++; 
	
	// run the oscillators 
	OSC_Process(&synth->osc1); 
	OSC_Process(&synth->osc2); 
	OSC_Process(&synth->lfo); 
	
	// set lfo frequency
	synth->lfo.phase_dx = synth->lfo_speed * synth->increment_per_herz;
	
	int16_t osc1_out = synth->osc1.output; 
	int16_t osc2_out = synth->osc2.output;
	
	// mix the output from the oscillators 
	uint8_t osc1_vol = 255 - (uint8_t)(127 - synth->mix); 
	uint8_t osc2_vol = (uint8_t)(127 - synth->mix); 
	
	int16_t sample = MIX(VOLUME(osc1_out, osc1_vol), VOLUME(osc2_out, osc2_vol)); 
	
	// apply lfo oscillation
	int8_t lfo_to_filt = VOLUME(synth->lfo.output, synth->lfo_filt_amount);
	int8_t lfo_to_osc 	= VOLUME(synth->lfo.output, synth->lfo_osc_amount);
	int8_t lfo_to_amp 	= VOLUME(synth->lfo.output, synth->lfo_amp_amount);
	synth->osc1.phase_acc += synth->increment_per_herz * lfo_to_osc; 
	synth->osc2.phase_acc += synth->increment_per_herz * lfo_to_osc; 
	
	// adjust filter 
	synth->filter.cutoff = LIMIT(0, 255, synth->preset_cutoff + lfo_to_filt); 
	
	// adjust amplifier
	synth->amp.level = LIMIT(0, 255, synth->preset_amp_level + lfo_to_amp); 
	
	// pass through the effect buffer
	//sample = synth->effect.run(&synth->effect, sample); 
	
	// filter the signal through the filters
	sample = FI_SimpleLP_Fast(&synth->filter, sample);
	//sample = FI_Distortion(&synth->filter, sample); 
	//sample = FI_Schroder(&synth->filter, sample);
	//sample = FI_Distortion(0, sample); 
	
	// amplify the signal with the output amp
	sample = VOLUME(sample, synth->amp.level); 
	
	// apply the final sound envelope
	sample = VOLUME(sample, synth->envelope.volume); 
	
	// clamp sample to the limits
	sample = LIMIT(-128, 127, sample); 
	
	// return final sample
	return (int8_t)sample + 128; 
}
