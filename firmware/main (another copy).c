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

const signed char sineTable[256] PROGMEM = {0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116, 117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118, 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92, 90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51, 49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3, 0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46, -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88, -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116, -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118, -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100, -98, -96, -94, -92, -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51, -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12, -9, -6, -3 };


const int note_freq[] PROGMEM = {
// c    c#   d   d#    e    f   f#    g    g#   a   a#     b
	16 , 17,  18 , 20,  21 , 22 , 23,  25 , 26,  28 , 29,  31 , //0	
	33 , 35,  37 , 39,  41 , 44 , 46,  49 , 52,  55 , 58,  62 ,	//1
	65 , 69,  73 , 78,  82 , 87 , 93,  98 , 104, 110, 117, 124,	//2
	131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, //3
	262, 278, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, //4
	523, 554, 587, 622, 659, 699, 740, 784, 831, 880, 932, 988, //5
	1047,1109,1175,1245,1319,1397,1475,1568,1661,1760,1865,1976 //6
};

uint16_t NOTE_INDEX(int note){
	return (((note) >= 0 && (note) < 2)?((note) * 2):(((note) >= 2 && (note) < 5)?((note) * 2 - 1):((note) * 2 - 2)));
}

uint16_t base_freq(int note, int oct){
	return (pgm_read_word_near(&note_freq[oct * 12 + ((NOTE_INDEX(note) + 9) % 12)]));
}
uint16_t sharp_freq(int note, int oct){
	return (pgm_read_word_near(&note_freq[oct * 12 + ((NOTE_INDEX(note) + 10) % 12)]));
}
uint16_t flat_freq(int note, int oct){
	return (pgm_read_word_near(&note_freq[oct * 12 + ((NOTE_INDEX(note) + 8) % 12)]));
}

static char _reg[U_REGISTER_COUNT];

#define REG(name) *(&_reg[U_##name])

#define SIN(x) ((char)pgm_read_byte_near(&sineTable[x]))


voice_t voices[1];	

/* FILTERS */
/*
signed char FI_SimpleLP_Fast(filter_t *fi, signed char sample){
	fi->acc += ((sample << 7) - fi->acc) * fi->response;
	return fi->acc >> 7;
}

signed char FI_SimpleHP_Fast(filter_t *fi, signed char sample){
	fi->acc += ((sample << 7) - fi->acc) * fi->response;
	return sample - (fi->acc >> 7);
}

signed char FI_SimpleLP(filter_t *fi, signed char sample){
	if(!fi->initialized){
		float x = x = exp(-2.0*M_PI*fi->frequency/SAMPLES_PER_SECOND);
		fi->a0 = 1.0-x;
		fi->x1 = -x;
		fi->initialized = 1;
	}
	fi->acc = fi->a0*sample - fi->x1*fi->acc;
	return fi->acc;
}

signed char FI_SimpleHP(filter_t *fi, signed char sample){
	if(!fi->initialized){
		float x = x = exp(-2.0*M_PI*fi->frequency/SAMPLES_PER_SECOND);
		fi->a0 = 1.0-x;
		fi->x1 = -x;
		fi->initialized = 1;
	}
	fi->acc = fi->a0*sample - fi->x1*fi->acc;
	return sample - fi->acc;
}

signed char FI_Distortion(filter_t *fi, signed char x){
	float amount = 0.04; 
	float in = x; 
	float k = 2*amount/(1-amount);

	return 127 * (1+k)*in/(1+k*((in >= 0)?in:-in));
}

signed char FI_Overdrive(filter_t *fi, signed char x){
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
signed char FI_1PoleLP(filter_t *fi, signed char sample){
  //((out = (in * (1 - resp)) + (prev * resp);
  //in * 1 - in * resp + prev * resp; 
  fi->acc = (sample << 8) + ((fi->acc - (sample << 8)) * fi->response); 
  return fi->acc >> 8; 
}
*/
/* Synth functions */
void VO_Filter(voice_t *vo, filter_t *fi, signed char (*filter)(filter_t *filter, signed char sample)){
	vo->signal = filter(fi, vo->signal);
}

void VO_Delay(voice_t *vo, delay_t *dl){
	dl->buffer_pos++; dl->buffer_pos &= DELAY_BUF_SIZE;
	dl->buffer[dl->buffer_pos] = vo->signal;
	vo->signal = dl->buffer[(dl->buffer_pos - dl->delay) & DELAY_BUF_SIZE];
}

void VO_ApplyEnvelope(voice_t *vo){
	vo->amplitude_fall 	-= (vo->amplitude_fall >> vo->decay) ;
	vo->phase_rise 			-= (vo->phase_rise >> vo->attack);
	vo->amplitude 			= ((max_amp - vo->phase_rise) >> 8) * (vo->amplitude_fall >> 8) ;
}

#define MAX(a, b) ((a >= b)?a:b)

void OSC_Step(oscillator_t *osc){
	osc->phase += osc->increment;
	
	int wave = 0;
	int phase = (osc->phase >> 8) + osc->phase_offset;
	switch(osc->waveform) {
		case WAVE_SINE: 
			wave = SIN(phase);
			break;
		case WAVE_SAW: 
			wave = (char)(-127+phase);
			break;
		case WAVE_SQUARE:
			wave = ((phase < 128)?-127:127);
			break;
		case WAVE_TRIANGLE:
			wave = (char)(
							(phase < 64)
								?-(phase * 2)
								:((phase >= 64 && phase < 192)
									?(-128 + (phase - 64) * 2)
									:((256 - (phase - 192)) << 1)
								)
							);
			break;
		case WAVE_NOISE:
			wave = rand();
			break;
		default:
			wave = 0;
			break;
	}
	
	// scale the waveform according to volume and mix with input
	int8_t res = ((osc->amplitude >> 8) * wave) >> 8;
	res = ((int16_t)res + (int16_t)osc->input) >> 1;
	osc->output = res;
}

/*** MODULATORS ****/
void OSC_Modulate(int mod, oscillator_t *a, oscillator_t *b){
	switch(mod){
	case MOD_PHASE: 
		b->phase_offset = a->output; 
		break;
	case MOD_AMP: 
		b->amplitude = (a->output + 127) << 8;
		break;
	case MOD_MIX:
		b->input = a->output;
		break;
	case MOD_SYNC:
		if((a->phase + a->increment) < a->phase)
			b->phase = 0; 
		break;
	case MOD_FREQ: 
		b->phase += ((uint16_t)a->output) << 5; // 5 seems to give best range
		break;
	default:
		//b->output = a->output;
		break;
	}
}

void VO_Iterate(voice_t *vo){
	// compute exponential attack and decay of amplitude
	// the (time & 0x0ff) slows down the decay computation by 256 times		
	if ((vo->time & 0x0ff) == 0) {
		VO_ApplyEnvelope(vo);
	}
	
	OSC_Step(&vo->osc[0]);
	OSC_Modulate(REG(MOD_01), &vo->osc[0], &vo->osc[1]);
	OSC_Step(&vo->osc[1]);
	OSC_Modulate(REG(MOD_12), &vo->osc[1], &vo->osc[2]);
	OSC_Step(&vo->osc[2]);
	
	int vol = ((vo->amplitude >> 8) * ((uint8_t)REG(MASTER_VOLUME))) >> 8;
	vo->signal = (vol * vo->osc[2].output) >> 6;
}

void VO_Pluck(voice_t *vo){
	vo->amplitude_fall = max_amp; 
	vo->phase_rise = max_amp ;
	//vo->amplitude = max_amp ;
	
	vo->osc[0].phase_offset = REG(OSC0_PHASE_OFFSET);
	vo->osc[0].amplitude = REG(OSC0_VOLUME) << 8;
	vo->osc[0].waveform = REG(OSC0_WAVEFORM);
	vo->osc[0].detune = REG(OSC0_DETUNE_COARSE);
	vo->osc[0].fine_detune = REG(OSC0_DETUNE_FINE);
	
	vo->osc[1].phase_offset = REG(OSC1_PHASE_OFFSET);
	vo->osc[1].amplitude = REG(OSC1_VOLUME) << 8;
	vo->osc[1].waveform = REG(OSC1_WAVEFORM);
	vo->osc[1].detune = REG(OSC1_DETUNE_COARSE);
	vo->osc[1].fine_detune = REG(OSC1_DETUNE_FINE);
	
	vo->osc[2].phase_offset = REG(OSC2_PHASE_OFFSET);
	vo->osc[2].amplitude = REG(OSC2_VOLUME) << 8;
	vo->osc[2].waveform = REG(OSC2_WAVEFORM);
	vo->osc[2].detune = REG(OSC2_DETUNE_COARSE);
	vo->osc[2].fine_detune = REG(OSC2_DETUNE_FINE);
	
	vo->osc[0].phase = 0;
	vo->osc[1].phase = 0;
	vo->osc[2].phase = 0; 
	
	vo->pluck = 0;
}

const char *melody = 
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	"c4\x08" "e4\x08" "g4\x08" "e5\x08"
	
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	"b4\x08" "d4\x08" "g4\x08" "c5\x08"
	"b4\x08" "d4\x08" "g4\x08" "b5\x08"
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
ISR (TIMER1_COMPA_vect) { 
	// turn on timer for profiling
	TCNT2 = 0; TCCR2B = 2;
	
	for(int v = 0; v < 1; v++) {
		voice_t *vo = &voices[v];
		
		// Init the synth
		if (vo->pluck==1) {
			VO_Pluck(vo);
		}
		
		VO_Iterate(vo);
		vo->time ++;
	}
	
	//VO_Filter(&voices[0], &lowpass, FI_SimpleLP);
	//signed char s = voices[0].signal;
	//VO_Delay(&voices[0], &delay); 
	//VO_Filter(&voices[0], &normal, &FI_Normalize);
	
	int sig = voices[0].signal + 127;
	OCR0A = (sig > 256)?256:((sig < 0)?0:sig); //((voices[0].signal + voices[1].signal) / 2);
	
	// profiling 
	TCCR2B = 0;
} 


int freeMem () { 
	extern int __heap_start, *__brkval;
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void OSC_SetFrequencyFromNote(oscillator_t *osc, char note, int octave, uint8_t detune){
	int n = note - 'a';
	int d = octave * 12 + ((NOTE_INDEX(n) + 9) % 12);
	int fq = (pgm_read_word_near(&note_freq[d + detune + osc->detune] + osc->fine_detune));
	osc->increment = INCREMENT_FROM_FREQ(fq);
}

/////////////////////////////////////////////////////

#define AUDIO_OUT(reg) BIT(D, 6, reg)

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
	
	REG(OSC0_PHASE_OFFSET) = 64; 	// oscillator phase ofset from clock 0 - 256
	REG(OSC0_VOLUME) = 255;			// volume 0 - 256
	REG(OSC0_DETUNE_COARSE) = 12; // detune amount - high byte = octave, low byte = cycles
	REG(OSC0_DETUNE_FINE) = 0;		// amount of fine detuning
	REG(OSC0_WAVEFORM) = WAVE_TRIANGLE;			// 0 = sine, 1 = triangle, 2 = saw, 3 = square, 4 = noise
	
	REG(OSC1_PHASE_OFFSET) = 0;
	REG(OSC1_VOLUME) = 255;
	REG(OSC1_DETUNE_COARSE) = -12;
	REG(OSC1_DETUNE_FINE) = 0;
	REG(OSC1_WAVEFORM) = WAVE_SINE;
	
	REG(OSC2_PHASE_OFFSET) = 0;
	REG(OSC2_VOLUME) = 255;
	REG(OSC2_DETUNE_COARSE) = 0;
	REG(OSC2_DETUNE_FINE) = 0;
	REG(OSC2_WAVEFORM) = WAVE_TRIANGLE; 
	
	// osc1 mod - 0 = passthrough, 1 = phase, 2 = frequency, 3 = amplitude, 4 = mix
	REG(MOD_01) = MOD_PHASE;		
	REG(MOD_12) = MOD_MIX;
	
	REG(INPUT_OSC) = 0b00000110;	// which oscillators get set to input frequency (bit 0 = osc0 etc..)
	
	// envelope settings
	REG(ENV_ATTACK) = 50;	// time from pluck to reaching full volume in ms * 10, 256 = 2.56 senconds. 
	REG(ENV_DECAY) = 255;		// time from pluck to reaching sustain level
	REG(ENV_SUSTAIN) = 127;	// level of sustain 0 = no sustain, 256 = full volume
	REG(ENV_RELEASE) = 255;	// time after the string is released until it is quiet
	 
	REG(MASTER_VOLUME) = 255;
	
	// filter settings
	REG(FL0_TYPE) = 0;				// 0 = none 1 = lowpass, 2 highpass, 3 distortion
	REG(FL0_FREQUENCY) = 0;	// corner frequency of the filter
	
	// synth settings
	REG(CONTROL) = 3; // bits: 0 = enable, 1 = loop
	
	voice_t *vo = &voices[0];

	vo->osc[0].increment = INCREMENT_FROM_FREQ(2);
	vo->osc[1].increment = INCREMENT_FROM_FREQ(500);
	vo->osc[2].increment = INCREMENT_FROM_FREQ(60);
	
	vo->decay = 4 ;
	vo->attack = 1 ;

	/*lowpass.frequency = 1200;
	lowpass.resonance = 1;
	lowpass.initialized = 0; 
	lowpass.acc = 0;
		*/
	//FI_Init(&lowpass, 1200);
	
 
  ////////////////////////////////////////////////////
		
	unsigned int full = SAMPLES_PER_SECOND;

	typedef struct  {
		unsigned char note;
		unsigned char octave;
		uint8_t length; 
	} note_t ;

	note_t *note = (note_t*) melody;
	
	while(1) {  
		if (voices[0].time >= (full / (note->length & 0x0f))) {
			note++;
			if(note->length == 0) note = (note_t*)melody;
			
			// convert flat to sharp
			if(note->note != 'p'){
				int flatness = ((note->length & 0xf0) == 0xf0)?(-1):(((note->length & 0xf0) == 0x10)?1:0);
				for(int c = 0; c < 3; c++){
					if(REG(INPUT_OSC) & (1 << c))
						OSC_SetFrequencyFromNote(&voices[0].osc[c], note->note, note->octave - '0', flatness);
				}
				voices[0].pluck = 1;
			}
			//printf("Memory: %d\r\n", freeMem());
			//printf("Note: %c, len: %d\r\n", note->note, fq);
			voices[0].time = 0;
			//printf("%d\n\r", TCNT2*8);
		}
	}
}
