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

#include "termite_gui.h"

#include "value.h"
#include "termite_synth.h"

#include <math.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>

#define WIDTH 120
#define HEIGHT 44

#define LOGO_WIDTH 36
#define LOGO_Y 0
#define SPACE 3
#define TOTAL_COLUMNS 3
#define MAX_STATUS_SIZE 20
#define MAX_SAVE_SIZE 40
#define SAVE_COLUMN 2
#define PATCH_BROWSER_ROWS 5
#define PATCH_BROWSER_WIDTH 26

namespace laf {

  void TermiteGui::drawLogo() {
    attron(A_BOLD);
    attron(COLOR_PAIR(LOGO_COLOR));
    int logo_x = (WIDTH - LOGO_WIDTH) / 2 + 1;

    move(LOGO_Y, logo_x);
    printw("   __                        __");
    move(LOGO_Y + 1, logo_x);
    printw("  / /____  _________ ___ (--) /____");
    move(LOGO_Y + 2, logo_x);
    printw(" / __/ _ \\/ ___/ __ `__ \\/ / __/ _ \\");
    move(LOGO_Y + 3, logo_x);
    printw("/ /_/  __/ /  / / / / / / / /_/  __/");
    move(LOGO_Y + 4, logo_x);
    printw("\\__/\\___/_/  /_/ /_/ /_/_/\\__/\\___/");

    attroff(A_BOLD);
    attroff(COLOR_PAIR(LOGO_COLOR));

    move(LOGO_Y + 5, logo_x);
    printw("                          Little IO");
  }

  void TermiteGui::drawMidi(std::string status) {
    move(2, 2);
    printw("MIDI Learn: ");
    attron(A_BOLD);
    hline(' ', MAX_STATUS_SIZE);
    printw(status.substr(0, MAX_STATUS_SIZE).c_str());
    attroff(A_BOLD);
    refresh();
  }

  void TermiteGui::drawStatus(std::string status) {
    move(1, 2);
    printw("Current Value: ");
    attron(A_BOLD);
    hline(' ', MAX_STATUS_SIZE);
    printw(status.substr(0, MAX_STATUS_SIZE).c_str());
    attroff(A_BOLD);
    refresh();
  }

  void TermiteGui::clearPatches() {
    int selection_row = (PATCH_BROWSER_ROWS - 1) / 2;
    move(1 + selection_row, 82);
    hline(' ', PATCH_BROWSER_WIDTH);
    for (int i = 0; i < PATCH_BROWSER_ROWS; ++i) {
      move(1 + i, 94);
      hline(' ', PATCH_BROWSER_WIDTH);
    }
  }

  void TermiteGui::drawPatchSaving(std::string patch_name) {
    int selection_row = (PATCH_BROWSER_ROWS - 1) / 2;
    move(1 + selection_row, 82);
    printw("            ");
    hline(' ', PATCH_BROWSER_WIDTH);
    move(1 + selection_row, 82);
    printw("Save Patch: ");
    printw(patch_name.c_str());
  }

  void TermiteGui::drawPatchLoading(std::vector<std::string> patches,
                                    int selected_index) {
    int selection_row = (PATCH_BROWSER_ROWS - 1) / 2;
    move(1 + selection_row, 82);
    printw("Load Patch:");

    int patch_index = selected_index - selection_row;
    int num_patches = patches.size();
    for (int i = 0; i < PATCH_BROWSER_ROWS; ++i) {
      if (i % 2)
        attroff(COLOR_PAIR(PATCH_LOAD_COLOR));
      else
        attron(COLOR_PAIR(PATCH_LOAD_COLOR));

      move(1 + i, 94);
      hline(' ', PATCH_BROWSER_WIDTH);
      if (patch_index == selected_index)
        attron(A_BOLD);

      if (patch_index >= 0 && patch_index < num_patches)
        printw(patches[patch_index].c_str());
      attroff(A_BOLD);
      patch_index++;
    }
    attroff(COLOR_PAIR(PATCH_LOAD_COLOR));
    refresh();
  }

  void TermiteGui::drawSlider(const DisplayDetails* slider,
                              float percentage, bool active) {
    // Clear slider.
    char slider_char = active ? '=' : ' ';
    move(slider->y + 1, slider->x);
    attron(COLOR_PAIR(SLIDER_BG_COLOR));
    hline(slider_char, slider->width);
    attroff(COLOR_PAIR(SLIDER_BG_COLOR));

    // Find slider position.
    int position = round(slider->width * percentage);
    int slider_midpoint = (slider->width + 1) / 2;

    attron(COLOR_PAIR(SLIDER_FG_COLOR));
    if (slider->bipolar) {
      if (position < slider_midpoint) {
        move(slider->y + 1, slider->x + position);
        hline(' ', slider_midpoint - position);
      }
      else {
        move(slider->y + 1, slider->x + slider_midpoint);
        hline(' ', position - slider_midpoint);
      }
    }
    else {
      hline(' ', position);
    }

    attroff(COLOR_PAIR(SLIDER_FG_COLOR));
    refresh();
  }

  void TermiteGui::drawText(const DisplayDetails* details,
                            std::string text, bool active) {
    // Clear area.
    move(details->y + 1, details->x);
    attron(COLOR_PAIR(CONTROL_TEXT_COLOR));
    hline(' ', details->width);

    // Draw text.
    move(details->y + 1, details->x);
    if (active)
      attron(A_BOLD);
    printw(text.c_str());
    attroff(A_BOLD);
    attroff(COLOR_PAIR(CONTROL_TEXT_COLOR));
  }

  void TermiteGui::drawControl(const Control* control, bool active) {
    DisplayDetails* details = details_lookup_[control];
    if (!details)
      return;

    // Draw label.
    if (active)
      attron(A_BOLD);
    move(details->y, details->x);
    printw(details->label.c_str());
    attroff(A_BOLD);

    // Draw status.
    if (control->display_strings()) {
      int display_index = static_cast<int>(control->current_value());
      drawText(details, control->display_strings()[display_index], active);
    }
    else
      drawSlider(details, control->getPercentage(), active);
  }

  void TermiteGui::drawControlStatus(const Control* control, bool midi_armed) {
    std::ostringstream midi_learn;
    if (midi_armed)
      midi_learn << "ARMED";
    else if (control->midi_learn())
      midi_learn << control->midi_learn();
    else
      midi_learn << "-";
    drawMidi(midi_learn.str());

    std::ostringstream status;
    if (control->display_strings()) {
      int display_index = static_cast<int>(control->current_value());
      status << control->display_strings()[display_index];
    }
    else
      status << control->current_value();
    drawStatus(status.str());
  }

  void TermiteGui::start() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    if(has_colors() == FALSE) {
      endwin();
      printf("Your terminal does not support color\n");
      return;
    }

    start_color();
    init_pair(SLIDER_FG_COLOR, COLOR_WHITE, COLOR_YELLOW);
    init_pair(SLIDER_BG_COLOR, COLOR_YELLOW, COLOR_WHITE);
    init_pair(LOGO_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(PATCH_LOAD_COLOR, COLOR_BLACK, COLOR_CYAN);
    init_pair(CONTROL_TEXT_COLOR, COLOR_BLACK, COLOR_WHITE);

    refresh();
    drawLogo();
    curs_set(0);
  }

  void TermiteGui::stop() {
    endwin();
  }

  void TermiteGui::placeControl(std::string name, const Control* control,
      int x, int y, int width) {
    DisplayDetails* details = new DisplayDetails();
    details->x = x;
    details->y = y;
    details->width = width;
    details->label = name;
    details->bipolar = control->isBipolar();

    details_lookup_[control] = details;
    drawControl(control, false);
    control_order_.push_back(name);
  }

  void TermiteGui::addControls(const control_map& controls) {
    // Oscillators.
    placeControl("osc 1 waveform", controls.at("osc 1 waveform"),
        2, 7, 18);
    placeControl("osc 2 waveform", controls.at("osc 2 waveform"),
        22, 7, 18);
    placeControl("osc 2 transpose", controls.at("osc 2 transpose"),
        2, 10, 38);
    placeControl("osc 2 tune", controls.at("osc 2 tune"),
        2, 13, 38);

    // LFOs.
    placeControl("lfo 1 waveform", controls.at("lfo 1 waveform"),
        2, 16, 18);
    placeControl("lfo 1 frequency", controls.at("lfo 1 frequency"),
        22, 16, 18);
    placeControl("lfo 2 waveform", controls.at("lfo 2 waveform"),
        2, 19, 18);
    placeControl("lfo 2 frequency", controls.at("lfo 2 frequency"),
        22, 19, 18);


    // Volume / Delay.
    placeControl("volume", controls.at("volume"),
        2, 25, 38);
    placeControl("delay time", controls.at("delay time"),
        2, 28, 38);
    placeControl("delay feedback", controls.at("delay feedback"),
        2, 31, 18);
    placeControl("delay dry/wet", controls.at("delay dry/wet"),
        22, 31, 18);

    // Filter.
    placeControl("filter type", controls.at("filter type"),
        42, 7, 38);
    placeControl("cutoff", controls.at("cutoff"),
        42, 10, 38);
    placeControl("resonance", controls.at("resonance"),
        42, 13, 38);
    placeControl("keytrack", controls.at("keytrack"),
        42, 16, 38);
    placeControl("fil env depth", controls.at("fil env depth"),
        42, 19, 38);
    placeControl("fil attack", controls.at("fil attack"),
        42, 22, 38);
    placeControl("fil decay", controls.at("fil decay"),
        42, 25, 38);
    placeControl("fil sustain", controls.at("fil sustain"),
        42, 28, 38);
    placeControl("fil release", controls.at("fil release"),
        42, 31, 38);

    // Performance.
    placeControl("polyphony", controls.at("polyphony"),
        82, 7, 30);
    placeControl("legato", controls.at("legato"),
        114, 7, 6);
    placeControl("portamento", controls.at("portamento"),
        82, 10, 21);
    placeControl("portamento type", controls.at("portamento type"),
        105, 10, 15);
    placeControl("pitch bend range", controls.at("pitch bend range"),
        82, 13, 38);

    // Amplitude Envelope.
    placeControl("amp attack", controls.at("amp attack"),
        82, 19, 38);
    placeControl("amp decay", controls.at("amp decay"),
        82, 22, 38);
    placeControl("amp sustain", controls.at("amp sustain"),
        82, 25, 38);
    placeControl("amp release", controls.at("amp release"),
        82, 28, 38);
    placeControl("velocity track", controls.at("velocity track"),
        82, 31, 38);
  }

  std::string TermiteGui::getCurrentControl() {
    return control_order_[control_index_];
  }

  std::string TermiteGui::getNextControl() {
    control_index_ = (control_index_ + 1) % control_order_.size();
    return getCurrentControl();
  }

  std::string TermiteGui::getPrevControl() {
    control_index_ = (control_index_ + control_order_.size() - 1) %
      control_order_.size();
    return getCurrentControl();
  }
} // namespace laf
