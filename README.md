uSYNTH Basic
============

uSynth delivers a complete analogue synthesizer in an 8kb package specifically designed for the AVR ATMega88 microcontroller. 

Functions include: 
- 2 oscillator units with 5 waveforms each: left saw, right saw, triangle, square and sin
- 1 low frequency oscillator for vibrato effects
- Four modulation modes: FM, AM, Phase, Sync
- Enhanced lowpass filter
- Attack, decay, sustain, release controls
- Can be programmed through serial interface
- Plays midi notes with corresponding frequencies

uSynth is written completely from scratch to take into account the lack of floating point support on AVR microcontrollers. To tackle this problem, all mathematics involved in sound generation have been rewritten into fixed point integer math. uSynth uses intristic properties of 16 bit integers to enforce compact and fast code. 

PROGRAMMING
===========

To program uSynth, download the archive with firmware and driver code for PC. It is beyond this tutorial to show you how to flash the code to an ATMega chip. However you will find avrdude and USBasp programmer very useful for this. Included makefile is specifically designed with all the commands needed to transfer the firmware onto the chip. 

You can also buy a preprogrammed uSynth chip by sending an email to info@svenskasakerhetsdorrar.se

INTERFACE
=========

uSynth firmware can be programmed over serial interface using a simple event based API. 

	U_NoteOn(
	
Knobs
-----

The following knobs can be adjusted through the control api: 

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
	
All knob values are either 0-127 translated into 0-255 internally (for controls that only support positive values), or -128 to 127 for controls that support both positive and negative values. 

How does it work?
=================

The synth mixes two waveforms, applies a third wave to it, filters the signal and passes it through an envelope. Let's look more closely how the final signal is generated. 

You always start with the basic waveform. 

Square wave: 

![Square Wave](http://i.imgur.com/MkYGSqd.jpg)

Sawtooth wave: 

![Sawtooth Wave](http://i.imgur.com/K9lcyJL.jpg)

Triangle wave: 

![Triangle Wave](http://i.imgur.com/Kfz4H2R.jpg)

Then you mix waveforms from both oscillators to create something more interesting. Here is two sine waves, one octave apart, mixed together at equal amount: 

![Mixed sinusoids](http://i.imgur.com/K0Ebqtu.jpg)

Then you filter the signal as you wish: 

Filtered sawtooth: 

![Filtered Sawtooth](http://i.imgur.com/iqlSnSr.jpg)

Slowly moving the cutoff amount upwards generates this: 

![Cutoff](http://i.imgur.com/mryPODF.jpg)

Then you modulate the signal like this: 

![Frequency Modulation](http://i.imgur.com/DOvOki6.jpg)

![Amplitude Modulation](http://i.imgur.com/W7t5suN.jpg)

And finally you apply the envelope: 

![Envelope](http://i.imgur.com/oco3mst.jpg)

Oscillators
-----------

There are five basic waveforms for the oscillators - sin, left saw, right saw, triangle, square and noise. Sine is the cleanest, saw and triangle (and square) contain many frequencies (harmonics), noise is "everything". 

The waveform can have variable period - this is frequency. It can also have a frequency that is detuned by an ammount of an octave (12 semitones - double the base frequency). The waveform can also have phase that is ofset - mixing two same waveforms with one waveform having phase offset set to 127 produces silence because both waveforms then cancel eachother. 

The waveform frequency can also be altered by a low frequency oscillator (LFO) that shifts the frequency back and forth. KB_LFO_TO_OSC setting specifies the magnitude of the shift related to the current note frequency. 

	U_SetKnob(s, KB_OSC1_WAVEFORM, WAVE_SIN); // sets waveform: SIN, TRIANGLE, SAWR, SAWL, SQUARE, NOISE
	U_SetKnob(s, KB_OSC1_DETUNE, 0); // sets detune amount in semitones (12 semitones = 1 octave)
	U_SetKnob(s, KB_OSC1_FINE_TUNE, 0); // sets fine tune amount in herz
	U_SetKnob(s, KB_OSC1_PHASE_OFFSET, 0); // sets signal phase offset (127 is half a period forwards, -128 is half a period backwards)
	
The phase offset determines how much one waveform is shifted out from the other one. 

Here is a sawtooth wave mixed with a sine wave where the sawtooth is shifted half a period right from the sine wave: 
![Phase offset](http://i.imgur.com/JCfvIw2.jpg) 

The mixer
---------

Mixer takes two oscillator waveforms and ads them together - producing a third waveform with both of the original waveforms mixed in. This signal is now passed to the filter. 

	U_SetKnob(s, KB_OSC_MIX_AMOUNT, 0); // a value -128 to 127
	
-127 means only second oscillator is heard. 127 means only first one is heard. 

The filter
----------

The synth has a low pass filter. It works like a "capacitor" that charges and discharges as the waveform moves. This smoothes the waveform and thus also dampens sharp high frequency harmonics while leaving lower frequency harmonics in tact. 

	U_SetKnob(s, KB_FILTER_CUTOFF, 127); // filter cutoff: 0 = all frequencies are discarded, 127 = all frequencies are heard
	
The LFO
-------

Low frequecy oscillator is used to create vibrato effects. 

	U_SetKnob(s, KB_LFO_SPEED, 40); // lfo speed in Hz
	U_SetKnob(s, KB_LFO_TO_OSC, 0); // how much lfo influences the oscillator frequecy (FM)
	U_SetKnob(s, KB_LFO_TO_FILTER, 0); // how much lfo influences the filter cutoff
	U_SetKnob(s, KB_LFO_TO_AMP, 0); // how much lfo is effecting the amplitude of the signal (AM)

Envelope
--------

Envelope determines the final shape of the sound and how it's volume changes over time. This is done with four settings: attack, decay, sustain and release. 

	U_SetKnob(s, KB_AMP_ENV_ATTACK, 0); // attack (32 = one second)
	U_SetKnob(s, KB_AMP_ENV_DECAY, 127); 
	U_SetKnob(s, KB_AMP_ENV_SUSTAIN, 127);
	U_SetKnob(s, KB_AMP_ENV_RELEASE, 0);

