// SysExHandler.h
#ifndef SYSEX_HANDLER_H
#define SYSEX_HANDLER_H

#include <Arduino.h>
#include "SoundModes.h"
#include "PresetStorage.h"

class SysExHandler {
private:
    static const byte MANUFACTURER_ID = 0x7D;
    static const byte CMD_SEND_PRESET = 0x01;
    static const byte CMD_REQUEST_PRESET = 0x02;
    static const byte CMD_SAVE_PRESET = 0x03;
    static const byte CMD_PRESET_DATA = 0x04;
    
    Preset* presetStorage;  // Pointeur vers le stockage des presets
    int maxPresets;
    Preset* currentPreset;  // Pointeur vers le preset actif
    PresetStorage* storage;
    static char tempPresetName[13];
    // Fonctions de décodage
    int16_t decodeInt16(const byte* data, int& idx);
    float decodeFloat(const byte* data, int& idx);
    void decodeString(const byte* data, int& idx, char* output, int maxLen);
    
    // Fonctions d'encodage
    void encodeInt16(byte* output, int& idx, int16_t value);
    void encodeFloat(byte* output, int& idx, float value);
    void encodeString(byte* output, int& idx, const char* str, int maxLen);
    
    // Gestion des presets
    void parseSynthParams(const byte* data, int& idx, SynthParams& params);
    void parseDrumParams(const byte* data, int& idx, DrumParams& params);
    void parseStringParams(const byte* data, int& idx, StringParams& params);
    void parseSampleParams(const byte* data, int& idx, SampleParams& params);
    void parseEffects(const byte* data, int& idx, EffectsParams& effects);
    
    void encodeSynthParams(byte* output, int& idx, const SynthParams& params);
    void encodeDrumParams(byte* output, int& idx, const DrumParams& params);
    void encodeStringParams(byte* output, int& idx, const StringParams& params);
    void encodeSampleParams(byte* output, int& idx, const SampleParams& params);
    void encodeEffects(byte* output, int& idx, const EffectsParams& effects);
    
public:
    SysExHandler(Preset* presetBank, int numPresets, Preset* current, PresetStorage* stor);

    
    // Callback principal pour les messages SysEx
    void handleSysEx(byte* data, unsigned int length);
    
    // Envoyer un preset vers l'ordinateur
    void sendPresetData(int presetNumber);
    
    // Callback pour appliquer le preset reçu
    void (*onPresetReceived)(Preset* preset) = nullptr;
};

#endif // SYSEX_HANDLER_H