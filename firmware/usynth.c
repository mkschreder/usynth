
#include "usynth.h"

#define BUFSIZE 1024

#define COUNTER_MAX 65535
#define COUNTER_MIN 0

#define SAMPLES_PER_SECOND (COUNTER_MAX / 4)
#define INCREMENT_FROM_FREQ(q) (uint16_t)(4 * (q)) 

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


static const uint8_t expTable[256] PSTORE = {255, 235, 217, 201, 186, 171, 158, 146, 135, 125, 115, 107, 98, 91, 84, 77, 71, 66, 61, 56, 52, 48, 44, 41, 37, 35, 32, 29, 27, 25, 23, 21, 19, 18, 16, 15, 14, 13, 11, 11, 10, 9, 8, 7, 7, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0};

// linear approximation between the missing points
// input value 0..255 - output = value 0..255
static const uint8_t EXP(uint8_t x){
	uint8_t i = (x >> 2);
	uint8_t dy = (expTable[i] - expTable[i + 1]);
	uint8_t i2 = ((x >> 1) & 1)?(dy >> 1):0;
	uint8_t i1 = (x & 1)?(dy >> 2):0;
	return expTable[i] - i2 - i1; 
}

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
	
	/*uint16_t s = sample + 128;
	int32_t dy = ((s * 128) - fi->acc);
	fi->acc += ((dy * fi->cutoff) / 256); //(uint16_t)fi->cutoff); 
	return (fi->acc >> 8) - 128;*/
}
/*
static int8_t FI_SimpleHP_Fast(fil_t *fi, int8_t sample){
	uint8_t s = sample + 127;
	return ((s - (FI_SimpleLP_Fast(fi, sample) + 127)) / 2) - 128;
}
*/
static int8_t FI_Distortion(fil_t *fi, int8_t x){
	int16_t s = x; 
	if(s < -30) s = -30; 
	if(s > 30) s = 30; 
	return x; 
	/*
	int16_t s = 0; 
	for(uint8_t c = 0; c < 10; c++){
		s += SIN((c * ((int16_t)x + 128)) & 0xff);
	}
	return s; 
	float amount = 0.04; 
	float in = x; 
	float k = 2*amount/(1-amount);

	return 127 * (1+k)*in/(1+k*((in >= 0)?in:-in));*/
}


static void OSC_Process(osc_t *osc) {
	if(osc->waveform){
		osc->output = osc->waveform(((osc->phase_acc >> 8) + (uint16_t)osc->phase_offset) & 0xff); 
		osc->output = ((osc->amp_acc >> 9) * osc->output) / 128; 
	}
	osc->phase_acc += osc->phase_dx; 
	
	// fade the oscillator 
	// osc->amp_acc -= osc->amp_acc >> osc->fade; 
}

static void OSC_Reset(osc_t *osc){
	osc->amp_acc = UINT16_MAX; 
}

static void ENV_Reset(env_t *env){
	env->time = 0; 
	env->volume = 0; 
}

static void ENV_Setup(env_t *env, uint8_t a, uint8_t d, uint8_t s, uint8_t r){
	env->attack = a; env->decay = d; env->release = r;
	env->a_dx = UINT16_MAX/a;
	env->d_dx = UINT16_MAX/d;
	env->r_dx = UINT16_MAX/r;
	env->sustain = s; 
	env->time = 0; 
	env->volume = 0; 
}

// should be called at 64 times per second relative to sample rate
static void ENV_Process(env_t *env, uint8_t pressed){
	uint8_t a = (env->attack + env->decay); 
	if(env->time > (a + env->release)) return; // we are done
	
	if(env->time < env->attack){ // attack stage 0..attack
		env->volume = 255 - EXP(((uint8_t)env->time * (uint16_t)env->a_dx) / 256); 
	} else if(env->time >= env->attack && env->time < a && env->volume > env->sustain){ // decay attack..decay && volume > sustain
		env->volume = env->sustain + ((255 - env->sustain) * 
				EXP((((uint8_t)env->time - (uint8_t)env->attack) * (uint16_t)env->d_dx) / 256))/ 256; 
	} else if(env->time >= a && pressed){
		return; // halt until key is released
	} else if(env->time >= a && env->time < (a + env->release)){
		env->volume = env->sustain * EXP(((env->time - a) * env->r_dx) / 256) / 256;
	}
	env->time++; 
}

/*
static int16_t EFT_KarplusRun(eft_t *eft, int16_t sample){
	const uint8_t j = eft->phase >> 12; 
	const int8_t v = (int8_t) (eft->buf[j] >> 8);
	const int16_t ret = v + 128;
	const uint8_t next = (eft->phase + eft->phase_dx) >> 12; //bh!=N-1 ? bh+1 : 0;
	register int32_t avg = (eft->buf[j] + (int32_t)eft->buf[next]) >> 1;
	eft->buf[j] = avg;
	eft->phase += eft->phase_dx; 
	return ret;
}

static void EFT_KarplusReset(eft_t *eft){
	for (uint8_t i=0; i!=33; i++)
		eft->buf[i] = (int16_t) random(-32768,32767);
	eft->phase = 0; 
}

void EFT_KarplusInit(eft_t *eft){
	eft->run = EFT_KarplusRun; 
	eft->reset = EFT_KarplusReset; 
	EFT_KarplusReset(eft); 
}
*/
void U_Init(synth_t *s){
	// first oscillator
	s->osc1.detune = 0; 
	s->osc1.pitch = 3000; 
	s->osc1.fade = 0;
	s->osc1.waveform = SIN; 
	s->osc1.lfo = 0; 
	
	s->osc2.detune = 0; 
	s->osc2.pitch = 0; 
	s->osc2.lfo = 0; 
	s->osc2.waveform = SIN; 
	
	// sinusoidal lfo
	s->lfo.phase_dx = INCREMENT_FROM_FREQ(4); 
	s->lfo.waveform = SIN; 
	s->lfo.amp_acc = UINT16_MAX; 
	
	s->mix = 128; 
	
	s->lowpass.cutoff = 80; 
	s->lowpass.lfo = 255; 
	s->lowpass.decay = 0;
	
	ENV_Setup(&s->filter_env, 32, 32, 0, 32); 
	//ENV_Setup(&s->envelope, 2, 1, 255, 12);
	ENV_Setup(&s->envelope, 2, 1, 255, 32);
	
	//EFT_KarplusInit(&s->effect); 
	
	s->amp.level = 255; 
	
}

void U_PlayNoteRaw(synth_t *s, uint8_t note){
	ENV_Reset(&s->envelope); 
	ENV_Reset(&s->filter_env); 
	
	OSC_Reset(&s->osc1);
	OSC_Reset(&s->osc2); 
	OSC_Reset(&s->lfo); 
	
	uint16_t fq = S_FrequencyFromIndex(note);
	s->osc1.phase_dx = INCREMENT_FROM_FREQ(fq); 
	
	//fq = S_FrequencyFromIndex(note);
	s->osc2.phase_dx = INCREMENT_FROM_FREQ(fq); 
	
	//s->effect.phase_dx = INCREMENT_FROM_FREQ(fq); 
}

void U_PlayNote(synth_t *s, uint8_t note, uint8_t octave, int8_t kind){
	ENV_Reset(&s->envelope); 
	ENV_Reset(&s->filter_env); 
	
	OSC_Reset(&s->osc1);
	OSC_Reset(&s->osc2); 
	OSC_Reset(&s->lfo); 
	
	//s->effect.reset(&s->effect); 
	
	uint16_t fq = S_FrequencyFromNote(note, octave, kind + s->osc1.detune, s->osc1.pitch);
	s->osc1.phase_dx = INCREMENT_FROM_FREQ(fq); 
	
	fq = S_FrequencyFromNote(note, octave, kind + s->osc2.detune, s->osc2.pitch);
	s->osc2.phase_dx = INCREMENT_FROM_FREQ(fq); 
}

#define MIN(x, y) (((x) < (y))?(x):(y))
#define MAX(x, y) MIN(y, x)
#define saturate(x) MIN(MAX(-1.0,x),1.0)

float BassBoosta(float sample)
{
static float selectivity, gain1, gain2, ratio, cap;
gain1 = 1.0/(selectivity + 1.0);

cap= (sample + cap*selectivity )*gain1;
sample = saturate((sample + cap*ratio)*gain2);

return sample;
}

float getPt( float n1 , float n2 , float perc )
{
    float diff = n2 - n1;

    return n1 + ( diff * perc );
}    

void Bezier(float x1, float y1, float x2, float y2, float x3, float y3, float *x, float *y, float i){
    // The Green Line
    float xa = getPt( x1 , x2 , i );
    float ya = getPt( y1 , y2 , i );
    float xb = getPt( x2 , x3 , i );
    float yb = getPt( y2 , y3 , i );

    // The Black Dot
    *x = getPt( xa , xb , i );
    *y = getPt( ya , yb , i );
}

// should be called once for each sample at the rate of samplerate
uint8_t U_GenSample(synth_t *synth){
	static uint16_t counter = 0; 
	if((counter & 0xff) == 0){
		ENV_Process(&synth->envelope, 0);
		ENV_Process(&synth->filter_env, 0); 
	}
	counter++; 
	
	static int16_t s = 0;  
	static int incr = 1073; 
	static uint16_t cr = 0; 
	static volatile uint16_t p0 = 0, p1 = 64, p2 = 0, p3 = 0, fm = 0; 
	uint16_t w0 = INCREMENT_FROM_FREQ(220) ;
	uint16_t w1 = INCREMENT_FROM_FREQ(1000) ;
	static volatile uint16_t w2 = INCREMENT_FROM_FREQ(880) ;
	static volatile uint16_t w3 = INCREMENT_FROM_FREQ(1760) ;
	if((cr++) == 10000){
		incr+=20; 
	}
	if(incr == 2000) incr = 0; 
	p0 += w0;// + SIN(p1 >> 8) * 16;
	p1 += w1;
	p2 += w2;
	p3 += w3; 
	// this mixing procedure works
	s = (SAWR(p0 >> 8) - SQUARE(p1 >> 8) - SIN(p2 >> 8) - SIN(p3 >> 8)) / 4;
	s = SAWR(p0 >> 8);
	
	
	static long vel = 0; 
	static long pos = 0; 
	if(s - pos < 0){
		vel -= 1; 
		pos += vel >> 2; 
		s = pos; 
	} else {
		vel = 0; 
		pos = s; 
	} 
	
	float resonance = 0.4; 
	
	static int ss = 20; 
	static float *buffer = 0;
	if(!buffer) buffer = malloc(sizeof(float) * ss * 2 + 1); 
	static float b[3]; 
	static float y[3], out; 
	static int ctr = 0, frac = 0;
	if(ctr >= ss){
		y[0] = out; 
		y[1] = y[2]; 
		y[2] = s; 
		ctr = 0; 
	} else {
		ctr++; 
	}
	//buffer[ctr] = s; 
	float x; 
	Bezier(0, y[0], 1, y[1], 2, y[2], &x, &out, (float)ctr / (ss * 2.)); 
	frac++;  
	//s = out; 
	//s = BassBoosta((float)s / 127.f) * 127.f; 
	//s -= SIN(p1 >> 8) * resonance ;
	//s = (int8_t)(s / 2); 
	//s = (synth->envelope.volume * s) / 256; 
	/*
	static double fs = 0, fs1 = 0, fs2 = 0; 
	float a = 0.8;
	fs = (s * (1 - a)) + (fs * a); //= (fs2 + fs1 + fs + s) / 4; 
	fs2 = fs1;
	fs1 = fs; 
	//fs = 0.2 * s + (1.0 - 0.2) * fs;
	s = (int8_t) fs; //((fs + fs1 + fs2) / 3); */
	synth->lowpass.cutoff = 10; //128 + synth->lfo.output; 
	synth->lowpass.resonance = 0; 
	s = FI_SimpleLP_Fast(&synth->lowpass, s) ;
	//s = FI_SimpleLP_Fast(&synth->lowpass, s) * 4;*/
	return s + 128;
	
	// run the oscillators 
	OSC_Process(&synth->osc1); 
	OSC_Process(&synth->osc2); 
	OSC_Process(&synth->lfo); 
	
	// apply lfo to oscillator outputs
	int16_t osc1_out = synth->osc1.output; 
	int16_t osc2_out = synth->osc2.output;
	
	// mix the output from the oscillators 
	int16_t sample = (osc1_out + osc2_out) / 2; 
	
	// adjust filter params
	synth->lowpass.cutoff = 128; //128 + synth->lfo.output; 
	synth->lowpass.resonance = 0; 
	
	// modulate oscillators
	synth->osc2.phase_offset = 64; //osc1_out + 128; 
	//synth->osc2.phase_acc += synth->osc1.output; 
	
	//synth->osc1.amp_acc = 16000 + (128+synth->lfo.output) * 64; 
	//synth->lowpass.resonance = 128 + ((int8_t)synth->lfo.output * (uint16_t)synth->lowpass.lfo) / 256; //(((int8_t)synth->lfo.output + 128) * (uint8_t)synth->lowpass.lfo) / 256; 
	
	// pass through the effect buffer
	//sample = synth->effect.run(&synth->effect, sample); 
	
	// filter the signal through the filters
	sample = FI_SimpleLP_Fast(&synth->lowpass, sample);
	//sample = FI_Distortion(0, sample); 
	
	// amplify the signal with the output amp
	sample = ((int8_t)sample * ((uint16_t)synth->amp.level >> 1)) / 128; 
	
	// apply the final sound envelope
	sample = ((uint8_t)synth->envelope.volume * (int8_t)sample) / 256; 
	return (int8_t)sample + 128; 
}
