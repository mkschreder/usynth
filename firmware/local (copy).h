#include <inttypes.h>
#include <limits.h>

typedef int16_t 	sample_t; 
typedef uint16_t 	counter_t; 
typedef uint8_t 	period_t; 

#define COUNTER_MAX 65535
#define COUNTER_MIN 0

#define SAMPLES_PER_SECOND (COUNTER_MAX / 4)
#define INCREMENT_FROM_FREQ(q) (uint16_t)(4 * (q)) 


// All variables should be integers and division should not be used. 
typedef volatile struct oscillator_s {
	// modulatable parameters
	uint16_t 	phase; 				// 0 - 65536
	int8_t 		phase_offset; // -127, 127
	uint16_t 	increment;    // 0 - 65536 
	uint16_t 	amplitude; 		// 0 - 65536
	
	int8_t 		detune; // detune in semitones
	int8_t 		fine_detune; // detune in herz from final frequency
	
	uint8_t 		waveform;
	
	int16_t 		input, output; 
	
	uint8_t 		modulation_target;
	
	volatile struct oscillator_s *next;
} oscillator_t ;


typedef void (*mix_func_t)(oscillator_t *, oscillator_t*);

typedef volatile struct voice_s{
	oscillator_t osc[3];
	int16_t signal;
	char pluck; 
	unsigned int time;
	
	// envelope
	unsigned int phase_rise;
	unsigned int amplitude_fall;
	unsigned char attack, decay, sustain, release;
	unsigned int amplitude;
} voice_t; 

typedef volatile struct filter_s {
	uint8_t	cutoff; 
	uint8_t 	initialized;
	int16_t 	prev_sample;
	int16_t 	acc;
	signed		char	min, max; 
	float		 	a0, a1, a2, a3, a4, x1, x2, y1, y2;
} filter_t;

#define DELAY_BUF_SIZE (0x1f)
typedef volatile struct delay_s{
	signed char buffer[DELAY_BUF_SIZE+1];
	unsigned int buffer_pos;
	unsigned char delay; // delay in number of samples
}delay_t;

