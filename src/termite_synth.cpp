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
    registerInput(final_midi_.input(0));
    registerInput(oscillator_1_.input(Oscillator::kReset));
    registerInput(oscillator_2_.input(Oscillator::kReset));

    pitch_bend_range_.set(2);
    pitch_bend_amount_.set(0);
    pitch_bend_.plug(&pitch_bend_amount_, 0);
    pitch_bend_.plug(&pitch_bend_range_, 1);

    final_midi_.plug(&pitch_bend_, 1);

    oscillator_1_wave_type_.set(Wave::kDownSaw);
    oscillator_1_.plug(&oscillator_1_wave_type_, Oscillator::kWaveform);
    oscillator_1_freq_.plug(&final_midi_);
    oscillator_1_.plug(&oscillator_1_freq_, Oscillator::kFrequency);

    oscillator_2_wave_type_.set(Wave::kDownSaw);
    oscillator_2_.plug(&oscillator_2_wave_type_, Oscillator::kWaveform);
    oscillator_2_transpose_.set(0);
    oscillator_2_midi_.plug(&final_midi_, 0);
    oscillator_2_midi_.plug(&oscillator_2_transpose_, 1);
    oscillator_2_freq_.plug(&oscillator_2_midi_);
    oscillator_2_.plug(&oscillator_2_freq_, Oscillator::kFrequency);

    oscillator_mix_.plug(&oscillator_1_, 0);
    oscillator_mix_.plug(&oscillator_2_, 1);

    addProcessor(&pitch_bend_);
    addProcessor(&final_midi_);
    addProcessor(&oscillator_1_);
    addProcessor(&oscillator_1_freq_);
    addProcessor(&oscillator_2_midi_);
    addProcessor(&oscillator_2_freq_);
    addProcessor(&oscillator_2_);
    addProcessor(&oscillator_mix_);

    registerOutput(oscillator_mix_.output());
    section_.controls["oscillator 1 waveform"] =
        new Control(&oscillator_1_wave_type_, 0,
                    Wave::kNumWaveforms - 1, Wave::kNumWaveforms - 1);
    section_.controls["oscillator 2 waveform"] =
        new Control(&oscillator_2_wave_type_, 0,
                    Wave::kNumWaveforms - 1, Wave::kNumWaveforms - 1);
    section_.controls["oscillator 2 transpose"] =
        new Control(&oscillator_2_transpose_, -48, 48, 96);
    section_.controls["pitch bend range"] =
        new Control(&pitch_bend_range_, 0, 48, 48);
    section_.controls["pitch bend amount"] =
        new Control(&pitch_bend_amount_, -1, 1, 128);
  }

  TermiteFilter::TermiteFilter() {
    registerInput(filter_.input(Filter::kAudio));
    registerInput(keytrack_.input(0));
    registerInput(filter_envelope_.input(Envelope::kTrigger));
    registerInput(filter_.input(Filter::kReset));
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

  TermiteArticulation::TermiteArticulation() {
    registerInput(note_wait_.input(TriggerWait::kWait));
    registerInput(legato_filter_.input(LegatoFilter::kTrigger));

    portamento_.set(0.01);
    portamento_state_.set(0);
    legato_.set(0);

    legato_filter_.plug(&legato_, LegatoFilter::kLegato);
    frequency_ready_.plug(legato_filter_.output(LegatoFilter::kRemain), 0);
    frequency_ready_.plug(amplitude_envelope_.output(Envelope::kFinished), 1);
    note_wait_.plug(&frequency_ready_, TriggerWait::kTrigger);
    note_.plug(&note_wait_);

    portamento_filter_.plug(&portamento_state_, PortamentoFilter::kPortamento);
    portamento_filter_.plug(&frequency_ready_, PortamentoFilter::kTrigger);
    midi_sent_.plug(&note_, LinearSlope::kTarget);
    midi_sent_.plug(&portamento_, LinearSlope::kRunSeconds);
    midi_sent_.plug(&portamento_filter_, LinearSlope::kTriggerJump);

    amplitude_attack_.set(0.01);
    amplitude_decay_.set(0.6);
    amplitude_sustain_.set(0);
    amplitude_release_.set(0.3);

    amplitude_envelope_.plug(legato_filter_.output(LegatoFilter::kRetrigger),
                             Envelope::kTrigger);
    amplitude_envelope_.plug(&amplitude_attack_, Envelope::kAttack);
    amplitude_envelope_.plug(&amplitude_decay_, Envelope::kDecay);
    amplitude_envelope_.plug(&amplitude_sustain_, Envelope::kSustain);
    amplitude_envelope_.plug(&amplitude_release_, Envelope::kRelease);

    registerOutput(midi_sent_.output());
    registerOutput(amplitude_envelope_.output(Envelope::kFinished));
    registerOutput(amplitude_envelope_.output(Envelope::kValue));

    addProcessor(&note_);
    addProcessor(&note_wait_);
    addProcessor(&legato_filter_);
    addProcessor(&frequency_ready_);
    addProcessor(&portamento_filter_);
    addProcessor(&midi_sent_);
    addProcessor(&amplitude_envelope_);

    section_.controls["portamento"] =
        new Control(&portamento_, 0.0, 0.2, 128);
    section_.controls["portamento state"] =
        new Control(&portamento_state_, 0, 2, 2);
    section_.controls["legato"] =
        new Control(&legato_, 0, 1, 1);
    section_.controls["amp attack"] =
        new Control(&amplitude_attack_, 0, 3, 128);
    section_.controls["amp decay"] =
        new Control(&amplitude_decay_, 0, 3, 128);
    section_.controls["amp sustain"] =
        new Control(&amplitude_sustain_, 0, 1, 128);
    section_.controls["amp release"] =
        new Control(&amplitude_release_, 0, 3, 128);
  }

  TermiteVoiceHandler::TermiteVoiceHandler() {
    articulation_.plug(note(), 0);
    articulation_.plug(voice_event(), 1);

    oscillators_.plug(articulation_.output(0));
    oscillators_.plug(articulation_.output(1), 1);
    oscillators_.plug(articulation_.output(1), 2);
    center_adjust_.set(-MIDI_SIZE / 2);
    note_from_center_.plug(&center_adjust_, 0);
    note_from_center_.plug(articulation_.output(0), 1);
    filter_.plug(&oscillators_, 0);
    filter_.plug(&note_from_center_, 1);
    filter_.plug(articulation_.output(1), 2);
    filter_.plug(articulation_.output(1), 3);

    output_.plug(&filter_, 0);
    output_.plug(articulation_.output(2), 1);
    addProcessor(&output_);
    addProcessor(&note_from_center_);
    addProcessor(&articulation_);
    addProcessor(&oscillators_);
    addProcessor(&filter_);

    setVoiceOutput(&output_);
    setVoiceKiller(articulation_.output(2));

    section_.sub_groups["oscillators"] = oscillators_.getControls();
    section_.sub_groups["filter"] = filter_.getControls();
    section_.sub_groups["articulation"] = articulation_.getControls();
  }

  TermiteSynth::TermiteSynth() : ProcessorRouter(0, 1) {
    polyphony_.set(1);
    voice_handler_.setPolyphony(64);
    voice_handler_.plug(&polyphony_, VoiceHandler::kPolyphony);
    volume_.setHard(0.6);
    volume_mult_.plug(&delay_, 0);
    volume_mult_.plug(&volume_, 1);

    delay_time_.setHard(0.3);
    delay_feedback_.setHard(0.4);
    delay_wet_.setHard(0.2);

    delay_.plug(&voice_handler_, Delay::kAudio);
    delay_.plug(&delay_time_, Delay::kDelayTime);
    delay_.plug(&delay_feedback_, Delay::kFeedback);
    delay_.plug(&delay_wet_, Delay::kWet);

    addProcessor(&volume_);
    addProcessor(&volume_mult_);
    addProcessor(&voice_handler_);

    addProcessor(&delay_);
    addProcessor(&delay_time_);
    addProcessor(&delay_feedback_);
    addProcessor(&delay_wet_);

    section_.controls["volume"] = new Control(&volume_, 0, 1, 128);
    section_.controls["polyphony"] = new Control(&polyphony_, 1, 32, 31);
    section_.controls["delay time"] = new Control(&delay_time_, 0.01, 1, 128);
    section_.controls["delay feedback"] =
        new Control(&delay_feedback_, -1, 1, 128);
    section_.controls["delay dry/wet"] = new Control(&delay_wet_, 0, 1, 128);
    section_.sub_groups["voice handler"] = voice_handler_.getControls();
  }

  void TermiteSynth::process() {
    ProcessorRouter::process();
    memcpy(outputs_[0]->buffer, volume_mult_.output()->buffer,
           sizeof(laf_float) * BUFFER_SIZE);
  }

  void TermiteSynth::noteOn(laf_float note, laf_float velocity) {
    voice_handler_.noteOn(note, velocity);
  }

  void TermiteSynth::noteOff(laf_float note) {
    voice_handler_.noteOff(note);
  }
} // namespace laf
