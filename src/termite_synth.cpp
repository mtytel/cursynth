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

#include "termite_synth.h"

namespace laf {

  TermiteOscillators::TermiteOscillators() {
    registerInput(midi_input_.input());
    portamento_.set(0.0001);
    midi_input_.plug(&portamento_, SmoothFilter::kDecay);

    oscillator_1_wave_type_.set(Wave::kDownSaw);
    oscillator_1_.plug(&oscillator_1_wave_type_, Oscillator::kWaveType);
    oscillator_1_freq_.plug(&midi_input_);
    oscillator_1_.plug(&oscillator_1_freq_, Oscillator::kFrequency);

    oscillator_2_wave_type_.set(Wave::kDownSaw);
    oscillator_2_.plug(&oscillator_2_wave_type_, Oscillator::kWaveType);
    oscillator_2_transpose_.set(0);
    oscillator_2_midi_.plug(&midi_input_, 0);
    oscillator_2_midi_.plug(&oscillator_2_transpose_, 1);
    oscillator_2_freq_.plug(&oscillator_2_midi_);
    oscillator_2_.plug(&oscillator_2_freq_, Oscillator::kFrequency);

    oscillator_mix_.plug(&oscillator_1_, 0);
    oscillator_mix_.plug(&oscillator_2_, 1);

    addProcessor(&midi_input_);
    addProcessor(&oscillator_1_);
    addProcessor(&oscillator_1_freq_);
    addProcessor(&oscillator_2_midi_);
    addProcessor(&oscillator_2_freq_);
    addProcessor(&oscillator_2_);
    addProcessor(&oscillator_mix_);

    registerOutput(oscillator_mix_.output());
    section_.controls["portamento"] =
        new Control(&portamento_, 0.0, 0.001, 128);
    section_.controls["oscillator 1 waveform"] =
        new Control(&oscillator_1_wave_type_, 0,
                    Wave::kNumWaveforms - 1, Wave::kNumWaveforms - 1);
    section_.controls["oscillator 2 waveform"] =
        new Control(&oscillator_2_wave_type_, 0,
                    Wave::kNumWaveforms - 1, Wave::kNumWaveforms - 1);
    section_.controls["oscillator 2 transpose"] =
        new Control(&oscillator_2_transpose_, -48, 48, 96);
  }

  TermiteFilter::TermiteFilter() {
    registerInput(filter_.input(Filter::kAudio));
    registerInput(keytrack_.input(0));
    keytrack_.plug(&keytrack_amount_, 1);

    filter_attack_.set(0.2);
    filter_decay_.set(0.1);
    filter_sustain_.set(0);
    filter_release_.set(0.3);

    filter_envelope_.plug(&filter_attack_, Envelope::kAttack);
    filter_envelope_.plug(&filter_decay_, Envelope::kDecay);
    filter_envelope_.plug(&filter_sustain_, Envelope::kSustain);
    filter_envelope_.plug(&filter_release_, Envelope::kRelease);
    filter_envelope_depth_.set(12);

    depth_scale_.plug(&filter_envelope_, 0);
    depth_scale_.plug(&filter_envelope_depth_, 1);
    base_cutoff_.set(92);
    keytrack_cutoff_.plug(&base_cutoff_, 0);
    keytrack_cutoff_.plug(&keytrack_, 1);
    midi_cutoff_.plug(&keytrack_cutoff_, 0);
    midi_cutoff_.plug(&depth_scale_, 1);
    freq_cutoff_.plug(&midi_cutoff_);

    resonance_.set(3);
    filter_.plug(&freq_cutoff_, Filter::kCutoff);
    filter_.plug(&resonance_, Filter::kResonance);

    addProcessor(&filter_);
    addProcessor(&depth_scale_);
    addProcessor(&freq_cutoff_);
    addProcessor(&keytrack_);
    addProcessor(&keytrack_cutoff_);
    addProcessor(&midi_cutoff_);
    addProcessor(&filter_envelope_);

    registerOutput(filter_.output());
    section_.controls["cutoff"] = new Control(&base_cutoff_, 28, 127, 128);
    section_.controls["keytrack"] = new Control(&keytrack_amount_, -1, 1, 128);
    section_.controls["resonance"] = new Control(&resonance_, 0.5, 15, 128);
    section_.controls["fil env depth"] =
        new Control(&filter_envelope_depth_, -128, 128, 128);
    section_.controls["fil attack"] = new Control(&filter_attack_, 0, 3, 128);
    section_.controls["fil decay"] = new Control(&filter_decay_, 0, 3, 128);
    section_.controls["fil sustain"] =
        new Control(&filter_sustain_, 0, 1, 128);
    section_.controls["fil release"] =
        new Control(&filter_release_, 0, 3, 128);
  }

  TermiteAmp::TermiteAmp() {
    registerInput(amplitude_.input(0));

    amplitude_attack_.set(0.01);
    amplitude_decay_.set(0.6);
    amplitude_sustain_.set(0);
    amplitude_release_.set(0.3);

    amplitude_envelope_.plug(&amplitude_attack_, Envelope::kAttack);
    amplitude_envelope_.plug(&amplitude_decay_, Envelope::kDecay);
    amplitude_envelope_.plug(&amplitude_sustain_, Envelope::kSustain);
    amplitude_envelope_.plug(&amplitude_release_, Envelope::kRelease);
    amplitude_.plug(&amplitude_envelope_, 1);

    addProcessor(&amplitude_);
    addProcessor(&amplitude_envelope_);

    registerOutput(amplitude_.output());
    section_.controls["amp attack"] =
        new Control(&amplitude_attack_, 0, 3, 128);
    section_.controls["amp decay"] = new Control(&amplitude_decay_, 0, 3, 128);
    section_.controls["amp sustain"] =
        new Control(&amplitude_sustain_, 0, 1, 128);
    section_.controls["amp release"] =
        new Control(&amplitude_release_, 0, 3, 128);
  }

  TermiteVoiceHandler::TermiteVoiceHandler() {
    setPolyphony(20);
    registerInput(offset_note_.input(0));
    offset_note_.plug(note(), 1);

    oscillators_.plug(&offset_note_);
    filter_.plug(&oscillators_);
    center_adjust_.set(-64);
    note_from_center_.plug(&center_adjust_, 0);
    note_from_center_.plug(note(), 1);
    filter_.plug(&note_from_center_, 1);
    amplifier_.plug(&filter_);

    addProcessor(&note_from_center_);
    addProcessor(&offset_note_);
    addProcessor(&oscillators_);
    addProcessor(&filter_);
    addProcessor(&amplifier_);

    setVoiceOutput(&amplifier_);
    setVoiceKiller(amplifier_.envelope());

    section_.sub_groups["oscillators"] = oscillators_.getControls();
    section_.sub_groups["filter"] = filter_.getControls();
    section_.sub_groups["amplifier"] = amplifier_.getControls();
  }

  TermiteSynth::TermiteSynth() : ProcessorRouter(0, 1) {
    volume_.setHard(0.6);
    volume_mult_.plug(&delay_, 0);
    volume_mult_.plug(&volume_, 1);

    pitch_bend_range_.set(2);
    pitch_bend_.plug(&pitch_bend_amount_, 0);
    pitch_bend_.plug(&pitch_bend_range_, 1);
    voice_handler_.plug(&pitch_bend_);

    delay_time_.setHard(0.3);
    delay_feedback_.setHard(0.4);
    delay_wet_.setHard(0.2);

    delay_.plug(&voice_handler_, Delay::kAudio);
    delay_.plug(&delay_time_, Delay::kDelayTime);
    delay_.plug(&delay_feedback_, Delay::kFeedback);
    delay_.plug(&delay_wet_, Delay::kWet);

    addProcessor(&pitch_bend_);
    addProcessor(&volume_);
    addProcessor(&volume_mult_);
    addProcessor(&voice_handler_);

    addProcessor(&delay_);
    addProcessor(&delay_time_);
    addProcessor(&delay_feedback_);
    addProcessor(&delay_wet_);

    section_.controls["volume"] = new Control(&volume_, 0, 1, 128);
    section_.controls["pitch bend range"] =
        new Control(&pitch_bend_range_, 0, 24, 24);
    section_.controls["delay time"] = new Control(&delay_time_, 0.01, 1, 128);
    section_.controls["delay feedback"] =
        new Control(&delay_feedback_, -1, 1, 128);
    section_.controls["delay dry/wet"] = new Control(&delay_wet_, 0, 1, 128);
    section_.sub_groups["voice handler"] = voice_handler_.getControls();
  }

  void TermiteSynth::process() {
    ProcessorRouter::process();
    memcpy(outputs_[0]->buffer, volume_mult_.output()->buffer,
           sizeof(laf_sample) * BUFFER_SIZE);
  }

  void TermiteSynth::pitchBend(laf_sample amount) {
    pitch_bend_amount_.set(amount);
  }

  void TermiteSynth::noteOn(laf_sample note, laf_sample velocity) {
    voice_handler_.noteOn(note, velocity);
  }

  void TermiteSynth::noteOff(laf_sample note) {
    voice_handler_.noteOff(note);
  }
} // namespace laf
