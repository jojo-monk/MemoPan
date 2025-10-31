#include "Debug.h"
#include "config.h"
#include <U8g2lib.h>
//#include <MUIU8g2.h>
#include "TouchManager.h"
#include "LedManager.h"
#include "SoundManagerV2.h"
#include "MidiManager.h"
//#include <SimpleRotary.h>
#include <Encoder.h>
#include "ui.h"
#include "splash.h"
#include "SaveEprom.h"
#include "ArpegiateurV2.h"
#include "BatteryMonitor.h"

void testAllLeds();
void testAllSounds();



TouchManager touchManager;
LedManager ledManager;
SoundManager soundManager;
SaveEprom eeprom;
Arpegiateur arpeg;

// Rotary encoder
//SimpleRotary rotary(ENCODER_PIN_B, ENCODER_PIN_A, ENCODER_BUTTON_PIN);
Encoder rotaryEncoder(ENCODER_PIN_A, ENCODER_PIN_B);
long lastEncoderPosition = 0;

// Variables pour le bouton
unsigned long lastButtonPress = 0;
bool lastButtonState = HIGH;
const unsigned long DEBOUNCE_DELAY = 50;

BatteryMonitor battMonitor(BATTERY_PIN, 255, 51.0, 100.0); // Pin ADC, pas de pin shutdown, R1=51k, R2=100k

// Ecran
//U8G2_SH1107_128X128_F_4W_HW_SPI
//U8G2_SH1107_SEEED_128X128_F_4W_SW_SPI
U8G2_SH1107_SEEED_128X128_F_4W_SW_SPI u8g2(
  U8G2_R1, /* clock=*/13, /* data=*/11, /* cs=*/10,
  /* dc=*/5, /* reset=*/9);
//U8G2_SH1107_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//UI2 ui(u8g2, ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTTON_PIN); // pins pour l’encodeur

// ========================================
// ÉTATS DU JEU
// ========================================x

enum GameState {
  STATE_MENU,
  STATE_GAME,
  STATE_PLAY_SEQUENCE,
  STATE_PLAYER_TURN,
  STATE_CHECK_INPUT,
  STATE_GAME_OVER
};

// Variables d'état
GameState currentState = STATE_MENU;
uint8_t is_redraw = 1;
uint8_t rotary_event = 0;
uint8_t push_event = 0;
bool toucheEtat[12] = { false };
bool systemReady = false;
unsigned long lastActivity = 0;
static unsigned long lastCheck = 0;
int8_t previousNote = -1;


// Variables pour le jeu Simon
int gameSequence[100];
int gameLevel = 0;
int playerInput = 0;
bool gameInProgress = false;
bool newHighScore = false;
bool highScoreDisplayed = false;
uint16_t pressionSequence = 512; // Valeur de pression pour la séquence (moyenne)
uint8_t velocitySequence = 100;  // Vélocité pour la séquence
// unsigned long gameTimer = 0;
int gameTiming = 0;
uint8_t numPlayers = 0;
uint8_t player = 0;
uint8_t scores[NUM_MAX_PLAYER][2];
uint8_t currentScore[10] = { 0 };
uint8_t highScore = 0;
// Variables Son
uint8_t sonId = 0;
uint8_t gammeId = 0;
uint8_t noteId = 0;
uint8_t clavierId = 0;
uint8_t vitesseId = 0;
uint8_t gain = 8;
uint8_t reverbMix = 7;
uint8_t tremoloMix = 3;
uint8_t chorusMix = 3;
bool arpEnabled = false;
uint8_t arpSpeed_id = 3; // index dans STEP_INTERVALS_MS
uint8_t arpSpeedBPM = 120; // BPM pour l'arpégiateur
uint8_t arpMode = 0; // 0=UP, 1=DOWN, 2=UP_DOWN, 3=RANDOM
bool muteSound = true;
// Variables LED
uint8_t intensiteLED = 20;
bool ledsOn = true;
uint8_t fxVictory = 0;
float maxPressure = 550.0f; // Pression max typique

uint8_t sauvegarde[15] = { highScore, sonId, gammeId, noteId, clavierId, vitesseId, 
  intensiteLED, gain, reverbMix, tremoloMix, chorusMix, arpEnabled, arpMode, arpSpeedBPM, arpSpeed_id };

int currentPage = 0;
uint8_t scorePage = 0;            // page actuelle des scores
const uint8_t scoresPerPage = 3;  // nombre de joueurs affichés par page
bool scoreRedraw = true;

int touchesMinMax[2] = { 0, NUM_TOUCH_PADS };

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
void handleMidiCC(uint8_t cc, uint8_t value) {
    soundManager.handleCC(cc, value);
}

void handleTouchPress(int touchIndex) {
  DEBUG_VAR_MAIN("Touch pressed: ", touchIndex);

  // Récupérer la pression et la vélocité
  uint16_t pression = touchManager.getPressure(touchIndex);
  uint8_t velocite = touchManager.getVelocityMs(touchIndex);
  arpeg.noteOn(touchIndex, pression, velocite);

  // Afficher pour debug
  DEBUG_VAR_MAIN("Pression: ", pression);
  DEBUG_VAR_MAIN("Vélocité: ", velocite);

  // Utiliser la pression ou vélocité pour moduler le son ou la LED
  ledManager.setLedColor(touchIndex, TOUCH_COLORS[touchIndex]);
  ledManager.show();
  if (!arpeg.isEnabled()) {
        soundManager.playTouchSound(touchIndex, pression, velocite);
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
      // Mettre à jour amplitude / enveloppe / filtre en continu
      //polyWave[i].amplitude(baseAmplitude * normPression);
      //polyEnv[i].sustain(0.3 + 0.7 * normPression);
      //polyFilter[i].cutoff(baseCutoff + normPression * 2000);
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
    // Pas besoin de rate limiting pour l'encodeur !
    // Les interruptions capturent déjà tout en arrière-plan
    
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

// void detect_events(void) {
//   static unsigned long lastRotaryCheck = 0;
//   unsigned long currentTime = millis();

//   // Limiter la fréquence de vérification
//   if (currentTime - lastRotaryCheck < 10) {
//     return;
//   }
//   lastRotaryCheck = currentTime;

//   uint8_t tmp;

//   // Vérifier le bouton poussoir
//   tmp = rotary.push();
//   if (tmp != 0) {
//     push_event = tmp;
//     DEBUG_INFO_MAIN("Button pressed");
//   }

//   // Vérifier la rotation
//   tmp = rotary.rotate();
//   if (tmp != 0) {
//     rotary_event = tmp;
//     DEBUG_VAR_MAIN("Rotary: ", tmp == 1 ? "CW" : "CCW");
//   }
// }

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
        case 2:
          DEBUG_INFO_MAIN("Highscore button pressed");
          ui_sendSelect();
          break;
        case 3:
          //gameStartRequested = true;
          DEBUG_INFO_MAIN("Start button pressed - game will start!");
          soundManager.stopAllSounds();
          soundManager.setPolyMode(true);
          soundManager.setSoundMode(ui_getSoundType());
          delay(50);
          soundManager.volume(gain);
          startSimonGame();
          break;
        case 4:
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

      if (pos == 4) {  // Bouton Sauvegarder
        //gameStartRequested = true;
        DEBUG_INFO_MAIN("Sauvegarde");
        getAllValues();
        eeprom.sauvegarder(sauvegarde);
        ui_sendSelect();
        //delay(1000);
        //returnToMenu();
      } else {
        // Autres positions (champs de saisie, etc.)
        ui_sendSelect();
      }
    }

    else if (ui_getCurrentFormId() == 20) {
      ui_sendSelect();
      //returnToMenu();
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
      break;
    case 1:
      // difficulty = moyen (7 notes) touches de 0 à 6
      touchesMinMax[0] = 0;
      touchesMinMax[1] = 7;
      ledsOn = true;
      break;
    case 2:
      // difficulty = difficile (12 notes) touches de 0 à 11
      touchesMinMax[0] = 0;
      touchesMinMax[1] = NUM_TOUCH_PADS;
      ledsOn = true;
      break;
    case 3:
      // difficulty = extrême (12 notes) sans aides lumineuses
      touchesMinMax[0] = 0;
      touchesMinMax[1] = NUM_TOUCH_PADS;
      ledsOn = false;
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

void playScale(bool play) {
  if (play) {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      ledManager.setLedColor(i, TOUCH_COLORS[i]);
      ledManager.show();
      soundManager.playTouchSound(i);
      delay(300);
      ledManager.setLedColor(i, CRGB::Black);
      ledManager.show();
      soundManager.stopOneNote(i);
      delay(100);
    }
  } else {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      ledManager.setLedColor(i, CRGB::Black);
    }
    ledManager.show();
    soundManager.stopAllSounds();
  }
}

void startSimonGame() {
  DEBUG_INFO_MAIN("Starting Simon Game!");
  currentState = STATE_GAME;
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
  randomSeed(analogRead(0));
  for (int i = 0; i < 100; i++) {
    gameSequence[i] = random(touchesMinMax[0], touchesMinMax[1]);
  }

  // Afficher l'écran de jeu
  displayGameScreen();
  delay(1000);
  // Démarrer le premier niveau
  if (ui_getDifficulty() == 3) {
    // Si difficulté extrême, jouer 1 fois la gamme au début
    playScale(true);
    delay(1000);
  }
  playSequence();
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
    u8g2.drawStr(10, 90, "Regardez la sequence");
  } while (u8g2.nextPage());
}

void playSequence() {
  DEBUG_VAR_MAIN("Playing sequence for level: ", gameLevel + 1);
  // DÉSACTIVER le touch pendant la séquence
  touchManager.disableTouch();
  //touchManager.clearState();
  delay(gameTiming + 400);  // Pause avant de commencer

  for (int i = 0; i <= gameLevel; i++) {
    int touchIndex = gameSequence[i];

    // Allumer la LED
    if (ledsOn) {
    ledManager.setLedColor(touchIndex, TOUCH_COLORS[touchIndex]);
    ledManager.show();
    }

    // Jouer le son
    soundManager.playTouchSound(touchIndex, pressionSequence, velocitySequence);
    //DEBUG_VAR_MAIN("Sequence touch: ", touchIndex);

    delay(gameTiming);  // Durée d'affichage

    // Éteindre
    ledManager.setLedColor(touchIndex, CRGB::Black);
    ledManager.show();
    soundManager.stopOneNote(touchIndex);
    //soundManager.stopAllSounds();
    //delay(50);
    //touchManager.clearState();
    //soundManager.stopOneNote(touchIndex);
    //DEBUG_VAR_MAIN("Stop touch: ", touchIndex);

    delay(100);  // Pause entre les éléments
  }

  // Prêt pour la saisie du joueur
  // RÉACTIVER le touch pour la saisie du joueur
  touchManager.enableTouch();
  playerInput = 0;
  if (currentState != STATE_GAME_OVER) {
    displayPlayerTurn();
  }
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
    u8g2.drawStr(20, 110, "A votre tour!");
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

  // if (touchIndex >= 0 && touchIndex < NUM_TOUCH_PADS) {
  //   //ledManager.setLedColor(touchIndex, TOUCH_COLORS[touchIndex]);
  //   ///ledManager.show();
  //   //soundManager.playTouchSound(touchIndex);

  //   // Maintenir l'affichage un court instant
  //   //delay(200);

  //   // Éteindre (sauf si c'est la bonne réponse, on pourrait garder allumé plus longtemps)
  //   ledManager.setLedColor(touchIndex, CRGB::Black);
  //   ledManager.show();
  //   soundManager.stopOneNote(touchIndex);
  //   delay(50);
  // }

  // Vérifier si c'est correct
  if (touchIndex == gameSequence[playerInput]) {
    playerInput++;
    // Si toute la séquence est correcte
    if (playerInput > gameLevel) {
      gameLevel++;
      if (gameLevel % 10 == 0) {
        // Victoire !
        gameWon();
        delay(1000);
        if (gameLevel >= 100) {
          // Jeu terminé
          //waitForAllRelease();
          gameOver();
        } else {
          // Continuer le jeu
          //waitForAllRelease();
          playSequence();
        }
      } else {
        // Niveau suivant
        //waitForAllRelease();
        delay(1000);
        playSequence();
      }
    }
  } else {
    // Erreur !
    soundManager.stopAllSounds();
    delay(50);
    //touchManager.disableTouch();
    currentScore[player] = gameLevel;
    if (checkPlayer()) {
      looser();
      //waitForAllRelease();
      startSimonGame();
    } else {
      for (uint8_t i = 0; i < ui_getNumJoueur(); i++) {
        scores[i][0] = i;                // numéro du joueur
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
    //currentState = STATE_GAME_OVER;

    const char* title = "BRAVO!";
    const char* msg1 = "Tu as fini le jeu!";
    //const char* msg2 = "Click pour continuer";

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
        //u8g2.drawStr(10, 55, msg2);
    } while (u8g2.nextPage());
}


void gameWon() {
  DEBUG_INFO_MAIN("Game Won!");
  currentState = STATE_GAME;

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_helvB08_tr);
    u8g2.drawStr(30, 25, "BRAVO!");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(15, 40, "Vous avez gagne!");
    u8g2.drawStr(10, 55, "Appuyez pour continuer");
  } while (u8g2.nextPage());

  // Effet de victoire
  unsigned long t0 = millis();
  while (millis() - t0 < 1000) {
    ledManager.selectFx(fxVictory);
    ledManager.show();
  }
  ledManager.clear();
  fxVictory++;
  if (fxVictory > 11) fxVictory = 0;
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
  ledManager.setAllLeds(CRGB::Red);
  ledManager.show();
  unsigned long t0 = millis();
  while (millis() - t0 < 1000) {
    ledManager.fade(CRGB::Orange, false, 10, 5);
    ledManager.show();
  }
  delay(1000);
  ledManager.clear();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_helvB08_tr);
    u8g2.drawStr(25, 20, "PERDU");
    u8g2.setFont(u8g2_font_6x10_tf);
    String playerStr = "Joueur: " + String(player);
    u8g2.drawStr(10, 35, playerStr.c_str());

    String scoreStr = "Score: " + String(gameLevel);
    u8g2.drawStr(10, 50, scoreStr.c_str());
    //u8g2.drawStr(10, 50, "Appuyez pour menu");
  } while (u8g2.nextPage());
  delay(1000);
}

void displayHighScore() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawStr(30, 30, "BRAVO");
    u8g2.drawStr(20, 50, "Nouveau Record");
    u8g2.setFont(u8g2_font_6x10_tf);
    String playerStr = "Joueur: " + String(scores[0][0]);
    u8g2.drawStr(20, 70, playerStr.c_str());

    String scoreStr = "Score: " + String(scores[0][1]);
    u8g2.drawStr(20, 90, scoreStr.c_str());
    //u8g2.drawStr(10, 60, "Appuyez pour continuer");
    //u8g2.drawStr(10, 50, "Appuyez pour menu");
  } while (u8g2.nextPage());
  //delay(1000);
}

// Affiche une page donnée
void displayScorePage(int page) {
  u8g2.firstPage();
  do {
    if (page == 0) {
      // Page 0 : Titre + 3 premiers scores
      u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
      u8g2.drawStr(10, 15, "GAME OVER");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(40, 30, "SCORES");

      int y = 50;
      for (int i = 0; i < min(3, numPlayers); i++) {
        String line = "Joueur " + String(scores[i][0] + 1) + ": " + String(scores[i][1]);
        u8g2.drawStr(10, y, line.c_str());
        y += 10;
      }
    } else {
      // Pages suivantes : 4 scores par page
      int startIndex = 3 + (page - 1) * 4;
      int endIndex = min(startIndex + 4, numPlayers);

      int y = 10;
      u8g2.setFont(u8g2_font_6x10_tf);
      for (int i = startIndex; i < endIndex; i++) {
        String line = "Joueur " + String(scores[i][0] + 1) + ": " + String(scores[i][1]);
        u8g2.drawStr(10, y, line.c_str());
        y += 10;
      }
    }
  } while (u8g2.nextPage());
}

// Exemple de navigation (à appeler dans loop)
void nextPage() {
  int maxPages = getMaxPages(numPlayers);
  currentPage++;
  if (currentPage >= maxPages) currentPage = 0;  // boucle
}

void prevPage() {
  int maxPages = getMaxPages(numPlayers);
  currentPage--;
  if (currentPage < 0) currentPage = maxPages - 1;
}

// Calcule le nombre de pages
int getMaxPages(int numPlayers) {
  if (numPlayers <= 3) return 1;
  return 1 + ((numPlayers - 3) + 3) / 4;  // arrondi vers le haut
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
  //displayScorePage();

  // Effet game over
  ledManager.setAllLeds(CRGB::Red);
  ledManager.show();
  unsigned long t0 = millis();
  while (millis() - t0 < 1000) {
    ledManager.fade(CRGB::Red, false, 20, 10);
    ledManager.show();
  }
  delay(100);
  ledManager.clear();
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
  //ui_gotoForm();  // Retour au menu principal
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
  currentState = STATE_TEST_LEDS;

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
  currentState = STATE_TEST_SONS;

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
    //u8g2.drawXBMP(0, 0, 128, 64, splashScreen);
    u8g2.drawXBMP(0, 0, 128, 128, splash_MemoPan3);
  } while (u8g2.nextPage());
  delay(2000);

  u8g2.firstPage();
  do {
    //u8g2.clearDisplay();
    //u8g2.setFontMode(0);
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    //u8g2.setFont(u8g2_font_tenthinnerguys_tr);
    u8g2.drawStr(20, 50, "MEMO-PAN");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 70, "by GASTON MUSIC");
  } while (u8g2.nextPage());
  //u8g2.sendBuffer();
  delay(500);


  // Initialiser l'UI
  DEBUG_INFO_MAIN("Initializing UI...");
  //uiinit();
  ui_init(u8g2);
  delay(200);

  // Initialiser les autres composants
  DEBUG_INFO_MAIN("Initializing touch manager...");
  touchManager.begin();
  delay(100);

  DEBUG_INFO_MAIN("Initializing LED manager...");
  ledManager.begin();
  delay(100);

  DEBUG_INFO_MAIN("Initializing sound manager...");
  soundManager.begin();
  delay(200);
  //soundManager.setPolyMode(false);
  // soundManager.testPoly();
  // delay(200);

  // Test de démarrage - allumer toutes les LEDs brièvement
  DEBUG_INFO_MAIN("LED test...");
  //ledManager.setAllLeds(CRGB::Green);
  // ledManager.show();
  // delay(2000);
  // ledManager.breathe(CRGB::Blue);
  // ledManager.show();
  // delay(2000);
  unsigned long t0 = millis();
  //unsigned long t1 = 0;
  while (millis() - t0 < 1200) {
    //DEBUG_INFO_MAIN("chenillard");
    ledManager.chenillardRainbow(100);
    ledManager.show();
    //t1 = millis();
  }
  delay(40);
  ledManager.clear();

  //delay(5000);       // Chenillard rapide
  //ledManager.redGreenAlternance(500);
  //ledManager.show();
  //delay(500);           // Change toutes les 500ms
  //ledManager.rainbowCycle(30);
  //ledManager.show();
  //delay(5000);
  //delay(500);                  // Arc-en-ciel fluide
  //ledManager.clear();

  // Son de démarrage
  //soundManager.playStartupSound();

  // Marquer le système comme prêt

  //int note = ui_getNote();
  //int scale = ui_getScale();
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
  arpeg.setMode(ui_getModeArp());
  arpeg.setStepInterval(ui_getStepInterval(), ui_getTempo());
  arpeg.setEnabled(ui_getArp());

  systemReady = true;
  currentState = STATE_MENU;
  lastActivity = millis();

  DEBUG_INFO_MAIN("=== Simon Game Ready ===");
  //arp.setMode(arpegiateur::UP_DOWN);
  //arp.setStepInterval(150); // 150ms entre notes
  //arp.setEnabled(true);
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
  // Afficher le statut
  //battMonitor.printStatus();
  
  // Récupérer les valeurs
  float voltage = battMonitor.getVoltage();
  float percentage = battMonitor.getPercentage();
  //DEBUG_VAR_MAIN("Battery: ", voltage);
  //DEBUG_VAR_MAIN(" % ", percentage);

  ui_setBatteryLevel(uint8_t(percentage));
  //updateInput();
  //draw();
  MidiManager::updateConnection();
  MidiManager::read();
  soundManager.readVolumePot();  

  ArpegiateurOutput output = arpeg.tickAdvanced();
  if (output.valid) {
    if (output.isNoteOn) {
              // Jouer la note
              arpeg.printActiveNotes();
              soundManager.playTouchSound(output.note, output.velocity, output.pressure);
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



  unsigned long currentTime = millis();
// if (millis() - lastCheck > 2000) {
//   Serial.print("Mémoire audio max utilisée: ");
//   Serial.print(AudioMemoryUsageMax());
//   Serial.println("/50");
//   lastCheck = millis();
// }
  // Détecter les événements de l'encodeur
  detect_events();
  //ui_update(); // Lit l'encodeur et met à jour les menus
  //ui_draw();   // Redessine si nécessaire
  //updateInput();
  //draw();
  
  switch (currentState) {
    case STATE_MENU:
      handleMenu();
      break;

    case STATE_PLAY_SEQUENCE:
      playSequence();
      break;

    case STATE_PLAYER_TURN:
      handlePlayerTurn();
      break;

    case STATE_CHECK_INPUT:
      checkPlayerInput();
      break;

    case STATE_GAME_OVER:
      handleGameOver();
      break;
  }

  switch (currentState) {
    case STATE_MENU:
      handleMenu();
      break;

    case STATE_PLAY_SEQUENCE:
      playSequence();
      break;

    case STATE_PLAYER_TURN:
      handlePlayerTurn();
      break;

    case STATE_CHECK_INPUT:
      checkPlayerInput();
      break;

    case STATE_GAME_OVER:
      handleGameOver();
      break;
  }

  // Traiter selon l'état actuel
  switch (currentState) {
    case STATE_MENU:
      handle_menu_events();
      updateDisplay();
      msgSauvegarde();
      //DÉSACTIVER le touch sauf sur la page synth
      if (ui_getCurrentFormId() == 12) {  // Page synth
        if (varChanged(ui_getNote(), noteId) || varChanged(ui_getScale(), gammeId || varChanged(ui_getSoundType(), sonId))) {
          DEBUG_VAR_MAIN("changement note ", noteId);
          DEBUG_VAR_MAIN("changement gamme ", gammeId);
          soundManager.buildScale(noteId, gammeId);
          soundManager.setSoundMode(sonId);
        }
        soundManager.setPolyMode(true);
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

        touchManager.enableTouch();
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

      if (ui_getCurrentFormId() == 15) {
        if (varChanged(ui_getReverbLevel(), reverbMix)) {
          soundManager.setDryWetMix(reverbMix);
        }
        if (varChanged(ui_getTremolo(), tremoloMix)) {
          soundManager.setTremoloLevel(tremoloMix);
        }
        if (varChanged(ui_getChorus(), chorusMix)) {
          soundManager.setChorusLevel(chorusMix);
        }
      }

      if (ui_getCurrentFormId() == 17) {
        bool currentArp = ui_getArp();
        if (currentArp != arpEnabled) {
          DEBUG_INFO_MAIN("changement arp");
          DEBUG_VAR_MAIN("arpEnabled=", currentArp);
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

    case STATE_GAME:
      // Dans le jeu, seules les touches capacitives sont actives
      arpeg.setEnabled(false); // désactiver l'arpégiateur
      touchManager.enableTouch();
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
      }
      if (rotary_event == 2) {
        prevPage();
        scoreRedraw = true;
        highScoreDisplayed = false;
      }
      if (push_event == 1) {
        returnToMenu();  // revenir au menu après la dernière page
        newGame();
        currentPage = 0;
        push_event = 0;
        highScoreDisplayed = false;
      }
      if (scoreRedraw) {
        displayScorePage(currentPage);
        scoreRedraw = false;  // redessiné une fois
      }
      push_event = 0;
      break;

      // case STATE_GAME_OVER:
      //   // Attendre une action pour retourner au menu
      //   if (push_event == 1 || rotary_event != 0) {
      //     returnToMenu();
      //     push_event = 0;
      //     rotary_event = 0;
      //   }
      //   break;
  }

  // Lire les touches capacitives SEULEMENT si activées
  static unsigned long lastTouchCheck = 0;
  if (currentTime - lastTouchCheck > 20) {
    touchManager.update();
    lastTouchCheck = currentTime;

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
  }

  // Mettre à jour les LEDs
  static unsigned long lastLedUpdate = 0;
  if (currentTime - lastLedUpdate > 16) {
    ledManager.update();
    lastLedUpdate = currentTime;
  }

  // Petite pause
  delay(5);
}

