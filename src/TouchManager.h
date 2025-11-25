#ifndef TOUCHMANAGER_H
#define TOUCHMANAGER_H

#include "Debug.h"
#include "config.h"
#include <Adafruit_MPR121.h>

enum PressureCurve {
    CURVE_LINEAR,
    CURVE_EXPONENTIAL,    // Recommandé pour synthé
    CURVE_LOGARITHMIC,
    CURVE_S_CURVE,
    CURVE_HARD,          // Très sensible
    CURVE_SOFT           // Peu sensible
};

enum DifficultyLevel {
  DIFF_EASY,
  DIFF_MEDIUM,
  DIFF_HARD
};

class TouchManager {
private:
  Adafruit_MPR121 cap;
  uint16_t lastTouched;
  uint16_t currentTouched;
  bool touchStates[NUM_TOUCH_PADS];
  bool prevTouchStates[NUM_TOUCH_PADS];
  bool padEnabled[NUM_TOUCH_PADS];
  unsigned long lastDebounceTime[NUM_TOUCH_PADS];
  bool touchEnabled = true;
  const unsigned long maxDuration = 200;
  
  unsigned long firstSensorTime[NUM_TOUCH_PADS];
  unsigned long secondSensorTime[NUM_TOUCH_PADS];
  unsigned long velocityMs[NUM_TOUCH_PADS]; // Valeur finale en ms

  
  uint16_t baselinePressure[NUM_TOUCH_PADS];    // Pression de référence au moment du press
  PressureCurve currentCurve = CURVE_EXPONENTIAL;
  float pressureSensitivity = 1.5;
    
  // Seuils de pression pour simuler un double capteur
  const uint16_t PRESSURE_THRESHOLD_1 = 5;   // Premier capteur (détection)
  const uint16_t PRESSURE_THRESHOLD_2 = 200;  // Deuxième capteur (vélocité)

  // Ajoute un tableau pour stocker le temps de début et de fin de touch
  unsigned long touchStartTime[NUM_TOUCH_PADS] = {0};
  unsigned long touchEndTime[NUM_TOUCH_PADS];
  bool isTouching[NUM_TOUCH_PADS];

  // Configuration IRQ
  const int IRQ_PIN = A8;
  static volatile bool touchChanged;
  static void onTouchInterrupt();
  
  
  
public:
  TouchManager();
  bool begin();
  void update();
  void enableTouch();
  void disableTouch();
  bool isTouchEnabled();
  bool isPressed(int touchIndex);
  bool isReleased(int touchIndex);
  bool isTouched(int touchIndex);
  void printTouchStatus(); // Pour debug
  uint16_t getPressure(int touchIndex); // Retourne la "pression" estimée
  unsigned long getVelocityMs(int touchIndex); // Retourne une vélocité basée sur la durée du touch 
  void clearState();
  void setPadEnabled(int touchIndex, bool enabled);
  void enablePads(uint8_t start, uint8_t end);
  void disableAllPads();
  bool isPadEnabled(int touchIndex);
  void setDifficulty(DifficultyLevel level);
  void setPressureCurve(PressureCurve curve);
  float applyCurve(float input);
  float getNormalizedPressure(int touchIndex);
};

// Définition du flag statique
volatile bool TouchManager::touchChanged = false;

// Routine d'interruption
void TouchManager::onTouchInterrupt() {
  touchChanged = true;
}

// ===== IMPLÉMENTATION =====

TouchManager::TouchManager() {
  lastTouched = 0;
  currentTouched = 0;
  
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    touchStates[i] = false;
    prevTouchStates[i] = false;
    lastDebounceTime[i] = 0;
    padEnabled[i] = true; // Par défaut, toutes les touches sont actives
  }
}

bool TouchManager::begin() {
  DEBUG_INFO_TOUCH("Initializing MPR121...");
  
  if (!cap.begin(MPR121_I2C_ADDR)) {
    DEBUG_INFO_TOUCH("MPR121 not found!");
    return false;
  }
  
  // Configuration automatique recommandée
  cap.setAutoconfig(true);
  pinMode(IRQ_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), onTouchInterrupt, FALLING);
  // Alternative manuelle si nécessaire (décommentez si autoconfig ne fonctionne pas)
  /*
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    cap.setThreshold(i, TOUCH_THRESHOLD, TOUCH_RELEASE_THRESHOLD);
  }
  */
  
  // Test de lecture initial
  //delay(100);
  //uint16_t testRead = cap.touched();
  DEBUG_INFO_TOUCH("MPR121 initial reading: 0x");
  DEBUG_INFO_TOUCH("MPR121 initial reading: 0x");
  DEBUG_INFO2_TOUCH(testRead, HEX);
  
  DEBUG_INFO_TOUCH("MPR121 initialized successfully!");
  return true;
}

void TouchManager::update() {
  if (!touchEnabled) return;
  
  // Déterminer si on doit lire le MPR121
  bool needsReading = touchChanged;
  
  if (!needsReading) {
    // Vérifier s'il y a des touches actives
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      if (touchStates[i]) {
        needsReading = true;
        break;
      }
    }
  }
  
  if (!needsReading) return; // ✅ Économie de lecture I2C
  
  // On lit SEULEMENT si nécessaire
  currentTouched = cap.touched();
  touchChanged = false;

  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    if (!padEnabled[i]) continue;
    bool currentState = (currentTouched & (1 << i)) != 0;
    unsigned long currentTime = millis();
    uint16_t currentPressure = getPressure(i);

    // Debouncing
    if (currentState != touchStates[i]) {
      if (currentTime - lastDebounceTime[i] > DEBOUNCE_TIME) {
        prevTouchStates[i] = touchStates[i];
        touchStates[i] = currentState;
        lastDebounceTime[i] = currentTime;

        if (currentState) {
          touchStartTime[i] = currentTime;
          baselinePressure[i] = currentPressure;
        } else {
          touchStartTime[i] = 0;
        }
      }
    }

    // Pendant un touch : calcul de la "pression relative"
    if (currentState && touchStartTime[i] != 0) {
      int relativePressure = (int)currentPressure - (int)baselinePressure[i];
      relativePressure = max(relativePressure, 0);
    }
  }

  lastTouched = currentTouched;
}


void TouchManager::clearState() {
  // Forcer tous les pads à l'état "released"
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    prevTouchStates[i] = false;
    touchStates[i] = false;
    // Réinitialiser pression, vélocité, etc.
  }
}

void TouchManager::setPadEnabled(int touchIndex, bool enabled) {
  if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;
  padEnabled[touchIndex] = enabled;
}

void TouchManager::enablePads(uint8_t start, uint8_t end) {
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    padEnabled[i] = (i >= start && i <= end);
  }
}

void TouchManager::disableAllPads() {
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    padEnabled[i] = false;
  }
}

bool TouchManager::isPadEnabled(int touchIndex) {
  if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return false;
  return padEnabled[touchIndex];
}


void TouchManager::setDifficulty(DifficultyLevel level) {
  disableAllPads();

  switch (level) {
    case DIFF_EASY:
      // Exemple : touches 7 à 11 actives
      for (int i = 7; i <= 11 && i < NUM_TOUCH_PADS; i++) {
        padEnabled[i] = true;
      }
      //DEBUG_INFO_TOUCH("Mode FACILE : touches 7 à 11 actives");
      break;

    case DIFF_MEDIUM:
      // Exemple : touches 0 à 6 actives
      for (int i = 0; i <= 6 && i < NUM_TOUCH_PADS; i++) {
        padEnabled[i] = true;
      }
      //DEBUG_INFO_TOUCH("Mode MOYEN : touches 0 à 6 actives");
      break;

    case DIFF_HARD:
      // Toutes les touches actives
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        padEnabled[i] = true;
      }
      //DEBUG_INFO_TOUCH("Mode DIFFICILE : toutes les touches actives");
      break;
  }
}


bool TouchManager::isPressed(int touchIndex) {
  if (touchIndex >= NUM_TOUCH_PADS) return false;
  return touchStates[touchIndex] && !prevTouchStates[touchIndex];
}

bool TouchManager::isReleased(int touchIndex) {
  if (touchIndex >= NUM_TOUCH_PADS) return false;
  return !touchStates[touchIndex] && prevTouchStates[touchIndex];
}

bool TouchManager::isTouched(int touchIndex) {
  if (touchIndex >= NUM_TOUCH_PADS) return false;
  return touchStates[touchIndex];
}

void TouchManager::enableTouch() {
    touchEnabled = true;
    //DEBUG_INFO_TOUCH("Touch enabled");
}

void TouchManager::disableTouch() {
    touchEnabled = false;
    //DEBUG_INFO_TOUCH("Touch disabled");
}

bool TouchManager::isTouchEnabled() {
    return touchEnabled;
}

void TouchManager::printTouchStatus() {
  DEBUG_INFO_TOUCH("Touched: ");
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    if (touchStates[i]) {
      DEBUG_INFO_TOUCH(i);
      DEBUG_INFO_TOUCH(" ");
    }
  }
  DEBUG_INFO_TOUCH();
}


// Retourne la "pression" estimée pour un pad (écart entre baseline et filtered)
uint16_t TouchManager::getPressure(int touchIndex) {
  if (touchIndex >= NUM_TOUCH_PADS) return 0;
  uint16_t baseline = cap.baselineData(touchIndex);
  uint16_t filtered = cap.filteredData(touchIndex);
  return baseline > filtered ? baseline - filtered : 0;
}

// Fonction pour calculer la vélocité en ms basée sur la durée du touch
unsigned long TouchManager::getVelocityMs(int touchIndex) {
  if (touchIndex >= NUM_TOUCH_PADS) return 20; // Valeur par défaut

  uint16_t pressure = map(getPressure(touchIndex), 0, 600, 300, 5);
  pressure = constrain(pressure, 5, 300);
  return pressure;
}

void TouchManager::setPressureCurve(PressureCurve curve) {
  currentCurve = curve;
  DEBUG_VAR_TOUCH("Curve set to: ", (int)curve);
}
float TouchManager::applyCurve(float input) {
  switch(currentCurve) {
    case CURVE_LINEAR:
        return input;
    
    case CURVE_EXPONENTIAL:
        // Touche naturelle, sensible aux nuances
        return pow(input, 0.6);
    
    case CURVE_LOGARITHMIC:
        // Demande plus de pression pour avoir de l'effet
        return pow(input, 1.5);
    
    case CURVE_S_CURVE:
        // Contrôle au milieu de la plage
        if (input < 0.5) {
            return 2.0 * pow(input, 2.0);
        } else {
            return 1.0 - 2.0 * pow(1.0 - input, 2.0);
        }
    
    case CURVE_HARD:
        // Très sensible, réponse rapide
        return pow(input, 0.4);
    
    case CURVE_SOFT:
        // Peu sensible, demande beaucoup de pression
        return pow(input, 2.0);
    
    default:
        return input;
  }
}
    
float TouchManager::getNormalizedPressure(int touchIndex) {
  if (touchIndex >= NUM_TOUCH_PADS) return 0.0;
  
  uint16_t rawPressure = getPressure(touchIndex);
  
  // Normalisation simple
  float normalized = (float)rawPressure / 300.0f;
  normalized *= pressureSensitivity;
  normalized = constrain(normalized, 0.0f, 1.0f);
  DEBUG_VAR_TOUCH("pression Brute: ", rawPressure);
  DEBUG_VAR_TOUCH("pression Norm: ", applyCurve(normalized));
  // Appliquer la courbe
  return applyCurve(normalized);
}

// uint8_t TouchManager::getNumTouchedPads() {
//     uint8_t count = 0;
//     for (int i = 0; i < NUM_TOUCH_PADS; i++) {
//         if (touchStates[i] && padEnabled[i]) { // seulement les pads activés
//             count++;
//         }
//     }
//     return count;
// }




#endif