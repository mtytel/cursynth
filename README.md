termite v0.1
------------
termite is a MIDI enabled, subtractive synth that runs in your terminal with an ascii interface.
This is a first iteration so the features are minimal. termite will connect to all MIDI devices and send output to the default audio
device on your system.

![alt tag](http://littleio.co/static/img/termite_screen_shot.png)

### Building
run 'make' and it will compile a 'termite' executable which you can run.

### Controls
* awsedftgyhujkolp;' - a playable keyboard (no key up events)
* up/down - previous/next control
* left/right - decrement/increment control
* m - arm midi learn
* c - erase midi learn

### Requirements:
* OS: Mac OSX or Linux.
* Terminal: a color enabled terminal with minimum 120x44 ascii characters
* If you want key up events, you have to use a MIDI keyboard.

### TODO:
* Info/controls page.
* Modulation matrix.
* Routable LFOs.
* Routable Envelopes.
* Velocity mapping.
* Patch saving loading (with JSON).
* On startup select audio output device.
* On startup select MIDI input device.
* Better GUI text elements.
* Lot's more..

Questions? Bugs? email littleioaudio@gmail.com

Copyright 2013 Little IO <littleioaudio@gmail.com>
