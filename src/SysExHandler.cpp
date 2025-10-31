// SysExHandler.cpp
#include "SysExHandler.h"
#include "PresetStorage.h"
#include "ui.h"
#include "config.h"
//#include <MIDI.h>

// Créer une instance MIDI
//MIDI_CREATE_DEFAULT_INSTANCE();


// ========== FONCTIONS DE DÉCODAGE ==========
SysExHandler::SysExHandler(Preset* presetBank, int numPresets, Preset* current, PresetStorage* stor)
    : presetStorage(presetBank),
      maxPresets(numPresets),
      currentPreset(current),
      storage(stor)
{
}

int16_t SysExHandler::decodeInt16(const byte* data, int& idx) {
    if (idx + 3 > 1024) return 0;  // Protection
    
    int16_t value = data[idx] | (data[idx+1] << 7) | (data[idx+2] << 14);
    idx += 3;
    
    // Gestion du signe
    if (value & 0x8000) {
        value -= 0x10000;
    }
    return value;
}

// Décodage float sur 4×7 bits
float SysExHandler::decodeFloat(const byte* data, int& idx) {
    uint8_t bytes[4] = {0};
    for (int i = 0; i < 4; i++) {
        bytes[i] = (data[idx] & 0x7F) | ((data[idx+1] & 0x7F) << 7);
        idx += 2;
    }
    float result;
    memcpy(&result, bytes, 4);
    return result;
}

// float SysExHandler::decodeFloat(const byte* data, int& idx) {
//     if (idx + 8 > 1024) return 0.0f;  // Protection
    
//     byte bytes[4];
//     for (int i = 0; i < 4; i++) {
//         bytes[i] = data[idx] | (data[idx+1] << 7);
//         idx += 2;
//     }
    
//     float result;
//     memcpy(&result, bytes, 4);
//     return result;
// }

void SysExHandler::decodeString(const byte* data, int& idx, char* output, int maxLen) {
    int i;
    for (i = 0; i < maxLen; i++) {
        if (idx >= 1024) break;
        output[i] = data[idx++];
    }
    output[i] = '\0';  // Terminateur après les caractères lus
}


// ========== FONCTIONS D'ENCODAGE ==========

void SysExHandler::encodeInt16(byte* output, int& idx, int16_t value) {
    uint16_t uval = (uint16_t)value;
    output[idx++] = uval & 0x7F;
    output[idx++] = (uval >> 7) & 0x7F;
    output[idx++] = (uval >> 14) & 0x03;
}

void SysExHandler::encodeFloat(byte* output, int& idx, float value) {
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
    for (int i = 0; i < 4; i++) {
        output[idx++] = bytes[i] & 0x7F;       // bits 0-6
        output[idx++] = (bytes[i] >> 7) & 0x7F; // bits 7-13
    }
}


// void SysExHandler::encodeFloat(byte* output, int& idx, float value) {
//     byte bytes[4];
//     memcpy(bytes, &value, 4);
    
//     for (int i = 0; i < 4; i++) {
//         output[idx++] = bytes[i] & 0x7F;
//         output[idx++] = (bytes[i] >> 7) & 0x01;
//     }
// }
void SysExHandler::encodeString(byte* buffer, int& idx, const char* str, int maxLen) {
    Serial.print("encodeString: '"); Serial.print(str ? str : "NULL"); Serial.println("'");
    Serial.flush();
    
    if (str == nullptr) {
        Serial.println("ATTENTION: str est NULL!");
        // Remplir avec des espaces
        for (int i = 0; i < maxLen; i++) {
            buffer[idx++] = ' ';
        }
        return;
    }
    
    int len = strlen(str);
    if (len > maxLen) len = maxLen;
    
    for (int i = 0; i < len; i++) {
        buffer[idx++] = (byte)str[i];
    }
    
    // Remplir le reste avec des espaces ou des zéros
    for (int i = len; i < maxLen; i++) {
        buffer[idx++] = ' '; // ou 0
    }
}
// void SysExHandler::encodeString(byte* output, int& idx, const char* str, int maxLen) {
//     int len = strlen(str);
//     for (int i = 0; i < maxLen; i++) {
//         if (i < len) {
//             output[idx++] = str[i] & 0x7F;
//         } else {
//             output[idx++] = 0;
//         }
//     }
// }

// ========== PARSERS ==========

void SysExHandler::parseSynthParams(const byte* data, int& idx, SynthParams& params) {
    params.waveformType = data[idx++];
    params.amplitude = decodeFloat(data, idx);
    params.attack = decodeInt16(data, idx);
    params.decay = decodeInt16(data, idx);
    params.sustain = decodeFloat(data, idx);
    params.release = decodeInt16(data, idx);
    params.freqMod = decodeFloat(data, idx);
    params.phaseMod = decodeFloat(data, idx);
    params.ratioFm = decodeFloat(data, idx);
    params.modAmplitude = decodeFloat(data, idx);
    params.moogAttack = decodeInt16(data, idx);
    params.moogDecay = decodeInt16(data, idx);
    params.moogSustain = decodeFloat(data, idx);
    params.moogRelease = decodeFloat(data, idx);
    params.lpfResonance = decodeFloat(data, idx);
    params.octaveShift = data[idx++];
    params.lpfCutoff = decodeFloat(data, idx);
    params.lpfEnvAmount = decodeFloat(data, idx);
    params.FMEnvAmount = decodeFloat(data, idx);
    params.FMWaveform = data[idx++];
    params.lpfOctaveControl = decodeFloat(data, idx);

}

void SysExHandler::parseDrumParams(const byte* data, int& idx, DrumParams& params) {
    params.length = decodeInt16(data, idx);
    params.secondMix = decodeFloat(data, idx);
    params.pitchMod = decodeFloat(data, idx);
    params.moogAttack = decodeInt16(data, idx);
    params.moogDecay = decodeInt16(data, idx);
    params.moogSustain = decodeFloat(data, idx);
    params.moogRelease = decodeFloat(data, idx);
    params.octaveShift = data[idx++];
}

void SysExHandler::parseStringParams(const byte* data, int& idx, StringParams& params) {
    params.velocity = decodeFloat(data, idx);
    params.moogAttack = decodeInt16(data, idx);
    params.moogDecay = decodeInt16(data, idx);
    params.moogSustain = decodeFloat(data, idx);
    params.moogRelease = decodeFloat(data, idx);
    params.octaveShift = data[idx++];
}

void SysExHandler::parseSampleParams(const byte* data, int& idx, SampleParams& params) {
    // int numFiles = data[idx++];
    
    // // Parser les noms de fichiers
    // char tempFilename[33];
    // for (int i = 0; i < 12; i++) {
    //     decodeString(data, idx, tempFilename, 32);
    //     // Note: Il faudra allouer de la mémoire dynamique ou utiliser un buffer statique
    //     // pour stocker les noms de fichiers si nécessaire
    // }
    
    params.moogAttack = decodeInt16(data, idx);
    params.moogDecay = decodeInt16(data, idx);
    params.moogSustain = decodeFloat(data, idx);
    params.moogRelease = decodeFloat(data, idx);
}

void SysExHandler::parseEffects(const byte* data, int& idx, EffectsParams& effects) {
    effects.reverbRoomSize = decodeFloat(data, idx);
    effects.reverbDamping = decodeFloat(data, idx);
    effects.filterFreq = decodeFloat(data, idx);
    effects.filterResonance = decodeFloat(data, idx);
    effects.nChorus = data[idx++];
    effects.tremoloWaveform = data[idx++];
    effects.tremoloRate = decodeFloat(data, idx);
    effects.tremoloAmplitude = decodeFloat(data, idx);
    effects.moogLfoWaveform = data[idx++];
    effects.moogLfoRate = decodeFloat(data, idx);
    effects.moogLfoAmplitude = decodeFloat(data, idx);
    effects.noiseAmplitude = decodeFloat(data, idx);
    effects.noiseCutoff = decodeFloat(data, idx);
    effects.noiseResonance = decodeFloat(data, idx);
    effects.noiseEnvAttack = decodeInt16(data, idx);
    effects.noiseEnvDecay = decodeInt16(data, idx);
    effects.noiseEnvSustain = decodeFloat(data, idx);
    effects.noiseEnvRelease = decodeInt16(data, idx);
    effects.noiseCutoffEnvAmount = decodeFloat(data, idx);
    effects.noiseFilterType = data[idx++];
    effects.noiseOctaveControl = decodeFloat(data, idx);
    effects.moogOctaveControl = decodeFloat(data, idx);
    effects.moogCutoff = decodeFloat(data, idx);
    effects.moogResonance = decodeFloat(data, idx);
}

// ========== ENCODERS ==========

void SysExHandler::encodeSynthParams(byte* output, int& idx, const SynthParams& params) {
    Serial.print("Encoding waveform: ");
    Serial.println(params.waveformType);
    output[idx++] = params.waveformType & 0x7F;
    
    Serial.print("Encoding amplitude: ");
    Serial.println(params.amplitude, 6);
    encodeFloat(output, idx, params.amplitude);
    
    Serial.print("Encoding attack: ");
    Serial.println(params.attack);
    encodeInt16(output, idx, params.attack);
    encodeInt16(output, idx, params.decay);
    encodeFloat(output, idx, params.sustain);
    encodeInt16(output, idx, params.release);
    encodeFloat(output, idx, params.freqMod);
    encodeFloat(output, idx, params.phaseMod);
    encodeFloat(output, idx, params.ratioFm);
    encodeFloat(output, idx, params.modAmplitude);
    encodeInt16(output, idx, params.moogAttack);
    encodeInt16(output, idx, params.moogDecay);
    encodeFloat(output, idx, params.moogSustain);
    encodeFloat(output, idx, params.moogRelease);
    encodeFloat(output, idx, params.lpfResonance);
    output[idx++] = params.octaveShift & 0x7F;
    //encodeInt16(output, idx, params.octaveShift);
    encodeFloat(output, idx, params.lpfCutoff);
    encodeFloat(output, idx, params.lpfEnvAmount);
    encodeFloat(output, idx, params.FMEnvAmount);
    output[idx++] = params.FMWaveform & 0x7F;
    encodeFloat(output, idx, params.lpfOctaveControl);
}

void SysExHandler::encodeDrumParams(byte* output, int& idx, const DrumParams& params) {
    encodeInt16(output, idx, params.length);
    encodeFloat(output, idx, params.secondMix);
    encodeFloat(output, idx, params.pitchMod);
    encodeInt16(output, idx, params.moogAttack);
    encodeInt16(output, idx, params.moogDecay);
    encodeFloat(output, idx, params.moogSustain);
    encodeFloat(output, idx, params.moogRelease);
    output[idx++] = params.octaveShift & 0x7F;
    //encodeInt16(output, idx, params.octaveShift);
}

void SysExHandler::encodeStringParams(byte* output, int& idx, const StringParams& params) {
    encodeFloat(output, idx, params.velocity);
    encodeInt16(output, idx, params.moogAttack);
    encodeInt16(output, idx, params.moogDecay);
    encodeFloat(output, idx, params.moogSustain);
    encodeFloat(output, idx, params.moogRelease);
    output[idx++] = params.octaveShift & 0x7F;
    //encodeInt16(output, idx, params.octaveShift);
}

void SysExHandler::encodeSampleParams(byte* output, int& idx, const SampleParams& params) {
    // Note: Il faudra ajuster cette partie si on stocke plusieurs fichiers
    //int numFiles = 1; // Pour l'instant, on encode un seul fichier
    //output[idx++] = numFiles & 0x7F;
    
    // Encoder les noms de fichiers (ici, on utilise des noms fictifs)
    //const char* sampleFilename = "sample.wav";
    //encodeString(output, idx, sampleFilename, 32);
    
    encodeInt16(output, idx, params.moogAttack);
    encodeInt16(output, idx, params.moogDecay);
    encodeFloat(output, idx, params.moogSustain);
    encodeFloat(output, idx, params.moogRelease);
}

void SysExHandler::encodeEffects(byte* output, int& idx, const EffectsParams& effects) {
    encodeFloat(output, idx, effects.reverbRoomSize);
    encodeFloat(output, idx, effects.reverbDamping);
    encodeFloat(output, idx, effects.filterFreq);
    encodeFloat(output, idx, effects.filterResonance);
    output[idx++] = effects.nChorus & 0x7F;
    output[idx++] = effects.tremoloWaveform & 0x7F;
    encodeFloat(output, idx, effects.tremoloRate);
    encodeFloat(output, idx, effects.tremoloAmplitude);
    output[idx++] = effects.moogLfoWaveform & 0x7F;
    encodeFloat(output, idx, effects.moogLfoRate);
    encodeFloat(output, idx, effects.moogLfoAmplitude);
    encodeFloat(output, idx, effects.noiseAmplitude);
    encodeFloat(output, idx, effects.noiseCutoff);
    encodeFloat(output, idx, effects.noiseResonance);
    encodeInt16(output, idx, effects.noiseEnvAttack);
    encodeInt16(output, idx, effects.noiseEnvDecay);
    encodeFloat(output, idx, effects.noiseEnvSustain);
    encodeInt16(output, idx, effects.noiseEnvRelease);
    encodeFloat(output, idx, effects.noiseCutoffEnvAmount);
    output[idx++] = effects.noiseFilterType & 0x7F;
    encodeFloat(output, idx, effects.noiseOctaveControl);
    encodeFloat(output, idx, effects.moogOctaveControl);
    encodeFloat(output, idx, effects.moogCutoff);
    encodeFloat(output, idx, effects.moogResonance);
}

// ========== HANDLER PRINCIPAL ==========

void SysExHandler::handleSysEx(byte* data, unsigned int length) {

    Serial.print("SysEx reçu (");
    Serial.print(length);
    Serial.print(" bytes): ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print("0x");
        if (data[i] < 16) Serial.print("0");
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    if (length < 3) {
        Serial.println("SysEx trop court");
        return;
    }
    if (data[1] != MANUFACTURER_ID) {
        Serial.println("ID fabricant incorrect");
        return;
    } 
    byte command = data[2];
    int idx = 3;
    
    switch (command) {
        case CMD_SEND_PRESET: {
            // Recevoir un preset et l'appliquer
            if (length < 50) return;  // Taille minimale attendue
            
            byte presetType = data[idx++];
            
            // Décoder le nom
            char presetName[13];
            decodeString(data, idx, presetName, 12);
            //idx += 12;
            
            // Parser selon le type
            currentPreset->presetName = presetName;
            
            switch (presetType) {
                case 0: // SYNTH
                    currentPreset->sound.type = SYNTH;
                    parseSynthParams(data, idx, currentPreset->sound.synth);
                    break;
                case 1: // STRING
                    currentPreset->sound.type = STRING;
                    parseStringParams(data, idx, currentPreset->sound.string);
                    break;
                case 2: // DRUM
                    currentPreset->sound.type = DRUM;
                    parseDrumParams(data, idx, currentPreset->sound.drum);
                    break;
                case 3: // SAMPLE
                    currentPreset->sound.type = SAMPLE;
                    parseSampleParams(data, idx, currentPreset->sound.sample);
                    break;
            }
            
            // Parser les effets
            parseEffects(data, idx, currentPreset->effects);
            
            // Notifier l'application
            if (onPresetReceived) {
                onPresetReceived(currentPreset);
            }
            
            Serial.println("Preset reçu et appliqué");
            break;
        }
        
        case CMD_REQUEST_PRESET: {
            // Envoyer un preset stocké
            Serial.println("CMD_REQUEST_PRESET reçu !");
            byte presetNumber = data[idx];
            Serial.print("Preset demandé : ");
            Serial.println(presetNumber);
            if (presetNumber < maxPresets) {
                sendPresetData(presetNumber);
            }
            break;
        }
        case CMD_SAVE_PRESET: {
            Serial.print("sauvegarde demandé");
            byte presetNumber = data[idx++];
            if (presetNumber >= maxPresets) return;

            byte presetType = data[idx++];

            // Décodage du nom (12 caractères max)
            static char tempPresetName[13];
            for (int i = 0; i < 12; i++) {
                tempPresetName[i] = data[idx++];
            }
            tempPresetName[12] = '\0'; // Terminaison garantie

            Preset* targetPreset = &presetStorage[presetNumber];
            targetPreset->presetName = tempPresetName; // Assigne le nom décodé
            if (storage->savePreset(presetNumber, *targetPreset)) {
            Serial.println("✓ Sauvegardé dans flash");
    
            // TEST IMMÉDIAT
            Preset verification;
            if (storage->loadPreset(presetNumber, verification)) {
                Serial.print("TEST - Nom relu immédiatement : '");
                Serial.print(verification.presetName);
                Serial.println("'");
                
                Serial.print("TEST - Attack relu : ");
                Serial.println(verification.sound.synth.attack);
                
                // Comparer avec ce qu'on vient de sauvegarder
                if (strcmp(verification.presetName, targetPreset->presetName) == 0) {
                    Serial.println("✓ VERIFICATION OK - Flash correctement écrite");
                } else {
                    Serial.println("✗ ERREUR - Flash corrompue ou mauvaise lecture");
                }
            }
        }
            switch (presetType) {
                case 0: // SYNTH
                    targetPreset->sound.type = SYNTH;
                    parseSynthParams(data, idx, targetPreset->sound.synth);
                    break;
                case 1: // STRING
                    targetPreset->sound.type = STRING;
                    parseStringParams(data, idx, targetPreset->sound.string);
                    break;
                case 2: // DRUM
                    targetPreset->sound.type = DRUM;
                    parseDrumParams(data, idx, targetPreset->sound.drum);
                    break;
                case 3: // SAMPLE
                    targetPreset->sound.type = SAMPLE;
                    parseSampleParams(data, idx, targetPreset->sound.sample);
                    break;
            }
            
            parseEffects(data, idx, targetPreset->effects);
            if (storage) {
                if (storage->savePreset(presetNumber, *targetPreset)) {
                    Serial.print("✓ Preset ");
                    Serial.print(presetNumber);
                    Serial.println(" sauvegardé dans la flash");
                } else {
                    Serial.println("✗ Erreur sauvegarde flash");
                }
            }
            if (presetNumber < NUM_PRESETS) {
                strncpy(presetNames[presetNumber], targetPreset->presetName, MAX_PRESET_NAME_LEN - 1);
                presetNames[presetNumber][MAX_PRESET_NAME_LEN - 1] = '\0'; // garantit la terminaison

                //presetNames[presetNumber] = String(targetPreset->presetName);
                Serial.print("Updated preset ");
                Serial.print(presetNumber);
                Serial.print(" name to: ");
                Serial.println(presetNames[presetNumber]);
            }
    
            
            Serial.print("Preset sauvegardé dans le slot ");
            Serial.println(presetNumber);
            break;
        }
    }
}

void SysExHandler::sendPresetData(int presetNumber) {
    Serial.println("=== DEBUT sendPresetData ===");
    Serial.flush();
    
    if (presetNumber >= maxPresets) {
        Serial.println("ERREUR: presetNumber >= maxPresets");
        return;
    }
    
    Serial.print("Chargement preset "); Serial.println(presetNumber);
    Serial.flush();
    
    // Charger le preset depuis la flash
    Preset loadedPreset;
    if (!storage->loadPreset(presetNumber, loadedPreset)) {
        Serial.println("ERREUR: Impossible de charger le preset");
        return;
    }
    
    Serial.println("Preset chargé avec succès");
    Serial.print("Nom: '"); Serial.print(loadedPreset.presetName); Serial.println("'");
    Serial.print("Type: "); Serial.println(loadedPreset.sound.type);
    Serial.flush();
    
    byte sysexData[512];
    int idx = 0;
    
    sysexData[idx++] = MANUFACTURER_ID;
    sysexData[idx++] = CMD_PRESET_DATA;
    
    // Type de preset
    sysexData[idx++] = (byte)loadedPreset.sound.type;
    
    Serial.println("Encodage du nom...");
    Serial.flush();
    
    // Nom du preset
    encodeString(sysexData, idx, loadedPreset.presetName, 12);
    
    Serial.println("Encodage des paramètres...");
    Serial.flush();
    
    // Paramètres selon le type
    switch (loadedPreset.sound.type) {
        case SYNTH:
            encodeSynthParams(sysexData, idx, loadedPreset.sound.synth);
            break;
        case STRING:
            encodeStringParams(sysexData, idx, loadedPreset.sound.string);
            break;
        case DRUM:
            encodeDrumParams(sysexData, idx, loadedPreset.sound.drum);
            break;
        case SAMPLE:
            encodeSampleParams(sysexData, idx, loadedPreset.sound.sample);
            break;
    }
    
    Serial.println("Encodage des effets...");
    Serial.flush();
    
    // Effets
    encodeEffects(sysexData, idx, loadedPreset.effects);
    
    Serial.print("Taille totale SysEx: "); Serial.println(idx);
    Serial.flush();
    
    // Envoyer le message SysEx
    usbMIDI.sendSysEx(idx, sysexData);
    
    Serial.print("✓ Preset "); Serial.print(presetNumber); Serial.println(" envoyé");
    Serial.println("=== FIN sendPresetData ===");
    Serial.flush();
}



