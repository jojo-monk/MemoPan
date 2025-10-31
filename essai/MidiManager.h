#if 0
#ifndef MIDI_MANAGER_H
#define MIDI_MANAGER_H

#include <Arduino.h>
#include "Debug.h"

class MidiManager {
public:
    static uint8_t midiChannel;
    static bool enabled;
    
    // Callbacks
    static void (*onControlChange)(uint8_t cc, uint8_t value);
    static void (*onNoteOn)(uint8_t note, uint8_t velocity);
    static void (*onNoteOff)(uint8_t note, uint8_t velocity);
    static void (*onSysEx)(byte* data, unsigned int length);  // NOUVEAU
    
    static void updateConnection() {
        bool shouldBeEnabled = (usb_configuration != 0);
        if (shouldBeEnabled != enabled) {
            enabled = shouldBeEnabled;
            DEBUG_INFO_MIDI("MIDI: ");
            DEBUG_INFO_MIDI(enabled ? "Activé" : "Désactivé");
        }
    }
    
    static void setChannel(uint8_t channel) {
        if (channel >= 1 && channel <= 16) {
            midiChannel = channel;
        }
    }
    
    static uint8_t getChannel() {
        return midiChannel;
    }
    
    static void setEnabled(bool state) {
        enabled = state;
    }
    
    static bool getEnabled() {
        return enabled;
    }
    
    // === ENVOI ===
    
    static void noteOn(uint8_t note, uint8_t velocity) {
        if (enabled) {
            usbMIDI.sendNoteOn(note, velocity, midiChannel);
        }
    }
    
    static void noteOff(uint8_t note, uint8_t velocity = 0) {
        if (enabled) {
            usbMIDI.sendNoteOff(note, velocity, midiChannel);
        }
    }
    
    static void allNotesOff(uint8_t channel) {
        if (enabled) {
            usbMIDI.sendControlChange(123, 0, channel);
        }
    }
    
    static void controlChange(uint8_t control, uint8_t value) {
        if (enabled) {
            usbMIDI.sendControlChange(control, value, midiChannel);
        }
    }
    
    static void sendAllCC(uint8_t* params, uint8_t length) {
        if (!enabled) return;
        for (uint8_t i = 0; i < length; i++) {
            controlChange(i, params[i]);
            delay(1);
        }
    }
    
    // NOUVEAU : Envoyer SysEx
    static void sendSysEx(byte* data, unsigned int length) {
        if (enabled) {
            usbMIDI.sendSysEx(length, data);
        }
    }
    
    // === RÉCEPTION ===
    
    static void read() {
        if (!enabled) return;
        
        while (usbMIDI.read()) {
            byte type = usbMIDI.getType();
            byte channel = usbMIDI.getChannel();
            
            // SysEx : pas de canal, toujours traité
            if (type == usbMIDI.SystemExclusive) {
                if (onSysEx) {
                    // Récupérer les données SysEx
                    const byte* sysexData = usbMIDI.getSysExArray();
                    unsigned int sysexLength = usbMIDI.getSysExArrayLength();
                    
                    // Copier dans un buffer car getSysExArray() retourne un pointeur temporaire
                    static byte sysexBuffer[512];
                    if (sysexLength <= 512) {
                        memcpy(sysexBuffer, sysexData, sysexLength);
                        onSysEx(sysexBuffer, sysexLength);
                    } else {
                        DEBUG_VAR_MIDI("SysEx trop long: ", sysexLength);
                    }
                }
                continue;
            }
            
            // Autres messages : vérifier le canal
            if (channel != midiChannel) continue;
            
            switch (type) {
                case usbMIDI.ControlChange: {
                    uint8_t cc = usbMIDI.getData1();
                    uint8_t value = usbMIDI.getData2();
                    
                    if (onControlChange) {
                        onControlChange(cc, value);
                    }
                    break;
                }
                
                case usbMIDI.NoteOn: {
                    uint8_t note = usbMIDI.getData1();
                    uint8_t velocity = usbMIDI.getData2();
                    
                    if (onNoteOn) {
                        onNoteOn(note, velocity);
                    }
                    break;
                }
                
                case usbMIDI.NoteOff: {
                    uint8_t note = usbMIDI.getData1();
                    uint8_t velocity = usbMIDI.getData2();
                    
                    if (onNoteOff) {
                        onNoteOff(note, velocity);
                    }
                    break;
                }
            }
        }
    }
};

// Définitions des variables statiques
uint8_t MidiManager::midiChannel = 1;
bool MidiManager::enabled = false;
void (*MidiManager::onControlChange)(uint8_t, uint8_t) = nullptr;
void (*MidiManager::onNoteOn)(uint8_t, uint8_t) = nullptr;
void (*MidiManager::onNoteOff)(uint8_t, uint8_t) = nullptr;
void (*MidiManager::onSysEx)(byte*, unsigned int) = nullptr;  // NOUVEAU

#endif // MIDI_MANAGER_H
#endif