#ifndef MDA_EPIANO_VOICE_H
#define MDA_EPIANO_VOICE_H

#include "mdaEPianoCommon.h"
#include "mdaEPiano.peg"

#pragma GCC system_header
#include <lv2synth.hpp>

enum Param {
  Default,
  Current
};

class mdaEPianoVoice : public LV2::Voice {
  private:
    float Fs, iFs;

    /// global internal variables
    KGRP  *kgrp;
    short *waves;
    short sustain;
    float width;
    long size;
    float lfo0, lfo1, dlfo, lmod, rmod;
    float treb, tfrq, tl, tr;
    float tune, fine, random, stretch, overdrive;
    float muff, muffvel, sizevel, velsens, volume;

    //begin --- from VOICE
    long  delta;  // sample playback
    long  frac;
    long  pos;
    long  end;
    long  loop;

    float env;  // envelope
    float dec;

    float f0;   // first-order LPF
    float f1;
    float ff;

    float outl;
    float outr;
    unsigned short note; // remember what note triggered this
    //end --- from VOICE

    float default_preset[NPARAMS]; // contains the default preset

  protected:
    unsigned char m_key;

  public:
    mdaEPianoVoice(double, short*, KGRP*);
    void set_sustain(unsigned short v) { sustain = v; }
    void set_volume(float v) { volume = v; }
    void set_lmod(float v) { lmod = v; }
    void set_rmod(float v) { rmod = v; }

    float p_helper(unsigned short, Param);
    void update(Param); // recalculates internal variables
    void on(unsigned char key, unsigned char velocity);
    void release(unsigned char velocity);
    void reset(void);
    bool is_sustained(void) { return (note == SUSTAIN); }
    unsigned char get_key(void) const { return m_key; }

    // generates the sound for this voice
    void render(uint32_t from, uint32_t to);
};

#endif
