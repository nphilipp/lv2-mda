#ifndef MDA_EPIANO_H
#define MDA_EPIANO_H
//See associated .cpp file for copyright and other info

#include "mdaEPianoVoice.h"
#include "mdaEPiano.peg"

#pragma GCC system_header
#include <lv2synth.hpp>

class mdaEPiano : public LV2::Synth<mdaEPianoVoice, mdaEPiano> {
  public:
    float params[NPARAMS];
    bool sustain;
    float modwhl;
    short *samples;
    KGRP kgrp[34];

    mdaEPianoVoice *voices[NVOICES];

    mdaEPiano(double rate);
    ~mdaEPiano() {
      free(samples);
    }

    void load_kgrp(KGRP*);
    void load_samples(short**);
    void tweak_samples(void);
    void update(void);

    unsigned find_free_voice(unsigned char key, unsigned char velocity);
    void handle_midi(uint32_t size, unsigned char* data);
    void setVolume(float value);
};
#endif
