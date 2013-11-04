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

#include "delay.h"
#include "envelope.h"
#include "filter.h"
#include "operators.h"
#include "oscillator.h"
#include "processor_router.h"
#include "linear_slope.h"
#include "smooth_value.h"
#include "termite_strings.h"
#include "trigger_operators.h"
#include "value.h"

namespace laf {

  void TermiteVoiceHandler::createOscillators(Output* midi, Output* reset) {
    // Pitch bend.
    Value* pitch_bend_range = new Value(2);
    Multiply* pitch_bend = new Multiply();
    pitch_bend->plug(pitch_bend_amount_, 0);
    pitch_bend->plug(pitch_bend_range, 1);
    Add* final_midi = new Add();
    final_midi->plug(midi, 0);
    final_midi->plug(pitch_bend, 1);

    addGlobalProcessor(pitch_bend);
    addProcessor(final_midi);

    controls_["pitch bend range"] = new Control(pitch_bend_range, 0, 48, 48);

    // Oscillator 1.
    Value* oscillator1_waveform = new Value(Wave::kDownSaw);
    MidiScale* oscillator1_frequency = new MidiScale();
    oscillator1_frequency->plug(final_midi);
    oscillator1_ = new Oscillator();
    oscillator1_->plug(reset, Oscillator::kReset);
    oscillator1_->plug(oscillator1_waveform, Oscillator::kWaveform);
    oscillator1_->plug(oscillator1_frequency, Oscillator::kFrequency);

    addProcessor(oscillator1_frequency);
    addProcessor(oscillator1_);

    int wave_resolution = Wave::kNumWaveforms - 1;
    controls_["osc 1 waveform"] = new Control(oscillator1_waveform,
                                              TermiteStrings::wave_strings_,
                                              wave_resolution);

    // Oscillator 2.
    Value* oscillator2_waveform = new Value(Wave::kDownSaw);
    Value* oscillator2_transpose = new Value(-12);
    Value* oscillator2_tune = new Value(0.08);
    Add* oscillator2_transposed = new Add();
    oscillator2_transposed->plug(final_midi, 0);
    oscillator2_transposed->plug(oscillator2_transpose, 1);
    Add* oscillator2_midi = new Add();
    oscillator2_midi->plug(oscillator2_transposed, 0);
    oscillator2_midi->plug(oscillator2_tune, 1);

    MidiScale* oscillator2_frequency = new MidiScale();
    oscillator2_frequency->plug(oscillator2_midi);
    oscillator2_ = new Oscillator();
    oscillator2_->plug(reset, Oscillator::kReset);
    oscillator2_->plug(oscillator2_waveform, Oscillator::kWaveform);
    oscillator2_->plug(oscillator2_frequency, Oscillator::kFrequency);

    addProcessor(oscillator2_transposed);
    addProcessor(oscillator2_midi);
    addProcessor(oscillator2_frequency);
    addProcessor(oscillator2_);

    controls_["osc 2 waveform"] = new Control(oscillator2_waveform,
                                              TermiteStrings::wave_strings_,
                                              wave_resolution);
    controls_["osc 2 transpose"] =
        new Control(oscillator2_transpose, -48, 48, 96);
    controls_["osc 2 tune"] =
        new Control(oscillator2_tune, -1, 1, MIDI_SIZE);

    // LFO 1.
    Value* lfo1_waveform = new Value(Wave::kSin);
    SmoothValue* lfo1_frequency = new SmoothValue(2);
    lfo1_ = new Oscillator();
    lfo1_->plug(reset, Oscillator::kReset);
    lfo1_->plug(lfo1_waveform, Oscillator::kWaveform);
    lfo1_->plug(lfo1_frequency, Oscillator::kFrequency);

    addProcessor(lfo1_frequency);
    addProcessor(lfo1_);

    controls_["lfo 1 waveform"] = new Control(lfo1_waveform,
                                            TermiteStrings::wave_strings_,
                                            wave_resolution);
    controls_["lfo 1 frequency"] =
        new Control(lfo1_frequency, 0, 10, MIDI_SIZE);

    // LFO 2.
    Value* lfo2_waveform = new Value(Wave::kSin);
    SmoothValue* lfo2_frequency = new SmoothValue(2);
    lfo2_ = new Oscillator();
    lfo2_->plug(reset, Oscillator::kReset);
    lfo2_->plug(lfo2_waveform, Oscillator::kWaveform);
    lfo2_->plug(lfo2_frequency, Oscillator::kFrequency);

    addProcessor(lfo2_frequency);
    addProcessor(lfo2_);

    controls_["lfo 2 waveform"] = new Control(lfo2_waveform,
                                            TermiteStrings::wave_strings_,
                                            wave_resolution);
    controls_["lfo 2 frequency"] =
        new Control(lfo2_frequency, 0, 10, MIDI_SIZE);

    // Oscillator mix.
    oscillator_mix_ = new Add();
    oscillator_mix_->plug(oscillator1_, 0);
    oscillator_mix_->plug(oscillator2_, 1);

    addProcessor(oscillator_mix_);

    mod_sources_["osc 1"] = oscillator1_->output();
    mod_sources_["osc 2"] = oscillator2_->output();
  }

  void TermiteVoiceHandler::createFilter(
      Output* audio, Output* keytrack, Output* reset) {
    // Filter envelope.
    Value* filter_attack = new Value(0.0);
    Value* filter_decay = new Value(0.3);
    Value* filter_sustain = new Value(0.0);
    Value* filter_release = new Value(0.3);

    filter_envelope_ = new Envelope();
    filter_envelope_->plug(filter_attack, Envelope::kAttack);
    filter_envelope_->plug(filter_decay, Envelope::kDecay);
    filter_envelope_->plug(filter_sustain, Envelope::kSustain);
    filter_envelope_->plug(filter_release, Envelope::kRelease);
    filter_envelope_->plug(reset, Envelope::kTrigger);

    Value* filter_envelope_depth = new Value(12);
    Multiply* scaled_envelope = new Multiply();
    scaled_envelope->plug(filter_envelope_, 0);
    scaled_envelope->plug(filter_envelope_depth, 1);

    addProcessor(filter_envelope_);
    addProcessor(scaled_envelope);

    controls_["fil attack"] = new Control(filter_attack, 0, 3, MIDI_SIZE);
    controls_["fil decay"] = new Control(filter_decay, 0, 3, MIDI_SIZE);
    controls_["fil sustain"] = new Control(filter_sustain, 0, 1, MIDI_SIZE);
    controls_["fil release"] = new Control(filter_release, 0, 3, MIDI_SIZE);
    controls_["fil env depth"] =
        new Control(filter_envelope_depth, -MIDI_SIZE, MIDI_SIZE, MIDI_SIZE);

    // Filter.
    Value* filter_type = new Value(Filter::kLP12);
    Value* keytrack_amount = new Value(0);
    Multiply* current_keytrack = new Multiply();
    current_keytrack->plug(keytrack, 0);
    current_keytrack->plug(keytrack_amount, 1);

    SmoothValue* base_cutoff = new SmoothValue(92);
    Add* keytracked_cutoff = new Add();
    keytracked_cutoff->plug(base_cutoff, 0);
    keytracked_cutoff->plug(current_keytrack, 1);

    Add* midi_cutoff = new Add();
    midi_cutoff->plug(keytracked_cutoff, 0);
    midi_cutoff->plug(scaled_envelope, 1);
    MidiScale* frequency_cutoff = new MidiScale();
    frequency_cutoff->plug(midi_cutoff);

    Value* resonance = new Value(3);
    filter_ = new Filter();
    filter_->plug(audio, Filter::kAudio);
    filter_->plug(filter_type, Filter::kType);
    filter_->plug(reset, Filter::kReset);
    filter_->plug(frequency_cutoff, Filter::kCutoff);
    filter_->plug(resonance, Filter::kResonance);

    addGlobalProcessor(base_cutoff);
    addProcessor(current_keytrack);
    addProcessor(keytracked_cutoff);
    addProcessor(midi_cutoff);
    addProcessor(frequency_cutoff);
    addProcessor(filter_);

    controls_["filter type"] = new Control(filter_type,
                                           TermiteStrings::filter_strings_,
                                           Filter::kNumTypes - 1);
    controls_["cutoff"] = new Control(base_cutoff, 28, 127, MIDI_SIZE);
    controls_["keytrack"] = new Control(keytrack_amount, -1, 1, MIDI_SIZE);
    controls_["resonance"] = new Control(resonance, 0.5, 15, MIDI_SIZE);

    mod_sources_["filter env"] = filter_envelope_->output();
  }

  void TermiteVoiceHandler::createArticulation(
      Output* note, Output* velocity, Output* trigger) {
    // Legato.
    Value* legato = new Value(0);
    LegatoFilter* legato_filter = new LegatoFilter();
    legato_filter->plug(legato, LegatoFilter::kLegato);
    legato_filter->plug(trigger, LegatoFilter::kTrigger);

    controls_["legato"] =
        new Control(legato, TermiteStrings::legato_strings_, 1);
    addProcessor(legato_filter);

    // Amplitude envelope.
    Value* amplitude_attack = new Value(0.01);
    Value* amplitude_decay = new Value(0.7);
    Value* amplitude_sustain = new SmoothValue(0.0);
    Value* amplitude_release = new Value(0.3);

    amplitude_envelope_ = new Envelope();
    amplitude_envelope_->plug(legato_filter->output(LegatoFilter::kRetrigger),
                              Envelope::kTrigger);
    amplitude_envelope_->plug(amplitude_attack, Envelope::kAttack);
    amplitude_envelope_->plug(amplitude_decay, Envelope::kDecay);
    amplitude_envelope_->plug(amplitude_sustain, Envelope::kSustain);
    amplitude_envelope_->plug(amplitude_release, Envelope::kRelease);

    addProcessor(amplitude_envelope_);
    addGlobalProcessor(amplitude_sustain);

    controls_["amp attack"] = new Control(amplitude_attack, 0, 3, MIDI_SIZE);
    controls_["amp decay"] = new Control(amplitude_decay, 0, 3, MIDI_SIZE);
    controls_["amp sustain"] = new Control(amplitude_sustain, 0, 1, MIDI_SIZE);
    controls_["amp release"] = new Control(amplitude_release, 0, 3, MIDI_SIZE);

    // Voice and frequency resetting logic.
    TriggerCombiner* frequency_trigger = new TriggerCombiner();
    frequency_trigger->plug(legato_filter->output(LegatoFilter::kRemain), 0);
    frequency_trigger->plug(
        amplitude_envelope_->output(Envelope::kFinished), 1);

    TriggerWait* note_wait = new TriggerWait();
    Value* current_note = new Value();
    note_wait->plug(note, TriggerWait::kWait);
    note_wait->plug(frequency_trigger, TriggerWait::kTrigger);
    current_note->plug(note_wait);

    addProcessor(frequency_trigger);
    addProcessor(note_wait);
    addProcessor(current_note);

    // Key tracking.
    TriggerWait* velocity_wait = new TriggerWait();
    Value* current_velocity = new Value();

    Value* center_adjust = new Value(-MIDI_SIZE / 2);
    note_from_center_ = new Add();
    note_from_center_->plug(center_adjust, 0);
    note_from_center_->plug(current_note, 1);

    addProcessor(note_from_center_);
    addGlobalProcessor(center_adjust);

    // Velocity tracking.

    // Portamento.
    Value* portamento = new Value(0.01);
    Value* portamento_type = new Value(0);
    PortamentoFilter* portamento_filter = new PortamentoFilter();
    portamento_filter->plug(portamento_type, PortamentoFilter::kPortamento);
    portamento_filter->plug(frequency_trigger, PortamentoFilter::kTrigger);
    addProcessor(portamento_filter);

    current_frequency_ = new LinearSlope();
    current_frequency_->plug(current_note, LinearSlope::kTarget);
    current_frequency_->plug(portamento, LinearSlope::kRunSeconds);
    current_frequency_->plug(portamento_filter, LinearSlope::kTriggerJump);

    addProcessor(current_frequency_);
    controls_["portamento"] = new Control(portamento, 0.0, 0.2, MIDI_SIZE);
    int port_type_resolution = PortamentoFilter::kNumPortamentoStates - 1;
    controls_["portamento type"] =
        new Control(portamento_type, TermiteStrings::portamento_strings_,
                    port_type_resolution);

    mod_sources_["amp env"] = amplitude_envelope_->output();
    mod_sources_["note"] = current_note->output();
    mod_sources_["velocity"] = velocity;
  }

  TermiteVoiceHandler::TermiteVoiceHandler() {
    pitch_bend_amount_ = new SmoothValue(0);
    controls_["pitch bend amount"] =
        new Control(pitch_bend_amount_, -1, 1, MIDI_SIZE);

    createArticulation(note(), velocity(), voice_event());
    createOscillators(current_frequency_->output(),
                      amplitude_envelope_->output(Envelope::kFinished));
    createFilter(oscillator_mix_->output(), note_from_center_->output(),
                 amplitude_envelope_->output(Envelope::kFinished));

    output_ = new Multiply();
    output_->plug(filter_->output(), 0);
    output_->plug(amplitude_envelope_->output(Envelope::kValue), 1);

    addProcessor(output_);
    setVoiceOutput(output_);
    setVoiceKiller(amplitude_envelope_->output(Envelope::kValue));

    mod_sources_["pitch wheel"] = pitch_bend_amount_->output();
  }

  TermiteSynth::TermiteSynth() {
    // Voice Handler.
    Value* polyphony = new Value(12);
    voice_handler_ = new TermiteVoiceHandler();
    voice_handler_->setPolyphony(64);
    voice_handler_->plug(polyphony, VoiceHandler::kPolyphony);

    addProcessor(voice_handler_);
    controls_["polyphony"] = new Control(polyphony, 1, 32, 31);

    // Delay effect.
    SmoothValue* delay_time = new SmoothValue(0.06);
    SmoothValue* delay_feedback = new SmoothValue(-0.3);
    SmoothValue* delay_wet = new SmoothValue(0.3);

    Delay* delay = new Delay();
    delay->plug(voice_handler_, Delay::kAudio);
    delay->plug(delay_time, Delay::kDelayTime);
    delay->plug(delay_feedback, Delay::kFeedback);
    delay->plug(delay_wet, Delay::kWet);

    addProcessor(delay_time);
    addProcessor(delay_feedback);
    addProcessor(delay_wet);
    addProcessor(delay);

    controls_["delay time"] = new Control(delay_time, 0.01, 1, MIDI_SIZE);
    controls_["delay feedback"] = new Control(delay_feedback, -1, 1, MIDI_SIZE);
    controls_["delay dry/wet"] = new Control(delay_wet, 0, 1, MIDI_SIZE);

    // Volume.
    SmoothValue* volume = new SmoothValue(0.6);
    Multiply* scaled_audio = new Multiply();
    scaled_audio->plug(delay, 0);
    scaled_audio->plug(volume, 1);
    Clamp* clamp = new Clamp();
    clamp->plug(scaled_audio);

    addProcessor(clamp);
    addProcessor(volume);
    addProcessor(scaled_audio);
    registerOutput(scaled_audio->output());

    controls_["volume"] = new Control(volume, 0, 1, MIDI_SIZE);
  }

  control_map TermiteSynth::getControls() {
    control_map voice_controls = voice_handler_->getControls();
    voice_controls.insert(controls_.begin(), controls_.end());
    return voice_controls;
  }

  void TermiteSynth::noteOn(laf_float note, laf_float velocity) {
    voice_handler_->noteOn(note, velocity);
  }

  void TermiteSynth::noteOff(laf_float note) {
    voice_handler_->noteOff(note);
  }
} // namespace laf
