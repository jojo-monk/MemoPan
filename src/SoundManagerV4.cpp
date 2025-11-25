#include "SoundManagerV4.h"
//#include "audioSamples/DrumsKit1.h"
#include "DrumKitData.h"
#include "MapToMidi.h"
#include "MidiCCMapper.h"
#include "ui.h"
#include "waveforms.h"


MidiManager midiManager;
//MapToMidi mapToMidi;
uint8_t activeMidiNotes[NUM_TOUCH_PADS] = {0};

// ===== Définition des samples =====
// const unsigned int* drumSamplesLong[NUM_TOUCH_PADS] = { 
//     AudioSampleKick,
//     AudioSampleSnare,
//     AudioSampleOpenhh,
//     AudioSampleClosedhh,
//     AudioSampleCowbell,
//     AudioSampleLowcongaw,
//     AudioSampleRimshot,
//     AudioSampleTabla1,
//     AudioSampleTabla2,
//     AudioSampleTabla3,
//     AudioSampleTabla4,
//     AudioSampleTabla5
// };
// const unsigned int* drumSamples100ms[NUM_TOUCH_PADS] = {
//     AudioSampleKick40msw,
//     AudioSampleSnare40msw,
//     AudioSampleOpenhh100msw,
//     AudioSampleClosedhh40msw,
//     AudioSampleCowbell100msw,
//     AudioSampleLowconga100msw,
//     AudioSampleRimshot40msw,
//     AudioSampleTabla1100msw,
//     AudioSampleTabla2100msw,
//     AudioSampleTabla3100msw,
//     AudioSampleTabla4100msw,
//     AudioSampleTabla5100msw
// };
// const unsigned int* drumSamples40ms[NUM_TOUCH_PADS] = {
//     AudioSampleKick40msw,
//     AudioSampleSnare40msw,
//     AudioSampleOpenhh40msw,
//     AudioSampleClosedhh40msw,
//     AudioSampleCowbell40msw,
//     AudioSampleLowconga40msw,
//     AudioSampleRimshot40msw,
//     AudioSampleTabla140msw,
//     AudioSampleTabla240msw,
//     AudioSampleTabla340msw,
//     AudioSampleTabla440msw,
//     AudioSampleTabla540msw
// };

const int16_t* waveTables[NUM_WAVEFORMS] = {
  AKWF_aguitar_0001,
  AKWF_birds_0001,
  AKWF_blended_0001,
  AKWF_granular_0001,
  AKWF_theremin_0001,
  AKWF_vgame_0001,
  AKWF_snippet_0001,
  AKWF_snippet_0001,
  AKWF_sinharm_0001,
  AKWF_saw_0002,
  AKWF_overtone_0001,
  AKWF_dbass_0001,
  AKWF_raw_0001,
  AKWF_hvoice_0001,
  AKWF_fmsynth_0001,
  AKWF_eorgan_0001,
  AKWF_c604_0001,
  AKWF_clavinet_0001

};

// ===== Implémentation =====
SoundManager* SoundManager::instance = nullptr;

SoundManager::SoundManager()
    : audioEngine(),
      presetBank(),
      currentPreset(),
      currentSoundMode(),
      currentDrumKit(),
      soundStartTime(0),
      sysexHandler(nullptr),
      storage(),
      isPlaying(false),
      scaleFreq{0},
      polyMode(true),
      polyMixerGain(0.7f),
      stringLevel(1.0f),
      drumLevel(2.5f),
      sampleLevel(0.7f),
      keyboardMode(0),
      randomMap{0},
      drumMode(false),
      stringMode(false),
      arpegioMode(false),
      maxTime(200.0f),
      maxPressure(1000.0f),
      sampleDuration(500)
  {
    instance = this;
  }

bool SoundManager::begin() {
    if (!audioEngine.begin()) return false;
    DEBUG_INFO_SOUND("Initializing Audio Shield...");
    // Configuration initiale des gains, chorus, lfo, etc...
    audioEngine.fxMixer.gain(0, 0.7);
    audioEngine.fxMixer.gain(1, 0.2);
    audioEngine.fxMixer.gain(2, 0);
    audioEngine.fxMixer.gain(3, 0.5);
    audioEngine.noiseMix.gain(0, 0.8);
    audioEngine.noiseMix.gain(1, 0);
    audioEngine.noiseMix.gain(2, 0);
    audioEngine.noiseMix.gain(3, 0);
    audioEngine.chorus.begin(delayline,CHORUS_DELAY_LENGTH,8);
    audioEngine.chorus.voices(0);
    audioEngine.lfo.begin(0);
    audioEngine.lfo.amplitude(0);
    audioEngine.moogLfo.amplitude(0);
    audioEngine.polyReverb.bypass_setMode(BYPASS_MODE_TRAILS);
    audioEngine.polyReverb.bypass_set(false);
    setPolyMode(true);
    buildScale(0,0);
    storage.begin();
    delay(200);
    //storage.printFSInfo();
    //storage.format(); // pour test uniquement, à retirer plus tard
    //storage.printFSInfo();
    
    if (storage.getPresetCount() == 0) {
      for (int i=0;i<NUM_PRESETS;i++) presetBank[i] = DEFAULT_PRESETS[i];
        DEBUG_INFO_SOUND("Loading default presets into Flash storage...");
      storage.saveAllPresets(presetBank, NUM_PRESETS);
    }
    else {
      DEBUG_INFO_SOUND("Loading presets from storage");
      for (int i=0; i<NUM_PRESETS; i++) {
          storage.loadPreset(i, presetBank[i]);
          DEBUG_VAR_SOUND("Loaded preset: ", presetBank[i].presetName);
      }
    
    }
    loadPresetNamesFromStorage();
    ui_draw();
    currentPreset = presetBank[0];
    sysexHandler = new SysExHandler(presetBank, NUM_PRESETS, &currentPreset, &storage);
    sysexHandler->onPresetReceived = SoundManager::onPresetReceivedStatic;
    MidiManager::setEnabled(true);
    MidiManager::setChannel(1);
MidiManager::onSysEx = [this](byte* data, unsigned int length) {
     sysexHandler->handleSysEx(data, length);
 };

MidiManager::onControlChange = [this](uint8_t cc, uint8_t value) {
    handleControlChange(cc, value);
};

MidiManager::onNoteOn = [this](uint8_t note, uint8_t velocity) {
    // Non utilisé pour l'instant
};

MidiManager::onNoteOff = [this](uint8_t note, uint8_t velocity) {
    // Non utilisé pour l'instant
};
    applyPreset(&currentPreset);
    unMute();
    return true;
}

void SoundManager::update() {
    // Utiliser MidiManager pour traiter tous les messages
    MidiManager::updateConnection();
    MidiManager::read();  // Gère automatiquement SysEx, CC, Notes, etc.
    
}
void SoundManager::loadPresetNamesFromStorage() {

    int loaded = 0;

    for (int i = 0; i < NUM_PRESETS; i++) {
        Preset p;

        if (storage.loadPreset(i, p) &&
            p.presetName != nullptr &&
            strlen(p.presetName) > 0) {

            // Copier le nom dans le tableau fixe
            strncpy(presetNames[i], p.presetName, MAX_PRESET_NAME_LEN - 1);
            presetNames[i][MAX_PRESET_NAME_LEN- 1] = '\0';
            
            Serial.print("  [");
            Serial.print(i);
            Serial.print("] '");
            Serial.print(presetNames[i]);
            Serial.println("' ✓");
            loaded++;

        } else {
            Serial.println("pas de sauvegarde");
            // strncpy(presetNames[i], son[i], MAX_PRESET_NAME_LEN - 1);
            // presetNames[i][MAX_PRESET_NAME_LEN - 1] = '\0';

            // Serial.print("  [");
            // Serial.print(i);
            // Serial.print("] '");
            // Serial.print(presetNames[i]);
            // Serial.println("' (default)");
            // defaults++;
        }
    }

    presetNamesLoaded = true;
}



void SoundManager::onPresetReceivedStatic(Preset* preset) {
    if(instance) {
        instance->applyPreset(preset);
    }
}

void SoundManager::analyseAudio() {
  if (audioEngine.peakL.available() && audioEngine.peakR.available()) {
    float peakLevelL = audioEngine.peakL.read();   // Valeur de 0.0 à 1.0
    float peakLevelR = audioEngine.peakR.read();
    float rmsLevelL = audioEngine.rmsL.read();
    float rmsLevelR = audioEngine.rmsR.read();     // Valeur de 0.0 à 1.0
    const int maxBar = 50; // largeur max de la barre
    int barL = peakLevelL * maxBar;
    int barR = peakLevelR * maxBar;

    // Serial.print("L: ");
    // for (int i = 0; i < barL; i++) Serial.print("█");
    // for (int i = barL; i < maxBar; i++) Serial.print(" "); // espace pour compléter la barre

    // Serial.print("  R: ");
    // for (int i = 0; i < barR; i++) Serial.print("█");
    // for (int i = barR; i < maxBar; i++) Serial.print(" ");


    Serial.println();
    Serial.printf("L: Peak %.3f RMS %.3f | R: Peak %.3f RMS %.3f\n",
              peakLevelL, rmsLevelL, peakLevelR, rmsLevelR);

    // Exemple : si le peak dépasse 1.0 → clipping
    if (peakLevelL > 1.0f or peakLevelR > 1.0f) {
      Serial.println("⚠️  Clipping detected!");
    }
  }
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
}

void SoundManager::setPolyMode(bool enabled) {
    polyMode = enabled;
    if (polyMode) {
        //DEBUG_INFO_SOUND("Polyphonic mode ON");
        for (int i = 0; i < 3;i++) {
          audioEngine.polyMixerGlobal.gain(i, POLY_GAIN);
        }
    } else {
        for (int i = 0; i < 4;i++) {
          audioEngine.polyMixerGlobal.gain(i, MONO_GAIN);
        }
        //DEBUG_INFO_SOUND("Polyphonic mode OFF");
        
      }

    
}

void SoundManager::setNoiseEffect(Preset* preset) {
    // Configuration initiale du générateur de bruit
    EffectsParams& fx = preset->effects;
    if (fx.noiseAmplitude == 0.0f) {
      audioEngine.noiseDc.amplitude(0);
      audioEngine.whiteNoise.amplitude(fx.noiseAmplitude);
      audioEngine.noiseMix.gain(1, 0); 
      audioEngine.noiseMix.gain(2, 0);
      audioEngine.noiseMix.gain(3, 0);
      return;
    }
    audioEngine.noiseDc.amplitude(1);
    audioEngine.whiteNoise.amplitude(fx.noiseAmplitude);
    audioEngine.noiseEnv.attack(fx.noiseEnvAttack);
    audioEngine.noiseEnv.decay(fx.noiseEnvDecay);
    audioEngine.noiseEnv.sustain(fx.noiseEnvSustain);
    audioEngine.noiseEnv.release(fx.noiseEnvRelease);
    audioEngine.noiseCutOffEnv.attack(fx.noiseEnvAttack * fx.noiseCutoffEnvAmount);
    audioEngine.noiseCutOffEnv.decay(fx.noiseEnvDecay * fx.noiseCutoffEnvAmount);
    audioEngine.noiseCutOffEnv.sustain(fx.noiseEnvSustain);
    audioEngine.noiseCutOffEnv.release(fx.noiseEnvRelease * fx.noiseCutoffEnvAmount);
    audioEngine.noiseFilter.frequency(fx.noiseCutoff);
    audioEngine.noiseFilter.resonance(fx.noiseResonance);
    audioEngine.noiseFilter.octaveControl(fx.noiseOctaveControl); // lowpass par défaut
    setNoiseFilter(fx.noiseFilterType, ui_getNoise());
}

void SoundManager::setNoiseFilter(uint8_t filterType, uint8_t noiseLevel) {
  float noiseLevelFloat = ((float)noiseLevel / 10.0f) * 0.8; // 0 à 0.6

  switch (filterType) {
      case 0:
        audioEngine.noiseMix.gain(1, noiseLevelFloat); 
        audioEngine.noiseMix.gain(2, 0);
        audioEngine.noiseMix.gain(3, 0); // lowpass
        break;
      case 1:
        audioEngine.noiseMix.gain(1, 0); 
        audioEngine.noiseMix.gain(2, noiseLevelFloat);
        audioEngine.noiseMix.gain(3, 0);// bandpass
        break;
      case 2:
        audioEngine.noiseMix.gain(1, 0); 
        audioEngine.noiseMix.gain(2, 0);
        audioEngine.noiseMix.gain(3, noiseLevelFloat); // highpass
        break;
    }
}


void SoundManager::updateFxLevels() {
  float chorus = (float)currentChorusLevel / 10.0f;
  float tremolo = (float)currentTremoloLevel / 10.0f;
  DEBUG_VAR_SOUND("  currentTremolo level: ", currentTremoloLevel);
  DEBUG_VAR_SOUND("  currentChorus level: ", currentChorusLevel);
  // courbes légèrement logarithmiques pour plus de naturel
  float chorusWet = 0.05f + 1.0f * powf(chorus, 0.8f);
  float tremoloWet = 0.05f + 1.0f * powf(tremolo, 0.8f);

  // dry baisse légèrement si plusieurs effets actifs
  float dry = 2.0f - 0.7f * chorus - 0.6f * tremolo;
  if (dry < 0.05f) dry = 0.1f; // minimum pour ne jamais étouffer le son

  // Normalisation pour éviter clipping
  float total = dry + chorusWet + tremoloWet;
  if (total > 2.0f) {
    float scale = 2.0f / total;
    dry *= scale;
    chorusWet *= scale;
    tremoloWet *= scale;
  }

  audioEngine.fxMixer.gain(0, dry);
  audioEngine.fxMixer.gain(1, tremoloWet);
  audioEngine.fxMixer.gain(2, chorusWet);
  DEBUG_INFO_SOUND("Updated FX levels:");
  DEBUG_VAR_SOUND("  Dry level: ", dry);
  DEBUG_VAR_SOUND("  Tremolo level: ", tremoloWet);
  DEBUG_VAR_SOUND("  Chorus level: ", chorusWet);

}

void SoundManager::setChorusLevel(uint8_t chorusLevel) {
  currentChorusLevel = chorusLevel;
  updateFxLevels();
}

void SoundManager::setTremoloLevel(uint8_t tremoloLevel) {
  currentTremoloLevel = tremoloLevel;
  updateFxLevels();
}


void SoundManager::setReverb(float size, float damp) {
  audioEngine.polyReverb.size(size);
  audioEngine.polyReverb.diffusion(damp);
}

void SoundManager::setDryWetMix(uint8_t mix) {
  float wetFloat = (float)mix / 10.0f;
  if (mix == 0.0f) {
    Serial.println("reverb bypass");
    audioEngine.polyReverb.bypass_set(true);
  }
  audioEngine.polyReverb.bypass_set(false);
  audioEngine.polyReverb.mix(wetFloat);
}

void SoundManager::setNoiseLevel(uint8_t mix) {
  float mixF = ((float)mix / 10.0f) * 0.8;
  uint8_t fType = presetBank[currentSoundMode].effects.noiseFilterType;
  setNoiseFilter(fType, mixF);
}

void SoundManager::setSoundMode(int mode) {
    if (mode < 0 || mode >= NUM_PRESETS) return;
    currentSoundMode = mode;
    // Serial.print("currentSoundMode SetSoundMode: ");
    // Serial.println(currentSoundMode);
    applyPreset(&presetBank[mode]);
}

void SoundManager::setKeyboardMode(int mode) { 
    keyboardMode = mode; 
  }

void SoundManager::setLfoEnv(int att, int dec, float sust, int rel) {
  audioEngine.moogLfoEnv.attack(att);
  audioEngine.moogLfoEnv.decay(dec);
  audioEngine.moogLfoEnv.sustain(sust);
  audioEngine.moogLfoEnv.release(rel);
  audioEngine.tremoloEnv.attack(att);
  audioEngine.tremoloEnv.decay(dec);
  audioEngine.tremoloEnv.sustain(sust);
  audioEngine.tremoloEnv.release(rel);
}


void SoundManager::applyPreset(Preset* preset) {
  if (!preset) {
    Serial.println("erreur de preset");
    return;
  }
    //const Preset& p = getPreset(mode);
  // Serial.println("=== APPLYING PRESET ===");
  // Serial.print("Preset name: ");
  // Serial.println(preset->presetName);
  // Serial.print("Sound type: ");
  // Serial.println(preset->sound.type);
  DEBUG_VAR_SOUND("Sound type: ", preset->sound.type);
  if (currentSoundMode != preset->soundTypeIndex) {
    currentSoundMode = preset->soundTypeIndex;
  }
  //bool moogActive = false;
  EffectsParams& fx = preset->effects;
  // if (fx.moogCutoff > 0.0f) {
  //   moogActive = true;
  // }
  switch (preset->sound.type) {
    case SYNTH:
        {
        SynthParams& p = preset->sound.synth;
        for (int i = 0; i < NUM_TOUCH_PADS; i++) {
            audioEngine.polyWave[i].begin(p.waveformType);
            if (p.waveformType == WAVEFORM_ARBITRARY) {
              audioEngine.polyWave[i].arbitraryWaveform(waveTables[p.wavetable], 172);
            }
            audioEngine.polyWave[i].amplitude(p.amplitude);
            audioEngine.polyWave[i].frequencyModulation(p.freqMod);
            audioEngine.polyWave[i].phaseModulation(p.phaseMod);
            audioEngine.polyModFM[i].begin(p.FMWaveform);
            if (p.FMWaveform == WAVEFORM_ARBITRARY) {
              audioEngine.polyModFM[i].arbitraryWaveform(waveTables[p.FMwavetable], 172);
            }
            audioEngine.polyModFM[i].amplitude(p.modAmplitude);
            audioEngine.polyEnv[i].attack(p.attack);
            audioEngine.polyEnv[i].decay(p.decay);
            audioEngine.polyEnv[i].sustain(p.sustain);
            audioEngine.polyEnv[i].release(p.release);
            audioEngine.polyEnv2[i].attack(p.attack * p.lpfEnvAmount);
            audioEngine.polyEnv2[i].decay(p.decay * p.lpfEnvAmount);
            audioEngine.polyEnv2[i].sustain(p.sustain);
            audioEngine.polyEnv2[i].release(p.release * p.lpfEnvAmount);
            audioEngine.lpFiltre[i].resonance(p.lpfResonance);
            audioEngine.lpFiltre[i].frequency(p.lpfCutoff);
            audioEngine.lpFiltre[i].octaveControl(p.lpfOctaveControl); // lowpass par défaut
            audioEngine.polyEnv3[i].attack(p.attack * p.FMEnvAmount);
            audioEngine.polyEnv3[i].decay(p.decay * p.FMEnvAmount);
            audioEngine.polyEnv3[i].sustain(p.sustain);
            audioEngine.polyEnv3[i].release(p.release * p.FMEnvAmount);
          }
          // if (moogActive) {
            audioEngine.moogEnv.attack(p.moogAttack);
            audioEngine.moogEnv.decay(p.moogDecay);
            audioEngine.moogEnv.sustain(p.moogSustain);
            audioEngine.moogEnv.release(p.moogRelease);
          // }
            ui_setOctaveShift(p.octaveShift - 2);
          setLfoEnv(p.attack, p.decay, p.sustain, p.release);
        }
    break;

    case DRUM:
    {
      DrumParams& p = preset->sound.drum;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyDrum[i].length(p.length);
        audioEngine.polyDrum[i].secondMix(p.secondMix);
        audioEngine.polyDrum[i].pitchMod(p.pitchMod);
      }
      // if (moogActive) {
        audioEngine.moogEnv.attack(p.moogAttack);
        audioEngine.moogEnv.decay(p.moogDecay);
        audioEngine.moogEnv.sustain(p.moogSustain);
        audioEngine.moogEnv.release(p.moogRelease);
      // }
        ui_setOctaveShift(p.octaveShift - 2);
        setLfoEnv(p.moogAttack, p.moogDecay, p.moogSustain, p.moogRelease);
    }
      break;

    case STRING:
    {
      StringParams& p = preset->sound.string;
      //monoString.noteOn(60, m.string.velocity); // ex: C4
      // if (moogActive) {
        audioEngine.moogEnv.attack(p.moogAttack);
        audioEngine.moogEnv.decay(p.moogDecay);
        audioEngine.moogEnv.sustain(p.moogSustain);
        audioEngine.moogEnv.release(p.moogRelease);
      // }
        ui_setOctaveShift(p.octaveShift - 2);
      setLfoEnv(p.moogAttack, p.moogDecay, p.moogSustain, p.moogRelease);
    }
      break;
    case SAMPLE:
    {
      SampleParams& p = preset->sound.sample;
      // if (moogActive) {
        audioEngine.moogEnv.attack(p.moogAttack);
        audioEngine.moogEnv.decay(p.moogDecay);
        audioEngine.moogEnv.sustain(p.moogSustain);
        audioEngine.moogEnv.release(p.moogRelease);
      // }
        ui_setOctaveShift(0);
        setLfoEnv(p.moogAttack, p.moogDecay, p.moogSustain, p.moogRelease);
        currentDrumKit = p.drumKitId;
    }
      break;
  }
    audioEngine.polyReverb.size(fx.reverbRoomSize);
    audioEngine.polyReverb.diffusion(fx.reverbDamping);
    audioEngine.polyReverb.lowpass(fx.filterFreq);
    audioEngine.polyReverb.hipass(fx.filterResonance);
    //audioEngine.filtre.frequency(fx.filterFreq);
    //audioEngine.filtre.resonance(fx.filterResonance);
    // if (moogActive) {
    audioEngine.moogFilter.frequency(fx.moogCutoff);
    audioEngine.moogFilter.resonance(fx.moogResonance);
    audioEngine.moogFilter.octaveControl(fx.moogOctaveControl);
    audioEngine.moogLfo.begin(fx.moogLfoAmplitude, fx.moogLfoRate, fx.moogLfoWaveform);
    // }
    // else {
      // audioEngine.moogFilter.frequency(0);
      // audioEngine.moogLfo.begin(0.0, 0.0, WAVEFORM_SINE); // désactiver
    // }

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
    
    //DEBUG_VAR_SOUND("  1currentTremolo level: ", currentTremoloLevel);
    //DEBUG_VAR_SOUND("  2currentChorus level: ", currentChorusLevel);
    setNoiseEffect(preset);
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
  activesNotes[touchIndex] = false;
  if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;
  audioEngine.polyEnv[touchIndex].noteOff();
  audioEngine.polyEnv2[touchIndex].noteOff();
  audioEngine.polyString[touchIndex].noteOff(0.5);
  audioEngine.noiseEnv.noteOff();
  audioEngine.moogEnv.noteOff();
  audioEngine.moogLfoEnv.noteOff();
  audioEngine.tremoloEnv.noteOff();
  audioEngine.whiteNoise.amplitude(0);
  //audioEngine.polyReverb.bypass_set(true);
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
      audioEngine.polyEnv3[i].noteOff();
      activesNotes[i] = false;
    }
    audioEngine.moogEnv.noteOff();
    audioEngine.lfo.amplitude(0.0);
    audioEngine.moogLfo.amplitude(0);
    audioEngine.whiteNoise.amplitude(0);
    //audioEngine.polyReverb.bypass_set(true);
    //fxMixer.gain(2, 0);
    if (MidiManager::getEnabled()) {
      updateMidiNotes();
    }
    isPlaying = false;
}


uint8_t SoundManager::getNumActiveVoices() {
    uint8_t count = 0;
    for(int i=0; i<NUM_TOUCH_PADS; i++) if(activesNotes[i]) count++;
    return count;
}

const char* SoundManager::getSoundModeName() const {
  return presetBank[currentSoundMode].sound.name;
}

void SoundManager::volume(uint8_t vol) {
  float volFloat;
  // Conversion en float normalisé 0.0 à 1.0
  volFloat = (float)vol / 10.0f;
  //amp1.gain(volFloat);
  audioEngine.polyAmp.setGain(volFloat);
  //audioEngine.polyAmpLeft.gain(volFloat * 0.9);
  //audioEngine.polyAmpRight.gain(volFloat * 0.9);
  //amp2.gain(volFloat);
  //DEBUG_VAR_SOUND("volume amp1: ", volFloat);
  // DEBUG_PRINT("volume amp1: ");
  // DEBUG_PRINTLN(volFloat);
}

void SoundManager::readVolumePot() {
  int potValue = analogRead(VOL_POT);  // Lecture du pot (0–1023)
  float volume = (potValue / 1023.0);    // Normalisé 0.0 – 1.0
  audioEngine.sgtl5000_1.volume(volume * 0.9);
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
  auto& preset = presetBank[currentSoundMode];
  auto& sound  = preset.sound;
  auto& fx     = preset.effects;
  // // --- Déterminer automatiquement le type selon le mode ---
  // if (currentSoundMode == 29)      sound.type = STRING;
  // else if (currentSoundMode == 30) sound.type = DRUM;
  // else if (currentSoundMode == 31) sound.type = SAMPLE;
  // else                             sound.type = SYNTH;

  // --- Déléguer au bon gestionnaire ---
  switch (currentSoundMode) {
    case 29:
      handleStringControlChange(cc, value, sound.string, fx);
      break;
    case 30:
      handleDrumControlChange(cc, value, sound.drum, fx);
      break;
    case 31:
      handleSampleControlChange(cc, value, sound.sample, fx);
      break;
    default:
      handleSynthControlChange(cc, value, sound.synth, fx);
    break;
  }
}



void SoundManager::handleSynthControlChange(uint8_t cc, uint8_t value, SynthParams& synth, EffectsParams& fx) {
  float normValue = MidiCCMapper::convertCC(cc, value);

  switch (cc) {
    case 20: // Waveform
      synth.waveformType = value;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyWave[i].begin(synth.waveformType);
      }
    break;
    case 23: //amplitude
      synth.amplitude = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyWave[i].amplitude(synth.amplitude);
      }
    break;
    case 24: //attack
      synth.attack = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyEnv[i].attack(synth.attack);
        audioEngine.polyEnv2[i].attack(synth.attack);

      }
    break;
    case 25: //decay
      synth.decay = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyEnv[i].decay(synth.decay);
        audioEngine.polyEnv2[i].decay(synth.decay);
      }
    break;
    case 26:
      synth.sustain = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyEnv[i].sustain(synth.sustain);
        audioEngine.polyEnv2[i].sustain(synth.sustain);
      }
    break;
    case 27:
      synth.release = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyEnv[i].release(synth.release);
        audioEngine.polyEnv2[i].release(synth.release);
      }
    break;
    case 28:
      synth.freqMod = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyWave[i].frequencyModulation(synth.freqMod);
      }
    break;
    case 29:
      synth.phaseMod = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyWave[i].phaseModulation(synth.phaseMod);
      }
    break;
    case 30:
      synth.ratioFm = normValue;
    break;
    case 31:
      synth.modAmplitude = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyModFM[i].amplitude(synth.modAmplitude);
      }
    break;
    case 34:
      synth.moogAttack = normValue;
      audioEngine.moogEnv.attack(synth.moogAttack);
    break;
    case 35:
      synth.moogDecay = normValue;
      audioEngine.moogEnv.decay(synth.moogDecay);
    break;
    case 36:
      synth.moogSustain = normValue;
      audioEngine.moogEnv.sustain(synth.moogSustain);
    break;
    case 37: // Moog Release
      synth.moogRelease = normValue;
      audioEngine.moogEnv.release(synth.moogRelease);
      break;

    case 39: // LPF Resonance
      synth.lpfResonance = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.lpFiltre[i].resonance(synth.lpfResonance);
      }
      break;
    case 49: // OctaveShift
      synth.octaveShift = value; 
      ui_setOctaveShift(synth.octaveShift - 2);
      break;
    case 50: // lpfCutOff
      synth.lpfCutoff = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.lpFiltre[i].frequency(synth.lpfCutoff);
      }
      break;
    case 51: // lpfEnvAmount
      synth.lpfEnvAmount = normValue;
      break;
    case 52: // FMEnvAmount
      synth.FMEnvAmount = normValue;
      break;
    case 53: // FM Waveform
      synth.FMWaveform = value;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyModFM[i].begin(synth.FMWaveform);
      }
      break;
    case 54: // lpfOctaveControl
      synth.lpfOctaveControl = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.lpFiltre[i].octaveControl(synth.lpfOctaveControl);
      }
      break;
    case 55: // FMwavetable
      synth.FMwavetable = value;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyModFM[i].arbitraryWaveform(waveTables[synth.FMwavetable], 170);
      }
      break;
    case 56:
      synth.wavetable = value;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyWave[i].arbitraryWaveform(waveTables[synth.wavetable], 170);
      }
      break;
    default:
      handleCommonEffects(cc, value, fx);
      break;
  }
}

void SoundManager::handleStringControlChange(uint8_t cc, uint8_t value, StringParams& string, EffectsParams& fx) {
  float normValue = MidiCCMapper::convertCC(cc, value);

  switch (cc) {
    case 70: // String Velocity
      string.velocity = normValue;
    break;
    case 34:
      string.moogAttack = normValue;
      audioEngine.moogEnv.attack(string.moogAttack);
    break;
    case 35:
      string.moogDecay = normValue;
      audioEngine.moogEnv.decay(string.moogDecay);
    break;
    case 36:
      string.moogSustain = normValue;
      audioEngine.moogEnv.sustain(string.moogSustain);
    break;
    case 37: // Moog Release
      string.moogRelease = normValue;
      audioEngine.moogEnv.release(string.moogRelease);
      break;
    case 49: // OctaveShift
      string.octaveShift = value; 
      ui_setOctaveShift(string.octaveShift - 2);
      break;
    default:
      handleCommonEffects(cc, value, fx);
      break;
  }
}

void SoundManager::handleDrumControlChange(uint8_t cc, uint8_t value, DrumParams& drum, EffectsParams& fx) {
  float normValue = MidiCCMapper::convertCC(cc, value);

  switch (cc) {
    case 60: // Length
      drum.length = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyDrum[i].length(drum.length);
      }
    break;
    case 61:
      drum.secondMix = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyDrum[i].secondMix(drum.secondMix);
      }
    break;
    case 62:
      drum.pitchMod = normValue;
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        audioEngine.polyDrum[i].pitchMod(drum.pitchMod);
      }
    break;
    case 34:
      drum.moogAttack = normValue;
      audioEngine.moogEnv.attack(drum.moogAttack);
    break;
    case 35:
      drum.moogDecay = normValue;
      audioEngine.moogEnv.decay(drum.moogDecay);
    break;
    case 36:
      drum.moogSustain = normValue;
      audioEngine.moogEnv.sustain(drum.moogSustain);
    break;
    case 37: // Moog Release
      drum.moogRelease = normValue;
      audioEngine.moogEnv.release(drum.moogRelease);
      break;
    case 49: // OctaveShift
      drum.octaveShift = value; 
      ui_setOctaveShift(drum.octaveShift - 2);
      break;
    default:
      handleCommonEffects(cc, value, fx);
      break;
  }
}

void SoundManager::handleSampleControlChange(uint8_t cc, uint8_t value, SampleParams& sample, EffectsParams& fx) {
  float normValue = MidiCCMapper::convertCC(cc, value);

  switch (cc) {
    case 34:
      sample.moogAttack = normValue;
      audioEngine.moogEnv.attack(sample.moogAttack);
    break;
    case 35:
      sample.moogDecay = normValue;
      audioEngine.moogEnv.decay(sample.moogDecay);
    break;
    case 36:
      sample.moogSustain = normValue;
      audioEngine.moogEnv.sustain(sample.moogSustain);
    break;
    case 37: // Moog Release
      sample.moogRelease = normValue;
      audioEngine.moogEnv.release(sample.moogRelease);
      break;
    default:
      handleCommonEffects(cc, value, fx);
      break;
  }
}

void SoundManager::handleCommonEffects(uint8_t cc, uint8_t value, EffectsParams& fx) {
  float normValue = MidiCCMapper::convertCC(cc, value);
  switch (cc) {
    case 21:  // tremolo Waveform
      fx.tremoloWaveform = value;
      audioEngine.lfo.begin(fx.tremoloWaveform);
    break;
    case 22:
      fx.moogLfoWaveform = value;
      audioEngine.moogLfo.begin(fx.moogLfoWaveform);
    break;
    case 40: // reverb roomSize
      fx.reverbRoomSize = normValue;
      audioEngine.polyReverb.size(fx.reverbRoomSize);
    break;
    case 41: // reverb damping
      fx.reverbDamping = normValue;
      audioEngine.polyReverb.diffusion(fx.reverbDamping);
    break;
    case 42: // reverb filter frequency
      fx.filterFreq = normValue;
      //audioEngine.filtre.frequency(fx.filterFreq);
      audioEngine.polyReverb.lowpass(fx.filterFreq);
    break;
    case 43: // reverb filter resonance
      fx.filterResonance = normValue;
      //audioEngine.filtre.resonance(fx.filterResonance);
      audioEngine.polyReverb.hipass(fx.filterResonance);
    break;
    case 44: // chorus voices
      fx.nChorus = value;
      audioEngine.chorus.voices(fx.nChorus);
    break;
    case 45: // tremolo rate
      fx.tremoloRate = normValue;
      audioEngine.lfo.frequency(fx.tremoloRate);
    break;
    case 46: // tremolo amplitude
      fx.tremoloAmplitude = normValue;
      audioEngine.lfo.amplitude(fx.tremoloAmplitude);
    break;
    case 47: // Moog lfo rate
      fx.moogLfoRate = normValue;
      audioEngine.moogLfo.frequency(fx.moogLfoRate);
    break;
    case 48: // Moog lfo amplitude
      fx.moogLfoAmplitude = normValue;
      audioEngine.moogLfo.amplitude(fx.moogLfoAmplitude);
    break;
    case 80: // noiseAmplitude
      fx.noiseAmplitude = normValue;
      audioEngine.whiteNoise.amplitude(fx.noiseAmplitude);
    break;
    case 81: //noiseCutoff
      fx.noiseCutoff = normValue;
      audioEngine.noiseFilter.frequency(fx.noiseCutoff);
    break;
    case 82:// noiseResonance
      fx.noiseResonance = normValue;
      audioEngine.noiseFilter.resonance(fx.noiseResonance);
      break;
    case 83: // noiseEnvAttack
      fx.noiseEnvAttack = normValue;
      audioEngine.noiseEnv.attack(fx.noiseEnvAttack);
      audioEngine.noiseCutOffEnv.attack(fx.noiseEnvAttack * fx.noiseCutoffEnvAmount);
      break;
    case 84: // noiseEnvDecay
      fx.noiseEnvDecay = normValue;
      audioEngine.noiseEnv.decay(fx.noiseEnvDecay);
      audioEngine.noiseCutOffEnv.decay(fx.noiseEnvDecay * fx.noiseCutoffEnvAmount);
      break;
    case 85: // noiseEnvSustain
      fx.noiseEnvSustain = normValue;
      audioEngine.noiseEnv.sustain(fx.noiseEnvSustain);
      audioEngine.noiseCutOffEnv.sustain(fx.noiseEnvSustain);
      break;
    case 86: // noiseEnvRelease
      fx.noiseEnvRelease = normValue;
      audioEngine.noiseEnv.release(fx.noiseEnvRelease);
      audioEngine.noiseCutOffEnv.release(fx.noiseEnvRelease * fx.noiseCutoffEnvAmount);
      break;
    case 87: // noiseCutoffEnvAmount
      fx.noiseCutoffEnvAmount = normValue;
      audioEngine.noiseCutOffEnv.attack(fx.noiseEnvAttack * fx.noiseCutoffEnvAmount);
      audioEngine.noiseCutOffEnv.decay(fx.noiseEnvDecay * fx.noiseCutoffEnvAmount);
      audioEngine.noiseCutOffEnv.release(fx.noiseEnvRelease * fx.noiseCutoffEnvAmount);
      break;
    case 88: // noiseFilterType
      fx.noiseFilterType = value;
      setNoiseFilter(fx.noiseFilterType, ui_getNoise());
      break;
    case 89: // noiseOctaveControl
      fx.noiseOctaveControl = normValue;
      audioEngine.noiseFilter.octaveControl(fx.noiseOctaveControl);
      break;
    case 90: // moogCutoff
      fx.moogCutoff = normValue;
      audioEngine.moogFilter.frequency(fx.moogCutoff);
      break;
    case 91: // moogResonance
      fx.moogResonance = normValue;
      audioEngine.moogFilter.resonance(fx.moogResonance);
      break;
    case 92: // moogOctaveControl
      fx.moogOctaveControl = normValue;
      audioEngine.moogFilter.octaveControl(fx.moogOctaveControl);
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
    audioEngine.polyEnv2[i].attack(attack);
    audioEngine.polyEnv2[i].decay(decay);
    audioEngine.polyEnv2[i].sustain(sustain);
    audioEngine.polyEnv2[i].release(release);
    audioEngine.polyEnv3[i].attack(attack);
    audioEngine.polyEnv3[i].decay(decay);
    audioEngine.polyEnv3[i].sustain(sustain);
    audioEngine.polyEnv3[i].release(release);
  }
  audioEngine.moogEnv.attack(attack);
  audioEngine.moogEnv.decay(decay);
  audioEngine.moogEnv.sustain(sustain);
  audioEngine.moogEnv.release(release);
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

void SoundManager::playTouchSound(int touchIndex, float pression, uint8_t velocite, int octaveShift) {
    if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;
    DEBUG_VAR_SOUND("Playing touch index: ", touchIndex);
    //const SoundMode& sm = presetBank[currentSoundMode];
    activesNotes[touchIndex] = true;
    int activeVoices = getNumActiveVoices();
    if (activeVoices == 0) activeVoices = 1; // éviter division par zéro
    float autolevelFactor = 1.0 / sqrt(activeVoices);
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
    float ratiolpfEnv = presetBank[currentSoundMode].sound.synth.lpfEnvAmount;
    float ratioFMEnv = presetBank[currentSoundMode].sound.synth.FMEnvAmount;
    float moogAtt = presetBank[currentSoundMode].sound.synth.moogAttack;
    float moogDec = presetBank[currentSoundMode].sound.synth.moogDecay;
    float moogSust = presetBank[currentSoundMode].sound.synth.moogSustain;
    float moogRel = presetBank[currentSoundMode].sound.synth.moogRelease;
    float noiseAtt = presetBank[currentSoundMode].effects.noiseEnvAttack;
    float noiseDec = presetBank[currentSoundMode].effects.noiseEnvDecay;
    float noiseSust = presetBank[currentSoundMode].effects.noiseEnvSustain;
    float noiseRel = presetBank[currentSoundMode].effects.noiseEnvRelease;
    float sustain = presetBank[currentSoundMode].sound.synth.sustain;
    //float noiseRatio = presetBank[currentSoundMode].effects.noiseCutoffEnvAmount;
    float rel = presetBank[currentSoundMode].sound.synth.release;
    float att = presetBank[currentSoundMode].sound.synth.attack;
    float att2 = att * ratiolpfEnv;
    float dec = presetBank[currentSoundMode].sound.synth.decay;
    float dec2 = dec * ratiolpfEnv;
    float rel2 = rel * ratiolpfEnv;
    float att3 = att * ratioFMEnv;
    float dec3 = dec * ratioFMEnv;
    float rel3 = rel * ratioFMEnv;
    float normVelocite = 1.0f - (float)(velocite - 5) / 295.0f;
    float sustainMod = sustain + (1.0f - sustain) * (pression * 0.3f);
    float releaseMod = rel * (1.0f + pression * 0.4f);
    float releaseMod2 = rel2 * (1.0f + pression * 0.4f);
    float releaseMod3 = rel3 * (1.0f + pression * 0.4f);
    
    
    switch(presetBank[currentSoundMode].sound.type) {
      case SYNTH:
      {
        audioEngine.polyWave[voiceIndex].frequency(baseFreq);
        audioEngine.polyModFM[voiceIndex].frequency(baseFreq * presetBank[currentSoundMode].sound.synth.ratioFm);
        audioEngine.polyWave[voiceIndex].amplitude(presetBank[currentSoundMode].sound.synth.amplitude * 
          pression * autolevelFactor);  // amplitude max pour l’enveloppe
        if (!arpegioMode) {
        // Modulation de l’enveloppe par pression/vélocité
        DEBUG_VAR_SOUND("Effective sustain: ", sustain);
        audioEngine.polyEnv[voiceIndex].attack(att * (1.0f - normVelocite * 0.8f));   // vélocité rapide = attaque courte
        audioEngine.polyEnv[voiceIndex].decay(dec * (1.0f - pression * 0.3f));  // pression forte = decay court
        audioEngine.polyEnv[voiceIndex].sustain(sustainMod);         // pression forte = sustain élevé
        audioEngine.polyEnv[voiceIndex].release(releaseMod);
        audioEngine.polyEnv2[voiceIndex].attack(att2 - velocite * 0.5);   // vélocité rapide = attaque courte
        audioEngine.polyEnv2[voiceIndex].decay(dec2 * (1.0f - pression * 0.5f));  // pression forte = decay court
        audioEngine.polyEnv2[voiceIndex].sustain(sustainMod);         // pression forte = sustain élevé
        audioEngine.polyEnv2[voiceIndex].release(releaseMod2);
        audioEngine.polyEnv3[voiceIndex].attack(att3 * (1.0f - normVelocite * 0.8f));   // vélocité rapide = attaque courte
        audioEngine.polyEnv3[voiceIndex].decay(dec3 * (1.0f - pression * 0.3f));  // pression forte = decay court
        audioEngine.polyEnv3[voiceIndex].sustain(sustainMod);         // pression forte = sustain élevé
        audioEngine.polyEnv3[voiceIndex].release(releaseMod3);
        setLfoEnv(att * (1.0f - normVelocite * 0.8f), dec * (1.0f - pression * 0.3f), sustainMod, releaseMod);
        }
        audioEngine.polyEnv[voiceIndex].noteOn();
        audioEngine.polyEnv2[voiceIndex].noteOn();
        audioEngine.polyEnv3[voiceIndex].noteOn();
        //audioEngine.moogEnv.noteOn();
      }
      break;

      case DRUM:
        audioEngine.polyDrum[voiceIndex].frequency(baseFreq * 0.25);
        audioEngine.polyDrum[voiceIndex].secondMix(presetBank[currentSoundMode].sound.drum.secondMix * pression);
        audioEngine.polyDrum[voiceIndex].noteOn();
      break;

      case STRING:
      {
        float stringVelocity = 0.5 + 0.5 * pression * autolevelFactor;
        audioEngine.polyString[voiceIndex].noteOn(baseFreq * 0.5, stringVelocity);
      }
      break;
      case SAMPLE:
        {
        int nMixer = voiceIndex / 4;
        int nchannel = voiceIndex % 4;
        audioEngine.sampleMixer[nMixer].gain(nchannel, sampleLevel * pression);
        if (!arpegioMode) {        
        audioEngine.polySample[voiceIndex].play(drumKits[currentDrumKit].samplesLong[touchIndex]);
        }
        else {
          if (sampleDuration >= 250) {
            audioEngine.polySample[voiceIndex].play(drumKits[currentDrumKit].samplesLong[touchIndex]);
          }
          else if (sampleDuration < 250 && sampleDuration >= 50) {
            audioEngine.polySample[voiceIndex].play(drumKits[currentDrumKit].samplesMoyen[touchIndex]);
          }
          else if (sampleDuration < 50) {
            audioEngine.polySample[voiceIndex].play(drumKits[currentDrumKit].samplesCourt[touchIndex]);
          }

        }
        
      }
      break;
    }
  if (presetBank[currentSoundMode].effects.noiseAmplitude > 0.0f) {
    audioEngine.whiteNoise.amplitude(presetBank[currentSoundMode].effects.noiseAmplitude);
    audioEngine.noiseEnv.attack(noiseAtt - velocite * 0.5);
    audioEngine.noiseEnv.decay(noiseDec * (1.0f - pression * 0.5));
    audioEngine.noiseEnv.sustain(noiseSust + (1.0f - noiseSust) * (pression * 0.3f));
    audioEngine.noiseEnv.release(noiseRel * (1.0f + pression * 0.4f));
    audioEngine.noiseEnv.noteOn();
    audioEngine.noiseCutOffEnv.noteOn();
  }
  if (presetBank[currentSoundMode].effects.moogCutoff > 0.0f) {
    audioEngine.moogEnv.attack(moogAtt - velocite * 0.5);
    audioEngine.moogEnv.decay(moogDec * (1.0f - pression * 0.5));
    audioEngine.moogEnv.sustain(moogSust + (1.0f - moogSust) * (pression * 0.3f));
    audioEngine.moogEnv.release(moogRel * (1.0f + pression * 0.4f));
    audioEngine.moogEnv.noteOn();
    audioEngine.moogLfoEnv.noteOn();
  }
  audioEngine.tremoloEnv.noteOn();
  if (MidiManager::getEnabled()) {
    playMidi(touchIndex, baseFreq, pression);
  }
}


// ... toutes tes autres fonctions ici, sans changer le contenu, juste déplacer depuis le header ...

