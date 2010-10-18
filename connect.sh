#!/bin/bash

# connect MIDI mate II to MIDI through (Alsa MIDI)
aconnect 28:0 14:0

# connect to JACK
jack_connect mdaEPiano:midi "a2j:Midi Through [14] (capture): Midi Through Port-0"

jack_connect system:playback_1 mdaEPiano:left
jack_connect system:playback_2 mdaEPiano:right
