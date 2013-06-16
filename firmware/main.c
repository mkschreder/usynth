// DDS output thru PWM on timer0 OC0A (pin B.3)
// Mega644 version
// FM synthesis
#define F_CPU 18432000UL

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h> 		// for sine
#include <stdio.h>
#include <avr/pgmspace.h>
//#include "local.h"
#include "control.h"
#include "uart.h"
#include "util.h"
#include "../driver/types.h"
#include "usynth.h"

uint16_t rand();

static volatile synth_t synth; 
static volatile uint16_t time = 0; 

ISR (TIMER1_COMPA_vect) { 
	// turn on timer for profiling
	TCNT2 = 0; TCCR2B = 2;
	
	OCR0A = U_GenSample(&synth);
	
	time++; 
	// profiling 
	TCCR2B = 0;
} 
/*
static uint16_t freeMem () { 
	extern int __heap_start, *__brkval;
  int v; 
  #pragma GCC diagnostic ignored "-Wreturn-local-addr"
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
*/
/////////////////////////////////////////////////////

#define AUDIO_OUT(reg) BIT(D, 6, reg)

static 
void midi_command_proc(midi_command_t cmd, uint8_t byte0, uint8_t byte1, uint8_t byte2){
	if(cmd == CMD_NOTE_ON){
		cli(); 
		U_PlayNoteRaw(&synth, byte0);
		//U_PlayNote(&synth, 0, 3, 0);
		sei(); 
		uart_putchar('K');
	} else if(cmd == CMD_NOTE_OFF){
		cli();
		U_ReleaseNoteRaw(&synth, byte0); 
		sei();
		uart_putchar('F');
	} else if(cmd == CMD_SET_KNOB){
		U_SetKnob(&synth, byte0, byte1); 
	} else {
		uart_putchar('N');
		uart_putchar(cmd);
	}
}

#include <stdio.h>

int main(void){
	AUDIO_OUT(DDR) = OUTPUT;

	UART_Init((F_CPU / (16UL * 9600)) - 1);

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

	U_Init(&synth, SAMPLES_PER_SECOND);
	
	// turn on all ISRs
	sei() ;
	
	printf("READY!\r\n");
  ////////////////////////////////////////////////////

	static midi_device_t midi; 
	MIDI_Init(&midi); 
	midi.command_callback = midi_command_proc; 
	
	static FILE out = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
	static FILE in = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
	stdout = &out; 
	stdin = &in; 
	
	while(1) {  
		MIDI_Update(&midi); 
		if(UCSR0A & _BV(RXC0)){
			uint8_t b = UDR0;
			if(MIDI_ProcessByte(&midi, b)){
				cli();
				while(1){
					if(UCSR0A & _BV(RXC0) && !MIDI_ProcessByte(&midi, UDR0))
						break; 
					if(MIDI_Update(&midi))
						break; 
				}
				sei();
			}
			//midi_device_process(&midi);
			//uint8_t ch = getchar(); 
		}
	}
}
