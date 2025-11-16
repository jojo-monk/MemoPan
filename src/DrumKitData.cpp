#include "DrumKitData.h"
#include "audioSamples/DrumsKit1.h"

// Définition du nombre de drumkits
const int NUM_DRUMKITS = 3;

// Définition du tableau de drumkits
const DrumKitSamples drumKits[NUM_DRUMKITS] = {
    // DrumKit 0
    {
        // samplesLong
        {
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
        },
        // samples100ms
        {
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
        },
        // samples40ms
        {
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
        }
    },
    // DrumKit 1
    {
        {   AudioSampleKick80ms,
            AudioSampleBodhran200ms,
            AudioSampleTuck200ms,
            AudioSampleTack100ms,
            AudioSampleLowperc100ms,
            AudioSampleCabasa140ms,
            AudioSampleLowperca200ms,
            AudioSampleBasque250ms,
            AudioSampleCastagnette10ms,
            AudioSampleCrash1000ms,
            AudioSampleTonk140ms,          
            AudioSampleMaracas250ms
        },
        {   AudioSampleKick80ms,
            AudioSampleBodhran140ms,
            AudioSampleTuck140ms,
            AudioSampleTack100ms,
            AudioSampleLowperc100ms,
            AudioSampleCabasa100ms,
            AudioSampleLowperca140ms,
            AudioSampleBasque140ms,
            AudioSampleCastagnette10ms,
            AudioSampleCrash400ms,
            AudioSampleTonk140ms,
            AudioSampleMaracas140ms, 
        },
        {   AudioSampleKick40ms,
            AudioSampleBodhran40ms,
            AudioSampleTuck40ms,
            AudioSampleTack40ms,
            AudioSampleLowperc40ms,
            AudioSampleCabasa40ms,
            AudioSampleLowperca40ms,
            AudioSampleBasque40ms,
            AudioSampleCastagnette10ms,
            AudioSampleCrash140ms,
            AudioSampleTonk40ms,
            AudioSampleMaracas40ms,
        }
    },
    // DrumKit 2
    {
        {   AudioSampleMusicbox1200msw,
            AudioSampleMusicbox2400msw,
            AudioSampleMusicbox3300msw,
            AudioSampleMusicbox4300msw,
            AudioSampleMusicbox5300msw,
            AudioSampleMusicbox6300msw,
            AudioSampleMusicbox7300msw,
            AudioSampleMusicbox8300msw,
            AudioSampleMusicbox9300msw,
            AudioSampleMusicbox10300msw,
            AudioSampleMusicbox11300msw,
            AudioSampleMusicbox12700msw

        },
        {   AudioSampleMusicbox1100msw,
            AudioSampleMusicbox2200msw,
            AudioSampleMusicbox3200msw,
            AudioSampleMusicbox4200msw,
            AudioSampleMusicbox5200msw,
            AudioSampleMusicbox6200msw,
            AudioSampleMusicbox7200msw,
            AudioSampleMusicbox8200msw,
            AudioSampleMusicbox9200msw,
            AudioSampleMusicbox10200msw,
            AudioSampleMusicbox11200msw,
            AudioSampleMusicbox12500msw,
        },
        {   AudioSampleMusicbox140msw,
            AudioSampleMusicbox2100msw,
            AudioSampleMusicbox3100msw,
            AudioSampleMusicbox4100msw,
            AudioSampleMusicbox5100msw,
            AudioSampleMusicbox6100msw,
            AudioSampleMusicbox7100msw,
            AudioSampleMusicbox8100msw,
            AudioSampleMusicbox9100msw,
            AudioSampleMusicbox10100msw,
            AudioSampleMusicbox11100msw,
            AudioSampleMusicbox12200msw
            
        }

    }
    
};