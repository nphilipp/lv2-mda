#!/bin/bash
#jack_connect mdaEPiano:midi alsa_pcm:MIDIMATE-II/midi_capture_1

jack_connect mdaEPiano:midi "a2j:Midi Through [14] (capture): Midi Through Port-0"
jack_connect system:playback_1 mdaEPiano:left                  
jack_connect system:playback_2 mdaEPiano:right
