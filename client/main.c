#define _POSIX_C_SOURCE 199309L
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

#include "../driver/usynth.h"

#define MAX_COMMAND_LENGTH 10


// Opens the USB serial port and continuously attempts to read from the port.
// On receiving data, looks for a defined command.

handle_t synth; 

void  sigint_handler(int sig)
{
	U_Close(synth);
	exit (sig);
}


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
	"\0\0\0"; 

typedef volatile struct  {
	unsigned char note;
	unsigned char octave;
	uint8_t length; 
} note_t ;

struct itimerval tout_val;


static const uint16_t NOTE_INDEX(int note, int octave, int detune){
	int idx = (((note) >= 0 && (note) < 2)?((note) * 2):(((note) >= 2 && (note) < 5)?((note) * 2 - 1):((note) * 2 - 2)));
	return octave * 12 + ((idx + 9) % 12) + detune;
}

volatile note_t *note = 0; 

#define BPM 60.0f

void alarm_wakeup (int i)
{
	signal(SIGALRM, alarm_wakeup);
	char buf[32]; 
	int flatness = ((note->length & 0xf0) == 0xf0)?(-1):(((note->length & 0xf0) == 0x10)?1:0);
	int idx = NOTE_INDEX(note->note - 'a', note->octave - '0', flatness);
	U_NoteOn(synth, idx, 255); 
	printf("Play: %d\n", idx);
	
	tout_val.it_value.tv_usec = 1000000 / (BPM / 60) / (note->length & 0x0f); 
	setitimer(ITIMER_REAL, &tout_val,0);
	note++; 
	if(note->note == 0){
	 note = (note_t*)melody; 
	 sleep(1);
	}
}

typedef struct preset_s {
	uint8_t knob; 
	int8_t value; 
} preset_t; 

static preset_t preset_flute[] = {
	{KB_OSC1_WAVEFORM, WAVE_SIN},
	{KB_OSC1_DETUNE, 0},
	{KB_OSC1_FINE_TUNE, 0},
	{KB_OSC2_WAVEFORM, WAVE_SIN},
	{KB_OSC2_DETUNE, 0},
	{KB_OSC2_FINE_TUNE, 0},
	{KB_OSC_MIX_AMOUNT, 0},
	{KB_LFO_SPEED, 4},
	{KB_LFO_TO_OSC, 0},
	{KB_LFO_TO_FILTER, 0},
	{KB_LFO_TO_AMP, 0},
	{KB_AMP_ENV_ATTACK, 2},
	{KB_AMP_ENV_DECAY, 1}, 
	{KB_AMP_ENV_SUSTAIN, 127},
	{KB_AMP_ENV_RELEASE, 1},
	{KB_FILTER_CUTOFF, 32}, 
	{KB_FILTER_ENV_AMOUNT, 0},
	{KB_FILTER_ATTACK, 0},
	{KB_FILTER_DECAY, 0},
	{KB_FILTER_SUSTAIN, 0},
	{KB_FILTER_RELEASE, 0},
	{KB_AMP_VOLUME, 127},
	{0, 0}
}; 

static preset_t preset_derren1[] = {
	{KB_OSC1_WAVEFORM, WAVE_SQUARE},
	{KB_OSC1_DETUNE, 0},
	{KB_OSC1_FINE_TUNE, 0},
	{KB_OSC1_PHASE_OFFSET, 0},
	{KB_OSC2_WAVEFORM, WAVE_SQUARE},
	{KB_OSC2_DETUNE, -12},
	{KB_OSC2_FINE_TUNE, 0},
	{KB_OSC2_PHASE_OFFSET, 0},
	{KB_OSC_MIX_AMOUNT, 0},
	{KB_LFO_SPEED, 5},
	{KB_LFO_TO_OSC, 10},
	{KB_LFO_TO_FILTER, 40},
	{KB_LFO_TO_AMP, 20},
	{KB_AMP_ENV_ATTACK, 1},
	{KB_AMP_ENV_DECAY, 2}, 
	{KB_AMP_ENV_SUSTAIN, 64},
	{KB_AMP_ENV_RELEASE, 5},
	{KB_FILTER_CUTOFF, 255}, 
	{KB_FILTER_ENV_AMOUNT, 0},
	{KB_FILTER_ATTACK, 0},
	{KB_FILTER_DECAY, 0},
	{KB_FILTER_SUSTAIN, 0},
	{KB_FILTER_RELEASE, 0},
	{KB_AMP_VOLUME, 64},
	{0, 0}
}; 


static preset_t preset[] = {
	{KB_OSC1_WAVEFORM, WAVE_SAWL},
	{KB_OSC1_DETUNE, 0},
	{KB_OSC1_FINE_TUNE, 0},
	{KB_OSC1_PHASE_OFFSET, 0},
	{KB_OSC2_WAVEFORM, WAVE_SAWL},
	{KB_OSC2_DETUNE, 0},
	{KB_OSC2_FINE_TUNE, 0},
	{KB_OSC2_PHASE_OFFSET, 0},
	{KB_OSC_MIX_AMOUNT, 0},
	{KB_LFO_SPEED, 8},
	{KB_LFO_TO_OSC, 0},
	{KB_LFO_TO_FILTER, 0},
	{KB_LFO_TO_AMP, 0},
	{KB_AMP_ENV_ATTACK, 20},
	{KB_AMP_ENV_DECAY, 10}, 
	{KB_AMP_ENV_SUSTAIN, 32},
	{KB_AMP_ENV_RELEASE, 10},
	{KB_FILTER_CUTOFF, 64}, 
	{KB_FILTER_ENV_AMOUNT, 0},
	{KB_FILTER_ATTACK, 0},
	{KB_FILTER_DECAY, 0},
	{KB_FILTER_SUSTAIN, 0},
	{KB_FILTER_RELEASE, 0},
	{KB_AMP_VOLUME,64},
	{0, 0}
}; 

int main()
{
	int chars_read;
	char read_buffer[MAX_COMMAND_LENGTH + 1] = {0};

	synth = U_Open();

	if(synth == 0) return 0; 
	
	signal (SIGINT, (void*)sigint_handler);
	
	note = (note_t*) melody;
	
	
  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 0;
  tout_val.it_value.tv_sec = 0; /* set timer for "INTERVAL (10) seconds */
  tout_val.it_value.tv_usec = 400000;
  setitimer(ITIMER_REAL, &tout_val,0);

  signal(SIGALRM,alarm_wakeup); /* set the Alarm signal capture */
  
  sleep(1); 
  
  for(int c = 0; c< KB_OPTION_COUNT; c++){
		U_SetKnob(synth, preset[c].knob, preset[c].value);
	}
  
	for (;;)
	{
		while(getchar() != '\n');
		/*
		int rs = SER_Read(port, read_buffer, MAX_COMMAND_LENGTH);
		if (rs > 0)
		{
			// Data was read.
			printf("%s\r\n", read_buffer);
			fflush(stdout);
			//respond_to_command(read_buffer, chars_read);
		}*/
		// The application can perform other tasks here.
	}
	return 0;
}


