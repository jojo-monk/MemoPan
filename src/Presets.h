#ifndef PRESETS_H
#define PRESETS_H

#include "SoundModes.h"
#include "config.h"

//#define NUM_PRESETS 32

// Déclaration (extern)
extern const Preset DEFAULT_PRESETS[NUM_PRESETS];

// Accès pratique
const Preset& getPreset(int index);
//void initPresets();
#endif // PRESETS_H
