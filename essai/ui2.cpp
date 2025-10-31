#include "ui2.h"

// Identifiants des menus
enum {
    MENU_MAIN,
    MENU_JEU,
    MENU_SYNTH,
    MENU_ARPEGE,
    MENU_EFFETS,
    MENU_REGLAGES,
    MENU_HIGHSCORE,
    MENU_NONE = 0xFF
};
int num_joueur = 1;
int difficult_id = 0;
int son_id = 0;
int volume = 80;
int gamme_id = 2;
int note_id = 0;
int clavier_id = 0;
int highScoreValue = 0;
int drywet = 3;
int tremo = 3;
int chor = 3;
bool arp = false;
//int difficulty = 1;
bool editingNumber = false;
bool editingList = false;
int tempo = 120;
int modeArp_id = 0;
int stepInterval_id = 3;
int brightness = 60;
int speed_id = 1;
//const char *waveforms[] = {"Sine", "Square", "Saw"};
const char *modeArp[] = { "UP", "DOWN", "UP/DOWN", "RANDOM" };
const char *stepInterval[] = { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" };
const char *difficult[] = { "Facile", "Moyen", "Difficile", "Extreme" };
const char *son[] = { "Sine", "Saw", "Square", "HandPan", "Pluck", "Boom", "Drums" };
const char *gamme[] = { "Majeur", "Mineur", "Chromatique", "Penta Maj", "Penta min", "Akebono", "Hijaz", "ton-ton", "diminue" };
const char *note[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
const char *vitesse[] = { "Lent", "Moyen", "Rapide", "+ Rapide" };
const char *clavier[] = { "Normal", "HandPan", "Inverse", "Random", "Spirale" };

// MenuItem mainMenu[] = {
//     {.icon = 0, .label = "Play",  .type = MENU_TYPE_ACTION},
//     {.icon = 0, .label = "Volume",  .type = MENU_TYPE_INT, .value = 5, .min = 0, .max = 10},
//     {.icon = 0, .label = "Wave",  .type = MENU_TYPE_LIST, .value = 0, .max = 2, .list = waveforms},
//     {.icon = 0, .label = "FX On", .type = MENU_TYPE_CHECKBOX, .value = 1},
//     {.icon = 0, .label = "Config", .type = MENU_TYPE_ACTION}
// };


//Définition des menus
const MenuItem mainMenu[] = {
    {0x00F9, "Jeu",  MENU_JEU, ITEM_MENU},
    {0x0240, "Synth", MENU_SYNTH, ITEM_MENU},
    {0x019E, "Arp",  MENU_ARPEGE, ITEM_MENU},
    {0x01F8, "FX", MENU_EFFETS, ITEM_MENU},
    {0x01A9, "Score", MENU_HIGHSCORE, ITEM_MENU},
    {0x01D7, "Config", MENU_REGLAGES, ITEM_MENU},
};

const MenuItem jeuMenu[] = {
    {0, "Joueurs", MENU_NONE, ITEM_NUMBER, &num_joueur, 1, 10},
    {0, "Niveau",  MENU_NONE, ITEM_LIST, nullptr, 0, 0, .listIndex=&difficult_id, .listOptions=&difficult, .listSize=4},
    {0, "Start",  MENU_NONE, ITEM_BUTTON},
    {0, "Retour",  MENU_MAIN, ITEM_BACK}
};

const MenuItem synthMenu[] = {
    {0, "Son", MENU_NONE, ITEM_LIST, .listIndex=&son_id, .listOptions=&son, .listSize=7},
    {0, "Gamme", MENU_NONE, ITEM_LIST, .listIndex=&gamme_id, .listOptions=&gamme, .listSize=9},
    {0, "Note", MENU_NONE, ITEM_LIST, .listIndex=&note_id, .listOptions=&note, .listSize=12},
    {0, "Clavier", MENU_NONE, ITEM_LIST, .listIndex=&clavier_id, .listOptions=&clavier, .listSize=5},
    {0, "Retour", MENU_MAIN, ITEM_BACK},
};

const MenuItem arpegeMenu[] = {
    {0, "ON/OFF", MENU_NONE, ITEM_CHECKBOX, &arp},
    {0, "Intervalle", MENU_NONE, ITEM_LIST, .listIndex=&stepInterval_id, .listOptions=&stepInterval, .listSize=6},
    {0, "Tempo", MENU_NONE, ITEM_NUMBER, &tempo, 30, 200},
    {0, "Mode",  MENU_NONE, ITEM_LIST, .listIndex=&modeArp_id, .listOptions=&modeArp, .listSize=4},
    {0, "Retour", MENU_MAIN, ITEM_BACK}
};

const MenuItem effetsMenu[] = {
    {0,"Reverb", MENU_NONE, ITEM_BAR},
    {0,"Tremolo", MENU_NONE, ITEM_BAR},
    {0,"Chorus", MENU_NONE, ITEM_BAR},
    {0,"Retour", MENU_MAIN, ITEM_BACK}
};

const MenuItem reglagesMenu[] = {
    {0, "LEDS", MENU_NONE, ITEM_BAR, &brightness, 0, 100},
    {0, "Vitesse", MENU_NONE, ITEM_BAR, .listIndex=&speed_id, .listOptions=&vitesse, .listSize=4},
    {0, "Volume", MENU_NONE, ITEM_BAR, &volume, 0, 100},
    {0, "Retour", MENU_MAIN, ITEM_BACK}
};



// Tableau de pointeurs vers les menus
const MenuItem* menus[] = {
    mainMenu,
    jeuMenu,
    synthMenu,
    arpegeMenu,
    effetsMenu,
    reglagesMenu,
};

const uint8_t menuSizes[] = {
    sizeof(mainMenu) / sizeof(MenuItem),
    sizeof(jeuMenu) / sizeof(MenuItem),
    sizeof(synthMenu) / sizeof(MenuItem),
    sizeof(arpegeMenu) / sizeof(MenuItem),
    sizeof(effetsMenu) / sizeof(MenuItem),
    sizeof(reglagesMenu) / sizeof(MenuItem),
};

GridSize calcGrid(int n) {
    GridSize g;

    if (n <= 3) {
        g.rows = 1;
        g.cols = n;
    }
    else if (n == 4) {
        g.rows = 2;
        g.cols = 2;
    }
    else {
        int cols = (int)ceil(sqrt(n));
        int rows = (n + cols - 1) / cols;
        g.rows = rows;
        g.cols = cols;
    }

    return g;
}


// Définition des cases : x, y, largeur, hauteur
struct Case {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
};

Case grid[6] = {
    {  0,  0, 42, 64}, // case 0
    { 42,  0, 42, 64}, // case 1
    { 84,  0, 44, 64}, // case 2
    {  0, 64, 42, 64}, // case 3
    { 42, 64, 42, 64}, // case 4
    { 84, 64, 44, 64}  // case 5
};

// === Constructeur ===
UI2::UI2(U8G2 &disp, int pinA, int pinB, int pinBtn)
: u8g2(disp), rotary(pinA, pinB), button() {
    // configure pin et attach du Bounce dans le corps du constructeur
    pinMode(pinBtn, INPUT_PULLUP);          // bouton vers la masse (les valeurs peuvent varier selon ton montage)
    button.attach(pinBtn);                  // on attache après pinMode
    button.interval(10);                    // debounce 10 ms

    lastEncoderRaw = rotary.read();
    lastEncoderDetent = detentFromRaw(lastEncoderRaw);
}

// === Initialisation ===
void UI2::init() {
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_6x12_tr);
}

// === Gestion input ===

void UI2::updateInput() {
    long raw = rotary.read();
    long detent = detentFromRaw(raw);

    // --- Gestion rotation ---
    if (!editingNumber) {
        if (detent != lastEncoderDetent) {
            if (detent > lastEncoderDetent) nextField();
            else prevField();
            lastEncoderDetent = detent;
        }
    } else {
        // Mode édition nombre ou liste
        MenuItem &item = (MenuItem&)menus[current_menu][cursor_position];

        if (detent != lastEncoderDetent) {
            if (item.type == ITEM_NUMBER && item.value) {
                if (detent > lastEncoderDetent && *item.value < item.maxVal) {
                    (*item.value)++;
                } else if (detent < lastEncoderDetent && *item.value > item.minVal) {
                    (*item.value)--;
                }
            }
            else if (item.type == ITEM_LIST && item.listValue) {
                if (detent > lastEncoderDetent) {
                    *item.listValue = (*item.listValue + 1) % item.listSize;
                } else {
                    *item.listValue = (*item.listValue - 1 + item.listSize) % item.listSize;
                }
            }
            lastEncoderDetent = detent;
        }
    }

    // --- Gestion bouton ---
    button.update();
    if (button.fell()) {
        if (!editingNumber) {
            select();
        } else {
            editingNumber = false; // quitter édition
            editingList = false;
        }
    }

    lastEncoderRaw = raw;
}



// === Navigation ===
void UI2::nextField() {
    uint8_t max_items = menuSizes[current_menu];
    if (max_items > 0) cursor_position = (cursor_position + 1) % max_items;
}

void UI2::prevField() {
    uint8_t max_items = menuSizes[current_menu];
    if (max_items > 0) {
        if (cursor_position == 0) cursor_position = max_items - 1;
        else cursor_position--;
    }
}

void UI2::select() {
    MenuItem &item = (MenuItem&)menus[current_menu][cursor_position];

    if (item.type == ITEM_NUMBER) {
        editingNumber = true;
    } 
    else if (item.type == ITEM_LIST) {
        editingList = true;
    }
    else if (item.nextMenu != MENU_NONE) {
        current_menu = item.nextMenu;
        cursor_position = 0;
    } else {
        back();
    }
}


void UI2::back() {
    current_menu = MENU_MAIN;
    cursor_position = 0;
}

// === Dessin ===
void UI2::draw() {
    u8g2.clearBuffer();

    switch (current_menu) {
        case MENU_MAIN:      drawMainMenu(); break;
        case MENU_JEU:       drawJeuMenu(); break;
        case MENU_SYNTH:     drawSynthMenu(); break;
        case MENU_ARPEGE:    drawArpegeMenu(); break;
        case MENU_EFFETS:    drawEffetsMenu(); break;
        case MENU_REGLAGES:  drawReglagesMenu(); break;
        case MENU_HIGHSCORE: break;
    }

    u8g2.sendBuffer();
}

void UI2::drawIconWithText(int cellule, uint16_t icon, const char* text, bool selected) {
  // Si sélectionné → cadre autour
    uint16_t w = u8g2.getStrWidth(text);
    int iconX = grid[cellule].x + (grid[cellule].w - 21) / 2;
    int iconY = grid[cellule].y + (grid[cellule].h - 33) / 2 + 21;
    int textX = grid[cellule].x + (grid[cellule].w - w)/ 2; // pour drawStr centré horizontalement
    int textY = iconY + 2 + 10; // en bas de la zone icône+gap+texte
  if (selected) {
    u8g2.drawFrame(grid[cellule].x, grid[cellule].y, grid[cellule].w, grid[cellule].h);
  }
  if (icon != 0) {
  // Icône
    u8g2.setFont(u8g2_font_streamline_all_t);  
    u8g2.drawGlyph(iconX, iconY, icon);
  }

  // Texte (sous l’icône)
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(textX, textY, text);
}

void drawMenuGrid(u8g2_t *u8g2, MenuItem *menu, int n, int selected) {
    int screenW = u8g2_GetDisplayWidth(u8g2);
    int screenH = u8g2_GetDisplayHeight(u8g2);

    GridSize g = calcGrid(n);
    int cellW = screenW / g.cols;
    int cellH = screenH / g.rows;

    for (int i = 0; i < n; i++) {
        int row = i / g.cols;
        int col = i % g.cols;
        int x = col * cellW;
        int y = row * cellH;

        // cadre sélection
        if (i == selected) {
            u8g2_SetDrawColor(u8g2, 1);
            u8g2_DrawBox(u8g2, x, y, cellW, cellH);
            u8g2_SetDrawColor(u8g2, 0);
        } else {
            u8g2_SetDrawColor(u8g2, 1);
            u8g2_DrawFrame(u8g2, x, y, cellW, cellH);
        }

        // texte centré
        int textX = x + 4;
        int textY = y + cellH / 2;

        // icône
        if (menu[i].icon) {
            u8g2_DrawStr(u8g2, textX, textY, menu[i].icon);
            textX += 12; // décaler après icône
        }

        // label
        if (menu[i].label) {
            u8g2_DrawStr(u8g2, textX, textY, menu[i].label);
        }

        // valeur selon type
        switch (menu[i].type) {
            case MENU_TYPE_INT: {
                char buf[8];
                sprintf(buf, "%d", menu[i].value);
                u8g2_DrawStr(u8g2, x + cellW - 20, textY, buf);
                break;
            }
            case MENU_TYPE_LIST: {
                if (menu[i].list) {
                    u8g2_DrawStr(u8g2, x + cellW - 40, textY,
                                 menu[i].list[menu[i].value]);
                }
                break;
            }
            case MENU_TYPE_CHECKBOX: {
                u8g2_DrawStr(u8g2, x + cellW - 20, textY,
                             menu[i].value ? "[x]" : "[ ]");
                break;
            }
            default:
                break;
        }

        // reset couleur
        u8g2_SetDrawColor(u8g2, 1);
    }
}


void UI2::drawMenu(uint8_t menuId) {
    for (uint8_t i = 0; i < menuSizes[menuId]; i++) {
        const MenuItem &item = menus[menuId][i];
        int y = 12 + i * 12;

        bool selected = (cursor_position == i);
        bool editing = (selected && ((item.type == ITEM_NUMBER && editingNumber) ||
                                     (item.type == ITEM_LIST && editingList)));

        if (editing) {
            u8g2.setDrawColor(1); // texte blanc
            u8g2.drawBox(0, y - 10, 128, 12); // fond noir
            u8g2.setDrawColor(0);
        } else if (selected) {
            u8g2.drawFrame(0, y - 10, 128, 12);
            u8g2.setDrawColor(1);
        } else {
            u8g2.setDrawColor(1);
        }

        u8g2.setCursor(14, y);

        if (item.type == ITEM_NUMBER && item.value) {
            char buf[20];
            sprintf(buf, "%s: %d", item.label, *item.value);
            u8g2.print(buf);
        }
        else if (item.type == ITEM_LIST && item.listValue && item.listOptions) {
            char buf[30];
            sprintf(buf, "%s: %s", item.label, item.listOptions[*item.listValue]);
            u8g2.print(buf);
        }
        else {
            u8g2.print(item.label);
        }

        u8g2.setDrawColor(1);
    }
}


// void UI2::drawMenu(uint8_t menuId) {
//     for (uint8_t i = 0; i < menuSizes[menuId]; i++) {
//         const MenuItem &item = menus[menuId][i];
//         u8g2.setCursor(14, 12 + i * 12);

//         if (item.type == ITEM_NUMBER && item.value) {
//             char buf[20];
//             sprintf(buf, "%s: %d", item.label, *item.value);
//             u8g2.print(buf);
//         } else {
//             drawIconWithText(i, item.icon, item.label, cursor_position == i);
//         }

//         if (cursor_position == i) {
//             if (editingNumber && item.type == ITEM_NUMBER)
//                 u8g2.setDrawColor(2); // ou 3 selon l’effet que tu veux
//                 u8g2.drawBox(grid[i].x, grid[i].y, grid[i].w, grid[i].h); // fond noir
//                 u8g2.setDrawColor(0);             // texte blanc
//         } else {
//                 u8g2.setDrawColor(1); // normal
//                 u8g2.drawFrame(grid[i].x, grid[i].y, grid[i].w, grid[i].h); // simple cadre
//         }
//     }
// }

void UI2::drawMainMenu()      { drawMenuGrid(MENU_MAIN); }
void UI2::drawJeuMenu()       { drawMenuGrid(MENU_JEU); }
void UI2::drawSynthMenu()     { drawMenuGrid(MENU_SYNTH); }
void UI2::drawArpegeMenu()    { drawMenuGrid(MENU_ARPEGE); }
void UI2::drawEffetsMenu()    { drawMenuGrid(MENU_EFFETS); }
void UI2::drawReglagesMenu()  { drawMenuGrid(MENU_REGLAGES); }

// === Getters / Setters ===
uint8_t UI2::ui_getNumJoueur()        { return numJoueur; }
uint16_t UI2::ui_getDifficulty()      { return difficulty; }
uint16_t UI2::ui_getSoundType()       { return soundType; }
uint16_t UI2::ui_getScale()           { return scale; }
uint16_t UI2::ui_getNote()            { return note; }
uint8_t UI2::ui_getBrightness()       { return brightness; }
uint16_t UI2::ui_getSpeed()           { return speed; }
uint8_t UI2::ui_getVolume()           { return volume; }
uint8_t UI2::ui_getReverbLevel()      { return reverbLevel; }
uint8_t UI2::ui_getClavier()          { return clavier; }
uint8_t UI2::ui_getHighScore()        { return highScore; }
uint8_t UI2::ui_getTremolo()          { return tremolo; }
uint8_t UI2::ui_getChorus()           { return chorus; }
bool UI2::ui_getArp()                 { return arp; }
uint16_t UI2::ui_getStepInterval()    { return stepInterval; }
uint16_t UI2::ui_getModeArp()         { return modeArp; }
uint8_t UI2::ui_getTempo()            { return tempo; }

void UI2::ui_setHighScore(uint8_t value)     { highScore = value; }
void UI2::ui_setSpeed(uint16_t value)        { speed = value; }
void UI2::ui_setBrightness(uint8_t value)    { brightness = value; }
void UI2::ui_setVolume(uint8_t value)        { volume = value; }
void UI2::ui_setSoundType(uint16_t value)    { soundType = value; }
void UI2::ui_setScale(uint16_t value)        { scale = value; }
void UI2::ui_setNote(uint16_t value)         { note = value; }
void UI2::ui_setClavier(uint8_t value)       { clavier = value; }
void UI2::ui_setReverbLevel(uint8_t value)   { reverbLevel = value; }
void UI2::ui_setTremolo(uint8_t value)       { tremolo = value; }
void UI2::ui_setChorus(uint8_t value)         { chorus = value; }
void UI2::ui_setModeArp(uint16_t value)        { modeArp = value; }
void UI2::ui_setStepInterval(uint16_t value)   { stepInterval = value; }
void UI2::ui_setTempo(uint8_t value)          { tempo = value; }

void UI2::ui_setAll(const uint8_t vars[11]) {
    numJoueur = vars[0];
    brightness = vars[1];
    volume = vars[2];
    reverbLevel = vars[3];
    clavier = vars[4];
    highScore = vars[5];
    tremolo = vars[6];
    chorus = vars[7];
    // speed, stepInterval, tempo can be set separately
}
