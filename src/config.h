#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// ===== CONFIGURATION HARDWARE =====

// Pins
#define LED_DATA_PIN 1
#define ENCODER_PIN_A 3
#define ENCODER_PIN_B 2
#define ENCODER_BUTTON_PIN 4
// Configuration OLED SPI
#define OLED_MOSI   11
#define OLED_CLK    13
#define OLED_DC     5
#define OLED_CS     10
#define OLED_RESET  9

#define BATTERY_PIN A2  // Pin de lecture de la batterie


// MPR121 (I2C par défaut sur Teensy 4: SDA=18, SCL=19) et IRQ=22
#define MPR121_I2C_ADDR 0x5A

// AudioShield (pins par défaut)
#define AUDIO_INPUT_LINEIN 0
#define AUDIO_INPUT_MIC 1
#define VOL_POT A1

// ===== CONFIGURATION JEU =====

// Nombre de composants
#define NUM_LEDS 12
#define NUM_TOUCH_PADS 12
#define NUM_SOUND_MODES 32
#define NUM_PRESETS 34
#define MAX_PRESET_NAME_LEN 13
#define NUM_WAVEFORMS 18

// Configuration LEDs
#define LED_BRIGHTNESS 65
#define LED_TYPE WS2812
#define COLOR_ORDER GRB

// Configuration tactile
#define TOUCH_THRESHOLD 12
#define TOUCH_RELEASE_THRESHOLD 6

// Configuration ecran oled
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define CC_DUMP_REQUEST = 124;

// ===== COULEURS =====

// Couleurs pour chaque touche (cycle de couleurs attrayant)
const CRGB TOUCH_COLORS[NUM_TOUCH_PADS] = {
  CRGB::Red,      // 0
  CRGB::Orange,   // 1
  CRGB::Yellow,   // 2
  CRGB::Green,    // 3
  CRGB::Cyan,     // 4
  CRGB::Blue,     // 5
  CRGB::Purple,   // 6
  CRGB::Magenta,  // 7
  CRGB::Pink,     // 8
  CRGB::White,    // 9
  CRGB::Lime,     // 10
  CRGB::Aqua      // 11
};

// Couleurs pour les modes sonores
const CRGB MODE_COLORS[NUM_SOUND_MODES] = {
  CRGB::Red,    // Mode 0: Sine
  CRGB::Green,  // Mode 1: Saw
  CRGB::Blue,   // Mode 2: Square
  CRGB::Yellow,  // Mode 3: HandPan
  CRGB::Pink,    // Mode 4: Pluck
  CRGB::Lime    // Mode 5: Boom
};

// ===== FRÉQUENCES SONORES =====

// Notes musicales (gamme chromatique à partir de C4)
const float BASE_FREQUENCIES[NUM_TOUCH_PADS] = {
  261.63, // C4
  277.18, // C#4
  293.66, // D4
  311.13, // D#4
  329.63, // E4
  349.23, // F4
  369.99, // F#4
  392.00, // G4
  415.30, // G#4
  440.00, // A4
  466.16, // A#4
  493.88  // B4
};



const int CHROMATIC_INTERVALS[NUM_TOUCH_PADS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const int MAJOR_INTERVALS[NUM_TOUCH_PADS] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19};
const int MINOR_INTERVALS[NUM_TOUCH_PADS] = {0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19};
const int PENTATONIC_MAJOR_INTERVALS[NUM_TOUCH_PADS] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26};
const int PENTATONIC_MINOR_INTERVALS[NUM_TOUCH_PADS] = {0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27};
const int AKEBONO_INTERVALS[NUM_TOUCH_PADS] = {0, 1, 5, 7, 8, 12, 13, 17, 19, 20, 24, 25};
const int HIJAZ_INTERVALS[NUM_TOUCH_PADS] = {0, 1, 4, 5, 7, 8, 10, 12, 13, 16, 17, 19};
const int TONESCALE_INTERVALS[NUM_TOUCH_PADS] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22};
const int DIMSCALE_INTERVALS[NUM_TOUCH_PADS] = {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16};
//const char *gamme[] = { "Majeur", "Mineur", "Chromatique", "Penta Maj", "Penta min" };

const int HANDPAN_MODE[NUM_TOUCH_PADS] = {0, 1, 5, 9, 10, 6, 2, 3, 7, 11, 8, 4};
const int SPIRAL_MODE[NUM_TOUCH_PADS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const int NORMAL_MODE[NUM_TOUCH_PADS] = {0, 1, 3, 5, 7, 9, 11, 2, 4, 6, 8, 10};
const int8_t OCTAVE_MIN = -2;
const int8_t OCTAVE_MAX = 2;
// ===== PARAMÈTRES DIVERS =====
#define NUM_STEP_INTERVALS 6
#define NUM_ARP_MODES 4
const int STEP_INTERVALS_MS[NUM_STEP_INTERVALS] = {1, 2, 3, 4, 6, 8}; // Correspond à 1/4, 1/8, 1/8T, 1/16, 1/16T, 1/32 

// ===== TIMING =====

#define DEBOUNCE_TIME 50
#define MODE_DISPLAY_TIME 1000
#define SOUND_DURATION 200
#define TIMING_SLOW 600
#define TIMING_MEDIUM 400
#define TIMING_QUICK 250
#define TIMING_SPEED 120



// États de l'application
enum EtatApplication {
  ETAT_MENU_PRINCIPAL,
  ETAT_MENU_PARAMETRES,
  ETAT_JEU_SIMON,
  ETAT_SCORES,
  ETAT_TEST_LEDS,
  ETAT_TEST_SONS
};

#define NUM_MAX_PLAYER 10


#endif