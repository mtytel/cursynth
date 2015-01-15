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

#include "cursynth_gui.h"

#include "value.h"

#include <cmath>
#include <cstdlib>
#include <libintl.h>
#include <locale.h>
#include <ncurses.h>
#include <sstream>
#include <unistd.h>

#define gettext_noop(String) String

#define WIDTH 120
#define HEIGHT 44

#define LOGO_WIDTH 44
#define LOGO_Y 0
#define SPACE 3
#define TOTAL_COLUMNS 3
#define MAX_STATUS_SIZE 20
#define MAX_SAVE_SIZE 40
#define SAVE_COLUMN 2
#define PATCH_BROWSER_ROWS 5
#define PATCH_BROWSER_WIDTH 26

namespace mopo {

  void CursynthGui::drawHelp() {
    erase();
    drawLogo();
    move(7, 41);
    attron(A_BOLD);
    printw(gettext("INFO"));
    attroff(A_BOLD);
    move(8, 43);
    printw(gettext("version"));
    printw(" - ");
    printw(VERSION);
    move(9, 43);
    printw(gettext("website"));
    printw(" - gnu.org/software/cursynth");
    move(10, 43);
    printw(gettext("contact"));
    printw(" - ");
    printw(PACKAGE_BUGREPORT);
    move(12, 41);
    attron(A_BOLD);
    printw(gettext("CONTROLS"));
    attroff(A_BOLD);
    move(14, 43);
    printw("awsedftgyhujkolp;' - ");
    printw(gettext("a playable keyboard"));
    move(16, 43);
    printw("`1234567890 - ");
    printw(gettext("a slider for the current selected control"));
    move(18, 43);
    printw(gettext("up/down"));
    printw(" - ");
    printw(gettext("previous/next control"));
    move(20, 43);
    printw(gettext("left/right"));
    printw(" - ");
    printw(gettext("decrement/increment control"));
    move(22, 43);
    printw(gettext("F1 (or [shift] + H)"));
    printw(" - ");
    printw(gettext("help/controls"));
    move(24, 43);
    printw(gettext("[shift] + L"));
    printw(" - ");
    printw(gettext("browse/load patches"));
    move(26, 43);
    printw(gettext("[shift] + S"));
    printw(" - ");
    printw(gettext("save patch"));
    move(28, 43);
    printw("m - ");
    printw(gettext("arm MIDI learn"));
    move(30, 43);
    printw("c - ");
    printw(gettext("erase MIDI learn"));
  }

  void CursynthGui::drawMain() {
    erase();
    drawLogo();
    drawModulationMatrix();
  }

  void CursynthGui::drawLogo() {
    attron(A_BOLD);
    attron(COLOR_PAIR(LOGO_COLOR));
    int logo_x = (WIDTH - LOGO_WIDTH) / 2 + 1;

    move(LOGO_Y, logo_x);
    printw("                                    __  __");
    move(LOGO_Y + 1, logo_x);
    printw("  _______  ______________  ______  / /_/ /_");
    move(LOGO_Y + 2, logo_x);
    printw(" / ___/ / / / ___/ ___/ / / / __ \\/ __/ __ \\");
    move(LOGO_Y + 3, logo_x);
    printw("/ /__/ /_/ / /  /__  / /_/ / / / / /_/ / / /");
    move(LOGO_Y + 4, logo_x);
    printw("\\___/\\____/_/  /____/\\__  /_/ /_/\\__/_/ /_/");
    move(LOGO_Y + 5, logo_x);
    printw("                    /____/");

    attroff(A_BOLD);
    attroff(COLOR_PAIR(LOGO_COLOR));

    move(LOGO_Y + 5, logo_x + 33);
    printw("Matt Tytel");
  }

  void CursynthGui::drawModulationMatrix() {
    move(34, 26);
    attron(A_BOLD);
    printw("----------------------------------------------------------------------");
    move(34, 53);
    printw(gettext("Modulation Matrix"));
    attroff(A_BOLD);

    move(35, 26);
    printw("                     ");
    move(35, 34);
    printw(gettext("source"));
    move(35, 50);
    printw("                     ");
    move(35, 58);
    printw(gettext("scale"));
    move(35, 74);
    printw("                     ");
    move(35, 79);
    printw(gettext("destination"));
  }

  void CursynthGui::drawMidi(std::string status) {
    move(2, 2);
    printw(gettext("MIDI Learn: "));
    attron(A_BOLD);
    hline(' ', MAX_STATUS_SIZE);
    printw(status.substr(0, MAX_STATUS_SIZE).c_str());
    attroff(A_BOLD);
    refresh();
  }

  void CursynthGui::drawStatus(std::string status) {
    move(1, 2);
    printw(gettext("Current Value: "));
    attron(A_BOLD);
    hline(' ', MAX_STATUS_SIZE);
    printw(gettext(status.substr(0, MAX_STATUS_SIZE).c_str()));
    attroff(A_BOLD);
    refresh();
  }

  void CursynthGui::clearPatches() {
    int selection_row = (PATCH_BROWSER_ROWS - 1) / 2;
    move(1 + selection_row, 83);
    hline(' ', PATCH_BROWSER_WIDTH);
    for (int i = 0; i < PATCH_BROWSER_ROWS; ++i) {
      move(1 + i, 94);
      hline(' ', PATCH_BROWSER_WIDTH);
    }
  }

  void CursynthGui::drawPatchSaving(std::string patch_name) {
    int selection_row = (PATCH_BROWSER_ROWS - 1) / 2;
    move(1 + selection_row, 83);
    printw("            ");
    hline(' ', PATCH_BROWSER_WIDTH);
    move(1 + selection_row, 83);
    printw(gettext("Save Patch: "));
    printw(patch_name.c_str());
  }

  void CursynthGui::drawPatchLoading(std::vector<std::string> patches,
                                     int selected_index) {
    int selection_row = (PATCH_BROWSER_ROWS - 1) / 2;
    move(1 + selection_row, 83);
    printw(gettext("Load Patch:"));

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

  void CursynthGui::drawSlider(const DisplayDetails* slider,
                               float percentage, bool active) {
    int y = slider->y;
    if (slider->label.size())
      y += 1;

    // Clear slider.
    move(y, slider->x - 1);
    attron(COLOR_PAIR(BG_COLOR));
    hline(' ', slider->width + 2);

    char slider_char = active ? '=' : ' ';
    move(y, slider->x);
    attron(COLOR_PAIR(SLIDER_BG_COLOR));
    hline(slider_char, slider->width);

    // If active draw a bit different.
    if (active) {
      move(y, slider->x - 1);
      attron(COLOR_PAIR(LOGO_COLOR));
      hline('|', 1);
      move(y, slider->x + slider->width);
      hline('|', 1);
    }

    // Find slider position.
    int position = round(slider->width * percentage);
    int slider_midpoint = (slider->width + 1) / 2;

    attron(COLOR_PAIR(SLIDER_FG_COLOR));
    if (slider->bipolar) {
      if (position < slider_midpoint) {
        move(y, slider->x + position);
        hline(' ', slider_midpoint - position);
      }
      else {
        move(y, slider->x + slider_midpoint);
        hline(' ', position - slider_midpoint);
      }
    }
    else {
      move(y, slider->x);
      hline(' ', position);
    }

    attroff(COLOR_PAIR(SLIDER_FG_COLOR));
    refresh();
  }

  void CursynthGui::drawText(const DisplayDetails* details,
                             std::string text, bool active) {
    int y = details->y;
    if (details->label.size())
      y += 1;

    // Clear area.
    move(y, details->x - 1);
    attron(COLOR_PAIR(BG_COLOR));
    hline(' ', details->width + 2);
    move(y, details->x);
    attron(COLOR_PAIR(CONTROL_TEXT_COLOR));
    hline(' ', details->width);

    // If active draw a bit different.
    if (active) {
      move(y, details->x - 1);
      attron(COLOR_PAIR(LOGO_COLOR));
      hline('|', 1);
      move(y, details->x + details->width);
      hline('|', 1);
      attron(A_BOLD);
    }

    // Draw text.
    attron(COLOR_PAIR(CONTROL_TEXT_COLOR));
    move(y, details->x);
    printw(gettext(text.c_str()));
    attroff(A_BOLD);
    attroff(COLOR_PAIR(CONTROL_TEXT_COLOR));
  }

  void CursynthGui::drawControl(const Control* control, bool active) {
    DisplayDetails* details = details_lookup_[control];
    if (!details)
      return;

    // Draw label.
    if (details->label.size()) {
      if (active)
        attron(A_BOLD);
      move(details->y, details->x);
      printw(gettext(details->label.c_str()));
      attroff(A_BOLD);
    }

    // Draw status.
    if (control->display_strings().size()) {
      int display_index = static_cast<int>(control->current_value());
      drawText(details, control->display_strings()[display_index], active);
    }
    else
      drawSlider(details, control->getPercentage(), active);
  }

  void CursynthGui::drawControlStatus(const Control* control,
                                      bool midi_armed) {
    std::ostringstream midi_learn;
    if (midi_armed)
      midi_learn << "ARMED";
    else if (control->midi_learn())
      midi_learn << control->midi_learn();
    else
      midi_learn << "-";
    drawMidi(midi_learn.str());

    std::ostringstream status;
    if (control->display_strings().size()) {
      int display_index = static_cast<int>(control->current_value());
      status << control->display_strings()[display_index];
    }
    else
      status << control->current_value();
    drawStatus(status.str());
  }

  void CursynthGui::start() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Make sure we have color.
    if(has_colors() == FALSE) {
      endwin();
      printf("Your terminal does not support color\n");
      return;
    }

    // Setup gettext for internationalization.
    setlocale(LC_ALL, "");
    bindtextdomain("cursynth", "/usr/share/locale");
    textdomain("cursynth");

    // Prepare all the color schemes.
    start_color();
    init_pair(BG_COLOR, COLOR_BLACK, COLOR_BLACK);
    init_pair(SLIDER_FG_COLOR, COLOR_WHITE, COLOR_YELLOW);
    init_pair(SLIDER_BG_COLOR, COLOR_YELLOW, COLOR_WHITE);
    init_pair(LOGO_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(PATCH_LOAD_COLOR, COLOR_BLACK, COLOR_CYAN);
    init_pair(CONTROL_TEXT_COLOR, COLOR_BLACK, COLOR_WHITE);

    // Start initial drawing.
    redrawBase();
  }

  void CursynthGui::stop() {
    endwin();
  }

  void CursynthGui::redrawBase() {
    erase();
    refresh();
    drawLogo();
    drawModulationMatrix();
    curs_set(0);
  }

  DisplayDetails* CursynthGui::initControl(std::string name,
                                           const Control* control) {
    std::map<const Control*, DisplayDetails*>::iterator found =
        details_lookup_.find(control);

    // If we've already created this control, just return that one.
    if (found != details_lookup_.end())
      return found->second;

    control_order_.push_back(name);
    return new DisplayDetails();
  }

  void CursynthGui::placeMinimalControl(std::string name,
                                        const Control* control,
                                        int x, int y, int width) {
    DisplayDetails* details = initControl(name, control);
    details->x = x;
    details->y = y;
    details->width = width;
    details->label = "";
    details->bipolar = control->isBipolar();

    details_lookup_[control] = details;
    drawControl(control, false);
  }

  void CursynthGui::placeControl(std::string name, const Control* control,
                                 int x, int y, int width) {
    DisplayDetails* details = initControl(name, control);
    details->x = x;
    details->y = y;
    details->width = width;
    details->label = name;
    details->bipolar = control->isBipolar();

    details_lookup_[control] = details;
    drawControl(control, false);
  }

  void CursynthGui::addControls(const control_map& controls) {
    // Oscillators.
    placeControl(gettext_noop("osc 1 waveform"),
                 controls.at("osc 1 waveform"),
                 2, 7, 18);
    placeControl(gettext_noop("osc 2 waveform"),
                 controls.at("osc 2 waveform"),
                 22, 7, 18);
    placeControl(gettext_noop("cross modulation"),
                 controls.at("cross modulation"),
                 2, 10, 18);
    placeControl(gettext_noop("osc mix"),
                 controls.at("osc mix"),
                 22, 10, 18);
    placeControl(gettext_noop("osc 2 transpose"),
                 controls.at("osc 2 transpose"),
                 2, 13, 18);
    placeControl(gettext_noop("osc 2 tune"),
                 controls.at("osc 2 tune"),
                 22, 13, 18);

    // LFOs.
    placeControl(gettext_noop("lfo 1 waveform"),
                 controls.at("lfo 1 waveform"),
                 2, 19, 18);
    placeControl(gettext_noop("lfo 1 frequency"),
                 controls.at("lfo 1 frequency"),
                 22, 19, 18);
    placeControl(gettext_noop("lfo 2 waveform"),
                 controls.at("lfo 2 waveform"),
                 2, 22, 18);
    placeControl(gettext_noop("lfo 2 frequency"),
                 controls.at("lfo 2 frequency"),
                 22, 22, 18);

    // Volume / Delay.
    placeControl(gettext_noop("volume"),
                 controls.at("volume"),
                 2, 25, 38);
    placeControl(gettext_noop("delay time"),
                 controls.at("delay time"),
                 2, 28, 38);
    placeControl(gettext_noop("delay feedback"),
                 controls.at("delay feedback"),
                 2, 31, 18);
    placeControl(gettext_noop("delay dry/wet"),
                 controls.at("delay dry/wet"),
                 22, 31, 18);

    // Filter.
    placeControl(gettext_noop("filter type"),
                 controls.at("filter type"),
                 42, 7, 38);
    placeControl(gettext_noop("cutoff"),
                 controls.at("cutoff"),
                 42, 10, 38);
    placeControl(gettext_noop("resonance"),
                 controls.at("resonance"),
                 42, 13, 38);
    placeControl(gettext_noop("keytrack"),
                 controls.at("keytrack"),
                 42, 16, 38);
    placeControl(gettext_noop("fil env depth"),
                 controls.at("fil env depth"),
                 42, 19, 38);
    placeControl(gettext_noop("fil attack"),
                 controls.at("fil attack"),
                 42, 22, 38);
    placeControl(gettext_noop("fil decay"),
                 controls.at("fil decay"),
                 42, 25, 38);
    placeControl(gettext_noop("fil sustain"),
                 controls.at("fil sustain"),
                 42, 28, 38);
    placeControl(gettext_noop("fil release"),
                 controls.at("fil release"),
                 42, 31, 38);

    // Performance.
    placeControl(gettext_noop("polyphony"),
                 controls.at("polyphony"),
                 82, 7, 30);
    placeControl(gettext_noop("legato"),
                 controls.at("legato"),
                 114, 7, 6);
    placeControl(gettext_noop("portamento"),
                 controls.at("portamento"),
                 82, 10, 21);
    placeControl(gettext_noop("portamento type"),
                 controls.at("portamento type"),
                 105, 10, 15);
    placeControl(gettext_noop("pitch bend range"),
                 controls.at("pitch bend range"),
                 82, 13, 38);

    // Amplitude Envelope.
    placeControl(gettext_noop("amp attack"),
                 controls.at("amp attack"),
                 82, 19, 38);
    placeControl(gettext_noop("amp decay"),
                 controls.at("amp decay"),
                 82, 22, 38);
    placeControl(gettext_noop("amp sustain"),
                 controls.at("amp sustain"),
                 82, 25, 38);
    placeControl(gettext_noop("amp release"),
                 controls.at("amp release"),
                 82, 28, 38);
    placeControl(gettext_noop("velocity track"),
                 controls.at("velocity track"),
                 82, 31, 38);

    // Modulation Matrix.
    placeMinimalControl(gettext_noop("mod source 1"),
                        controls.at("mod source 1"),
                        26, 36, 22);
    placeMinimalControl(gettext_noop("mod scale 1"),
                        controls.at("mod scale 1"),
                        50, 36, 22);
    placeMinimalControl(gettext_noop("mod destination 1"),
                        controls.at("mod destination 1"),
                        74, 36, 22);
    placeMinimalControl(gettext_noop("mod source 2"),
                        controls.at("mod source 2"),
                        26, 37, 22);
    placeMinimalControl(gettext_noop("mod scale 2"),
                        controls.at("mod scale 2"),
                        50, 37, 22);
    placeMinimalControl(gettext_noop("mod destination 2"),
                        controls.at("mod destination 2"),
                        74, 37, 22);
    placeMinimalControl(gettext_noop("mod source 3"),
                        controls.at("mod source 3"),
                        26, 38, 22);
    placeMinimalControl(gettext_noop("mod scale 3"),
                        controls.at("mod scale 3"),
                        50, 38, 22);
    placeMinimalControl(gettext_noop("mod destination 3"),
                        controls.at("mod destination 3"),
                        74, 38, 22);
    placeMinimalControl(gettext_noop("mod source 4"),
                        controls.at("mod source 4"),
                        26, 39, 22);
    placeMinimalControl(gettext_noop("mod scale 4"),
                        controls.at("mod scale 4"),
                        50, 39, 22);
    placeMinimalControl(gettext_noop("mod destination 4"),
                        controls.at("mod destination 4"),
                        74, 39, 22);
    placeMinimalControl(gettext_noop("mod source 5"),
                        controls.at("mod source 5"),
                        26, 40, 22);
    placeMinimalControl(gettext_noop("mod scale 5"),
                        controls.at("mod scale 5"),
                        50, 40, 22);
    placeMinimalControl(gettext_noop("mod destination 5"),
                        controls.at("mod destination 5"),
                        74, 40, 22);
  }

  std::string CursynthGui::getCurrentControl() {
    return control_order_[control_index_];
  }

  std::string CursynthGui::getNextControl() {
    control_index_ = (control_index_ + 1) % control_order_.size();
    return getCurrentControl();
  }

  std::string CursynthGui::getPrevControl() {
    control_index_ = (control_index_ + control_order_.size() - 1) %
      control_order_.size();
    return getCurrentControl();
  }
} // namespace mopo
