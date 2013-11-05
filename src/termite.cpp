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
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <stdio.h>

#define KEYBOARD "awsedftgyhujkolp;'"
#define SLIDER "`1234567890"
#define EXTENSION ".mite"
#define PATCH_DIRECTORY "patches"
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
    termite->processAudio((laf::laf_float*)out_buffer, n_frames);
    return 0;
  }
} // namespace

namespace laf {
  Termite::Termite() : state_(STANDARD), pitch_bend_(0), patch_load_index_(0) {
    pthread_mutex_init(&mutex_, 0);
  }

  void Termite::start() {
    setupAudio();
    setupMidi();
    setupGui();

    // Wait for input.
    while(textInput(getch()))
      ;

    stop();
  }

  bool Termite::textInput(int key) {
    if (state_ == PATCH_LOADING) {
      int num_patches = patches_.size();
      switch(key) {
        case '\n':
          state_ = STANDARD;
          gui_.clearPatches();
          return true;
        case KEY_UP:
          patch_load_index_ = CLAMP(patch_load_index_ - 1, 0, num_patches - 1);
          gui_.drawPatchLoading(patches_, patch_load_index_);
          loadFromFile(patches_[patch_load_index_]);
          return true;
        case KEY_DOWN:
          patch_load_index_ = CLAMP(patch_load_index_ + 1, 0, num_patches - 1);
          gui_.drawPatchLoading(patches_, patch_load_index_);
          loadFromFile(patches_[patch_load_index_]);
          return true;
      }
    }

    std::string current_control = gui_.getCurrentControl();
    Control* control = controls_.at(current_control);
    switch(key) {
      case 'H':
      case KEY_F(1):
        startHelp();
        break;
      case 'S':
        startSave();
        break;
      case 'L':
        startLoad();
        break;
      case 'M':
      case 'm':
        if (state_ != MIDI_LEARN)
          state_ = MIDI_LEARN;
        else
          state_ = STANDARD;
        break;
      case 'C':
      case 'c':
        eraseMidiLearn(control);
        state_ = STANDARD;
        break;
      case KEY_UP:
        current_control = gui_.getPrevControl();
        state_ = STANDARD;
        lock();
        gui_.drawControl(control, false);
        unlock();
        break;
      case KEY_DOWN:
        current_control = gui_.getNextControl();
        state_ = STANDARD;
        lock();
        gui_.drawControl(control, false);
        unlock();
        break;
      case KEY_RIGHT:
        lock();
        control->increment();
        unlock();
        break;
      case KEY_LEFT:
        lock();
        control->decrement();
        unlock();
        break;
      default:
        size_t slider_size = strlen(SLIDER) - 1;
        for (size_t i = 0; i <= slider_size; ++i) {
          if (SLIDER[i] == key) {
            control->setPercentage((1.0 * i) / slider_size);
            break;
          }
        }
        for (size_t i = 0; i < strlen(KEYBOARD); ++i) {
          if (KEYBOARD[i] == key) {
            synth_.noteOn(48 + i);
            return true;
          }
        }
    }

    lock();
    control = controls_.at(current_control);
    gui_.drawControl(control, true);
    gui_.drawControlStatus(control, state_ == MIDI_LEARN);
    unlock();

    return true;
  }

  void Termite::setupAudio() {
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

    controls_ = synth_.getControls();
    gui_.addControls(controls_);

    Control* control = controls_.at(gui_.getCurrentControl());
    gui_.drawControl(control, true);
    gui_.drawControlStatus(control, false);
  }

  void Termite::processAudio(laf_float *out_buffer, unsigned int n_frames) {
    lock();
    synth_.process();
    unlock();
    const laf_float* buffer = synth_.output()->buffer;
    for (size_t i = 0; i < n_frames; ++i) {
      for (int c = 0; c < NUM_CHANNELS; ++c)
        out_buffer[NUM_CHANNELS * i + c] = buffer[i];
    }
  }

  void Termite::eraseMidiLearn(Control* control) {
    if (control->midi_learn()) {
      midi_learn_.erase(control->midi_learn());
      control->midi_learn(0);
    }
  }

  void Termite::setupMidi() {
    RtMidiIn* midi_in = new RtMidiIn();
    if (midi_in->getPortCount() <= 0) {
      std::cout << "No midi devices found.\n";
    }
    for (unsigned int i = 0; i < midi_in->getPortCount(); ++i) {
      RtMidiIn* device = new RtMidiIn();
      device->openPort(i);
      device->setCallback(&midiCallback, (void*)this);
      midi_ins_.push_back(device);
    }

    delete midi_in;
  }

  void Termite::processMidi(std::vector<unsigned char>* message) {
    if (message->size() < 3)
      return;

    lock();
    int midi_port = message->at(0);
    int midi_id = message->at(1);
    int midi_val = message->at(2);
    Control* selected_control = controls_.at(gui_.getCurrentControl());
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
      pitch_bend_->set((2.0 * midi_val) / (MIDI_SIZE - 1) - 1);
      gui_.drawControl(pitch_bend_, selected_control == pitch_bend_);
    }
    else if (midi_port == SUSTAIN_PORT && midi_id == SUSTAIN_ID) {
      if (midi_val)
        synth_.sustainOn();
      else
        synth_.sustainOff();
    }
    else if (state_ == MIDI_LEARN && midi_port < 254) {
      eraseMidiLearn(selected_control);

      midi_learn_[midi_id] = selected_control;
      selected_control->midi_learn(midi_id);
      state_ = STANDARD;
      gui_.drawControlStatus(selected_control, false);
    }

    if (midi_learn_.find(midi_id) != midi_learn_.end()) {
      Control* midi_control = midi_learn_[midi_id];
      midi_control->setMidi(midi_val);
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

  // Help.

  void Termite::startHelp() {
    gui_.drawHelp();
    getch();

    gui_.drawMain();
    std::string current = gui_.getCurrentControl();
    std::string name = gui_.getNextControl();
    while (name != current) {
      gui_.drawControl(controls_.at(name), false);
      name = gui_.getNextControl();
    }
    gui_.drawControl(controls_.at(current), true);
  }

  // Loading and Saving.

  void Termite::startSave() {
    gui_.clearPatches();
    curs_set(1);
    std::stringstream save_stream;
    gui_.drawPatchSaving(save_stream.str());
    int key = 0;
    while(1) {
      key = getch();
      std::string current = save_stream.str();
      switch(key) {
        case KEY_DC:
        case KEY_BACKSPACE:
        case 127:
          save_stream.str(current.substr(0, current.length() - 1));
          save_stream.seekp(save_stream.str().length());
          gui_.drawPatchSaving(save_stream.str());
          break;
        case '\n':
          if (save_stream.str().length() > 0)
            saveToFile(save_stream.str() + EXTENSION);
          state_ = STANDARD;
          gui_.clearPatches();
          curs_set(0);
          return;
        default:
          if (isprint(key))
            save_stream << static_cast<char>(key);
          gui_.drawPatchSaving(save_stream.str());
          break;
      }
    }
  }

  void Termite::startLoad() {
    DIR *dir;
    struct dirent *ent;
    patches_.clear();
    if ((dir = opendir(PATCH_DIRECTORY)) != NULL) {
      while ((ent = readdir(dir)) != NULL) {
        std::string name = ent->d_name;
        if (name.find(EXTENSION) != std::string::npos)
          patches_.push_back(name);
      }
      closedir (dir);
    }

    if (patches_.size() == 0)
      return;

    state_ = PATCH_LOADING;
    patch_load_index_ = 0;
    loadFromFile(patches_[patch_load_index_]);
  }

  void Termite::saveToFile(const std::string& file_name) {
    std::ofstream save_file;
    std::string path = PATCH_DIRECTORY;
    path = path + "/" + file_name;
    save_file.open(path.c_str());
    save_file << writeStateToString();
    save_file.close();
  }

  void Termite::loadFromFile(const std::string& file_name) {
    std::string path = PATCH_DIRECTORY;
    path += "/";
    path += file_name;
    std::ifstream load_file;
    load_file.open(path.c_str());
    if (!load_file.is_open())
      return;

    load_file.seekg(0, std::ios::end);
    int length = load_file.tellg();
    load_file.seekg(0, std::ios::beg);
    char file_contents[length];
    load_file.read(file_contents, length);
    readStateFromString(file_contents);
    load_file.close();

    gui_.drawPatchLoading(patches_, patch_load_index_);
  }

  std::string Termite::writeStateToString() {
    cJSON* root = cJSON_CreateObject();

    control_map::iterator iter = controls_.begin();
    for(; iter != controls_.end(); ++iter) {
      cJSON* value = cJSON_CreateNumber(iter->second->value()->value());
      cJSON_AddItemToObject(root, iter->first.c_str(), value);
    }

    char* json = cJSON_Print(root);
    std::string output = json;
    free(json);
    return output;
  }

  void Termite::readStateFromString(const std::string& state) {
    cJSON* root = cJSON_Parse(state.c_str());

    lock();
    control_map::iterator iter = controls_.begin();
    for(; iter != controls_.end(); ++iter) {
      cJSON* value = cJSON_GetObjectItem(root, iter->first.c_str());
      Control* control = iter->second;
      control->set(value->valuedouble);
      gui_.drawControl(control, false);
    }

    Control* current_control = controls_.at(gui_.getCurrentControl());
    gui_.drawControl(current_control, true);
    gui_.drawControlStatus(current_control, false);
    unlock();

    cJSON_Delete(root);
  }
} // namespace laf
