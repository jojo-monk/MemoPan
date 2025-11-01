<p align="center">
  <img src="gaston-handpan.jpg" alt="Mascotte MemoPan" width="200"/>
</p>

# 🪘 MemoPan

**MemoPan** est un instrument hybride **handpan / jeu Simon électronique**, basé sur un **Teensy 4.0**.  
Ce projet combine un **jeu de mémoire musicale** et un **handpan électronique polyphonique**, avec synthèse FM, effets et échantillons percussifs.

---

## 🎛️ Fonctions principales

### 🧠 Mode Simon (jeu de mémoire)
- Jusqu’à **10 joueurs**
- **4 niveaux de difficulté** :
  - 🟢 *Facile* → 5 notes  
  - 🟡 *Moyen* → 7 notes  
  - 🔴 *Difficile* → 12 notes  
  - ⚫ *Extrême* → 12 notes sans aides visuelles  
- **Vitesses ajustables**
- Feedback lumineux via **12 LED WS2812**

### 🎵 Mode Handpan
- **12 zones tactiles** sensibles à la pression (MPR121)
- **Polyphonie complète**
- **32 presets** de sons
- **Synthèse FM à 2 oscillateurs**
- **Effets intégrés** : Tremolo, Chorus, Reverb
- **Mode Sample** : lecture de 12 échantillons de percussions

---

## ⚙️ Matériel utilisé

| Composant | Description |
|------------|-------------|
| 🧠 **Teensy 4.0** | Microcontrôleur principal |
| 🎧 **Audio Shield** | Convertisseur audio stéréo |
| 🖥️ **Écran OLED 128×128** | Interface utilisateur |
| 👆 **MPR121** | Détection capacitive 12 touches |
| 💡 **12 LEDs WS2812** | Indicateurs visuels et feedback |
| 🔋 **Module TP4056** | Gestion de charge Li-Po |
| 🔋 **Batterie Li-Po 2200 mAh** | Alimentation |
| 🎚️ **Potentiomètre avec switch** | Réglage du volume |
| 🔄 **Encodeur rotatif** | Navigation dans les menus |

---

## 💻 Développement

- **Environnement** : [PlatformIO](https://platformio.org/)  
- **Langage** : C++  
- **Carte cible** : `Teensy 4.0`  

### Installation
1. Cloner le dépôt :
   ```bash
   git clone https://github.com/jojo-monk/MemoPan.git
   cd MemoPan
