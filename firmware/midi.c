#include "midi.h"

enum {
	STATE_READY, 
	STATE_RX1,
	STATE_RX2,
	STATE_RX3
}; 

void MIDI_Init(midi_device_t *dev){
	dev->command_callback = 0; 
}

void MIDI_Update(midi_device_t *dev){
	if(dev->_timeout > 10000){
		dev->_state = STATE_READY;
		dev->_timeout = 0; 
	}
	dev->_timeout++; 
}

void MIDI_ProcessByte(midi_device_t *dev, uint8_t byte){
	if(dev->_state == STATE_READY){
		dev->_cmd = byte; 
		dev->_state = STATE_RX1;
		dev->_timeout = 0; 
	}
	else if(dev->_state == STATE_RX1){ // || dev->_state == STATE_RX2){
		dev->_data[dev->_state - STATE_RX1] = byte; 
		dev->_state++; 
		dev->_timeout = 0; 
	} else if(dev->_state == STATE_RX2){
		if(dev->command_callback){
			dev->command_callback(dev->_cmd, dev->_data[0], byte, 0); 
		}
		dev->_state = STATE_READY; 
	}
}

