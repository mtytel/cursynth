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

#include "delay.h"
#include "envelope.h"
#include "filter.h"
#include "operators.h"
#include "oscillator.h"
#include "processor_router.h"
#include "smooth_filter.h"
#include "linear_slope.h"
#include "smooth_value.h"
#include "trigger_operators.h"
#include "value.h"
#include "voice_handler.h"
#include "termite_common.h"

#include <vector>

namespace laf {

  class TermiteSection {
    public:
      ControlGroup* getControls() { return &section_; }

    protected:
      ControlGroup section_;
  };

  class TermiteOscillators : public ProcessorRouter, public TermiteSection {
    public:
      TermiteOscillators();

    private:
      Value pitch_bend_amount_;
      Value pitch_bend_range_;
      Multiply pitch_bend_;
      Add final_midi_;
      MidiScale oscillator_1_freq_;
      MidiScale oscillator_2_freq_;
      Oscillator oscillator_1_;
      Value oscillator_1_wave_type_;
      Value oscillator_2_wave_type_;
      Value oscillator_2_transpose_;
      Add oscillator_2_midi_;
      Oscillator oscillator_2_;
      Add oscillator_mix_;
  };

  class TermiteFilter : public ProcessorRouter, public TermiteSection {
    public:
      TermiteFilter();

    private:
      Filter filter_;
      Value base_cutoff_;
      Multiply keytrack_;
      Add keytrack_cutoff_;
      Value keytrack_amount_;
      Value resonance_;
      Value filter_envelope_depth_;
      Multiply depth_scale_;
      Add midi_cutoff_;
      MidiScale freq_cutoff_;

      Envelope filter_envelope_;
      Value filter_attack_;
      Value filter_decay_;
      Value filter_sustain_;
      Value filter_release_;
  };

  class TermiteArticulation : public ProcessorRouter, public TermiteSection {
    public:
      TermiteArticulation();

    private:
      Value note_;
      TriggerWait note_wait_;
      Value legato_;
      Value portamento_;
      Value portamento_state_;
      LegatoFilter legato_filter_;
      PortamentoFilter portamento_filter_;
      LinearSlope midi_sent_;
      TriggerCombiner frequency_ready_;

      Envelope amplitude_envelope_;
      Value amplitude_attack_;
      Value amplitude_decay_;
      Value amplitude_sustain_;
      Value amplitude_release_;
  };

  class TermiteVoiceHandler : public VoiceHandler, public TermiteSection {
    public:
      TermiteVoiceHandler();

    private:
      TermiteArticulation articulation_;
      Value center_adjust_;
      Add note_from_center_;
      TermiteOscillators oscillators_;
      TermiteFilter filter_;
      Multiply output_;

      ControlGroup amplifier_group_;
  };

  class TermiteSynth : public ProcessorRouter, public TermiteSection {
    public:
      TermiteSynth();

      void process();
      void noteOn(laf_sample note, laf_sample velocity = 1);
      void noteOff(laf_sample note);
      void sustainOn() { voice_handler_.sustainOn(); }
      void sustainOff() { voice_handler_.sustainOff(); }

    private:
      Value polyphony_;
      TermiteVoiceHandler voice_handler_;
      Delay delay_;
      SmoothValue delay_time_;
      SmoothValue delay_feedback_;
      SmoothValue delay_wet_;
      SmoothValue volume_;
      Multiply volume_mult_;
  };
} // namespace laf

#endif // TERMITE_SYNTH_H
