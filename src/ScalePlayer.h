#ifndef SCALEPLAYER_H
#define SCALEPLAYER_H

#include "Debug.h"
#include "config.h"
#include "LedManager.h"
#include "SoundManagerV4.h"
#include <Arduino.h>

extern LedManager ledManager;
extern SoundManager soundManager;

class ScalePlayer {
private:
    bool scalePlaying = false;
    unsigned long lastNoteTime = 0;
    int playScaleNoteId = 0;
    unsigned long noteDuration = 500;
    unsigned long pauseDuration = 100;
    bool waitingPause = false; // indique si on est dans la pause

    void stopScale();

public:
    ScalePlayer();
    void playScale(bool play, unsigned long noteDur = 500, unsigned long pauseDur = 100);
    bool isPlaying();
    void update();
};

ScalePlayer::ScalePlayer() {}

bool ScalePlayer::isPlaying() {
    return scalePlaying;
}

void ScalePlayer::stopScale() {
    scalePlaying = false;
    playScaleNoteId = 0;
    waitingPause = false;
    soundManager.stopAllSounds();
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        ledManager.setLedColor(i, CRGB::Black);
    }
    ledManager.show();
}

void ScalePlayer::playScale(bool play, unsigned long noteDur, unsigned long pauseDur) {
    noteDuration = noteDur;
    pauseDuration = pauseDur;

    if (play) {
        scalePlaying = true;
        waitingPause = false;
        playScaleNoteId = 0;
        lastNoteTime = millis();

        ledManager.setLedColor(playScaleNoteId, TOUCH_COLORS[playScaleNoteId]);
        ledManager.show();
        soundManager.playTouchSound(playScaleNoteId);
    } else {
        stopScale();
    }
}

void ScalePlayer::update() {
    if (!scalePlaying) return;

    unsigned long now = millis();

    if (!waitingPause && (now - lastNoteTime >= noteDuration)) {
        // Fin de la note -> extinction + pause
        ledManager.setLedColor(playScaleNoteId, CRGB::Black);
        ledManager.show();
        soundManager.stopAllSounds();
        lastNoteTime = now;
        waitingPause = true;
    } 
    else if (waitingPause && (now - lastNoteTime >= pauseDuration)) {
        // Fin de la pause -> prochaine note
        playScaleNoteId++;
        if (playScaleNoteId >= NUM_TOUCH_PADS) {
            stopScale();
            return;
        }

        ledManager.setLedColor(playScaleNoteId, TOUCH_COLORS[playScaleNoteId]);
        ledManager.show();
        soundManager.playTouchSound(playScaleNoteId);
        lastNoteTime = now;
        waitingPause = false;
    }
}

#endif
