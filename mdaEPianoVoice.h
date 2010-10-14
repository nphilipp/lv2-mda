#ifndef MDA_EPIANO_VOICE_H
#define MDA_EPIANO_VOICE_H

#include "mdaEPianoCommon.h"
//#include "mdaEPianoData.h"
#include <lv2synth.hpp>
#include "mdaEPiano.peg"


/* TODO:
 * - tuning is off when rate is not 44100
 * - retriggering sustained notes just sounds wrong and clicky
 */

struct KGRP  //keygroup
{
	long root;  //MIDI root note
	long high;  //highest note
	long pos;
	long end;
	long loop;
};

class mdaEPianoVoice : public LV2::Voice {
	private:
		float Fs, iFs;

		///global internal variables
		KGRP  kgrp[34];
		short *waves;
		short sustain;
		float width;
		long size;
		float lfo0, lfo1, dlfo, lmod, rmod;
		float treb, tfrq, tl, tr;
		float tune, fine, random, stretch, overdrive;
		float muff, muffvel, sizevel, velsens, volume;

		//begin --- from VOICE
		long  delta;  //sample playback
		long  frac;
		long  pos;
		long  end;
		long  loop;

		float env;  //envelope
		float dec;

		float f0;   //first-order LPF
		float f1;
		float ff;

		float outl;
		float outr;
		unsigned short note; //remember what note triggered this
		//end --- from VOICE

		float param[NPARAMS];

	protected:
		unsigned char m_key;

	public:
		mdaEPianoVoice(double rate);
		void setParams(float *p);
		void set_sustain(unsigned short v);
		void set_volume(float v);
		void set_dec(float v);
		void set_muff(float v);
		void set_lmod(float v);
		void set_rmod(float v);

		void update(void);
		void on(unsigned char key, unsigned char velocity);
		void off(unsigned char velocity);
		bool is_sustained(void);
		unsigned char get_key(void) const;

		// generates the sound for this voice
		void render(uint32_t from, uint32_t to);
};

#endif
