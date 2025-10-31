// PresetStorage.cpp
#include "PresetStorage.h"

// Buffers statiques
char PresetStorage::nameBuffer[32][13];
char PresetStorage::fileBuffer[32][13];
int PresetStorage::nameBufferIndex = 0;
int PresetStorage::fileBufferIndex = 0;

PresetStorage::PresetStorage() {
}

bool PresetStorage::begin() {
    if (initialized) return true;
    // Initialiser LittleFS avec 128kb de Flash
    if (!myfs.begin(256*1024)) {
        Serial.println("✗ Échec initialisation LittleFS");
        return false;
    }
    //myfs.lowLevelFormat();
    //delay(2000);
    //myfs.quickFormat();
    // Créer le dossier presets s'il n'existe pas
    // if (!myfs.exists("/presets")) {
    //     myfs.mkdir("/presets");
    // }
    // if (!myfs.exists("/presets")) {
    //     myfs.mkdir("/presets");
    // }
    
    initialized = true;
    Serial.println("✓ myfs initialisé");
    //Serial.println("✓ LittleFS initialisé");
    //Serial.printf("Espace total: %u bytes, utilisé: %u bytes\n", myfs.totalBytes(), myfs.usedBytes());

    // Afficher l'espace disponible
    // FSInfo fs_info;
    // myfs.info(fs_info);
    // Serial.print("  Espace total: ");
    // Serial.print(fs_info.totalBytes / 1024);
    // Serial.println(" KB");
    // Serial.print("  Espace utilisé: ");
    // Serial.print(fs_info.usedBytes / 1024);
    // Serial.println(" KB");
    
    return true;
}


void PresetStorage::printFSInfo() {
    Serial.println("=== Contenu du système de fichiers ===");
    
    File root = myfs.open("/");
    if (!root) {
        Serial.println("✗ Impossible d'ouvrir /");
        return;
    }
    
    uint32_t total_used = 0;
    int file_count = 0;
    
    File entry;
    while (entry = root.openNextFile()) {
        if (!entry.isDirectory()) {
            uint32_t size = entry.size();
            total_used += size;
            file_count++;
            Serial.printf("  %s: %u bytes\n", entry.name(), size);
        }
        entry.close();
    }
    root.close();
    
    Serial.printf("\nTotal: %d fichiers, %u bytes utilisés\n", file_count, total_used);
    Serial.printf("Espace alloué: 1024 KB (%u bytes)\n", 256*1024);
    Serial.printf("Estimation libre: ~%u bytes\n", (256*1024) - total_used);
}

void PresetStorage::convertToFlash(const Preset& preset, PresetFlash& flash) {
    // ✓ CRITICAL : Tout initialiser à zéro AVANT
    //memset(&flash, 0, sizeof(PresetFlash));
    
    flash.type = preset.sound.type;
    
    // Copier presetName (sécurisé)
    if (preset.presetName != nullptr) {
        strncpy(flash.presetName, preset.presetName, 12);
        flash.presetName[12] = '\0';
    }
    
    // Copier soundName (sécurisé)
    if (preset.sound.name != nullptr) {
        strncpy(flash.soundName, preset.sound.name, 12);
        flash.soundName[12] = '\0';
    }
    
    // Copier les paramètres selon le type
    switch(preset.sound.type) {
        case SYNTH:
            memcpy(&flash.sound.synth, &preset.sound.synth, sizeof(SynthParams));
            break;
        case DRUM:
            memcpy(&flash.sound.drum, &preset.sound.drum, sizeof(DrumParams));
            break;
        case STRING:
            memcpy(&flash.sound.string, &preset.sound.string, sizeof(StringParams));
            break;
        case SAMPLE:
            flash.sound.sample.moogAttack = preset.sound.sample.moogAttack;
            flash.sound.sample.moogDecay = preset.sound.sample.moogDecay;
            flash.sound.sample.moogSustain = preset.sound.sample.moogSustain;
            flash.sound.sample.moogRelease = preset.sound.sample.moogRelease;
            break;
    }
    
    // Copier les effets
    memcpy(&flash.effects, &preset.effects, sizeof(EffectsParams));
    
    flash.version = 1;
    
    // ✓ DEBUG : Vérifier que la structure est valide
    Serial.printf("[DEBUG] PresetFlash validé: type=%d, name='%s', version=%u\n",
                  flash.type, flash.presetName, flash.version);
}


void PresetStorage::convertFromFlash(const PresetFlash& flash, Preset& preset, int slot) {
    // Type
    preset.sound.type = flash.type;
    preset.soundTypeIndex = flash.soundTypeIndex;
    // Nom du preset - utiliser directement le slot comme index
    if (slot >= 0 && slot < 32) {
        strncpy(nameBuffer[slot], flash.presetName, 12);
        nameBuffer[slot][12] = '\0';
        preset.presetName = nameBuffer[slot];
    }
    // Nom du preset
    int nameIdx = nameBufferIndex % 32;
    strncpy(nameBuffer[nameIdx], flash.presetName, 13);
    nameBuffer[nameIdx][12] = '\0'; // Terminaison
    preset.presetName = nameBuffer[nameIdx];
    nameBufferIndex++;
    if (nameBufferIndex >= 32) nameBufferIndex = 0; // Réinitialisation
    
    // Le nom du son reste celui d'origine (pas sauvegardé)
    // preset.sound.name reste inchangé
    
    // Paramètres selon le type
    switch (preset.sound.type) {
        case SYNTH:
            preset.sound.synth = flash.sound.synth;
            break;
            
        case DRUM:
            preset.sound.drum = flash.sound.drum;
            break;
            
        case STRING:
            preset.sound.string = flash.sound.string;
            break;
            
        case SAMPLE:
            // Les noms de fichiers restent ceux de la Teensy (pas restaurés)
            // preset.sound.sample.filename[] reste inchangé
            
            // Restaurer seulement les paramètres Moog/LFO
            preset.sound.sample.moogAttack = flash.sound.sample.moogAttack;
            preset.sound.sample.moogDecay = flash.sound.sample.moogDecay;
            preset.sound.sample.moogSustain = flash.sound.sample.moogSustain;
            preset.sound.sample.moogRelease = flash.sound.sample.moogRelease;
            break;
    }
    
    // Effets (toujours restaurés)
    preset.effects = flash.effects;
}

bool PresetStorage::saveAllPresets(const Preset* presets, int count) {
    File file = myfs.open("/presets.dat", FILE_WRITE);
    if (!file) return false;
    
    for (int i = 0; i < count; i++) {
        PresetFlash flash;
        convertToFlash(presets[i], flash);
        
        size_t written = file.write((uint8_t*)&flash, sizeof(PresetFlash));
        if (written != sizeof(PresetFlash)) {
            file.close();
            return false;
        }
    }
    
    file.close();
    return true;
}

bool PresetStorage::loadPreset(int slot, Preset& preset) {
    //Serial.println("=== DEBUT loadPreset ===");
    Serial.flush(); // Important !
    
    File file = myfs.open("/presets.dat", FILE_READ);
    Serial.println("Fichier ouvert");
    Serial.flush();
    
    if (!file) {
        Serial.println("ERREUR: Fichier non ouvert");
        return false;
    }
    
    size_t fileSize = file.size();
    size_t expectedPos = slot * sizeof(PresetFlash);
    
    Serial.print("Position: "); Serial.println(expectedPos);
    Serial.flush();
    
    if (expectedPos + sizeof(PresetFlash) > fileSize) {
        Serial.println("ERREUR: Slot hors limites");
        file.close();
        return false;
    }
    
    //Serial.println("Avant seek");
    Serial.flush();
    
    if (!file.seek(expectedPos)) {
        Serial.println("ERREUR: Seek échoué");
        file.close();
        return false;
    }
    
    //Serial.println("Après seek");
    Serial.flush();
    
    PresetFlash flash;
    //memset(&flash, 0, sizeof(PresetFlash)); // IMPORTANT
    
    //Serial.println("Avant read");
    Serial.flush();
    
    size_t read = file.read((uint8_t*)&flash, sizeof(PresetFlash));
    
    //Serial.print("Après read: "); Serial.println(read);
    Serial.flush();
    
    file.close();
    
    Serial.println("Fichier fermé");
    Serial.flush();
    
    if (read != sizeof(PresetFlash)) {
        Serial.println("ERREUR: Lecture incomplète");
        return false;
    }
    
    //Serial.println("Avant convertFromFlash");
    Serial.flush();
    
    convertFromFlash(flash, preset, slot);
    
    //Serial.println("=== FIN loadPreset ===");
    Serial.flush();
    
    return true;
}

bool PresetStorage::savePreset(int slot, const Preset& preset) {
    Serial.println("\n=== PresetStorage::savePreset ===");
    Serial.print("Slot: "); Serial.println(slot);
    
    if (slot < 0 || slot >= 32) {
        Serial.println("✗ Slot invalide");
        return false;
    }
    
    // Ouvrir LE fichier unique
    File file = myfs.open("/presets.dat", FILE_WRITE);
    if (!file) {
        Serial.println("✗ Impossible d'ouvrir /presets.dat");
        return false;
    }
    
    // Aller à la position du preset dans le fichier
    size_t position = slot * sizeof(PresetFlash);
    if (!file.seek(position)) {
        Serial.println("✗ Seek échoué");
        file.close();
        return false;
    }
    
    PresetFlash flash;
    convertToFlash(preset, flash);
    
    Serial.print("Ecriture à position "); Serial.print(position);
    Serial.print(" dans /presets.dat");
    
    size_t written = file.write((uint8_t*)&flash, sizeof(PresetFlash));
    file.close();
    
    if (written != sizeof(PresetFlash)) {
        Serial.print("✗ Ecriture incomplète: ");
        Serial.print(written);
        Serial.print("/");
        Serial.println(sizeof(PresetFlash));
        return false;
    }
    
    Serial.println(" ✓");
    Serial.println("=================================\n");
    return true;
}

bool PresetStorage::presetExists(int slot) {
    if (!initialized) return false;
    if (slot < 0 || slot >= 32) return false;
    
    File file = myfs.open("/presets.dat", FILE_READ);
    if (!file) {
        return false;
    }
    
    // Vérifier que le fichier est assez grand pour contenir ce slot
    size_t expectedSize = (slot + 1) * sizeof(PresetFlash);
    size_t fileSize = file.size();
    file.close();
    
    if (fileSize < expectedSize) {
        return false;
    }
    
    // Optionnel : vérifier que le preset n'est pas vide
    // En chargeant et vérifiant le nom ou un champ version
    Preset temp;
    if (!loadPreset(slot, temp)) {
        return false;
    }
    
    // Considérer qu'un preset existe si son nom n'est pas vide
    return (temp.presetName != nullptr && strlen(temp.presetName) > 0);
}

bool PresetStorage::deletePreset(int slot) {
    if (!initialized) return false;
    if (slot < 0 || slot >= 32) return false;
    
    // Pour "supprimer" un preset dans un fichier unique,
    // on écrit un preset vide à sa position
    File file = myfs.open("/presets.dat", FILE_WRITE);
    if (!file) {
        Serial.println("✗ Impossible d'ouvrir /presets.dat pour suppression");
        return false;
    }
    
    size_t position = slot * sizeof(PresetFlash);
    if (!file.seek(position)) {
        Serial.println("✗ Seek échoué");
        file.close();
        return false;
    }
    
    // Créer un preset vide (tous les bytes à 0)
    PresetFlash emptyFlash;
    memset(&emptyFlash, 0, sizeof(PresetFlash));
    
    size_t written = file.write((uint8_t*)&emptyFlash, sizeof(PresetFlash));
    file.close();
    
    if (written == sizeof(PresetFlash)) {
        Serial.print("✓ Preset ");
        Serial.print(slot);
        Serial.println(" supprimé");
        return true;
    }
    
    return false;
}

void PresetStorage::listPresets() {
    if (!initialized) return;
    
    Serial.println("\n=== Presets sauvegardés ===");
    
    File file = myfs.open("/presets.dat", FILE_READ);
    if (!file) {
        Serial.println("✗ Aucun fichier presets.dat");
        return;
    }
    
    size_t fileSize = file.size();
    Serial.print("Taille fichier: "); Serial.print(fileSize); Serial.println(" bytes");
    Serial.print("Capacité: "); Serial.print(fileSize / sizeof(PresetFlash)); Serial.println(" presets");
    Serial.println();
    
    int count = 0;
    for (int i = 0; i < MAX_PRESETS; i++) {
        size_t position = i * sizeof(PresetFlash);
        
        // Vérifier qu'on ne dépasse pas la taille du fichier
        if (position + sizeof(PresetFlash) > fileSize) {
            break;
        }
        
        if (!file.seek(position)) {
            continue;
        }
        
        PresetFlash flash;
        size_t read = file.read((uint8_t*)&flash, sizeof(PresetFlash));
        
        if (read == sizeof(PresetFlash) && flash.presetName[0] != '\0') {
            Serial.print("  [");
            if (i < 10) Serial.print(" ");
            Serial.print(i);
            Serial.print("] ");
            Serial.print(flash.presetName);
            Serial.print(" (type: ");
            Serial.print(flash.type);
            Serial.print(", version: ");
            Serial.print(flash.version);
            Serial.println(")");
            count++;
        }
    }
    
    file.close();
    
    Serial.println();
    Serial.print("Total: ");
    Serial.print(count);
    Serial.print(" / ");
    Serial.print(MAX_PRESETS);
    Serial.println(" presets");
    Serial.println("===========================\n");
}

int PresetStorage::getPresetCount() {
    if (!initialized) return 0;
    
    File file = myfs.open("/presets.dat", FILE_READ);
    if (!file) {
        return 0;
    }
    
    size_t fileSize = file.size();
    int maxSlots = fileSize / sizeof(PresetFlash);
    int count = 0;
    
    for (int i = 0; i < min(maxSlots, MAX_PRESETS); i++) {
        size_t position = i * sizeof(PresetFlash);
        
        if (!file.seek(position)) {
            continue;
        }
        
        PresetFlash flash;
        size_t read = file.read((uint8_t*)&flash, sizeof(PresetFlash));
        
        // Compter si le preset a un nom non vide
        if (read == sizeof(PresetFlash) && flash.presetName[0] != '\0') {
            count++;
        }
    }
    
    file.close();
    return count;
}

bool PresetStorage::format() {
    if (!initialized) return false;
    
    Serial.println("⚠️  Formatage du système de fichiers...");
    
    // Supprimer le fichier presets.dat
    if (myfs.exists("/presets.dat")) {
        if (myfs.remove("/presets.dat")) {
            Serial.println("✓ Fichier /presets.dat supprimé");
        } else {
            Serial.println("✗ Erreur suppression /presets.dat");
        }
    }
    
    // Format complet du filesystem (optionnel)
    myfs.quickFormat();
    
    Serial.println("✓ Formatage terminé");
    return true;
}

// // Exemple d'utilisation dans SoundManager.cpp
// /*
// #include "PresetStorage.h"

// class SoundManager {
// private:
//     PresetStorage storage;
//     Preset presetBank[NUM_PRESETS];
//     Preset currentPreset;
    
// public:
//     void begin() {
//         // Initialiser le stockage Flash
//         if (!storage.begin()) {
//             Serial.println("✗ Échec initialisation stockage");
//             return;
//         }
        
//         // Charger les presets depuis la Flash
//         Serial.println("Chargement des presets...");
//         for (int i = 0; i < NUM_PRESETS; i++) {
//             if (storage.presetExists(i)) {
//                 if (storage.loadPreset(i, presetBank[i])) {
//                     Serial.print("  Preset ");
//                     Serial.print(i);
//                     Serial.print(": ");
//                     Serial.println(presetBank[i].presetName);
//                 }
//             } else {
//                 // Utiliser le preset par défaut
//                 presetBank[i] = DEFAULT_PRESETS[i];
//             }
//         }
        
//         // Lister les presets disponibles
//         storage.listPresets();
//     }
    
//     void handleSysExSave(int slot, const Preset& preset) {
//         // Sauvegarder dans le tableau RAM
//         if (slot < NUM_PRESETS) {
//             presetBank[slot] = preset;
//         }
        
//         // Sauvegarder dans la Flash
//         if (storage.savePreset(slot, preset)) {
//             Serial.println("✓ Preset sauvegardé en Flash");
//         } else {
//             Serial.println("✗ Échec sauvegarde Flash");
//         }
//     }
// };