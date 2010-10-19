#ifndef MDA_EPIANO_COMMON_H
#define MDA_EPIANO_COMMON_H

#define NPARAMS 12       //number of parameters
#define NPROGS   8       //number of programs
#define NOUTS    2       //number of outputs
#define NVOICES 64       //max polyphony
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
//#define WAVELEN 422414   //wave data bytes

enum Parameter {
	envelope_decay_param,	/* 0: 0.500*/
	envelope_release_param,	/* 1: 0.500*/
	hardness_param,			/* 2: 0.500*/
	treble_boost_param,		/* 3: 0.500*/
	mod_param,				/* 4: 0.500*/
	lfo_rate_param,			/* 5: 0.650*/
	velosense_param,		/* 6: 0.250*/
	stereo_width_param,		/* 7: 0.500*/
	polyphony_param,		/* 8: 0.50*/
	fine_tuning_param,		/* 9: 0.500*/
	random_tuning_param,	/*10: 0.146*/
	overdrive_param			/*11: 0.000*/
};

static float scale_midi_to_f(unsigned char data)
{
	float scaled;
	scaled = 0.0078f * (float)(data);
	return scaled;
}

#endif
