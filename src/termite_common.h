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
#ifndef TERMITE_COMMON_H
#define TERMITE_COMMON_H

#include "value.h"

#include <map>
#include <string>

namespace laf {

  struct Control {
    Control(Value* value, laf_float min, laf_float max, int resolution) :
        value(value), min(min), max(max),
        resolution(resolution), midi_learn(0), display_strings(0) {
      current_value = value->value();
    }

    Control(Value* value, const char** strings, int resolution) :
        value(value), min(0), max(resolution),
        resolution(resolution), midi_learn(0), display_strings(strings) {
      current_value = value->value();
    }

    Control() : value(0), min(0), max(0), current_value(0),
                resolution(0), midi_learn(0), display_strings(0) { }

    Value* value;
    laf_float min, max, current_value;
    int resolution, midi_learn;
    const char** display_strings;
  };

  typedef std::map<std::string, Control*> control_map;

} // namespace laf

#endif // TERMITE_COMMON_H
