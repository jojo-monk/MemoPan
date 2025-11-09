#ifndef SAVEEPROM_H
#define SAVEEPROM_H

#include "Debug.h"
#include <EEPROM.h>

class SaveEprom {
private:
    uint8_t numDonnees = 15;
    
    // Structure des données stockées
    // variables = {0:Highscore, 1:son, 2:gamme, 3:note, 4:clavier, 5:vitesse, 
    // 6:lumiere, 7:volume, 8:reverbLevel, 9:tremoloLevel, 10:chorusLevel, 11:arpEnabled, 12:arpMode,
    // 13:arpSpeedBPM, 14:arpSpeed_id} }
    struct DonneesEEPROM {
        uint32_t magic;
        uint8_t variables[15]; // Fixé à 8 pour correspondre à numDonnees
        uint8_t checksum;
    };
    
    static const uint32_t MAGIC_NUMBER = 0xEE12345B;
    static const int ADRESSE_BASE = 0;
    
    uint8_t calculerChecksum(const uint8_t vars[15]) {
        uint8_t sum = 0;
        for (int i = 0; i < numDonnees; i++) {
            sum += vars[i];
        }
        return sum;
    }

public:
    SaveEprom() {}
    
    // Sauvegarder toutes les variables
    bool sauvegarder(const uint8_t variables[15]) {
        DonneesEEPROM donnees;
        donnees.magic = MAGIC_NUMBER;
        
        for (int i = 0; i < numDonnees; i++) {
            donnees.variables[i] = variables[i];
        }
        
        donnees.checksum = calculerChecksum(variables);
        EEPROM.put(ADRESSE_BASE, donnees);
        return true;
    }
    
    // Sauvegarder une variable spécifique
    bool sauvegarderVariable(uint8_t index, uint8_t valeur) {
        if (index >= numDonnees) return false;
        
        DonneesEEPROM donnees;
        EEPROM.get(ADRESSE_BASE, donnees);
        
        if (donnees.magic != MAGIC_NUMBER) {
            donnees.magic = MAGIC_NUMBER;
            for (int i = 0; i < numDonnees; i++) {
                donnees.variables[i] = 0;
            }
        }
        
        donnees.variables[index] = valeur;
        donnees.checksum = calculerChecksum(donnees.variables);
        EEPROM.put(ADRESSE_BASE, donnees);
        return true;
    }
    
    // CORRIGÉ : Charger une variable spécifique
    bool chargerVariable(uint8_t index, uint8_t& valeur) {
        if (index >= numDonnees) return false;
        
        DonneesEEPROM donnees;
        EEPROM.get(ADRESSE_BASE, donnees);
        
        if (donnees.magic != MAGIC_NUMBER) {
            return false;
        }
        
        uint8_t checksumCalcule = calculerChecksum(donnees.variables);
        if (checksumCalcule != donnees.checksum) {
            return false;
        }
        
        valeur = donnees.variables[index];
        return true;
    }
    
    // Charger toutes les variables
    bool charger(uint8_t variables[15]) {
        DonneesEEPROM donnees;
        EEPROM.get(ADRESSE_BASE, donnees);
        
        if (donnees.magic != MAGIC_NUMBER) {
            return false;
        }
        
        uint8_t checksumCalcule = calculerChecksum(donnees.variables);
        if (checksumCalcule != donnees.checksum) {
            return false;
        }
        
        for (int i = 0; i < numDonnees; i++) {
            variables[i] = donnees.variables[i];
        }
        return true;
    }
    
    // AJOUTÉ : Obtenir une variable spécifique (version simple)
    uint8_t obtenirVariable(uint8_t index) {
        if (index >= numDonnees) return 0;
        
        DonneesEEPROM donnees;
        EEPROM.get(ADRESSE_BASE, donnees);
        
        if (donnees.magic != MAGIC_NUMBER) {
            return 0;
        }
        
        uint8_t checksumCalcule = calculerChecksum(donnees.variables);
        if (checksumCalcule != donnees.checksum) {
            return 0;
        }
        
        return donnees.variables[index];
    }
    
    bool initialiser(const uint8_t valeursDefaut[15]) {
        return sauvegarder(valeursDefaut);
    }
    
    bool donneesValides() {
        DonneesEEPROM donnees;
        EEPROM.get(ADRESSE_BASE, donnees);
        
        if (donnees.magic != MAGIC_NUMBER) {
            return false;
        }
        
        uint8_t checksumCalcule = calculerChecksum(donnees.variables);
        return (checksumCalcule == donnees.checksum);
    }
    
    void effacer() {
        DonneesEEPROM donnees;
        donnees.magic = 0;
        for (int i = 0; i < numDonnees; i++) {
            donnees.variables[i] = 0;
        }
        donnees.checksum = 0;
        EEPROM.put(ADRESSE_BASE, donnees);
    }
    
    static int tailleUtilisee() {
        return sizeof(DonneesEEPROM);
    }

    uint8_t getHighScore() {
      return obtenirVariable(0);
    }

    void setHighScore(uint8_t score) {
      sauvegarderVariable(0, score);
    }

    uint8_t getSon() {
      return obtenirVariable(1);
    }

    void setSon(uint8_t son) {
      sauvegarderVariable(1, son);
    }

    uint8_t getGamme() {
      return obtenirVariable(2);
    }

    void setGamme(uint8_t gamme) {
      sauvegarderVariable(2, gamme);
    }

    uint8_t getNote() {
      return obtenirVariable(3);
    }

    void setNote(uint8_t note) {
      sauvegarderVariable(3, note);
    }

    uint8_t getClavier() {
      return obtenirVariable(4);
    }

    void setClavier(uint8_t clavier) {
      sauvegarderVariable(4, clavier);
    }

    uint8_t getVitesse() {
      return obtenirVariable(5);
    }

    void setVitesse(uint8_t vitesse) {
      sauvegarderVariable(5, vitesse);
    }

    uint8_t getLumiere() {
      return obtenirVariable(6);
    }

    void setLumiere(uint8_t lumiere) {
      sauvegarderVariable(6, lumiere);
    }

    uint8_t getVolume() {
      return obtenirVariable(7);
    }

    void setVolume(uint8_t volume) {
      sauvegarderVariable(7, volume);
    }

    uint8_t getRverbLevel() {
      return obtenirVariable(8);
    }

    void setReverbLevel(uint8_t reverbMix) {
      sauvegarderVariable(8, reverbMix);
    }

    uint8_t getTremoloLevel() {
      return obtenirVariable(9);
    }

    void setTremoloLevel(uint8_t tremo) {
      sauvegarderVariable(9, tremo);
    }

    uint8_t getChorusLevel() {
      return obtenirVariable(10);
    }

    void setChorusLevel(uint8_t chor) {
      sauvegarderVariable(10, chor);
    }

    uint8_t getArpEnabled() {
      return obtenirVariable(11);
    }

    void setArpEnabled(uint8_t arpEnabled) {
      sauvegarderVariable(11, arpEnabled);
    }

    uint8_t getArpMode() {
      return obtenirVariable(12);
    }

    void setArpMode(uint8_t arpMode) {
      sauvegarderVariable(12, arpMode);
    }

    uint8_t getArpSpeedBPM() {
      return obtenirVariable(13);
    }

    void setArpSpeedBPM(uint8_t arpSpeedBPM) {
      sauvegarderVariable(13, arpSpeedBPM);
    }

    uint8_t getArpSpeedId() {
      return obtenirVariable(14);
    }

    void setArpSpeedId(uint8_t arpSpeed_id) {
      sauvegarderVariable(14, arpSpeed_id);
    }

  



    
    void debug() {
        DonneesEEPROM donnees;
        EEPROM.get(ADRESSE_BASE, donnees);
        
        DEBUG_INFO_EEPROM("=== Debug EEPROM ===");
        DEBUG_VAR_EEPROM("Magic: 0x", String(donnees.magic, HEX));
        //Serial.println(String(donnees.magic, HEX));
        DEBUG_INFO_EEPROM("Variables: ");
        for (int i = 0; i < numDonnees; i++) {
            DEBUG_INFO_EEPROM(String(donnees.variables[i]));
            if (i < (numDonnees-1)) DEBUG_INFO_EEPROM(", ");
        }
        DEBUG_INFO_EEPROM("");
        DEBUG_VAR_EEPROM("Checksum stocké: ", String(donnees.checksum));
        //Serial.println(String(donnees.checksum));
        DEBUG_VAR_EEPROM("Checksum calculé: ", String(calculerChecksum(donnees.variables)));
        //Serial.println(String(calculerChecksum(donnees.variables)));
        DEBUG_INFO_EEPROM("Données valides: ");
        DEBUG_INFO_EEPROM(donneesValides() ? "OUI" : "NON");
        DEBUG_VAR_EEPROM("Taille utilisée: ", String(tailleUtilisee()));
        DEBUG_INFO_EEPROM(" bytes");
        DEBUG_INFO_EEPROM("==================");
    }
};


#endif