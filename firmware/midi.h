#ifndef __UMIDI_H_
#define __UMIDI_H_

#include <inttypes.h>

/** SERIAL COMMAND INTERFACE
 Note play: 
 - byte0: 0x80 note on, 0x90 note off
 - byte1: note 0..127
 - byte2: velocity (volume)
*/
typedef enum {
	CMD_NONE,
	CMD_NOTE_ON = 0x80,
	CMD_NOTE_OFF = 0x90,
	CMD_SET
}midi_command_t; 

typedef void (*midi_command_proc_t)(midi_command_t cmd, uint8_t b1, uint8_t b2, uint8_t b3); 

typedef volatile struct midi_device_s {
	midi_command_proc_t command_callback; 
	
	// private data
	uint8_t 	_state; 
	uint8_t	_cmd; 
	uint8_t 	_data[2]; 
	uint16_t _timeout; 
} midi_device_t; 

void MIDI_Init(midi_device_t *dev); 
void MIDI_Update(midi_device_t *dev); 
void MIDI_ProcessByte(midi_device_t *dev, uint8_t byte); 

#endif
