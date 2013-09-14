/* Copyright 2013 Little IO
 *
 * termite is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * termite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with termite.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef TERMITE_H
#define TERMITE_H

#include "RtAudio.h"
#include "RtMidi.h"
#include "termite_gui.h"
#include "termite_patch_handler.h"
#include "termite_synth.h"

#include <pthread.h>
#include <map>
#include <vector>

namespace laf {

  class Termite {
    public:
      enum State {
        STANDARD,
        MIDI_LEARN,
        SAVING,
        LOADING,
      };

      Termite();

      void start();
      void processAudio(laf_float *out_buffer, unsigned int n_frames);
      void processMidi(std::vector<unsigned char>* message);

      void lock() { pthread_mutex_lock(&mutex_); }
      void unlock() { pthread_mutex_unlock(&mutex_); }

    private:
      void loadTextInput(int key);
      void saveTextInput(int key);
      bool textInput(int key);
      void setupAudio();
      void eraseMidiLearn(Control* control);

      std::string writeStateToString();
      void readStateFromString(const std::string& state);
      void startLoad();

      void setupMidi();
      void setupControls();
      void setupGui();
      void stop();

      TermiteSynth synth_;
      TermiteGui gui_;
      TermitePatchHandler patch_handler_;

      RtAudio dac_;
      State state_;
      std::vector<RtMidiIn*> midi_ins_;
      pthread_mutex_t mutex_;

      Control* pitch_bend_;
  };
} // namespace laf

#endif // TERMITE_H
