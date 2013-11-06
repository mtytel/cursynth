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
#include "termite_synth.h"

#include <pthread.h>

namespace laf {

  class Termite {
    public:
      enum InputState {
        STANDARD,
        MIDI_LEARN,
        PATCH_LOADING,
        PATCH_SAVING,
      };

      Termite();

      void start();
      void processAudio(laf_float *out_buffer, unsigned int n_frames);
      void processMidi(std::vector<unsigned char>* message);

      void lock() { pthread_mutex_lock(&mutex_); }
      void unlock() { pthread_mutex_unlock(&mutex_); }

    private:
      std::string writeStateToString();
      void loadConfiguration();
      void saveConfiguration();
      void readStateFromString(const std::string& state);
      void saveToFile(const std::string& file_name);
      void loadFromFile(const std::string& file_name);
      void startHelp();
      void startLoad();
      void startSave();

      bool textInput(int key);
      void setupAudio();
      void eraseMidiLearn(Control* control);

      void setupMidi();
      void setupControls();
      void setupGui();
      void stop();

      // Termite parts.
      TermiteSynth synth_;
      TermiteGui gui_;

      // IO.
      RtAudio dac_;
      std::vector<RtMidiIn*> midi_ins_;
      std::map<int, std::string> midi_learn_;

      // State.
      InputState state_;
      control_map controls_;
      pthread_mutex_t mutex_;
      Control* pitch_bend_;

      // Loading and Saving.
      std::vector<std::string> patches_;
      int patch_load_index_;
  };
} // namespace laf

#endif // TERMITE_H
