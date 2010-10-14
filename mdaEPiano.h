//See associated .cpp file for copyright and other info

#ifndef __mdaEPiano__
#define __mdaEPiano__

#include <string.h>

#define NPARAMS 12       //number of parameters
#define NPROGS   8       //number of programs
#define NOUTS    2       //number of outputs
#define NVOICES 64       //max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
#define WAVELEN 422414   //wave data bytes

class mdaEPianoProgram
{
	friend class mdaEPiano;
	private:
	float param[NPARAMS];
	char  name[24];
};

struct KGRP  //keygroup
{
	long root;  //MIDI root note
	long high;  //highest note
	long pos;
	long end;
	long loop;
};
#endif
