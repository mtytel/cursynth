/* Copyright 2013-2015 Matt Tytel
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
      // Computer keyboard reading states.
      enum InputState {
        STANDARD,
        MIDI_LEARN,
        PATCH_LOADING,
        PATCH_SAVING,
      };

      Cursynth();

      // Start/stop everything - UI, synth engine, input/output.
      void start(unsigned sample_rate, unsigned buffer_size);
      void stop();

      // Runs the synth engine for _n_frames_ samples and copies the output
      // to _out_buffer_.
      void processAudio(mopo_float *out_buffer, unsigned int n_frames);

      // Processes MIDI data like note and velocity, and knob data.
      void processMidi(std::vector<unsigned char>* message);

      // If we're working with the synth or UI, lock so we don't break
      // something. Then unlock when we're done.
      void lock() { pthread_mutex_lock(&mutex_); }
      void unlock() { pthread_mutex_unlock(&mutex_); }

    private:
      unsigned int chooseSampleRate(const RtAudio::DeviceInfo& device,
                                    unsigned preferred_sample_rate);

      // Load and save global configuration settings (like MIDI learn).
      void loadConfiguration();
      void saveConfiguration();

      // Writes the synth state to a string so we can save it to a patch.
      std::string writeStateToString();
      // Read the synth state from a string.
      void readStateFromString(const std::string& state);

      // Saves the state to a given filename.
      void saveToFile(const std::string& file_name);
      void loadFromFile(const std::string& file_name);

      // When the user enters help state. Show controls and contact.
      void startHelp();

      // When user starts to load, launch patch browser.
      void startLoad();

      // When user starts to save, start save text field.
      void startSave();

      // Computer keyboard text input callback.
      bool textInput(int key);

      // Setup functions initialize objects and callbacks.
      void setupAudio(unsigned sample_rate, unsigned buffer_size);
      void setupMidi();
      void setupControls();
      void setupGui();

      // Clear screen and redraw GUI
      void refreshGui();

      // Helper function to erase all evidence of MIDI learn for a control.
      void eraseMidiLearn(Control* control);

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
