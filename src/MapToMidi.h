#ifndef MAP_TO_MIDI_H
#define MAP_TO_MIDI_H

#include <Arduino.h>

class MapToMidi {
public:
    // Paramètres de normalisation (ajustables)
    static const uint16_t minPressure = 5;
    static const uint16_t maxPressure = 600;
    static const unsigned long minTime = 0;
    static const unsigned long maxTime = 1000;

    // --- Fréquence (Hz) -> Note MIDI (0–127) ---
    static uint8_t freqToMidi(float freqHz) {
        if (freqHz <= 0) return 0;
        // MIDI note 69 = 440 Hz (A4)
        return (uint8_t) round(69 + 12 * log2(freqHz / 440.0f));
    }

    // --- Pression brute -> 0–127 MIDI ---
    static uint8_t pressureToMidi(uint16_t rawPressure) {
        if (rawPressure <= minPressure) return 0;
        if (rawPressure >= maxPressure) return 127;
        return (uint8_t) map(rawPressure, minPressure, maxPressure, 1, 127);
    }

    // --- Durée d'appui -> Vélocité MIDI ---
    static uint8_t velocityToMidi(unsigned long durationMs) {
        if (durationMs <= minTime) return 127; // frappe rapide → max
        if (durationMs >= maxTime) return 1;   // frappe lente → min
        return (uint8_t) map(durationMs, minTime, maxTime, 127, 1);
    }
    // --- Note MIDI (0–127) -> Fréquence (Hz) ---
    static float midiToFreq(uint8_t midiNote) {
        return 440.0f * pow(2.0f, (midiNote - 69) / 12.0f);
    }
    // --- 0–127 MIDI -> Pression brute ---
    static uint16_t midiToPressure(uint8_t midiPress) {
        if (midiPress >= 127) return maxPressure; // pression forte
        if (midiPress <= 0)   return minPressure; // pression faible
        return map(midiPress, 1, 127, minPressure, maxPressure);
    }
    // --- Vélocité MIDI -> Durée d'appui ---
    static unsigned long midiToDuration(uint8_t midiVel) {
        if (midiVel >= 127) return minTime;  // frappe rapide
        if (midiVel <= 1)   return maxTime;  // frappe lente
    return map(midiVel, 127, 1, minTime, maxTime);
    }
    // --- Paramètre MIDI (0–127) -> Float (0.0–1.0) ---
    static float midiToFloat(uint8_t midiValue) {
        return midiValue / 127.0f;
    }

    // --- Float (0.0–1.0) -> Paramètre MIDI (0–127) ---
    static uint8_t floatToMidi(float floatValue) {
        if (floatValue <= 0.0f) return 0;
        if (floatValue >= 1.0f) return 127;
        return (uint8_t) round(floatValue * 127.0f);
    }
    
};


#endif
