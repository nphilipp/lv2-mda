/* ==================================================
 * Native LV2 port of the famous mdaEPiano VSTi
 * ==================================================
 *
 * Port
 *   Author:    Ricardo Wurmus (rekado)
 *   Based on:  mda-vst-src-2010-02-14.zip
 *   Started:   Wed Oct 13, 2010
 *
 * mdaEPiano
 *   Author:    Paul Kellett (paul.kellett@mda-vst.com)
 *
 * ==================================================
 */

#include "mdaEPianoCommon.h"
#include "mdaEPiano.h"
#include <cstdlib> //for exit

#define STRING_BUF 2048
static const char* sample_file = "samples.raw";

mdaEPiano::mdaEPiano(double rate)
  : LV2::Synth<mdaEPianoVoice, mdaEPiano>(p_n_ports, p_midi) {

  //license notice
  printf("\nlv2-mdaEPiano v.0.0.1, Copyright (c) 2010 Ricardo Wurmus\n"
         "    port of mdaEPiano, Copyright (c) 2008 Paul Kellett\n"
         "This is free software, and you are welcome to redistribute it\n"
         "under certain conditions; see LICENSE file for details.\n\n");

  //init global variables
  sustain = 0;

  load_kgrp(kgrp);
  load_samples(&samples);
  tweak_samples();

  // initialise parameters
  params[p_offset(p_envelope_decay)]       = 0.500f;
  params[p_offset(p_envelope_release)]     = 0.500f;
  params[p_offset(p_hardness)]             = 0.500f;
  params[p_offset(p_treble_boost)]         = 0.500f;
  params[p_offset(p_modulation)]           = 0.500f;
  params[p_offset(p_lfo_rate)]             = 0.650f;
  params[p_offset(p_velocity_sensitivity)] = 0.250f;
  params[p_offset(p_stereo_width)]         = 0.500f;
  params[p_offset(p_polyphony)]            = 0.500f;
  params[p_offset(p_fine_tuning)]          = 0.500f;
  params[p_offset(p_random_tuning)]        = 0.146f;
  params[p_offset(p_overdrive)]            = 0.000f;

  for(uint32_t i=0; i<NVOICES; ++i) {
    voices[i] = new mdaEPianoVoice(rate, samples, kgrp, params);
    add_voices(voices[i]);
  }

  add_audio_outputs(p_left, p_right);
}

unsigned mdaEPiano::find_free_voice(unsigned char key, unsigned char velocity) {
  //is this a retriggered note during sustain?
  if (sustain) {
    for (unsigned i = 0; i < NVOICES; ++i) {
      if ((voices[i]->get_key() == key) && (voices[i]->is_sustained()))
        return i;
    }
  }

  //take the next free voice if
  // ... notes are sustained but not this new one
  // ... notes are not sustained
  for (unsigned i = 0; i < NVOICES; ++i) {
    if (voices[i]->get_key() == LV2::INVALID_KEY)
    {
      //TODO: LPF is not used anyway
      //initialize LPF
      //voices[i]->f0 = voices[i]->f1 = 0.0f;

      //return voice's index
      return i;
    }
  }

  //TODO: steal note if all voices are used up
  return 0;
}

void mdaEPiano::setVolume(float value)
{
  for (uint32_t v=0; v<NVOICES; ++v)
    voices[v]->set_volume(value);
}

//TODO: implement run()
//TODO: check if input params have changed; then update
//void run() {
  //update();
//TODO
/*void mdaEPiano::update()
{
  for (uint32_t v=0; v<NVOICES; ++v)
    voices[v]->update(params);
}
*/

void mdaEPiano::handle_midi(uint32_t size, unsigned char* data) {
#ifdef DEBUG
  printf("%d\n", data[1]);
#endif

  //discard invalid midi messages
  if (size != 3)
    return;

  //receive on all channels
  switch(data[0] & 0xf0)
  {
    case 0x80: //note off
      for (unsigned i = 0; i < NVOICES; ++i) {
        if (voices[i]->get_key() == data[1]) {
          voices[i]->release(data[2]);
          break;
        }
      }
      break;

    case 0x90: //note on
      {
        unsigned int v = find_free_voice(data[1], data[2]);
        if (v < NVOICES)
          voices[v]->on(data[1], data[2]);
        break;
      }

    case 0xE0: break; //TODO: pitch bend

    //controller
    case 0xB0:
      // standard controller stuff
      switch(data[1])
      {
        case 0x01: //mod wheel
          //scale the mod value to cover the range [0..1]
          modwhl = scale_midi_to_f(data[2]);
          if(modwhl > 0.05f) //over-ride pan/trem depth
          {
            for (unsigned i = 0; i < NVOICES; ++i) {
              //set lfo depth
              voices[i]->set_lmod(modwhl);
              if(*p(p_modulation) < 0.5f)
                voices[i]->set_rmod(-modwhl);
              else
                voices[i]->set_rmod(modwhl);
            }
          }
          break;

        //volume
        case 0x07:
          setVolume(0.00002f * (float)(data[2] * data[2]));
          break;

        //sustain pedal
        case 0x40:
        //sostenuto pedal
        case 0x42:
          sustain = data[2] & 0x40;

          for (unsigned i = 0; i < NVOICES; ++i) {
            voices[i]->set_sustain(sustain);
            //if pedal was released: dampen sustained notes
            if((sustain==0) && (voices[i]->is_sustained()))
              voices[i]->release(0);
          }
          break;

        //all sound off
        case 0x78:
        //all notes off
        case 0x7b:
          for(short v=0; v<NVOICES; v++)
            voices[v]->reset();
          break;

        default: break;
      }
      break;

    default: break;
  }
}

void mdaEPiano::load_samples(short **buffer)
{
  FILE *f;
  long num, size;
  char filepath[STRING_BUF];

  strncpy(filepath, bundle_path(), STRING_BUF);
  strncat(filepath,
          sample_file,
          STRING_BUF - strlen(filepath));
  f = fopen(filepath, "rb");
  if (f == NULL) {
    fputs("File error", stderr);
    exit(1);
  }

  // obtain file size
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  rewind(f);

  // allocate memory to contain the whole file
  *buffer = (short*) malloc (sizeof(short)*size);
  if (*buffer == NULL) {
    fputs("Memory error", stderr);
    exit(2);
  }

  // copy the file into the buffer
  num = fread(*buffer, 1, size, f);
  if (num != size) {
    fputs ("Reading error", stderr);
    exit (3);
  }
  fclose (f);
  return;
}

// TODO: load this from a file
void mdaEPiano::load_kgrp(KGRP *kgrp)
{
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
  kgrp[4].pos = 41385;    kgrp[4].end = 45760;    kgrp[4].loop = 1633; //was 1636
  kgrp[5].pos = 45761;    kgrp[5].end = 65211;    kgrp[5].loop = 5245;
  kgrp[6].pos = 65212;    kgrp[6].end = 72897;    kgrp[6].loop = 2937;
  kgrp[7].pos = 72898;    kgrp[7].end = 78626;    kgrp[7].loop = 2203; //was 2204
  kgrp[8].pos = 78627;    kgrp[8].end = 100387;   kgrp[8].loop = 6368;
  kgrp[9].pos = 100388;   kgrp[9].end = 116297;   kgrp[9].loop = 10452;
  kgrp[10].pos = 116298;  kgrp[10].end = 127661;  kgrp[10].loop = 5217; //was 5220
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
}

void mdaEPiano::tweak_samples()
{
  //extra xfade looping...
  for(uint32_t k=0; k<28; k++)
  {
    long p0 = kgrp[k].end;
    long p1 = kgrp[k].end - kgrp[k].loop;

    float xf = 1.0f;
    float dxf = -0.02f;

    while(xf > 0.0f)
    {
      samples[p0] = (short)((1.0f - xf) * (float)samples[p0] + xf * (float)samples[p1]);
      p0--;
      p1--;
      xf += dxf;
    }
  }
}

static int _ = mdaEPiano::register_class(p_uri);
