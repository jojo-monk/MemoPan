#ifndef SOUNDMANAGERV4_H
#define SOUNDMANAGERV4_H

#include "Debug.h"
#include "config.h"
#include "Presets.h"
#include "AudioEngine.h"
#include "MidiManagerV2.h"
#include "MapToMidi.h"
#include "SysExHandler.h"
#include "PresetStorage.h"

#define CHORUS_DELAY_LENGTH (16*AUDIO_BLOCK_SAMPLES)

class SoundManager {
private:
    AudioEngine audioEngine;
    Preset presetBank[NUM_PRESETS];
    Preset currentPreset;
    uint8_t currentSoundMode;
    unsigned long soundStartTime;
    SysExHandler* sysexHandler;
    PresetStorage storage;
    bool isPlaying;
    float scaleFreq[NUM_TOUCH_PADS];
    bool polyMode;
    float polyMixerGain;
    float stringLevel;
    float drumLevel;
    float sampleLevel;
    int keyboardMode;
    int randomMap[NUM_TOUCH_PADS];
    bool drumMode;
    bool stringMode;
    bool arpegioMode;
    float maxTime;
    float maxPressure;
    uint16_t sampleDuration;
    short delayline[CHORUS_DELAY_LENGTH];
    // pour la conversion des CC
    float normLevel; // 0-1
    float normFreq; // 0-20000
    float normTime; // 0-1000
    // float ratioGlobal = 1.0f;         // contrôlé par potar ou preset
    // float ratioAttack  = 1.2f;
    // float ratioDecay   = 0.8f;
    // float ratioSustain = 0.9f;
    // float ratioRelease = 1.1f;
    uint8_t currentChorusLevel = 0;
    uint8_t currentTremoloLevel = 0;
    uint8_t noiseDefault = 5;


    static constexpr float MONO_GAIN = 0.5f;
    static constexpr float POLY_GAIN = 0.7f;
    static constexpr float AMP2_GAIN = 1.0f;

    float getNote(int padIndex);
    void stopMidi();
    void playMidi(int touchIndex, float frequency, float velocity);
    void updateFxLevels();
    

public:
    SoundManager();
    bool begin();
    void setSoundMode(int mode);
    void playTouchSound(int touchIndex, float pression = 0.4, uint8_t velocite = 150, int octaveShift = 0);
    void stopOneNote(int touchIndex);
    void stopAllSounds();
    void update();
    void volume(uint8_t vol);
    void buildScale(int noteIndex, int gammeIndex);
    void setReverb(float size, float damp);
    void setDryWetMix(uint8_t mix);
    void setTremoloLevel(uint8_t tremoloLevel);
    //void setMoogFilterLevel(uint8_t moogLevel);
    void setChorusLevel(uint8_t chorusLevel);
    void handleControlChange(uint8_t cc, uint8_t value);
    void handleSynthControlChange(uint8_t cc, uint8_t value, SynthParams& synth, EffectsParams& fx);
    void handleStringControlChange(uint8_t cc, uint8_t value, StringParams& string, EffectsParams& fx);
    void handleDrumControlChange(uint8_t cc, uint8_t value, DrumParams& drum, EffectsParams& fx);
    void handleSampleControlChange(uint8_t cc, uint8_t value, SampleParams& sample, EffectsParams& fx);
    void handleCommonEffects(uint8_t cc, uint8_t value, EffectsParams& fx);
    const char* getSoundModeName() const;
    void setKeyboardMode(int mode);
    void generateRandomMap();
    void setPolyMode(bool enabled);
    void testPoly();
    void updateMidiNotes();
    void unMute();
    void setPolyEnv(int attack, int decay, float sustain, int release);
    void setArpeggio(bool enabled);
    void soundDuration(uint16_t durationMs);
    //void getAllCC(uint8_t cc, uint8_t value);
    void readVolumePot();
    bool isPlayingNotes();
    Preset* getCurrentPreset();
    Preset* getPreset(int index);
    void applyPreset(Preset* preset);
    static void onPresetReceivedStatic(Preset* preset);
    static SoundManager* instance;
    void loadPresetNamesFromStorage();
    void setNoiseLevel(uint8_t level);
    void setNoiseEffect(Preset* preset);
    void setNoiseFilter(uint8_t filterType, uint8_t noiseLevel);
    void setLfoEnv(int att, int dec, float sust, int rel);
};

extern uint8_t activeMidiNotes[NUM_TOUCH_PADS];

#endif
