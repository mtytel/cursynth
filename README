cursynth - a GNU program
------------
cursynth is a polyphonic, MIDI enabled, subtractive synth that runs in your
terminal with an ascii interface.

cursynth will connect to all MIDI devices and send output to the default audio
device on your system.

### Building
If you just want to build Cursynth, check out [ftp://ftp.gnu.org/gnu/cursynth]
for latest source releases that don't require as much to build.

If you want to develop Cursynth, this source is the right place.
Run these commands to build the project from source:

$ autoreconf -i
$ ./configure
$ make

And then if you want to install, next run:

$ sudo make install

### Usage
cursynth [--buffer-size OR -b preferred-buffer-size]
         [--sample-rate OR -s preferred-sample-rate]
         [--version OR -V]

### Controls
* awsedftgyhujkolp;' - a playable keyboard (no key up events)
* \`1234567890 - a slider for the current selected control
* up/down - previous/next control
* left/right - decrement/increment control
* F1 (or [shift] + H) - help/controls
* [shift] + L - browse/load patches
* [shift] + S - save patch
* m - arm midi learn
* c - erase midi learn

### Requirements:
* OS: Mac OSX or GNU/Linux
* Terminal: a color enabled terminal with minimum 120x44 ascii characters
* If you want key up events, you have to use a MIDI keyboard

### TODO:
* Make a LV2 plugin version of cursynth
* More modulation sources and destinations
* Routable Envelopes
* On startup select audio output device
* On startup select MIDI input device
* Lot's more...

Questions? Feature requests? Bugs? email matthewtytel@gmail.com

Copyright 2013-2015 Matt Tytel <matthewtytel@gmail.com>
