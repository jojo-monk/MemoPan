// PresetStorage.h
#ifndef PRESET_STORAGE_H
#define PRESET_STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include "SoundModes.h"

// Structure simplifiée compatible avec la sauvegarde (sans pointeurs)
// Les noms de samples ne sont PAS sauvegardés (stockés dans la Teensy)
struct __attribute__((packed)) PresetFlash {
    SoundCategory type;
    char presetName[13];
    char soundName[13];
    uint8_t soundTypeIndex;
    
    // Paramètres selon le type
    union __attribute__((packed)) {
        SynthParams synth;
        DrumParams drum;
        StringParams string;
        struct __attribute__((packed)) {
            // Pas de noms de fichiers - ils sont fixes dans la Teensy
            int moogAttack, moogDecay;
            float moogSustain, moogRelease;

        } sample;
    } sound;
    
    // Les effets sont TOUJOURS sauvegardés (modifiables pour tous les types)
    EffectsParams effects;
    
    // Version pour évolutions futures
    uint16_t version = 1;
};


class PresetStorage {
private:
    LittleFS_Program myfs;
    static const int MAX_PRESETS = 32;  // Limité à 32 presets
    bool initialized = false;
    
    // Convertir Preset vers PresetFlash (pour sauvegarde)
    void convertToFlash(const Preset& preset, PresetFlash& flash);
    
    // Convertir PresetFlash vers Preset (pour chargement)
    void convertFromFlash(const PresetFlash& flash, Preset& preset, int slot);
    
    // Buffers statiques pour les chaînes (éviter allocation dynamique)
    static char nameBuffer[32][13];
    static char fileBuffer[32][13];
    static int nameBufferIndex;
    static int fileBufferIndex;
    
public:
    PresetStorage();
    
    // Initialiser le système de fichiers
    bool begin();
    
    // Sauvegarder un preset dans la Flash
    bool savePreset(int slot, const Preset& preset);
    
    // Charger un preset depuis la Flash
    bool loadPreset(int slot, Preset& preset);
    bool saveAllPresets(const Preset* presets, int count);
    
    // Vérifier si un preset existe
    bool presetExists(int slot);
    
    // Supprimer un preset
    bool deletePreset(int slot);
    
    // Lister tous les presets disponibles
    void listPresets();
    
    // Obtenir le nombre de presets sauvegardés
    int getPresetCount();
    
    // Formater la zone de stockage (ATTENTION: efface tout!)
    bool format();
    void printFSInfo();
};

#endif // PRESET_STORAGE_H