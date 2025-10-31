#ifndef UI2_H
#define UI2_H

#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
//#include <SimpleRotary.h>
#include <Encoder.h>
#include <Bounce2.h>

enum MenuItemType {
    ITEM_MENU,
    ITEM_BACK,
    ITEM_NUMBER,
    ITEM_LIST,
    ITEM_CHECKBOX,  // case à cocher
    ITEM_BUTTON,
    ITEM_BAR
};

struct MenuItem {
    uint16_t icon;
    const char* label;
    uint8_t nextMenu;
    MenuItemType type = ITEM_MENU;
    int* value = nullptr;    // pointeur vers variable liée
    int minVal = 0;
    int maxVal = 100;
    // Pour ITEM_LIST
    int* listIndex = nullptr;         // pointeur vers l’index courant
    const char** listOptions = nullptr; // tableau de chaînes
    int listSize = 0;
};

typedef struct {
    int rows;
    int cols;
} GridSize;

GridSize calcGrid(int n);


class UI2 {
public:
    UI2(U8G2 &disp, int pinA, int pinB, int pinBtn);

    void init();
    void updateInput();
    void draw();

    // Getters
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

    // Setters
    void ui_setHighScore(uint8_t value);
    void ui_setSpeed(uint16_t value);
    void ui_setBrightness(uint8_t value);
    void ui_setVolume(uint8_t value);
    void ui_setSoundType(uint16_t value);
    void ui_setScale(uint16_t value);
    void ui_setNote(uint16_t value);
    void ui_setClavier(uint8_t value);
    void ui_setAll(const uint8_t variables[11]);
    void ui_setReverbLevel(uint8_t value);
    void ui_setTremolo(uint8_t value);
    void ui_setChorus(uint8_t value);
    void ui_setModeArp(uint16_t value);
    void ui_setStepInterval(uint16_t value);
    void ui_setTempo(uint8_t value);

private:
    U8G2 &u8g2;
    Encoder rotary;
    Bounce button = Bounce(); // pour le bouton de l’encodeur

    // Navigation
    uint8_t current_menu = 0;
    uint8_t cursor_position = 0;
    long lastEncoderRaw;     // <= définis ici
    long lastEncoderDetent;  // <= définis ici
    int detentDivider = 4; // ajuste selon ton encodeur
    long detentFromRaw(long rawVal) { return rawVal / detentDivider; }

    void nextField();
    void prevField();
    void select();
    void back();
    void selectField();

    void drawMenu(uint8_t menuId);
    void drawMenuGrid(uint8_t menuId, int screenW, int screenH, int selected);

    void drawMainMenu();
    void drawJeuMenu();
    void drawSynthMenu();
    void drawArpegeMenu();
    void drawEffetsMenu();
    void drawReglagesMenu();
    void drawIconWithText(int cellule, uint16_t icon, const char* text, bool selected);


    // Valeurs
    uint8_t numJoueur = 1;
    uint16_t difficulty = 1;
    uint16_t soundType = 0;
    uint16_t scale = 0;
    uint16_t note = 60;
    uint8_t brightness = 255;
    uint16_t speed = 100;
    uint8_t volume = 100;
    uint8_t reverbLevel = 0;
    uint8_t clavier = 0;
    uint8_t highScore = 0;
    uint8_t tremolo = 0;
    uint8_t chorus = 0;
    bool arp = false;
    uint16_t stepInterval = 100;
    uint16_t modeArp = 0;
    uint8_t tempo = 120;
};

#endif