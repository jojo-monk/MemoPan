#include "Debug.h"
#include "config.h"
#include <U8g2lib.h>
#include "TouchManager.h"
#include "LedManager.h"
#include "SoundManagerV4.h"
#include "MidiManagerV2.h"
#include <Encoder.h>
#include "ui.h"
#include "splash.h"
#include "SaveEprom.h"
#include "ArpegiateurV2.h"
#include "BatteryMonitor.h"
#include <Chrono.h>
#include "ScalePlayer.h"


TouchManager touchManager;
LedManager ledManager;
SoundManager soundManager;
SaveEprom eeprom;
Arpegiateur arpeg;
Chrono seqChrono;
Chrono affichageChrono;
ScalePlayer scalePlayer;


// Rotary encoder
Encoder rotaryEncoder(ENCODER_PIN_A, ENCODER_PIN_B);
long lastEncoderPosition = 0;

// Variables pour le bouton
unsigned long lastButtonPress = 0;
bool lastButtonState = HIGH;
const unsigned long DEBOUNCE_DELAY = 50;

BatteryMonitor battMonitor(BATTERY_PIN, 255, 51.0, 100.0); // Pin ADC, pas de pin shutdown, R1=51k, R2=100k

// Ecran
U8G2_SH1107_SEEED_128X128_F_4W_SW_SPI u8g2(
  U8G2_R1, /* clock=*/13, /* data=*/11, /* cs=*/10,
  /* dc=*/5, /* reset=*/9);

// ========================================
// ÉTATS DU JEU
// ========================================x
enum GameState {
  STATE_MENU,
  STATE_INIT_GAME,
  STATE_WAIT,          
  STATE_GAME,
  STATE_SCALE_PLAY,          
  STATE_PLAYER_LOOSE,  
  STATE_GAME_OVER,
  STATE_VICTORY     
  //STATE_TEST_LEDS,     // Test des LEDs
  //STATE_TEST_SONS      // Test des sons
};

enum SequenceState {
  SEQ_IDLE,
  SEQ_WAIT_START,
  SEQ_SHOW_STEP,
  SEQ_WAIT_END,
  SEQ_PAUSE_BETWEEN
};

// Variables d'état
SequenceState seqState = SEQ_IDLE;
int seqIndex = 0;
GameState currentState = STATE_MENU;
uint8_t is_redraw = 1;
uint8_t rotary_event = 0;
uint8_t push_event = 0;
bool toucheEtat[12] = { false };
bool systemReady = false;
unsigned long lastActivity = 0;
//static unsigned long lastCheck = 0;
int8_t previousNote = -1;

// Variables pour le jeu Simon
int gameSequence[100];
int gameLevel = 0;
int playerInput = 0;
bool gameInProgress = false;
bool newHighScore = false;
bool highScoreDisplayed = false;
float pressionSequence = 0.5; // Valeur de pression pour la séquence (moyenne)
uint8_t velocitySequence = 80;  // Vélocité pour la séquence
int gameTiming = 0;
unsigned long looseTime = 0;
unsigned long waitDisplayGameScreen = 0;
unsigned long victoryStartTime = 0;
uint8_t numPlayers = 0;
uint8_t player = 0;
uint8_t scores[NUM_MAX_PLAYER][2];
uint8_t currentScore[10] = { 0 };
uint8_t highScore = 0;
// Variables Son
uint8_t sonId = 0;
uint8_t gammeId = 2;
uint8_t noteId = 2;
uint8_t clavierId = 2;
uint8_t vitesseId = 0;
uint8_t gain = 8;
uint8_t reverbMix = 7;
uint8_t tremoloMix = 3;
uint8_t chorusMix = 3;
uint8_t noiseMix = 3;
uint8_t lastNote = 0;
uint8_t lastScale = 0;
uint16_t lastSoundType = 0;
bool arpEnabled = false;
uint8_t arpSpeed_id = 3; // index dans STEP_INTERVALS_MS
uint8_t arpSpeedBPM = 120; // BPM pour l'arpégiateur
uint8_t arpMode = 0; // 0=UP, 1=DOWN, 2=UP_DOWN, 3=RANDOM
bool muteSound = true;
// Variables LED
uint8_t intensiteLED = 20;
bool ledsOn = true;
uint8_t fxVictory = 0;
float maxPressure = 600.0f; // Pression max typique
int8_t octaveShift = 0;
float lastVoltage = 0;
const float VOLTAGE_THRESHOLD = 0.05; // 50mV de différence
unsigned long lastBatteryCheck = 0;
unsigned long lastInfoCheck = 0;
unsigned long lastAnalyse;
const unsigned long BATTERY_CHECK_INTERVAL = 10000; // Vérifier batterie toutes les 10s

uint8_t sauvegarde[15] = { highScore, sonId, gammeId, noteId, clavierId, vitesseId, 
  intensiteLED, gain, reverbMix, tremoloMix, chorusMix, arpEnabled, arpMode, 
  arpSpeedBPM, arpSpeed_id
};

int currentScorePage = 0;
uint8_t scorePage = 0;            // page actuelle des scores
const uint8_t scoresPerPage = 3;  // nombre de joueurs affichés par page
bool scoreRedraw = true;

int touchesMinMax[2] = { 0, NUM_TOUCH_PADS };

void infosCpu() {
  DEBUG_VAR_MAIN("CPU:", AudioProcessorUsage());
  DEBUG_VAR_MAIN("% max: ", AudioProcessorUsageMax());
  
  // Usage mémoire audio (nombre de blocs)
  DEBUG_VAR_MAIN("Mémoire Audio F32: ", AudioMemoryUsage_F32());
  DEBUG_VAR_MAIN("Mémoire Audio: ", AudioMemoryUsage());
  DEBUG_VAR_MAIN("blocs max: ", AudioMemoryUsageMax());
}

void onLowBattery(float voltage, float percentage) {
  Serial.println("⚠️ Batterie faible!");
  // Activer mode économie d'énergie
}

void onCriticalBattery(float voltage, float percentage) {
  Serial.println("🔴 BATTERIE CRITIQUE!");
  // Éteindre LEDs, sauvegarder données
}

void onShutdown(float voltage, float percentage) {
  Serial.println("💀 Arrêt imminent...");
  // Dernière sauvegarde
}

// Fonction callback pour les messages CC
void handleMidiCC(int cc, int value) {
    soundManager.handleControlChange(cc, value);
}

void handleTouchPress(int touchIndex) {
  DEBUG_VAR_MAIN("Touch pressed: ", touchIndex);

  // Récupérer la pression et la vélocité
  float pression = touchManager.getNormalizedPressure(touchIndex);
  uint8_t velocite = touchManager.getVelocityMs(touchIndex);
  DEBUG_VAR_MAIN("Calling noteOn with index: ", touchIndex);
  DEBUG_VAR_MAIN("Arpeg enabled: ", arpeg.isEnabled());
  arpeg.noteOn(touchIndex, pression, velocite);

  // Afficher pour debug
  //DEBUG_VAR_MAIN("Pression: ", pression);
  //DEBUG_VAR_MAIN("Vélocité: ", velocite);
  DEBUG_VAR_MAIN("Note count after add: ", arpeg.getNoteCount());
  // Utiliser la pression ou vélocité pour moduler le son ou la LED
  ledManager.setLedColor(touchIndex, TOUCH_COLORS[touchIndex]);
  ledManager.show();
  if (!arpeg.isEnabled()) {
        soundManager.playTouchSound(touchIndex, pression, velocite, octaveShift);
    }
  // exemple si playTouchSound accepte la vélocité
  // Modulation continue (aftertouch)
    
  if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) {
    DEBUG_INFO_MAIN("ERROR: Invalid touch index");
    return;
  }
}

void handleTouchRelease(int touchIndex) {
  DEBUG_VAR_MAIN("Touch released: ", touchIndex);
  arpeg.noteOff(touchIndex);
  // Éteindre la LED correspondante
  ledManager.setLedColor(touchIndex, CRGB::Black);
  ledManager.show();

  // Arrêter le son
  if (!arpeg.isEnabled()) {
    soundManager.stopOneNote(touchIndex);
  }
  

  if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) {
    DEBUG_INFO_MAIN("ERROR: Invalid touch index");
    return;
  }
}
float afterTouch(int touchIndex) {
    uint16_t rawPressure = touchManager.getPressure(touchIndex);
    float normPression = constrain((float)rawPressure / maxPressure, 0.0f, 1.0f);
    return normPression;
      
  }

void getAllValues() {
  sauvegarde[1] = ui_getSoundType();
  sauvegarde[2] = ui_getScale();
  sauvegarde[3] = ui_getNote();
  sauvegarde[4] = ui_getClavier();
  sauvegarde[5] = ui_getSpeed();
  sauvegarde[6] = ui_getBrightness();
  sauvegarde[7] = ui_getVolume();
  sauvegarde[8] = ui_getReverbLevel();
  sauvegarde[9] = ui_getTremolo();
  sauvegarde[10] = ui_getChorus();
  sauvegarde[11] = ui_getArp();
  sauvegarde[12] = ui_getModeArp();
  sauvegarde[13] = ui_getTempo();
  sauvegarde[14] = ui_getStepInterval();
}

void detect_events(void) {    
    // Vérifier la rotation
    long newPosition = rotaryEncoder.read();
    long positionDiff = newPosition - lastEncoderPosition;
    
    if (positionDiff != 0) {
        if (abs(positionDiff) >= 4) {
            if (positionDiff > 0) {
                rotary_event = 1;  // CW
                DEBUG_VAR_MAIN("Rotary: ", "CW");
            } else {
                rotary_event = 2;  // CCW
                DEBUG_VAR_MAIN("Rotary: ", "CCW");
            }
            lastEncoderPosition = newPosition;
        }
    }
    
    // Vérifier le bouton avec debounce
    static unsigned long lastButtonPress = 0;
    static bool lastButtonState = HIGH;
    unsigned long currentTime = millis();
    bool buttonState = digitalRead(ENCODER_BUTTON_PIN);
    
    if (buttonState != lastButtonState && buttonState == LOW) {
        if (currentTime - lastButtonPress > 50) {
            push_event = 1;
            DEBUG_INFO_MAIN("Button pressed");
            lastButtonPress = currentTime;
        }
        lastButtonState = buttonState;
    } else if (buttonState != lastButtonState) {
        lastButtonState = buttonState;
    }
}


void handle_menu_events(void) {
  bool eventHandled = false;

  // Gérer le bouton poussoir
  if (push_event == 1) {
    DEBUG_INFO_MAIN("Handling button press in menu");

    // Vérifier si on est sur le formulaire de jeu
    if (ui_getCurrentFormId() == 10) {
      uint8_t pos = ui_getCurrentCursorPosition();
      DEBUG_VAR_MAIN("Button press at position: ", pos);
      switch (pos) {
        case 3:
          DEBUG_INFO_MAIN("Highscore button pressed");
          ui_sendSelect();
          break;
        case 4:
          //gameStartRequested = true;
          DEBUG_INFO_MAIN("Start button pressed - game will start!");
          soundManager.stopAllSounds();
          //soundManager.setPolyMode(true);
          soundManager.setSoundMode(ui_getSoundType());
          soundManager.volume(gain);
          startSimonGame();
          //currentState = STATE_WAIT;
          //waitDisplayGameScreen = millis();
          break;
        case 2:
          DEBUG_INFO_MAIN("Back button pressed - returning to main menu");
          ui_sendSelect();  // Laisser MUI gérer le retour
          break;
        default:
          ui_sendSelect();
          break;
      }

    }

    else if (ui_getCurrentFormId() == 14) {
      uint8_t pos = ui_getCurrentCursorPosition();
      DEBUG_VAR_MAIN("Button press at position: ", pos);

      if (pos == 4) {  // Bouton Start
        DEBUG_INFO_MAIN("Sauvegarde");
        getAllValues();
        eeprom.sauvegarder(sauvegarde);
        ui_sendSelect();
      } else {
        // Autres positions (champs de saisie, etc.)
        ui_sendSelect();
      }
    }

    else if (ui_getCurrentFormId() == 20) {
      ui_sendSelect();
    } else {
      // Sur d'autres formulaires, comportement normal
      ui_sendSelect();
    }

    is_redraw = 1;
    push_event = 0;
    eventHandled = true;
  }

  // Gérer la rotation (inchangé)
  if (rotary_event == 1) {
    DEBUG_INFO_MAIN("Handling CW rotation in menu");
    ui_nextField();
    is_redraw = 1;
    rotary_event = 0;
    eventHandled = true;
  }

  if (rotary_event == 2) {
    DEBUG_INFO_MAIN("Handling CCW rotation in menu");
    ui_prevField();
    is_redraw = 1;
    rotary_event = 0;
    eventHandled = true;
  }
  if (eventHandled) {
    lastActivity = millis();
  }
}

void checkDifficulty() {
  uint8_t difficulty = ui_getDifficulty();
  switch (difficulty) {
    case 0:
      // difficulty = facile (5 notes) touches de 7 à 11
      touchesMinMax[0] = 7;
      touchesMinMax[1] = 12;  // touche max + 1 pour le random
      ledsOn = true;
      touchManager.setDifficulty(DIFF_EASY);
      break;
    case 1:
      // difficulty = moyen (7 notes) touches de 0 à 6
      touchesMinMax[0] = 0;
      touchesMinMax[1] = 7;
      ledsOn = true;
      touchManager.setDifficulty(DIFF_MEDIUM);
      break;
    case 2:
      // difficulty = difficile (12 notes) touches de 0 à 11
      touchesMinMax[0] = 0;
      touchesMinMax[1] = NUM_TOUCH_PADS;
      ledsOn = true;
      touchManager.setDifficulty(DIFF_HARD);
      break;
    case 3:
      // difficulty = extrême (12 notes) sans aides lumineuses
      touchesMinMax[0] = 0;
      touchesMinMax[1] = NUM_TOUCH_PADS;
      ledsOn = false;
      touchManager.setDifficulty(DIFF_HARD);
      break;
  }
}

int checkSpeed() {
  uint8_t speed = ui_getSpeed();
  int timing;
  switch (speed) {
    case 0:
      // vitesse = lent temps d'affichage 600ms
      timing = TIMING_SLOW;
      break;
    case 1:
      // vitesse = moyen temps d'affichage 400ms
      timing = TIMING_MEDIUM;
      break;
    case 2:
      // vitesse = rapide temps d'affichage 250ms
      timing = TIMING_QUICK;
      break;
    // vitesse = rapide temps d'affichage 120ms
    case 3:
      timing = TIMING_SPEED;
      break;
    default:
      timing = TIMING_SLOW;
      break;
  }
  return timing;
}


void playSequence(bool start = false) {
  // DÉMARRER la séquence
  if (start) {
    DEBUG_VAR_MAIN("Playing sequence for level: ", gameLevel + 1);
    touchManager.disableTouch();
    seqIndex = 0;
    seqChrono.restart();
    seqState = SEQ_WAIT_START;
    return;
  }
  int touchIndex;
  // METTRE À JOUR la séquence si en cours
  switch (seqState) {

    // Petite attente avant de commencer
    case SEQ_WAIT_START:
      DEBUG_INFO_MAIN("Sequence starting soon...");
      if (seqChrono.hasPassed(gameTiming + 400)) {
        seqState = SEQ_SHOW_STEP;
      }
    break;

    // Affiche et joue l'étape courante
    case SEQ_SHOW_STEP: {
      DEBUG_INFO_MAIN("Showing sequence step");
        if (seqIndex > gameLevel) {
          // Fin de séquence
          playerInput = 0;
          if (currentState != STATE_GAME_OVER) {
            displayPlayerTurn();
          }
          else {
            returnToMenu();
          }
            seqState = SEQ_IDLE;
          break;
        }
      touchIndex = gameSequence[seqIndex];
      }

      
      if (ledsOn) {
        ledManager.setLedColor(touchIndex, TOUCH_COLORS[touchIndex]);
        ledManager.show();
      }
      soundManager.playTouchSound(touchIndex, pressionSequence, velocitySequence, octaveShift);

      seqChrono.restart();
      seqState = SEQ_WAIT_END;
    break;

    // Attend la fin d'affichage
    case SEQ_WAIT_END: {
      DEBUG_INFO_MAIN("Waiting for sequence step to end");
      if (seqChrono.hasPassed(gameTiming)) {
        touchIndex = gameSequence[seqIndex];
        ledManager.setLedColor(touchIndex, CRGB::Black);
        ledManager.show();
        soundManager.stopOneNote(touchIndex);

        seqChrono.restart();
        seqState = SEQ_PAUSE_BETWEEN;
      }
    break;
    }

    // Petite pause avant la prochaine note
    case SEQ_PAUSE_BETWEEN:
      DEBUG_INFO_MAIN("Pausing between sequence steps");
      if (seqChrono.hasPassed(100)) {
        seqIndex++;
        seqState = SEQ_SHOW_STEP;
      }
      break;

    case SEQ_IDLE:
      DEBUG_INFO_MAIN("Sequence idle");
      touchManager.enableTouch();
      // Rien à faire
      break;
    default:
      // rien à faire
      break;
  }
}

void startSimonGame() {
  DEBUG_INFO_MAIN("Starting Simon Game!");
  currentState = STATE_WAIT;
  gameLevel = 0;
  playerInput = 0;
  gameInProgress = true;
  is_redraw = 0;

  // Récupérer les paramètres du menu
  uint8_t difficulty = ui_getDifficulty();
  numPlayers = ui_getNumJoueur();
  uint8_t speed = ui_getSpeed();

  DEBUG_VAR_MAIN("Game settings - Players: ", numPlayers);
  DEBUG_VAR_MAIN(", Difficulty: ", difficulty);
  DEBUG_VAR_MAIN(", Speed: ", speed);
  checkDifficulty();
  gameTiming = checkSpeed();
  // Initialiser la séquence
  uint32_t seed = *(uint32_t *)0x400C0000;  // Adresse du registre RNG matériel
  randomSeed(seed);
  for (int i = 0; i < 100; i++) {
    gameSequence[i] = random(touchesMinMax[0], touchesMinMax[1]);
  }
  // Afficher l'écran de jeu
  displayGameScreen();
  waitDisplayGameScreen = millis();
  //delay(1000);
  // Démarrer le premier niveau
}

void displayGameScreen() {
  DEBUG_INFO_MAIN("displayGameScreen()");

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawStr(20, 50, "MEMO-PAN");
    u8g2.setFont(u8g2_font_6x10_tf);
    String levelStr = "Niveau: " + String(gameLevel + 1);
    u8g2.drawStr(20, 70, levelStr.c_str());
    u8g2.drawStr(8, 90, "Regardez la sequence");
  } while (u8g2.nextPage());
}


void displayPlayerTurn() {
  DEBUG_INFO_MAIN("displayPlayerTurn()");
  //touchManager.enableTouch();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawStr(20, 50, "MEMO-PAN");
    u8g2.setFont(u8g2_font_6x10_tf);
    String levelStr = "Niveau: " + String(gameLevel + 1);
    u8g2.drawStr(20, 70, levelStr.c_str());
    String playerStr = "Joueur: " + String(player + 1);
    u8g2.drawStr(20, 90, playerStr.c_str());
    //u8g2.drawStr(20, 110, "A votre tour!");
  } while (u8g2.nextPage());
}

void waitForAllRelease() {
  DEBUG_INFO_MAIN("Waiting for all pads to be released...");
  ledManager.clear();
  ledManager.show();
  unsigned long timeout = millis() + 2000; // Timeout de 2 secondes
  bool allReleased = false;
  
  while (!allReleased && millis() < timeout) {
    touchManager.update();
    allReleased = true;
    
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      if (touchManager.isPressed(i)) {
        allReleased = false;
        break;
      }
    }
    
    delay(10);
  }
  
  // Forcer le nettoyage final
  soundManager.stopAllSounds();
  ledManager.clear();
  
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    toucheEtat[i] = false;
  }
  
  delay(100);
  
  DEBUG_INFO_MAIN("All pads released");
}


void handleGameInput(int touchIndex) {
  DEBUG_VAR_MAIN("Game input: ", touchIndex);

  if (touchIndex == gameSequence[playerInput]) {
    playerInput++;
    // Si toute la séquence est correcte
    if (playerInput > gameLevel) {
      gameLevel++;
      if (gameLevel % 10 == 0) {
        // Victoire !
        gameWon();
        delay(1000);
        displayPlayerTurn();
        if (gameLevel >= 100) {
          // Jeu terminé
          gameFinished();
        } else {
          // Continuer le jeu
          playSequence(true);
        }
      } else {
        // Niveau suivant
        delay(1000);
        playSequence(true);
      }
    }
  } else {
    // Erreur !
    soundManager.stopAllSounds();
    delay(50);
    currentScore[player] = gameLevel;
    if (checkPlayer()) {
      looser();
    } else {
      for (uint8_t i = 0; i < ui_getNumJoueur(); i++) {
        scores[i][0] = i + 1;                // numéro du joueur
        scores[i][1] = currentScore[i];  // score
      }
      gameOver();
    }
  }
}

void sortScores2D() {
  uint8_t n = ui_getNumJoueur();
  for (uint8_t i = 0; i < n - 1; i++) {
    for (uint8_t j = i + 1; j < n; j++) {
      if (scores[j][1] > scores[i][1]) {
        // échanger les lignes
        uint8_t tmp0 = scores[i][0];
        uint8_t tmp1 = scores[i][1];
        scores[i][0] = scores[j][0];
        scores[i][1] = scores[j][1];
        scores[j][0] = tmp0;
        scores[j][1] = tmp1;
      }
    }
  }
}

void congratulation() {
    DEBUG_INFO_MAIN("Game Finished!");
    const char* title = "BRAVO!";
    const char* msg1 = "Tu as fini le jeu!";

    const int numStars = 10;
    int starX[numStars], starY[numStars];

    // Initialisation des étoiles aléatoires
    for(int i = 0; i < numStars; i++){
        starX[i] = random(0, u8g2.getDisplayWidth());
        starY[i] = random(0, u8g2.getDisplayHeight()/2);
    }

    for (int frame = 0; frame < 50; frame++) { // durée de l'animation
        u8g2.firstPage();
        do {
            // Texte principal avec légère oscillation
            int yOffset = 2 * sin8(frame*6)/10;
            u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
            u8g2.drawStr(20, 25 + yOffset, title);

            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(10, 90, msg1);

            // Clignotement du texte “Appuyez pour continuer”
            // Dessin des étoiles
            for(int i = 0; i < numStars; i++){
                u8g2.drawPixel(starX[i], starY[i]);
                starY[i] += 1;                 // descente
                if(starY[i] >= u8g2.getDisplayHeight()) {
                    starY[i] = 0;              // réapparition en haut
                    starX[i] = random(0, u8g2.getDisplayWidth());
                }
            }

        } while (u8g2.nextPage());
        delay(50);  // vitesse de l'animation
    }

    // Affichage final fixe
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
        u8g2.drawStr(20, 40, title);
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 90, msg1);
    } while (u8g2.nextPage());
}


void gameWon() {
  DEBUG_INFO_MAIN("Game Won!");
  ledManager.selectFx(fxVictory);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawStr(30, 25, "BRAVO!");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 40, "Tu as gagne!");
  } while (u8g2.nextPage());
  victoryStartTime = millis();
  currentState = STATE_VICTORY;
}

bool checkPlayer() {
  numPlayers = ui_getNumJoueur();
  player++;
  if (player >= numPlayers) {
    return false;
  }
  return true;
}

void looser() {
  DEBUG_INFO_MAIN("Looser!");
  currentState = STATE_PLAYER_LOOSE;
  gameInProgress = true;
  ledManager.startFade(CRGB::Orange, false, 20, 10);

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawStr(30, 50, "PERDU");
    u8g2.setFont(u8g2_font_6x10_tf);
    String playerStr = "Joueur: " + String(player);
    u8g2.drawStr(25, 70, playerStr.c_str());
    String scoreStr = "Score: " + String(gameLevel);
    u8g2.drawStr(25, 90, scoreStr.c_str());
  } while (u8g2.nextPage());
  looseTime = millis();
}

void displayHighScore() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawStr(30, 30, "BRAVO");
    u8g2.setFont(u8g2_font_sticker100complete_tr);
    u8g2.drawStr(25, 50, "Nouveau");
    u8g2.drawStr(30, 70, "Record");
    u8g2.setFont(u8g2_font_6x10_tf);
    String playerStr = "Joueur: " + String(scores[0][0]);
    u8g2.drawStr(30, 90, playerStr.c_str());

    String scoreStr = "Score: " + String(scores[0][1]);
    u8g2.drawStr(30, 110, scoreStr.c_str());
  } while (u8g2.nextPage());
}

// Affiche une page donnée
void displayScorePage(int page) {
  u8g2.firstPage();
  do {
    if (page == 0) {
      // Page 0 : Titre + 3 premiers scores
      u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
      u8g2.drawStr(15, 40, "GAME OVER");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(45, 55, "SCORES");

      int y = 70;
      for (int i = 0; i < min(4, numPlayers); i++) {
        String line = String(i+1) + " Joueur " + String(scores[i][0] + 1) + ": " + String(scores[i][1]);
        u8g2.drawStr(20, y, line.c_str());
        y += 10;
      }
    } else {
      // Pages suivantes : 4 scores par page
      int startIndex = 4 + (page - 1) * 4;
      int endIndex = min(startIndex + 4, numPlayers);

      int y = 40;
      u8g2.setFont(u8g2_font_6x10_tf);
      for (int i = startIndex; i < endIndex; i++) {
        String line = String(i+1) + " Joueur " + String(scores[i][0] + 1) + ": " + String(scores[i][1]);
        u8g2.drawStr(20, y, line.c_str());
        y += 10;
      }
    }
  } while (u8g2.nextPage());
}

// Exemple de navigation (à appeler dans loop)
void nextPage() {
  int maxPages = getMaxPages(numPlayers);
  currentScorePage++;
  if (currentScorePage >= maxPages) currentScorePage = 0;  // boucle
}

void prevPage() {
  int maxPages = getMaxPages(numPlayers);
  currentScorePage--;
  if (currentScorePage < 0) currentScorePage = maxPages - 1;
}

// Calcule le nombre de pages
int getMaxPages(int numPlayers) {
  if (numPlayers <= 4) return 1;
  return 1 + ((numPlayers - 4) + 3) / 4;  // arrondi vers le haut
}

void gameFinished() {
  DEBUG_INFO_MAIN("Game Finished!");
  currentState = STATE_GAME_OVER;
  gameInProgress = false;
  scorePage = 0;  // démarrer à la première page
  sortScores2D();
  if (scores[0][1] > highScore) {
    newHighScore = true;
    DEBUG_INFO_MAIN("new Highscore");
    highScore = scores[0][1];
    ui_setHighScore(highScore);
    eeprom.setHighScore(highScore);
  } else {
    newHighScore = false;
  }
  ledManager.victorySequence();
  congratulation();
}

void gameOver() {
  DEBUG_INFO_MAIN("Game Over!");
  currentState = STATE_GAME_OVER;
  gameInProgress = false;
  scorePage = 0;  // démarrer à la première page
  ledManager.startTemporaryFade(CRGB::Red, false, 1000);
  ledManager.clear();
  sortScores2D();
  if (scores[0][1] > highScore) {
    newHighScore = true;
    DEBUG_INFO_MAIN("new Highscore");
    highScore = scores[0][1];
    ui_setHighScore(highScore);
    eeprom.setHighScore(highScore);
  } else {
    newHighScore = false;
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
      u8g2.drawStr(20, 50, "GAME OVER");
      u8g2.setFont(u8g2_font_6x10_tf);
      String levelStr = "Niveau atteint: " + String(gameLevel);
      u8g2.drawStr(20, 70, levelStr.c_str());
      u8g2.drawStr(50, 90, "Cliquez");
      u8g2.drawStr(25, 105, "pour continuer");
    } while (u8g2.nextPage());
  }
  displayScorePage(currentScorePage);
  for (int i = 0; i < NUM_MAX_PLAYER; i++) {
    currentScore[i] = 0;
  }
  player = 0;
}

void msgSauvegarde() {
  if (ui_getCurrentFormId() == 18) {
    delay(500);
    ui_gotoForm(14, 0);
    is_redraw = 1;
  }
}

void returnToMenu() {
  DEBUG_INFO_MAIN("Returning to menu");
  currentState = STATE_MENU;
  is_redraw = 1;
  touchManager.enableTouch();
  arpeg.setEnabled(arpEnabled);
}

void updateDisplay() {
  static unsigned long lastDisplayUpdate = 0;
  unsigned long currentTime = millis();

  // Limiter les mises à jour d'affichage
  if (currentTime - lastDisplayUpdate < 33) {
    return;
  }

  if (is_redraw && currentState == STATE_MENU) {
    DEBUG_INFO_MAIN("Updating menu display");

    u8g2.firstPage();
    do {
      ui_draw();
    } while (u8g2.nextPage());

    is_redraw = 0;
    lastDisplayUpdate = currentTime;
  }
}



void newGame() {
  for (uint8_t i = 0; i < NUM_MAX_PLAYER; i++) {
    currentScore[i] = 0;
    scores[i][0] = i;
    scores[i][1] = 0;
  }
}

bool varChanged(uint8_t currentValue, uint8_t &previousValue) {

  if (currentValue != previousValue) {
    previousValue = currentValue;
    return true;
  }
  return false;
}

// === Fonctions de test ===
void testAllLeds() {
  DEBUG_INFO_MAIN("Testing all LEDs...");
  //currentState = STATE_TEST_LEDS;

  // Afficher message de test
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(25, 20, "TEST LEDS");
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 40, "Test en cours...");
  u8g2.sendBuffer();

  // Tester chaque LED
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    ledManager.setLedColor(i, TOUCH_COLORS[i]);
    ledManager.show();
    delay(500);
    ledManager.setLedColor(i, CRGB::Black);
  }

  // Test final - toutes ensemble
  ledManager.setAllLeds(CRGB::White);
  ledManager.show();
  delay(1000);
  ledManager.clear();

  // Retour au menu
  delay(500);
  returnToMenu();
}

void testAllSounds() {
  DEBUG_INFO_MAIN("Testing all sounds...");
  //currentState = STATE_TEST_SONS;

  // Afficher message de test
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(25, 20, "TEST SONS");
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 40, "Test en cours...");
  u8g2.sendBuffer();

  // Tester chaque son avec LED correspondante
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    ledManager.setLedColor(i, TOUCH_COLORS[i]);
    ledManager.show();
    soundManager.playTouchSound(i);
    delay(800);
    ledManager.setLedColor(i, CRGB::Black);
    soundManager.stopOneNote(i);
    delay(200);
  }

  // Retour au menu
  delay(500);
  returnToMenu();
}

void differentTests(bool testing, int testNumber) {
  if (!testing) {
    return;
  }
  if (testNumber < 1 || testNumber > 5) {
    DEBUG_INFO_MAIN("Invalid test number");
    return;
  }
  switch(testNumber) {
    // Test des LEDs
    case 1:
      testAllLeds();
      delay(1000);
      break;
    case 2:
      testAllSounds();
      delay(1000);
      break;
    case 3:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_helvB08_tr);
      u8g2.drawStr(25, 20, "TEST EFFETS LEDS CELEBRATION");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(10, 40, "Test en cours...");
      u8g2.sendBuffer();
      ledManager.victorySequence();
      delay(1000);
      break;
    case 4:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_helvB08_tr);
      u8g2.drawStr(25, 20, "TEST ECRAN CELEBRATION");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(10, 40, "Test en cours...");
      u8g2.sendBuffer();
      congratulation();
      delay(1000);
      break;
    case 5:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_helvB08_tr);
      u8g2.drawStr(25, 20, "TEST EFFETS LEDS VICTOIRE");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(10, 40, "Test en cours...");
      u8g2.sendBuffer();
      fxVictory = 0;
      for (int i = 0; i < 12; i++) {
        ledManager.selectFx(fxVictory);
        ledManager.show();
        delay(1000);
        fxVictory++;
      }
      break;
    default:
      break;
  }
}

void setup() {
  DEBUG_BEGIN(115200);
  delay(500);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  MidiManager::onControlChange = handleMidiCC;
  DEBUG_INFO_MAIN("=== Simon Game Initializing ===");
  // Initialiser la batterie
  battMonitor.begin();
  battMonitor.onLowBattery(onLowBattery);
  battMonitor.onCriticalBattery(onCriticalBattery);
  battMonitor.onShutdown(onShutdown);

  // Initialiser l'écran en premier
  DEBUG_INFO_MAIN("Initializing display...");
  if (!u8g2.begin()) {
    DEBUG_INFO_MAIN("ERROR: Display initialization failed!");
    while (1) {
      delay(1000);
      DEBUG_INFO_MAIN("Display init failed, system halted");
    }
  }
  
  // Affichage de démarrage
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(0, 0, 128, 128, splash_MemoPan3);
  } while (u8g2.nextPage());
  delay(2000);

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawStr(20, 50, "MEMO-PAN");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 70, "by GASTON MUSIC");
    u8g2.drawStr(30, 100, "Johann Moine");
  } while (u8g2.nextPage());
  delay(500);

  DEBUG_INFO_MAIN("Initializing sound manager...");
  soundManager.begin();
  delay(200);
  // Initialiser l'UI
  DEBUG_INFO_MAIN("Initializing UI...");
  ui_init(u8g2);
  delay(200);

  // Initialiser les autres composants
  DEBUG_INFO_MAIN("Initializing touch manager...");
  touchManager.begin();
  touchManager.setPressureCurve(CURVE_S_CURVE);
  delay(100);

  DEBUG_INFO_MAIN("Initializing LED manager...");
  ledManager.begin();
  delay(100);

  

  // Test de démarrage - allumer toutes les LEDs brièvement
  DEBUG_INFO_MAIN("LED test...");
  unsigned long t0 = millis();
  while (millis() - t0 < 1200) {
    //DEBUG_INFO_MAIN("chenillard");
    ledManager.chenillardRainbow(100);
    ledManager.show();
  }
  
  ledManager.clear();


  if (!eeprom.donneesValides()) {
    DEBUG_INFO_MAIN("Aucune donnée valide trouvée, initialisation...");
    eeprom.initialiser(sauvegarde);
  } else {
    DEBUG_INFO_MAIN("Sauvegarde trouvée");
    eeprom.charger(sauvegarde);
  }
  DEBUG_EEPROM_PRINT(eeprom.debug());
  ui_setAll(sauvegarde);
  highScore = sauvegarde[0];
  ui_setHighScore(highScore);
  soundManager.setSoundMode(ui_getSoundType());
  soundManager.buildScale(ui_getNote(), ui_getScale());
  soundManager.setKeyboardMode(ui_getClavier());
  soundManager.volume(ui_getVolume());
  soundManager.setDryWetMix(ui_getReverbLevel());
  soundManager.setTremoloLevel(ui_getTremolo());
  soundManager.setChorusLevel(ui_getChorus());
  soundManager.setNoiseLevel(ui_getNoise());
  arpeg.setMode(ui_getModeArp());
  arpeg.setStepInterval(ui_getStepInterval(), ui_getTempo());
  arpeg.setEnabled(ui_getArp());

  systemReady = true;
  currentState = STATE_MENU;
  lastActivity = millis();

  DEBUG_INFO_MAIN("=== Simon Game Ready ===");
  // Forcer un premier redessin
  is_redraw = 1;
  for (int i = 3; i < 6; i++) { differentTests(DEBUG_TEST, i); }
  ledManager.clear();
}

void loop() {
  if (!systemReady) {
    delay(100);
    return;
  }
  battMonitor.loop();
  soundManager.update();
  // Afficher le statut
  //battMonitor.printStatus();
  // Récupérer les valeurs
  unsigned long now = millis();
  // if (now - lastInfoCheck >= 2000) {
  // infosCpu();
  //   lastInfoCheck = now;
  // }
  // if (now - lastAnalyse >= 50) {
  // soundManager.analyseAudio();
  // lastAnalyse = now;
  // }

  if (now - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
    float voltage = battMonitor.getVoltage();
    float percentage = battMonitor.getPercentage();
    
    if (abs(voltage - lastVoltage) >= VOLTAGE_THRESHOLD) {
      ui_setBatteryLevel(uint8_t(percentage));
      ui_draw();
      lastVoltage = voltage;
    }
    
    lastBatteryCheck = now;
  }
  MidiManager::updateConnection();
  MidiManager::read();
  soundManager.readVolumePot();  
  ArpegiateurOutput output = arpeg.tickAdvanced();
  if (output.valid) {
    if (output.isNoteOn) {
              // Jouer la note
              octaveShift = ui_getOctaveShift();
              arpeg.printActiveNotes();
              soundManager.playTouchSound(output.note, output.pressure, output.velocity, octaveShift);
          } else {
              // Couper la note
              soundManager.stopOneNote(output.note);
          }
      
  }
  if (arpEnabled && output.allNotesReleased) {
    // Ici, on sait qu'il n'y a plus aucune note active
    // On peut faire un reset complet ou couper tout le son
    soundManager.stopAllSounds();
  }

  // Détecter les événements de l'encodeur
  detect_events();
  
  // Traiter selon l'état actuel
  switch (currentState) {
    case STATE_MENU:
      handle_menu_events();
      updateDisplay();
      msgSauvegarde();
      touchManager.enableTouch();
      touchManager.setDifficulty(DIFF_HARD); // réactiver toutes les touches
      //DÉSACTIVER le touch sauf sur la page synth
      if (ui_getCurrentFormId() == 12) {  // Page synth
        if (ui_getOctaveShift() != octaveShift) {
          octaveShift = ui_getOctaveShift();
        }
        if (varChanged(ui_getNote(), lastNote) || varChanged(ui_getScale(), lastScale)) {
          soundManager.buildScale(ui_getNote(), ui_getScale());
        }
        
        if (ui_getSoundType() != lastSoundType) {
          lastSoundType = ui_getSoundType();
          soundManager.setSoundMode(ui_getSoundType());
        }
        static int lastClavier = -1;  // garde en mémoire l'ancien mode

        uint8_t currentClavier = ui_getClavier();
        if (currentClavier != lastClavier) {
          // le mode a changé !
          if (currentClavier == 3) {
            soundManager.generateRandomMap();
          }
          soundManager.setKeyboardMode(currentClavier);
          lastClavier = currentClavier;
        }

        
      }
      // Vérifier les autres actions demandées depuis le menu
      if (ui_getCurrentFormId() == 14) {  //page reglages

        if (varChanged(ui_getBrightness(), intensiteLED)) {
          DEBUG_VAR_MAIN("changement d'intensite ", intensiteLED);
          ledManager.setIntensity(intensiteLED);
          ledManager.update();
        }
        if (varChanged(ui_getVolume(), gain)) {
          soundManager.volume(gain);
        }
      }

      if (ui_getCurrentFormId() == 15) { // page effets
        if (varChanged(ui_getReverbLevel(), reverbMix)) {
          soundManager.setDryWetMix(reverbMix);
        }
        if (varChanged(ui_getTremolo(), tremoloMix)) {
          soundManager.setTremoloLevel(tremoloMix);
        }
        if (varChanged(ui_getChorus(), chorusMix)) {
          soundManager.setChorusLevel(chorusMix);
        }
        if (varChanged(ui_getNoise(), noiseMix)) {
          soundManager.setNoiseLevel(noiseMix);
        }
      }

      if (ui_getCurrentFormId() == 17) { // page arpégiateur
        bool currentArp = ui_getArp();
        if (currentArp != arpEnabled) {
          //DEBUG_INFO_MAIN("changement arp");
          //DEBUG_VAR_MAIN("arpEnabled=", currentArp);
          arpEnabled = currentArp;
          arpeg.setEnabled(arpEnabled);
          soundManager.setArpeggio(arpEnabled);
          is_redraw = 1;
        }
        
        if (varChanged(ui_getModeArp(), arpMode)) {
          arpeg.setMode(arpMode); 
      }
        if (varChanged(ui_getStepInterval(), arpSpeed_id) || varChanged(ui_getTempo(), arpSpeedBPM)) {
          uint16_t duration = arpeg.setStepInterval(arpSpeed_id, arpSpeedBPM);
          soundManager.soundDuration(duration);
        }
      }
      if (ui_isTestLedsRequested()) {
        testAllLeds();
      }
      if (ui_isTestSonsRequested()) {
        testAllSounds();
      }
      break;
    case STATE_WAIT:
      // Attente avant de démarrer le jeu
      if (millis() - waitDisplayGameScreen > 1000) { // 0.5 seconde d'attente
        currentState = STATE_INIT_GAME;
      }
      break;
    case STATE_INIT_GAME:
      // Initialisation du jeu
      //startSimonGame();
      if (ui_getDifficulty() == 3) {
      // Si difficulté extrême, jouer 1 fois la gamme au début
        currentState = STATE_SCALE_PLAY;
        scalePlayer.playScale(true);
      } else {
        currentState = STATE_GAME;
        playSequence(true);
      }

    break;
    case STATE_SCALE_PLAY:
      // Jouer la gamme
      scalePlayer.update();
      if (!scalePlayer.isPlaying()) {
        currentState = STATE_GAME;
        playSequence(true);
      }
    break;

    case STATE_GAME:
      // Dans le jeu, seules les touches capacitives sont actives
      playSequence();
      arpeg.setEnabled(false); // désactiver l'arpégiateur
      if (push_event == 1) {
        // Option pour quitter le jeu
        returnToMenu();
        push_event = 0;
      }
      break;

    case STATE_GAME_OVER:
      // Pagination des scores
      if (newHighScore && !highScoreDisplayed) {
        displayHighScore();
        highScoreDisplayed = true;
      }
      if (rotary_event == 1) {
        nextPage();
        scoreRedraw = true;
        highScoreDisplayed = false;
        rotary_event = 0;
      }
      if (rotary_event == 2) {
        prevPage();
        scoreRedraw = true;
        highScoreDisplayed = false;
        rotary_event = 0;
      }
      if (push_event == 1) {
        returnToMenu();  // revenir au menu après la dernière page
        newGame();
        currentScorePage = 0;
        push_event = 0;
        highScoreDisplayed = false;
      }
      if (scoreRedraw) {
        displayScorePage(currentScorePage);
        scoreRedraw = false;  // redessiné une fois
      }
      push_event = 0;
      break;
      case STATE_PLAYER_LOOSE:
        // Attendre 2 secondes puis démarrer le jeu pour le joueur suivant
        if (millis() - looseTime > 2000) { // 2 secondes d'attente
        startSimonGame();
        }
      break;
      case STATE_VICTORY:
        if (millis() - victoryStartTime >= 3000) {
          ledManager.clear();
          fxVictory++;
          if (fxVictory > 11) fxVictory = 0;
          currentState = STATE_GAME;
        }
      break;
  }

  // Lire les touches capacitives SEULEMENT si activées
  //static unsigned long lastTouchCheck = 0;
  touchManager.update();
  // if (currentTime - lastTouchCheck > 20) {
  //   touchManager.update();
  //   lastTouchCheck = currentTime;

  // Traiter les touches seulement si activées
  if (touchManager.isTouchEnabled()) {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      if (touchManager.isPressed(i) && !toucheEtat[i]) {
        handleTouchPress(i);
        toucheEtat[i] = true;
      } else if (touchManager.isReleased(i) && toucheEtat[i]) {
        handleTouchRelease(i);
        if (currentState == STATE_GAME && gameInProgress) {
            handleGameInput(i);
        }
        toucheEtat[i] = false;
      }
    }
  }
  
ledManager.update();
}

