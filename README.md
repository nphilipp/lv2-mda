# Native LV2 port of the famous mdaEPiano VSTi

Port
  Author: rekado
  Based on: `mda-vst-src-2010-02-14.zip`

mdaEPiano
  Author: Paul Kellett (paul.kellett@mda-vst.com)
          http://sourceforge.net/projects/mda-vst/

[![button](http://api.flattr.com/button/flattr-badge-large.png "Flattr this")](http://flattr.com/thing/72898/LV2-port-of-the-mda-E-Piano-VSTi)
----------------------

## About

This is an attempt to port the mdaEPiano VSTi plugin to the LV2 plugin
platform. So far it seems to work quite well. It responds to common MIDI
events, generates sounds and sounds very much like the original---that is
because most of the sound-generating code has been taken from Paul Kellett's
source code. I'd like to thank Paul Kellett for his decision to put the
plugin's source code under a free license (see LICENSE for more info).


## Compiling

To compile the plugin, just execute the following statement:

    make && sudo make install

This will copy the compiled plugin to `/usr/local/lib/lv2`.

**NOTE**: you will need larsl's `lv2-c++-tools` to compile this plugin: http://freecode.com/projects/lv2-c-tools


## Connecting

At the current stage you will need an LV2 host to use the plugin. I'm using
`lv2_jack_host` during development:

    lv2_jack_host http://elephly.net/lv2/mdaEPiano

You should now be able to connect your MIDI device to the plugin via JACK. Also
don't forget to connect the plugin's stereo output to your system's outputs:

    jack_connect system:playback_1 mdaEPiano:left
    jack_connect system:playback_2 mdaEPiano:right

At least this is what I do on my system. I provide a simple connect script with
the code which does these things automatically.


## Known Issues

None. Only missing features:

  - proper note stealing as in the original
  - GUI (I don't need it)
  - saving and importing of presets (should be handled by the host)


## License

The original mda VST plug-ins by Paul Kellett are released under the MIT
license or under the GPL "either version 2 of the License, or (at your option)
any later version". The LV2 port of the mdaEPiano plugin by rekado is released
under the same licenses.
