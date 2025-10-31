// BatteryMonitor.h
#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>

class BatteryMonitor {
private:
  // Configuration pins
  uint8_t _adcPin;
  uint8_t _shutdownPin;
  
  // Configuration pont diviseur
  float _dividerFactor;  // (R1 + R2) / R2
  
  // Seuils de tension
  float _shutdownVoltage;
  float _lowVoltage;
  float _criticalVoltage;
  
  // État
  float _currentVoltage;
  float _currentPercentage;
  unsigned long _lastUpdate;
  uint16_t _updateInterval;  // ms
  
  // Courbe de décharge LiPo (tension, pourcentage)
  static const uint8_t _curvePoints = 27;
  const float _dischargeCurve[_curvePoints][2] = {
    {4.20, 100}, {4.10, 96}, {4.00, 92}, {3.96, 89},
    {3.92, 85}, {3.89, 81}, {3.86, 77}, {3.83, 73},
    {3.80, 69}, {3.77, 65}, {3.75, 62}, {3.72, 58}, 
    {3.70, 55}, {3.66, 51}, {3.62, 47}, {3.58, 43},
    {3.55, 40}, {3.51, 35}, {3.48, 32}, {3.44, 26},
    {3.40, 24}, {3.37, 20}, {3.35, 17}, {3.27, 13},
    {3.20, 9}, {3.10, 6}, {3.00, 3}
  };

  
  // Callbacks
  typedef void (*VoltageCallback)(float voltage, float percentage);
  VoltageCallback _onLowBattery;
  VoltageCallback _onCriticalBattery;
  VoltageCallback _onShutdown;
  
  // Interpolation linéaire sur la courbe
  float interpolatePercentage(float voltage) {
    // Hors limites
    if (voltage >= _dischargeCurve[0][0]) return 100.0;
    if (voltage <= _dischargeCurve[_curvePoints - 1][0]) return 0.0;
    
    // Recherche de l'intervalle
    for (uint8_t i = 0; i < _curvePoints - 1; i++) {
      float v1 = _dischargeCurve[i][0];
      float v2 = _dischargeCurve[i + 1][0];
      
      if (voltage >= v2 && voltage <= v1) {
        float p1 = _dischargeCurve[i][1];
        float p2 = _dischargeCurve[i + 1][1];
        
        // Interpolation linéaire
        return p2 + (voltage - v2) * (p1 - p2) / (v1 - v2);
      }
    }
    
    return 0.0;
  }

public:
  // Constructeur
  BatteryMonitor(uint8_t adcPin, 
                 uint8_t shutdownPin = 255,
                 float r1 = 51.0, 
                 float r2 = 100.0) 
    : _adcPin(adcPin),
      _shutdownPin(shutdownPin),
      _dividerFactor((r1 + r2) / r2),
      _shutdownVoltage(3.30),
      _lowVoltage(3.55),
      _criticalVoltage(3.44),
      _currentVoltage(0),
      _currentPercentage(0),
      _lastUpdate(0),
      _updateInterval(2000),
      _onLowBattery(nullptr),
      _onCriticalBattery(nullptr),
      _onShutdown(nullptr) {
  }
  
  // Initialisation
  void begin() {
    pinMode(_adcPin, INPUT);
    analogReadResolution(10); // 0-1023
    
    if (_shutdownPin != 255) {
      pinMode(_shutdownPin, OUTPUT);
      digitalWrite(_shutdownPin, HIGH); // Alimentation ON
    }
    
    // Première lecture
    update();
  }
  
  // Mise à jour de la lecture
  void update() {
    _lastUpdate = millis();
    
    // Lecture ADC (moyenne de 5 mesures pour stabilité)
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 5; i++) {
      sum += analogRead(_adcPin);
      delayMicroseconds(100);
    }
    uint16_t adcValue = sum / 5;
    
    // Conversion en tension
    float vADC = adcValue * 3.3 / 1023.0;
    _currentVoltage = vADC * _dividerFactor;
    
    // Calcul du pourcentage
    _currentPercentage = interpolatePercentage(_currentVoltage);
    
    // Vérification des seuils
    checkThresholds();
  }
  
  // Appel automatique dans loop()
  void loop() {
    if (millis() - _lastUpdate >= _updateInterval) {
      update();
    }
  }
  
  // Vérification des seuils critiques
  void checkThresholds() {
    // Shutdown critique
    if (_currentVoltage <= _shutdownVoltage) {
      if (_onShutdown) {
        _onShutdown(_currentVoltage, _currentPercentage);
      }
      shutdown();
    }
    // Critique
    else if (_currentVoltage <= _criticalVoltage) {
      if (_onCriticalBattery) {
        _onCriticalBattery(_currentVoltage, _currentPercentage);
      }
    }
    // Faible
    else if (_currentVoltage <= _lowVoltage) {
      if (_onLowBattery) {
        _onLowBattery(_currentVoltage, _currentPercentage);
      }
    }
  }
  
  // Coupure d'alimentation
  void shutdown() {
    if (_shutdownPin != 255) {
      Serial.println("!!! BATTERY CRITICAL - SHUTTING DOWN !!!");
      delay(100);
      digitalWrite(_shutdownPin, LOW); // Coupure MOSFET
      while(1); // Ne devrait jamais être atteint
    } else {
      // Mode deep sleep si pas de pin shutdown
      Serial.println("!!! BATTERY CRITICAL - DEEP SLEEP !!!");
      // Mode basse consommation Teensy 4
      //set_arm_power_mode(0);
      // while(1) {
      //   delay(10000);
      // }
    }
  }
  
  // Getters
  float getVoltage() const { return _currentVoltage; }
  float getPercentage() const { return _currentPercentage; }
  uint16_t getADCRaw() const { return analogRead(_adcPin); }
  
  bool isLow() const { return _currentVoltage <= _lowVoltage; }
  bool isCritical() const { return _currentVoltage <= _criticalVoltage; }
  
  // Estimation de l'autonomie restante (en minutes)
  uint16_t getEstimatedRuntime(float currentConsumption_mA, uint16_t capacity_mAh = 2200) const {
    float remainingCapacity = capacity_mAh * (_currentPercentage / 100.0);
    return (uint16_t)(remainingCapacity / currentConsumption_mA * 60);
  }
  
  // Setters
  void setShutdownVoltage(float voltage) { _shutdownVoltage = voltage; }
  void setLowVoltage(float voltage) { _lowVoltage = voltage; }
  void setCriticalVoltage(float voltage) { _criticalVoltage = voltage; }
  void setUpdateInterval(uint16_t ms) { _updateInterval = ms; }
  
  // Callbacks
  void onLowBattery(VoltageCallback callback) { _onLowBattery = callback; }
  void onCriticalBattery(VoltageCallback callback) { _onCriticalBattery = callback; }
  void onShutdown(VoltageCallback callback) { _onShutdown = callback; }
  
  // Affichage debug
  void printStatus() const {
    Serial.print("Battery: ");
    Serial.print(_currentVoltage, 2);
    Serial.print("V (");
    Serial.print(_currentPercentage, 1);
    Serial.print("%) - ");
    
    if (isCritical()) {
      Serial.println("CRITICAL!");
    } else if (isLow()) {
      Serial.println("LOW");
    } else {
      Serial.println("OK");
    }
  }
  
  // Retourne une icône de batterie
  const char* getBatteryIcon() const {
    if (_currentPercentage > 80) return "🔋";
    if (_currentPercentage > 50) return "🔋";
    if (_currentPercentage > 20) return "🪫";
    return "🔴";
  }
};

#endif // BATTERY_MONITOR_H


// ========== EXEMPLE D'UTILISATION ==========

/*
// Dans ton sketch principal (.ino)

#include "BatteryMonitor.h"

// Créer l'objet batterie
// BatteryMonitor(pin ADC, pin shutdown, R1, R2)
BatteryMonitor battery(A0, 2, 51.0, 100.0);

// Callbacks optionnels
void onLowBattery(float voltage, float percentage) {
  Serial.println("⚠️ Batterie faible!");
  // Activer mode économie d'énergie
}

void onCriticalBattery(float voltage, float percentage) {
  Serial.println("🔴 BATTERIE CRITIQUE!");
  // Éteindre LEDs, sauvegarder données
}

void onShutdown(float voltage, float percentage) {
  Serial.println("💀 Arrêt imminent...");
  // Dernière sauvegarde
}

void setup() {
  Serial.begin(115200);
  
  // Initialiser la batterie
  battery.begin();
  
  // Configurer les seuils (optionnel)
  battery.setLowVoltage(3.70);        // Alerte à 20%
  battery.setCriticalVoltage(3.50);   // Critique à 5%
  battery.setShutdownVoltage(3.35);   // Coupure à 0%
  
  // Configurer les callbacks
  battery.onLowBattery(onLowBattery);
  battery.onCriticalBattery(onCriticalBattery);
  battery.onShutdown(onShutdown);
  
  // Intervalle de mise à jour (1 seconde par défaut)
  battery.setUpdateInterval(2000); // 2 secondes
}

void loop() {
  // Mise à jour automatique
  battery.loop();
  
  // Afficher le statut
  battery.printStatus();
  
  // Récupérer les valeurs
  float voltage = battery.getVoltage();
  float percentage = battery.getPercentage();
  
  // Estimation autonomie (exemple avec 250mA de consommation)
  uint16_t minutes = battery.getEstimatedRuntime(250);
  Serial.print("Autonomie restante: ");
  Serial.print(minutes);
  Serial.println(" min");
  
  delay(5000);
}
*/