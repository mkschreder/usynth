/*******
 -- This file is part of the uSynth project. 
 
 Authors: 
 Martin K. Schröder
*/

#ifndef _USYNTH_PUB_H_
#define _USYNTH_PUB_H_

#include "types.h"

typedef void * handle_t; 

handle_t U_Open();
void U_Close(handle_t h);
int U_NoteOn(handle_t h, uint8_t note, uint8_t velocity);
int U_NoteOff(handle_t h, uint8_t note, uint8_t velocity);
int U_SetKnob(handle_t h, knob_t knob, int8_t value);

//int usSetOption(usynth_register_t reg, int16_t value); 

#endif
