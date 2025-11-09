#ifndef SOUNDMODES_H
#define SOUNDMODES_H

#include <Arduino.h>
#include <Audio.h>  // pour WAVEFORM_SINE, etc.

// === Types de sons disponibles ===
enum SoundCategory {
    SYNTH,
    DRUM,
    STRING,
    SAMPLE
};

// === Paramètres des différents types de sons ===
struct __attribute__((packed)) SynthParams {
    uint8_t waveformType;
    float amplitude;
    int attack, decay;
    float sustain;
    int release;
    float freqMod;
    float phaseMod;
    float ratioFm;
    float modAmplitude;
    int moogAttack, moogDecay;
    float moogSustain;
    float moogRelease;
    float lpfResonance;
    uint8_t octaveShift; // de 0 à 4
    float lpfCutoff;
    float lpfEnvAmount; // ratio d'env de coupure de 0 à 3
    float FMEnvAmount;  // ratio d'env de FM de 0 à 3
    uint8_t FMWaveform;
    float lpfOctaveControl; // de 0.0 à 4.0
    uint8_t FMwavetable;
    uint8_t wavetable;
};

struct __attribute__((packed)) DrumParams {
    int length;
    float secondMix;
    float pitchMod;
    int moogAttack, moogDecay;
    float moogSustain;
    float moogRelease;
    uint8_t octaveShift;
};

struct __attribute__((packed)) StringParams {
    float velocity;
    int moogAttack, moogDecay;
    float moogSustain;
    float moogRelease;
    uint8_t octaveShift;
};

struct __attribute__((packed)) SampleParams {
    const char* filename[12]; // nombre de pads ou samples
    int moogAttack, moogDecay;
    float moogSustain;
    float moogRelease;
};

// === Mode de son complet ===
struct __attribute__((packed)) SoundMode {
    SoundCategory type;
    const char* name;
    union {
        SynthParams synth;
        DrumParams drum;
        StringParams string;
        SampleParams sample;
    };
};

// === Paramètres d’effets ===

struct __attribute__((packed)) EffectsParams {
    // Reverb
    float reverbRoomSize = 0.5;
    float reverbDamping = 0.5;
    
    // Filter
    float filterFreq = 1000.0;
    float filterResonance = 1.0;
    
    // Chorus
    uint8_t nChorus = 0;
    
    // Tremolo
    uint8_t tremoloWaveform = WAVEFORM_SINE;
    float tremoloRate = 0.0;
    float tremoloAmplitude = 0.0;
    
    // moogLfo
    uint8_t moogLfoWaveform = WAVEFORM_SINE;
    float moogLfoRate = 0.0;
    float moogLfoAmplitude = 0.0;
    float noiseAmplitude = 0.0;
    float noiseCutoff = 0.0;
    float noiseResonance = 0.0;
    int noiseEnvAttack = 10;
    int noiseEnvDecay = 500;
    float noiseEnvSustain = 0.0;
    int noiseEnvRelease = 300;
    float noiseCutoffEnvAmount = 0.0; // amount of envelope applied to cutoff 0 à 3
    uint8_t noiseFilterType = 0; // 0=lowpass, 1=bandpass, 2=highpass
    float noiseOctaveControl = 0.0; // de 0.0 à 7.0
    float moogOctaveControl = 0.0; // de 0.0 à 7.0
    float moogCutoff = 1000.0; // fréquence de coupure Moog 2000 à 20000 Hz
    float moogResonance = 0.5; // résonance Moog 0 à 1.8
};


// === Preset complet ===
struct Preset {
    SoundMode sound;
    EffectsParams effects;
    const char* presetName;
    uint8_t soundTypeIndex;
};

#endif // SOUNDMODES_H
