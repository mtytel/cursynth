    Copyright 2013 Little IO <littleioaudio@gmail.com>

    termite is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    termite is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with termite.  If not, see <http://www.gnu.org/licenses/>.

termite v0.1

About: termite is a MIDI enabled, subtractive synth that runs in your terminal
with an ascii interface. This is a first iteration so the features are minimal.

termite will connect to all MIDI devices and send output to the default audio
device on your system.

Building:
  run 'make' and it will compile a 'termite' executable which you can run.

Controls:
  awsedftgyhujkolp;' - a playable keyboard (no key up events)
  up/down - previous/next control
  left/right - decrement/increment control
  m - arm midi learn
  c - erase midi learn

Requirements:
  Mac OSX or Linux.
  Terminal dimensions: minimum 120x44 ascii characters
  If you want key up events, you have to use a MIDI keyboard.

TODO:
  Info/controls page.
  Modulation matrix.
  Routable LFOs.
  Routable Envelopes.
  Velocity mapping.
  Patch saving loading (with JSON).
  On startup select audio output device.
  On startup select MIDI input device.
