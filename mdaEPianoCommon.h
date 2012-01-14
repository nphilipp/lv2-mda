#ifndef MDA_EPIANO_COMMON_H
#define MDA_EPIANO_COMMON_H

#define NPARAMS 12       //number of parameters
#define NOUTS    2       //number of outputs
#define NVOICES 64       //max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
#define CONTROL_PORT_OFFSET 3 //number of non-control ports

enum Parameter {
	envelope_decay_param,		/* 0: 0.500*/
	envelope_release_param,		/* 1: 0.500*/
	hardness_param,				/* 2: 0.500*/
	treble_boost_param,			/* 3: 0.500*/
	modulation_param,			/* 4: 0.500*/
	lfo_rate_param,				/* 5: 0.650*/
	velocity_sensitivity_param,	/* 6: 0.250*/
	stereo_width_param,			/* 7: 0.500*/
	polyphony_param,			/* 8: 0.500*/
	fine_tuning_param,			/* 9: 0.500*/
	random_tuning_param,		/*10: 0.146*/
	overdrive_param				/*11: 0.000*/
};

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
