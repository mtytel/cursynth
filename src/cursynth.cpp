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

#include "cursynth.h"

#include "cJSON.h"

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <sys/stat.h>

#define KEYBOARD "awsedftgyhujkolp;'"
#define SLIDER "`1234567890"
#define EXTENSION ".mite"
#define CONFIG_DIR ".cursynth/"
#define USER_PATCHES_DIR "patches/"
#define CONFIG_FILE ".cursynth_conf"
#define NUM_CHANNELS 2
#define MOD_WHEEL_ID 1
#define PITCH_BEND_PORT 224
#define SUSTAIN_PORT 176
#define SUSTAIN_ID 64

namespace {

  // Receive MIDI data and send it to the synth.
  void midiCallback(double delta_time, std::vector<unsigned char>* message,
                    void* user_data) {
    UNUSED(delta_time);
    mopo::Cursynth* cursynth = static_cast<mopo::Cursynth*>(user_data);
    cursynth->processMidi(message);
  }

  // Receive audio buffers and send them to the synth.
  int audioCallback(void *out_buffer, void *in_buffer,
                    unsigned int n_frames, double stream_time,
                    RtAudioStreamStatus status, void *user_data) {
    UNUSED(in_buffer);
    UNUSED(stream_time);
    if (status)
      std::cout << "Stream underflow detected!" << std::endl;

    mopo::Cursynth* cursynth = static_cast<mopo::Cursynth*>(user_data);
    cursynth->processAudio((mopo::mopo_float*)out_buffer, n_frames);
    return 0;
  }

  // Return the directory containing the configuration file and patches.
  std::string getConfigPath() {
    std::stringstream config_path;
    config_path << getenv("HOME") << "/" << CONFIG_DIR;
    return config_path.str();
  }

  // Return the location of the configuration file.
  std::string getConfigFile() {
    std::stringstream config_path;
    config_path << getConfigPath() << CONFIG_FILE;
    return config_path.str();
  }

  // Return the location of the user patches.
  std::string getUserPatchesPath() {
    std::stringstream patches_path;
    patches_path << getConfigPath() << USER_PATCHES_DIR;
    return patches_path.str();
  }

  // Check if the directory _path_ exists, if not, create it.
  void confirmPathExists(std::string path) {
    if (opendir(path.c_str()) == NULL)
      mkdir(path.c_str(), 0755);
  }

  // Returns all files in directory _dir_ with extenstion _ext_.
  std::vector<std::string> getAllFiles(std::string dir, std::string ext) {
    std::vector<std::string> file_names;
    DIR *directory = NULL;
    struct dirent *ent = NULL;

    if ((directory = opendir(dir.c_str())) != NULL) {
      while ((ent = readdir(directory)) != NULL) {
        std::string name = ent->d_name;
        if (name.find(ext) != std::string::npos)
          file_names.push_back(name);
      }
      closedir(directory);
    }

    return file_names;
  }
} // namespace

namespace mopo {
  Cursynth::Cursynth() : state_(STANDARD), patch_load_index_(0) {
    pthread_mutex_init(&mutex_, 0);
  }

  void Cursynth::start(unsigned sample_rate, unsigned buffer_size) {
    // Setup all callbacks.
    setupAudio(sample_rate, buffer_size);
    setupMidi();
    setupGui();
    loadConfiguration();

    // Wait for computer keyboard input.
    while(textInput(getch()))
      ;

    stop();
  }

  void Cursynth::loadConfiguration() {
    // Try to open the configuration file.
    std::ifstream config_file;
    config_file.open(getConfigFile().c_str());
    if (!config_file.is_open())
      return;

    // Read file contents into buffer.
    config_file.seekg(0, std::ios::end);
    int length = config_file.tellg();
    config_file.seekg(0, std::ios::beg);
    char file_contents[length];
    config_file.read(file_contents, length);

    // Parse the JSON configuration file.
    cJSON* root = cJSON_Parse(file_contents);

    // For all controls, try to load the MIDI learn assignment.
    std::string current = gui_.getCurrentControl();
    std::string name = current;
    do {
      cJSON* value = cJSON_GetObjectItem(root, name.c_str());
      if (value)
        midi_learn_[value->valueint] = name;

      name = gui_.getNextControl();
    } while(name != current);

    // Delete the parsing data.
    cJSON_Delete(root);
  }

  void Cursynth::saveConfiguration() {
    confirmPathExists(getConfigPath());

    // Store all the MIDI learn data into JSON.
    cJSON* root = cJSON_CreateObject();
    std::map<int, std::string>::iterator iter = midi_learn_.begin();
    for (; iter != midi_learn_.end(); ++iter) {
      cJSON* midi = cJSON_CreateNumber(iter->first);
      cJSON_AddItemToObject(root, iter->second.c_str(), midi);
    }

    // Write the configuration JSON to the configuration file.
    char* json = cJSON_Print(root);
    std::ofstream save_file;
    save_file.open(getConfigFile().c_str());
    MOPO_ASSERT(save_file.is_open());
    save_file << json;
    save_file.close();

    free(json);
    cJSON_Delete(root);
  }

  bool Cursynth::textInput(int key) {
    if (state_ == PATCH_LOADING) {
      int num_patches = patches_.size();
      switch(key) {
        case '\n':
          // Finish loading patches.
          state_ = STANDARD;
          gui_.clearPatches();
          return true;
        case KEY_UP:
          // Go to previous patch.
          patch_load_index_ = CLAMP(patch_load_index_ - 1, 0, num_patches - 1);
          gui_.drawPatchLoading(patches_, patch_load_index_);
          loadFromFile(patches_[patch_load_index_]);
          return true;
        case KEY_DOWN:
          // Go to next patch.
          patch_load_index_ = CLAMP(patch_load_index_ + 1, 0, num_patches - 1);
          gui_.drawPatchLoading(patches_, patch_load_index_);
          loadFromFile(patches_[patch_load_index_]);
          return true;
      }
    }

    std::string current_control = gui_.getCurrentControl();
    Control* control = controls_.at(current_control);
    bool should_redraw_control = false;
    lock();
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
        should_redraw_control = true;
        break;
      case 'C':
      case 'c':
        eraseMidiLearn(control);
        state_ = STANDARD;
        should_redraw_control = true;
        break;
      case KEY_UP:
        current_control = gui_.getPrevControl();
        state_ = STANDARD;
        gui_.drawControl(control, false);
        should_redraw_control = true;
        break;
      case KEY_DOWN:
        current_control = gui_.getNextControl();
        state_ = STANDARD;
        gui_.drawControl(control, false);
        should_redraw_control = true;
        break;
      case KEY_RIGHT:
        control->increment();
        should_redraw_control = true;
        break;
      case KEY_LEFT:
        control->decrement();
        should_redraw_control = true;
        break;
      case KEY_RESIZE:
        refreshGui();
        break;
      default:
        // Check if they pressed the slider keys and change the current value.
        size_t slider_size = strlen(SLIDER) - 1;
        for (size_t i = 0; i <= slider_size; ++i) {
          if (SLIDER[i] == key) {
            control->setPercentage((1.0 * i) / slider_size);
            should_redraw_control = true;
            break;
          }
        }

        // Check if they pressed note keys and play the corresponding note.
        for (size_t i = 0; i < strlen(KEYBOARD); ++i) {
          if (KEYBOARD[i] == key) {
            synth_.noteOn(48 + i);
            break;
          }
        }
    }

    if (should_redraw_control) {
      control = controls_.at(current_control);
      gui_.drawControl(control, true);
      gui_.drawControlStatus(control, state_ == MIDI_LEARN);
    }

    unlock();
    return true;
  }

  unsigned int Cursynth::chooseSampleRate(const RtAudio::DeviceInfo& device,
                                          unsigned preferred_sample_rate) {
    for (int i = 0; i < device.sampleRates.size(); ++i) {
      if (device.sampleRates[i] == preferred_sample_rate)
        return preferred_sample_rate;
    }

    return device.sampleRates[0];
  }

  void Cursynth::setupAudio(unsigned sample_rate, unsigned buffer_size) {
    // Make sure we have a device to make sound with.
    if (dac_.getDeviceCount() < 1) {
      std::cout << "No audio devices found.\n";
      exit(0);
    }

    // Setup audio preferences.
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac_.getDefaultOutputDevice();
    parameters.nChannels = NUM_CHANNELS;
    parameters.firstChannel = 0;
    RtAudio::DeviceInfo device_info = dac_.getDeviceInfo(parameters.deviceId);

    unsigned actual_sample_rate = chooseSampleRate(device_info, sample_rate);
    synth_.setSampleRate(actual_sample_rate);
    buffer_size = CLAMP(buffer_size, 0, mopo::MAX_BUFFER_SIZE);

    // Start the audio callbacks.
    try {
      dac_.openStream(&parameters, NULL, RTAUDIO_FLOAT64, actual_sample_rate,
                      &buffer_size, &audioCallback, (void*)this);
      dac_.startStream();
    }
    catch (RtError& error) {
      error.printMessage();
      exit(0);
    }

    synth_.setBufferSize(buffer_size);
  }

  void Cursynth::setupGui() {
    gui_.start();

    // Add the controls to the GUI for viewing.
    controls_ = synth_.getControls();
    gui_.addControls(controls_);

    // Make sure we are drawing he current control.
    Control* control = controls_.at(gui_.getCurrentControl());
    gui_.drawControl(control, true);
    gui_.drawControlStatus(control, false);
  }

  void Cursynth::refreshGui() {
    gui_.redrawBase();
    controls_ = synth_.getControls();
    gui_.addControls(controls_);

    // Make sure we are drawing the current control.
    Control* control = controls_.at(gui_.getCurrentControl());
    gui_.drawControl(control, true);
    gui_.drawControlStatus(control, false);
  }

  void Cursynth::processAudio(mopo_float *out_buffer, unsigned int n_frames) {
    // Run the synth.
    lock();
    synth_.process();
    unlock();

    // Copy the synth output to the output buffer.
    const mopo_float* buffer = synth_.output()->buffer;
    for (size_t i = 0; i < n_frames; ++i) {
      for (int c = 0; c < NUM_CHANNELS; ++c)
        out_buffer[NUM_CHANNELS * i + c] = buffer[i];
    }
  }

  void Cursynth::eraseMidiLearn(Control* control) {
    if (control->midi_learn()) {
      midi_learn_.erase(control->midi_learn());
      control->midi_learn(0);
    }
  }

  void Cursynth::setupMidi() {
    RtMidiIn* midi_in = new RtMidiIn();
    if (midi_in->getPortCount() <= 0) {
      std::cout << "No midi devices found.\n";
    }

    // Setup MIDI callbacks for every MIDI device.
    // TODO: Have a menu for only enabling some MIDI devices.
    for (unsigned int i = 0; i < midi_in->getPortCount(); ++i) {
      RtMidiIn* device = new RtMidiIn();
      device->openPort(i);
      device->setCallback(&midiCallback, (void*)this);
      midi_ins_.push_back(device);
    }

    delete midi_in;
  }

  void Cursynth::processMidi(std::vector<unsigned char>* message) {
    if (message->size() < 3)
      return;

    lock();
    int midi_port = message->at(0);
    int midi_id = message->at(1);
    int midi_val = message->at(2);
    std::string selected_control_name = gui_.getCurrentControl();
    Control* selected_control = controls_.at(selected_control_name);
    if (midi_port >= 144 && midi_port < 160) {
      // A MIDI keyboard key was pressed. Play a note.
      int midi_note = midi_id;
      int midi_velocity = midi_val;

      if (midi_velocity)
        synth_.noteOn(midi_note, (1.0 * midi_velocity) / MIDI_SIZE);
      else
        synth_.noteOff(midi_note);
    }
    else if (midi_port >= 128 && midi_port < 144) {
      // A MIDI keyboard key was released. Release that note.
      int midi_note = midi_id;
      synth_.noteOff(midi_note);
    }
    else if (midi_port == PITCH_BEND_PORT)
      synth_.setPitchWheel((2.0 * midi_val) / (MIDI_SIZE - 1) - 1);
    else if (midi_port == SUSTAIN_PORT && midi_id == SUSTAIN_ID) {
      if (midi_val)
        synth_.sustainOn();
      else
        synth_.sustainOff();
    }
    else if (midi_port < 254) {
      // Must have gotten MIDI from some knob or other control.
      if (state_ == MIDI_LEARN && midi_port < 254) {
        // MIDI learn is armed so map this MIDI signal to the current control.
        eraseMidiLearn(selected_control);

        midi_learn_[midi_id] = selected_control_name;
        selected_control->midi_learn(midi_id);
        state_ = STANDARD;
        gui_.drawControlStatus(selected_control, false);
        saveConfiguration();
      }
      else if (midi_learn_.find(midi_id) != midi_learn_.end()) {
        // MIDI learn is enabled for this control. Change the paired control.
        Control* midi_control = controls_.at(midi_learn_[midi_id]);
        midi_control->setMidi(midi_val);
        gui_.drawControl(midi_control, selected_control == midi_control);
        gui_.drawControlStatus(midi_control, false);
      }

      if (midi_id == MOD_WHEEL_ID)
        synth_.setModWheel(midi_val);
    }
    unlock();
  }

  void Cursynth::stop() {
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

  void Cursynth::startHelp() {
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

  void Cursynth::startSave() {
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
          // Pressed delete, so remove the last character from the file name.
          save_stream.str(current.substr(0, current.length() - 1));
          save_stream.seekp(save_stream.str().length());
          gui_.drawPatchSaving(save_stream.str());
          break;
        case '\n':
          // Pressed enter, so save to the current file name.
          // Then go back to the normal input state.
          if (save_stream.str().length() > 0)
            saveToFile(save_stream.str() + EXTENSION);
          state_ = STANDARD;
          gui_.clearPatches();
          curs_set(0);
          return;
        default:
          // If it's a printable character, append it to the file name.
          if (isprint(key))
            save_stream << static_cast<char>(key);
          gui_.drawPatchSaving(save_stream.str());
          break;
      }
    }
  }

  void Cursynth::startLoad() {
    // Store all patche names from system and user patch directories.
    patches_ = getAllFiles(PATCHES_DIRECTORY, EXTENSION);
    std::vector<std::string> user_patches =
        getAllFiles(getUserPatchesPath(), EXTENSION);
    patches_.insert(patches_.end(), user_patches.begin(), user_patches.end());

    if (patches_.size() == 0)
      return;

    // Start patch loading by loading last patch browsed.
    state_ = PATCH_LOADING;
    patch_load_index_ = std::min<int>(patch_load_index_, patches_.size() - 1);
    loadFromFile(patches_[patch_load_index_]);
  }

  void Cursynth::saveToFile(const std::string& file_name) {
    confirmPathExists(getConfigPath());
    confirmPathExists(getUserPatchesPath());
    std::ofstream save_file;
    std::string path = getUserPatchesPath();
    path = path + "/" + file_name;
    save_file.open(path.c_str());
    save_file << writeStateToString();
    save_file.close();
  }

  void Cursynth::loadFromFile(const std::string& file_name) {
    std::ifstream load_file;

    // First try to load the patch from user patches.
    std::string path = getUserPatchesPath();
    path += "/";
    path += file_name;
    load_file.open(path.c_str());

    // If we can't find the patch try default system patches.
    if (!load_file.is_open()) {
      path = PATCHES_DIRECTORY;
      path += "/";
      path += file_name;
      load_file.open(path.c_str());
    }

    if (!load_file.is_open())
      return;

    // Read file into buffer and load it.
    load_file.seekg(0, std::ios::end);
    int length = load_file.tellg();
    load_file.seekg(0, std::ios::beg);
    char file_contents[length];
    load_file.read(file_contents, length);
    readStateFromString(file_contents);
    load_file.close();

    // Draw the patch loading happen.
    gui_.drawPatchLoading(patches_, patch_load_index_);
  }

  std::string Cursynth::writeStateToString() {
    // Write all controls to JSON.
    cJSON* root = cJSON_CreateObject();
    control_map::iterator iter = controls_.begin();
    for (; iter != controls_.end(); ++iter) {
      cJSON* value = cJSON_CreateNumber(iter->second->value()->value());
      cJSON_AddItemToObject(root, iter->first.c_str(), value);
    }

    // Write the JSON to a string and free the JSON.
    char* json = cJSON_Print(root);
    std::string output = json;
    free(json);
    cJSON_Delete(root);

    return output;
  }

  void Cursynth::readStateFromString(const std::string& state) {
    // Parse state into JSON and read all controls.
    cJSON* root = cJSON_Parse(state.c_str());
    control_map::iterator iter = controls_.begin();
    for (; iter != controls_.end(); ++iter) {
      cJSON* value = cJSON_GetObjectItem(root, iter->first.c_str());

      if (value) {
        Control* control = iter->second;
        control->set(value->valuedouble);
        gui_.drawControl(control, false);
      }
    }

    // Setup current control.
    Control* current_control = controls_.at(gui_.getCurrentControl());
    gui_.drawControl(current_control, true);
    gui_.drawControlStatus(current_control, false);

    cJSON_Delete(root);
  }
} // namespace mopo
