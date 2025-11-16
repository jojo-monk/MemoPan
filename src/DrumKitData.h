#ifndef DRUMKIT_DATA_H
#define DRUMKIT_DATA_H
#include "config.h"
#include "audioSamples/DrumsKit1.h"
#include "audioSamples/DrumsKit2.h"
#include "audioSamples/DrumsKit3.h"

// Structure pour organiser les samples par durée
struct DrumKitSamples {
    const unsigned int* samplesLong[NUM_TOUCH_PADS];
    const unsigned int* samplesMoyen[NUM_TOUCH_PADS];
    const unsigned int* samplesCourt[NUM_TOUCH_PADS];
};

// Déclaration du nombre de drumkits disponibles
extern const int NUM_DRUMKITS;

// Déclaration du tableau de drumkits
extern const DrumKitSamples drumKits[];

#endif // DRUMKIT_DATA_H