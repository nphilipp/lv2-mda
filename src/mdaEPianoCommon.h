#ifndef MDA_EPIANO_COMMON_H
#define MDA_EPIANO_COMMON_H

#define NPARAMS 12       //number of parameters
#define NOUTS    2       //number of outputs
#define NVOICES 64       //max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
#define CONTROL_PORT_OFFSET 3 //number of non-control ports

struct KGRP  //keygroup
{
	long root;  //MIDI root note
	long high;  //highest note
	long pos;
	long end;
	long loop;
};

static float scale_midi_to_f(unsigned char data)
{
	float scaled;
	scaled = 0.0078f * (float)(data);
	return scaled;
}

#endif
