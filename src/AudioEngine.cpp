#include "AudioEngine.h"
#include "Debug.h"

AudioEngine::AudioEngine() :
  // ========== CONNEXIONS DES SAMPLES ==========
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

  // ========== CONNEXIONS POLY WAVE -> ENVELOPPE ==========
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

  // ========== CONNEXIONS MODULATION FM ==========
  polyPatch13(polyModFM[0], 0, polyEnv3[0], 0),
  polyPatch14(polyModFM[1], 0, polyEnv3[1], 0),
  polyPatch15(polyModFM[2], 0, polyEnv3[2], 0),
  polyPatch16(polyModFM[3], 0, polyEnv3[3], 0),
  polyPatch17(polyModFM[4], 0, polyEnv3[4], 0),
  polyPatch18(polyModFM[5], 0, polyEnv3[5], 0),
  polyPatch19(polyModFM[6], 0, polyEnv3[6], 0),
  polyPatch20(polyModFM[7], 0, polyEnv3[7], 0),
  polyPatch21(polyModFM[8], 0, polyEnv3[8], 0),
  polyPatch22(polyModFM[9], 0, polyEnv3[9], 0),
  polyPatch23(polyModFM[10], 0, polyEnv3[10], 0),
  polyPatch24(polyModFM[11], 0, polyEnv3[11], 0),

  polyPatch25(polyEnv3[0], 0, polyWave[0], 0),
  polyPatch26(polyEnv3[1], 0, polyWave[1], 0),
  polyPatch27(polyEnv3[2], 0, polyWave[2], 0),
  polyPatch28(polyEnv3[3], 0, polyWave[3], 0),
  polyPatch29(polyEnv3[4], 0, polyWave[4], 0),
  polyPatch30(polyEnv3[5], 0, polyWave[5], 0),
  polyPatch31(polyEnv3[6], 0, polyWave[6], 0),
  polyPatch32(polyEnv3[7], 0, polyWave[7], 0),
  polyPatch33(polyEnv3[8], 0, polyWave[8], 0),
  polyPatch34(polyEnv3[9], 0, polyWave[9], 0),
  polyPatch35(polyEnv3[10], 0, polyWave[10], 0),
  polyPatch36(polyEnv3[11], 0, polyWave[11], 0),

  // ========== CONNEXIONS STRINGS ==========
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

  // ========== CONNEXIONS DRUMS ==========
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

  // ========== CONNEXIONS DES MIXERS ENTRE EUX ==========
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

    // ========== CONNEXIONS dc polyenv2 pour lpFiltre ==========
    dcPolyEnv2ToPolyEnv2_1(dcPolyEnv2[0], 0, polyEnv2[0], 0),
    dcPolyEnv2ToPolyEnv2_2(dcPolyEnv2[1], 0, polyEnv2[1], 0),
    dcPolyEnv2ToPolyEnv2_3(dcPolyEnv2[2], 0, polyEnv2[2], 0),
    dcPolyEnv2ToPolyEnv2_4(dcPolyEnv2[3], 0, polyEnv2[3], 0),
    dcPolyEnv2ToPolyEnv2_5(dcPolyEnv2[4], 0, polyEnv2[4], 0),
    dcPolyEnv2ToPolyEnv2_6(dcPolyEnv2[5], 0, polyEnv2[5], 0),
    dcPolyEnv2ToPolyEnv2_7(dcPolyEnv2[6], 0, polyEnv2[6], 0),
    dcPolyEnv2ToPolyEnv2_8(dcPolyEnv2[7], 0, polyEnv2[7], 0),
    dcPolyEnv2ToPolyEnv2_9(dcPolyEnv2[8], 0, polyEnv2[8], 0),
    dcPolyEnv2ToPolyEnv2_10(dcPolyEnv2[9], 0, polyEnv2[9], 0),
    dcPolyEnv2ToPolyEnv2_11(dcPolyEnv2[10], 0, polyEnv2[10], 0),
    dcPolyEnv2ToPolyEnv2_12(dcPolyEnv2[11], 0, polyEnv2[11], 0),

    //========== CONNEXIONS polyEnv2 lpfiltre ==========
    polyEnv2ToLpFiltre1(polyEnv2[0], 0, lpFiltre[0], 1),
    polyEnv2ToLpFiltre2(polyEnv2[1], 0, lpFiltre[1], 1),
    polyEnv2ToLpFiltre3(polyEnv2[2], 0, lpFiltre[2], 1),
    polyEnv2ToLpFiltre4(polyEnv2[3], 0, lpFiltre[3], 1),
    polyEnv2ToLpFiltre5(polyEnv2[4], 0, lpFiltre[4], 1),
    polyEnv2ToLpFiltre6(polyEnv2[5], 0, lpFiltre[5], 1),
    polyEnv2ToLpFiltre7(polyEnv2[6], 0, lpFiltre[6], 1),
    polyEnv2ToLpFiltre8(polyEnv2[7], 0, lpFiltre[7], 1),
    polyEnv2ToLpFiltre9(polyEnv2[8], 0, lpFiltre[8], 1),
    polyEnv2ToLpFiltre10(polyEnv2[9], 0, lpFiltre[9], 1),
    polyEnv2ToLpFiltre11(polyEnv2[10], 0, lpFiltre[10], 1),
    polyEnv2ToLpFiltre12(polyEnv2[11], 0, lpFiltre[11], 1),

    // ========== CONNEXIONS lpFiltre polyMixer ==========
    lpFiltre1ToPolyMixer1(lpFiltre[0], 0, polyMixer1, 0),
    lpFiltre2ToPolyMixer1(lpFiltre[1], 0, polyMixer1, 1),
    lpFiltre3ToPolyMixer1(lpFiltre[2], 0, polyMixer1, 2),
    lpFiltre4ToPolyMixer1(lpFiltre[3], 0, polyMixer1, 3),
    lpFiltre5ToPolyMixer2(lpFiltre[4], 0, polyMixer2, 0),
    lpFiltre6ToPolyMixer2(lpFiltre[5], 0, polyMixer2, 1),
    lpFiltre7ToPolyMixer2(lpFiltre[6], 0, polyMixer2, 2),
    lpFiltre8ToPolyMixer2(lpFiltre[7], 0, polyMixer2, 3),
    lpFiltre9ToPolyMixer3(lpFiltre[8], 0, polyMixer3, 0),
    lpFiltre10ToPolyMixer3(lpFiltre[9], 0, polyMixer3, 1),
    lpFiltre11ToPolyMixer3(lpFiltre[10], 0, polyMixer3, 2),
    lpFiltre12ToPolyMixer3(lpFiltre[11], 0, polyMixer3, 3),

  // ========== CONNEXIONS POLY MIXERS ==========
  polyMixer1Patch1(polyEnv[0], 0, lpFiltre[0], 0),
  polyMixer1Patch2(polyEnv[1], 0, lpFiltre[1], 0),
  polyMixer1Patch3(polyEnv[2], 0, lpFiltre[2], 0),
  polyMixer1Patch4(polyEnv[3], 0, lpFiltre[3], 0),
  polyMixer2Patch1(polyEnv[4], 0, lpFiltre[4], 0),
  polyMixer2Patch2(polyEnv[5], 0, lpFiltre[5], 0),
  polyMixer2Patch3(polyEnv[6], 0, lpFiltre[6], 0),
  polyMixer2Patch4(polyEnv[7], 0, lpFiltre[7], 0),
  polyMixer3Patch1(polyEnv[8], 0, lpFiltre[8], 0),
  polyMixer3Patch2(polyEnv[9], 0, lpFiltre[9], 0),
  polyMixer3Patch3(polyEnv[10], 0, lpFiltre[10], 0),
  polyMixer3Patch4(polyEnv[11], 0, lpFiltre[11], 0),

  polyMixerGlobalPatch1(polyMixer1, 0, polyMixerGlobal, 0),
  polyMixerGlobalPatch2(polyMixer2, 0, polyMixerGlobal, 1),
  polyMixerGlobalPatch3(polyMixer3, 0, polyMixerGlobal, 2),

  // ========== CONNEXIONS MoogFilter ========
  dcMoogEnvToMoogEnv(dcMoogEnv, 0, moogEnv, 0),
  moogEnvToMoogFilter(moogEnv, 0, moogFilter, 1),
  moogLfoToEnv(moogLfo, 0, moogLfoEnv, 0),
  moogLfoEnvToMoogFilter(moogLfoEnv, 0, moogFilter, 2),
  moogMixToConvert(moogFilter, 0, convertIn, 0),
  ConvertToM2S(convertIn, 0, mono2stereo, 0),
  M2SToPolyReverbL(mono2stereo, 0, polyReverb, 0),
  M2SToPolyReverbR(mono2stereo, 1, polyReverb, 1),
  polyReverbToPolyAmpL(polyReverb, 0, polyAmp, 0),
  polyReverbToPolyAmpR(polyReverb, 1, polyAmp, 1),
  polyAmpLToCompL(polyAmp, 0, comp, 0),
  polyAmpRToCompR(polyAmp, 1, comp, 1),
  compLToConvert(comp, 0, convert_left, 0),
  compRToConvert(comp, 1, convert_right, 0),

  // ========== CONNEXIONS EFFETS ==========
  tremoloLfoToEnv(lfo, 0, tremoloEnv, 0),
  EnvToTremolo(tremoloEnv, 0, tremolo, 1),
  tremoloToFxMixer(tremolo, 0, fxMixer, 1),
  polyMixerGlobalToTremolo(polyMixerGlobal, 0, tremolo, 0),
  polyMixerGlobalToChorus(polyMixerGlobal, 0, chorus, 0),
  chorusToFxMixer(chorus, 0, fxMixer, 2),
  polyMixerGlobalToNoiseMix(polyMixerGlobal, 0, noiseMix, 0),
  fxMixerToF(fxMixer, 0, moogFilter, 0),
  noiseMixToFxMixer(noiseMix, 0, fxMixer, 0),
  
  //========== CONNEXIONS NOISE GENERATOR ========
  whiteNoiseToNoiseEnv(whiteNoise, 0, noiseEnv, 0),
  noiseEnvToNoiseFilter(noiseEnv, 0, noiseFilter, 0),
  noiseCutOffEnvToNoiseFilter(noiseCutOffEnv, 0, noiseFilter, 1),
  noiseFilterToNoiseMix1(noiseFilter, 0, noiseMix, 1),
  noiseFilterToNoiseMix2(noiseFilter, 1, noiseMix, 2),
  noiseFilterToNoiseMix3(noiseFilter, 2, noiseMix, 3),
  noiseDcToNoiseCutOffEnv(noiseDc, 0, noiseCutOffEnv, 0),

  // ========== SORTIE FINALE I2S ==========
  finalPeakL(convert_left, 0, peakL, 0),
  finalPeakR(convert_right, 0, peakR, 0),
  finalRmsL(convert_left, 0, rmsL, 0),
  finalRmsR(convert_right, 0, rmsR, 0),
  finalToI2S_L0(convert_left, 0, i2s1, 0),
  finalToI2S_R0(convert_right, 0, i2s1, 1)
{
  // Initialisation de base des oscillateurs et enveloppes
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    polyWave[i].begin(WAVEFORM_SINE);
    polyWave[i].amplitude(0.6);
    polyEnv[i].attack(30);
    polyEnv[i].decay(1800);
    polyEnv[i].sustain(0.6);
    polyEnv[i].release(800);
    polyEnv2[i].attack(30);
    polyEnv2[i].decay(1800);
    polyEnv2[i].sustain(0.6);
    polyEnv2[i].release(800);
    dcPolyEnv2[i].amplitude(1);
  }
    moogEnv.attack(30);
    moogEnv.decay(1800);
    moogEnv.sustain(0.6);
    moogEnv.release(800);
    moogLfoEnv.attack(30);
    moogLfoEnv.decay(1800);
    moogLfoEnv.sustain(0.6);
    moogLfoEnv.release(800);
    tremoloEnv.attack(30);
    tremoloEnv.decay(1800);
    tremoloEnv.sustain(0.6);
    tremoloEnv.release(800);
    dcMoogEnv.amplitude(1);
    noiseDc.amplitude(1);
    whiteNoise.amplitude(0);
    noiseEnv.attack(10);
    noiseEnv.decay(500);
    noiseEnv.sustain(0.0);
    noiseEnv.release(300);
    noiseCutOffEnv.attack(10);
    noiseCutOffEnv.decay(500);
    noiseCutOffEnv.sustain(0.0);
    noiseCutOffEnv.release(300);
    comp.setPreGain_dB(9.0f);          
    comp.setAttack_sec(0.001f);        
    comp.setRelease_sec(0.050f);       
    comp.setThresh_dBFS(-6.0f);     
    comp.setCompressionRatio(6.0f);       
    comp.setPostGain_dB(9.0f);
}

bool AudioEngine::begin() {
  DEBUG_INFO_SOUND("Initializing Audio Shield...");
  delay(300);
  
  // ========== ALLOCATION MÉMOIRE AUDIO ==========
  AudioMemory(90);
  AudioMemory_F32(50);

  delay(100);

  // ========== ACTIVATION DU CODEC AUDIO ==========
  if (!sgtl5000_1.enable()) {
    DEBUG_INFO_SOUND("Audio Shield not found!");
  }
  delay(20);

  // ========== CONFIGURATION INITIALE (ÉVITER LES POPS) ==========
  sgtl5000_1.dacVolumeRamp();  // Rampe DAC
  sgtl5000_1.muteHeadphone();  // Muter pendant l'init
  sgtl5000_1.muteLineout();    // Muter pendant l'init
  delay(1000);

  // ========== INITIALISER TOUS LES OBJETS À AMPLITUDE 0 ==========
  for (int i = 0; i < NUM_TOUCH_PADS; i++) {
    polyWave[i].begin(WAVEFORM_TRIANGLE);
    polyWave[i].amplitude(0.0);
    polyWave[i].frequency(440);
    polyModFM[i].amplitude(0.0);
    polyEnv[i].noteOff();
    polyEnv2[i].noteOff();
  }
  moogEnv.noteOff();
  polyAmp.setGain(0);
  // ========== RAMPE PROGRESSIVE DU VOLUME ==========
  for (float vol = 0.0; vol <= 0.7; vol += 0.02) {
    sgtl5000_1.volume(vol);
    delay(20);
  }

  delay(300);

  // ========== CONFIGURATION FINALE DU CODEC ==========
  sgtl5000_1.volume(0.8);
  sgtl5000_1.unmuteHeadphone();
  sgtl5000_1.unmuteLineout();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(0);
  sgtl5000_1.lineOutLevel(10);

  delay(300);

  // ========== RAMPE FINALE DE L'AMPLI PRINCIPAL ==========
  for (float gain = 0.0; gain <= 1; gain += 0.02) {
    polyAmp.setGain(gain);
    delay(5);
  }

  delay(300);

  // ========== CONFIGURATION DES GAINS PAR DÉFAUT ==========
  setupDefaultGains();

  DEBUG_INFO_SOUND("Audio Shield initialized!");
  return true;
}

void AudioEngine::setupDefaultGains() {
  const float sampleLevel = 1;
  const float polyMixerGain = 0.6;
  const float stringLevel = 0.8;
  const float drumLevel = 0.8;

  // ========== SAMPLE MIXERS ==========
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

  // ========== POLY MIXERS ==========
  for (int i = 0; i < 4; i++) {
    polyMixer1.gain(i, polyMixerGain);
    polyMixer2.gain(i, polyMixerGain);
    polyMixer3.gain(i, polyMixerGain);
  }

  // ========== SOUND MIXERS ==========
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

  // ========== GLOBAL MIXER ==========
  polyMixerGlobal.gain(0, 0.7);
  polyMixerGlobal.gain(1, 0.7);
  polyMixerGlobal.gain(2, 0.7);
  polyMixerGlobal.gain(3, 0.7);

  amp2.gain(1.0);

  // ========== EFFECTS MIXER ==========
  fxMixer.gain(0, 0.6);  // Signal direct
  fxMixer.gain(1, 0.2);  // Tremolo
  fxMixer.gain(2, 0);    // Chorus (off par défaut)
  fxMixer.gain(3, 0.7);  // Samples
  // ========== DRY/WET MIX ==========
  mono2stereo.setSpread(0.5);
  mono2stereo.setPan(0.5);
  polyReverb.size(0.9);
  polyReverb.hidamp(0.3);
  polyReverb.lodamp(0.2);
  polyReverb.lowpass(0.8);
  polyReverb.diffusion(0.95);
  polyReverb.mix(0.7);
  noiseMix.gain(0, 0.8);  // signal direct du polyMixerGlobal
  noiseMix.gain(1, 0);  // signal sortie du noiseFilter 1
  noiseMix.gain(2, 0);  // signal sortie du noiseFilter 2
  noiseMix.gain(3, 0);
}