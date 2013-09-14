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
#ifndef TERMITE_GUI_H
#define TERMITE_GUI_H

#include "laf.h"
#include "termite_common.h"

#include <map>
#include <string>

namespace laf {
  class Value;
  class TermiteSynth;

  class TermiteGui {
    public:
      enum {
        SLIDER_FG_COLOR = 1,
        SLIDER_BG_COLOR,
        LOGO_COLOR,
      };

      void start();
      void stop();

      void addGlobalControls(const control_map* controls);
      void addVoiceControls(const control_map* controls);

      void drawControl(const Control* control, bool active);
      void drawControlStatus(const Control* control, bool armed);
      void clearSave();
      void drawSave(std::string file_name);
      void clearLoad();
      void drawLoad(std::string file_name);

    private:
      struct Slider {
        int x, y, width;
        std::string label;
        bool bipolar;
      };

      void drawLogo();
      void drawMidi(std::string status);
      void drawStatus(std::string status);
      void placeSliders(std::map<std::string, Control*> controls,
                        int x, int y, int width);

      std::map<const Control*, Slider*> slider_lookup_;
  };
} // namespace laf

#endif // TERMITE_GUI_H
