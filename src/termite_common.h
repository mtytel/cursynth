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

  class Control {
    public:
      Control(Value* value, laf_float min, laf_float max, int resolution) :
        value_(value), min_(min), max_(max),
        resolution_(resolution), midi_learn_(0), display_strings_(0) {
          current_value_ = value->value();
      }

      Control(Value* value, const char** strings, int resolution) :
        value_(value), min_(0), max_(resolution),
        resolution_(resolution), midi_learn_(0), display_strings_(strings) {
          current_value_ = value->value();
      }

      Control() : value_(0), min_(0), max_(0), current_value_(0),
                  resolution_(0), midi_learn_(0), display_strings_(0) { }

      void set(laf_float val) {
        current_value_ = CLAMP(val, min_, max_);
        value_->set(current_value_);
      }

      void setMidi(int midi_val) {
        int index = resolution_ * midi_val / (MIDI_SIZE - 1);
        set(min_ + index * (max_ - min_) / resolution_);
      }

      void increment() {
        set(current_value_ + (max_ - min_) / resolution_);
      }

      void decrement() {
        set(current_value_ - (max_ - min_) / resolution_);
      }

      laf_float getPercentage() const {
        return (current_value_ - min_) / (max_ - min_);
      }

      int midi_learn() const { return midi_learn_; }

      void midi_learn(float midi) { midi_learn_ = midi; }

      const char** display_strings() const { return display_strings_; }

      laf_float current_value() const { return current_value_; }

      const Value* value() const { return value_; }

      bool isBipolar() const { return max_ == -min_; }

    private:
      Value* value_;
      laf_float min_, max_, current_value_;
      int resolution_, midi_learn_;
      const char** display_strings_;
  };

  typedef std::map<std::string, Control*> control_map;
  typedef std::map<std::string, Processor::Input*> input_map;
  typedef std::map<std::string, Processor::Output*> output_map;

} // namespace laf

#endif // TERMITE_COMMON_H
