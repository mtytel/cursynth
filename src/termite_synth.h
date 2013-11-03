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
#ifndef TERMITE_SYNTH_H
#define TERMITE_SYNTH_H

#include "termite_common.h"
#include "voice_handler.h"

#include <vector>

namespace laf {
  class Add;
  class Envelope;
  class Filter;
  class LinearSlope;
  class Multiply;
  class Oscillator;
  class SmoothValue;

  class TermiteVoiceHandler : public VoiceHandler {
    public:
      TermiteVoiceHandler();

      control_map getControls() { return controls_; }

    private:
      void createArticulation(Output* note, Output* velocity, Output* trigger);
      void createOscillators(Output* frequency, Output* reset);
      void createFilter(Output* audio, Output* keytrack, Output* reset);

      Add* note_from_center_;
      SmoothValue* pitch_bend_amount_;
      LinearSlope* current_frequency_;
      Envelope* amplitude_envelope_;

      Oscillator* oscillator1_;
      Oscillator* oscillator2_;
      Add* oscillator_mix_;

      Filter* filter_;
      Envelope* filter_envelope_;

      Multiply* output_;

      control_map controls_;
      output_map mod_sources_;
      input_map mod_destinations_;
  };

  class TermiteSynth : public ProcessorRouter {
    public:
      TermiteSynth();

      control_map getControls();

      void noteOn(laf_float note, laf_float velocity = 1);
      void noteOff(laf_float note);
      void sustainOn() { voice_handler_->sustainOn(); }
      void sustainOff() { voice_handler_->sustainOff(); }

    private:
      TermiteVoiceHandler* voice_handler_;

      control_map controls_;
  };
} // namespace laf

#endif // TERMITE_SYNTH_H
