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

#include "mdaEPianoCommon.h"
#include "mdaEPiano.h"
#include "mdaEPianoVoice.h"
#include <lv2synth.hpp>
#include <math.h>
#include <iostream>
#include "mdaEPiano.peg"

/* TODO:
 * - tuning is off when rate is not 44100
 * - retriggering sustained notes just sounds wrong and clicky
 */

class mdaEPiano : public LV2::Synth<mdaEPianoVoice, mdaEPiano> {
	public:
		uint32_t curProgram;
		bool sustain;
		float modwhl;

		//container for all voices
		mdaEPianoVoice *voices[NVOICES];
		mdaEPianoProgram programs[NPROGS];

		mdaEPiano(double rate)
			: LV2::Synth<mdaEPianoVoice, mdaEPiano>(p_n_ports, p_midi) {

				//license notice
				std::cout << std::endl;
				std::cout << "lv2-mdaEPiano v.0.0.1, Copyright (c) 2010 Ricardo Wurmus" << std::endl;
				std::cout << "    port of mdaEPiano, Copyright (c) 2008 Paul Kellett" << std::endl;
				std::cout << "This is free software, and you are welcome to redistribute it" << std::endl;
				std::cout << "under certain conditions; see LICENSE file for details." << std::endl;
				std::cout << std::endl;

				//init global variables
				sustain = 0;

				if(programs)
				{
					//fill patches...
					uint32_t i=0;
					/* param[0] -
					 * param[1] -
					 * ...
					 * param[11] - overdrive
					 */
					fillpatch(i++, "Default", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.146f, 0.000f);
					fillpatch(i++, "Bright", 0.500f, 0.500f, 1.000f, 0.800f, 0.500f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.146f, 0.500f);
					fillpatch(i++, "Mellow", 0.500f, 0.500f, 0.000f, 0.000f, 0.500f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.246f, 0.000f);
					fillpatch(i++, "Autopan", 0.500f, 0.500f, 0.500f, 0.500f, 0.250f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.246f, 0.000f);
					fillpatch(i++, "Tremolo", 0.500f, 0.500f, 0.500f, 0.500f, 0.750f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.246f, 0.000f);
					fillpatch(i++, "(default)", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.146f, 0.000f);
					fillpatch(i++, "(default)", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.146f, 0.000f);
					fillpatch(i++, "(default)", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 0.50f, 0.500f, 0.146f, 0.000f);
				}

				for(uint32_t i=0; i<NVOICES; ++i)
				{
					voices[i] = new mdaEPianoVoice(rate);
					voices[i]->setParams(programs[curProgram].param);
					add_voices(voices[i]);
				}

				add_audio_outputs(p_left, p_right);
				setProgram(0);
			}

		unsigned find_free_voice(unsigned char key, unsigned char velocity) {
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

		void handle_midi(uint32_t size, unsigned char* data) {
			//discard invalid midi messages
			if (size < 2)
				return;

			//receive on all channels
			switch(data[0] & 0xf0)
			{
				case 0x80: //note off
					{
						//discard invalid midi messages
						if (size != 3)
							return;

						for (unsigned i = 0; i < NVOICES; ++i) {
							if (voices[i]->get_key() == data[1]) {
								voices[i]->off(data[2]);
								break;
							}
						}
					}
					break;

				case 0x90: //note on
					{
						//discard invalid midi messages
						if (size != 3)
							return;

						unsigned int v = find_free_voice(data[1], data[2]);
						if (v < NVOICES)
							voices[v]->on(data[1], data[2]);
					}
					break;

					//controller
				case 0xB0:
					switch(data[1])
					{
						case 0x01: //mod wheel
							//discard invalid midi messages
							if (size != 3)
								return;

							//scale the mod value to cover the range [0..0.9906]
							modwhl = 0.0078f * (float)(data[2]);
							if(modwhl > 0.05f) //over-ride pan/trem depth
							{
								for (unsigned i = 0; i < NVOICES; ++i) {
									//set lfo depth
									voices[i]->set_lmod(modwhl);
									if(programs[curProgram].param[4] < 0.5f)
										voices[i]->set_rmod(-modwhl);
									else
										voices[i]->set_rmod(-modwhl);
								}
							}
							break;

							//volume
						case 0x07:
							//discard invalid midi messages
							if (size != 3)
								return;

							setVolume(0.00002f * (float)(data[2] * data[2]));
							break;

							//sustain pedal
						case 0x40:
							//sostenuto pedal
						case 0x42:
							//discard invalid midi messages
							if (size != 3)
								return;

							{
								sustain = data[2] & 0x40;

								for (unsigned i = 0; i < NVOICES; ++i) {
									voices[i]->set_sustain(sustain);
									//if pedal was released: dampen sustained notes
									if((sustain==0) && (voices[i]->is_sustained()))
										voices[i]->off(0);
								}
							}
							break;

							//all notes off
						default:
							if(data[1]>0x7A) 
							{  
								for(short v=0; v<NVOICES; v++)
								{
									voices[v]->set_dec(0.99f);
									voices[v]->set_sustain(0);
									voices[v]->set_muff(160.0f);
								}
							}
							break;
					}
					break;

					//program change
				case 0xC0:
					if(data[1]<NPROGS) setProgram(data[1]);
					break;

				default: break;
			}
		}

		void setVolume(unsigned char value)
		{
			for (uint32_t v=0; v<NVOICES; ++v)
				voices[v]->set_volume(value);
		}

		void setProgram(uint32_t program)
		{
			curProgram = program;
			std::cout << "loading program " << programs[curProgram].name << std::endl;
			for (uint32_t v=0; v<NVOICES; ++v)
				voices[v]->setParams(programs[curProgram].param);

			update();
		}

		void update() //parameter change
		{
			for (uint32_t v=0; v<NVOICES; ++v)
				voices[v]->update();
		}

		void fillpatch(uint32_t p, const char *name, float p0, float p1,
				float p2, float p3,
				float p4, float p5,
				float p6, float p7,
				float p8, float p9,
				float p10, float p11)
		{
			//TODO: better use strncopy (first learn how)
			//strncpy(programs[p].name, name, 50);
			strcpy(programs[p].name, name);
			programs[p].param[0] = p0;
			programs[p].param[1] = p1;
			programs[p].param[2] = p2;
			programs[p].param[3] = p3;
			programs[p].param[4] = p4;
			programs[p].param[5] = p5;
			programs[p].param[6] = p6;
			programs[p].param[7] = p7;
			programs[p].param[8] = p8;
			programs[p].param[9] = p9;
			programs[p].param[10] = p10;
			programs[p].param[11] = p11;
		}
};


static int _ = mdaEPiano::register_class(p_uri);
