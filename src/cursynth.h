/* Copyright 2013 Little IO
 *
 * cursynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cursynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cursynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef CURSYNTH_H
#define CURSYNTH_H

#include "RtAudio.h"
#include "RtMidi.h"
#include "cursynth_gui.h"
#include "cursynth_engine.h"

#include <pthread.h>

namespace mopo {

  class Cursynth {
    public:
      enum InputState {
        STANDARD,
        MIDI_LEARN,
        PATCH_LOADING,
        PATCH_SAVING,
      };

      Cursynth();

      void start();
      void processAudio(mopo_float *out_buffer, unsigned int n_frames);
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

      // Cursynth parts.
      CursynthEngine synth_;
      CursynthGui gui_;

      // IO.
      RtAudio dac_;
      std::vector<RtMidiIn*> midi_ins_;
      std::map<int, std::string> midi_learn_;

      // State.
      InputState state_;
      control_map controls_;
      pthread_mutex_t mutex_;

      // Loading and Saving.
      std::vector<std::string> patches_;
      int patch_load_index_;
  };
} // namespace mopo

#endif // CURSYNTH_H
