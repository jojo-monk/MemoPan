#ifndef DEBUG_H
#define DEBUG_H

// Active (1) ou désactive (0) les messages de debug
#define DEBUG_VERBOSE 0
#define DEBUG_LED 0
#define DEBUG_SOUND 0
#define DEBUG_TOUCH 0
#define DEBUG_UI 0
#define DEBUG_MAIN 0
#define DEBUG_EEPROM 0
#define DEBUG_MIDI 0
#define DEBUG_ARP 0
#define DEBUG_TEST 0 // Activer les tests au démarrage
#define DEBUG_BEGIN(speed)   Serial.begin(speed)

#if DEBUG_VERBOSE
  #define DEBUG_LED 1
  #define DEBUG_SOUND 1
  #define DEBUG_TOUCH 1
  #define DEBUG_UI 1
  #define DEBUG_MAIN 1
  #define DEBUG_EEPROM 1
#endif

#if DEBUG_LED
  #define DEBUG_VAR_LED(tag, x) \
    do { \
        Serial.print("[LED]: "); \
        Serial.print(tag); \
        Serial.print(" : "); \
        Serial.println(x); \
    } while(0)
  #define DEBUG_INFO_LED(x) \
    do { \
      Serial.print("[LED]: "); \
      Serial.println(x); \
    } while(0)
  #define DEBUG_INFO2_LED(x, y) \
  do { \
      Serial.print("[LED]: "); \
      Serial.println(x, y); \
    } while(0)
#else
  #define DEBUG_VAR_LED(tag, x)
  #define DEBUG_INFO_LED(x)
  #define DEBUG_INFO2_LED(x, y)
#endif

#if DEBUG_SOUND
  #define DEBUG_VAR_SOUND(tag, x) \
    do {  \
        Serial.print("[SOUND]: "); \
        Serial.print(tag);  \
        Serial.print(" : ");  \
        Serial.println(x);  \
    } while(0)
  #define DEBUG_INFO_SOUND(x) \
    do { \
      Serial.print("[SOUND]: "); \
      Serial.println(x); \
    } while(0)
  #define DEBUG_INFO2_SOUND(x, y) \
  do { \
      Serial.print("[SOUND]: "); \
      Serial.println(x, y); \
    } while(0)
#else
  #define DEBUG_VAR_SOUND(tag, x)
  #define DEBUG_INFO_SOUND(x)
  #define DEBUG_INFO2_SOUND(x, y)
#endif

#if DEBUG_TOUCH
  #define DEBUG_VAR_TOUCH(tag, x) \
    do {  \
        Serial.print("[TOUCH]: "); \
        Serial.print(tag);  \
        Serial.print(" : ");  \
        Serial.println(x);  \
    } while(0)
  #define DEBUG_INFO_TOUCH(x) \
    do { \
      Serial.print("[TOUCH]: "); \
      Serial.println(x); \
    } while(0)
  #define DEBUG_INFO2_TOUCH(x, y) \
  do { \
      Serial.print("[TOUCH]: "); \
      Serial.println(x, y); \
    } while(0)
#else
  #define DEBUG_VAR_TOUCH(tag, x)
  #define DEBUG_INFO_TOUCH(x)
  #define DEBUG_INFO2_TOUCH(x, y)
#endif

#if DEBUG_UI
  #define DEBUG_VAR_UI(tag, x) \
    do {  \
        Serial.print("[UI]: "); \
        Serial.print(tag);  \
        Serial.print(" : ");  \
        Serial.println(x);  \
    } while(0)
  #define DEBUG_INFO_UI(x) \
    do { \
      Serial.print("[UI]: "); \
      Serial.println(x); \
    } while(0)
  #define DEBUG_INFO2_UI(x, y) \
  do { \
      Serial.print("[UI]: "); \
      Serial.println(x, y); \
    } while(0)
#else
  #define DEBUG_VAR_UI(tag, x)
  #define DEBUG_INFO_UI(x)
  #define DEBUG_INFO2_UI(x, y)
#endif

#if DEBUG_MAIN
  #define DEBUG_VAR_MAIN(tag, x) \
    do {  \
        Serial.print("[MAIN]: "); \
        Serial.print(tag);  \
        Serial.print(" : ");  \
        Serial.println(x);  \
    } while(0)
  #define DEBUG_INFO_MAIN(x) \
    do { \
      Serial.print("[MAIN]: "); \
      Serial.println(x); \
    } while(0)
  #define DEBUG_INFO2_MAIN(x, y) \
  do { \
      Serial.print("[MAIN]: "); \
      Serial.println(x, y); \
    } while(0)
#else
  #define DEBUG_VAR_MAIN(tag, x)
  #define DEBUG_INFO_MAIN(x)
  #define DEBUG_INFO2_MAIN(x, y)
#endif

#if DEBUG_EEPROM
  #define DEBUG_EEPROM_PRINT(x) do { x; } while(0)
  #define DEBUG_VAR_EEPROM(tag, x) \
  do {  \
      Serial.print("[EEPROM]: "); \
      Serial.print(tag);  \
      Serial.print(" : ");  \
      Serial.println(x);  \
  } while(0)
  #define DEBUG_INFO_EEPROM(x) \
    do { \
      Serial.print("[EEPROM]: "); \
      Serial.println(x); \
    } while(0)
#else
  #define DEBUG_EEPROM_PRINT(x)
  #define DEBUG_INFO_EEPROM(x)
  #define DEBUG_VAR_EEPROM(tag, x)
#endif

#if DEBUG_MIDI
  #define DEBUG_VAR_MIDI(tag, x) \
    do {  \
        Serial.print("[MIDI]: "); \
        Serial.print(tag);  \
        Serial.print(" : ");  \
        Serial.println(x);  \
    } while(0)
  #define DEBUG_INFO_MIDI(x) \
    do { \
      Serial.print("[MIDI]: "); \
      Serial.println(x); \
    } while(0)
  #define DEBUG_INFO2_MIDI(x, y) \
  do { \
      Serial.print("[MIDI]: "); \
      Serial.println(x, y); \
    } while(0)
#else
  #define DEBUG_VAR_MIDI(tag, x)
  #define DEBUG_INFO_MIDI(x)
  #define DEBUG_INFO2_MIDI(x, y)
#endif

#if DEBUG_ARP
  #define DEBUG_VAR_ARP(tag, x) \
    do {  \
        Serial.print("[ARP]: "); \
        Serial.print(tag);  \
        Serial.print(" : ");  \
        Serial.println(x);  \
    } while(0)
  #define DEBUG_INFO_ARP(x) \
    do { \
      Serial.print("[ARP]: "); \
      Serial.println(x); \
    } while(0)
  #define DEBUG_INFO2_ARP(x, y) \
  do { \
      Serial.print("[ARP]: "); \
      Serial.println(x, y); \
    } while(0)
#else
  #define DEBUG_VAR_ARP(tag, x)
  #define DEBUG_INFO_ARP(x)
  #define DEBUG_INFO2_ARP(x, y)
#endif

#endif

