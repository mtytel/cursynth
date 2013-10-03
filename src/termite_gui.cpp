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

namespace laf {

  void TermiteGui::drawLogo() {
    attron(A_BOLD);
    attron(COLOR_PAIR(LOGO_COLOR));
    int logo_x = (WIDTH - LOGO_WIDTH) / 2;

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

  void TermiteGui::drawControl(const Control* control, bool active) {
    Slider* slider = slider_lookup_[control];
    char slider_char = active ? '=' : ' ';
    if (active)
      attron(A_BOLD);
    move(slider->y, slider->x);
    printw(slider->label.c_str());
    attroff(A_BOLD);

    // Clear slider.
    move(slider->y + 1, slider->x);
    attron(COLOR_PAIR(SLIDER_BG_COLOR));
    hline(slider_char, slider->width);
    attroff(COLOR_PAIR(SLIDER_BG_COLOR));

    // Find slider position.
    laf_float relative_value = control->current_value - control->min;
    laf_float relative_width = control->max - control->min;
    int position = round(slider->width * relative_value / relative_width);
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

  void TermiteGui::drawControlStatus(const Control* control, bool midi_armed) {
    std::ostringstream midi_learn;
    if (midi_armed)
      midi_learn << "ARMED";
    else if (control->midi_learn)
      midi_learn << control->midi_learn;
    else
      midi_learn << "-";
    drawMidi(midi_learn.str());

    std::ostringstream status;
    if (control->display_strings) {
      int display_index = static_cast<int>(control->current_value);
      status << control->display_strings[display_index];
    }
    else
      status << control->current_value;
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

    drawLogo();

    curs_set(0);
  }

  void TermiteGui::stop() {
    endwin();
  }

  void TermiteGui::placeSlider(std::string name, const Control* control,
                               int x, int y, int width) {
    Slider* slider = new Slider();
    slider->x = x;
    slider->y = y;
    slider->width = width;
    slider->label = name;
    slider->bipolar = control->min < 0;

    slider_lookup_[control] = slider;
    drawControl(control, false);
    control_order_.push_back(name);
  }

  void TermiteGui::addControls(const control_map& controls) {
    // Oscillators.
    placeSlider("osc 1 waveform", controls.at("osc 1 waveform"),
                2, 7, 18);
    placeSlider("osc 2 waveform", controls.at("osc 2 waveform"),
                22, 7, 18);
    placeSlider("osc 2 transpose", controls.at("osc 2 transpose"),
                2, 10, 38);

    // Volume / Delay.
    placeSlider("volume", controls.at("volume"),
                2, 22, 38);
    placeSlider("delay time", controls.at("delay time"),
                2, 25, 38);
    placeSlider("delay feedback", controls.at("delay feedback"),
                2, 28, 18);
    placeSlider("delay wet/dry", controls.at("delay wet/dry"),
                22, 28, 18);

    // Filter.
    placeSlider("cutoff", controls.at("cutoff"),
                42, 7, 38);
    placeSlider("resonance", controls.at("resonance"),
                42, 10, 38);
    placeSlider("keytrack", controls.at("keytrack"),
                42, 13, 38);
    placeSlider("fil env depth", controls.at("fil env depth"),
                42, 16, 38);
    placeSlider("fil attack", controls.at("fil attack"),
                42, 19, 38);
    placeSlider("fil decay", controls.at("fil decay"),
                42, 22, 38);
    placeSlider("fil sustain", controls.at("fil sustain"),
                42, 25, 38);
    placeSlider("fil release", controls.at("fil release"),
                42, 28, 38);

    // Performance.
    placeSlider("polyphony", controls.at("polyphony"),
                82, 7, 30);
    placeSlider("legato", controls.at("legato"),
                114, 7, 6);
    placeSlider("portamento", controls.at("portamento"),
                82, 10, 21);
    placeSlider("portamento type", controls.at("portamento type"),
                105, 10, 15);
    placeSlider("pitch bend range", controls.at("pitch bend range"),
                82, 13, 38);

    // Amplitude Envelope.
    placeSlider("amp attack", controls.at("amp attack"),
                82, 19, 38);
    placeSlider("amp decay", controls.at("amp decay"),
                82, 22, 38);
    placeSlider("amp sustain", controls.at("amp sustain"),
                82, 25, 38);
    placeSlider("amp release", controls.at("amp release"),
                82, 28, 38);
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
