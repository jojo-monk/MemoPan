#ifndef SOUNDMANAGERV3_H
#define SOUNDMANAGERV3_H

#include "Debug.h"
#include "config.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "AudioSampleClosedhh.h"
#include "AudioSampleClosedhh40msw.h"
#include "AudioSampleCowbell.h"
#include "AudioSampleCowbell100msw.h"
#include "AudioSampleCowbell40msw.h"
#include "AudioSampleKick.h"
#include "AudioSampleKick40msw.h"
#include "AudioSampleOpenhh.h"
#include "AudioSampleOpenhh100msw.h"
#include "AudioSampleOpenhh40msw.h"
#include "AudioSampleLowcongaw.h"
#include "AudioSampleLowconga100msw.h"
#include "AudioSampleLowconga40msw.h"
#include "AudioSampleRimshot.h"
#include "AudioSampleRimshot40msw.h"
#include "AudioSampleSnare.h"
#include "AudioSampleSnare40msw.h"
#include "AudioSampleTabla1.h"
#include "AudioSampleTabla1100msw.h"
#include "AudioSampleTabla140msw.h"
#include "AudioSampleTabla2.h"
#include "AudioSampleTabla2100msw.h"
#include "AudioSampleTabla240msw.h"
#include "AudioSampleTabla3.h"
#include "AudioSampleTabla3100msw.h"
#include "AudioSampleTabla340msw.h"
#include "AudioSampleTabla4.h"
#include "AudioSampleTabla4100msw.h"
#include "AudioSampleTabla440msw.h"
#include "AudioSampleTabla5.h"
#include "AudioSampleTabla5100msw.h"
#include "AudioSampleTabla540msw.h"
#include "MidiManager.h"
#include "MapToMidi.h"


#define CHORUS_DELAY_LENGTH (32*AUDIO_BLOCK_SAMPLES)
//#define FLANGE_DELAY_LENGTH (2*AUDIO_BLOCK_SAMPLES)

MidiManager midiManager;


class SoundManager {
private:
  // Objects audio


  AudioOutputI2S                i2s1;
  AudioControlSGTL5000          sgtl5000_1;
  //AudioAmplifier                amp1;
  AudioAmplifier                polyAmp;
  AudioAmplifier                amp2;
  //AudioEffectFreeverb           monoReverb;


  // Poly : un oscillateur + enveloppe par note
  AudioPlayMemory             polySample[NUM_TOUCH_PADS];
  AudioSynthWaveformModulated polyWave[NUM_TOUCH_PADS];  // 12 oscillateurs poly
  AudioEffectEnvelope         polyEnv[NUM_TOUCH_PADS]; // polyWave -> polyEnv -> mixer
  AudioSynthWaveformSine      polyModFM[NUM_TOUCH_PADS];
  AudioSynthSimpleDrum        polyDrum[NUM_TOUCH_PADS];
  AudioSynthKarplusStrong     polyString[NUM_TOUCH_PADS];
  AudioEffectFreeverb         polyReverb;
  AudioFilterStateVariable    filtre;
  AudioFilterStateVariable    lpFiltre;
  AudioEffectEnvelope         lpfEnv;
  AudioSynthWaveformDc        cvEnv;
  AudioSynthWaveform          lfo;
  AudioEffectMultiply         tremolo;
  AudioEffectChorus           chorus;
  //AudioEffectFlange           flanger;

  AudioMixer4 sampleMixer[4];
  AudioMixer4 fxMixer;
  AudioMixer4 soundMixer[9];
  AudioMixer4 polyMixer1;
  AudioMixer4 polyMixer2;
  AudioMixer4 polyMixer3;
  AudioMixer4 polyMixerGlobal;

  //AudioMixer4 monoStringDrum;
  AudioMixer4 dryWetMix;

  AudioConnection          sample1ToSampleMixer1;
  AudioConnection          sample2ToSampleMixer1;
  AudioConnection          sample3ToSampleMixer1;
  AudioConnection          sample4ToSampleMixer1;
  AudioConnection          sample5ToSampleMixer2;
  AudioConnection          sample6ToSampleMixer2;
  AudioConnection          sample7ToSampleMixer2;
  AudioConnection          sample8ToSampleMixer2;
  AudioConnection          sample9ToSampleMixer3;
  AudioConnection          sample10ToSampleMixer3;
  AudioConnection          sample11ToSampleMixer3;
  AudioConnection          sample12ToSampleMixer3;
  AudioConnection          sampleMixer1ToSampleMixer4;
  AudioConnection          sampleMixer2ToSampleMixer4;
  AudioConnection          sampleMixer3ToSampleMixer4;
  AudioConnection          sampleMixer4TofxMixer;

  AudioConnection          polyPatch1;
  AudioConnection          polyPatch2;
  AudioConnection          polyPatch3;
  AudioConnection          polyPatch4;
  AudioConnection          polyPatch5;
  AudioConnection          polyPatch6;
  AudioConnection          polyPatch7;
  AudioConnection          polyPatch8;
  AudioConnection          polyPatch9;
  AudioConnection          polyPatch10;
  AudioConnection          polyPatch11;
  AudioConnection          polyPatch12;

  AudioConnection          polyPatch13;
  AudioConnection          polyPatch14;
  AudioConnection          polyPatch15;
  AudioConnection          polyPatch16;
  AudioConnection          polyPatch17;
  AudioConnection          polyPatch18;
  AudioConnection          polyPatch19;
  AudioConnection          polyPatch20;
  AudioConnection          polyPatch21;
  AudioConnection          polyPatch22;
  AudioConnection          polyPatch23;
  AudioConnection          polyPatch24;
  

  
  AudioConnection          string1ToSoundMixer1;
  AudioConnection          string2ToSoundMixer1;
  AudioConnection          string3ToSoundMixer1;
  AudioConnection          string4ToSoundMixer1;
  AudioConnection          string5ToSoundMixer2;
  AudioConnection          string6ToSoundMixer2;
  AudioConnection          string7ToSoundMixer2;
  AudioConnection          string8ToSoundMixer2;
  AudioConnection          string9ToSoundMixer3;
  AudioConnection          string10ToSoundMixer3;
  AudioConnection          string11ToSoundMixer3;
  AudioConnection          string12ToSoundMixer3;
  AudioConnection          drum1ToSoundMixer4;
  AudioConnection          drum2ToSoundMixer4;  
  AudioConnection          drum3ToSoundMixer4;
  AudioConnection          drum4ToSoundMixer4;
  AudioConnection          drum5ToSoundMixer5;
  AudioConnection          drum6ToSoundMixer5;
  AudioConnection          drum7ToSoundMixer5;
  AudioConnection          drum8ToSoundMixer5;
  AudioConnection          drum9ToSoundMixer6;
  AudioConnection          drum10ToSoundMixer6;
  AudioConnection          drum11ToSoundMixer6;
  AudioConnection          drum12ToSoundMixer6;
  AudioConnection          soundMixerToMix1;
  AudioConnection          soundMixerToMix2;
  AudioConnection          soundMixerToMix3;
  AudioConnection          soundMixerToMix4;
  AudioConnection          soundMixerToMix5;
  AudioConnection          soundMixerToMix6;
  AudioConnection          mixToMix1;
  AudioConnection          mixToMix2;
  AudioConnection          mixToAmp2;
  AudioConnection          amp2ToGlobal;
  AudioConnection          polyMixer1Patch1;
  AudioConnection          polyMixer1Patch2;
  AudioConnection          polyMixer1Patch3;
  AudioConnection          polyMixer1Patch4;
  AudioConnection          polyMixer2Patch1;
  AudioConnection          polyMixer2Patch2;
  AudioConnection          polyMixer2Patch3;
  AudioConnection          polyMixer2Patch4;
  AudioConnection          polyMixer3Patch1;
  AudioConnection          polyMixer3Patch2;
  AudioConnection          polyMixer3Patch3;
  AudioConnection          polyMixer3Patch4;
  AudioConnection          polyMixerGlobalPatch1;
  AudioConnection          polyMixerGlobalPatch2;
  AudioConnection          polyMixerGlobalPatch3;
  //AudioConnection          polyMixerGlobalToF;
  AudioConnection          cvToEnv;
  AudioConnection          envTolpf;
  AudioConnection          lpfToF;
  AudioConnection          lpfTofxMixer;
  AudioConnection          FToReverb;
  AudioConnection          lfoToTremolo;
  AudioConnection          tremoloToFxMixer;
  AudioConnection          polyMixerGlobalToTremolo;
  AudioConnection          polyReverbTodryWet;
  AudioConnection          polyMixerGlobalToChorus;
  AudioConnection          chorusToFxMixer;
  AudioConnection          lpfTodrywet;
  AudioConnection          polyMixerGlobalToFxMixer;
  AudioConnection          fxMixerTolpf;
  AudioConnection          fxMixerToF;

  AudioConnection          dryWetToAmp;
  //AudioConnection          polyAmpTofinal;
  AudioConnection          finalToI2S_L0;
  AudioConnection          finalToI2S_R0;
  // AudioConnection          finalToI2S_L1;
  // AudioConnection          finalToI2S_R1;
  
 
  int currentSoundMode;
  bool isPlaying;
  unsigned long soundStartTime;
  float scaleFreq[NUM_TOUCH_PADS];
  bool polyMode = false;               // mode poly activable/désactivable
  float polyMixerGain = 0.5;
  float stringLevel = 1;
  float drumLevel = 2.5;
  float sampleLevel = 0.7;
  int keyboardMode = 0;
  int randomMap[NUM_TOUCH_PADS];
  bool drumMode = false;
  bool stringMode = false;
  bool arpegioMode = false;
  float maxTime = 200.0f; // durée max en ms pour vélocité
  float maxPressure = 550.0f; // pression max pour vélocité
  uint16_t sampleDuration = 500;
  

  // Number of samples in each delay line

  // Allocate the delay lines for left and right channels
  short delayline[CHORUS_DELAY_LENGTH];

  // Catégorie de son
  enum SoundCategory {
    SYNTH,
    DRUM,
    STRING,
    SAMPLE
  };

  // Paramètres pour chaque type de son
  struct SynthParams {
    int waveformType;
    float amplitude;
    int attack, decay;
    float sustain;
    int release;
    float freqMod;
    float phaseMod;
    float ratioFm;
    float modAmplitude;
    int synthFx_id;
  };

  struct DrumParams {
    int length;
    float secondMix;
    float pitchMod;
    int drumFx_id;
  };

  struct StringParams {
    float velocity;
    int stringFx_id;
  };

  struct SampleParams {
    const char* filename[NUM_TOUCH_PADS];
    int stringFx_id;
  };

  // Structure principale
  struct SoundMode {
    SoundCategory type;
    const char* name;
    union {
      SynthParams synth;
      StringParams string;
      DrumParams drum;
      SampleParams sample;
    };
  };


  // Paramètres pour chaque mode sonore
  // struct SoundMode {
  //   int waveformType;
  //   float amplitude;
  //   int attack;
  //   int decay;
  //   float sustain;
  //   int release;
  //   const char* name;
  //   int modFM;
  // };
  
  //static const SoundMode soundModes[NUM_SOUND_MODES];
  static constexpr float MONO_GAIN = 0.5f;
  static constexpr float POLY_GAIN = 0.5f;
  static constexpr float AMP2_GAIN = 1.0f;
  float getNote(int padIndex);
  void stopMidi();
  void playMidi(int touchIndex, float frquency, float velocity);
  // Définition des 6 sons (4 synths, 1 drum, 1 string)
  // synth = {waveFormType, amplitude, attack, decay, sustain, release, freqMod, phaseMod, ratioFm, modAmplitude}
  // amplitude = float 0-1, 
  // attack = int en ms
  // decay = int en ms
  // sustain = float entre 0 et 1
  // release = int en ms
  // freqMod = float, nb d'octaves sur laquelle s'applique la modulation
  // phaseMode = float modulation de phase en degrés
  // ratioFm = float multiplicateur de la fréquence de la modulante
  // modAmplitude = float amplitude de la modulante
const SoundMode soundModes[NUM_SOUND_MODES] = {
  { SYNTH, "Sine",    { .synth = { WAVEFORM_SINE,     0.6, 10, 50, 0.7, 1000, 0.6, 90, 0.75, 0.2, 0 } } },
  { SYNTH, "Saw",     { .synth = { WAVEFORM_SAWTOOTH, 0.4, 100, 30, 0.5, 400, 2, 120, 0.51, 0.314, 1 } } },
  { SYNTH, "Square",  { .synth = { WAVEFORM_SQUARE,   0.5, 20, 50, 0.1, 200, 1.5, 10, 0.5, 0.25, 2 } } },
  { SYNTH, "HandPan", { .synth = { WAVEFORM_TRIANGLE, 0.6, 30, 1800, 0.3, 1000, 0.1, 5, 1.3, 0.1, 3 } } },
  //{ SYNTH, "MemoPan1", { .synth = { WAVEFORM_TRIANGLE, 0.6, 30, 1800, 0.3, 1000, 0.1, 5, 1.3, 0.1, 4 } } },
  //{ SYNTH, "MemoPan2", { .synth = { WAVEFORM_TRIANGLE, 0.6, 30, 1800, 0.3, 1000, 0.1, 5, 1.3, 0.1, 5 } } },
  //{ SYNTH, "MemoPan3", { .synth = { WAVEFORM_TRIANGLE, 0.6, 30, 1800, 0.3, 1000, 0.1, 5, 1.3, 0.1, 6 } } },
  //{ SYNTH, "MemoPan4", { .synth = { WAVEFORM_TRIANGLE, 0.6, 30, 1800, 0.3, 1000, 0.1, 5, 1.3, 0.1, 7 } } },
  //{ SYNTH, "MemoPan5", { .synth = { WAVEFORM_TRIANGLE, 0.6, 30, 1800, 0.3, 1000, 0.1, 5, 1.3, 0.1, 8 } } },
  //{ SYNTH, "MemoPan6", { .synth = { WAVEFORM_TRIANGLE, 0.6, 30, 1800, 0.3, 1000, 0.1, 5, 1.3, 0.1, 9 } } },
  { STRING,"String",  { .string= { 0.8, 10 } } },
  { DRUM,  "Drum",    { .drum  = { 50, 0.8, 0.55, 11 } } },
  //{ DRUM,  "Drum1",    { .drum  = { 50, 0.8, 0.55, 12 } } },
  //{ DRUM,  "Drum2",    { .drum  = { 50, 0.8, 0.55, 13 } } },
  //{ DRUM,  "Drum3",    { .drum  = { 50, 0.8, 0.55, 14 } } },
  { SAMPLE, "Sample",  { .sample= { {"kick", "snare", "openHH", "closedHH", "cowbell", "lowConga", "rimShot", "tabla1", "tabla2", "tabla3", "tabla4", "tabla5", 15 } } } }
};


struct EffectsParams {
    // Reverb
    float reverbRoomSize;   // 0.0 – 1.0
    float reverbDamping;    // 0.0 – 1.0

    // Filter
    float filterFreq;       // Hz
    float filterResonance;  // 0.0 – 10.0

    // Chorus
    int nChorus;            // nombre de voix

    // Tremolo
    int tremoloWaveform; // type d'onde
    float tremoloRate;      // Hz
    float tremoloAmplitude; // 0.0 – 1.0
};

const EffectsParams defaultEffects = {
    0.5,    // reverbRoomSize
    0.7,    // reverbDamping
    10000,  // filterFreq
    0.5,    // filterResonance
    8,      // nChorus
    0,      // tremoloWaveform
    5.0,    // tremoloRate
    0.5     // tremoloAmplitude
};
EffectsParams effects[NUM_SOUND_MODES];

  // Définition des samples de batterie
  // Chaque tableau doit être déclaré dans un fichier séparé
const unsigned int* drumSamplesLong[NUM_TOUCH_PADS] = {
  AudioSampleKick,
  AudioSampleSnare,
  AudioSampleOpenhh,
  AudioSampleClosedhh,
  AudioSampleCowbell,
  AudioSampleLowcongaw,
  AudioSampleRimshot,
  AudioSampleTabla1,
  AudioSampleTabla2,
  AudioSampleTabla3,
  AudioSampleTabla4,
  AudioSampleTabla5
};

const unsigned int* drumSamples100ms[NUM_TOUCH_PADS] = {
  AudioSampleKick40msw,
  AudioSampleSnare40msw,
  AudioSampleOpenhh100msw,
  AudioSampleClosedhh40msw,
  AudioSampleCowbell100msw,
  AudioSampleLowconga100msw,
  AudioSampleRimshot40msw,
  AudioSampleTabla1100msw,
  AudioSampleTabla2100msw,
  AudioSampleTabla3100msw,
  AudioSampleTabla4100msw,
  AudioSampleTabla5100msw
};

const unsigned int* drumSamples40ms[NUM_TOUCH_PADS] = {
  AudioSampleKick40msw,
  AudioSampleSnare40msw,
  AudioSampleOpenhh40msw,
  AudioSampleClosedhh40msw,
  AudioSampleCowbell40msw,
  AudioSampleLowconga40msw,
  AudioSampleRimshot40msw,
  AudioSampleTabla140msw,
  AudioSampleTabla240msw,
  AudioSampleTabla340msw,
  AudioSampleTabla440msw,
  AudioSampleTabla540msw
};

public:
  SoundManager();
  bool begin();
  void setSoundMode(int mode);
  void playTouchSound(int touchIndex, uint16_t pression, uint8_t velocite);
  void stopOneNote(int touchIndex);
  void update();
  void volume(uint8_t vol);
  void buildScale(int noteIndex, int gammeIndex);
  void setReverb(float size, float damp);
  void setDryWetMix(uint8_t mix);
  void setTremoloLevel(uint8_t tremoloLevel);
  void setChorusLevel(uint8_t chorusLevel);
  void handleCC(int cc, int value);
  int getSoundMode() const { 
    return currentSoundMode; 
  }
  const char* getSoundModeName() const;
  void setKeyboardMode(int mode) { 
    keyboardMode = mode; 
  }
  void generateRandomMap();
  void setPolyMode(bool);
  void testPoly();
  void updateMidiNotes();
  void unMute();
  void stopAllSounds();
  void setPolyEnv(int attack, int decay, float sustain, int release);
  void setArpeggio(bool enabled);
  void soundDuration(uint16_t durationMs);
  void getAllCC(uint8_t cc, uint8_t value);
  void applyEffects(const EffectsParams &fx);
  
};
uint8_t activeMidiNotes[NUM_TOUCH_PADS] = { 0 };
// paramètres pour la reverbe
//float roomsize = 0.5;
//float damping = 0.7;
// fx paramètres
//float lfoFreq = 5;
//float lfoAmplitude = 0.5;
//int s_idx = 2*FLANGE_DELAY_LENGTH/4;
//int s_depth = FLANGE_DELAY_LENGTH/4;
//double s_freq = 3;
//float chorusVoice = 8;

// ===== IMPLÉMENTATION =====

SoundManager::SoundManager() :
  sample1ToSampleMixer1(polySample[0], 0, sampleMixer[0], 0),
  sample2ToSampleMixer1(polySample[1], 0, sampleMixer[0], 1),
  sample3ToSampleMixer1(polySample[2], 0, sampleMixer[0], 2),
  sample4ToSampleMixer1(polySample[3], 0, sampleMixer[0], 3),
  sample5ToSampleMixer2(polySample[4], 0, sampleMixer[1], 0),
  sample6ToSampleMixer2(polySample[5], 0, sampleMixer[1], 1),
  sample7ToSampleMixer2(polySample[6], 0, sampleMixer[1], 2),
  sample8ToSampleMixer2(polySample[7], 0, sampleMixer[1], 3),
  sample9ToSampleMixer3(polySample[8], 0, sampleMixer[2], 0),
  sample10ToSampleMixer3(polySample[9], 0, sampleMixer[2], 1),
  sample11ToSampleMixer3(polySample[10], 0, sampleMixer[2], 2),
  sample12ToSampleMixer3(polySample[11], 0, sampleMixer[2], 3),
  sampleMixer1ToSampleMixer4(sampleMixer[0], 0, sampleMixer[3], 0),
  sampleMixer2ToSampleMixer4(sampleMixer[1], 0, sampleMixer[3], 1),
  sampleMixer3ToSampleMixer4(sampleMixer[2], 0, sampleMixer[3], 2),
  sampleMixer4TofxMixer(sampleMixer[3], 0, fxMixer, 3),
  polyPatch1(polyWave[0], polyEnv[0]),
  polyPatch2(polyWave[1], polyEnv[1]),
  polyPatch3(polyWave[2], polyEnv[2]),
  polyPatch4(polyWave[3], polyEnv[3]),
  polyPatch5(polyWave[4], polyEnv[4]),
  polyPatch6(polyWave[5], polyEnv[5]),
  polyPatch7(polyWave[6], polyEnv[6]),
  polyPatch8(polyWave[7], polyEnv[7]),
  polyPatch9(polyWave[8], polyEnv[8]),
  polyPatch10(polyWave[9], polyEnv[9]),
  polyPatch11(polyWave[10], polyEnv[10]),
  polyPatch12(polyWave[11], polyEnv[11]),

  polyPatch13(polyModFM[0], 0, polyWave[0], 0),
  polyPatch14(polyModFM[1], 0, polyWave[1], 0),
  polyPatch15(polyModFM[2], 0, polyWave[2], 0),
  polyPatch16(polyModFM[3], 0, polyWave[3], 0),
  polyPatch17(polyModFM[4], 0, polyWave[4], 0),
  polyPatch18(polyModFM[5], 0, polyWave[5], 0),
  polyPatch19(polyModFM[6], 0, polyWave[6], 0),
  polyPatch20(polyModFM[7], 0, polyWave[7], 0),
  polyPatch21(polyModFM[8], 0, polyWave[8], 0),
  polyPatch22(polyModFM[9], 0, polyWave[9], 0),
  polyPatch23(polyModFM[10], 0, polyWave[10], 0),
  polyPatch24(polyModFM[11], 0, polyWave[11], 0),

  string1ToSoundMixer1(polyString[0], 0, soundMixer[0], 0),
  string2ToSoundMixer1(polyString[1], 0, soundMixer[0], 1),
  string3ToSoundMixer1(polyString[2], 0, soundMixer[0], 2),
  string4ToSoundMixer1(polyString[3], 0, soundMixer[0], 3),
  string5ToSoundMixer2(polyString[4], 0, soundMixer[1], 0),
  string6ToSoundMixer2(polyString[5], 0, soundMixer[1], 1),
  string7ToSoundMixer2(polyString[6], 0, soundMixer[1], 2),
  string8ToSoundMixer2(polyString[7], 0, soundMixer[1], 3),
  string9ToSoundMixer3(polyString[8], 0, soundMixer[2], 0),
  string10ToSoundMixer3(polyString[9], 0, soundMixer[2], 1),
  string11ToSoundMixer3(polyString[10], 0, soundMixer[2], 2),
  string12ToSoundMixer3(polyString[11], 0, soundMixer[2], 3),

  drum1ToSoundMixer4(polyDrum[0], 0, soundMixer[3], 0),
  drum2ToSoundMixer4(polyDrum[1], 0, soundMixer[3], 1),
  drum3ToSoundMixer4(polyDrum[2], 0, soundMixer[3], 2),
  drum4ToSoundMixer4(polyDrum[3], 0, soundMixer[3], 3),
  drum5ToSoundMixer5(polyDrum[4], 0, soundMixer[4], 0),
  drum6ToSoundMixer5(polyDrum[5], 0, soundMixer[4], 1),
  drum7ToSoundMixer5(polyDrum[6], 0, soundMixer[4], 2),
  drum8ToSoundMixer5(polyDrum[7], 0, soundMixer[4], 3),
  drum9ToSoundMixer6(polyDrum[8], 0, soundMixer[5], 0),
  drum10ToSoundMixer6(polyDrum[9], 0, soundMixer[5], 1),
  drum11ToSoundMixer6(polyDrum[10], 0, soundMixer[5], 2),
  drum12ToSoundMixer6(polyDrum[11], 0, soundMixer[5], 3),

  soundMixerToMix1(soundMixer[0], 0, soundMixer[6], 0),
  soundMixerToMix2(soundMixer[1], 0, soundMixer[6], 1),
  soundMixerToMix3(soundMixer[2], 0, soundMixer[6], 2),
  soundMixerToMix4(soundMixer[3], 0, soundMixer[7], 0),
  soundMixerToMix5(soundMixer[4], 0, soundMixer[7], 1),
  soundMixerToMix6(soundMixer[5], 0, soundMixer[7], 2),

  mixToMix1(soundMixer[6], 0, soundMixer[8], 0),
  mixToMix2(soundMixer[7], 0, soundMixer[8], 1),

  mixToAmp2(soundMixer[8], 0, amp2, 0),
  amp2ToGlobal(amp2, 0, polyMixerGlobal, 3),

  polyMixer1Patch1(polyEnv[0], 0, polyMixer1, 0),
  polyMixer1Patch2(polyEnv[1], 0, polyMixer1, 1),
  polyMixer1Patch3(polyEnv[2], 0, polyMixer1, 2),
  polyMixer1Patch4(polyEnv[3], 0, polyMixer1, 3),

  polyMixer2Patch1(polyEnv[4], 0, polyMixer2, 0),
  polyMixer2Patch2(polyEnv[5], 0, polyMixer2, 1),
  polyMixer2Patch3(polyEnv[6], 0, polyMixer2, 2),
  polyMixer2Patch4(polyEnv[7], 0, polyMixer2, 3),

  polyMixer3Patch1(polyEnv[8], 0, polyMixer3, 0),
  polyMixer3Patch2(polyEnv[9], 0, polyMixer3, 1),
  polyMixer3Patch3(polyEnv[10], 0, polyMixer3, 2),
  polyMixer3Patch4(polyEnv[11], 0, polyMixer3, 3),

  polyMixerGlobalPatch1(polyMixer1, 0, polyMixerGlobal, 0),
  polyMixerGlobalPatch2(polyMixer2, 0, polyMixerGlobal, 1),
  polyMixerGlobalPatch3(polyMixer3, 0, polyMixerGlobal, 2),
  cvToEnv(cvEnv, 0, lpfEnv, 0),
  envTolpf(lpfEnv, 0, lpFiltre, 1),
  FToReverb(filtre, 0, polyReverb, 0),
  lfoToTremolo(lfo, 0, tremolo, 1),
  tremoloToFxMixer(tremolo, 0, fxMixer, 1),
  polyMixerGlobalToTremolo(polyMixerGlobal, 0, tremolo, 0),
  polyReverbTodryWet(polyReverb, 0, dryWetMix, 0),
  polyMixerGlobalToChorus(polyMixerGlobal, 0, chorus, 0),
  chorusToFxMixer(chorus, 0, fxMixer, 2),
  lpfTodrywet(lpFiltre, 0, dryWetMix, 1),
  polyMixerGlobalToFxMixer(polyMixerGlobal, 0, fxMixer, 0),
  fxMixerToF(fxMixer, 0, filtre, 0),
  fxMixerTolpf(fxMixer, 0, lpFiltre, 0),
  dryWetToAmp(dryWetMix, 0, polyAmp, 0),
  lpfToF(lpFiltre, 0, filtre, 0),

  finalToI2S_L0(polyAmp, 0, i2s1, 0),
  finalToI2S_R0(polyAmp, 0, i2s1, 1)

  

  {
    currentSoundMode = 0;
    isPlaying = false;
    soundStartTime = 0;
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      polyWave[i].begin(WAVEFORM_SINE);
      polyWave[i].amplitude(0.6);  // pas de son au départ

      polyEnv[i].attack(30);
      polyEnv[i].decay(1800);
      polyEnv[i].sustain(0.6);
      polyEnv[i].release(800);

    }

  }

bool SoundManager::begin() {
  DEBUG_INFO_SOUND("Initializing Audio Shield...");
  delay(300);
  AudioMemory(50);
  delay(100);
  //sgtl5000_1.dacVolume(0.0);
  if (!sgtl5000_1.enable()) {
    DEBUG_INFO_SOUND("Audio Shield not found!");
    // Continuez quand même, parfois ça marche malgré le message
  }
  delay(20);
  
  //delay(1000);
  sgtl5000_1.dacVolumeRamp();  // Rampe DAC d'abord
  //sgtl5000_1.volume(0.0);           // Volume à 0 pendant l'init
  sgtl5000_1.muteHeadphone();       // Muter pendant l'init
  sgtl5000_1.muteLineout();         // Muter pendant l'init
  delay(1000);
  // Allocation mémoire audio (plus importante pour Teensy 4.1)
  
  
  //2. Initialiser tous les objets audio à amplitude 0
  for(int i = 0; i < NUM_TOUCH_PADS; i++) {
    polyWave[i].begin(WAVEFORM_TRIANGLE);
    polyWave[i].amplitude(0.0);
    polyWave[i].frequency(440);
    polyModFM[i].amplitude(0.0);
    polyEnv[i].noteOff();
  }
    
  polyAmp.gain(0.0);  // Amp principal à 0

  // Rampe volume principal progressivement
  for(float vol = 0.0; vol <= 0.6; vol += 0.02) {
    sgtl5000_1.volume(vol);
    delay(20);
  }

  // PUIS démuter
  
  delay(300);

  sgtl5000_1.volume(0.6);
  sgtl5000_1.unmuteHeadphone();
  sgtl5000_1.unmuteLineout();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);
  sgtl5000_1.lineOutLevel(10);
  //sgtl5000_1.unmuteHeadphone();
  //sgtl5000_1.unmuteLineout();
  //sgtl5000_1.dacVolumeRamp();
  
  // 5. Rampe douce finale
  delay(300);
  for(float gain = 0.0; gain <= 0.8; gain += 0.02) {
    polyAmp.gain(gain);
    delay(5);
  }

  //sgtl5000_1.dacVolume(1);
  delay(300);

  for (int i = 0; i < 3; i++) {
    sampleMixer[i].gain(0, sampleLevel);
    sampleMixer[i].gain(1, sampleLevel);
    sampleMixer[i].gain(2, sampleLevel);
    sampleMixer[i].gain(3, sampleLevel);
  }
  sampleMixer[3].gain(0, sampleLevel);
  sampleMixer[3].gain(1, sampleLevel);
  sampleMixer[3].gain(2, sampleLevel);
  sampleMixer[3].gain(3, 0);

  for (int i = 0; i < 4; i++) {
    polyMixer1.gain(i, polyMixerGain);
    polyMixer2.gain(i, polyMixerGain);
    polyMixer3.gain(i, polyMixerGain);
  }

  for (int mx = 0; mx < 6; mx++) {
    for (int ch = 0; ch < 4; ch++) {
      soundMixer[mx].gain(ch, polyMixerGain);
    }
  }
  for (int i = 0; i < 3; i++) {
    soundMixer[6].gain(i, stringLevel);
    soundMixer[7].gain(i, drumLevel);
  }
  soundMixer[6].gain(3, 0);
  soundMixer[7].gain(3, 0);
  soundMixer[8].gain(0, stringLevel);
  soundMixer[8].gain(1, drumLevel);
  

  polyMixerGlobal.gain(0, 0.7);
  polyMixerGlobal.gain(1, 0.7);
  polyMixerGlobal.gain(2, 0.7);
  polyMixerGlobal.gain(3, 0.9);
  amp2.gain(AMP2_GAIN);

  polyReverb.roomsize(effects.reverbRoomSize);
  polyReverb.damping(effects.reverbDamping);
  fxMixer.gain(0, 0.7);
  fxMixer.gain(1, 0.2);
  fxMixer.gain(2, 0);
  fxMixer.gain(3, 0.5);
  dryWetMix.gain(0, 0.3);
  dryWetMix.gain(1, 0.7);
  filtre.frequency(effects.filterFreq);
  filtre.resonance(effects.filterResonance);

  chorus.begin(delayline,CHORUS_DELAY_LENGTH,effects.nChorus);
  //flanger.begin(delayline, FLANGE_DELAY_LENGTH, s_idx, s_depth, s_freq);
  chorus.voices(0);
  lfo.begin(0);
  lfo.frequency(effects.tremoloRate);
  lfo.amplitude(0);


  
  // // Configuration initiale des waveforms
  setPolyMode(true);
  for (int i = 0; i < NUM_SOUND_MODES) {
    effects[i] = defaultEffects;
  }
  //setSoundMode(0);
  //sgtl5000_1.unmuteHeadphone();
  //sgtl5000_1.unmuteLineout();

  DEBUG_INFO_SOUND("Audio Shield initialized!");
  return true;
}

void SoundManager::unMute() {
  sgtl5000_1.unmuteHeadphone();
  sgtl5000_1.unmuteLineout();
}

void SoundManager::applyEffects(const EffectsParams &fx) {
  polyReverb.roomsize(fx.reverbRoomSize);
  polyReverb.damping(fx.reverbDamping);
  filtre.frequency(fx.filterFreq);
  filtre.resonance(fx.filterResonance);
  chorus.voices(fx.nChorus);
  tremolo.begin(fx.tremoloWaveform);
  tremolo.frequency(fx.tremoloRate);
  tremolo.amplitude(fx.tremoloAmplitude);
}

void SoundManager::setPolyMode(bool enabled) {
    polyMode = enabled;
    if (polyMode) {
        //env.noteOff();
        //delay(10);
        //DEBUG_INFO_SOUND("Polyphonic mode ON");
        for (int i = 0; i < 3;i++) {
          polyMixerGlobal.gain(i, POLY_GAIN);
        }
        
        //finalMixer.gain(1, POLY_GAIN);
    } else {
        // for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        //   polyEnv[i].noteOff();
        // } 
        //delay(10);
        for (int i = 0; i < 4;i++) {
          polyMixerGlobal.gain(i, MONO_GAIN);
        }
      //polyenv[0].attack(10);
      // polyenv[0].decay(50);
      // polyenv[0].sustain(0.7);
      // polyenv[0].release(200);
        
        //DEBUG_INFO_SOUND("Polyphonic mode OFF");
        // Couper toutes les notes si on désactive
        
      }

    
}
void SoundManager::setChorusLevel(uint8_t chorusLevel) {
  float chorusLevelFloat;
  chorusLevelFloat = (float)chorusLevel / 10.0f;
  float dry = 1 - chorusLevelFloat;
  fxMixer.gain(0, dry);
  fxMixer.gain(2, chorusLevelFloat);
}

void SoundManager::setTremoloLevel(uint8_t tremoloLevel) {
  float tremoloLevelFloat;
  tremoloLevelFloat = (float)tremoloLevel / 10.0f;
  fxMixer.gain(1, tremoloLevelFloat);
}

void SoundManager::setReverb(float size, float damp) {
  polyReverb.roomsize(size);
  polyReverb.damping(damp);
}

void SoundManager::setDryWetMix(uint8_t mix) {
  float wetFloat;
  wetFloat = (float)mix / 10.0f;
  float dry = 1 - wetFloat;
  dryWetMix.gain(0, wetFloat);
  dryWetMix.gain(1, dry);
}

void SoundManager::setSoundMode(int mode) {
  if (mode < 0 || mode >= NUM_SOUND_MODES) return;
  currentSoundMode = mode;
  const SoundMode &sm = soundModes[currentSoundMode];
  //DEBUG_VAR_SOUND("Sound mode set to: ", sm.name);
  switch (sm.type) {
    case SYNTH:
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        polyWave[i].begin(sm.synth.waveformType);
        polyWave[i].amplitude(sm.synth.amplitude);
        polyWave[i].frequencyModulation(sm.synth.freqMod);
        polyWave[i].phaseModulation(sm.synth.phaseMod);
        polyModFM[i].amplitude(sm.synth.modAmplitude);
        polyEnv[i].attack(sm.synth.attack);
        polyEnv[i].decay(sm.synth.decay);
        polyEnv[i].sustain(sm.synth.sustain);
        polyEnv[i].release(sm.synth.release);

      }
      break;

    case DRUM:
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        polyDrum[i].length(sm.drum.length);
        polyDrum[i].secondMix(sm.drum.secondMix);
        polyDrum[i].pitchMod(sm.drum.pitchMod);
      }
      break;

    case STRING:
      //monoString.noteOn(60, m.string.velocity); // ex: C4
      break;
    case SAMPLE:

      break;
  }
}


// void SoundManager::setSoundMode(int mode) {
//   if (mode >= 0 && mode < 4) {
//     currentSoundMode = mode;
    
//     const SoundMode& sm = soundModes[currentSoundMode];
    
//     // Configuration des waveforms selon le mode
//     waveform1.begin(sm.waveformType);
//     waveform1.amplitude(sm.amplitude);
//     // Configuration de l'enveloppe
//     env.attack(sm.attack);
//     env.decay(sm.decay);
//     env.sustain(sm.sustain);
//     env.release(sm.release);
    
//     waveform2.begin(sm.waveformType);
//     waveform2.amplitude(sm.amplitude * 0.7); // Deuxième oscillateur plus faible
//     DEBUG_VAR_SOUND("Sound mode set to: ", sm.name);
//   }
//   else if (mode == 5) {
//     DEBUG_INFO_SOUND("Mode String");
//     stringMode = true;

//   }
//   else if (mode == 6) {
//     DEBUG_INFO_SOUND("Mode Drum");
//     drumMode = true;
//   }

// }

void SoundManager::buildScale(int noteIndex, int gammeIndex) {
  //float baseFreq = BASE_FREQUENCIES[noteIndex];
  int intervals[NUM_TOUCH_PADS];
  int octaveOffset = 0;
  switch (gammeIndex) {
    case 0:
    // Gamme majeur
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = MAJOR_INTERVALS[i];
      }
    break;
    case 1:
    // Gamme mineure
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = MINOR_INTERVALS[i];
      }
    break;
    case 2:
    // Gamme chromatique
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = CHROMATIC_INTERVALS[i];
      }
    break;
    case 3:
    // penta majeur
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = PENTATONIC_MAJOR_INTERVALS[i];
      }
    break;
    case 4:
    // penta mineure
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = PENTATONIC_MINOR_INTERVALS[i];
      }
      break;
    case 5:
    // Akebono
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = AKEBONO_INTERVALS[i];
      }
    break;
    case 6:
    //HIJAZ_INTERVALS
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = HIJAZ_INTERVALS[i];
      }
    break;
    case 7:
    //TONE_SCALE
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = TONESCALE_INTERVALS[i];
      }
    break;
    case 8:
    //Diminuée
      for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        intervals[i] = DIMSCALE_INTERVALS[i];
      }
    break;

  }
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    int index = noteIndex + intervals[i];
    if (index > 11 && index < 24) {
      octaveOffset = 1;
    }
    else if (index >= 24) {
      octaveOffset = 2;
    }
    int freqIndex = index % 12;
    scaleFreq[i] = BASE_FREQUENCIES[freqIndex] * (1 << octaveOffset); // multiplier par 2^octaveOffset
  }
  
}

void SoundManager::generateRandomMap() {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        randomMap[i] = i;
    }
    // Shuffle Fisher-Yates
    for (int i = NUM_TOUCH_PADS - 1; i > 0; i--) {
        int j = random(i + 1);
        int temp = randomMap[i];
        randomMap[i] = randomMap[j];
        randomMap[j] = temp;
    }
}


float SoundManager::getNote(int padIndex) {
    switch (keyboardMode) {
        case 0: 
          return scaleFreq[NORMAL_MODE[padIndex]];
        case 1: 
          return scaleFreq[HANDPAN_MODE[padIndex]];
        case 2: 
          return scaleFreq[NUM_TOUCH_PADS - 1 - padIndex];
        case 3: 
          return scaleFreq[randomMap[padIndex]];
        case 4: 
          return scaleFreq[SPIRAL_MODE[padIndex]];
        default:
          DEBUG_VAR_SOUND("Invalid keyboard mode", keyboardMode);
          return 0.0f;

    }
}

/* void SoundManager::playTouchSound(int touchIndex) {
    if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;

    const SoundMode& sm = soundModes[currentSoundMode];
    float baseFreq = getNote(touchIndex);
    int voiceIndex;
    if (!polyMode) {
      voiceIndex = 0;
    }
    else {
      voiceIndex = touchIndex;
    }
    switch(sm.type) {
      case SYNTH:
        //polyWave[voiceIndex].begin(sm.synth.waveformType);
        polyWave[voiceIndex].frequency(baseFreq);
        polyModFM[voiceIndex].frequency(baseFreq * sm.synth.ratioFm);
        //polyWave[voiceIndex].amplitude(sm.amplitude * 0.6);  // amplitude max pour l’enveloppe
        //polyWave[voiceIndex].frequencyModulation(sm.)
        polyEnv[voiceIndex].noteOn();                // déclenche ADSR
      break;

      case DRUM:
        polyDrum[voiceIndex].frequency(baseFreq * 0.25);
        polyDrum[voiceIndex].noteOn();
      break;

      case STRING:
        polyString[voiceIndex].noteOn(baseFreq * 0.5, sm.string.velocity);
    }
    

} */

void SoundManager::stopMidi() {
  uint8_t channel = MidiManager::getChannel();
  MidiManager::allNotesOff(channel);
}

void SoundManager::stopOneNote(int touchIndex) {
  if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;
  polyEnv[touchIndex].noteOff();
  polyString[touchIndex].noteOff(0.5);
  if (MidiManager::getEnabled() && activeMidiNotes[touchIndex] > 0) {
    MidiManager::noteOff(activeMidiNotes[touchIndex]);
    activeMidiNotes[touchIndex] = 0;
  }
}

void SoundManager::stopAllSounds() {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
      polyEnv[i].noteOff();  // release ADSR par note
      polyString[i].noteOff(0.5);
    }
    lfo.amplitude(0.0);
    //fxMixer.gain(2, 0);
    if (MidiManager::getEnabled()) {
      updateMidiNotes();
    }
    isPlaying = false;
}


const char* SoundManager::getSoundModeName() const {
  return soundModes[currentSoundMode].name;
}

void SoundManager::volume(uint8_t vol) {
  float volFloat;
  // Conversion en float normalisé 0.0 à 1.0
  volFloat = (float)vol / 10.0f;
  //amp1.gain(volFloat);
  polyAmp.gain(volFloat * 0.8);
  //amp2.gain(volFloat);
  //DEBUG_VAR_SOUND("volume amp1: ", volFloat);
  // DEBUG_PRINT("volume amp1: ");
  // DEBUG_PRINTLN(volFloat);
  
}

void SoundManager::testPoly() {
    DEBUG_INFO_SOUND("Testing poly mode...");
    for (int i = 0; i < 4; i++) {  // Test sur 4 notes
        polyWave[i].begin(WAVEFORM_SINE);
        polyWave[i].frequency(440 + i * 100);  // Fréquences différentes
        polyWave[i].amplitude(0.3);
        polyEnv[i].noteOn();
        delay(200);
    }
}

void sendSoundModeCC(uint8_t soundIndex) {
    const SoundMode &sm = soundModes[soundIndex];
    uint8_t baseCC = CC_BASE_SOUND[soundIndex];

    if (sm.type == SYNTH) {
        MidiManager::controlChange(baseCC + 0, sm.synth.waveFormType);
        MidiManager::controlChange(baseCC + 1, sm.synth.amplitude * 127);
        MidiManager::controlChange(baseCC + 2, map(sm.synth.attack, 0, 5000, 0, 127));
        MidiManager::controlChange(baseCC + 3, map(sm.synth.decay, 0, 5000, 0, 127));
        MidiManager::controlChange(baseCC + 4, sm.synth.sustain * 127);
        MidiManager::controlChange(baseCC + 5, map(sm.synth.release, 0, 5000, 0, 127));
        MidiManager::controlChange(baseCC + 6, map(sm.synth.freqMod, 0, 5, 0, 127));

        // … continuer pour tous les paramètres
    }
    if (sm.type == DRUM) {
      MidiManager::controlChange(28, sm.drum.secondMix * 127);
      MidiManager::controlChange(28, sm.drum.pitchMode * 127);
    }
    // gérer STRING, DRUM, SAMPLE comme avant
}

void sendGlobalEffectsCC() {
    // Exemple : réverbération, chorus, etc.
    MidiManager::controlChange(CC_BASE_EFFECTS + 0, reverbAmount * 127);
    MidiManager::controlChange(CC_BASE_EFFECTS + 1, chorusAmount * 127);
}



void SoundManager::handleCC(int cc, int value) {
  switch(cc) {
    case 20: // Reverb size
      effects.reverbRoomSize = (float)value / 127.0f;
      polyReverb.roomsize(effects.reverbRoomSize);
      break;
    case 21: // Reverb damping
      effects.reverbDamping = (float)value / 127.0f;
      polyReverb.damping(effects.reverbDamping);
      break;
    case 22: // Filter frequency
      effects.filterFreq = map(value, 0, 127, 100, 20000);
      filtre.frequency(effects.filterFreq);
      break;
    case 23: // Filter resonance
      effects.filterResonance = (float)value / 127.0f * 5.0f; // Ajuster l'échelle si nécessaire
      filtre.resonance(effects.filterResonance);
      break;
    case 24: // Chorus voices
      effects.nChorus = map(value, 0, 127, 0, 16);
      chorus.voices(effects.nChorus);
      break;
    case 25: // Tremolo rate
      effects.tremoloRate = map(value, 0, 127, 1, 20);
      lfo.frequency(effects.tremoloRate);
      break;
    case 26: // Tremolo amplitude
      effects.tremoloAmplitude = (float)value / 127.0f;
      lfo.amplitude(effects.tremoloAmplitude);
      break;
    case 27: // Tremolo Waveform
      effects.tremoloWaveform = map(value, 0, 127, 0, 6);
      switch(effects.tremoloWaveform) {
        case 0:
          lfo.begin(WAVEFORM_SINE);
          break;
        case 1:
          lfo.begin(WAVEFORM_SAWTOOTH);
          break;
        case 2:
          lfo.begin(WAVEFORM_SQUARE);
          break;
        case 3:
          lfo.begin(WAVEFORM_TRIANGLE);
          break;
        case 4:
          lfo.begin(WAVEFORM_SAWTOOTH_REVERSE);
          break;
        case 5:
          lfo.begin(WAVEFORM_PULSE);
          break;
        case 6:
          lfo.begin(WAVEFORM_SAMPLE_HOLD);
          break;
      }
      lfo.begin(effects.tremoloWaveform);
      break;
      
    default:
      DEBUG_VAR_SOUND("Unhandled CC: ", cc);
      break;
  }
}

void SoundManager::updateMidiNotes() {
    for (int i = 0; i < NUM_TOUCH_PADS; i++) {
        if (activeMidiNotes[i] > 0) {
          DEBUG_INFO_SOUND("fin d'enveloppe"); // L'enveloppe s'est arrêtée
          MidiManager::noteOff(activeMidiNotes[i]);
          activeMidiNotes[i] = 0;
            
        }
    }
}

void SoundManager::playMidi(int touchIndex, float frequency, float velocity) {
  uint8_t midiNote = MapToMidi::freqToMidi(frequency);
  uint8_t midiVelocity = MapToMidi::pressureToMidi(velocity);
  //uint8_t channel = MidiManager::getChannel();
  DEBUG_VAR_SOUND("MIDI Note: ", midiNote);
  DEBUG_VAR_SOUND("MIDI Velocity: ", midiVelocity);
  MidiManager::noteOn(midiNote, midiVelocity);
  activeMidiNotes[touchIndex] = midiNote;
}

void SoundManager::setPolyEnv(int attack, int decay, float sustain, int release) {
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    polyEnv[i].attack(attack);
    polyEnv[i].decay(decay);
    polyEnv[i].sustain(sustain);
    polyEnv[i].release(release);
  }
}

void SoundManager::soundDuration(uint16_t durationMS) {
  sampleDuration = durationMS;
  int attack = constrain(durationMS / 10, 10, 100); 
  int decay = constrain(durationMS / 3, 10, 1000);
  int release = constrain(durationMS / 4, 10, 400);
  if (attack + decay + release > durationMS) {
    decay = decay / 2;
    release = release / 2;
  }
  setPolyEnv(attack, decay, 0.5, release);

}

void SoundManager::setArpeggio(bool enabled) {
  arpegioMode = enabled;
}

void SoundManager::playTouchSound(int touchIndex, uint16_t pression = 150, uint8_t velocite = 150) {
    if (touchIndex < 0 || touchIndex >= NUM_TOUCH_PADS) return;

    const SoundMode& sm = soundModes[currentSoundMode];
    float baseFreq = getNote(touchIndex);
    int voiceIndex = polyMode ? touchIndex : 0;

    // Normalisation pression et vélocité (à adapter selon tes valeurs typiques)
    float normPression = constrain((float)pression / maxPressure, 0.1f, 1.0f); // 550 = pression max typique
    DEBUG_VAR_SOUND("Norm Pression: ", normPression);
    //float normVelocite = constrain((float)velocite / 255.0f, 0.0f, 1.0f);
    //float normVelocite = 1.0f - constrain((float)velocite / (float)maxTime, 0.0f, 1.0f);
    uint8_t velociteMs = map(velocite, 10, 200, int(maxTime), 10);
    DEBUG_VAR_SOUND("Norm Vélocité: ", velociteMs);
    lfo.amplitude(effects.tremoloRate * normPression); // LFO amplitude modulée par la pression
    //fxMixer.gain(2, 0.5);
    chorus.voices(effects.nChorus);


    switch(sm.type) {
      case SYNTH:
        if (!arpegioMode) {
        polyWave[voiceIndex].frequency(baseFreq);
        polyModFM[voiceIndex].frequency(baseFreq * sm.synth.ratioFm);
        polyWave[voiceIndex].amplitude(sm.synth.amplitude * normPression);  // amplitude max pour l’enveloppe

        // Modulation de l’enveloppe par pression/vélocité
        polyEnv[voiceIndex].attack(10 + velocite);   // vélocité rapide = attaque courte
        polyEnv[voiceIndex].decay(200 + 800 * (1.0f - normPression));  // pression forte = decay court
        polyEnv[voiceIndex].sustain(0.3 + 0.7 * normPression);         // pression forte = sustain élevé
        lpfEnv.attack(10 + velocite);
        lpfEnv.decay(200 + 800 * (1.0f - normPression));
        lpfEnv.sustain(0.3 + 0.7 * normPression);
        //polyEnv[voiceIndex].release(100 + 400 * (1.0f - normVelocite)); // vélocité rapide = release court
        }
        polyEnv[voiceIndex].noteOn();
        cvEnv.amplitude(1);
      break;

      case DRUM:
        lpfEnv.attack(sm.drum.length);
        polyDrum[voiceIndex].frequency(baseFreq * 0.25);
        polyDrum[voiceIndex].secondMix(normPression); // exemple : moduler le mix par pression
        polyDrum[voiceIndex].noteOn();
        cvEnv.amplitude(1);
        lpfEnv.noteOn();
      break;

      case STRING:
        polyString[voiceIndex].noteOn(baseFreq * 0.5, 0.5 + 0.5 * normPression); // vélocité module l’intensité
      break;
      case SAMPLE:
        int nMixer = voiceIndex / 4;
        int nchannel = voiceIndex % 4;
        sampleMixer[nMixer].gain(nchannel, sampleLevel * normPression);
        if (!arpegioMode) {        
        polySample[voiceIndex].play(drumSamplesLong[touchIndex]);
        }
        else {
          if (sampleDuration >= 250) {
            polySample[voiceIndex].play(drumSamplesLong[touchIndex]);
          }
          else if (sampleDuration < 250 && sampleDuration >= 50) {
            polySample[voiceIndex].play(drumSamples100ms[touchIndex]);
          }
          else if (sampleDuration < 50) {
            polySample[voiceIndex].play(drumSamples40ms[touchIndex]);
          }

        }
        
    }
  if (MidiManager::getEnabled()) {
    playMidi(touchIndex, baseFreq, pression);
  }
}


#endif