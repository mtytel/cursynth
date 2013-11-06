termite v0.1
------------
termite is a polyphonic, MIDI enabled, subtractive synth that runs in your terminal with an ascii interface.
This is a first iteration so the features are minimal. termite will connect to all MIDI devices and send output to the default audio
device on your system.

![alt tag](http://littleio.co/static/img/termite_screen_shot.png)

### Building
First check if you have the laf submodule
```
git submodule init
git submodule update
```
Then run 'make' and it will compile a 'termite' executable which you can run.

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
* OS: Mac OSX or Linux.
* Terminal: a color enabled terminal with minimum 120x44 ascii characters
* If you want key up events, you have to use a MIDI keyboard.

### TODO:
* Modulation matrix.
* Routable Envelopes.
* On startup select audio output device.
* On startup select MIDI input device.
* Lot's more..

Questions? Bugs? email littleioaudio@gmail.com

Copyright 2013 Little IO <littleioaudio@gmail.com>
