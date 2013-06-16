#define _POSIX_C_SOURCE 199309L
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "firmware/usynth.h"

#define SAMPLE_RATE (65535 / 4)

/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_write(int fd, const void*data, size_t size) {
    ssize_t ret = 0;

    while (size > 0) {
        ssize_t r;

        if ((r = write(fd, data, size)) < 0)
            return r;

        if (r == 0)
            break;

        ret += r;
        data = (const uint8_t*) data + r;
        size -= (size_t) r;
    }

    return ret;
}

int output_samples(synth_t *s, pa_simple *pa){
	int error;
	uint8_t buf[1024];
	U_PlayNote(s, 2, 3, 0); 
	uint8_t pressed = 1; 
	for(double sec = 0; sec < SAMPLE_RATE * 2; sec++){	
		if(pressed && sec > SAMPLE_RATE){
			U_ReleaseNote(s, 2, 3, 0); 
			pressed = 0; 
		}
		for (int i = 0; i<1024;i++,sec++){
			buf[i] = U_GenSample(s); 
		}
		if (pa_simple_write(pa, &buf, 1024, &error) < 0) {
			fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
			return -1; 
		}
		if(1){
			if (loop_write(STDOUT_FILENO, buf, sizeof(buf)) != sizeof(buf)) {
				fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
				return -1; 
			}
		}
	} 
}

// simple sin waveform 
void init_test_sin(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 40);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 10);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 8); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 64);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}

void init_test_saw(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SAWR);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_NOISE);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 40);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 0); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 127);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}

void init_test_triangle(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_TRIANGLE);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_NOISE);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 40);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}

void init_test_square(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SQUARE);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_NOISE);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 40);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}
// mixing of two harmonics
void init_test_mix(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC2_DETUNE, 12);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 0);
	U_SetKnob(s, KB_LFO_SPEED, 40);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}

// two identical waveforms where one is offset by 90degrees
void init_test_phase(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 127);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SAWR);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 0);
	U_SetKnob(s, KB_LFO_SPEED, 40);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}

void init_test_lowpass(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SAWR);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 40);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 30); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}


void init_test_lfo_freq(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 4);
	U_SetKnob(s, KB_LFO_TO_OSC, 127);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}


void init_test_lfo_filter(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 4);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 127);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 64); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}

void init_test_lfo_volume(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 4);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 64);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,64);
}

void init_test_envelope(synth_t *s){
	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC1_DETUNE, 0);
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC2_WAVEFORM, WAVE_SIN);
	U_SetKnob(s, KB_OSC2_DETUNE, 0);
	U_SetKnob(s, KB_OSC2_FINE_TUNE, 0);
	U_SetKnob(s, KB_OSC2_PHASE_OFFSET, 0);
	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 127);
	U_SetKnob(s, KB_LFO_SPEED, 4);
	U_SetKnob(s, KB_LFO_TO_OSC, 0);
	U_SetKnob(s, KB_LFO_TO_FILTER, 0);
	U_SetKnob(s, KB_LFO_TO_AMP, 0);
	U_SetKnob(s, KB_AMP_ENV_ATTACK, 64);
	U_SetKnob(s, KB_AMP_ENV_DECAY, 16); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 64);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 32);
	U_SetKnob(s, KB_FILTER_CUTOFF, 127); 
	U_SetKnob(s, KB_FILTER_ENV_AMOUNT, 0);
	U_SetKnob(s, KB_FILTER_ATTACK, 0);
	U_SetKnob(s, KB_FILTER_DECAY, 0);
	U_SetKnob(s, KB_FILTER_SUSTAIN, 127);
	U_SetKnob(s, KB_FILTER_RELEASE, 0);
	U_SetKnob(s, KB_AMP_VOLUME,127);
}


int main(){
	synth_t synth; 
	
	
	static const pa_sample_spec ss = {
		.format = PA_SAMPLE_U8,
		.rate = SAMPLE_RATE,
		.channels = 1
	};

	pa_simple *s = NULL;
	int ret = 1;
	int error;

	/* Create a new playback stream */
	if (!(s = pa_simple_new(NULL, NULL, PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
			fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
			return -1; 
	}
	
	U_Init(&synth, SAMPLE_RATE); 
	
	// sinus wave test
	init_test_sin(&synth); 
	output_samples(&synth, s);
	
	init_test_saw(&synth); 
	output_samples(&synth, s);
	
	init_test_triangle(&synth); 
	output_samples(&synth, s);
	
	init_test_square(&synth); 
	output_samples(&synth, s);
	
	init_test_mix(&synth); 
	output_samples(&synth, s);
	
	init_test_phase(&synth); 
	output_samples(&synth, s); 
	
	init_test_lowpass(&synth); 
	output_samples(&synth, s); 
	
	init_test_lfo_freq(&synth); 
	output_samples(&synth, s); 
	
	init_test_lfo_filter(&synth); 
	output_samples(&synth, s); 
	
	init_test_lfo_volume(&synth); 
	output_samples(&synth, s); 
	
	init_test_envelope(&synth); 
	output_samples(&synth, s); 
}
