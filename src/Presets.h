#ifndef PRESETS_H
#define PRESETS_H

#include "SoundModes.h"
#include "config.h"


// Déclaration (extern)
extern const Preset DEFAULT_PRESETS[NUM_PRESETS];

// Accès pratique
const Preset& getPreset(int index);
#endif // PRESETS_H
