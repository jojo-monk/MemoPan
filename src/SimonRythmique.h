/*
 * SimonRythmique.h - Classe pour le jeu de Simon Rythmique avec support multi-notes
 * Usage: Instancier la classe et implémenter les callbacks audio/écran
 */

#ifndef SIMON_RYTHMIQUE_H
#define SIMON_RYTHMIQUE_H

#include <Arduino.h>

class SimonRythmique {
public:
    // États du jeu
    enum GameState {
        WAITING_TO_START,
        METRONOME_BEFORE,
        PLAYING_SEQUENCE,
        METRONOME_AFTER, 
        PLAYER_INPUT,
        SUCCESS_FEEDBACK,
        GAME_OVER
    };

    // Modes de difficulté
    enum DifficultyMode {
        RHYTHM_ONLY,      // Seulement le rythme (1 son)
        MULTI_NOTE        // Rythme + hauteurs de notes différentes
    };

    // Configuration par défaut
    static const int MAX_MEASURES = 12;
    static const int BEATS_PER_MEASURE = 16;
    static const int METRONOME_BEATS = 4;
    static const int MAX_NOTES = 8;  // Maximum 8 notes différentes

    // Structure pour une note dans la séquence
    struct Note {
        bool active;      // true si une note doit être jouée
        uint8_t noteId;   // ID de la note (0-7)
        
        Note() : active(false), noteId(0) {}
        Note(bool a, uint8_t n) : active(a), noteId(n) {}
    };

    // Constructeur
    SimonRythmique(float bpm = 120.0, DifficultyMode mode = RHYTHM_ONLY);

    // Configuration
    void setBPM(float bpm);
    void setMaxMeasures(int maxMeasures);
    void setRhythmProbability(float probability);
    void setDifficultyMode(DifficultyMode mode);
    void setNumNotes(int numNotes); // Nombre de notes différentes (2-8)

    // Contrôle du jeu
    void begin();
    void update();
    void startGame();
    void resetGame();

    // Callbacks à implémenter dans votre code principal
    void setMetronomeCallback(void (*callback)());
    void setRhythmCallback(void (*callback)(uint8_t noteId)); // Modifié pour inclure l'ID de note
    void setDisplayCallback(void (*callback)(GameState state, int measure, int score, int beat));
    void setSuccessCallback(void (*callback)());
    void setGameOverCallback(void (*callback)());
    void setTouchCallback(bool (*callback)()); // Pour le mode rhythm only
    void setMultiTouchCallback(int (*callback)()); // Pour le mode multi-note (retourne l'ID de la note pressée, -1 si rien)

    // Getters pour l'état du jeu
    GameState getCurrentState() const { return currentState; }
    int getCurrentMeasure() const { return currentMeasure; }
    int getScore() const { return score; }
    int getCurrentBeat() const { return currentBeat; }
    int getTotalBeats() const { return sequence.totalBeats; }
    DifficultyMode getDifficultyMode() const { return difficultyMode; }
    int getNumNotes() const { return numNotes; }
    Note getSequenceNote(int beat) const;

    // Debug
    void printSequence() const;
    void printGameState() const;

private:
    // Structure pour les séquences rythmiques avec notes
    struct RhythmSequence {
        Note pattern[MAX_MEASURES * BEATS_PER_MEASURE];
        int currentLength;
        int totalBeats;
        
        void clear() {
            for(int i = 0; i < MAX_MEASURES * BEATS_PER_MEASURE; i++) {
                pattern[i] = Note();
            }
            currentLength = 1;
            totalBeats = BEATS_PER_MEASURE;
        }
    };

    // Variables de configuration
    float bpm;
    float sixteenthNoteMs;
    int maxMeasures;
    float rhythmProbability;
    DifficultyMode difficultyMode;
    int numNotes;

    // État du jeu
    GameState currentState;
    RhythmSequence sequence;
    RhythmSequence playerInput;
    int currentMeasure;
    int currentBeat;
    unsigned long lastBeatTime;
    int metronomeCount;
    int score;

    // Variables pour la saisie joueur
    bool lastTouchState;
    int lastNotePressed;

    // Callbacks
    void (*metronomeCallback)();
    void (*rhythmCallback)(uint8_t noteId);
    void (*displayCallback)(GameState state, int measure, int score, int beat);
    void (*successCallback)();
    void (*gameOverCallback)();
    bool (*touchCallback)();
    int (*multiTouchCallback)();

    // Méthodes privées
    void generateNewMeasure();
    void startNewRound();
    void handlePlayerInput(unsigned long currentTime);
    void handlePlayerInputRhythmOnly(unsigned long currentTime);
    void handlePlayerInputMultiNote(unsigned long currentTime);
    void checkPlayerSequence();
    void nextLevel();
    void updateTiming();
    uint8_t getRandomNoteId();
};

// Implémentation

SimonRythmique::SimonRythmique(float bpm, DifficultyMode mode) : 
    bpm(bpm),
    maxMeasures(MAX_MEASURES),
    rhythmProbability(0.4),
    difficultyMode(mode),
    numNotes(4),
    currentState(WAITING_TO_START),
    currentMeasure(1),
    currentBeat(0),
    lastBeatTime(0),
    metronomeCount(0),
    score(0),
    lastTouchState(false),
    lastNotePressed(-1),
    metronomeCallback(nullptr),
    rhythmCallback(nullptr),
    displayCallback(nullptr),
    successCallback(nullptr),
    gameOverCallback(nullptr),
    touchCallback(nullptr),
    multiTouchCallback(nullptr) {
    
    updateTiming();
}

void SimonRythmique::setBPM(float newBpm) {
    bpm = newBpm;
    updateTiming();
}

void SimonRythmique::setMaxMeasures(int max) {
    maxMeasures = constrain(max, 1, MAX_MEASURES);
}

void SimonRythmique::setRhythmProbability(float prob) {
    rhythmProbability = constrain(prob, 0.1, 0.8);
}

void SimonRythmique::setDifficultyMode(DifficultyMode mode) {
    difficultyMode = mode;
}

void SimonRythmique::setNumNotes(int num) {
    numNotes = constrain(num, 2, MAX_NOTES);
}

void SimonRythmique::updateTiming() {
    sixteenthNoteMs = (60000.0 / bpm) / 4.0;
}

uint8_t SimonRythmique::getRandomNoteId() {
    return random(numNotes);
}

void SimonRythmique::begin() {
    randomSeed(analogRead(A0));
    resetGame();
}

void SimonRythmique::resetGame() {
    currentState = WAITING_TO_START;
    currentMeasure = 1;
    currentBeat = 0;
    metronomeCount = 0;
    score = 0;
    lastTouchState = false;
    lastNotePressed = -1;
    
    sequence.clear();
    playerInput.clear();
    
    generateNewMeasure();
    
    if (displayCallback) {
        displayCallback(currentState, currentMeasure, score, currentBeat);
    }
}

void SimonRythmique::generateNewMeasure() {
    int startBeat = (sequence.currentLength - 1) * BEATS_PER_MEASURE;
    
    // Générer le pattern rythmique avec notes
    for (int i = 0; i < BEATS_PER_MEASURE; i++) {
        bool hasNote = (random(100) < (rhythmProbability * 100));
        
        if (hasNote) {
            uint8_t noteId = (difficultyMode == MULTI_NOTE) ? getRandomNoteId() : 0;
            sequence.pattern[startBeat + i] = Note(true, noteId);
        } else {
            sequence.pattern[startBeat + i] = Note(false, 0);
        }
    }
    
    // S'assurer qu'il y a au moins une note dans la mesure
    bool hasSound = false;
    for (int i = 0; i < BEATS_PER_MEASURE; i++) {
        if (sequence.pattern[startBeat + i].active) {
            hasSound = true;
            break;
        }
    }
    
    if (!hasSound) {
        uint8_t noteId = (difficultyMode == MULTI_NOTE) ? getRandomNoteId() : 0;
        sequence.pattern[startBeat + random(BEATS_PER_MEASURE)] = Note(true, noteId);
    }
}

void SimonRythmique::startGame() {
    if (currentState == WAITING_TO_START) {
        startNewRound();
    }
}

void SimonRythmique::startNewRound() {
    currentState = METRONOME_BEFORE;
    currentBeat = 0;
    metronomeCount = 0;
    lastBeatTime = millis();
    lastTouchState = false;
    lastNotePressed = -1;
    
    // Effacer l'entrée du joueur
    playerInput.clear();
    playerInput.totalBeats = sequence.totalBeats;
}

void SimonRythmique::update() {
    unsigned long currentTime = millis();
    
    switch (currentState) {
        case WAITING_TO_START:
            if (difficultyMode == RHYTHM_ONLY) {
                if (touchCallback && touchCallback()) {
                    startNewRound();
                }
            } else {
                if (multiTouchCallback && multiTouchCallback() >= 0) {
                    startNewRound();
                }
            }
            break;
            
        case METRONOME_BEFORE:
            if (currentTime - lastBeatTime >= sixteenthNoteMs * 4) {
                if (metronomeCallback) metronomeCallback();
                metronomeCount++;
                lastBeatTime = currentTime;
                
                if (metronomeCount >= METRONOME_BEATS) {
                    currentState = PLAYING_SEQUENCE;
                    currentBeat = 0;
                    metronomeCount = 0;
                }
            }
            break;
            
        case PLAYING_SEQUENCE:
            if (currentTime - lastBeatTime >= sixteenthNoteMs) {
                Note currentNote = sequence.pattern[currentBeat];
                if (currentNote.active && rhythmCallback) {
                    rhythmCallback(currentNote.noteId);
                }
                
                currentBeat++;
                lastBeatTime = currentTime;
                
                if (currentBeat >= sequence.totalBeats) {
                    currentState = METRONOME_AFTER;
                    currentBeat = 0;
                    metronomeCount = 0;
                }
            }
            break;
            
        case METRONOME_AFTER:
            if (currentTime - lastBeatTime >= sixteenthNoteMs * 4) {
                if (metronomeCallback) metronomeCallback();
                metronomeCount++;
                lastBeatTime = currentTime;
                
                if (metronomeCount >= METRONOME_BEATS) {
                    currentState = PLAYER_INPUT;
                    currentBeat = 0;
                    lastBeatTime = currentTime;
                }
            }
            break;
            
        case PLAYER_INPUT:
            handlePlayerInput(currentTime);
            break;
            
        case SUCCESS_FEEDBACK:
            if (currentTime - lastBeatTime >= 2000) {
                nextLevel();
            }
            break;
            
        case GAME_OVER:
            if (difficultyMode == RHYTHM_ONLY) {
                if (touchCallback && touchCallback()) {
                    resetGame();
                }
            } else {
                if (multiTouchCallback && multiTouchCallback() >= 0) {
                    resetGame();
                }
            }
            break;
    }
    
    // Mettre à jour l'affichage
    if (displayCallback) {
        displayCallback(currentState, currentMeasure, score, currentBeat);
    }
}

void SimonRythmique::handlePlayerInput(unsigned long currentTime) {
    if (difficultyMode == RHYTHM_ONLY) {
        handlePlayerInputRhythmOnly(currentTime);
    } else {
        handlePlayerInputMultiNote(currentTime);
    }
}

void SimonRythmique::handlePlayerInputRhythmOnly(unsigned long currentTime) {
    if (currentTime - lastBeatTime >= sixteenthNoteMs) {
        bool currentTouch = touchCallback ? touchCallback() : false;
        
        // Détecter l'appui (front montant)
        if (currentTouch && !lastTouchState) {
            playerInput.pattern[currentBeat] = Note(true, 0);
            if (rhythmCallback) rhythmCallback(0); // Feedback immédiat
        }
        
        lastTouchState = currentTouch;
        currentBeat++;
        lastBeatTime = currentTime;
        
        if (currentBeat >= sequence.totalBeats) {
            checkPlayerSequence();
        }
    }
}

void SimonRythmique::handlePlayerInputMultiNote(unsigned long currentTime) {
    if (currentTime - lastBeatTime >= sixteenthNoteMs) {
        int currentNote = multiTouchCallback ? multiTouchCallback() : -1;
        
        // Détecter l'appui d'une nouvelle note
        if (currentNote >= 0 && currentNote != lastNotePressed) {
            playerInput.pattern[currentBeat] = Note(true, currentNote);
            if (rhythmCallback) rhythmCallback(currentNote); // Feedback immédiat
        }
        
        lastNotePressed = currentNote;
        currentBeat++;
        lastBeatTime = currentTime;
        
        if (currentBeat >= sequence.totalBeats) {
            checkPlayerSequence();
        }
    }
}

void SimonRythmique::checkPlayerSequence() {
    bool sequenceCorrect = true;
    
    for (int i = 0; i < sequence.totalBeats; i++) {
        Note seqNote = sequence.pattern[i];
        Note playerNote = playerInput.pattern[i];
        
        // Comparer si les notes sont actives
        if (seqNote.active != playerNote.active) {
            sequenceCorrect = false;
            break;
        }
        
        // Si les deux sont actives, comparer les hauteurs (en mode multi-note)
        if (seqNote.active && playerNote.active && difficultyMode == MULTI_NOTE) {
            if (seqNote.noteId != playerNote.noteId) {
                sequenceCorrect = false;
                break;
            }
        }
    }
    
    if (sequenceCorrect) {
        // Bonus de score pour le mode multi-note
        int baseScore = currentMeasure * 100;
        int bonus = (difficultyMode == MULTI_NOTE) ? (numNotes * 10) : 0;
        score += baseScore + bonus;
        
        currentState = SUCCESS_FEEDBACK;
        lastBeatTime = millis();
        if (successCallback) successCallback();
    } else {
        currentState = GAME_OVER;
        if (gameOverCallback) gameOverCallback();
    }
}

void SimonRythmique::nextLevel() {
    currentMeasure++;
    
    if (currentMeasure > maxMeasures) {
        currentState = GAME_OVER;
        if (gameOverCallback) gameOverCallback();
        return;
    }
    
    sequence.currentLength = currentMeasure;
    sequence.totalBeats = currentMeasure * BEATS_PER_MEASURE;
    generateNewMeasure();
    
    startNewRound();
}

// Setters pour les callbacks
void SimonRythmique::setMetronomeCallback(void (*callback)()) {
    metronomeCallback = callback;
}

void SimonRythmique::setRhythmCallback(void (*callback)(uint8_t)) {
    rhythmCallback = callback;
}

void SimonRythmique::setDisplayCallback(void (*callback)(GameState, int, int, int)) {
    displayCallback = callback;
}

void SimonRythmique::setSuccessCallback(void (*callback)()) {
    successCallback = callback;
}

void SimonRythmique::setGameOverCallback(void (*callback)()) {
    gameOverCallback = callback;
}

void SimonRythmique::setTouchCallback(bool (*callback)()) {
    touchCallback = callback;
}

void SimonRythmique::setMultiTouchCallback(int (*callback)()) {
    multiTouchCallback = callback;
}

SimonRythmique::Note SimonRythmique::getSequenceNote(int beat) const {
    if (beat >= 0 && beat < sequence.totalBeats) {
        return sequence.pattern[beat];
    }
    return Note();
}

void SimonRythmique::printSequence() const {
    Serial.print("Séquence: ");
    for (int i = 0; i < sequence.totalBeats; i++) {
        if (i % BEATS_PER_MEASURE == 0 && i > 0) {
            Serial.print(" | ");
        }
        
        Note note = sequence.pattern[i];
        if (note.active) {
            if (difficultyMode == MULTI_NOTE) {
                Serial.print(note.noteId);
            } else {
                Serial.print("X");
            }
        } else {
            Serial.print("-");
        }
    }
    Serial.println();
}

void SimonRythmique::printGameState() const {
    Serial.print("État: ");
    switch (currentState) {
        case WAITING_TO_START: Serial.print("En attente"); break;
        case METRONOME_BEFORE: Serial.print("Métronome initial"); break;
        case PLAYING_SEQUENCE: Serial.print("Lecture séquence"); break;
        case METRONOME_AFTER: Serial.print("Métronome final"); break;
        case PLAYER_INPUT: Serial.print("Saisie joueur"); break;
        case SUCCESS_FEEDBACK: Serial.print("Succès"); break;
        case GAME_OVER: Serial.print("Game Over"); break;
    }
    Serial.print(" - Mode: ");
    Serial.print(difficultyMode == RHYTHM_ONLY ? "Rythme" : "Multi-notes");
    Serial.print(" - Mesure: ");
    Serial.print(currentMeasure);
    Serial.print("/");
    Serial.print(maxMeasures);
    Serial.print(" - Score: ");
    Serial.println(score);
}

#endif // SIMON_RYTHMIQUE_H