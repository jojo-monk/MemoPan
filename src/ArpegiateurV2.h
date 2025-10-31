#ifndef ARPEGIATEURV2_H
#define ARPEGIATEURV2_H
#include <Arduino.h>
#include "Debug.h"
#include "config.h"

// Structure pour stocker les informations d'une note
struct NoteInfo {
    uint8_t note;
    float pressure;    // Valeur de pression Normalisé (0-1)
    uint8_t velocity;    // Vélocité calculée ou mesurée (0-127 MIDI standard)
    
    NoteInfo() : note(0), pressure(0), velocity(0) {}
    NoteInfo(uint8_t n, float p, uint8_t v) : note(n), pressure(p), velocity(v) {}
};

// Structure pour retourner les informations lors du tick
struct ArpegiateurOutput {
    int8_t note;
    float pressure;
    uint8_t velocity;
    bool valid;
    bool isNoteOn;  // true pour note-on, false pour note-off
    bool allNotesReleased;
    
    ArpegiateurOutput() : note(-1), pressure(0), velocity(0), valid(false), isNoteOn(false), allNotesReleased(false) {}
    ArpegiateurOutput(int8_t n, float p, uint8_t v, bool noteOn = true) 
        : note(n), pressure(p), velocity(v), valid(true), isNoteOn(noteOn), allNotesReleased(false) {}
};

class Arpegiateur {
public:
    // 0 = UP, 1 = DOWN, 2 = UP_DOWN, 3 = RANDOM
    Arpegiateur(uint8_t maxNotes=NUM_TOUCH_PADS)
        : maxNotes(NUM_TOUCH_PADS), noteCount(0), currentIndex(0),
          directionUp(true), enabled(false),
          stepIntervalMs(200), lastStepTime(0), mode(0), 
          currentPlayingNote(-1), noteDuration(150) {
            for (uint8_t i = 0; i < NUM_TOUCH_PADS; i++) {
                activeNotes[i] = NoteInfo();
            }
          }


    void setMode(uint8_t m) {
        mode = m;
    }

    void setEnabled(bool state) { 
        enabled = state; 
        if (!enabled && currentPlayingNote != -1) {
            // Couper la note en cours si on désactive l'arpégiateur
            stopCurrentNote = true;
        }
    }
    
    bool isEnabled() { return enabled; }

    unsigned long setStepInterval(uint8_t interval_id, uint8_t bpm) {
        unsigned long interval = STEP_INTERVALS_MS[interval_id];
        stepIntervalMs = (unsigned long)(60000 / (bpm * interval));
        // Ajuster la durée des notes (par exemple 80% de l'intervalle)
        noteDuration = stepIntervalMs * 0.9;
        return noteDuration;
    }

    // Définir la durée des notes manuellement (en ms)
    void setNoteDuration(unsigned long duration) {
        noteDuration = duration;
    }

    // Ajouter une note avec pression et vélocité
    void noteOn(uint8_t note, float pressure, uint8_t velocity) {
        if (!enabled) return;
        if (noteCount >= maxNotes) return;
        
        // Vérifier si la note existe déjà
        for (uint8_t i = 0; i < noteCount; i++) {
            if (activeNotes[i].note == note) {
                // Mettre à jour la pression et vélocité si la note existe
                activeNotes[i].pressure = pressure;
                activeNotes[i].velocity = velocity;
                return;
            }
        }
        
        // Ajouter nouvelle note
        activeNotes[noteCount] = NoteInfo(note, pressure, velocity);
        noteCount++;
        sortNotes();

    }

    // Version simplifiée pour compatibilité (vélocité par défaut basée sur la pression)
    // void noteOn(uint8_t note) {
    //     noteOn(note, 127, 64); // Valeurs par défaut
    // }

    // Supprimer une note (touche relâchée)
    void noteOff(uint8_t note) {
        for (uint8_t i = 0; i < noteCount; i++) {
            if (activeNotes[i].note == note) {
                for (uint8_t j = i; j < noteCount - 1; j++)
                    activeNotes[j] = activeNotes[j + 1];
                noteCount--;
                if (currentIndex >= noteCount && noteCount > 0) currentIndex = 0;
                if (noteCount == 0) {
                currentIndex = 0;
                directionUp = true;   // remet le sens (utile pour UP_DOWN)
                currentPlayingNote = -1;
                stopCurrentNote = false;
            }
                break;
            }
        }
        sortNotes();
    }

    // Mettre à jour la pression d'une note active (pour les capteurs en temps réel)
    void updatePressure(uint8_t note, float pressure) {
        for (uint8_t i = 0; i < noteCount; i++) {
            if (activeNotes[i].note == note) {
                activeNotes[i].pressure = pressure;
                break;
            }
        }
    }

    // Appeler dans loop() pour avancer l'arpège - retourne maintenant une structure complète
    ArpegiateurOutput tickAdvanced() {
        if (!enabled || noteCount == 0) {
            if (currentPlayingNote != -1) {
                stopCurrentNote = true;
                ArpegiateurOutput noteOff(currentPlayingNote, 0, 0, false);
                currentPlayingNote = -1;
                stopCurrentNote = false;
                noteOff.allNotesReleased = true;
                return noteOff;
            }
            else {
                // Rien à jouer
                ArpegiateurOutput empty;
                empty.allNotesReleased = true;
                return empty;
            }
            return ArpegiateurOutput();
        }
        
        unsigned long now = millis();
        
        // Vérifier si on doit couper la note actuelle
        if (currentPlayingNote != -1 && (stopCurrentNote || (now - noteStartTime >= noteDuration))) {
            ArpegiateurOutput noteOff(currentPlayingNote, 0, 0, false);
            currentPlayingNote = -1;
            stopCurrentNote = false;
            return noteOff;
        }
        
        // Vérifier si on doit jouer la prochaine note
        if (currentPlayingNote == -1 && (now - lastStepTime >= stepIntervalMs)) {
            lastStepTime = now;
            noteStartTime = now;
            
            NoteInfo& currentNote = activeNotes[currentIndex];
            currentPlayingNote = currentNote.note;
            
            // Logique de navigation identique
            switch (mode) {
                case 0: // UP
                    currentIndex++;
                    if (currentIndex >= noteCount) currentIndex = 0;
                    break;
                case 1: // DOWN
                    if (currentIndex == 0) currentIndex = noteCount - 1;
                    else currentIndex--;
                    break;
                case 2: // UP_DOWN
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
                case 3: // RANDOM
                    currentIndex = random(noteCount);
                    break;
            }
            
            return ArpegiateurOutput(currentNote.note, currentNote.pressure, currentNote.velocity, true);
        }
        
        return ArpegiateurOutput(); // Pas d'événement à ce tick
    }

    // Version compatible avec l'ancienne API - ATTENTION: ne gère que les note-on
    int8_t tick() {
        ArpegiateurOutput output = tickAdvanced();
        if (output.valid && output.isNoteOn) {
            return output.note;
        }
        return -1;
    }

    // Obtenir les informations de la note courante
    NoteInfo getCurrentNoteInfo() {
        if (noteCount == 0 || currentIndex >= noteCount) return NoteInfo();
        return activeNotes[currentIndex];
    }

    // Obtenir les informations d'une note spécifique
    NoteInfo getNoteInfo(uint8_t note) {
        for (uint8_t i = 0; i < noteCount; i++) {
            if (activeNotes[i].note == note) {
                return activeNotes[i];
            }
        }
        return NoteInfo();
    }

    void reset() {
        noteCount = 0;
        currentIndex = 0;
        directionUp = true;
        if (currentPlayingNote != -1) {
            stopCurrentNote = true;
        }
    }

    // Fonctions utilitaires pour accéder aux données
    uint8_t getNoteCount() { return noteCount; }
    int8_t getCurrentPlayingNote() { return currentPlayingNote; }
    
    void printActiveNotes() {
        DEBUG_INFO_ARP("Active notes: ");
        for (uint8_t i = 0; i < noteCount; i++) {
            DEBUG_VAR_ARP("Note:", activeNotes[i].note);
            DEBUG_VAR_ARP(" P:", activeNotes[i].pressure);
            DEBUG_VAR_ARP(" V:", activeNotes[i].velocity);
        }
        if (currentPlayingNote != -1) {
            DEBUG_VAR_ARP("Currently playing:", currentPlayingNote);
        }
    }
        

private:
    uint8_t maxNotes;
    uint8_t noteCount;
    uint8_t currentIndex;
    bool directionUp;
    bool enabled;
    unsigned long stepIntervalMs;
    unsigned long lastStepTime;
    uint8_t mode;
    int8_t currentPlayingNote;
    unsigned long noteDuration; 
    NoteInfo activeNotes[NUM_TOUCH_PADS];
    unsigned long noteStartTime;    // Quand la note actuelle a commencé
        // Durée de chaque note en ms
          // Note actuellement en cours (-1 si aucune)
    bool stopCurrentNote;           // Flag pour forcer l'arrêt de la note actuelle


    
    // void sortNotes() {
    //     // Tri par bulle basé sur le numéro de note
    //     for (uint8_t i = 0; i < noteCount - 1; i++) {
    //         for (uint8_t j = i + 1; j < noteCount; j++) {
    //             if (activeNotes[i].note > activeNotes[j].note) {
    //                 NoteInfo tmp = activeNotes[i];
    //                 activeNotes[i] = activeNotes[j];
    //                 activeNotes[j] = tmp;
    //             }
    //         }
    //     }
    // }
    void sortNotes() {
        // Tri par insertion (plus efficace pour petits tableaux souvent déjà triés)
        for (uint8_t i = 1; i < noteCount; i++) {
            NoteInfo key = activeNotes[i];
            int8_t j = i - 1;
            
            while (j >= 0 && activeNotes[j].note > key.note) {
                activeNotes[j + 1] = activeNotes[j];
                j--;
            }
            activeNotes[j + 1] = key;
        }
    }
};



#endif