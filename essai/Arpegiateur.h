#ifndef ARPEGIATEUR_H
#define ARPEGIATEUR_H

#include <Arduino.h>
#include "Debug.h"
#include "config.h"

class Arpegiateur {
public:
    //0 = UP, 1 = DOWN, 2 = UP_DOWN, 3 = RANDOM
    

    Arpegiateur(uint8_t maxNotes = NUM_TOUCH_PADS)
        : maxNotes(maxNotes), noteCount(0), currentIndex(0),
          directionUp(true), enabled(false),
          stepIntervalMs(200), lastStepTime(0),mode(0) {}

    void setMode(uint8_t m) { 
        mode = m;
     }
    void setEnabled(bool state) { enabled = state; }
    bool isEnabled() { return enabled; }
    void setStepInterval(uint8_t interval_id, uint8_t bpm) { 
        unsigned long interval = STEP_INTERVALS_MS[interval_id];
        stepIntervalMs = (unsigned long)(60000 / (bpm * interval));
     }

    // Ajouter une note (touche MPR121 pressée)
    void noteOn(uint8_t note) {
        if (noteCount >= maxNotes) return;
        for (uint8_t i = 0; i < noteCount; i++) {
            if (activeNotes[i] == note) return; // déjà présente
        }
        activeNotes[noteCount++] = note;
        sortNotes();
    }

    // Supprimer une note (touche relâchée)
    void noteOff(uint8_t note) {
        for (uint8_t i = 0; i < noteCount; i++) {
            if (activeNotes[i] == note) {
                for (uint8_t j = i; j < noteCount - 1; j++)
                    activeNotes[j] = activeNotes[j + 1];
                noteCount--;
                if (currentIndex >= noteCount && noteCount > 0) currentIndex = 0;
                break;
            }
        }
    }

    // Appeler dans loop() pour avancer l'arpège
    int8_t tick() {
        if (!enabled || noteCount == 0) return -1;

        unsigned long now = millis();
        if (now - lastStepTime < stepIntervalMs) return -1;

        lastStepTime = now;
        int8_t note = activeNotes[currentIndex];

        switch (mode) {
            case 0:
                currentIndex++;
                if (currentIndex >= noteCount) currentIndex = 0;
                break;
            case 1:
                if (currentIndex == 0) currentIndex = noteCount - 1;
                else currentIndex--;
                break;
            case 2:
                if (directionUp) {
                    currentIndex++;
                    if (currentIndex >= noteCount) {
                        currentIndex = noteCount - 2;
                        directionUp = false;
                    }
                } else {
                    if (currentIndex == 0) {
                        currentIndex = 1;
                        directionUp = true;
                    } else {
                        currentIndex--;
                    }
                }
                break;
            case 3:
                currentIndex = random(noteCount);
                break;
        }
        return note;
    }

    void reset() {
        noteCount = 0;
        currentIndex = 0;
        directionUp = true;
    }

private:
    uint8_t activeNotes[NUM_TOUCH_PADS];
    uint8_t maxNotes;
    uint8_t noteCount;
    uint8_t currentIndex;
    bool directionUp;
    bool enabled;
    unsigned long stepIntervalMs;
    unsigned long lastStepTime;
    uint8_t mode;

    void sortNotes() {
        for (uint8_t i = 0; i < noteCount - 1; i++) {
            for (uint8_t j = i + 1; j < noteCount; j++) {
                if (activeNotes[i] > activeNotes[j]) {
                    uint8_t tmp = activeNotes[i];
                    activeNotes[i] = activeNotes[j];
                    activeNotes[j] = tmp;
                }
            }
        }
    }
};

#endif
