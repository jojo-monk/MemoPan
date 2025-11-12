
#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <Audio.h>
#include "config.h"
#include <OpenAudio_ArduinoLibrary.h>
#include <hexefx_audiolib_F32.h>

// Classe dédiée au moteur audio (objets audio et connexions)
class AudioEngine {
public:
  // Objets audio publics pour accès direct
  AudioOutputI2S                i2s1;
  AudioControlSGTL5000          sgtl5000_1;
  AudioEffectGainStereo_F32     polyAmp;
  AudioEffectMonoToStereo_F32   mono2stereo;
  AudioAmplifier                amp2;
  AudioConvert_I16toF32         convertIn;
  AudioConvert_F32toI16         convert_left;
  AudioConvert_F32toI16         convert_right;
  AudioAnalyzePeak              peakL;
  AudioAnalyzePeak              peakR;
  AudioAnalyzeRMS               rmsL;
  AudioAnalyzeRMS               rmsR;
  

  // Synthétiseurs polyphoniques
  AudioPlayMemory               polySample[NUM_TOUCH_PADS];
  AudioSynthWaveformModulated   polyWave[NUM_TOUCH_PADS];
  AudioEffectEnvelope           polyEnv[NUM_TOUCH_PADS];
  AudioEffectEnvelope           polyEnv2[NUM_TOUCH_PADS]; // pour filtre lp
  AudioEffectEnvelope           polyEnv3[NUM_TOUCH_PADS]; // pour Modulation FM
  AudioSynthWaveform            polyModFM[NUM_TOUCH_PADS];
  AudioSynthKarplusStrong       polyString[NUM_TOUCH_PADS];
  AudioSynthSimpleDrum          polyDrum[NUM_TOUCH_PADS];
  
  

  // Effets
  AudioSynthNoiseWhite          whiteNoise;
  AudioEffectEnvelope           noiseEnv;
  AudioEffectEnvelope           noiseCutOffEnv;
  AudioSynthWaveformDc          noiseDc;
  AudioSynthWaveformDc          dcPolyEnv2[NUM_TOUCH_PADS]; // pour enveloppe 2
  AudioSynthWaveformDc          dcMoogEnv;
  AudioEffectEnvelope           moogEnv;
  AudioSynthWaveform            moogLfo;
  AudioFilterStateVariable      lpFiltre[NUM_TOUCH_PADS]; // pour enveloppe 2
  AudioMixer4                   polyMixer1;
  AudioMixer4                   polyMixer2;
  AudioMixer4                   polyMixer3;
  AudioMixer4                   polyMixerGlobal;
  AudioMixer4                   soundMixer[9];
  AudioMixer4                   fxMixer;
  AudioEffectPlateReverb_F32    polyReverb;
  AudioSynthWaveform            lfo;
  AudioEffectMultiply           tremolo;
  AudioEffectChorus             chorus;
  AudioFilterLadder             moogFilter;
  AudioFilterStateVariable      noiseFilter;
  AudioEffectEnvelope           tremoloEnv;
  AudioEffectEnvelope           moogLfoEnv;
  AudioEffectCompressorStereo_F32 comp;
  

  
  // Mixers
  AudioMixer4 sampleMixer[4];
  AudioMixer4 noiseMix;

  // Constructeur : initialise toutes les connexions
  AudioEngine();

  // Initialisation du hardware audio
  bool begin();

  // Configuration des gains par défaut
  void setupDefaultGains();

  // Accès aux buffers de delay
  short* getDelayLine() { return delayline; }
  int getDelayLength() const { return CHORUS_DELAY_LENGTH; }

private:
  // Buffer pour chorus
  static constexpr int CHORUS_DELAY_LENGTH = (32 * AUDIO_BLOCK_SAMPLES);
  short delayline[CHORUS_DELAY_LENGTH];

  // Toutes les connexions audio
    AudioConnection sample1ToSampleMixer1;
    AudioConnection sample2ToSampleMixer1;
    AudioConnection sample3ToSampleMixer1;
    AudioConnection sample4ToSampleMixer1;
    AudioConnection sample5ToSampleMixer2;
    AudioConnection sample6ToSampleMixer2;
    AudioConnection sample7ToSampleMixer2;
    AudioConnection sample8ToSampleMixer2;
    AudioConnection sample9ToSampleMixer3;
    AudioConnection sample10ToSampleMixer3;
    AudioConnection sample11ToSampleMixer3;
    AudioConnection sample12ToSampleMixer3;
    AudioConnection sampleMixer1ToSampleMixer4;
    AudioConnection sampleMixer2ToSampleMixer4;
    AudioConnection sampleMixer3ToSampleMixer4;
    AudioConnection sampleMixer4TofxMixer;

    AudioConnection polyPatch1;
    AudioConnection polyPatch2;
    AudioConnection polyPatch3;
    AudioConnection polyPatch4;
    AudioConnection polyPatch5;
    AudioConnection polyPatch6;
    AudioConnection polyPatch7;
    AudioConnection polyPatch8;
    AudioConnection polyPatch9;
    AudioConnection polyPatch10;
    AudioConnection polyPatch11;
    AudioConnection polyPatch12;
    AudioConnection polyPatch13;
    AudioConnection polyPatch14;
    AudioConnection polyPatch15;
    AudioConnection polyPatch16;
    AudioConnection polyPatch17;
    AudioConnection polyPatch18;
    AudioConnection polyPatch19;
    AudioConnection polyPatch20;
    AudioConnection polyPatch21;
    AudioConnection polyPatch22;
    AudioConnection polyPatch23;
    AudioConnection polyPatch24;
    AudioConnection polyPatch25;
    AudioConnection polyPatch26;
    AudioConnection polyPatch27;
    AudioConnection polyPatch28;
    AudioConnection polyPatch29;
    AudioConnection polyPatch30;
    AudioConnection polyPatch31;
    AudioConnection polyPatch32;
    AudioConnection polyPatch33;
    AudioConnection polyPatch34;
    AudioConnection polyPatch35;
    AudioConnection polyPatch36;

    AudioConnection polyMixer1Patch1;
    AudioConnection polyMixer1Patch2;
    AudioConnection polyMixer1Patch3;
    AudioConnection polyMixer1Patch4;
    AudioConnection polyMixer2Patch1;
    AudioConnection polyMixer2Patch2;
    AudioConnection polyMixer2Patch3;
    AudioConnection polyMixer2Patch4;
    AudioConnection polyMixer3Patch1;
    AudioConnection polyMixer3Patch2;
    AudioConnection polyMixer3Patch3;
    AudioConnection polyMixer3Patch4;

    AudioConnection dcMoogEnvToMoogEnv;
    AudioConnection moogEnvToMoogFilter;
    AudioConnection moogFilterToMoogMix1;
    //AudioConnection moogMixToFiltre;
    AudioConnection moogMixToConvert;
    AudioConnection_F32 ConvertToM2S;
    AudioConnection_F32 M2SToPolyReverbL;
    AudioConnection_F32 M2SToPolyReverbR;
    AudioConnection moogLfoToEnv;
    AudioConnection moogLfoEnvToMoogFilter;
    //AudioConnection moogMixToDryWet;
    AudioConnection_F32 polyReverbToPolyAmpL;
    AudioConnection_F32 polyReverbToPolyAmpR;
    AudioConnection_F32 polyAmpLToCompL;
    AudioConnection_F32 polyAmpRToCompR;
    AudioConnection_F32 compLToConvert;
    AudioConnection_F32 compRToConvert;
    AudioConnection fxMixerTomoogMix;
    AudioConnection fxMixerToBiquad;
    AudioConnection biquadToMoogFilter;

    AudioConnection whiteNoiseToNoiseEnv;
    AudioConnection noiseFilterToNoiseMix1;
    AudioConnection noiseFilterToNoiseMix2;
    AudioConnection noiseFilterToNoiseMix3;
    AudioConnection noiseCutOffEnvToNoiseFilter;
    AudioConnection noiseEnvToNoiseFilter;
    AudioConnection noiseMixToFxMixer;
    AudioConnection noiseDcToNoiseCutOffEnv;

    AudioConnection polyMixerGlobalPatch1;
    AudioConnection polyMixerGlobalPatch2;
    AudioConnection polyMixerGlobalPatch3;

    AudioConnection string1ToSoundMixer1;
    AudioConnection string2ToSoundMixer1;
    AudioConnection string3ToSoundMixer1;
    AudioConnection string4ToSoundMixer1;
    AudioConnection string5ToSoundMixer2;
    AudioConnection string6ToSoundMixer2;
    AudioConnection string7ToSoundMixer2;
    AudioConnection string8ToSoundMixer2;
    AudioConnection string9ToSoundMixer3;
    AudioConnection string10ToSoundMixer3;
    AudioConnection string11ToSoundMixer3;
    AudioConnection string12ToSoundMixer3;

    AudioConnection drum1ToSoundMixer4;
    AudioConnection drum2ToSoundMixer4;
    AudioConnection drum3ToSoundMixer4;
    AudioConnection drum4ToSoundMixer4;
    AudioConnection drum5ToSoundMixer5;
    AudioConnection drum6ToSoundMixer5;
    AudioConnection drum7ToSoundMixer5;
    AudioConnection drum8ToSoundMixer5;
    AudioConnection drum9ToSoundMixer6;
    AudioConnection drum10ToSoundMixer6;
    AudioConnection drum11ToSoundMixer6;
    AudioConnection drum12ToSoundMixer6;

    AudioConnection soundMixerToMix1;
    AudioConnection soundMixerToMix2;
    AudioConnection soundMixerToMix3;
    AudioConnection soundMixerToMix4;
    AudioConnection soundMixerToMix5;
    AudioConnection soundMixerToMix6;
    AudioConnection mixToMix1;
    AudioConnection mixToMix2;
    AudioConnection mixToAmp2;
    AudioConnection amp2ToGlobal;

    AudioConnection polyEnv2ToLpFiltre1;
    AudioConnection polyEnv2ToLpFiltre2;
    AudioConnection polyEnv2ToLpFiltre3;
    AudioConnection polyEnv2ToLpFiltre4;
    AudioConnection polyEnv2ToLpFiltre5;
    AudioConnection polyEnv2ToLpFiltre6;
    AudioConnection polyEnv2ToLpFiltre7;
    AudioConnection polyEnv2ToLpFiltre8;
    AudioConnection polyEnv2ToLpFiltre9;
    AudioConnection polyEnv2ToLpFiltre10;
    AudioConnection polyEnv2ToLpFiltre11;
    AudioConnection polyEnv2ToLpFiltre12;

    AudioConnection lpFiltre1ToPolyMixer1;
    AudioConnection lpFiltre2ToPolyMixer1;
    AudioConnection lpFiltre3ToPolyMixer1;
    AudioConnection lpFiltre4ToPolyMixer1;
    AudioConnection lpFiltre5ToPolyMixer2;
    AudioConnection lpFiltre6ToPolyMixer2;
    AudioConnection lpFiltre7ToPolyMixer2;
    AudioConnection lpFiltre8ToPolyMixer2;
    AudioConnection lpFiltre9ToPolyMixer3;
    AudioConnection lpFiltre10ToPolyMixer3;
    AudioConnection lpFiltre11ToPolyMixer3;
    AudioConnection lpFiltre12ToPolyMixer3;
    
    AudioConnection dcPolyEnv2ToPolyEnv2_1;
    AudioConnection dcPolyEnv2ToPolyEnv2_2;
    AudioConnection dcPolyEnv2ToPolyEnv2_3;
    AudioConnection dcPolyEnv2ToPolyEnv2_4;
    AudioConnection dcPolyEnv2ToPolyEnv2_5;
    AudioConnection dcPolyEnv2ToPolyEnv2_6;
    AudioConnection dcPolyEnv2ToPolyEnv2_7;
    AudioConnection dcPolyEnv2ToPolyEnv2_8;
    AudioConnection dcPolyEnv2ToPolyEnv2_9;
    AudioConnection dcPolyEnv2ToPolyEnv2_10;
    AudioConnection dcPolyEnv2ToPolyEnv2_11;
    AudioConnection dcPolyEnv2ToPolyEnv2_12;

    AudioConnection tremoloLfoToEnv;
    AudioConnection EnvToTremolo;
    AudioConnection tremoloToFxMixer;
    AudioConnection polyMixerGlobalToTremolo;
    AudioConnection polyMixerGlobalToChorus;
    AudioConnection chorusToFxMixer;
    AudioConnection FxMixerTodrywet;
    AudioConnection polyMixerGlobalToNoiseMix;
    AudioConnection fxMixerToF;
    AudioConnection finalPeakL;
    AudioConnection finalPeakR;
    AudioConnection finalRmsL;
    AudioConnection finalRmsR;
    AudioConnection finalToI2S_L0;
    AudioConnection finalToI2S_R0;
};

#endif // AUDIOENGINE_H
