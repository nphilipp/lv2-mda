#include "mdaEPianoVoice.h"

mdaEPianoVoice::mdaEPianoVoice(double rate, short *samples, KGRP * master_kgrp, float * master_params)
{
  //set tuning
  Fs = rate;
  iFs = 1.0f/Fs;

  waves  = samples;
  kgrp   = master_kgrp;
  params = master_params;

  init();
  reset();
  volume = 0.2f;
  for (unsigned char id=0; id<NPARAMS; id++)
    update(id);
}

void mdaEPianoVoice::init(void) {
  sustain = 0;
  width = 0;
  size = 0;

  lfo0 = lfo1 = dlfo = lmod = rmod = 0;
  treb = tfrq = tl = tr = 0;
  tune = fine = random = stretch = overdrive = 0;
  muff = muffvel = sizevel = velsens = volume = 0;

  delta = frac = pos = end = loop = 0;
  env = dec = 0;

  f0 = f1 = ff = 0;

  outl = outr = 0;
  note = 0;
}

void mdaEPianoVoice::update(unsigned char id)
{
  id = p_offset(id);
  switch(id)
  {
    case p_hardness:
      size = (long)(12.0f * params[id] - 6.0f);
      break;
    case p_treble_boost:
      treb = 4.0f * params[id] * params[id] - 1.0f; // treble gain
      if(params[id] > 0.5f) tfrq = 14000.0f; else tfrq = 5000.0f; // treble freq
      tfrq = 1.0f - (float)exp(-iFs * tfrq);
      break;
    case p_modulation:
      rmod = lmod = params[id] + params[id] - 1.0f; // lfo depth
      if(params[id] < 0.5f) rmod = -rmod;
      break;
    case p_lfo_rate:
      dlfo = 6.283f * iFs * (float)exp(6.22f * params[id] - 2.61f); // lfo rate
      break;
    case p_velocity_sensitivity:
      velsens = 1.0f + params[id] + params[id];
      if(params[id] < 0.25f) velsens -= 0.75f - 3.0f * params[id];
      break;
    case p_stereo_width:
      width = 0.03f * params[p_stereo_width];
      break;
    case p_polyphony:
      //poly = 1 + (long)(31.9f * params[id]);
      break;
    case p_fine_tuning:
      fine = params[id] - 0.5f;
      break;
    case p_random_tuning:
      random = 0.077f * params[id] * params[id];
      break;
    case p_overdrive:
      overdrive = 1.8f * params[id];
      break;
    default:
#ifdef DEBUG
      printf("UNKNOWN: %d\n", id);
#endif
      break;
  }
  // TODO: add envelope_decay and envelope_release
  /*
   * TODO: what about stretch?
   * stretch = 0.0f; //0.000434f * (params[p_overdrive] - 0.5f); parameter re-used for overdrive!
   * overdrive = 1.8f * params[p_overdrive];
   */
  stretch = 0.0f; //0.000434f * (params[p_overdrive] - 0.5f); parameter re-used for overdrive!
}

void mdaEPianoVoice::on(unsigned char key, unsigned char velocity)
{
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
    l = fine + random * ((float)(k % 13) - 6.5f); // random & fine tune
    if(key > 60) l += stretch * (float)k; // stretch

    s = size;
    //if(velocity > 40) s += (uint32_t)(sizevel * (float)(velocity - 40));  - no velocity to hardness in ePiano

    k = 0;
    while(key > (kgrp[k].high + s)) k += 3; // find keygroup
    l += (float)(key - kgrp[k].root); // pitch
    l = 32000.0f * iFs * (float)exp(0.05776226505 * l);
    delta = (long)(65536.0f * l);
    frac = 0;

    if(velocity > 48) k++; // mid velocity sample
    if(velocity > 80) k++; // high velocity sample
    pos = kgrp[k].pos;
    end = kgrp[k].end - 1;
    loop = kgrp[k].loop;

    env = (3.0f + 2.0f * velsens) * (float)pow(0.0078f * velocity, velsens); // velocity

    if(key > 60) env *= (float)exp(0.01f * (float)(60 - key)); // high notes quieter

    l = 50.0f + params[p_offset(p_modulation)] * params[p_offset(p_modulation)] * muff + muffvel * (float)(velocity - 64); // muffle
    if(l < (55.0f + 0.4f * (float)key)) l = 55.0f + 0.4f * (float)key;
    if(l > 210.0f) l = 210.0f;
    ff = l * l * iFs;

    // store the pressed key in this voice
    note = key; // note->pan
    if(key <  12) key = 12;
    if(key > 108) key = 108;
    outr = volume + volume * width * (float)(key - 60);
    outl = volume + volume - outr;

    if(key < 44) key = 44; // limit max decay length
    dec = (float)exp(-iFs * exp(-1.0 + 0.03 * (double)key - 2.0f * params[p_offset(p_envelope_decay)]));
  }
}

void mdaEPianoVoice::reset()
{
  env = 0.0f;
  dec = 0.99f;
  muff = 160.0f;
  sustain = 0;
  tl = tr = lfo0 = dlfo = 0.0f;
  lfo1 = 1.0f;
  m_key = LV2::INVALID_KEY;
}

void mdaEPianoVoice::release(unsigned char velocity)
{
  if(sustain==0) {
    dec = (float)exp(-iFs * exp(6.0 + 0.01 * (double)note - 5.0 * params[p_offset(p_envelope_release)]));
  } else {
    note = SUSTAIN;
  }

  //Mark the voice to be turned off later. It may not be set to
  //INVALID_KEY yet, because the release sound still needs to be
  //rendered.  m_key is finally set to INVALID_KEY by 'render' when
  //env < SILENCE
  m_key = SUSTAIN;
}

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

    frac += delta;  // integer-based linear interpolation
    pos += frac >> 16; // no idea what frac is
    frac &= 0xFFFF; // why AND it with all ones?
    if(pos > end) pos -= loop; // jump back to loop sample

    // alternative method (probably faster), said to not work on intel mac
    //i = waves[pos];
    //i = (i << 7) + (frac >> 9) * (waves[pos + 1] - i) + 0x40400000;

    i = waves[pos] + ((frac * (waves[pos + 1] - waves[pos])) >> 16);
    x = env * (float)i / 32768.0f;
    env = env * dec;  // envelope

    if(x>0.0f) { x -= od * x * x;  if(x < -env) x = -env; } //+= 0.5f * x * x;  //overdrive

    l += outl * x;
    r += outr * x;

    // treble boost
    tl += tfrq * (l - tl);
    tr += tfrq * (r - tr);
    r  += treb * (r - tr);
    l  += treb * (l - tl);

    lfo0 += dlfo * lfo1; // LFO for tremolo and autopan
    lfo1 -= dlfo * lfo0;
    l += l * lmod * lfo1;
    r += r * rmod * lfo1; // TODO: worth making all these local variables?

    // write to output
    p(p_left)[frame] += l;
    p(p_right)[frame] += r;
  }

  // turn off further processing when the envelope has rendered the voice silent
  if (env < SILENCE)
    m_key = LV2::INVALID_KEY;

  if(fabs(tl)<1.0e-10) tl = 0.0f; // anti-denormal
  if(fabs(tr)<1.0e-10) tr = 0.0f;
}
