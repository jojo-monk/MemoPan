#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include "Debug.h"
#include "config.h"
#include <FastLED.h>

class LedManager {
private:
  CRGB leds[NUM_LEDS];
  bool needsUpdate;
   unsigned long modeChangeStartTime;
  bool showingModeChange;
  int currentMode;
  struct FadeState {
    bool active;
    bool fadeIn;
    uint8_t brightness;
    unsigned long lastUpdate;
    CRGB baseColor;
  };
  struct GlobalFadeState {
  bool active = false;
  bool fadeIn = true;
  int brightness = 0;
  unsigned long lastUpdate = 0;
  CRGB color = CRGB::Black;
  uint16_t speed = 20;
  uint8_t step = 10;
  };

  GlobalFadeState globalFade;


  FadeState fadeStates[NUM_TOUCH_PADS];
  bool tempFadeRunning = false;
  CRGB tempFadeColor;
  bool tempFadeIn;
  uint16_t tempFadeDuration;
  unsigned long tempFadeStartTime;
  int currentEffect = -1;
  bool effectNeedsReset = false;

  void resetEffectStates() {
    // Force la réinitialisation de tous les static dans les effets
    // En appelant chaque effet une fois avec un flag de reset
    clear();
    needsUpdate = true;
  }

  
public:
  LedManager();
  void begin();
  void update();
  void setLedColor(int ledIndex, CRGB color);
  void setAllLeds(CRGB color);
  void clear();
  void show();
  void showModeChange(int mode);
  void rainbow();
  void breathe(CRGB color);
  void chenillard(CRGB color, int speed);
  void redGreenAlternance(int speed);
  void rainbowCycle(int speed);
  void chenillardRainbow(int speed);
  void selectFx(int fx);
  void circleRainbow(int speed);
  void fade(CRGB color, bool fadeIn, uint16_t speed, uint8_t step);
  void setIntensity(uint8_t percent);
  void fadeOneLed(int pad, CRGB color, bool fadeIn, uint16_t speed, uint8_t step);
  void flashRandomColors(uint8_t times, uint16_t delayMs);
  void sequentialWave(uint8_t repeats, uint16_t delayMs, CRGB color);
  void rainbowFade(uint16_t speed);
  void strobe(CRGB color, uint8_t times, uint16_t delayMs);
  void rainbowPulse(uint16_t speed);
  void victorySequence();
  void startTemporaryFade(CRGB color, bool fadeIn, uint16_t durationMs);
  void updateFade();
  void startFade(CRGB color, bool fadeIn, uint16_t speed, uint8_t step);
  
  
};

// ===== IMPLÉMENTATION ==

LedManager::LedManager() {
  needsUpdate = false;
  showingModeChange = false;
  modeChangeStartTime = 0;
  currentMode = 0;
}

void LedManager::begin() {
  DEBUG_INFO_LED("Initializing LEDs...");
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  
  // Test initial
  clear();
  show();
  delay(50);
  DEBUG_INFO_LED("LEDs initialized successfully!");
}

void LedManager::startFade(CRGB color, bool fadeIn, uint16_t speed, uint8_t step) {
  globalFade.active = true;
  globalFade.fadeIn = fadeIn;
  globalFade.color = color;
  globalFade.speed = speed;
  globalFade.step = step;
  globalFade.brightness = fadeIn ? 0 : 255;
  globalFade.lastUpdate = millis();
}

void LedManager::updateFade() {
  if (!globalFade.active) return;

  if (millis() - globalFade.lastUpdate < globalFade.speed) return;
  globalFade.lastUpdate = millis();

  // Avance la luminosité
  if (globalFade.fadeIn) {
    globalFade.brightness += globalFade.step;
    if (globalFade.brightness >= 255) {
      globalFade.brightness = 255;
      globalFade.active = false;  // fade terminé
    }
  } else {
    globalFade.brightness -= globalFade.step;
    if (globalFade.brightness <= 0) {
      globalFade.brightness = 0;
      globalFade.active = false;  // fade terminé
    }
  }

  // Appliquer la luminosité actuelle
  CRGB c = globalFade.color;
  c.nscale8((uint8_t)globalFade.brightness);
  setAllLeds(c);
  needsUpdate = true;
}


void LedManager::startTemporaryFade(CRGB color, bool fadeIn, uint16_t durationMs) {
  tempFadeColor = color;
  tempFadeIn = fadeIn;
  tempFadeDuration = durationMs;
  tempFadeStartTime = millis();
  tempFadeRunning = true;

  // Démarre le fade principal
  startFade(color, fadeIn, 20, 10);  // vitesse et pas par défaut
}





void LedManager::update() {
  // Gestion de l'affichage du changement de mode
  updateFade();
  if (showingModeChange) {
    unsigned long elapsed = millis() - modeChangeStartTime;
    
    if (elapsed < MODE_DISPLAY_TIME) {
      // Animation de changement de mode
      float progress = (float)elapsed / MODE_DISPLAY_TIME;
      uint8_t brightness = (uint8_t)(255 * (1.0 - progress));
      
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = MODE_COLORS[currentMode];
        leds[i].fadeToBlackBy(255 - brightness);
      }
      needsUpdate = true;
    } else {
      showingModeChange = false;
      clear();
    }
  }
  if (tempFadeRunning) {
    // Le fade progresse automatiquement via updateFade()
    if (millis() - tempFadeStartTime >= tempFadeDuration) {
        tempFadeRunning = false;
        globalFade.active = false; // stoppe le fade proprement
        clear();
        needsUpdate = true;
    }
}

  
  if (needsUpdate) {
    show();
    needsUpdate = false;
  }
}

// Régle l'intensité globale des LEDs en pourcentage (0-100)
void LedManager::setIntensity(uint8_t percent) {
  if (percent > 100) percent = 100;  // clamp à 100 max
  
  uint8_t brightness = map(percent, 0, 100, 0, 255);
  FastLED.setBrightness(brightness);
  needsUpdate = true;  // force un update à la prochaine boucle
}


void LedManager::setLedColor(int ledIndex, CRGB color) {
  if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
    leds[ledIndex] = color;
    needsUpdate = true;
  }
}

void LedManager::setAllLeds(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  needsUpdate = true;
}

void LedManager::clear() {
  setAllLeds(CRGB::Black);
}

void LedManager::show() {
  FastLED.show();
  delayMicroseconds(300);
}

void LedManager::showModeChange(int mode) {
  currentMode = mode;
  showingModeChange = true;
  modeChangeStartTime = millis();
  
  // Affichage immédiat de la couleur du mode
  setAllLeds(MODE_COLORS[mode]);
  show();
}

void LedManager::rainbow() {
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + (i * 255 / NUM_LEDS), 255, 255);
  }
  hue++;
  needsUpdate = true;
}

void LedManager::fadeOneLed(int pad, CRGB color, bool fadeIn, uint16_t speed, uint8_t step) {
  FadeState &fs = fadeStates[pad];

  if (!fs.active || fs.fadeIn != fadeIn) {
    fs.brightness = fadeIn ? 0 : 255;
    fs.active = true;
    fs.fadeIn = fadeIn;
    fs.baseColor = color;
  }

  if (millis() - fs.lastUpdate < speed) return;

  if (fadeIn) {
    fs.brightness = min(255, fs.brightness + step);
  } else {
    fs.brightness = max(0, fs.brightness - step);
  }

  CRGB c = fs.baseColor;
  c.nscale8(fs.brightness);
  leds[pad] = c;  // applique seulement sur la LED du pad

  fs.lastUpdate = millis();
}

// Ajoute un paramètre "step" pour régler la longueur du fade.
// Ex: speed = ms entre pas ; step = taille du pas (1=très lent, 10=rapide).
void LedManager::fade(CRGB color, bool fadeIn, uint16_t speed, uint8_t step) {
  static bool initialized = false;
  static bool lastFadeIn = true;
  static int brightness = 0;                 // 0..255
  static unsigned long lastUpdate = 0;

  // Première utilisation ou changement de sens -> on réinitialise correctement
  if (!initialized || lastFadeIn != fadeIn) {
    brightness = fadeIn ? 0 : 255;
    initialized = true;
    lastFadeIn = fadeIn;
  }

  if (millis() - lastUpdate < speed) return;

  // Avance d'un pas
  if (fadeIn) {
    brightness += step;
    if (brightness > 255) brightness = 255;
  } else {
    brightness -= step;
    if (brightness < 0) brightness = 0;
  }

  CRGB c = color;
  c.nscale8((uint8_t)brightness);  // applique la luminosité 0..255
  setAllLeds(c);

  needsUpdate = true;
  lastUpdate = millis();
}



void LedManager::breathe(CRGB color) {
  static unsigned long lastUpdate = 0;
  static float breath = 0;
  static float direction = 1;

  
  
  if (millis() - lastUpdate > 20) {
    breath += direction * 0.02;
    if (breath >= 1.0) {
      breath = 1.0;
      direction = -1;
    } else if (breath <= 0.1) {
      breath = 0.1;
      direction = 1;
    }
    
    uint8_t brightness = (uint8_t)(255 * breath);
    CRGB fadedColor = color;
    fadedColor.fadeToBlackBy(255 - brightness);
    
    setAllLeds(fadedColor);
    lastUpdate = millis();
  }
}

void LedManager::chenillard(CRGB color, int speed) {
  static int pos = 0;
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > (unsigned long)speed) {
    clear();
    leds[pos] = color;
    pos = (pos + 1) % NUM_LEDS;
    needsUpdate = true;
    lastUpdate = millis();
  }
}

// Alternance rouge / vert
void LedManager::redGreenAlternance(int speed) {
  static bool state = false;
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > (unsigned long)speed) {
    state = !state;
    if (state) {
      setAllLeds(CRGB::Red);
    } else {
      setAllLeds(CRGB::Green);
    }
    needsUpdate = true;
    lastUpdate = millis();
  }
}

void LedManager::chenillardRainbow(int speed) {
  static int pos = -1;
  static int prev = -1;
  static uint8_t hue = 0;               // teinte HSV (0–255)
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate < (unsigned long)speed) return;

  // Éteindre l'ancienne LED
  if (prev >= 0 && prev < NUM_LEDS) {
    leds[prev] = CRGB::Black;
  }

  // Avancer la position
  pos = (pos + 1) % NUM_LEDS;

  // Choisir une couleur arc-en-ciel
  leds[pos] = CHSV(hue, 255, 255);

  // Faire évoluer la teinte à chaque pas
  hue += 8; // incrément de couleur, ajuste pour plus/moins de variations

  prev = pos;
  needsUpdate = true;
  lastUpdate = millis();
}

void LedManager::circleRainbow(int speed) {
  static uint8_t hue = 0;
  static unsigned long lastUpdate = 0;
  static bool outerActive = true; // alterner entre cercle extérieur et intérieur

  if (millis() - lastUpdate < (unsigned long)speed) return;
  lastUpdate = millis();

  // Efface tout
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  if (outerActive) {
    // Cercle extérieur (0-6)
    for (int j = 0; j < 7; j++) {
      leds[j] = CHSV((hue + j * 256 / 7) & 255, 255, 255);
    }
  } else {
    // Cercle intérieur (7-12)
    for (int j = 7; j < NUM_LEDS; j++) {
      leds[j] = CHSV((hue + (j - 7) * 256 / 6) & 255, 255, 255);
    }
  }
  hue += 5;                // fait tourner les couleurs
  outerActive = !outerActive; // alterne cercle ext / int
}


// Arc-en-ciel défilant
void LedManager::rainbowCycle(int speed) {
  static uint8_t hue = 0;
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > (unsigned long)speed) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV((hue + (i * 256 / NUM_LEDS)) & 255, 255, 255);
    }
    hue++;
    needsUpdate = true;
    lastUpdate = millis();
  }
}

void LedManager::flashRandomColors(uint8_t times, uint16_t delayMs) {
    static uint8_t flashCount = 0;
    static unsigned long lastFlash = 0;
    static bool initialized = false;
    
    if (!initialized) {
        flashCount = 0;
        initialized = true;
    }
    
    if (flashCount >= times) {
        clear();
        needsUpdate = true;
        flashCount = 0;
        initialized = false;
        return;
    }
    
    if (millis() - lastFlash > delayMs) {
        for(int pad = 0; pad < NUM_TOUCH_PADS; pad++) {
            leds[pad] = CHSV(random8(), 255, 255);
        }
        needsUpdate = true;
        flashCount++;
        lastFlash = millis();
    }
}
// void LedManager::flashRandomColors(uint8_t times, uint16_t delayMs) {
//     for(int i=0; i<times; i++){
//         for(int pad=0; pad<NUM_TOUCH_PADS; pad++){
//             leds[pad] = CHSV(random8(), 255, 255);
//         }
//         show();
//         delay(delayMs);
//     }
//     clear();
// }

void LedManager::sequentialWave(uint8_t repeats, uint16_t delayMs, CRGB color) {
    static uint8_t currentRepeat = 0;
    static int currentPad = 0;
    static unsigned long lastUpdate = 0;
    static bool initialized = false;
    static bool ledOn = true;
    
    if (!initialized) {
        currentRepeat = 0;
        currentPad = 0;
        ledOn = true;
        initialized = true;
    }
    
    if (currentRepeat >= repeats) {
        clear();
        needsUpdate = true;
        currentRepeat = 0;
        currentPad = 0;
        initialized = false;
        return;
    }
    
    if (millis() - lastUpdate > delayMs) {
        if (ledOn) {
            leds[currentPad] = color;
            ledOn = false;
        } else {
            leds[currentPad] = CRGB::Black;
            ledOn = true;
            currentPad++;
            
            if (currentPad >= NUM_TOUCH_PADS) {
                currentPad = 0;
                currentRepeat++;
            }
        }
        needsUpdate = true;
        lastUpdate = millis();
    }
}

// void LedManager::sequentialWave(uint8_t repeats, uint16_t delayMs, CRGB color) {
//     for(int r=0; r<repeats; r++){
//         for(int pad=0; pad<NUM_TOUCH_PADS; pad++){
//             leds[pad] = color;
//             show();
//             delay(delayMs);
//             leds[pad] = CRGB::Black;
//         }
//     }
// }

// void LedManager::rainbowFade(uint16_t speed) {
//     for(int b=0; b<=255; b+=5){
//         for(int pad=0; pad<NUM_TOUCH_PADS; pad++){
//             leds[pad] = CHSV((pad*16 + b) % 255, 255, b);
//         }
//         show();
//         delay(speed);
//     }
// }



void LedManager::rainbowFade(uint16_t speed) {
    static int brightness = 0;
    static unsigned long lastUpdate = 0;
    static bool effectRunning = false;
    
    if (!effectRunning) {
        brightness = 0;
        effectRunning = true;
    }
    
    if (millis() - lastUpdate < speed) return;
    lastUpdate = millis();
    
    for(int pad=0; pad<NUM_TOUCH_PADS; pad++){
        leds[pad] = CHSV((pad*16 + brightness) % 255, 255, brightness);
    }
    needsUpdate = true;
    
    brightness += 5;
    if (brightness > 255) {
        brightness = 0;
        effectRunning = false;
        clear();  // ✓ Efface à la fin du cycle
    }
}

void LedManager::strobe(CRGB color, uint8_t times, uint16_t delayMs) {
    static uint8_t strobeCount = 0;
    static bool strobeOn = false;
    static unsigned long lastToggle = 0;
    static bool initialized = false;
    
    if (!initialized) {
        strobeCount = 0;
        strobeOn = false;
        initialized = true;
    }
    
    if (strobeCount >= times * 2) {
        clear();
        needsUpdate = true;
        strobeCount = 0;
        initialized = false;
        return;
    }
    
    if (millis() - lastToggle > delayMs) {
        if (strobeOn) {
            setAllLeds(color);
        } else {
            clear();
        }
        strobeOn = !strobeOn;
        strobeCount++;
        lastToggle = millis();
    }
}

// void LedManager::strobe(CRGB color, uint8_t times, uint16_t delayMs){
//     for(int i=0; i<times; i++){
//         setAllLeds(color);
//         show();
//         delay(delayMs);
//         clear();
//         show();
//         delay(delayMs);
//     }
// }

void LedManager::rainbowPulse(uint16_t speed) {
    static int brightness = 0;
    static unsigned long lastUpdate = 0;
    static bool initialized = false;
    
    if (!initialized) {
        brightness = 0;
        initialized = true;
    }
    
    if (millis() - lastUpdate < speed) return;
    lastUpdate = millis();
    
    for(int pad = 0; pad < NUM_TOUCH_PADS; pad++) {
        leds[pad] = CHSV((pad*10 + brightness) % 255, 255, sin8(brightness));
    }
    needsUpdate = true;
    
    brightness += 5;
    if (brightness > 255) {
        brightness = 0;
        // Boucle infinie ou arrêt :
        // clear();
        // initialized = false;
    }
}

// void LedManager::rainbowPulse(uint16_t speed){
//     for(int b=0; b<=255; b+=5){
//         for(int pad=0; pad<NUM_TOUCH_PADS; pad++){
//             leds[pad] = CHSV((pad*10 + b) % 255, 255, sin8(b));
//         }
//         show();
//         delay(speed);
//     }
// }

void LedManager::selectFx(int fx) {
  if (fx < 0) fx = 0;
  if (fx > 11) fx = 11;
  
  // Si changement d'effet, reset
  static int lastFx = -1;
  if (fx != lastFx) {
    clear();
    show();
    lastFx = fx;
    // Ici vous pourriez ajouter un système de reset des static
    // mais c'est complexe en C++, donc on compte sur le clear()
  }
  
  switch(fx) {
    case 0:
      rainbow();
      break;
    case 1:
      breathe(CRGB::Blue);
      break;
    case 2:
      chenillard(CRGB::Green, 100);
      break;
    case 3:
      redGreenAlternance(500);
      break;
    case 4:
      rainbowCycle(100);
      break;
    case 5:
      chenillardRainbow(100);
      break;
    case 6:
      circleRainbow(200);
      break;
    case 7:
      rainbowFade(20);
      break;
    case 8:
      strobe(CRGB::Yellow, 10, 50);
      break;
    case 9:
      rainbowPulse(30);
      break;
    case 10:
      flashRandomColors(50, 50);
      break;
    case 11:
      sequentialWave(5, 30, CRGB::Green);
      break;
    default:
      clear();
      break;
  }
}

// void LedManager::selectFx(int fx) {
//   if (fx < 0) fx = 0;
//   if (fx > 11) fx = 11;
//   switch(fx) {
//     case 0:
//       rainbow();
//       break;
//     case 1:
//       breathe(CRGB::Blue);
//       break;
//     case 2:
//       chenillard(CRGB::Green, 100);
//       break;
//     case 3:
//       redGreenAlternance(500);
//       break;
//     case 4:
//       rainbowCycle(100);
//       break;
//     case 5:
//       chenillardRainbow(100);
//       break;
//     case 6:
//       circleRainbow(200);
//       break;
//     case 7:
//       rainbowFade(20);
//       break;
//     case 8:
//       strobe(CRGB::Yellow, 10, 50);
//       break;
//     case 9:
//       rainbowPulse(30);
//       break;
//     case 10:
//       flashRandomColors(50, 50);
//       break;
//     case 11:
//       sequentialWave(5, 30, CRGB::Green);
//       break;
//     default:
//       clear();
//       break;
//   }
// }

void LedManager::victorySequence() {
    static int stage = 0;
    static unsigned long stageStartTime = 0;
    static bool initialized = false;
    
    if (!initialized) {
        stage = 0;
        stageStartTime = millis();
        initialized = true;
    }
    
    unsigned long elapsed = millis() - stageStartTime;
    
    switch(stage) {
        case 0: // Strobe
            strobe(CRGB::Yellow, 10, 50);
            if (elapsed > 1000) { // 10 * 50 * 2 = 1000ms
                stage++;
                stageStartTime = millis();
            }
            break;
            
        case 1: // Sequential Wave
            sequentialWave(5, 30, CRGB::Green);
            if (elapsed > 1950) { // 5 * 13 * 30 = ~1950ms
                stage++;
                stageStartTime = millis();
            }
            break;
            
        case 2: // Flash Random
            flashRandomColors(50, 50);
            if (elapsed > 2500) { // 50 * 50 = 2500ms
                stage++;
                stageStartTime = millis();
            }
            break;
            
        case 3: // Rainbow Pulse
            rainbowPulse(30);
            if (elapsed > 1530) { // (255/5) * 30 = 1530ms
                stage = 0;
                initialized = false;
                clear();
                needsUpdate = true;
            }
            break;
    }
}

// void LedManager::victorySequence(){
//     strobe(CRGB::Yellow, 10, 50);
//     sequentialWave(5, 30, CRGB::Green);
//     flashRandomColors(50, 50);
//     rainbowPulse(30);
// }


#endif