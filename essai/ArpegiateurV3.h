#ifndef ARPEGIATEURV3_H
#define ARPEGIATEURV3_H
#include <Arduino.h>
#include "Debug.h"
#include "config.h"

// Structure pour stocker les informations d'une note
struct NoteInfo {
    uint8_t note;
    uint8_t pressure;    
    uint8_t velocity;    
    
    NoteInfo() : note(0), pressure(0), velocity(0) {}
    NoteInfo(uint8_t n, uint8_t p, uint8_t v) : note(n), pressure(p), velocity(v) {}
    
    bool operator==(const NoteInfo& other) const {
        return note == other.note;
    }
    
    bool operator<(const NoteInfo& other) const {
        return note < other.note;
    }
};

// Structure pour retourner les informations lors du tick
struct ArpegiateurOutput {
    int8_t note;
    uint8_t pressure;
    uint8_t velocity;
    bool valid;
    bool isNoteOn;
    
    ArpegiateurOutput() : note(-1), pressure(0), velocity(0), valid(false), isNoteOn(false) {}
    ArpegiateurOutput(int8_t n, uint8_t p, uint8_t v, bool noteOn = true) 
        : note(n), pressure(p), velocity(v), valid(true), isNoteOn(noteOn) {}
};

class ArpegiateurRefactored {
public:
    enum State {
        IDLE,           // Pas de note en cours
        NOTE_PLAYING,   // Une note est en cours
        NOTE_STOPPING   // Une note va s'arrêter
    };

    ArpegiateurRefactored(uint8_t maxNotes = NUM_TOUCH_PADS)
        : maxNotes(maxNotes), noteCount(0), currentNoteIndex(-1),
          directionUp(true), enabled(false), mode(0),
          stepIntervalMs(200), noteDuration(150),
          lastStepTime(0), noteStartTime(0),
          currentState(IDLE), pendingNoteOff(-1) {
        
        // Initialisation du tableau
        for (uint8_t i = 0; i < maxNotes; i++) {
            activeNotes[i] = NoteInfo();
        }
    }

    // ========== Configuration ==========
    void setMode(uint8_t m) {  // Correction: suppression de la parenthèse en trop
        mode = m;
        resetSequence();
    }

    void setEnabled(bool state) { 
        if (enabled && !state) {
            // Désactivation : stopper immédiatement
            forceStop();
        }
        enabled = state;
    }
    
    bool isEnabled() const { return enabled; }
    
    // Nouvelle méthode pour forcer l'arrêt immédiat de toutes les notes
    void stopAllNotes() {
        forceStop();
    }

    unsigned long setStepInterval(uint8_t interval_id, uint8_t bpm) {
        unsigned long interval = STEP_INTERVALS_MS[interval_id];
        stepIntervalMs = (60000UL * interval) / bpm;
        noteDuration = stepIntervalMs * 9 / 10; // 90% de l'intervalle
        return noteDuration;
    }

    void setNoteDuration(unsigned long duration) {
        noteDuration = duration;
    }

    // ========== Gestion des notes ==========
    void noteOn(uint8_t note, uint8_t pressure, uint8_t velocity) {
        if (!enabled) return;
        
        DEBUG_VAR_ARP("Adding note:", note);
        
        // Chercher si la note existe déjà
        int8_t existingIndex = findNoteIndex(note);
        if (existingIndex >= 0) {
            // Mettre à jour les paramètres
            activeNotes[existingIndex].pressure = pressure;
            activeNotes[existingIndex].velocity = velocity;
            DEBUG_INFO_ARP("Note updated");
            return;
        }
        
        // Ajouter nouvelle note si on a de la place
        if (noteCount >= maxNotes) {
            DEBUG_INFO_ARP("Max notes reached");
            return;
        }
        
        // Sauvegarder la note actuellement jouée pour maintenir la position
        int8_t currentlyPlayingNote = getCurrentPlayingNote();
        
        // Ajouter la nouvelle note
        activeNotes[noteCount] = NoteInfo(note, pressure, velocity);
        noteCount++;
        
        // Retrier les notes
        sortNotes();
        
        // Rétablir la position dans la séquence
        restoreSequencePosition(currentlyPlayingNote);
        
        DEBUG_VAR_ARP("Note added. Count:", noteCount);
        printDebugState();
    }

    void noteOn(uint8_t note) {
        noteOn(note, 127, 64);
    }

    void noteOff(uint8_t note) {
        DEBUG_VAR_ARP("Removing note:", note);
        
        int8_t noteIndex = findNoteIndex(note);
        if (noteIndex < 0) {
            DEBUG_INFO_ARP("Note not found");
            return;
        }
        
        // Sauvegarder la note actuellement jouée
        int8_t currentlyPlayingNote = getCurrentPlayingNote();
        bool wasPlayingRemovedNote = (currentlyPlayingNote == note);
        
        // Supprimer la note
        removeNoteAtIndex(noteIndex);
        
        if (wasPlayingRemovedNote) {
            // La note qu'on jouait a été supprimée
            handleRemovedCurrentNote();
        } else {
            // Rétablir la position dans la séquence
            restoreSequencePosition(currentlyPlayingNote);
        }
        
        DEBUG_VAR_ARP("Note removed. Count:", noteCount);
        printDebugState();
    }

    void updatePressure(uint8_t note, uint8_t pressure) {
        int8_t index = findNoteIndex(note);
        if (index >= 0) {
            activeNotes[index].pressure = pressure;
        }
    }

    // ========== Moteur principal ==========
    ArpegiateurOutput tickAdvanced() {
        unsigned long now = millis();
        
        // Gérer les note-off en attente EN PREMIER
        if (pendingNoteOff >= 0) {
            ArpegiateurOutput output(pendingNoteOff, 0, 0, false);
            pendingNoteOff = -1;
            currentState = IDLE;
            return output;
        }
        
        // Si désactivé ou plus de notes, forcer l'arrêt
        if (!enabled || noteCount == 0) {
            return handleIdleState();
        }
        
        switch (currentState) {
            case IDLE:
                return handleIdleToPlaying(now);
                
            case NOTE_PLAYING:
                return handleNotePlaying(now);
                
            case NOTE_STOPPING:
                return handleNoteStopping(now);
        }
        
        return ArpegiateurOutput();
    }

    // Version compatible avec l'ancienne API
    int8_t tick() {
        ArpegiateurOutput output = tickAdvanced();
        if (output.valid && output.isNoteOn) {
            return output.note;
        }
        return -1;
    }

    // ========== Utilitaires publics ==========
    void reset() {
        // Forcer l'arrêt de toute note en cours
        forceStop();
        noteCount = 0;
        resetSequence();
    }

    uint8_t getNoteCount() const { return noteCount; }
    
    int8_t getCurrentPlayingNote() const {
        if (currentState == NOTE_PLAYING && currentNoteIndex >= 0 && currentNoteIndex < noteCount) {
            return activeNotes[currentNoteIndex].note;
        }
        return -1;
    }
    
    NoteInfo getCurrentNoteInfo() const {
        if (currentNoteIndex >= 0 && currentNoteIndex < noteCount) {
            return activeNotes[currentNoteIndex];
        }
        return NoteInfo();
    }

    NoteInfo getNoteInfo(uint8_t note) const {
        int8_t index = findNoteIndex(note);
        if (index >= 0) {
            return activeNotes[index];
        }
        return NoteInfo();
    }

    void printActiveNotes() {
        printDebugState();
    }

private:
    // ========== Données membres ==========
    NoteInfo activeNotes[NUM_TOUCH_PADS];
    uint8_t maxNotes;
    uint8_t noteCount;
    int8_t currentNoteIndex;        // Index dans le tableau trié (-1 = aucune)
    bool directionUp;
    bool enabled;
    uint8_t mode;
    
    unsigned long stepIntervalMs;
    unsigned long noteDuration;
    unsigned long lastStepTime;
    unsigned long noteStartTime;
    
    State currentState;
    int8_t pendingNoteOff;          // Note en attente de note-off (-1 = aucune)
    
    // ========== Méthodes privées - Utilitaires ==========
    int8_t findNoteIndex(uint8_t note) const {
        for (uint8_t i = 0; i < noteCount; i++) {
            if (activeNotes[i].note == note) {
                return i;
            }
        }
        return -1;
    }
    
    void removeNoteAtIndex(int8_t index) {
        if (index < 0 || index >= noteCount) return;
        
        for (uint8_t i = index; i < noteCount - 1; i++) {
            activeNotes[i] = activeNotes[i + 1];
        }
        noteCount--;
        
        // Le tableau reste trié après suppression
    }
    
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
    
    // ========== Gestion de la séquence ==========
    void resetSequence() {
        currentNoteIndex = -1;
        directionUp = true;
    }
    
    void restoreSequencePosition(int8_t targetNote) {
        if (targetNote < 0 || noteCount == 0) {
            resetSequence();
            return;
        }
        
        // Chercher la nouvelle position de la note cible
        int8_t newIndex = findNoteIndex(targetNote);
        if (newIndex >= 0) {
            currentNoteIndex = newIndex;
        } else {
            // La note cible n'existe plus, ajuster intelligemment
            if (currentNoteIndex >= noteCount) {
                currentNoteIndex = noteCount - 1;
            }
            // Sinon on garde currentNoteIndex tel quel
        }
    }
    
    void moveToNextNote() {
        if (noteCount == 0) {
            resetSequence();
            return;
        }
        
        // Si on n'a pas encore de position, commencer au début
        if (currentNoteIndex < 0) {
            currentNoteIndex = 0;
            return;
        }
        
        switch (mode) {
            case 0: // UP
                currentNoteIndex = (currentNoteIndex + 1) % noteCount;
                break;
                
            case 1: // DOWN
                currentNoteIndex = (currentNoteIndex == 0) ? noteCount - 1 : currentNoteIndex - 1;
                break;
                
            case 2: // UP_DOWN
                if (noteCount == 1) {
                    currentNoteIndex = 0;
                } else if (directionUp) {
                    currentNoteIndex++;
                    if (currentNoteIndex >= noteCount) {
                        currentNoteIndex = noteCount - 2;
                        directionUp = false;
                    }
                } else {
                    currentNoteIndex--;
                    if (currentNoteIndex < 0) {
                        currentNoteIndex = 1;
                        directionUp = true;
                    }
                }
                break;
                
            case 3: // RANDOM
                if (noteCount > 1) {
                    int8_t newIndex;
                    do {
                        newIndex = random(noteCount);
                    } while (newIndex == currentNoteIndex && noteCount > 1);
                    currentNoteIndex = newIndex;
                } else {
                    currentNoteIndex = 0;
                }
                break;
        }
        
        // Vérification de sécurité
        if (currentNoteIndex >= noteCount) currentNoteIndex = 0;
        if (currentNoteIndex < 0) currentNoteIndex = 0;
    }
    
    // ========== Gestion des états ==========
    ArpegiateurOutput handleIdleState() {
        // Si on était en train de jouer une note, l'arrêter
        if (currentState == NOTE_PLAYING && currentNoteIndex >= 0 && currentNoteIndex < noteCount) {
            int8_t noteToStop = activeNotes[currentNoteIndex].note;
            currentState = IDLE;
            currentNoteIndex = -1;
            return ArpegiateurOutput(noteToStop, 0, 0, false);
        }
        
        currentState = IDLE;
        currentNoteIndex = -1;
        return ArpegiateurOutput();
    }
    
    ArpegiateurOutput handleIdleToPlaying(unsigned long now) {
        if (now - lastStepTime >= stepIntervalMs) {
            lastStepTime = now;
            noteStartTime = now;
            
            moveToNextNote();
            
            if (currentNoteIndex >= 0 && currentNoteIndex < noteCount) {
                currentState = NOTE_PLAYING;
                NoteInfo& note = activeNotes[currentNoteIndex];
                return ArpegiateurOutput(note.note, note.pressure, note.velocity, true);
            }
        }
        return ArpegiateurOutput();
    }
    
    ArpegiateurOutput handleNotePlaying(unsigned long now) {
        if (now - noteStartTime >= noteDuration) {
            currentState = NOTE_STOPPING;
            if (currentNoteIndex >= 0 && currentNoteIndex < noteCount) {
                pendingNoteOff = activeNotes[currentNoteIndex].note;
            }
            return ArpegiateurOutput();
        }
        return ArpegiateurOutput();
    }
    
    ArpegiateurOutput handleNoteStopping(unsigned long now) {
        // Cette fonction ne devrait normalement pas être appelée
        // car pendingNoteOff est géré en premier
        currentState = IDLE;
        return ArpegiateurOutput();
    }
    
    void handleRemovedCurrentNote() {
        // Si on supprime la note actuellement jouée, générer un note-off immédiat
        if (currentState == NOTE_PLAYING && currentNoteIndex >= 0) {
            // La note sera stoppée au prochain tick via handleIdleState
            // On ne génère pas de pendingNoteOff ici car la note n'existe plus
        }
        
        if (noteCount == 0) {
            resetSequence();
            currentState = IDLE;
        } else {
            // Ajuster l'index si nécessaire
            if (currentNoteIndex >= noteCount) {
                currentNoteIndex = noteCount - 1;
            }
        }
    }
    
    void forceStop() {
        // Si une note est en cours, programmer son arrêt
        if (currentState == NOTE_PLAYING && currentNoteIndex >= 0 && currentNoteIndex < noteCount) {
            pendingNoteOff = activeNotes[currentNoteIndex].note;
        }
        currentState = IDLE;
        resetSequence();
    }
    
    // ========== Debug ==========
    void printDebugState() {
        DEBUG_INFO_ARP("=== Arpeggiator State ===");
        DEBUG_VAR_ARP("Enabled:", enabled);
        DEBUG_VAR_ARP("Note count:", noteCount);
        DEBUG_VAR_ARP("Current index:", currentNoteIndex);
        DEBUG_VAR_ARP("Mode:", mode);
        DEBUG_VAR_ARP("Direction up:", directionUp);
        DEBUG_VAR_ARP("State:", currentState);
        
        DEBUG_INFO_ARP("Active notes (sorted):");
        for (uint8_t i = 0; i < noteCount; i++) {
            DEBUG_VAR_ARP("  [", i);
            DEBUG_VAR_ARP("] Note:", activeNotes[i].note);
            DEBUG_VAR_ARP(" P:", activeNotes[i].pressure);
            DEBUG_VAR_ARP(" V:", activeNotes[i].velocity);
            if (i == currentNoteIndex) DEBUG_INFO_ARP(" <- CURRENT");
        }
        
        int8_t playing = getCurrentPlayingNote();
        if (playing >= 0) {
            DEBUG_VAR_ARP("Currently playing:", playing);
        }
        if (pendingNoteOff >= 0) {
            DEBUG_VAR_ARP("Pending note-off:", pendingNoteOff);
        }
        DEBUG_INFO_ARP("========================");
    }
};

// Alias pour compatibilité
typedef ArpegiateurRefactored Arpegiateur;

#endif