#ifndef MIDI_MANAGERV2_H
#define MIDI_MANAGERV2_H

#include <Arduino.h>
//#include <usbMIDI.h>
#include <functional>
#include "Debug.h"

class MidiManager {
private:
    inline static byte sysexBuffer[512];
    inline static int sysexBufferIndex = 0;
    inline static bool sysexInProgress = false;

public:
    inline static uint8_t midiChannel = 1;
    inline static bool enabled = false;

    inline static std::function<void(uint8_t, uint8_t)> onControlChange = nullptr;
    inline static std::function<void(uint8_t, uint8_t)> onNoteOn = nullptr;
    inline static std::function<void(uint8_t, uint8_t)> onNoteOff = nullptr;
    inline static std::function<void(byte*, unsigned int)> onSysEx = nullptr;

    static void updateConnection() {
#ifdef usb_configuration
        bool shouldBeEnabled = (usb_configuration != 0);
#else
        bool shouldBeEnabled = true;
#endif
        if (shouldBeEnabled != enabled) {
            enabled = shouldBeEnabled;
            DEBUG_INFO_MIDI(enabled ? "MIDI activé" : "MIDI désactivé");
        }
    }

    static void setChannel(uint8_t ch) {
        if (ch >= 1 && ch <= 16) midiChannel = ch;
    }

    static uint8_t getChannel() { return midiChannel; }
    static void setEnabled(bool state) { enabled = state; }
    static bool getEnabled() { return enabled; }

    static void noteOn(uint8_t note, uint8_t vel) { if (enabled) usbMIDI.sendNoteOn(note, vel, midiChannel); }
    static void noteOff(uint8_t note, uint8_t vel = 0) { if (enabled) usbMIDI.sendNoteOff(note, vel, midiChannel); }
    static void allNotesOff(uint8_t ch) { if (enabled) usbMIDI.sendControlChange(123, 0, ch); }
    static void controlChange(uint8_t cc, uint8_t val) { if (enabled) usbMIDI.sendControlChange(cc, val, midiChannel); }

    static void sendAllCC(uint8_t* params, uint8_t len) {
        if (!enabled) return;
        for (uint8_t i = 0; i < len; i++) {
            controlChange(i, params[i]);
            delay(1);
        }
    }

    static void sendSysEx(byte* data, unsigned int len) {
        if (enabled) usbMIDI.sendSysEx(len, data);
    }

    static void read() {
        if (!enabled) return;
        while (usbMIDI.read()) {
            byte type = usbMIDI.getType();
            if (type == usbMIDI.SystemExclusive) {
                const byte* sysexData = usbMIDI.getSysExArray();
                unsigned int sysexLength = usbMIDI.getSysExArrayLength();

                if (sysexLength == 0 || sysexBufferIndex + sysexLength > sizeof(sysexBuffer)) {
                    sysexBufferIndex = 0;
                    sysexInProgress = false;
                    continue;
                }

                memcpy(sysexBuffer + sysexBufferIndex, sysexData, sysexLength);
                sysexBufferIndex += sysexLength;

                if (sysexData[sysexLength - 1] == 0xF7) {
                    if (onSysEx) onSysEx(sysexBuffer, sysexBufferIndex);
                    sysexBufferIndex = 0;
                    sysexInProgress = false;
                } else {
                    sysexInProgress = true;
                }
                continue;
            }

            switch (type) {
                case usbMIDI.NoteOn:
                    if (onNoteOn) onNoteOn(usbMIDI.getData1(), usbMIDI.getData2());
                    break;
                case usbMIDI.NoteOff:
                    if (onNoteOff) onNoteOff(usbMIDI.getData1(), usbMIDI.getData2());
                    break;
                case usbMIDI.ControlChange:
                    if (onControlChange) onControlChange(usbMIDI.getData1(), usbMIDI.getData2());
                    break;
            }
        }
    }
};

// byte MidiManager::sysexBuffer[512];
// int MidiManager::sysexBufferIndex = 0;
// bool MidiManager::sysexInProgress = false;

#endif // MIDI_MANAGERV2_H
