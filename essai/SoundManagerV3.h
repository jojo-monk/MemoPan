#if 0
#ifndef SOUNDMANAGERV3_H
#define SOUNDMANAGERV3_H

#include "Debug.h"
#include "config.h"
//#include <Audio.h>
#include "Presets.h"
#include "AudioEngine.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "AudioSampleClosedhh.h"
#include "AudioSampleClosedhh40msw.h"
#include "AudioSampleCowbell.h"
#include "AudioSampleCowbell100msw.h"
#include "AudioSampleCowbell40msw.h"
#include "AudioSampleKick.h"
#include "AudioSampleKick40msw.h"
#include "AudioSampleOpenhh.h"
#include "AudioSampleOpenhh100msw.h"
#include "AudioSampleOpenhh40msw.h"
#include "AudioSampleLowcongaw.h"
#include "AudioSampleLowconga100msw.h"
#include "AudioSampleLowconga40msw.h"
#include "AudioSampleRimshot.h"
#include "AudioSampleRimshot40msw.h"
#include "AudioSampleSnare.h"
#include "AudioSampleSnare40msw.h"
#include "AudioSampleTabla1.h"
#include "AudioSampleTabla1100msw.h"
#include "AudioSampleTabla140msw.h"
#include "AudioSampleTabla2.h"
#include "AudioSampleTabla2100msw.h"
#include "AudioSampleTabla240msw.h"
#include "AudioSampleTabla3.h"
#include "AudioSampleTabla3100msw.h"
#include "AudioSampleTabla340msw.h"
#include "AudioSampleTabla4.h"
#include "AudioSampleTabla4100msw.h"
#include "AudioSampleTabla440msw.h"
#include "AudioSampleTabla5.h"
#include "AudioSampleTabla5100msw.h"
#include "AudioSampleTabla540msw.h"
#include "MidiManager.h"
#include "MapToMidi.h"
#include "SysExHandler.h"
#include "PresetStorage.h"


#define CHORUS_DELAY_LENGTH (32*AUDIO_BLOCK_SAMPLES)
//#define FLANGE_DELAY_LENGTH (2*AUDIO_BLOCK_SAMPLES)

MidiManager midiManager;

class SoundManager {
private:
  // Objects audio
  AudioEngine audioEngine;
  Preset presetBank[NUM_PRESETS];
  Preset currentPreset;
  SysExHandler* sysexHandler;
  PresetStorage storage;  
  int currentSoundMode;
  bool isPlaying;
  unsigned long soundStartTime;
  float scaleFreq[NUM_TOUCH_PADS];
  bool polyMode = false;               // mode poly activable/désactivable
  float polyMixerGain = 0.5;
  float stringLevel = 1;
  float drumLevel = 2.5;
  float sampleLevel = 0.7;
  int keyboardMode = 0;
  int randomMap[NUM_TOUCH_PADS];
  bool drumMode = false;
  bool stringMode = false;
  bool arpegioMode = false;
  float maxTime = 200.0f; // durée max en ms pour vélocité
  float maxPressure = 1000.0f; // pression max pour vélocité
  uint16_t sampleDuration = 500;
  

  // Number of samples in each delay line

  // Allocate the delay lines for left and right channels
  short delayline[CHORUS_DELAY_LENGTH];

  
  //static const SoundMode soundModes[NUM_SOUND_MODES];
  static constexpr float MONO_GAIN = 0.5f;
  static constexpr float POLY_GAIN = 0.5f;
  static constexpr float AMP2_GAIN = 1.0f;
  float getNote(int padIndex);
  void stopMidi();
  void playMidi(int touchIndex, float frquency, float velocity);
  void applyPreset(Preset* preset);
  
  // Définition des samples de batterie
  // Chaque tableau doit être déclaré dans un fichier séparé
const unsigned int* drumSamplesLong[NUM_TOUCH_PADS] = {
  AudioSampleKick,
  AudioSampleSnare,
  AudioSampleOpenhh,
  AudioSampleClosedhh,
  AudioSampleCowbell,
  AudioSampleLowcongaw,
  AudioSampleRimshot,
  AudioSampleTabla1,
  AudioSampleTabla2,
  AudioSampleTabla3,
  AudioSampleTabla4,
  AudioSampleTabla5
};

const unsigned int* drumSamples100ms[NUM_TOUCH_PADS] = {
  AudioSampleKick40msw,
  AudioSampleSnare40msw,
  AudioSampleOpenhh100msw,
  AudioSampleClosedhh40msw,
  AudioSampleCowbell100msw,
  AudioSampleLowconga100msw,
  AudioSampleRimshot40msw,
  AudioSampleTabla1100msw,
  AudioSampleTabla2100msw,
  AudioSampleTabla3100msw,
  AudioSampleTabla4100msw,
  AudioSampleTabla5100msw
};

const unsigned int* drumSamples40ms[NUM_TOUCH_PADS] = {
  AudioSampleKick40msw,
  AudioSampleSnare40msw,
  AudioSampleOpenhh40msw,
  AudioSampleClosedhh40msw,
  AudioSampleCowbell40msw,
  AudioSampleLowconga40msw,
  AudioSampleRimshot40msw,
  AudioSampleTabla140msw,
  AudioSampleTabla240msw,
  AudioSampleTabla340msw,
  AudioSampleTabla440msw,
  AudioSampleTabla540msw
};

public:
  SoundManager();
  bool begin();
  void setSoundMode(int mode);
  void playTouchSound(int touchIndex, float pression, uint8_t velocite, int octaveShift);
  void stopOneNote(int touchIndex);
  void update();
  void volume(uint8_t vol);
  void buildScale(int noteIndex, int gammeIndex);
  void setReverb(float size, float damp);
  void setDryWetMix(uint8_t mix);
  void setTremoloLevel(uint8_t tremoloLevel);
  void setMoogFilterLevel(uint8_t moogLevel);
  void setChorusLevel(uint8_t chorusLevel);
  void handleControlChange(uint8_t cc, uint8_t value);
  int getSoundMode() const { 
    return currentSoundMode; 
  }
  const char* getSoundModeName() const;
  void setKeyboardMode(int mode) { 
    keyboardMode = mode; 
  }
  void generateRandomMap();
  void setPolyMode(bool);
  void testPoly();
  void updateMidiNotes();
  void unMute();
  void stopAllSounds();
  void setPolyEnv(int attack, int decay, float sustain, int release);
  void setArpeggio(bool enabled);
  void soundDuration(uint16_t durationMs);
  void getAllCC(uint8_t cc, uint8_t value);
  void readVolumePot();
  bool isPlayingNotes();
  Preset* getCurrentPreset() { return &currentPreset; }
  Preset* getPreset(int index) { 
      if (index >= 0 && index < NUM_PRESETS) 
          return &presetBank[index]; 
      return nullptr;
  }
  
};
uint8_t activeMidiNotes[NUM_TOUCH_PADS] = { 0 };

// ===== IMPLÉMENTATION =====

SoundManager::SoundManager()
  
  {
    
    currentSoundMode = 0;
    isPlaying = false;
    soundStartTime = 0;
  }

bool SoundManager::begin() {
  if (!audioEngine.begin()) {
    return false;
  }
  DEBUG_INFO_SOUND("Initializing Audio Shield...");
  audioEngine.fxMixer.gain(0, 0.7);
  audioEngine.fxMixer.gain(1, 0.2);
  audioEngine.fxMixer.gain(2, 0);
  audioEngine.fxMixer.gain(3, 0.5);
  audioEngine.dryWetMix.gain(0, 0.3);
  audioEngine.dryWetMix.gain(1, 0.7);
  audioEngine.chorus.begin(delayline,CHORUS_DELAY_LENGTH,8);
  audioEngine.chorus.voices(0);
  audioEngine.lfo.begin(0);
  audioEngine.lfo.amplitude(0);
  audioEngine.moogLfo.amplitude(0);

  setPolyMode(true);
  buildScale(0, 0);
    if (!storage.begin()) {
    Serial.println("✗ Échec initialisation stockage Flash");
  }
  for (int i = 0; i < NUM_PRESETS; i++) {
    presetBank[i] = DEFAULT_PRESETS[i];
  }
  currentPreset = presetBank[0];
  // 5. Créer le SysEx handler
  sysexHandler = new SysExHandler(presetBank, NUM_PRESETS, &currentPreset, &storage);
    
  // 6. Attacher le callback pour appliquer les presets reçus
  sysexHandler->onPresetReceived = [this](Preset* preset) {
    Serial.println("\n🎹 Nouveau preset reçu via SysEx!");
    this->applyPreset(preset);
  };
    // 7. NOUVEAU : Configurer MidiManager avec les callbacks
  MidiManager::setEnabled(true);
  MidiManager::setChannel(1);
  
  // Callback pour SysEx
  MidiManager::onSysEx = [this](byte* data, unsigned int length) {
      if (this->sysexHandler) {
          this->sysexHandler->handleSysEx(data, length);
      }
  };
  
  // Callback pour Control Change (sliders temps réel)
  MidiManager::onControlChange = [this](uint8_t cc, uint8_t value) {
      this->handleControlChange(cc, value);
  };
  
  // Callback pour Note On
  MidiManager::onNoteOn = [this](uint8_t note, uint8_t velocity) {
      this->handleNoteOn(note, velocity);
  };
  
  // Callback pour Note Off
  MidiManager::onNoteOff = [this](uint8_t note, uint8_t velocity) {
      this->handleNoteOff(note, velocity);
  };
  
  // 8. Appliquer le preset initial
  applyPreset(&currentPreset);
  DEBUG_INFO_SOUND("Audio Shield initialized!");
  return true;
}

void SoundManager::update() {
    // Utiliser MidiManager pour traiter tous les messages
    MidiManager::updateConnection();
    MidiManager::read();  // Gère automatiquement SysEx, CC, Notes, etc.
    
    // Votre autre code...
}

void SoundManager::unMute() {
  audioEngine.sgtl5000_1.unmuteHeadphone();
  audioEngine.sgtl5000_1.unmuteLineout();
}

bool SoundManager::isPlayingNotes() {
  bool anyNotePlaying = false;
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    if (activeMidiNotes[i] != 0) {
      anyNotePlaying = true;
      break;
    }
    if (audioEngine.polyEnv[i].isActive()) {
      anyNotePlaying = true;
      break;
    }
  }
  return anyNotePlaying;
//isPlaying = anyNotePlaying;
}

void SoundManager::setPolyMode(bool enabled) {
    polyMode = enabled;
    if (polyMode) {
        //env.noteOff();
        //delay(10);
        //DEBUG_INFO_SOUND("Polyphonic mode ON");
        for (int i = 0; i < 3;i++) {
          audioEngine.polyMixerGlobal.gain(i, POLY_GAIN);
        }
        
        //finalMixer.gain(1, POLY_GAIN);
    } else {
        // for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        //   polyEnv[i].noteOff();
        // } 
        //delay(10);
        for (int i = 0; i < 4;i++) {
          audioEngine.polyMixerGlobal.gain(i, MONO_GAIN);
        }
      //polyenv[0].attack(10);
      // polyenv[0].decay(50);
      // polyenv[0].sustain(0.7);
      // polyenv[0].release(200);
        
        //DEBUG_INFO_SOUND("Polyphonic mode OFF");
        // Couper toutes les notes si on désactive
        
      }

    
}
void SoundManager::setMoogFilterLevel(uint8_t moogLevel) {
    float moogLevelFloat;
    moogLevelFloat = (float)moogLevel / 10.0f;
  float dry = 1 - moogLevelFloat;
  audioEngine.moogMix.gain(0, dry);
  audioEngine.moogMix.gain(1, moogLevelFloat);
}

void SoundManager::setChorusLevel(uint8_t chorusLevel) {
  float chorusLevelFloat;
  chorusLevelFloat = (float)chorusLevel / 10.0f;
  float dry = 1 - chorusLevelFloat;
  audioEngine.fxMixer.gain(0, dry);
  audioEngine.fxMixer.gain(2, chorusLevelFloat);
}

void SoundManager::setTremoloLevel(uint8_t tremoloLevel) {
  float tremoloLevelFloat;
  tremoloLevelFloat = (float)tremoloLevel / 10.0f;
  audioEngine.fxMixer.gain(1, tremoloLevelFloat);
}

void SoundManager::setReverb(float size, float damp) {
  audioEngine.polyReverb.roomsize(size);
  audioEngine.polyReverb.damping(damp);
}

void SoundManager::setDryWetMix(uint8_t mix) {
  float wetFloat;
  wetFloat = (float)mix / 10.0f;
  float dry = 1 - wetFloat;
  audioEngine.dryWetMix.gain(0, wetFloat);
  audioEngine.dryWetMix.gain(1, dry);
}

void SoundManager::setSoundMode(int mode) {
    if (mode < 0 || mode >= NUM_PRESETS) return;
    
    currentSoundMode = mode;
    applyPreset(&presetBank[mode]);
}

void SoundManager::applyPreset(Preset* preset) {
  if (!preset) return;
    //const Preset& p = getPreset(mode);
  DEBUG_VAR_SOUND("Applying preset: ", preset->presetName);
  //const SoundMode &sm = soundModes[currentSoundMode];
  //DEBUG_VAR_SOUND("Sound mode set to: ", sm.name);
  switch (preset->sound.type) {
    case SYNTH:
      SynthParams& p = preset->sound.synth;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyWave[i].begin(p.waveformType);
        audioEngine.polyWave[i].amplitude(p.amplitude);
        audioEngine.polyWave[i].frequencyModulation(p.freqMod);
        audioEngine.polyWave[i].phaseModulation(p.phaseMod);
        audioEngine.polyModFM[i].amplitude(p.modAmplitude);
        audioEngine.polyEnv[i].attack(p.attack);
        audioEngine.polyEnv[i].decay(p.decay);
        audioEngine.polyEnv[i].sustain(p.sustain);
        audioEngine.polyEnv[i].release(p.release);
        audioEngine.polyEnv2[i].attack(p.attack);
        audioEngine.polyEnv2[i].decay(p.decay);
        audioEngine.polyEnv2[i].sustain(p.sustain);
        audioEngine.polyEnv2[i].release(p.release);
        audioEngine.lpFiltre[i].resonance(p.lpfResonance);
      }
        audioEngine.moogEnv.attack(p.moogAttack);
        audioEngine.moogEnv.decay(p.moogDecay);
        audioEngine.moogEnv.sustain(p.moogSustain);
        audioEngine.moogEnv.release(p.moogRelease);
    break;

    case DRUM:
      DrumParams& p = preset->sound.drum;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyDrum[i].length(p.length);
        audioEngine.polyDrum[i].secondMix(p.secondMix);
        audioEngine.polyDrum[i].pitchMod(p.pitchMod);
      }
        audioEngine.moogEnv.attack(p.moogAttack);
        audioEngine.moogEnv.decay(p.moogDecay);
        audioEngine.moogEnv.sustain(p.moogSustain);
        audioEngine.moogEnv.release(p.moogRelease);

      break;

    case STRING:
      StringParams& p = preset->sound.string;
      //monoString.noteOn(60, m.string.velocity); // ex: C4
        audioEngine.moogEnv.attack(p.moogAttack);
        audioEngine.moogEnv.decay(p.moogDecay);
        audioEngine.moogEnv.sustain(p.moogSustain);
        audioEngine.moogEnv.release(p.moogRelease);
      break;
    case SAMPLE:
      SampleParams& p = preset->sound.sample;
        audioEngine.moogEnv.attack(p.moogAttack);
        audioEngine.moogEnv.decay(p.moogDecay);
        audioEngine.moogEnv.sustain(p.moogSustain);
        audioEngine.moogEnv.release(p.moogRelease);
      break;
  }
  EffectsParams& fx = preset->effects;
    audioEngine.polyReverb.roomsize(fx.reverbRoomSize);
    audioEngine.polyReverb.damping(fx.reverbDamping);
    audioEngine.filtre.frequency(fx.filterFreq);
    audioEngine.filtre.resonance(fx.filterResonance);

    if (preset->sound.type != SAMPLE) {
        audioEngine.chorus.voices(fx.nChorus);
    } else {
        audioEngine.chorus.voices(0);  // Désactiver pour SAMPLE
    }
    
    // Tremolo (pas pour SAMPLE)
    if (preset->sound.type != SAMPLE) {
        audioEngine.lfo.begin(fx.tremoloAmplitude, fx.tremoloRate, fx.tremoloWaveform);
    } else {
        audioEngine.lfo.begin(0.0, 0.0, WAVEFORM_SINE);  // Désactiver
    }
    audioEngine.moogLfo.begin(fx.moogLfoAmplitude, fx.moogLfoRate, fx.moogLfoWaveform);

}


void SoundManager::buildScale(int noteIndex, int gammeIndex) {
  //float baseFreq = BASE_FREQUENCIES[noteIndex];
  int intervals[NUM_TOUCH_PADS];
  int octaveOffset = 0;
  switch (gammeIndex) {
    case 0:
    // Gamme majeur
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = MAJOR_INTERVALS[i];
      }
    break;
    case 1:
    // Gamme mineure
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = MINOR_INTERVALS[i];
      }
    break;
    case 2:
    // Gamme chromatique
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = CHROMATIC_INTERVALS[i];
      }
    break;
    case 3:
    // penta majeur
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = PENTATONIC_MAJOR_INTERVALS[i];
      }
    break;
    case 4:
    // penta mineure
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = PENTATONIC_MINOR_INTERVALS[i];
      }
      break;
    case 5:
    // Akebono
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = AKEBONO_INTERVALS[i];
      }
    break;
    case 6:
    //HIJAZ_INTERVALS
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = HIJAZ_INTERVALS[i];
      }
    break;
    case 7:
    //TONE_SCALE
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = TONESCALE_INTERVALS[i];
      }
    break;
    case 8:
    //Diminuée
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = DIMSCALE_INTERVALS[i];
      }
    break;

  }
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    int index = noteIndex + intervals[i];
    if (index > 11 && index < 24) {
      octaveOffset = 1;
    }
    else if (index >= 24) {
      octaveOffset = 2;
    }
    int freqIndex = index % 12;
    scaleFreq[i] = BASE_FREQUENCIES[freqIndex] * (1 << octaveOffset); // multiplier par 2^octaveOffset
  }
  
}


void SoundManager::generateRandomMap() {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        randomMap[i] = i;
    }
    // Shuffle Fisher-Yates
    for (int i = NUM_TOUCH_PADS - 1; i > 0; i--) {
        int j = random(i + 1);
        int temp = randomMap[i];
        randomMap[i] = randomMap[j];
        randomMap[j] = temp;
    }
}


float SoundManager::getNote(int padIndex) {
    switch (keyboardMode) {
        case 0: 
          return scaleFreq[NORMAL_MODE[padIndex]];
        case 1: 
          return scaleFreq[HANDPAN_MODE[padIndex]];
        case 2: 
          return scaleFreq[NUM_TOUCH_PADS - 1 - padIndex];
        case 3: 
          return scaleFreq[randomMap[padIndex]];
        case 4: 
          return scaleFreq[SPIRAL_MODE[padIndex]];
        default:
          DEBUG_VAR_SOUND("Invalid keyboard mode", keyboardMode);
          return 0.0f;

    }
}

void SoundManager::stopMidi() {
  uint8_t channel = MidiManager::getChannel();
  MidiManager::allNotesOff(channel);
}

void SoundManager::stopOneNote(int touchIndex) {
  if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;
  audioEngine.polyEnv[touchIndex].noteOff();
  audioEngine.polyEnv2[touchIndex].noteOff();
  audioEngine.polyString[touchIndex].noteOff(0.5);
  audioEngine.moogEnv.noteOff();
  if (MidiManager::getEnabled() && activeMidiNotes[touchIndex] > 0) {
    MidiManager::noteOff(activeMidiNotes[touchIndex]);
    activeMidiNotes[touchIndex] = 0;
  }
}

void SoundManager::stopAllSounds() {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      audioEngine.polyEnv[i].noteOff();  // release ADSR par note
      audioEngine.polyString[i].noteOff(0.5);
      audioEngine.polyEnv2[i].noteOff();
    }
    audioEngine.moogEnv.noteOff();
    audioEngine.lfo.amplitude(0.0);
    audioEngine.moogLfo.amplitude(0);
    //fxMixer.gain(2, 0);
    if (MidiManager::getEnabled()) {
      updateMidiNotes();
    }
    isPlaying = false;
}


const char* SoundManager::getSoundModeName() const {
  return presetBank[currentSoundMode].sound.name;
}

void SoundManager::volume(uint8_t vol) {
  float volFloat;
  // Conversion en float normalisé 0.0 à 1.0
  volFloat = (float)vol / 10.0f;
  //amp1.gain(volFloat);
  audioEngine.polyAmp.gain(volFloat * 0.8);
  //amp2.gain(volFloat);
  //DEBUG_VAR_SOUND("volume amp1: ", volFloat);
  // DEBUG_PRINT("volume amp1: ");
  // DEBUG_PRINTLN(volFloat);
}

void SoundManager::readVolumePot() {
  int potValue = analogRead(VOL_POT);  // Lecture du pot (0–1023)
  float volume = (potValue / 1023.0) * 0.7;    // Normalisé 0.0 – 1.0
  audioEngine.sgtl5000_1.volume(volume);
}

void SoundManager::testPoly() {
    DEBUG_INFO_SOUND("Testing poly mode...");
    for (int i = 0; i < 4; i++) {  // Test sur 4 notes
        audioEngine.polyWave[i].begin(WAVEFORM_SINE);
        audioEngine.polyWave[i].frequency(440 + i * 100);  // Fréquences différentes
        audioEngine.polyWave[i].amplitude(0.3);
        audioEngine.polyEnv[i].noteOn();
        delay(200);
    }
}


void SoundManager::handleControlChange(uint8_t cc, uint8_t value) {
  switch(cc) {
    case 20: // Reverb size
      presetBank[currentSoundMode].effects.reverbRoomSize = (float)value / 127.0f;
      audioEngine.polyReverb.roomsize(presetBank[currentSoundMode].effects.reverbRoomSize);
      break;
    case 21: // Reverb damping
      presetBank[currentSoundMode].effects.reverbDamping = (float)value / 127.0f;
      audioEngine.polyReverb.damping(presetBank[currentSoundMode].effects.reverbDamping);
      break;
    case 22: // Filter frequency
      presetBank[currentSoundMode].effects.filterFreq = map(value, 0, 127, 100, 20000);
      audioEngine.filtre.frequency(presetBank[currentSoundMode].effects.filterFreq);
      break;
    case 23: // Filter resonance
      presetBank[currentSoundMode].effects.filterResonance = (float)value / 127.0f * 5.0f; // Ajuster l'échelle si nécessaire
      audioEngine.filtre.resonance(presetBank[currentSoundMode].effects.filterResonance);
      break;
    case 24: // Chorus voices
      presetBank[currentSoundMode].effects.nChorus = map(value, 0, 127, 0, 16);
      audioEngine.chorus.voices(presetBank[currentSoundMode].effects.nChorus);
      break;
    case 25: // Tremolo rate
      presetBank[currentSoundMode].effects.tremoloRate = map(value, 0, 127, 1, 20);
      audioEngine.lfo.frequency(presetBank[currentSoundMode].effects.tremoloRate);
      break;
    case 26: // Tremolo amplitude
      presetBank[currentSoundMode].effects.tremoloAmplitude = (float)value / 127.0f;
      audioEngine.lfo.amplitude(presetBank[currentSoundMode].effects.tremoloAmplitude);
      break;
    case 27: // Tremolo Waveform
      presetBank[currentSoundMode].effects.tremoloWaveform = map(value, 0, 127, 0, 6);
      switch(presetBank[currentSoundMode].effects.tremoloWaveform) {
        case 0:
          audioEngine.lfo.begin(WAVEFORM_SINE);
          break;
        case 1:
          audioEngine.lfo.begin(WAVEFORM_SAWTOOTH);
          break;
        case 2:
          audioEngine.lfo.begin(WAVEFORM_SQUARE);
          break;
        case 3:
          audioEngine.lfo.begin(WAVEFORM_TRIANGLE);
          break;
        case 4:
          audioEngine.lfo.begin(WAVEFORM_SAWTOOTH_REVERSE);
          break;
        case 5:
          audioEngine.lfo.begin(WAVEFORM_PULSE);
          break;
        case 6:
          audioEngine.lfo.begin(WAVEFORM_SAMPLE_HOLD);
          break;
      }
      audioEngine.lfo.begin(presetBank[currentSoundMode].effects.tremoloWaveform);
      break;
      
    default:
      DEBUG_VAR_SOUND("Unhandled CC: ", cc);
      break;
  }
}

void SoundManager::updateMidiNotes() {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        if (activeMidiNotes[i] > 0) {
          DEBUG_INFO_SOUND("fin d'enveloppe"); // L'enveloppe s'est arrêtée
          MidiManager::noteOff(activeMidiNotes[i]);
          activeMidiNotes[i] = 0;
            
        }
    }
}

void SoundManager::playMidi(int touchIndex, float frequency, float velocity) {
  uint8_t midiNote = MapToMidi::freqToMidi(frequency);
  uint8_t midiVelocity = MapToMidi::pressureToMidi(velocity);
  //uint8_t channel = MidiManager::getChannel();
  DEBUG_VAR_SOUND("MIDI Note: ", midiNote);
  DEBUG_VAR_SOUND("MIDI Velocity: ", midiVelocity);
  MidiManager::noteOn(midiNote, midiVelocity);
  activeMidiNotes[touchIndex] = midiNote;
}

void SoundManager::setPolyEnv(int attack, int decay, float sustain, int release) {
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    audioEngine.polyEnv[i].attack(attack);
    audioEngine.polyEnv[i].decay(decay);
    audioEngine.polyEnv[i].sustain(sustain);
    audioEngine.polyEnv[i].release(release);
  }
}

void SoundManager::soundDuration(uint16_t durationMS) {
  sampleDuration = durationMS;
  int attack = constrain(durationMS / 10, 10, 100); 
  int decay = constrain(durationMS / 3, 10, 1000);
  int release = constrain(durationMS / 4, 10, 400);
  if (attack + decay + release > durationMS) {
    decay = decay / 2;
    release = release / 2;
  }
  setPolyEnv(attack, decay, 0.5, release);

}

void SoundManager::setArpeggio(bool enabled) {
  arpegioMode = enabled;
}

void SoundManager::playTouchSound(int touchIndex, float pression = 0.4, uint8_t velocite = 150, int octaveShift = 0) {
    if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;
    DEBUG_VAR_SOUND("Playing touch index: ", touchIndex);
    //const SoundMode& sm = presetBank[currentSoundMode];
    float baseFreq = getNote(touchIndex);
    if (octaveShift != 0) {
      baseFreq *= pow(2.0, octaveShift);
    }
    int voiceIndex = polyMode ? touchIndex : 0;
    float minAmplitude = 0.1; // Amplitude minimale audible
    float amplitudeRange = 1.0 - minAmplitude; // 0.7
    
    // Mapper pression sur la plage [minAmplitude, 1.0]
    float effectiveAmplitude = minAmplitude + (amplitudeRange * pression);
    DEBUG_VAR_SOUND("=== AUDIO VALUES ===", "");
    DEBUG_VAR_SOUND("Pressure: ", pression);
    DEBUG_VAR_SOUND("Effective amplitude: ", effectiveAmplitude);

    // Normalisation pression et vélocité (à adapter selon tes valeurs typiques)
    //float normPression = constrain((float)pression / maxPressure, 0.1f, 1.0f); // 550 = pression max typique
    //DEBUG_VAR_SOUND("Norm Pression: ", pression);
    //float normVelocite = constrain((float)velocite / 255.0f, 0.0f, 1.0f);
    //float normVelocite = 1.0f - constrain((float)velocite / (float)maxTime, 0.0f, 1.0f);
    //uint8_t velociteMs = map(velocite, 10, 200, int(maxTime), 10);
    DEBUG_VAR_SOUND("Norm Vélocité: ", velocite);
    audioEngine.lfo.amplitude(presetBank[currentSoundMode].effects.tremoloRate * pression); // LFO amplitude modulée par la pression
    //fxMixer.gain(2, 0.5);
    audioEngine.chorus.voices(presetBank[currentSoundMode].effects.nChorus);
    audioEngine.moogLfo.amplitude(presetBank[currentSoundMode].effects.moogLfoAmplitude * effectiveAmplitude);


    switch(presetBank[currentSoundMode].sound.type) {
      case SYNTH:
      {
        audioEngine.polyWave[voiceIndex].frequency(baseFreq);
        audioEngine.polyModFM[voiceIndex].frequency(baseFreq * presetBank[currentSoundMode].sound.synth.ratioFm);
        audioEngine.polyWave[voiceIndex].amplitude(presetBank[currentSoundMode].sound.synth.amplitude * pression);  // amplitude max pour l’enveloppe
        if (!arpegioMode) {
        // Modulation de l’enveloppe par pression/vélocité
        float sustainMin = 0.1; // Minimum audible
        float sustain = sustainMin + (1.0 - sustainMin) * pression;
        DEBUG_VAR_SOUND("Effective sustain: ", sustain);
        audioEngine.polyEnv[voiceIndex].attack(10 + velocite);   // vélocité rapide = attaque courte
        audioEngine.polyEnv[voiceIndex].decay(200 + 800 * (1.0f - pression));  // pression forte = decay court
        audioEngine.polyEnv[voiceIndex].sustain(sustain);         // pression forte = sustain élevé
        audioEngine.polyEnv2[voiceIndex].attack(10 + velocite);   // vélocité rapide = attaque courte
        audioEngine.polyEnv2[voiceIndex].decay(200 + 800 * (1.0f - pression));  // pression forte = decay court
        audioEngine.polyEnv2[voiceIndex].sustain(sustain);         // pression forte = sustain élevé
        audioEngine.moogEnv.attack(10 + velocite);
        audioEngine.moogEnv.decay(200 + 800 * (1.0f - pression));
        audioEngine.moogEnv.sustain(sustain); 
        }
        audioEngine.polyEnv[voiceIndex].noteOn();
        audioEngine.polyEnv2[voiceIndex].noteOn();
        audioEngine.moogEnv.noteOn();
      }
      break;

      case DRUM:
        audioEngine.polyDrum[voiceIndex].frequency(baseFreq * 0.25);
        audioEngine.polyDrum[voiceIndex].secondMix(presetBank[currentSoundMode].sound.drum.secondMix * effectiveAmplitude); // exemple : moduler le mix par pression
        audioEngine.polyDrum[voiceIndex].noteOn();
        audioEngine.moogEnv.noteOn();
      break;

      case STRING:
      {
        float stringVelocity = 0.5 + 0.5 * pression;
        audioEngine.polyString[voiceIndex].noteOn(baseFreq * 0.5, stringVelocity); // vélocité module l’intensité
        audioEngine.moogEnv.noteOn();
      }
      break;
      case SAMPLE:
        {
        int nMixer = voiceIndex / 4;
        int nchannel = voiceIndex % 4;
        audioEngine.sampleMixer[nMixer].gain(nchannel, sampleLevel * effectiveAmplitude);
        if (!arpegioMode) {        
        audioEngine.polySample[voiceIndex].play(drumSamplesLong[touchIndex]);
        }
        else {
          if (sampleDuration >= 250) {
            audioEngine.polySample[voiceIndex].play(drumSamplesLong[touchIndex]);
          }
          else if (sampleDuration < 250 && sampleDuration >= 50) {
            audioEngine.polySample[voiceIndex].play(drumSamples100ms[touchIndex]);
          }
          else if (sampleDuration < 50) {
            audioEngine.polySample[voiceIndex].play(drumSamples40ms[touchIndex]);
          }

        }
        audioEngine.moogEnv.noteOn();
      }
      break;
    }
  if (MidiManager::getEnabled()) {
    playMidi(touchIndex, baseFreq, pression);
  }
}


#endif
#endif