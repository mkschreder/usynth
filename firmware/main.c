// DDS output thru PWM on timer0 OC0A (pin B.3)
// Mega644 version
// FM synthesis
#define F_CPU 18432000UL

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h> 		// for sine
#include <stdio.h>
#include <avr/pgmspace.h>
#include "local.h"
#include "uart.h"
#include "util.h"
#include "../usynth.h"

uint16_t rand();


/*
const char *melody = 
	"c4\x01"
	"\0\0\0";
*/
static const char *melody = 
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	
	"f4\x08" "g4\x18" "c5\x08" "g5\x18"
	"f4\x08" "g4\x18" "c5\x08" "g5\x18"
	"f4\x08" "g4\x18" "c5\x08" "g5\x18"
	"f4\x08" "g4\x18" "c5\x08" "g5\x18"
	
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	"b4\x08" "d4\x08" "g4\x08" "b5\x08"
	"b4\x08" "d4\x08" "g4\x08" "b5\x08"
	"\0\0\0";

/*
const char *melody =  
	"e5\x08" "e5\x04" "e5\x04" "c5\x08" "e5\x04" "g5\x02" "g4\x02"

	// melody 
	"c5\x03" "g4\x03" "e4\x03" 
	"a4\x04" "b4\x04" "a4\x18" "a4\x04"
	"g4\x08" "e5\x04" "g5\x08" "a5\x04" "f5\x08" "g5\x04"
	"e5\x04" "c5\x08" "d5\x08" "b4\x03"

	// repeat
	"c5\x03" "g4\x03" "e4\x03" 
	"a4\x04" "b4\x04" "a4\x18" "a4\x04"
	"g4\x08" "g5\x04" "g5\x08" "a5\x04" "f5\x08" "g5\x04"
	"e5\x04" "c5\x08" "d5\x08" "b4\x03"

	// passage
	"g5\x08" "f5\x18" "f5\x08" "d5\x14" "e5\x04"
	"g5\x18" "a4\x08" "c5\x04" "a4\x08" "c5\x08" "d5\x03"
	"g5\x08" "f5\x18" "f5\x08" "d5\x14" "e5\x04"
	"c6\x04" "c6\x08" "c6\x02"
	// repeat
	"g5\x08" "f5\x18" "f5\x08" "d5\x14" "e5\x04"
	"g5\x18" "a4\x08" "c5\x04" "a4\x08" "c5\x08" "d5\x03"
	"d5\x13" "d5\x03" "c5\x02"
	"p0\x01"
	"\0\0\0"; 
*/

static voice_t synth;
static filter_t filter, highpass, dist;
static const signed char sineTable[256] PROGMEM = {0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116, 117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118, 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92, 90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51, 49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3, 0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46, -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88, -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116, -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118, -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100, -98, -96, -94, -92, -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51, -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12, -9, -6, -3 };


static const uint8_t expTable[256]  = {255, 235, 217, 201, 186, 171, 158, 146, 135, 125, 115, 107, 98, 91, 84, 77, 71, 66, 61, 56, 52, 48, 44, 41, 37, 35, 32, 29, 27, 25, 23, 21, 19, 18, 16, 15, 14, 13, 11, 11, 10, 9, 8, 7, 7, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0};

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
	33 , 35,  37 , 39,  41 , 44 , 46,  49 , 52,  55 , 58,  62 ,	//1
	65 , 69,  73 , 78,  82 , 87 , 93,  98 , 104, 110, 117, 124,	//2
	131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, //3
	262, 278, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, //4
	523, 554, 587, 622, 659, 699, 740, 784, 831, 880, 932, 988, //5
	1047,1109,1175,1245,1319,1397,1475,1568,1661,1760,1865,1976 //6
};

static const uint16_t NOTE_INDEX(int note){
	return (((note) >= 0 && (note) < 2)?((note) * 2):(((note) >= 2 && (note) < 5)?((note) * 2 - 1):((note) * 2 - 2)));
}

static char _reg[U_REGISTER_COUNT];

#define REG(name) *((int8_t*)&_reg[U_##name])
#define MAX(a, b) ((a >= b)?a:b)
#define SIN(x) ((char)pgm_read_byte_near(&sineTable[x]))
#define CLAMP(x, min, max) ((x > max)?max:((x < min)?min:x))

static uint16_t DT(uint16_t ui16, uint8_t ui8){
	//return ui16 >> (ui8 >> 5);
	return ((ui16 >> (ui8 >> 5)) * CLAMP(32 - (ui8 & 0x1f), 0, 28) >> 2);
}

static int16_t DTS(int16_t ui16, uint8_t ui8){
	return DT(((uint16_t)ui16) + INT_MAX, ui8) - INT_MAX;
}

/* FILTERS */
	
static int8_t FI_SimpleLP_Fast(filter_t *fi, int8_t sample){
	int16_t dy = ((sample * 128) - fi->acc);
	fi->acc += ((dy / 256) * fi->cutoff); 
	return fi->acc / 128;
}

static int8_t FI_SimpleHP_Fast(filter_t *fi, int8_t sample){
	return ((sample - FI_SimpleLP_Fast(fi, sample)) / 2);
}

static uint8_t FI_Distortion(filter_t *fi, uint8_t x){
	int16_t p = x - 128;
	return (p * 2) + 256;
	uint8_t amount = 10;
	uint16_t k = (2 * amount * 256)/ (256 - amount);
	return (128 * (256 + k) * x / (256 + k * (x - 128))) / 256;/*
	float amount = 0.04; 
	float in = x; 
	float k = 2*amount/(1-amount);

	return 127 * (1+k)*in/(1+k*((in >= 0)?in:-in));*/
}

static signed char FI_Overdrive(filter_t *fi, signed char x){
	float a = 0.5;
	int z = (int)(128 * a) % 256;
	int s = 128/SIN(z);
	int b = 1./a;

	if (x > b)
		return 127;
	else
		return SIN(z*x)*s; 
}

// 1 pole low pass filter equation variation #2 optimized for integer math
static signed char FI_1PoleLP(filter_t *fi, signed char sample){
  //((out = (in * (1 - resp)) + (prev * resp);
  //in * 1 - in * resp + prev * resp; 
  //fi->acc = (sample << 8) + ((fi->acc - (sample << 8)) * fi->response); 
  return fi->acc >> 8; 
}

/* Synth functions */

static int8_t OSC_GenWave(uint8_t waveform, uint8_t phase){
	switch(waveform) {
		case WAVE_SINE: 
			return SIN(phase);
			break;
		case WAVE_SAW: 
			return phase - 128;
			break;
		case WAVE_SQUARE:
			return ((phase < 128)?0:255) - 128;
			break;
		case WAVE_TRIANGLE:
			{
				int p = phase; 
				p -= 127;
				int w = (p < 0)?-p:p;
				// need this cap
				w = (w > 127)?127:w;
				w = (w < -127)?-127:w;
				return (w * 2) - 127; // - 127
			}
			break;
		case WAVE_NOISE:
			return rand() & 0xff - 128;
			break;
		default:
			return 0;
	}
	return 0;
}

// steps the oscillator. 
// how many times / sec you call this is determined by "increment" value. 
static void OSC_Step_R(oscillator_t *osc){
	osc->phase += osc->increment;
	
	uint8_t phase = ((osc->phase >> 8) + osc->phase_offset);
	int8_t wave = OSC_GenWave(osc->waveform, phase);
	
	// scale the waveform according to volume and mix with input
	int16_t res = ((uint16_t)wave + (uint16_t)osc->input) / 2;
	res = ((osc->amplitude / 256) * res) / 256;
	osc->output = res;
	
	if(osc->next){
		switch(osc->modulation_target){
		case MOD_PHASE: 
			osc->next->phase_offset = res; 
			break;
		case MOD_AMP: 
			osc->next->amplitude = res * 256;
			break;
		case MOD_MIX:
			osc->next->input = res;
			break;
		case MOD_SYNC:
			// test if it will overflow during next iteration
			if((osc->phase + osc->increment) < osc->phase)
				osc->next->phase = 0; 
			break;
		case MOD_FREQ: 
			osc->next->phase += res * 32; // 5 seems to give best range
			break;
		default:
			break;
		}
		
		OSC_Step_R(osc->next);
	}
}

void VO_Init(voice_t *vo){
	vo->osc[0].next = &vo->osc[1];
	vo->osc[1].next = &vo->osc[2];
	vo->osc[2].next = 0;
}


static void VO_ApplyEnvelope(voice_t *vo){
	// decay amount is divided by two in the end to make it less sensitive
	uint16_t a = (vo->amplitude_fall >> (vo->decay >> 5)) * (32 - (vo->decay & 0x1f)) >> 1;
	if(a > vo->amplitude_fall) a = vo->amplitude_fall; 
	vo->amplitude_fall 	-= a;
	vo->phase_rise 			-= (vo->phase_rise >> vo->attack);
	vo->amplitude 			 = ((UINT_MAX - vo->phase_rise) / 256) * (vo->amplitude_fall / 256) ;
}

void VO_Iterate(voice_t *vo){
	// compute exponential attack and decay of amplitude
	// the (time & 0x0ff) slows down the decay computation by 256 times		
	if ((vo->time & 0x0ff) == 0) {
		VO_ApplyEnvelope(vo);
	}
	
	// recursively step through all of the oscillators
	OSC_Step_R(&vo->osc[0]);
	
	int8_t s = vo->osc[2].output;
	//s = FI_SimpleLP_Fast(&filter, s);
	//s = FI_SimpleHP_Fast(&highpass, s);
	//s = FI_Distortion(&filter, s);
	
	// compute the final instrument volume
	uint8_t vol = ((vo->amplitude / 256) * ((uint8_t)REG(MASTER_VOLUME))) / 256;
	// compute the signal
	vo->signal = (vol * s) / 256;
	 //(sig * (uint8_t)REG(MASTER_GAIN)); //CLAMP((out * REG(MASTER_GAIN)) / 2, 0, 255); //vo->osc[1].output * REG(MASTER_GAIN); 
}

void VO_Pluck(voice_t *vo){
	vo->amplitude_fall = UINT_MAX; 
	vo->phase_rise = UINT_MAX ;
	vo->amplitude = UINT_MAX ;
	
	vo->osc[0].phase_offset = REG(OSC0_PHASE_OFFSET);
	vo->osc[0].amplitude = ((uint8_t)REG(OSC0_VOLUME)) << 8;
	vo->osc[0].waveform = REG(OSC0_WAVEFORM);
	vo->osc[0].detune = REG(OSC0_DETUNE_COARSE);
	vo->osc[0].fine_detune = REG(OSC0_DETUNE_FINE);
	vo->osc[0].modulation_target = REG(MOD_01);
	if(REG(OSC0_FREQ_H) || REG(OSC0_FREQ_L))
		vo->osc[0].increment = INCREMENT_FROM_FREQ((REG(OSC0_FREQ_H) << 8) | REG(OSC0_FREQ_L));
	
	vo->osc[1].phase_offset = REG(OSC1_PHASE_OFFSET);
	vo->osc[1].amplitude = ((uint8_t)REG(OSC1_VOLUME)) << 8;
	vo->osc[1].waveform = REG(OSC1_WAVEFORM);
	vo->osc[1].detune = REG(OSC1_DETUNE_COARSE);
	vo->osc[1].fine_detune = REG(OSC1_DETUNE_FINE);
	vo->osc[1].modulation_target = REG(MOD_12);
	if(REG(OSC1_FREQ_H) || REG(OSC1_FREQ_L))
		vo->osc[1].increment = INCREMENT_FROM_FREQ((REG(OSC1_FREQ_H) << 8) | REG(OSC1_FREQ_L));
	
	vo->osc[2].phase_offset = REG(OSC2_PHASE_OFFSET);
	vo->osc[2].amplitude = REG(OSC2_VOLUME) << 8;
	vo->osc[2].waveform = REG(OSC2_WAVEFORM);
	vo->osc[2].detune = REG(OSC2_DETUNE_COARSE);
	vo->osc[2].fine_detune = REG(OSC2_DETUNE_FINE);
	if(REG(OSC2_FREQ_H) || REG(OSC2_FREQ_L))
		vo->osc[2].increment = INCREMENT_FROM_FREQ((REG(OSC2_FREQ_H) << 8) | REG(OSC2_FREQ_L));
	
	vo->osc[0].phase = 0;
	vo->osc[1].phase = 0;
	vo->osc[2].phase = 0; 
	
	vo->attack = REG(ENV_ATTACK);		// exponential attack value 
	vo->decay = REG(ENV_DECAY);		// time from pluck to reaching sustain level
	vo->sustain = REG(ENV_SUSTAIN);	// level of sustain 0 = no sustain, 256 = full volume
	vo->release = REG(ENV_RELEASE);	// time after the string is released until it is quiet
	
	vo->pluck = 0;
}

ISR (TIMER1_COMPA_vect) { 
	// turn on timer for profiling
	TCNT2 = 0; TCCR2B = 2;
	
	voice_t *vo = &synth;
	
	// Init the synth
	if (vo->pluck==1) {
		VO_Pluck(vo);
	}
	
	VO_Iterate(vo);
	vo->time ++;
	
	OCR0A = vo->signal + 128;
	
	// profiling 
	TCCR2B = 0;
} 

static uint16_t freeMem () { 
	extern int __heap_start, *__brkval;
  int v; 
  #pragma GCC diagnostic ignored "-Wreturn-local-addr"
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

/////////////////////////////////////////////////////

#define AUDIO_OUT(reg) BIT(D, 6, reg)

static void OSC_SetFrequencyFromNote(oscillator_t *osc, char note, int octave, uint8_t detune){
	int n = note - 'a';
	int d = octave * 12 + ((NOTE_INDEX(n) + 9) % 12);
	uint16_t fq = (pgm_read_word_near(&note_freq[d + detune + osc->detune] + osc->fine_detune));
	osc->increment = INCREMENT_FROM_FREQ(fq);
}

int main(void){
	AUDIO_OUT(DDR) = OUTPUT;

	//UART_Init();

	// init pwm
	TCCR0B = 1 ;  
	TIMSK0 = 0 ;
	TCCR0A = (1<<COM0A0) | (1<<COM0A1) | (1<<WGM00) | (1<<WGM01) ; 
	OCR0A = 128 ; 
   
	// init waveform generator
	OCR1A = F_CPU / SAMPLES_PER_SECOND;
	TIMSK1 = (1<<OCIE1A) ;
	TCCR1B = 0x09; 	//full speed; clear-on-match
	TCCR1A = 0x00;	//turn off pwm and oc lines

	// turn on all ISRs
	sei() ;
	
	REG(OSC0_FREQ_H) = 0;
	REG(OSC0_FREQ_L) = 2;
	REG(OSC0_PHASE_OFFSET) = 0; 			// oscillator phase ofset from clock 0 - 256
	REG(OSC0_VOLUME) = 255;						// volume 0 - 256
	REG(OSC0_DETUNE_COARSE) = 0; 			// detune amount - high byte = octave, low byte = cycles
	REG(OSC0_DETUNE_FINE) = 0;				// amount of fine detuning
	REG(OSC0_WAVEFORM) = WAVE_SINE;		// 0 = sine, 1 = triangle, 2 = saw, 3 = square, 4 = noise
	
	REG(OSC1_FREQ_H) = 0;
	REG(OSC1_FREQ_L) = 0;
	REG(OSC1_PHASE_OFFSET) = 0;
	REG(OSC1_VOLUME) = 255;
	REG(OSC1_DETUNE_COARSE) = -12;
	REG(OSC1_DETUNE_FINE) = 0;
	REG(OSC1_WAVEFORM) = WAVE_SINE;
	
	REG(OSC2_FREQ_H) = 0;
	REG(OSC2_FREQ_L) = 0;
	REG(OSC2_PHASE_OFFSET) = 0;
	REG(OSC2_VOLUME) = 255;
	REG(OSC2_DETUNE_COARSE) = 0;
	REG(OSC2_DETUNE_FINE) = 0;
	REG(OSC2_WAVEFORM) = WAVE_SQUARE; 
	
	// osc1 mod - 0 = passthrough, 1 = phase, 2 = frequency, 3 = amplitude, 4 = mix
	REG(MOD_01) = MOD_PHASE;		
	REG(MOD_12) = MOD_PHASE;
	
	REG(INPUT_OSC) = 0b00000111;	// which oscillators get set to input frequency (bit 0 = osc0 etc..)
	
	// envelope settings
	REG(ENV_ATTACK) = 0;	// time from pluck to reaching full volume in ms * 10, 256 = 2.56 senconds. 
	REG(ENV_DECAY) = 240; //(5 << 5 | (31 & 0x1f));		// time from pluck to reaching sustain level
	REG(ENV_SUSTAIN) = 127;	// level of sustain 0 = no sustain, 256 = full volume
	REG(ENV_RELEASE) = 255;	// time after the string is released until it is quiet
	 
	REG(MASTER_VOLUME) = 255;
	REG(MASTER_GAIN) = 4;
	
	// filter settings
	REG(FL0_TYPE) = 0;				// 0 = none 1 = lowpass, 2 highpass, 3 distortion
	REG(FL0_FREQUENCY) = 0;	// corner frequency of the filter
	
	// synth settings
	REG(CONTROL) = 3; // bits: 0 = enable, 1 = loop
	
	voice_t *vo = &synth;
	VO_Init(vo);
	
	vo->attack = 1 ;

	filter.cutoff = 255;
	filter.initialized = 0; 
	filter.acc = 0; 
	
	highpass.cutoff = 0;
	highpass.initialized = 0; 
	highpass.acc = 0; 
	
	dist.initialized = 0; 
	dist.acc = 0; 
	
 
  ////////////////////////////////////////////////////
		
	unsigned int full = SAMPLES_PER_SECOND;

	typedef struct  {
		unsigned char note;
		unsigned char octave;
		uint8_t length; 
	} note_t ;

	note_t *note = (note_t*) melody;
	
	while(1) {  
		if (synth.time >= (full / (note->length & 0x0f))) {
			note++;
			if(note->length == 0) note = (note_t*)melody;
			
			// convert flat to sharp
			if(note->note != 'p'){
				int flatness = ((note->length & 0xf0) == 0xf0)?(-1):(((note->length & 0xf0) == 0x10)?1:0);
				
				for(int c = 0; c < 3; c++){
					if(REG(INPUT_OSC) & (1 << c)){
						OSC_SetFrequencyFromNote(&synth.osc[c], note->note, note->octave - '0', flatness);
					} 
				}
				
				synth.pluck = 1;
			}
			//printf("Memory: %d\r\n", freeMem());
			//printf("Note: %c, len: %d\r\n", note->note, fq);
			synth.time = 0;
			//printf("%d\n\r", TCNT2*8);
		}
	}
}
