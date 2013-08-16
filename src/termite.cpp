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

#include "termite.h"

#include "cJSON.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <ncurses.h>

#define KEYBOARD "AWSEDFTGYHUJKOLP;'"
#define NUM_CHANNELS 2
#define PITCH_BEND_PORT 224
#define SUSTAIN_PORT 176
#define SUSTAIN_ID 64

namespace {

  void midiCallback(double delta_time, std::vector<unsigned char>* message,
                    void* user_data) {
    UNUSED(delta_time);
    laf::Termite* termite = static_cast<laf::Termite*>(user_data);
    termite->processMidi(message);
  }

  int audioCallback(void *out_buffer, void *in_buffer,
                    unsigned int n_frames, double stream_time,
                    RtAudioStreamStatus status, void *user_data) {
    UNUSED(in_buffer);
    UNUSED(stream_time);
    if (status)
      std::cout << "Stream underflow detected!" << std::endl;

    laf::Termite* termite = static_cast<laf::Termite*>(user_data);
    termite->processAudio((laf::laf_sample*)out_buffer, n_frames);
    return 0;
  }
} // namespace

namespace laf {
  Termite::Termite() :
      active_group_(0),  midi_learn_armed_(false), pitch_bend_(0) {
    pthread_mutex_init(&mutex_, 0);
  }

  void Termite::start() {
    setupAudio();
    setupMidi();
    setupGui();
    load();

    // Wait for input.
    while(textInput(getch()))
      ;

    stop();
  }

  bool Termite::textInput(int key) {
    if (key == KEY_F(1))
      return false;

    Control* control = control_iter_->second;
    switch(key) {
      case 'm':
        lock();
        midi_learn_armed_ = !midi_learn_armed_;
        unlock();
        break;
      case 'l':
        load();
        break;
      case 's':
        save();
        break;
      case 'c':
        lock();
        eraseMidiLearn(control);
        midi_learn_armed_ = false;
        unlock();
        break;
      case KEY_UP:
        if (control_iter_ == groups_[active_group_]->controls.begin()) {
          active_group_ = (groups_.size() + active_group_ - 1) %
                          groups_.size();
          control_iter_ = groups_[active_group_]->controls.end();
        }
        control_iter_--;
        midi_learn_armed_ = false;
        gui_.drawControl(control, false);
        break;
      case KEY_DOWN:
        control_iter_++;
        if (control_iter_ == groups_[active_group_]->controls.end()) {
          active_group_ = (active_group_ + 1) % groups_.size();
          control_iter_ = groups_[active_group_]->controls.begin();
        }
        midi_learn_armed_ = false;
        gui_.drawControl(control, false);
        break;
      case KEY_RIGHT:
        control->current_value +=
            (control->max - control->min) / control->resolution;
        break;
      case KEY_LEFT:
        control->current_value -=
            (control->max - control->min) / control->resolution;
        break;
      default:
        for (size_t i = 0; i < strlen(KEYBOARD); ++i) {
          if (KEYBOARD[i] == key) {
            lock();
            synth_.noteOn(48 + i);
            unlock();
          }
        }
    }
    control = control_iter_->second;
    control->current_value =
      CLAMP(control->min, control->max, control->current_value);

    control->value->set(control->current_value);
    gui_.drawControl(control, true);
    gui_.drawControlStatus(control, midi_learn_armed_);

    return true;
  }

  void Termite::load() {
    std::ifstream load_file;
    load_file.open("save_file.mite");
    if (!load_file.is_open())
      return;

    load_file.seekg(0, std::ios::end);
    int length = load_file.tellg();
    load_file.seekg(0, std::ios::beg);
    char file_contents[length];
    load_file.read(file_contents, length);
    load_file.close();

    cJSON* root = cJSON_Parse(file_contents);
    cJSON* sections = cJSON_GetObjectItem(root, "sections");

    lock();
    for (unsigned int i = 0; i < groups_.size(); ++i) {
      std::map<std::string, Control*>::iterator iter =
          groups_[i]->controls.begin();
      cJSON* section = cJSON_GetArrayItem(sections, i);
      for(; iter != groups_[i]->controls.end(); ++iter) {
        cJSON* value = cJSON_GetObjectItem(section, iter->first.c_str());
        Control* control = iter->second;
        control->current_value =
            CLAMP(control->min, control->max, value->valuedouble);
        control->value->set(value->valuedouble);
        gui_.drawControl(control, false);
      }
    }
    gui_.drawControl(control_iter_->second, true);
    unlock();

    cJSON_Delete(root);
  }

  void Termite::save() {
    cJSON* root = cJSON_CreateObject();
    cJSON* sections = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "sections", sections);

    for (unsigned int i = 0; i < groups_.size(); ++i) {
      std::map<std::string, Control*>::iterator iter =
          groups_[i]->controls.begin();
      cJSON* section = cJSON_CreateObject();
      for(; iter != groups_[i]->controls.end(); ++iter) {
        cJSON* value = cJSON_CreateNumber(iter->second->value->value());
        cJSON_AddItemToObject(section, iter->first.c_str(), value);
      }
      cJSON_AddItemToArray(sections, section);
    }

    char* json = cJSON_Print(root);
    std::ofstream save_file;
    save_file.open("save_file.mite");
    save_file << json;
    save_file.close();
    free(json);
  }

  void Termite::setupAudio() {
    // Setup Audio
    if (dac_.getDeviceCount() < 1) {
      std::cout << "No audio devices found.\n";
      exit(0);
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac_.getDefaultOutputDevice();
    parameters.nChannels = NUM_CHANNELS;
    parameters.firstChannel = 0;
    unsigned int sample_rate = 44100;
    unsigned int buffer_frames = 64;

    synth_.setSampleRate(sample_rate);

    try {
      dac_.openStream(&parameters, NULL, RTAUDIO_FLOAT64, sample_rate,
                      &buffer_frames, &audioCallback, (void*)this);
      dac_.startStream();
    }
    catch (RtError& error) {
      error.printMessage();
      exit(0);
    }
  }

  void Termite::setupGui() {
    gui_.start();

    // Global Section.
    ControlGroup* controls = synth_.getControls();
    gui_.addPerformanceControls(controls);

    ControlGroup* voice_handler = controls->sub_groups["voice handler"];

    ControlGroup* oscillators = voice_handler->sub_groups["oscillators"];
    pitch_bend_ = oscillators->controls["pitch bend amount"];
    gui_.addOscillatorControls(oscillators);

    ControlGroup* filter = voice_handler->sub_groups["filter"];
    gui_.addFilterControls(filter);

    ControlGroup* amplifier = voice_handler->sub_groups["amplifier"];
    gui_.addAmplifierControls(amplifier);

    groups_.push_back(oscillators);
    groups_.push_back(controls);
    groups_.push_back(filter);
    groups_.push_back(amplifier);

    active_group_ = 0;
    control_iter_ = groups_[active_group_]->controls.begin();

    gui_.drawControl(control_iter_->second, true);
    gui_.drawControlStatus(control_iter_->second, false);
  }

  void Termite::processAudio(laf_sample *out_buffer, unsigned int n_frames) {
    lock();
    synth_.process();
    unlock();
    const laf_sample* buffer = synth_.output()->buffer;
    for (size_t i = 0; i < n_frames; ++i) {
      for (int c = 0; c < NUM_CHANNELS; ++c)
        out_buffer[NUM_CHANNELS * i + c] = buffer[i];
    }
  }

  void Termite::eraseMidiLearn(Control* control) {
    if (control->midi_learn) {
      midi_learn_.erase(control->midi_learn);
      control->midi_learn = 0;
    }
  }

  void Termite::setupMidi() {
    // Setup MIDI
    if (midi_in_.getPortCount() < 1) {
      std::cout << "No midi devices found.\n";
    }
    else {
      midi_in_.openPort(0);
      midi_in_.setCallback(&midiCallback, (void*)this);
    }
  }

  void Termite::processMidi(std::vector<unsigned char>* message) {
    if (message->size() < 3)
      return;

    lock();
    int midi_port = message->at(0);
    int midi_id = message->at(1);
    int midi_val = message->at(2);
    Control* selected_control = control_iter_->second;
    if (midi_port >= 144 && midi_port < 160) {
      int midi_note = midi_id;
      int midi_velocity = midi_val;

      if (midi_velocity)
        synth_.noteOn(midi_note, (1.0 * midi_velocity) / MIDI_SIZE);
      else
        synth_.noteOff(midi_note);
    }
    if (midi_port >= 128 && midi_port < 144) {
      int midi_note = midi_id;
      synth_.noteOff(midi_note);
    }
    else if (midi_port == PITCH_BEND_PORT) {
      pitch_bend_->value->set((2.0 * midi_val) / (MIDI_SIZE - 1) - 1);
      gui_.drawControl(pitch_bend_, selected_control == pitch_bend_);
    }
    else if (midi_port == SUSTAIN_PORT && midi_id == SUSTAIN_ID) {
      if (midi_val)
        synth_.sustainOn();
      else
        synth_.sustainOff();
    }
    else if (midi_learn_armed_ && midi_port < 254) {
      eraseMidiLearn(selected_control);

      midi_learn_[midi_id] = selected_control;
      selected_control->midi_learn = midi_id;
      midi_learn_armed_ = false;
      gui_.drawControlStatus(selected_control, false);
    }

    if (midi_learn_.find(midi_id) != midi_learn_.end()) {
      Control* midi_control = midi_learn_[midi_id];
      laf_sample resolution = midi_control->resolution;
      int index = resolution * midi_val / (MIDI_SIZE - 1);
      midi_control->current_value = midi_control->min +
          index * (midi_control->max - midi_control->min) / resolution;
      midi_control->value->set(midi_control->current_value);
      gui_.drawControl(midi_control, selected_control == midi_control);
      gui_.drawControlStatus(midi_control, false);
    }
    unlock();
  }

  void Termite::stop() {
    pthread_mutex_destroy(&mutex_);
    gui_.stop();
    try {
      dac_.stopStream();
    }
    catch (RtError& error) {
      error.printMessage();
    }

    if (dac_.isStreamOpen())
      dac_.closeStream();
  }
} // namespace laf
