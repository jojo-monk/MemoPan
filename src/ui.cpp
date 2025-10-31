#include "Debug.h"
#include "config.h"
#include "ui.h"
#include <U8g2lib.h>

// === Variables globales ===
uint8_t num_joueur = 1;
uint16_t difficult_id = 0;
uint16_t son_id = 0;
uint16_t gamme_id = 0;
uint16_t note_id = 0;
uint8_t lumiere = 40;
uint16_t vitesse_id = 0;
uint8_t volume = 8;
uint16_t clavier_id = 0;
uint8_t highScoreValue = 0;
uint8_t drywet = 3;
uint8_t tremo = 3;
uint8_t chor = 3;
uint8_t moog = 3;
bool arpegOn = false;
uint8_t tempo = 120;
uint16_t modeArp_id = 0;
uint16_t stepInterval_id = 3;
uint8_t batteryLevel = 75;  // exemple, de 0 à 100
int8_t octave = 0;  // Transposition d'octave, -2 à +2
//String highScoreStr = String(highScoreValue);
char highScoreStr[6];  // buffer pour le score (max 3 chiffres + '\0')
const int8_t octaveMin = -2; // Limite basse de la transposition d'octave
const int8_t octaveMax = 2;  // Limite haute de la transposition d'octave


// Variables d'état pour les actions du menu
bool gameStartRequested = false;
bool testLedsRequested = false;
bool testSonsRequested = false;
bool saveRequested = false;

// === Objets ===
MUIU8G2 mui;
extern U8G2 u8g2;

// === Listes ===
char presetNames[NUM_PRESETS][MAX_PRESET_NAME_LEN] = {};
bool presetNamesLoaded = false;
const char *modeArp[] = { "UP", "DOWN", "UP/DOWN", "RANDOM" };
const char *stepInterval[] = { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32" };
const char *difficult[] = { "Facile", "Moyen", "Difficile", "Extreme" };
const char *son[] = { "Lead Synth", "Deep Bass", "Strings", "Bell", "Wobble", "E.Piano", 
  "Pad", "Sine", "Square", "Mario", "HandPan", "8b-Lead", "Kick", "Snare", "Evolving pad", 
  "FM Bass", "Laser", "Metal", "8b-Arp", "HiHat", "E-Organ", "Glitch", "Siren", 
  "Bass", "Coin", "Drone", "Explose", "E-Pluck", "Wind", "Pluck", "Boom", "Drums" };
const char *gamme[] = { "Majeur", "Mineur", "Chromatique", "Penta Maj", "Penta min", "Akebono", "Hijaz", "ton-ton", "diminue" };
const char *note[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
const char *vitesse[] = { "Lent", "Moyen", "Rapide", "+ Rapide" };
const char *clavier[] = { "Normal", "HandPan", "Inverse", "Random", "Spirale" };

// === Fonctions callback pour les listes ===
uint16_t difficult_name_list_get_cnt(void*) { return sizeof(difficult)/sizeof(*difficult); }
const char* difficult_name_list_get_str(void*, uint16_t index) { return difficult[index]; }
uint16_t son_name_list_get_cnt(void*) { 
  if (presetNamesLoaded) {
      Serial.print("CNT = ");
      Serial.println(NUM_PRESETS);
    return NUM_PRESETS;
  }
  return sizeof(son)/sizeof(*son);  // Toujours retourner le nombre correct
}

const char* son_name_list_get_str(void*, uint16_t index) { 
    if (index >= NUM_PRESETS) {
        return "Error";
    }
    
    // Si les noms sont chargés depuis la flash, les utiliser
    if (presetNamesLoaded) { 
        return presetNames[index];
    }
    
    
    // Sinon, utiliser les noms par défaut
    Serial.println("presetName par défaut (son[])");
    return son[index];
}
// uint16_t son_name_list_get_cnt(void*) { return sizeof(son)/sizeof(*son); }
// const char* son_name_list_get_str(void*, uint16_t index) { 
//   if (presetNamesLoaded && index < NUM_PRESETS) {
//         // Retourner le nom dynamique
//         return presetNames[index].c_str();
//   }
//   return son[index]; }

uint16_t gamme_name_list_get_cnt(void*) { return sizeof(gamme)/sizeof(*gamme); }
const char* gamme_name_list_get_str(void*, uint16_t index) { return gamme[index]; }

uint16_t note_name_list_get_cnt(void*) { return sizeof(note)/sizeof(*note); }
const char* note_name_list_get_str(void*, uint16_t index) { return note[index]; }

uint16_t vitesse_name_list_get_cnt(void*) { return sizeof(vitesse)/sizeof(*vitesse); }
const char* vitesse_name_list_get_str(void*, uint16_t index) { return vitesse[index]; }

uint16_t clavier_name_list_get_cnt(void*) { return sizeof(clavier)/sizeof(*clavier); }
const char* clavier_name_list_get_str(void*, uint16_t index) { return clavier[index]; }

uint16_t modeArp_name_list_get_cnt(void*) { return sizeof(modeArp)/sizeof(*modeArp); }
const char* modeArp_name_list_get_str(void*, uint16_t index) { return modeArp[index]; }

uint16_t stepInterval_name_list_get_cnt(void*) { return sizeof(stepInterval)/sizeof(*stepInterval); }
const char* stepInterval_name_list_get_str(void*, uint16_t index) { return stepInterval[index]; }


uint8_t mui_hrule(mui_t *ui, uint8_t msg) {
  if ( msg == MUIF_MSG_DRAW ) {
      u8g2.drawHLine(0, mui_get_y(ui), u8g2.getDisplayWidth());
  }
  return 0;
}

void startGame() {
  gameStartRequested = true;
  //DEBUG_INFO_UI("Starting game...");

}

uint8_t exit_button_cb(mui_t *ui, uint8_t msg) {
  if (msg == MUIF_MSG_CURSOR_SELECT) {
    mui.gotoForm(1, 0);         // retour au menu principal (form 1)
  }
  return 0;
}

uint8_t save_button_cb(mui_t *ui, uint8_t msg) {
  switch(msg) {
    case MUIF_MSG_DRAW:
      mui_u8g2_draw_button_utf(ui, 
        mui_u8g2_get_pi_flags(ui),
        mui_get_x(ui), 
        mui_get_y(ui),
        50,  // width parameter
        "save");
      break;
      
    case MUIF_MSG_CURSOR_SELECT:
      saveRequested = true;
      DEBUG_INFO_UI("Save button pressed!");
      break;
      
    case MUIF_MSG_CURSOR_ENTER:
    case MUIF_MSG_CURSOR_LEAVE:
      return 1;
  }
  return 0;
}




void handleMenuSelection() {
  uint8_t currentForm = mui.getCurrentFormId();
  uint8_t currentField = mui.getCurrentCursorFocusPosition();
  
  
  DEBUG_VAR_UI("Menu selection - Form: ", currentForm);
  DEBUG_VAR_UI(", Field: ", currentField);
  
  
}
void updateHighScoreStr() {
  snprintf(highScoreStr, sizeof(highScoreStr), "%u", highScoreValue);
}

uint8_t mui_highscore_display(mui_t *ui, uint8_t msg) {
  if (msg == MUIF_MSG_DRAW) {
    //updateHighScoreStr();  // Met à jour la chaîne
    u8g2.setFont(u8g2_font_helvR08_tr);  // ou la police que vous voulez
    u8g2.drawStr(mui_get_x(ui), mui_get_y(ui), highScoreStr);
  }
  return 0;
}

// === Widgets ===
muif_t muif_list[] = {
  MUIF_U8G2_FONT_STYLE(0, u8g2_font_streamline_interface_essential_other_t),

  //MUIF_U8G2_FONT_STYLE(1, u8g2_font_helvB08_tr),
  MUIF_U8G2_FONT_STYLE(2, u8g2_font_streamline_entertainment_events_hobbies_t),
  MUIF_U8G2_FONT_STYLE(3, u8g2_font_streamline_music_audio_t),
  MUIF_U8G2_FONT_STYLE(4, u8g2_font_streamline_computers_devices_electronics_t),
  MUIF_U8G2_FONT_STYLE(5, u8g2_font_streamline_interface_essential_audio_t),
  MUIF_U8G2_FONT_STYLE(6, u8g2_font_streamline_interface_essential_setting_t),
  MUIF_U8G2_FONT_STYLE(7, u8g2_font_streamline_interface_essential_loading_t),
  MUIF_U8G2_FONT_STYLE(8, u8g2_font_helvR08_tr),
  MUIF_U8G2_FONT_STYLE(9, u8g2_font_prospero_bold_nbp_tr),
  //MUIF_U8G2_FONT_STYLE(9, u8g2_font_HelvetiPixelOutline_tr),
  MUIF_U8G2_FONT_STYLE(1, u8g2_font_siji_t_6x10),
  MUIF_U8G2_LABEL(),
  MUIF_RO("HR", mui_hrule),
  MUIF_RO("HS", mui_highscore_display),
  MUIF_RO("BT", mui_battery_display),
  //MUIF_RO("FM", mui_form_monitor),
  MUIF_RO("GP",mui_u8g2_goto_data),
  MUIF_RO("AS", arpeg_smart_label),
  MUIF_VARIABLE("ST", NULL, mui_u8g2_btn_exit_wm_fi),
  //MUIF_VARIABLE("ID",&variable,mui_u8g2_u8_opt_line_wa_mud_pi)
  //MUIF_RO("ST", startGame),
  //MUIF_BUTTON("ST", start_button_cb),
  MUIF_BUTTON("JE", mui_u8g2_btn_goto_wm_fi),
  MUIF_BUTTON("SY", mui_u8g2_btn_goto_wm_fi),
  MUIF_BUTTON("AG", mui_u8g2_btn_goto_wm_fi),
  MUIF_BUTTON("FX", mui_u8g2_btn_goto_wm_fi),
  MUIF_BUTTON("RE", mui_u8g2_btn_goto_wm_fi),
  MUIF_U8G2_U8_MIN_MAX("NV", &num_joueur, 1, 10, mui_u8g2_u8_min_max_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX("LI", &lumiere, 0, 100, mui_u8g2_u8_min_max_wm_mud_pi),
  MUIF_U8G2_S8_MIN_MAX("OC", &octave, octaveMin, octaveMax, mui_u8g2_s8_min_max_wm_mud_pi),
  //MUIF_U8G2_U8_MIN_MAX_STEP_WIDTH("LI", &lumiere, 0, 100, 1, 30, MUI_MMS_2X_BAR|MUI_MMS_SHOW_VALUE, mui_u8g2_u8_bar_wm_mud_pi),
  MUIF_U8G2_U16_LIST("DA", &difficult_id, NULL, difficult_name_list_get_str, difficult_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U16_LIST("SA", &son_id, NULL, son_name_list_get_str, son_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U16_LIST("GA", &gamme_id, NULL, gamme_name_list_get_str, gamme_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U16_LIST("NA", &note_id, NULL, note_name_list_get_str, note_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U16_LIST("VI", &vitesse_id, NULL, vitesse_name_list_get_str, vitesse_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U16_LIST("CL", &clavier_id, NULL, clavier_name_list_get_str, clavier_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U16_LIST("MA", &modeArp_id, NULL, modeArp_name_list_get_str, modeArp_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U16_LIST("SI", &stepInterval_id, NULL, stepInterval_name_list_get_str, stepInterval_name_list_get_cnt, mui_u8g2_u16_list_line_wa_mud_pi),
  MUIF_U8G2_U8_MIN_MAX_STEP("VO", &volume, 0, 10, 1, MUI_MMS_2X_BAR|MUI_MMS_NO_WRAP|MUI_MMS_SHOW_VALUE, mui_u8g2_u8_bar_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX_STEP("RV", &drywet, 0, 10, 1, MUI_MMS_2X_BAR|MUI_MMS_NO_WRAP|MUI_MMS_SHOW_VALUE, mui_u8g2_u8_bar_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX_STEP("TR", &tremo, 0, 10, 1, MUI_MMS_2X_BAR|MUI_MMS_NO_WRAP|MUI_MMS_SHOW_VALUE, mui_u8g2_u8_bar_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX_STEP("FL", &chor, 0, 10, 1, MUI_MMS_2X_BAR|MUI_MMS_NO_WRAP|MUI_MMS_SHOW_VALUE, mui_u8g2_u8_bar_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX_STEP("MO", &moog, 0, 10, 1, MUI_MMS_2X_BAR|MUI_MMS_NO_WRAP|MUI_MMS_SHOW_VALUE, mui_u8g2_u8_bar_wm_mud_pi),

  MUIF_VARIABLE("AR",&arpegOn,mui_u8g2_u8_chkbox_wm_pi),
  MUIF_U8G2_U8_MIN_MAX("TP", &tempo, 60, 200, mui_u8g2_u8_min_max_wm_mud_pi),
  MUIF_BUTTON("SV", mui_u8g2_btn_goto_wm_fi),
  MUIF_BUTTON("OK", mui_u8g2_btn_goto_wm_fi),
  MUIF_BUTTON("ST", mui_u8g2_btn_goto_wm_fi),
  MUIF_BUTTON("EX", mui_u8g2_btn_back_wm_fi),
  MUIF_BUTTON("HI", mui_u8g2_btn_goto_wm_fi)
  
};

//MUIF_BUTTON("ST", start_button_cb),
//MUIF_BUTTON("EX", exit_button_cb),


// === Pages / Forms ===
fds_t fds_data[] =
MUI_FORM(1)
//MUI_STYLE(0)
//MUI_LABEL(10, 20, "TEST")
MUI_STYLE(2)                   // font icône
MUI_LABEL(10,35,"\x39")         // icône 🎲 
MUI_STYLE(3)
MUI_LABEL(50,35,"\x31")        // icône 🎹 
MUI_STYLE(7)
MUI_LABEL(90,35,"\x3b")        // icône
MUI_STYLE(5)
MUI_LABEL(25,95,"\x35")         // icône 🎛 
MUI_STYLE(6)
MUI_LABEL(80,95,"\x33")        // icône ⚙
MUI_XY("BT", 110, 10)
// MUI_STYLE(1)
// MUI_LABEL(110,120,"\xee\x89\x82")
//MUI_STYLE(1)
//MUI_LABEL(20, 30, "Jeu")
MUI_XYAT("JE",20,51,10, "Jeu")
MUI_XYAT("SY",60,51,12, "Synth")
MUI_XYAT("AG",100,51,17, "Arpege")
MUI_XYAT("FX",35,111,15, "Effets")
MUI_XYAT("RE",90,111,14, "Reglages")
//MUI_LABEL(38,71,"Reglages")    // texte sous l'icône


MUI_FORM(10)
MUI_STYLE(2)                   // font icône
MUI_LABEL(6,21,"\x39")
MUI_STYLE(9)
MUI_LABEL(30, 21, "Jeu")
MUI_XY("HR", 0,24)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
MUI_STYLE(8)
MUI_LABEL(10,60, "nb joueurs: ")
//MUI_LABEL(5,36, "Bar:")
MUI_LABEL(10,80, "Difficulte: ")
MUI_XY("NV", 70, 60)
//MUI_XY("NB", 50, 36)
MUI_XYA("DA", 65, 80, 44)
MUI_XYAT("EX", 20, 110, 1, "Retour")
MUI_XYAT("HI", 68, 110, 20, "Record")
MUI_XYAT("ST", 110, 110, 16, "Start") 
 

MUI_FORM(12)
MUI_STYLE(3)
MUI_LABEL(6,21,"\x31")
MUI_STYLE(9)
MUI_LABEL(30, 21, "Synthe")
MUI_XY("HR", 0,24)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
MUI_STYLE(8)
MUI_LABEL(5,35,"Son:")
MUI_LABEL(5,50,"Gamme:")
MUI_LABEL(5,65,"Note:")
MUI_LABEL(5,80,"Clavier:")
MUI_LABEL(5,95,"Octave:")

//MUI_LABEL(5,55,"← Retour")
MUI_XYA("SA",60,35,40)
MUI_XYA("GA",60,50,40)
MUI_XYA("NA",60,65,40)
MUI_XYA("CL",60,80,40)
MUI_XYA("OC",60,95,40)

//MUI_XYAT("OK", 114, 60, 1, " Ok ") 
MUI_XYAT("EX", 20, 110, 1, "Retour") 
//MUI_XYA("GC",5,55,3)

MUI_FORM(14)
MUI_STYLE(6)
MUI_LABEL(6,21,"\x33")
MUI_STYLE(9)
MUI_LABEL(30,21,"Reglages")
MUI_XY("HR", 0,24)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
MUI_STYLE(8)
MUI_LABEL(10,50,"Vitesse:")
MUI_LABEL(10,65,"Lumiere:")
MUI_LABEL(10,80,"Volume:")
//MUI_LABEL(5,62," Retour ")
MUI_XY("VI",60,50)
MUI_XY("LI",60,65)
MUI_XY("VO",60,80)
//MUI_XYAT("OK", 110, 110, 1, "Ok")
MUI_XYAT("EX", 20, 110, 1, "Retour")
MUI_XYAT("SV", 110, 110, 18, "Save")
 
//MUI_XYA("GC",5,50,2)

MUI_FORM(15)
MUI_STYLE(5)
MUI_LABEL(6,21,"\x35")
MUI_STYLE(9)
MUI_LABEL(30,21,"Effets")
MUI_XY("HR", 0, 24)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
MUI_STYLE(8)
MUI_LABEL(10,40,"Reverb:")
MUI_LABEL(10,55,"Tremolo:")
MUI_LABEL(10,70,"Chorus:")
MUI_LABEL(10,85,"Filtre:")
//MUI_LABEL(5,62," Retour ")
MUI_XY("RV",60,40)
MUI_XY("TR",60,55)
MUI_XY("FL",60,70)
MUI_XY("MO",60,85)
//MUI_XYAT("OK", 110, 110, 1, "Ok")
//MUI_XYAT("SV", 72, 60, 18, " Save ")
MUI_XYAT("EX", 20, 110, 1, "Retour") 
//MUI_XYA("GC",5,50,2)

MUI_FORM(16)
MUI_STYLE(1)
MUI_LABEL(5,10,"MEMO-PAN")
MUI_XY("HR",0,11)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
MUI_STYLE(8)
MUI_LABEL(5,25,"Joueur:")
MUI_LABEL(5,35,"Score:")

MUI_FORM(17)
MUI_STYLE(7)
MUI_LABEL(6,21,"\x3b")
MUI_STYLE(9)
MUI_LABEL(30, 21, "Arpegiateur")
MUI_XY("HR", 0,24)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
//MUI_STYLE(0)
//MUI_LABEL(5,21, "\x41")  // Texte fixe
MUI_XY("AR",60, 40)          // Checkbox
MUI_XY("AS",10, 40)
MUI_STYLE(8)
MUI_LABEL(10,65,"Mode:")
MUI_LABEL(10,80,"Intervalle:")
MUI_LABEL(10,95,"Tempo:")
//MUI_XY("AR",60, 21)
//MUI_AUX("AS")
MUI_XY("MA",60, 65)
MUI_XY("SI",60, 80)
MUI_XY("TP",60, 95)
MUI_XYAT("EX", 20, 110, 1, "Retour")
//MUI_XYAT("OK", 110, 110, 1, " Ok ")

MUI_FORM(18)
MUI_STYLE(6)
MUI_LABEL(6,21,"\x33")
MUI_STYLE(9)
MUI_LABEL(30,21,"MEMO-PAN")
MUI_XY("HR",0,24)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
MUI_STYLE(8)
MUI_LABEL(5,65,"Sauvegarde en cours...")
//MUI_LABEL(5,35,"Score:")

MUI_FORM(20)
MUI_STYLE(2)                   // font icône
MUI_LABEL(6,21,"\x39")
MUI_STYLE(9)
MUI_LABEL(30,21,"MEMO-PAN")
MUI_XY("HR",0,24)
MUI_XY("BT", 110, 10)
//MUI_XY("FM", 0, 0)
MUI_STYLE(9)
MUI_LABEL(40,60,"HIGHSCORE")
MUI_XY("HS", 60, 80)
MUI_XYAT("EX", 20, 110, 10, "Retour")
;

// === Fonctions d'interface ===
void ui_init(U8G2 &u8g2) {
  DEBUG_INFO_UI("Initializing UI...");
  
  // Vérifier que les données sont correctes
  size_t muif_count = sizeof(muif_list)/sizeof(muif_t);
  DEBUG_VAR_UI("muif_list size: ", muif_count);
  
  strcpy(highScoreStr, "0");
  mui.begin(u8g2, fds_data, muif_list, muif_count);
  mui.gotoForm(1, 0);
  
  // Debug du formulaire actuel
  DEBUG_VAR_UI("Current form after init: ", mui.getCurrentFormId());
  DEBUG_VAR_UI("Current cursor position: ", mui.getCurrentCursorFocusPosition());
  DEBUG_INFO_UI("UI initialized");
}

void ui_draw() {
  // Debug occasionnel
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) {
    DEBUG_VAR_UI("Drawing form: ", mui.getCurrentFormId());
    DEBUG_VAR_UI(", cursor at: ", mui.getCurrentCursorFocusPosition());
    lastDebug = millis();
  }
  updateHighScoreStr();
  mui.draw();
}

uint8_t mui_battery_display(mui_t *ui, uint8_t msg) {
  if (msg == MUIF_MSG_DRAW) {
    int x = mui_get_x(ui);
    int y = mui_get_y(ui);

    // Dessin batterie (boîtier)
    u8g2.setFont(u8g2_font_siji_t_6x10);
    switch(batteryLevel) {
      case 0 ... 10: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x82");
        u8g2.drawUTF8(mui_get_x(ui) - 6, mui_get_y(ui), "\xE2\x10");

      break; // Vide
      case 11 ... 20: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x83");
        u8g2.drawUTF8(mui_get_x(ui) - 6, mui_get_y(ui), "\xE2\x10");
      break; // Faible
      case 21 ... 30: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x84");
        u8g2.drawUTF8(mui_get_x(ui) - 6, mui_get_y(ui), "\xE0\xAF");
      break; // Moyen
      case 31 ... 40: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x85");
      break; // Plein
      case 41 ... 50: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x86");
      break; // Très plein
      case 51 ... 60: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x87");
      break; // Complet
      case 61 ... 70: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x88");
      break; // Full
      case 71 ... 80: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x89");
      break; // Full+
      case 81 ... 90: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x8a");
      break; // Full++
      case 91 ... 100: 
        u8g2.drawUTF8(mui_get_x(ui), mui_get_y(ui), "\xee\x89\x8b");
      break; // Full+++
    }
    
    // Texte pourcentage
    char buf[6];
    snprintf(buf, sizeof(buf), "%d%%", batteryLevel);
    u8g2.setFont(u8g2_font_helvR08_tr);
    u8g2.drawStr(x-22, y, buf);
  }
  return 0;
}


uint8_t arpeg_smart_label(mui_t *ui, uint8_t msg) {
    static uint8_t last_state = 255;
    
    switch(msg) {
        case MUIF_MSG_DRAW:
            if (arpegOn != last_state) {
                u8g2_t *u8g2 = mui_get_U8g2(ui);
                
                DEBUG_INFO_UI("Drawing arpeg label");
                
                // UTILISER LES COORDONNÉES DE MUI (60, 21) au lieu de (5, 21)
                //u8g2_SetDrawColor(u8g2, 0);
                //u8g2_DrawBox(u8g2, 60, 13, 80, 10); // Effacer à partir de x=60
                
                u8g2_SetDrawColor(u8g2, 1);
                //u8g2_SetFont(u8g2, u8g2_font_helvR08_tr);
                
                if (arpegOn) {
                    DEBUG_INFO_UI("Arpeggiator ON");
                    u8g2_SetFont(u8g2, u8g2_font_helvR08_tr);
                    u8g2_DrawStr(u8g2, 80, 40, "ACTIF");
                    u8g2_SetFont(u8g2, u8g2_font_streamline_interface_essential_other_t);
                    u8g2_DrawStr(u8g2, 10, 50, "\x42");  // icône
                } else {
                    DEBUG_INFO_UI("Arpeggiator OFF");
                    u8g2_SetFont(u8g2, u8g2_font_helvR08_tr);
                    u8g2_DrawStr(u8g2, 80, 40, "INACTIF");
                    u8g2_SetFont(u8g2, u8g2_font_streamline_interface_essential_other_t);
                    u8g2_DrawStr(u8g2, 10, 50, "\x41");  // icône
                }
                
            }
            break;
    }
    
    return 0;
}

void ui_sendSelect() {
  mui.sendSelect();
}

void ui_nextField() {
  mui.nextField();
}

void ui_prevField() {
  mui.prevField();
}

bool ui_isFormActive() {
  return mui.isFormActive();
}

void ui_gotoForm() {
  mui.gotoForm(1, 0);
}

void ui_gotoForm(uint8_t form_id, uint8_t cursor_pos) {
  mui.gotoForm(form_id, cursor_pos);
}

uint8_t ui_getCurrentFormId() {
  return mui.getCurrentFormId();
}

uint8_t ui_getCurrentCursorPosition() {
  return mui.getCurrentCursorFocusPosition();
}

// === Fonctions pour récupérer l'état des paramètres ===
uint8_t ui_getNumJoueur() {
  return num_joueur;
}

uint16_t ui_getModeArp() {
  return modeArp_id;
}
uint16_t ui_getStepInterval() {
  return stepInterval_id;
}
bool ui_getArp() {
  return arpegOn;
}
uint8_t ui_getTempo() {
  return tempo;
}

uint16_t ui_getDifficulty() {
  return difficult_id;
}

uint16_t ui_getSoundType() {
  return son_id;
}

uint16_t ui_getScale() {
  return gamme_id;
}

uint16_t ui_getNote() {
  return note_id;
}

uint8_t ui_getClavier() {
  return clavier_id;
}

uint8_t ui_getBrightness() {
  return lumiere;
}

uint16_t ui_getSpeed() {
  return vitesse_id;
}

uint8_t ui_getVolume() {
  return volume;
}

uint8_t ui_getReverbLevel() {
  return drywet;
}

uint8_t ui_getTremolo() {
  return tremo;
}

uint8_t ui_getChorus() {
  return chor;
}

uint8_t ui_getMoogFilter() {
  return moog;
}

uint8_t ui_getHighScore() {
  return highScoreValue;
}

int8_t ui_getOctaveShift() {
  return octave;
}

// Fonctions pour définir les valeurs (setters)

void ui_setOctaveShift(int8_t shift) {
  if (shift >= octaveMin && shift <= octaveMax) {
    octave = shift;
  }
}
void ui_setBatteryLevel(uint8_t level) {
  if (level <= 100) {
    batteryLevel = level;
  }
}

void ui_setHighScore(uint8_t value) {
  highScoreValue = value;
  updateHighScoreStr();
}

void ui_setModeArp(uint16_t value) {
  if (value < sizeof(modeArp)/sizeof(*modeArp)) {
    modeArp_id = value;
  }
}

void ui_setStepInterval(uint16_t value) {
  if (value < sizeof(stepInterval)/sizeof(*stepInterval)) {
    stepInterval_id = value;
  }
}

void ui_setTempo(uint8_t value) {
  if (value >= 60 && value <= 200) {
    tempo = value;
  }
}

void ui_setSpeed(uint16_t value) {
  if (value < sizeof(vitesse)/sizeof(*vitesse)) {
    vitesse_id = value;
  }
}

void ui_setBrightness(uint8_t value) {
  if (value <= 100) {
    lumiere = value;
  }
}

void ui_setVolume(uint8_t value) {
  if (value <= 10) {
    volume = value;
  }
}

void ui_setTremolo(uint8_t value) {
  if (value <= 10) {
    tremo = value;
  }
}

void ui_setChorus(uint8_t value) {
  if (value <= 10) {
    chor = value;
  }
}

void ui_setMoogFilter(uint8_t value) {
  if (value <= 10) {
    moog = value;
  }
}


void ui_setReverbLevel(uint8_t value) {
  if (value <= 10) {
    drywet = value;
  }
}

void ui_setSoundType(uint16_t value) {
  if (value < NUM_PRESETS) {
        son_id = value;
    }
}

void ui_setScale(uint16_t value) {
  if (value < sizeof(gamme)/sizeof(*gamme)) {
    gamme_id = value;
  }
}

void ui_setNote(uint16_t value) {
  if (value < sizeof(note)/sizeof(*note)) {
    note_id = value;
  }
}

void ui_setClavier(uint8_t value) {
  if (value < sizeof(clavier)/sizeof(*clavier)) {
    clavier_id = value;
  }
}

void ui_setArp(bool state) {
  arpegOn = state;
}

void ui_setAll(const uint8_t variables[15]) {
  for (int i = 0; i < 15; i++) {
    switch(i) {
      case 1:
        ui_setSoundType(variables[i]);
      break;
      case 2:
        ui_setScale(variables[i]);
      break;
      case 3:
        ui_setNote(variables[i]);
      break;
      case 4:
        ui_setClavier(variables[i]);
      break;
      case 5:
        ui_setSpeed(variables[i]);
      break;
      case 6:
        ui_setBrightness(variables[i]);
      break;
      case 7:
        ui_setVolume(variables[i]);
      break;
      case 8:
        ui_setReverbLevel(variables[i]);
      break;
      case 9:
        ui_setTremolo(variables[i]);
      break;
      case 10:
        ui_setChorus(variables[i]);
      break;
      case 11:
        ui_setArp(variables[i]);
      break;
      case 12:
        ui_setModeArp(variables[i]);
      break;
      case 13:
        ui_setTempo(variables[i]);
      break;
      case 14:
        ui_setStepInterval(variables[i]);
      break;
      case 15:
        ui_setOctaveShift((int8_t)variables[i]);
      break;
    }
    
  }
}

// === Fonctions pour vérifier les actions demandées ===


bool ui_isSaveRequested() {
  if (saveRequested) {
    saveRequested = false;
    return true;
  }
  return false;
}

bool ui_isGameStartRequested() {
  if (gameStartRequested) {
    gameStartRequested = false;
    return true;
  }
  return false;
}

bool ui_isTestLedsRequested() {
  if (testLedsRequested) {
    testLedsRequested = false;
    return true;
  }
  return false;
}

bool ui_isTestSonsRequested() {
  if (testSonsRequested) {
    testSonsRequested = false;
    return true;
  }
  return false;
}

