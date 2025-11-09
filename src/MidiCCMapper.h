#ifndef MIDI_CC_MAPPER_H
#define MIDI_CC_MAPPER_H

#include <Arduino.h>

// Table de correspondance CC → plage de valeurs
struct CCMapping {
    uint8_t cc;
    float minValue;
    float maxValue;
};

// Tous les CC de ton projet
static const CCMapping ccTable[] = {
    {23, 0.0f, 1.0f}, // amplitude
    {24, 0.0f, 1000.0f}, // attack
    {25, 0.0f, 1000.0f}, // decay
    {26, 0.0f, 1.0f},    // sustain
    {27, 0.0f, 1000.0f}, // release
    {28, 0.0f, 1.0f},    // freqMod
    {29, 0.0f, 180.0f},    // phaseMod
    {30, 0.0f, 10.0f},   // ratioFm
    {31, 0.0f, 1.0f},    // modAmplitude
    {34, 0.0f, 1000.0f}, // moogAttack
    {35, 0.0f, 1000.0f}, // moogDecay
    {36, 0.0f, 1.0f},    // moogSustain
    {37, 0.0f, 1000.0f}, // moogRelease
    {39, 0.0f, 2.0f},    // lpfResonance
    {40, 0.0f, 1.0f},    // reverbRoomSize
    {41, 0.0f, 1.0f},    // reverbDamping
    {42, 0.0f, 1.0f}, // filterFreq
    //{42, 0.0f, 10000.0f}, // filterFreq
    {43, 0.0f, 1.0f},    // filterResonance
    {44, 0.0f, 16.0f},   // nChorus
    {45, 0.0f, 20.0f},   // tremoloRate
    {46, 0.0f, 1.0f},    // tremoloAmplitude
    {47, 0.0f, 20.0f},   // moogLfoRate
    {48, 0.0f, 1.0f},    // moogLfoAmplitude_fx
    {49, 0.0f, 1.0f},   // String Velocity
    {50, 0.0f, 10000.0f},  // lpfCutoff
    {51, 0.0f, 3.0f},   // lpfEnvAmount
    {52, 0.0f, 3.0f},  //FMEnvAmount
    {54, 0.0f, 2.0f},  // lpfOctaveControl
    {60, 0.0f, 1000.0f},  // Length (Drum)
    {61, 0.0f, 1.0f},  // secondMix
    {62, 0.0f, 1.0f}, // pitchMod
    {70, 0.0f, 1.0f}, //velocity
    {80, 0.0f, 1.0f}, // noiseAmplitude
    {81, 0.0f, 10000.0f}, // noiseCutoff
    {82, 0.0f, 2.0f}, // noiseResonance
    {83, 0.0f, 1000.0f}, // noiseEnvAttack
    {84, 0.0f, 1000.0f}, // noiseEnvDecay
    {85, 0.0f, 1.0f}, // noiseEnvSustain
    {86, 0.0f, 1000.0f}, // noiseEnvRelease
    {87, 0.0f, 3.0f}, // noiseCutoffEnvAmount
    {89, 0.0f, 2.0f}, // noiseOctaveControl
    {90, 0.0f, 10000.0f}, // moogCutoff
    {91, 0.0f, 1.8f}, // moogResonance
    {92, 0.0f, 2.0f} // moogOctaveControl


};



// Fonction utilitaire
class MidiCCMapper {
public:
    // Renvoie une valeur normalisée ou réelle selon le CC
    static float convertCC(uint8_t cc, uint8_t midiValue) {
        for (auto &m : ccTable) {
            if (m.cc == cc) {
                float norm = midiValue / 127.0f;
                return m.minValue + norm * (m.maxValue - m.minValue);
            }
        }
        // Si CC non trouvé → valeur normalisée
        return 0;
    }


private:

};

#endif
