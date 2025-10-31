#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "config.h"

// === Déclarations des fonctions d'interface ===
void ui_init(U8G2 &u8g2);
void ui_draw();
void ui_sendSelect();
void ui_nextField();
void ui_prevField();
bool ui_isFormActive();
void ui_gotoForm();
void ui_gotoForm(uint8_t form_id, uint8_t cursor_pos);
uint8_t arpeg_smart_label(mui_t *ui, uint8_t msg);
uint8_t mui_battery_display(mui_t *ui, uint8_t msg);


// === Fonctions pour récupérer l'état du menu ===
uint8_t ui_getCurrentFormId();
uint8_t ui_getCurrentCursorPosition();

// === Fonctions pour récupérer les paramètres ===
uint8_t ui_getNumJoueur();
uint16_t ui_getDifficulty();
uint16_t ui_getSoundType();
uint16_t ui_getScale();
uint16_t ui_getNote();
uint8_t ui_getBrightness();
uint16_t ui_getSpeed();
uint8_t ui_getVolume();
uint8_t ui_getReverbLevel();
uint8_t ui_getClavier();
uint8_t ui_getHighScore();
uint8_t ui_getTremolo();
uint8_t ui_getChorus();
bool ui_getArp();
uint16_t ui_getStepInterval();
uint16_t ui_getModeArp();
uint8_t ui_getTempo();
int8_t ui_getOctaveShift();
uint8_t ui_getMoogFilter();

// Fonctions pour définir les valeurs
void ui_setOctaveShift(int8_t shift);
void ui_setHighScore(uint8_t value);
void ui_setSpeed(uint16_t value);
void ui_setBrightness(uint8_t value);
void ui_setVolume(uint8_t value);
void ui_setSoundType(uint16_t value);
void ui_setScale(uint16_t value);
void ui_setNote(uint16_t value);
void ui_setClavier(uint8_t value);
void ui_setAll(const uint8_t variables[15]);
void ui_setReverbLevel(uint8_t value);
void ui_setTremolo(uint8_t value);
void ui_setChorus(uint8_t value);
void ui_setModeArp(uint16_t value);
void ui_setStepInterval(uint16_t value);
void ui_setTempo(uint8_t value);
void ui_setBatteryLevel(uint8_t level);
void ui_setArp(bool state);
void ui_setMoogFilter(uint8_t value);


// Fonction pour la sauvegarde
bool ui_isSaveRequested();

// === Fonctions pour vérifier les actions demandées ===
bool ui_isGameStartRequested();
bool ui_isTestLedsRequested();
bool ui_isTestSonsRequested();

// === Fonction de gestion des sélections ===
void handleMenuSelection();

// === Variables globales (extern) ===
extern uint8_t num_joueur;
extern uint16_t difficult_id;
extern uint16_t son_id;
extern uint16_t gamme_id;
extern uint16_t note_id;
extern uint8_t lumiere;
extern uint16_t vitesse_id;
extern const char* son[];
extern char presetNames[NUM_PRESETS][MAX_PRESET_NAME_LEN];
extern bool presetNamesLoaded;


#endif // UI_H