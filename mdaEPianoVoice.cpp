/* ==================================================
 * Native LV2 port of the famous mdaEPiano VSTi
 * ==================================================
 *
 * Port
 * 		Author:			Ricardo Wurmus (rekado)
 * 		Based on:		mda-vst-src-2010-02-14.zip
 *		Latest change:	Wed Oct 13, 2010
 *
 * mdaEPiano
 * 		Author:			Paul Kellett
 * 						paul.kellett@mda-vst.com
 *
 * ==================================================
 */

#include "mdaEPianoVoice.h"

/* TODO:
 * - tuning is off when rate is not 44100
 * - retriggering sustained notes just sounds wrong and clicky
 */


mdaEPianoVoice::mdaEPianoVoice(double rate)
	: m_key(LV2::INVALID_KEY) {
		//set tuning
		//Fs = 44100.0f; //rate; //TODO
		Fs = 48000.0f;
		iFs = 1.0f/Fs;

		//load sample data (maximum value seems to be -24284)
		extern short epianoData;
		waves = &epianoData;

		//Waveform data and keymapping
		kgrp[ 0].root = 36;  kgrp[ 0].high = 39; //C1
		kgrp[ 3].root = 43;  kgrp[ 3].high = 45; //G1
		kgrp[ 6].root = 48;  kgrp[ 6].high = 51; //C2
		kgrp[ 9].root = 55;  kgrp[ 9].high = 57; //G2
		kgrp[12].root = 60;  kgrp[12].high = 63; //C3
		kgrp[15].root = 67;  kgrp[15].high = 69; //G3
		kgrp[18].root = 72;  kgrp[18].high = 75; //C4
		kgrp[21].root = 79;  kgrp[21].high = 81; //G4
		kgrp[24].root = 84;  kgrp[24].high = 87; //C5
		kgrp[27].root = 91;  kgrp[27].high = 93; //G5
		kgrp[30].root = 96;  kgrp[30].high =999; //C6

		kgrp[0].pos = 0;        kgrp[0].end = 8476;     kgrp[0].loop = 4400;  
		kgrp[1].pos = 8477;     kgrp[1].end = 16248;    kgrp[1].loop = 4903;  
		kgrp[2].pos = 16249;    kgrp[2].end = 34565;    kgrp[2].loop = 6398;  
		kgrp[3].pos = 34566;    kgrp[3].end = 41384;    kgrp[3].loop = 3938;  
		kgrp[4].pos = 41385;    kgrp[4].end = 45760;    kgrp[4].loop = 1633; //was 1636;  
		kgrp[5].pos = 45761;    kgrp[5].end = 65211;    kgrp[5].loop = 5245;  
		kgrp[6].pos = 65212;    kgrp[6].end = 72897;    kgrp[6].loop = 2937;  
		kgrp[7].pos = 72898;    kgrp[7].end = 78626;    kgrp[7].loop = 2203; //was 2204;  
		kgrp[8].pos = 78627;    kgrp[8].end = 100387;   kgrp[8].loop = 6368;  
		kgrp[9].pos = 100388;   kgrp[9].end = 116297;   kgrp[9].loop = 10452;  
		kgrp[10].pos = 116298;  kgrp[10].end = 127661;  kgrp[10].loop = 5217; //was 5220; 
		kgrp[11].pos = 127662;  kgrp[11].end = 144113;  kgrp[11].loop = 3099;  
		kgrp[12].pos = 144114;  kgrp[12].end = 152863;  kgrp[12].loop = 4284;  
		kgrp[13].pos = 152864;  kgrp[13].end = 173107;  kgrp[13].loop = 3916;  
		kgrp[14].pos = 173108;  kgrp[14].end = 192734;  kgrp[14].loop = 2937;  
		kgrp[15].pos = 192735;  kgrp[15].end = 204598;  kgrp[15].loop = 4732;  
		kgrp[16].pos = 204599;  kgrp[16].end = 218995;  kgrp[16].loop = 4733;  
		kgrp[17].pos = 218996;  kgrp[17].end = 233801;  kgrp[17].loop = 2285;  
		kgrp[18].pos = 233802;  kgrp[18].end = 248011;  kgrp[18].loop = 4098;  
		kgrp[19].pos = 248012;  kgrp[19].end = 265287;  kgrp[19].loop = 4099;  
		kgrp[20].pos = 265288;  kgrp[20].end = 282255;  kgrp[20].loop = 3609;  
		kgrp[21].pos = 282256;  kgrp[21].end = 293776;  kgrp[21].loop = 2446;  
		kgrp[22].pos = 293777;  kgrp[22].end = 312566;  kgrp[22].loop = 6278;  
		kgrp[23].pos = 312567;  kgrp[23].end = 330200;  kgrp[23].loop = 2283;  
		kgrp[24].pos = 330201;  kgrp[24].end = 348889;  kgrp[24].loop = 2689;  
		kgrp[25].pos = 348890;  kgrp[25].end = 365675;  kgrp[25].loop = 4370;  
		kgrp[26].pos = 365676;  kgrp[26].end = 383661;  kgrp[26].loop = 5225;  
		kgrp[27].pos = 383662;  kgrp[27].end = 393372;  kgrp[27].loop = 2811;  
		kgrp[28].pos = 383662;  kgrp[28].end = 393372;  kgrp[28].loop = 2811; //ghost
		kgrp[29].pos = 393373;  kgrp[29].end = 406045;  kgrp[29].loop = 4522;  
		kgrp[30].pos = 406046;  kgrp[30].end = 414486;  kgrp[30].loop = 2306;  
		kgrp[31].pos = 406046;  kgrp[31].end = 414486;  kgrp[31].loop = 2306; //ghost
		kgrp[32].pos = 414487;  kgrp[32].end = 422408;  kgrp[32].loop = 2169;  

		//extra xfade looping...
		for(uint32_t k=0; k<28; k++)
		{
			long p0 = kgrp[k].end;
			long p1 = kgrp[k].end - kgrp[k].loop;

			float xf = 1.0f;
			float dxf = -0.02f;

			while(xf > 0.0f)
			{
				waves[p0] = (short)((1.0f - xf) * (float)waves[p0] + xf * (float)waves[p1]);
				p0--;
				p1--;
				xf += dxf;
			}
		}

		//initialise...
		env = 0.0f;
		dec = 0.99f; //all notes off

		volume = 0.2f;
		muff = 160.0f;
		sustain = 0;
		tl = tr = lfo0 = dlfo = 0.0f;
		lfo1 = 1.0f;

		update();
	}

void mdaEPianoVoice::setParams(float *p)
{
	for(unsigned char i=0; i<NPARAMS; ++i)
		param[i] = p[i];
}

//set functions
void mdaEPianoVoice::set_sustain(unsigned short v) 	{ sustain = v; }
void mdaEPianoVoice::set_volume(float v)		 	{ volume = v; }
void mdaEPianoVoice::set_dec(float v) 				{ dec = v; }
void mdaEPianoVoice::set_muff(float v) 				{ muff = v; }
void mdaEPianoVoice::set_lmod(float v) 				{ lmod = v; }
void mdaEPianoVoice::set_rmod(float v) 				{ rmod = v; }

void mdaEPianoVoice::update()
{
	size = (long)(12.0f * param[2] - 6.0f);
	treb = 4.0f * param[3] * param[3] - 1.0f; //treble gain
	if(param[3] > 0.5f) tfrq = 14000.0f; else tfrq = 5000.0f; //treble freq
	tfrq = 1.0f - (float)exp(-iFs * tfrq);

	rmod = lmod = param[4] + param[4] - 1.0f; //lfo depth
	if(param[4] < 0.5f) rmod = -rmod;
	dlfo = 6.283f * iFs * (float)exp(6.22f * param[5] - 2.61f); //lfo rate

	velsens = 1.0f + param[6] + param[6];
	if(param[6] < 0.25f) velsens -= 0.75f - 3.0f * param[6];

	width = 0.03f * param[7];
	//poly = 1 + (long)(31.9f * param[8]);
	fine = param[9] - 0.5f;
	random = 0.077f * param[10] * param[10];
	stretch = 0.0f; //0.000434f * (param[11] - 0.5f); parameter re-used for overdrive!
	overdrive = 1.8f * param[11];
}

void mdaEPianoVoice::on(unsigned char key, unsigned char velocity) { 
	// store key that turned this voice on (used in 'get_key')
	m_key = key;

	// get the keygroup
	float l=99.0f;
	long k, s;

	// initialize the LPF (which doesn't seem to be used)
	f0 = f1 = 0.0f;

	// TODO: some keyboards send note off events as 'note on \w velocity 0'
	if(velocity > 0) 
	{
		k = (key - 60) * (key - 60);
		l = fine + random * ((float)(k % 13) - 6.5f);  //random & fine tune
		if(key > 60) l += stretch * (float)k; //stretch

		s = size;
		//if(velocity > 40) s += (uint32_t)(sizevel * (float)(velocity - 40));  - no velocity to hardness in ePiano

		k = 0;
		while(key > (kgrp[k].high + s)) k += 3;  //find keygroup
		l += (float)(key - kgrp[k].root); //pitch
		l = 32000.0f * iFs * (float)exp(0.05776226505 * l);
		delta = (long)(65536.0f * l);
		frac = 0;

		if(velocity > 48) k++; //mid velocity sample
		if(velocity > 80) k++; //high velocity sample
		pos = kgrp[k].pos;
		end = kgrp[k].end - 1;
		loop = kgrp[k].loop;

		env = (3.0f + 2.0f * velsens) * (float)pow(0.0078f * velocity, velsens); //velocity

		if(key > 60) env *= (float)exp(0.01f * (float)(60 - key)); //new! high notes quieter

		l = 50.0f + param[4] * param[4] * muff + muffvel * (float)(velocity - 64); //muffle
		if(l < (55.0f + 0.4f * (float)key)) l = 55.0f + 0.4f * (float)key;
		if(l > 210.0f) l = 210.0f;
		ff = l * l * iFs;

		//store the pressed key in this voice
		note = key; //note->pan
		if(key <  12) key = 12;
		if(key > 108) key = 108;
		l = volume;
		outr = l + l * width * (float)(key - 60);
		outl = l + l - outr;

		if(key < 44) key = 44; //limit max decay length
		dec = (float)exp(-iFs * exp(-1.0 + 0.03 * (double)key - 2.0f * param[0]));
	}
	/*
	   else { //note off
	   if(sustain==0) {
	   dec = (float)exp(-iFs * exp(6.0 + 0.01 * (double)note - 5.0 * param[1]));
	   } else {
	   note = SUSTAIN;
	   }
	   }
	   */
}

void mdaEPianoVoice::off(unsigned char velocity) { 
	if(sustain==0) {
		dec = (float)exp(-iFs * exp(6.0 + 0.01 * (double)note - 5.0 * param[1]));
	} else {
		note = SUSTAIN;
	}

	//Mark the voice to be turned off later. It may not be set to
	//INVALID_KEY yet, because the release sound still needs to be
	//rendered.  m_key is finally set to INVALID_KEY by 'render' when
	//env < SILENCE
	m_key = SUSTAIN;
}

bool mdaEPianoVoice::is_sustained() { return (note == SUSTAIN); }
unsigned char mdaEPianoVoice::get_key() const { return m_key; }

// generates the sound for this voice
void mdaEPianoVoice::render(uint32_t from, uint32_t to) 
{
	// abort if no key is pressed
	// initially m_key is INVALID_KEY, so no sound will be rendered
	if (m_key == LV2::INVALID_KEY)
		return;

	float x, l, r, od=overdrive;
	long i;

	for (uint32_t frame = from; frame < to; ++frame) {
		// initialize left and right output
		l = r = 0.0f;

		frac += delta;  //integer-based linear interpolation
		pos += frac >> 16; //no idea what frac is
		frac &= 0xFFFF; //why AND it with all ones?
		if(pos > end) pos -= loop; //jump back to loop sample

		// alternative method (probably faster), said to not work on intel mac
		//i = waves[pos];
		//i = (i << 7) + (frac >> 9) * (waves[pos + 1] - i) + 0x40400000;

		i = waves[pos] + ((frac * (waves[pos + 1] - waves[pos])) >> 16);
		x = env * (float)i / 32768.0f;

		//same thing here, but didn't work
		//x = env * (*(float *)&i - 3.0f);  //fast int->float

		env = env * dec;  //envelope

		if(x>0.0f) { x -= od * x * x;  if(x < -env) x = -env; } //+= 0.5f * x * x;  //overdrive

		l += outl * x;
		r += outr * x;

		tl += tfrq * (l - tl);  //treble boost
		tr += tfrq * (r - tr);
		r  += treb * (r - tr);
		l  += treb * (l - tl);

		lfo0 += dlfo * lfo1;  //LFO for tremolo and autopan
		lfo1 -= dlfo * lfo0;
		l += l * lmod * lfo1;
		r += r * rmod * lfo1;  //worth making all these local variables?

		// write to output
		p(p_left)[frame] += l;
		p(p_right)[frame] += r;
	}

	// turn off further processing when the envelope has rendered the voice silent
	if (env < SILENCE)
		m_key = LV2::INVALID_KEY;

	if(fabs(tl)<1.0e-10) tl = 0.0f; //anti-denormal
	if(fabs(tr)<1.0e-10) tr = 0.0f;
}
