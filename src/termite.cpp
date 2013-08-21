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

#define KEYBOARD "AWSEDFTGYHUJKOLP;'"
#define EXTENSION ".mite"
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
  Termite::Termite() :
      active_group_(0), midi_learn_armed_(false), saving_(false),
      loading_(false), patch_index_(0), pitch_bend_(0) {
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

  void Termite::loadTextInput(int key) {
    switch(key) {
      case '\n':
        loading_ = false;
        gui_.clearLoad();
        break;
      case KEY_UP:
        loadPrev();
        break;
      case KEY_DOWN:
        loadNext();
        break;
    }
  }

  void Termite::saveTextInput(int key) {
    std::string current = saveAsStream.str();
    switch(key) {
      case '\n':
        saveToFile();
        saving_ = false;
        gui_.clearSave();
        break;
      case KEY_DC:
      case KEY_BACKSPACE:
      case 127:
        saveAsStream.str(current.substr(0, current.length() - 1));
        saveAsStream.seekp(saveAsStream.str().length());
        gui_.drawSave(saveAsStream.str());
        break;
      default:
        if (isprint(key))
          saveAsStream << static_cast<char>(key);
        gui_.drawSave(saveAsStream.str());
        break;
    }
  }

  bool Termite::textInput(int key) {
    if (key == KEY_F(1))
      return false;
    if (loading_) {
      loadTextInput(key);
      return true;
    }
    if (saving_) {
      saveTextInput(key);
      return true;
    }

    Control* control = control_iter_->second;
    switch(key) {
      case 'm':
        lock();
        midi_learn_armed_ = !midi_learn_armed_;
        unlock();
        break;
      case 'l':
        startLoad();
        break;
      case 's':
        saving_ = true;
        gui_.drawSave("");
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

  void Termite::loadNext() {
    patch_index_ = (patch_index_ + 1) % patches_.size();
    loadFromFile(patches_[patch_index_]);
  }

  void Termite::loadPrev() {
    patch_index_ = (patches_.size() + patch_index_ - 1) % patches_.size();
    loadFromFile(patches_[patch_index_]);
  }

  void Termite::saveToFile() {
    std::ofstream save_file;
    save_file.open(saveAsStream.str().c_str());
    saveAsStream.str("");
    save_file << writeStateToString();
    save_file.close();
  }

  std::string Termite::writeStateToString() {
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
    std::string output = json;
    free(json);
    return output;
  }

  void Termite::readStateFromString(const std::string& state) {
    cJSON* root = cJSON_Parse(state.c_str());
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
    gui_.drawControlStatus(control_iter_->second, false);
    unlock();

    cJSON_Delete(root);
  }

  void Termite::loadFromFile(const std::string& file_name) {
    std::ifstream load_file;
    load_file.open(file_name.c_str());
    if (!load_file.is_open())
      return;

    load_file.seekg(0, std::ios::end);
    int length = load_file.tellg();
    load_file.seekg(0, std::ios::beg);
    char file_contents[length];
    load_file.read(file_contents, length);
    readStateFromString(file_contents);
    load_file.close();

    gui_.drawLoad(file_name);
  }

  void Termite::startLoad() {
    DIR *dir;
    struct dirent *ent;
    patches_.clear();
    if ((dir = opendir (".")) != NULL) {
      while ((ent = readdir (dir)) != NULL) {
        std::string name = ent->d_name;
        if (name.find(EXTENSION) != std::string::npos)
          patches_.push_back(name);
      }
      closedir (dir);
    }

    if (patches_.size() == 0)
      return;

    loading_ = true;
    patch_index_ = 0;
    loadFromFile(patches_[patch_index_]);
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

    ControlGroup* articulation = voice_handler->sub_groups["articulation"];
    gui_.addArticulationControls(articulation);

    groups_.push_back(oscillators);
    groups_.push_back(controls);
    groups_.push_back(filter);
    groups_.push_back(articulation);

    active_group_ = 0;
    control_iter_ = groups_[active_group_]->controls.begin();

    gui_.drawControl(control_iter_->second, true);
    gui_.drawControlStatus(control_iter_->second, false);
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
    if (control->midi_learn) {
      midi_learn_.erase(control->midi_learn);
      control->midi_learn = 0;
    }
  }

  void Termite::setupMidi() {
    // Setup MIDI
    RtMidiIn* midi_in = new RtMidiIn();
    if (midi_in->getPortCount() < 1) {
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
      laf_float resolution = midi_control->resolution;
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
