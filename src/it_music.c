/*
Copyright (C) 2014, Jeffrey Lim. All Rights Reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/

#include "it_struc.h"

// temporary shit to make this work --GM

it_engine *ITEngineNew()
{
	int i;

	it_engine *ite = calloc(sizeof(it_engine), 1);

	// TODO: initialise properly
	for(i = 0; i < 199; i++)
		ite->pat[i] = NULL;
	for(i = 0; i < 99; i++)
		ite->SamplePointer[i] = NULL;

	ite->NumChannels = 256;
	// shifting this here so it doesn't try to use a real unpacked pattern --GM
	ite->CurrentEditPattern = 199;
	ite->PatternNumber = 199;
	ite->CmdLineNumChannels = -1;

	return ite;
}

it_drvdata *drv_oss_init(it_engine *ite);

it_drvdata *DriverSys_GetByName(it_engine *ite, const char *fname)
{
	printf("drv: %s\n", fname);

	if(!strcmp(fname, "oss"))
		return drv_oss_init(ite);
	
	return NULL;
}

it_pattern *PE_GetCurrentPattern(it_engine *ite, uint16_t *patnum, uint16_t *patrow)
{
	*patnum = ite->PatternNumber;
	*patrow = ite->MaxRow + 1;

	return NULL; // actually meant to return UNPACKED pattern data area --GM
}

uint16_t PE_GetLastInstrument(it_engine *ite)
{
	return ite->LastInstrument-1;
}

//
// Functions for playing control
//  Music_PlaySong........ parameters, AX = order
//  Music_Stop............ parameters, None
//  Music_PlayPattern..... parameters, AX = pattern, BX = number of rows, CX = row
//  Music_ToggleChannel... parameters, AX = channel
//  Music_SoloChannel..... parameters, AX = channel
//

const int8_t FineSineData[] = {
	  0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	 24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	 59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	 59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	 45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
	 24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
	  0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
	-24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
	-45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
	-59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
	-64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
	-59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
	-45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
	-24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2,
};

const int8_t FineRampDownData[] = {
	 64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
	 56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48,
	 48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 40,
	 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32,
	 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24,
	 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
	 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,
	  8,  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,
	  0, -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8,
	 -8, -9, -9,-10,-10,-11,-11,-12,-12,-13,-13,-14,-14,-15,-15,-16,
	-16,-17,-17,-18,-18,-19,-19,-20,-20,-21,-21,-22,-22,-23,-23,-24,
	-24,-25,-25,-26,-26,-27,-27,-28,-28,-29,-29,-30,-30,-31,-31,-32,
	-32,-33,-33,-34,-34,-35,-35,-36,-36,-37,-37,-38,-38,-39,-39,-40,
	-40,-41,-41,-42,-42,-43,-43,-44,-44,-45,-45,-46,-46,-47,-47,-48,
	-48,-49,-49,-50,-50,-51,-51,-52,-52,-53,-53,-54,-54,-55,-55,-56,
	-56,-57,-57,-58,-58,-59,-59,-60,-60,-61,-61,-62,-62,-63,-63,-64,
};

const int8_t FineSquareWave[] = {
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

const uint8_t EmptyPattern[] = {
	64, 0, 64, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// Zero globals.

const float PitchDepthConstant               = 98304.0f;


// TODO: Turn these two into proper structs
uint8_t InstrumentHeader[] = {
        'I', 'M', 'P', 'I',
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	60, 128, 32+128,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0xFF, 0xFF, 0xFF,
        0, 0, 1, 0, 2, 0, 3, 0, 4, 0,
        5, 0, 6, 0, 7, 0, 8, 0, 9, 0,
        10, 0, 11, 0, 12, 0, 13, 0, 14, 0,
        15, 0, 16, 0, 17, 0, 18, 0, 19, 0,
        20, 0, 21, 0, 22, 0, 23, 0, 24, 0,
        25, 0, 26, 0, 27, 0, 28, 0, 29, 0,
        30, 0, 31, 0, 32, 0, 33, 0, 34, 0,
        35, 0, 36, 0, 37, 0, 38, 0, 39, 0,
        40, 0, 41, 0, 42, 0, 43, 0, 44, 0,
        45, 0, 46, 0, 47, 0, 48, 0, 49, 0,
        50, 0, 51, 0, 52, 0, 53, 0, 54, 0,
        55, 0, 56, 0, 57, 0, 58, 0, 59, 0,
        60, 0, 61, 0, 62, 0, 63, 0, 64, 0,
        65, 0, 66, 0, 67, 0, 68, 0, 69, 0,
        70, 0, 71, 0, 72, 0, 73, 0, 74, 0,
        75, 0, 76, 0, 77, 0, 78, 0, 79, 0,
        80, 0, 81, 0, 82, 0, 83, 0, 84, 0,
        85, 0, 86, 0, 87, 0, 88, 0, 89, 0,
        90, 0, 91, 0, 92, 0, 93, 0, 94, 0,
        95, 0, 96, 0, 97, 0, 98, 0, 99, 0,
        100, 0, 101, 0, 102, 0, 103, 0, 104, 0,
        105, 0, 106, 0, 107, 0, 108, 0, 109, 0,
        110, 0, 111, 0, 112, 0, 113, 0, 114, 0,
        115, 0, 116, 0, 117, 0, 118, 0, 119, 0,
        0, 2, 0, 0, 0, 0, 64, 0, 0, 64, 100, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 2, 0, 0, 0, 0,  0, 0, 0,  0, 100, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 2, 0, 0, 0, 0,  0, 0, 0,  0, 100, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
};

uint8_t SampleHeader[80] = {
	'I', 'M', 'P', 'S',
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	64, 0, 64,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	(8363&0xFF), (8363>>8), 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
};

uint8_t MIDIPitchSendString[] = {0x65, 0x00, 0x64, 0x00, 0x06};

const char *PrepareSamplesMsg        = "Preparing Samples";
const char *ReverseMsg               = "Left/right outputs reversed";
const char *NoSoundCardMsg           = "   No sound card detected";
const char *MIDIConfigFileName       = "ITMIDI.CFG";

const char *OrderUpdateEnabledMsg    = "Order list unlocked";
const char *OrderUpdateDisabledMsg   = "Order list locked";

const char *UnsoloMsg                = "Solo disabled";
const char *SoloSampleMsg            = "Solo sample \375D";
const char *SoloInstrumentMsg        = "Solo instrument \375D";

#ifdef DEBUG

const char *LoadDriverMessage        = "Loading driver:";
const char *UnableToReadFileMessage  = "Unable to read file";
const char *DetectingMessage         = "Testing driver";
//uint16_t    ScreenOffset             = 0;

#endif

const char *PleaseWaitMsg            = "Please Wait...";

const char *PCSpeakerDriver         = "ITPCSPKR.DRV";
const char *SBDriver                = "ITSB.DRV";
const char *SB2Driver               = "ITSB2.DRV";
const char *SBProDriver             = "ITSBPRO.DRV";
const char *SB16Driver              = "ITSB16.DRV";
const char *AWE32Driver             = "ITAWE32.DRV";
const char *GUSDriver               = "ITGUS.DRV";
const char *InterwaveDriver         = "ITIW.DRV";
const char *PASDriver               = "ITPAS.DRV";
const char *PAS16Driver             = "ITPAS16.DRV";
const char *WSSDriver               = "ITWSS.DRV";
const char *ESSDriver               = "ITES1868.DRV";
const char *MIDIDriver              = "ITMPU401.DRV";
const char *EWSCodecDriver          = "ITEWSCOD.DRV";
const char *VIVOCodecDriver         = "ITVIVO.DRV";
const char *ST97PCICodecDriver      = "ITSTCODE.DRV";
const char *WAVDriver               = "ITWAV.DRV";
const char *MIDDriver               = "ITMID.DRV";
const char *VSoundMMXDriver         = "ITVSOUND.MMX";
const char *VSoundDriver            = "ITVSOUND.DRV";
const char *DefaultDriver           = "ITSOUND.DRV";

#if 0
const char *DriverNameTable[] = {
	NULL,
	"ITPAS16.DRV", "ITSB16.DRV",
	"ITIW.DRV", "ITGUS.DRV",
	"ITAWE32.DRV", "ITSBPRO.DRV",
	"ITSB.DRV", "ITPCSPKR.DRV",
	"ITSB2.DRV", "ITPAS.DRV",
	"ITWAV.DRV", "ITWSS.DRV",
	"ITES1868.DRV", "ITMPU401.DRV",
	"ITEWSCOD.DRV", "ITVIVO.DRV",
	"ITSTCODE.DRV", "ITMID.DRV",
	"ITSOUND.DRV", "ITVSOUND.MMX",
	"ITVSOUND.DRV",
	/*
        PAS16Driver, SB16Driver,
        InterwaveDriver, GUSDriver,
        AWE32Driver, SBProDriver,
        SBDriver, PCSpeakerDriver,
        SB2Driver, PASDriver,
        WAVDriver, WSSDriver,
        ESSDriver, MIDIDriver,
        EWSCodecDriver, VIVOCodecDriver,
        ST97PCICodecDriver, MIDDriver,
        DefaultDriver, VSoundMMXDriver,
        VSoundDriver,
	*/
};

const uint16_t DriverDetectionOrder[] = {
	19, 20, 21, 17, 16, 15, 13, 1, 10, 2, 12, 3, 4, 5, 6, 9, 7, 8, 0xFFFF,
};

const uint16_t DriverSoundCard[] = {
	0, 8, 7, 9, 6,
	2, 5, 4, 3, 10,
	1, 12, 13, 15, 16,
	0, 0, 0, 0, 14,
	11, 18,
	0xFFFF,
};
#else
const char *DriverNameTable[] = {
	NULL,
	"oss",
	// I hope to get SDL audio working at some point.
	// Possibly also a winmm driver for Windows.
	// JACK will definitely be there, too! Will use ezjack for that.
};

const uint16_t DriverDetectionOrder[] = {
	1, 0xFFFF,
};

const uint16_t DriverSoundCard[] = {
	0, 1,
	0xFFFF,
};
#endif

// TODO: translate this properly!
const uint32_t PitchTable[] = {
2048+((0)<<16), 2170+((0)<<16), 2299+((0)<<16), 2435+((0)<<16), 2580+((0)<<16), 2734+((0)<<16),
2896+((0)<<16), 3069+((0)<<16), 3251+((0)<<16), 3444+((0)<<16), 3649+((0)<<16), 3866+((0)<<16),

4096+((0)<<16), 4340+((0)<<16), 4598+((0)<<16), 4871+((0)<<16), 5161+((0)<<16), 5468+((0)<<16),
5793+((0)<<16), 6137+((0)<<16), 6502+((0)<<16), 6889+((0)<<16), 7298+((0)<<16), 7732+((0)<<16),

8192+((0)<<16), 8679+((0)<<16), 9195+((0)<<16), 9742+((0)<<16), 10321+((0)<<16), 10935+((0)<<16),
11585+((0)<<16), 12274+((0)<<16), 13004+((0)<<16), 13777+((0)<<16), 14596+((0)<<16), 15464+((0)<<16),

16384+((0)<<16), 17358+((0)<<16), 18390+((0)<<16), 19484+((0)<<16), 20643+((0)<<16), 21870+((0)<<16),
23170+((0)<<16), 24548+((0)<<16), 26008+((0)<<16), 27554+((0)<<16), 29193+((0)<<16), 30929+((0)<<16),

32768+((0)<<16), 34716+((0)<<16), 36781+((0)<<16), 38968+((0)<<16), 41285+((0)<<16), 43740+((0)<<16),
46341+((0)<<16), 49097+((0)<<16), 52016+((0)<<16), 55109+((0)<<16), 58386+((0)<<16), 61858+((0)<<16),

0+((1)<<16), 3897+((1)<<16), 8026+((1)<<16), 12400+((1)<<16), 17034+((1)<<16), 21944+((1)<<16),
27146+((1)<<16), 32657+((1)<<16), 38496+((1)<<16), 44682+((1)<<16), 51236+((1)<<16), 58179+((1)<<16),

0+((2)<<16), 7794+((2)<<16), 16051+((2)<<16), 24800+((2)<<16), 34068+((2)<<16), 43888+((2)<<16),
54292+((2)<<16), 65314+((2)<<16), 11456+((3)<<16), 23828+((3)<<16), 36936+((3)<<16), 50823+((3)<<16),

0+((4)<<16), 15588+((4)<<16), 32103+((4)<<16), 49600+((4)<<16), 2601+((5)<<16), 22240+((5)<<16),
43048+((5)<<16), 65092+((5)<<16), 22912+((6)<<16), 47656+((6)<<16), 8336+((7)<<16), 36110+((7)<<16),

0+((8)<<16), 31176+((8)<<16), 64205+((8)<<16), 33663+((9)<<16), 5201+((10)<<16), 44481+((10)<<16),
20559+((11)<<16), 64648+((11)<<16), 45823+((12)<<16), 29776+((13)<<16), 16671+((14)<<16), 6684+((15)<<16),

0+((16)<<16), 62352+((16)<<16), 62875+((17)<<16), 1790+((19)<<16), 10403+((20)<<16), 23425+((21)<<16),
41118+((22)<<16), 63761+((23)<<16), 26111+((25)<<16), 59552+((26)<<16), 33342+((28)<<16), 13368+((30)<<16),

// Pitch extention for loading some XIs,

0+((32)<<16), 59167+((33)<<16), 60214+((35)<<16), 3580+((38)<<16), 20806+((40)<<16), 46850+((42)<<16),
16701+((45)<<16), 61986+((47)<<16), 52221+((50)<<16), 53567+((53)<<16), 1148+((57)<<16), 26736+((60)<<16),
};

#if USEFPUCODE

uint8_t FPSave[128]; // note, init this to 0

const float Const_14317456 = 14317456.0;
const float Const1_On_768 = 1.0/768.0; //Const1_On_768   DD      3AAAAAABh
uint16_t SlideValue = 0;
uint16_t NewControlWord = 0x7F;

#else

const uint16_t FineLinearSlideUpTable16[] = {
	0, 1,     59, 1,    118, 1,   178, 1,   237, 1,    // 0->4
	296, 1,   356, 1,   415, 1,   475, 1,   535, 1,    // 5->9
	594, 1,   654, 1,   714, 1,   773, 1,   833, 1,    // 10->14
	893, 1,                                            // 15
};
const uint32_t *FineLinearSlideUpTable = (const uint32_t *)FineLinearSlideUpTable16;

const uint16_t LinearSlideUpTable16[] = { // Value = 2^(Val/192)
	0,     1, 237,   1, 475,   1, 714,   1, 953,  1,  // 0->4
	1194,  1, 1435,  1, 1677,  1, 1920,  1, 2164, 1,  // 5->9
	2409,  1, 2655,  1, 2902,  1, 3149,  1, 3397, 1,  // 10->14
	3647,  1, 3897,  1, 4148,  1, 4400,  1, 4653, 1,  // 15->19
	4907,  1, 5157,  1, 5417,  1, 5674,  1, 5932, 1,  // 20->24
	6190,  1, 6449,  1, 6710,  1, 6971,  1, 7233, 1,  // 25->29
	7496,  1, 7761,  1, 8026,  1, 8292,  1, 8559, 1,  // 30->34
	8027,  1, 9096,  1, 9366,  1, 9636,  1, 9908, 1,  // 35->39
	10181, 1, 10455, 1, 10730, 1, 11006, 1, 11283,1,  // 40->44
	11560, 1, 11839, 1, 12119, 1, 12400, 1, 12682,1,  // 45->49
	12965, 1, 13249, 1, 13533, 1, 13819, 1, 14106,1,  // 50->54
	14394, 1, 14684, 1, 14974, 1, 15265, 1, 15557,1,  // 55->59
	15850, 1, 16145, 1, 16440, 1, 16737, 1, 17034,1,  // 60->64
	17333, 1, 17633, 1, 17933, 1, 18235, 1, 18538,1,  // 65->69
	18842, 1, 19147, 1, 19454, 1, 19761, 1, 20070,1,  // 70->74
	20379, 1, 20690, 1, 21002, 1, 21315, 1, 21629,1,  // 75->79
	21944, 1, 22260, 1, 22578, 1, 22897, 1, 23216,1,  // 80->84
	23537, 1, 23860, 1, 24183, 1, 24507, 1, 24833,1,  // 85->89
	25160, 1, 25488, 1, 25817, 1, 26148, 1, 26479,1,  // 90->94
	26812, 1, 27146, 1, 27481, 1, 27818, 1, 28155,1,  // 95->99
	28494, 1, 28834, 1, 29175, 1, 29518, 1, 29862,1,  // 100->104
	30207, 1, 30553, 1, 30900, 1, 31248, 1, 31599,1,  // 105->109
	31951, 1, 32303, 1, 32657, 1, 33012, 1, 33369,1,  // 110->114
	33726, 1, 34085, 1, 34446, 1, 34807, 1, 35170,1,  // 115->119
	35534, 1, 35900, 1, 36267, 1, 36635, 1, 37004,1,  // 120->124
	37375, 1, 37747, 1, 38121, 1, 38496, 1, 38872,1,  // 125->129
	39250, 1, 39629, 1, 40009, 1, 40391, 1, 40774,1,  // 130->134
	41158, 1, 41544, 1, 41932, 1, 42320, 1, 42710,1,  // 135->139
	43102, 1, 43495, 1, 43889, 1, 44285, 1, 44682,1,  // 140->144
	45081, 1, 45481, 1, 45882, 1, 46285, 1, 46690,1,  // 145->149
	47095, 1, 47503, 1, 47917, 1, 48322, 1, 48734,1,  // 150->154
	49147, 1, 49562, 1, 49978, 1, 50396, 1, 50815,1,  // 155->159
	51236, 1, 51658, 1, 52082, 1, 52507, 1, 52934,1,  // 160->164
	53363, 1, 53793, 1, 54224, 1, 54658, 1, 55092,1,  // 165->169
	55529, 1, 55966, 1, 56406, 1, 56847, 1, 57289,1,  // 170->174
	57734, 1, 58179, 1, 58627, 1, 59076, 1, 59527,1,  // 175->179
	59979, 1, 60433, 1, 60889, 1, 61346, 1, 61805,1,  // 180->184
	62265, 1, 62727, 1, 63191, 1, 63657, 1, 64124,1,  // 185->189
	64593, 1, 65064, 1, 0,     2, 474,   2, 950,  2,  // 190->194
	1427,  2, 1906,  2, 2387,  2, 2870,  2, 3355, 2,  // 195->199
	3841,  2, 4327,  2, 4818,  2, 5310,  2, 5803, 2,  // 200->204
	6298,  2, 6795,  2, 7294,  2, 7794,  2, 8296, 2,  // 205->209
	8800,  2, 9306,  2, 9814,  2, 10323, 2, 10835,2,  // 210->214
	11348, 2, 11863, 2, 12380, 2, 12899, 2, 13419,2,  // 215->219
	13942, 2, 14467, 2, 14993, 2, 15521, 2, 16051,2,  // 220->224
	16583, 2, 17117, 2, 17653, 2, 18191, 2, 18731,2,  // 225->229
	19273, 2, 19817, 2, 20362, 2, 20910, 2, 21460,2,  // 230->234
	22011, 2, 22565, 2, 23121, 2, 23678, 2, 24238,2,  // 235->239
	24800, 2, 25363, 2, 25929, 2, 25497, 2, 27067,2,  // 240->244
	27639, 2, 28213, 2, 28789, 2, 29367, 2, 29947,2,  // 245->249
	30530, 2, 31114, 2, 31701, 2, 32289, 2, 32880,2,  // 250->254
	33473, 2, 34068, 2,                               // 255->256
};
const uint32_t *LinearSlideUpTable = (const uint32_t *)LinearSlideUpTable16;

const uint16_t FineLinearSlideDownTable[] = {
	65535, 65477, 65418, 65359, 65300, 65241, 65182, 65359, // 0->7
	65065, 65006, 64947, 64888, 64830, 64772, 64713, 64645, // 8->15
};

const uint16_t LinearSlideDownTable[] = {
	65535, 65300, 65065, 64830, 64596, 64364, 64132, 63901, // 0->7
	63670, 63441, 63212, 62984, 62757, 62531, 62306, 62081, // 8->15
	61858, 61635, 61413, 61191, 60971, 60751, 60532, 60314, // 16->23
	60097, 59880, 59664, 59449, 59235, 59022, 58809, 58597, // 24->31
	58386, 58176, 57966, 57757, 57549, 57341, 57135, 56929, // 32->39
	56724, 56519, 56316, 56113, 55911, 55709, 55508, 55308, // 40->47
	55109, 54910, 54713, 54515, 54319, 54123, 53928, 53734, // 48->55
	53540, 53347, 53155, 52963, 52773, 52582, 52393, 52204, // 56->63
	52016, 51829, 51642, 51456, 51270, 51085, 50901, 50718, // 64->71
	50535, 50353, 50172, 49991, 49811, 49631, 49452, 49274, // 72->79
	49097, 48920, 48743, 48568, 48393, 48128, 48044, 47871, // 80->87
	47699, 47527, 47356, 47185, 47015, 46846, 46677, 46509, // 88->95
	46341, 46174, 46008, 45842, 45677, 45512, 45348, 45185, // 96->103
	45022, 44859, 44698, 44537, 44376, 44216, 44057, 43898, //104->111
	43740, 43582, 43425, 43269, 43113, 42958, 42803, 42649, //112->119
	42495, 42342, 42189, 42037, 41886, 41735, 41584, 41434, //120->127
	41285, 41136, 40988, 40840, 40639, 40566, 40400, 40253, //128->135
	40110, 39965, 39821, 39678, 39535, 39392, 39250, 39109, //136->143
	38968, 38828, 38688, 38548, 38409, 38271, 38133, 37996, //144->151
	37859, 37722, 37586, 37451, 37316, 37181, 37047, 36914, //152->159
	36781, 36648, 36516, 36385, 36254, 36123, 35993, 35863, //160->167
	35734, 35605, 35477, 35349, 35221, 35095, 34968, 34842, //168->175
	34716, 34591, 34467, 34343, 34219, 34095, 33973, 33850, //176->183
	33728, 33607, 33486, 33365, 33245, 33125, 33005, 32887, //184->191
	32768, 32650, 32532, 32415, 32298, 32182, 32066, 31950, //192->199
	31835, 31720, 31606, 31492, 31379, 31266, 31153, 31041, //200->207
	30929, 30817, 30706, 30596, 30485, 30376, 30226, 30157, //208->215
	30048, 29940, 29832, 29725, 29618, 29511, 29405, 29299, //216->223
	29193, 29088, 28983, 28879, 28774, 28671, 28567, 28464, //224->231
	28362, 28260, 28158, 28056, 27955, 27855, 27754, 27654, //232->239
	27554, 27455, 27356, 27258, 27159, 27062, 26964, 26867, //240->247
	26770, 26674, 26577, 26482, 26386, 26291, 26196, 26102, //248->255
	26008,                                                  // 256
};

#endif /* USEFPUCODE */

void (*InitCommandTable[])(it_engine *ite, it_host *chn) = {
	InitNoCommand, InitCommandA,
	InitCommandB, InitCommandC,
	InitCommandD, InitCommandE,
	InitCommandF, InitCommandG,
	InitCommandH, InitCommandI,
	InitCommandJ, InitCommandK,
	InitCommandL, InitCommandM,
	InitCommandN, InitCommandO,
	InitCommandP, InitCommandQ,
	InitCommandR, InitCommandS,
	InitCommandT, InitCommandU,
	InitCommandV, InitCommandW,
	InitCommandX, InitCommandY,
	InitCommandZ, InitNoCommand,
	InitNoCommand, InitNoCommand,
	InitNoCommand, InitNoCommand,
};

void (*CommandTable[])(it_engine *ite, it_host *chn) = {
	NoCommand, NoCommand,
	NoCommand, NoCommand,
	CommandD, CommandE,
	CommandF, CommandG,
	CommandH, CommandI,
	CommandJ, CommandK,
	CommandL, NoCommand,
	CommandN, NoCommand,
	CommandP, CommandQ,
	CommandR, CommandS,
	CommandT, CommandH,
	NoCommand, CommandW,
	NoCommand, CommandY,
	NoCommand, NoCommand,
	NoCommand, NoCommand,
};

void (*VolumeEffectTable[])(it_engine *ite, it_host *chn) = {
	NoCommand, NoCommand, // Last 2 of command table + VolumeComA and VolumeComB
	VolumeCommandC, VolumeCommandD,
	VolumeCommandE, VolumeCommandF,
	VolumeCommandG, CommandH,
};


/*
RetrigOffsets           Label
CommandQ_0, CommandQ_1, CommandQ_2, CommandQ_3,
CommandQ_4, CommandQ_5, CommandQ_6, CommandQ_7,
CommandQ_8, CommandQ_9, CommandQ_A, CommandQ_B,
CommandQ_C, CommandQ_D, CommandQ_E, CommandQ_F,
*/

//
// Sound Driver Data
//

// TODO: Replace this with a dlopen()/LoadLibrary() system

#if OLDDRIVER
const char *DriverID = "Impulse Tracker Sound Driver";
#else
const char *DriverID = "Impulse Tracker Advanced Sound Driver";
#endif

// *******************

//
// Functions
//

// Command/Effect (call it what you like) information here!!
//
//  For initialisation, DS:DI points to host channel data.
//    Registers for use: All except DS:DI & ES (points to SongDataSegment)
//
//  For update, DS:DI points to host channel data.
//    Registers for use: AX, BX, DX, ES, SI

void RecalculateAllVolumes(it_engine *ite)
{
	int i;

	for(i = 0; i < ite->NumChannels; i++)
	{
		it_slave *slave = &ite->slave[i];
		slave->Flags |= 18;
	}
}

void InitPlayInstrument(it_engine *ite, it_host *chn, it_slave *slave, int bx) // BX = instrument offset
{
	slave->InsOffs = bx; // InsOffset
	it_instrument *ins = &ite->ins[bx];

	slave->NNA = ins->NNA;
	slave->DCT = ins->DCT;
	slave->DCA = ins->DCA;

	// MCh and MPr
	if(chn->MCh != 0)
	{
		slave->MCh = chn->MCh;
		slave->MPr = chn->MPr;
		slave->FCut = (ins->MIDIBnk & 0xFF);
		slave->FRes = (ins->MIDIBnk >> 8);
		slave->LpD = chn->Nte; // For MIDI, LpD = Pattern note
	}

	slave->CVl = chn->CV;
	uint8_t dl = chn->CP;

	if((ins->DfP & 0x80) == 0)
		dl = ins->DfP;

	// Check for sample pan
	if(chn->Smp != 0)
	{
		it_sample *smp = &ite->smp[chn->Smp];

		if((smp->DfP & 0x80) != 0)
			dl = smp->DfP & 0x7F;
	}

	int32_t ax = dl;
	if(dl != 100)
	{
		// TODO: Verify this part
		ax = (int32_t)chn->Nte - (int32_t)ins->PPC;
		ax *= (int32_t)ins->PPS;
		ax >>= 3;
		ax += dl;

		if(ax < 0) 
			ax = 0;
		else if(ax > 64)
			ax = 64;
	}

	// Write panning
	slave->Pan = slave->PS = ax;

	// Envelope init
	slave->V.EnvelopeValue = 0x400000; // 64*65536
	slave->V.EnvPos = 0;
	slave->V.CurEnN = 0;
	slave->P.EnvelopeValue = 0;
	slave->P.EnvPos = 0;
	slave->P.CurEnN = 0;
	slave->Pt.EnvelopeValue = 0;
	slave->Pt.EnvPos = 0;
	slave->Pt.CurEnN = 0;

	uint16_t etrigger = 0;
	etrigger |= ((ins->PitchEnv.Flg&1)<<14);
	etrigger |= ((ins->PanEnv.Flg&1)<<13);
	etrigger |= ((ins->VolEnv.Flg&1)<<12);
	etrigger |= 0x133;
	slave->Flags = etrigger;

	uint16_t lsc = ite->LastSlaveChannel;
	if(lsc != 0)
	{
		it_slave *lslave = &ite->slave[lsc-1];

		if((ins->VolEnv.Flg & 9) == 9)
		{
			// Transfer volume data
			slave->V.EnvelopeValue = lslave->V.EnvelopeValue;
			slave->V.EnvelopeDelta = lslave->V.EnvelopeDelta;
			slave->V.EnvPos = lslave->V.EnvPos;
			slave->V.CurEnN = lslave->V.CurEnN;
			slave->V.NextET = lslave->V.NextET;
		}

		if((ins->PanEnv.Flg & 9) == 9)
		{
			// Transfer pan data
			slave->P.EnvelopeValue = lslave->P.EnvelopeValue;
			slave->P.EnvelopeDelta = lslave->P.EnvelopeDelta;
			slave->P.EnvPos = lslave->P.EnvPos;
			slave->P.CurEnN = lslave->P.CurEnN;
			slave->P.NextET = lslave->P.NextET;
		}

		if((ins->PitchEnv.Flg & 9) == 9)
		{
			// Transfer pitch data
			slave->Pt.EnvelopeValue = lslave->Pt.EnvelopeValue;
			slave->Pt.EnvelopeDelta = lslave->Pt.EnvelopeDelta;
			slave->Pt.EnvPos = lslave->Pt.EnvPos;
			slave->Pt.CurEnN = lslave->Pt.CurEnN;
			slave->Pt.NextET = lslave->Pt.NextET;
		}
	}

	// Apply random volume/pan
	chn->Flags |= 0x80;

	if(chn->MCh != 0)
		return;

	slave->FCut = 0xFF;
	slave->FRes = 0x00;

	// If IFC bit 7 == 1, then set filter cutoff
	if((ins->IFC & 0x80) != 0)
	{
		// slave->FCut = (ins->IFC & 0x7F);
		SetFilterCutoff(ite, slave, ins->IFC & 0x7F);
	}

	// If IFR bit 7 == 1, then set filter resonance
	if((ins->IFR & 0x80) != 0)
	{
		slave->FRes = (ins->IFR & 0x7F);
		SetFilterResonance(ite, slave, slave->FRes & 0x7F);
	}
}

void ApplyRandomValues(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	it_instrument *ins = &ite->ins[slave->InsOffs];
	
	chn->Flags &= ~0x80;

	int8_t al = Random(ite); // AL = -128->+127

	if(ins->RV != 0) // Random volume, 0->100
	{
		int32_t ax = ((int32_t)al) * (int32_t)(int8_t)ins->RV; // AX = -12800->12700
		ax >>= 6; // AX = -200->+198(.4)
		ax++; // AX = -199->+199

		int32_t dx = (int32_t)(uint32_t)(uint8_t)slave->SVl; // Sample volume set
		ax *= dx; // AX = -199*128->199*128, -25472->25472
		ax /= 199; // AX = -128->+128
		ax += slave->SVl;

		if(ax < 0)
			slave->SVl = 0;
		else if(ax > 128)
			slave->SVl = 128;
		else
			slave->SVl = ax;
	}

	al = Random(ite);

	if(ins->RP != 0) // Random pan, 0->64
	{
		int32_t ax = ((int32_t)al) * (int32_t)(int8_t)ins->RP;
		// AX = -64*128->64*127, -8192->8128
		ax >>= 7;

		if(slave->Pan == 100)
			return;

		ax += slave->Pan;
		if(ax < 0)
			slave->Pan = slave->PS = 0;
		else if(ax > 64)
			slave->Pan = slave->PS = 64;
		else
			slave->Pan = slave->PS = ax;
	}
}

void MIDISendFilter(it_engine *ite, it_host *chn, uint8_t al)
{
	if((ite->d.DriverFlags & 1) == 0)
		return;

	if((al & 0x80) != 0 && (al < 0xF0))
	{
		if(al == ite->LastMIDIByte)
			return;

		ite->LastMIDIByte = al;
	}

	ite->d.DriverMIDIOut(ite, al);
}

void SetFilterCutoff(it_engine *ite, it_slave *slave, uint8_t bl)
{
	// Given BL = filtervalue.
	// Assumes that channel is non-disowned

	it_host *chn = &ite->chn[slave->HCOffst];

	MIDISendFilter(ite, chn, 0xF0);
	MIDISendFilter(ite, chn, 0xF0);
	MIDISendFilter(ite, chn, 0x00);
	MIDISendFilter(ite, chn, bl);
}

void SetFilterResonance(it_engine *ite, it_slave *slave, uint8_t bl)
{
	// Given BL = filtervalue.
	// Assumes that channel is non-disowned

	it_host *chn = &ite->chn[slave->HCOffst];

	MIDISendFilter(ite, chn, 0xF0);
	MIDISendFilter(ite, chn, 0xF0);
	MIDISendFilter(ite, chn, 0x01);
	MIDISendFilter(ite, chn, bl);
}

void MIDITranslate(it_engine *ite, it_host *chn, it_slave *slave, uint16_t bx)
{
	int i;

	// Assumes DS:SI points to slave
	// And DS:DI points to host.
	// BX = offset of MIDI command to interpret

	if((ite->d.DriverFlags & 1) == 0)
		return;

	if(bx >= 0xF000)
	{
		// Internal MIDI commands.

		if((ite->hdr.Flags & 64) == 0)
			return;

		// Pitch wheel
		// Formula is: Depth = 16384*12 / PitchWheelDepth * log2(Freq / OldFreq)
		// Do calculation, check whether pitch needs to be sent.

		int32_t pwd = (uint8_t)ite->hdr.PWD;
		if(pwd == 0) // No depth!
			return;

		float st0 = PitchDepthConstant / (float)pwd;

		// Current pitch / Original pitch
		float st1 = ((float)slave->Frequency) / (float)(slave->RVol_MIDIFSet);
		pwd *= (int32_t)log2f(st1);

		// OK.. [ChannelCountTable] contains pitch depth.
		// Have to check:
		//  a) Within ranges?
		//  b) Comparison to old pitch for this channel

		uint16_t ax = 1;
		uint8_t cl = slave->MCh-1;
		if((cl & 0x80) != 0)
			return;

		if(pwd != 0)
		{
			ax <<= (uint16_t)cl;
			if((ite->MIDIPitchDepthSent & ax) == 0)
			{
				ite->MIDIPitchDepthSent |= ax;

				// Send MIDI Pitch depth stuff
				MIDISendFilter(ite, chn, 0xB0 | cl);

				for(i = 0; i < 5; i++)
					MIDISendFilter(ite, chn, MIDIPitchSendString[i]);

				MIDISendFilter(ite, chn, ite->hdr.PWD);
			}
		}

		pwd += 0x2000;

		if((pwd & 0x8000) != 0)
			pwd = 0;
		if(pwd >= 0x4000)
			pwd = 0x3FFF;

		if(ite->MIDIPitch[slave->MCh-1] == pwd)
			return;

		ite->MIDIPitch[slave->MCh-1] = pwd;

		// Output pitch change

		MIDISendFilter(ite, chn, 0xE0 | cl); // Ec command
		MIDISendFilter(ite, chn, pwd & 0x7F);
		MIDISendFilter(ite, chn, (pwd>>7) & 0x7F);

		return;
	}

	// Now for user input MIDI stuff.

	// not a big priority right now --GM
#if 0
MIDITranslateParameterised:
	Push    FS

	Xor     AX, AX
	Xor     CX, CX
	Mov     FS, MIDIDataArea

MIDITranslate1:
	Mov     AH, [FS:BX]
	Inc     BX
	Test    AH, AH
	JZ      MIDITranslate2

	Cmp     AH, ' '                 ; Interpretation time.
	JNE     MIDITranslateNoSpace

	Test    CX, CX
	JZ      MIDITranslate1
	Jmp     MIDITranslateSend

MIDITranslateNoSpace:
	Sub     AH, '0'
	JC      MIDITranslate1
	Cmp     AH, 9
	JA      MIDITranslateValue1

	ShL     AL, 4
	Or      AL, AH
	Inc     CX
	Jmp     MIDITranslateValueEnd

MIDITranslateValue1:
	Sub     AH, 'A'-'0'
	JC      MIDITranslate1
	Cmp     AH, 'F'-'A'
	JA      MIDITranslateValue2

	ShL     AL, 4
	Add     AH, 10
	Or      AL, AH
	Inc     CX
	Jmp     MIDITranslateValueEnd

MIDITranslateValue2:
	Sub     AH, 'a'-'A'
	JC      MIDITranslate1
	Cmp     AH, 'z'-'a'
	JA      MIDITranslate1

	Cmp     AH, 'c'-'a'
	JNE     MIDITranslateValue3

	Test    SI, SI
	JZ      MIDITranslate1

;                Mov     AH, [DI+0Ch]
	Mov     AH, [SI+3Ch]
	ShL     AL, 4
	Dec     AH
	Or      AL, AH
	Inc     CX
	Jmp     MIDITranslateValueEnd

MIDITranslateValue3:
	Test    CX, CX
	JZ      MIDITranslateValue4

	Call    MIDISendFilter

	Xor     CX, CX

MIDITranslateValue4:
	Mov     AL, [DI+7]      ; Effect.
	Cmp     AH, 'z'-'a'     ; Zxx?
	JE      MIDITranslateSend

	Mov     AL, [DI+12h]
	Cmp     AH, 'o'-'a'
	JE      MIDITranslateSend

	Test    SI, SI
	JZ      MIDITranslate1

	Mov     AL, [SI+32h]    ; [DI+0Eh]
	Cmp     AH, 'n'-'a'     ; Note?
	JE      MIDITranslateSend

	Mov     AL, [SI+0Bh]
	Cmp     AH, 'm'-'a'
	JE      MIDITranslateSend

	Cmp     AH, 'v'-'a'     ; Velocity?
	JNE     MIDITranslateValue7

	Xor     AL, AL
	Test    Word Ptr [SI], 800h
	JNZ     MIDITranslateSend

	Mov     AL, [SI+22h]            ; 0->2^6
	Xor     DX, DX
	Mul     GlobalVolume            ; 0->2^13
	Mov     DL, [SI+23h]            ; Channel volume
	Mul     DX                      ; 0->2^19
	SHRD    AX, DX, 4               ; 0->2^15
	Mov     DL, [SI+24h]            ; Sample & Instrument Volume
	Mul     DX                      ; 0->2^22
	SHRD    AX, DX, 15              ; 0->2^7
	Sub     AL, 1
	AdC     AL, 1                   ; 1->2^7
	JNS     MIDITranslateSend
	Dec     AX
;                Mov     AL, 7Fh
	Jmp     MIDITranslateSend

Comment ~
	Mov     AL, [SI+22h]    ; 0->64
	Add     AL, AL          ; 0->128
	Sub     AL, 1
	AdC     AL, 1           ; 1->128
	Cmp     AL, 128
	JB      MIDITranslateSend
	Dec     AX
	Jmp     MIDITranslateSend
~

MIDITranslateValue7:
	Cmp     AH, 'u'-'a'     ; Volume?
	JNE     MIDITranslateValue8

	Xor     AL, AL
	Test    Word Ptr [SI], 800h
	JNZ     MIDITranslateSend

	Mov     AL, [SI+20h]    ; 0->128
	Sub     AL, 1
	AdC     AL, 1           ; 1->128
	Cmp     AL, 128
	JB      MIDITranslateSend
	Dec     AX
	Jmp     MIDITranslateSend

MIDITranslateValue8:
	Mov     AL, [SI+3Ah]    ; HCN
	And     AL, 7Fh
	Cmp     AH, 'h'-'a'
	JE      MIDITranslateSend

	Mov     AL, [SI+2Ah]    ; Pan set
	Cmp     AH, 'x'-'a'
	JE      MIDITranslatePanValue

	Mov     AL, [SI+25h]    ; Final pan
	Cmp     AH, 'y'-'a'
	JE      MIDITranslatePanValue

	Mov     AL, [SI+3Dh]
	Cmp     AH, 'p'-'a'     ; Program?
	JE      MIDITranslateSend

	Mov     DX, [SI+3Eh]
	Mov     AL, DL
	Add     AL, 1
	AdC     AX, 0
	Dec     AX
	Cmp     AH, 'b'-'a'
	JE      MIDITranslateSend

	Mov     AL, DH
	Add     AL, 1
	AdC     AX, 0
	Dec     AX
	Cmp     AH, 'a'-'a'
	JE      MIDITranslateSend

	Xor     AX, AX
	Jmp     MIDITranslate1

MIDITranslatePanValue:
	Add     AL, AL
	Cmp     AL, 7Fh
	JBE     MIDITranslateSend
	Dec     AX
	Cmp     AL, 7Fh
	JBE     MIDITranslateSend
	Mov     AL, 40h
	Jmp     MIDITranslateSend

MIDITranslateValueEnd:
	Cmp     CL, 2
	JB      MIDITranslate1

MIDITranslateSend:
	Call    MIDISendFilter

	Xor     AX, AX
	Xor     CX, CX
	Jmp     MIDITranslate1

MIDITranslate2:
	Test    CX, CX
	JZ      MIDITranslate3

	Call    MIDISendFilter

MIDITranslate3:
	Pop     FS
	PopA

MIDITranslateEnd:
	Ret

EndP            MIDITranslate
	Assume DS:Nothing
#endif
}

it_slave *AllocateChannel15(it_engine *ite, it_host *chn, uint8_t *ch)
{
	// Sample handler
	uint16_t hcn = chn->HCN;
	it_slave *slave = &ite->slave[hcn];

	if((ite->d.DriverFlags & 2) != 0 && (slave->Flags & 1) != 0) // Hi-Qual driver + Channel on?
	{
		// copy out channel
		slave->Flags |= 0x200;
		slave->HCN |= 0x80;
		memcpy(slave + 64, slave, sizeof(it_slave));
	}

	chn->SCOffst = hcn;
	slave->HCOffst = slave->HCN = (chn - ite->chn);
	slave->Flags = 0x0133; // Recalc freq,vol&pan Channel on.

	slave->CVl = chn->CV;
	slave->Pan = slave->PS = chn->CP;

	// Get sample offset.
	// ^ v ^ v one of these doesn't seem to fit --GM
	// General stuff.

	slave->FadeOut = 0x0400;
	slave->V.EnvelopeValue = 64<<16;
	slave->FCut = 0xFF;

	slave->Nte = chn->Nte;
	slave->Ins = chn->Ins;

	if(chn->Smp == 0)
	{
		slave->Flags = 0x0200;
		*ch &= ~4;
		return NULL;
	}

	I_TagSample(ite, chn->Smp-1); // TODO: work out what this does!
	slave->Smp = chn->Smp-1;
	slave->SmpOffs = chn->Smp-1;
	it_sample *smp = &ite->smp[chn->Smp-1];

	// Reset vibrato info.
	slave->Bit = 0;
	slave->ViP = 0;
	slave->ViDepth = 0;

	slave->P.EnvelopeValue &= 0xFFFF; // No pan deviation
	slave->Pt.EnvelopeValue &= 0xFFFF; // No pitch deviation
	slave->LpD = 0; // Reset loop dirn

	if(smp->Length == 0 || (smp->Flg & 1) == 0)
	{
		// No sample!
		slave->Flags = 0x0200;
		*ch &= ~4;
		return NULL;
	}

	slave->Bit = (smp->Flg & 2);
	slave->SVl = smp->GvL<<1;

	return slave;

}

it_slave *AllocateChannel(it_engine *ite, it_host *chn, uint8_t *ch)
{
	// Returns SI. Carry set if problems
	// (actually returns NULL here --GM)

	// TODO: de-goto-ise this.

	it_slave *slave = NULL;

	ite->LastSlaveChannel = 0;

	//printf("alloc slave\n");
	if((ite->hdr.Flags & 4) == 0)
		return AllocateChannel15(ite, chn, ch);

	// Instrument handler!
	// ^ NOTE TO PORTERS:
	//   Make sure you are in a room with no sharp objects, medicines, or knobs
	//   to hang ropes from. It's a lot of "WHAT IS MALLOC???" code. --GM

	ite->AllocateSlaveOffset = 0;
	ite->AllocateNumChannels = ite->NumChannels;

	if(chn->Smp == 101 && ite->NumChannels != MAXSLAVECHANNELS)
	{
		// CX = number of channels remaining
		// ^ but the code uses DX and BX, not CX --GM
		ite->AllocateNumChannels = (MAXSLAVECHANNELS - ite->NumChannels);
		ite->AllocateSlaveOffset = ite->NumChannels;
	}

	//printf("%i %i\n", ite->AllocateNumChannels, ite->AllocateSlaveOffset);

	if(chn->Ins == 0xFF)
		return AllocateChannel15(ite, chn, ch);

	if(chn->Ins == 0)
		return NULL;

	I_TagInstrument(ite, chn->Ins);

	it_instrument *ins = &ite->ins[chn->Ins-1];

	if((*ch & 0x04) == 0) // if((chn->Flags & 0x04) == 0)
		goto AllocateChannel8;

	// New note action handling...
	slave = &ite->slave[chn->SCOffst];
	if(slave->InsOffs == chn->Ins)
		ite->LastSlaveChannel = chn->SCOffst + 1;

	if(slave->NNA == 0)
		goto AllocateChannel8; // Notecut.
	
	printf("FUCK\n"); // Not properly handled

	// Disown channel
	slave->HCN |= 0x80;

AllocateHandleNNA:
	// Is volume set = 0?
	if(slave->VS  == 0) goto AllocateChannel20;
	if(slave->CVl == 0) goto AllocateChannel20;
	if(slave->SVl == 0) goto AllocateChannel20;

	if(slave->NNA > 2)
	{
		// AL = 3 -> Fade
		slave->Flags |= 8; // Fade flag.
	} else if(slave->NNA == 2) {
		// Note off.
		slave->Flags |= 4; // Note off..
		GetLoopInformation(ite, slave);
	} else {
		// Note continue
	}

	goto AllocateChannel8;

AllocateChannel20:
	if(1){}
	uint8_t dl = 0;
	uint8_t dh = 0;
	uint16_t bp = 0;
	uint8_t ah = 0;
	uint16_t cx = 0;

	if(slave->Smp == 100) // MIDI?
	{
		slave->Flags |= 0x0200;
		slave->HCN |= 0x80; // Disown channel

		if(chn->Smp != 101)
			goto AllocateChannel4;

	AllocateChannelMIDIDC:
		slave = &ite->slave[ite->AllocateSlaveOffset];
		cx = ite->AllocateNumChannels;
		//Mov     SI, AllocateSlaveOffset
		//Mov     CX, AllocateNumChannels

		dl = chn->Nt2;
		dh = chn->Ins;
		bp = 0x32;
		ah = chn->MCh;
		*ch = ins->DCA;
		goto AllocateChannel6;
	}

AllocateChannel20Samples:
	if((ite->d.DriverFlags & 2) != 0)
		goto AllocateChannelHiQual;

	uint8_t al = ins->DCT;
	slave->Flags = 0x200;
	if(al == 0)
		goto AllocateChannelInstrument;

	goto AllocateChannel11;

AllocateChannelHiQual:
	slave->Flags |= 0x200;
	slave->HCN |= 0x80; // Disown channel
	goto AllocateChannel4;

AllocateChannel8:
	if(chn->Smp == 101)
		goto AllocateChannelMIDIDC;

	al = ins->DCT;
	if(al == 0)
		goto AllocateChannel4; // Duplicate check off.

AllocateChannel11:
	// Duplicate check...
	slave = &ite->slave[ite->AllocateSlaveOffset];
	cx = ite->AllocateNumChannels;
	//Mov     SI, AllocateSlaveOffset
	//Mov     CX, AllocateNumChannels

	dl = chn->Nte;
	dh = chn->Ins;
	bp = 0x32;
	if(al == 1)
		goto AllocateDCT;

	// Duplicate instrument
	bp = 0x33;
	dl = dh;
	if(al == 3)
		goto AllocateDCT;

	// Duplicate sample
	bp = 0x36;
	dl = chn->Smp-1;
	if((dl & 0x80) != 0)
		goto AllocateChannel4;

AllocateDCT:
	ah = chn->HCN | 0x80;
	*ch = ins->DCA;

AllocateChannel6:
	if((slave->Flags & 1) == 0)
		goto AllocateChannel7;

	if(chn->Smp == 101)
		goto AllocateChannelMIDIDCT;

	al = slave->HCN;
	if(ah != al)
		goto AllocateChannel7;

	// OK. same channel... now..

AllocateChannelMIDIDCT:
	// Same inst?
	if(dh != slave->Ins)
		goto AllocateChannel7;

	// Same note/sample/inst?
	// ("else else" is 0x36 --GM)
	if(dh != (bp == 0x32 ? slave->Nte : bp == 0x33 ? slave->Ins : slave->Smp))
		goto AllocateChannel7;

	// New note is a MIDI?
	if(chn->Smp == 101)
		goto AllocateChannelMIDIHandling;

	if(*ch != slave->DCA)
		goto AllocateChannel7;

	// Checks for hiqual
	if(*ch == 0)
		goto AllocateChannel20Samples;

	slave->DCT = 0;
	al = *ch;
	al++; if(al == 0) ah++;
	goto AllocateHandleNNA;

AllocateChannelMIDIHandling:
	// Is current channel a MIDI chan
	if(slave->Smp != 100)
		goto AllocateChannel7;

	if(ah != slave->MCh)
		goto AllocateChannel7;

	slave->Flags |= 0x200;
	if((slave->HCN & 0x80) != 0)
		goto AllocateChannel7;

	bp = slave->HCOffst;
	slave->HCN |= 0x80;
	ite->chn[bp].Flags &= ~4;

AllocateChannel7:
	slave++;
	cx--;
	if((cx & 0xFF) != 0)
		goto AllocateChannel6;

AllocateChannel4:
	slave = &ite->slave[ite->AllocateSlaveOffset];
	cx = ite->AllocateNumChannels;
	//Mov     CX, AllocateNumChannels
	//Mov     SI, AllocateSlaveOffset

	if(chn->Smp != 101)
		goto AllocateChannel10;

	// MIDI 'slave channels' have to be maintained if still referenced

	//Push    DI

AllocateMIDIChannel1:
	if((slave->Flags & 1) != 0)
		goto AllocateMIDIChannel2;

	// Have a channel.. check that it's host's slave isn't SI
	if(slave->HCOffst == -1)
		goto AllocateMIDIChannelFound;
	if(ite->chn[slave->HCOffst].SCOffst == (slave - &ite->slave[0]))
		goto AllocateMIDIChannelFound;

AllocateMIDIChannel2:
	slave++;
	cx--;
	if(cx != 0)
		goto AllocateMIDIChannel1;

	//Pop     DI

	goto AllocateChannel17;

AllocateChannel10:
	if((slave->Flags & 1) == 0)
		goto AllocateChannelInstrument;

	slave++;
	cx--;
	if(cx != 0)
		goto AllocateChannel10;

AllocateChannel17:
	// Common sample search
	memset(ite->ChannelCountTable, 0, (100+200)); // Clear table
	memset(ite->ChannelCountTable+100, 0xFF, (200));
	memset(ite->ChannelCountTable+300, 0, 100); // Volumes

	cx = ite->AllocateNumChannels;
	uint32_t ebx = 0;
	it_slave *other = &ite->slave[ite->AllocateSlaveOffset];

AllocateChannelCommonSample1:
	// BX = sample pointer into table.
	if(1){}
	uint16_t bx = other->Smp;

	// Just for safety
	if(bx > 99)
		goto AllocateChannelCommonSample2;

	ite->ChannelCountTable[bx]++;

	// Volume
	ah = ite->ChannelCountTable[bx+300];

	// Disowned channel?
	if((other->HCN & 0x80) == 0)
		goto AllocateChannelCommonSample2;

	// Lower Volume?
	if(ah <= other->FV)
		goto AllocateChannelCommonSample2;

	// Get volume
	ah = other->FV;

	// Store location
	ite->ChannelCountTable[100+bx+bx+0] = other - &ite->slave[0];
	//ite->ChannelCountTable[100+bx+bx+1] = (other - &ite->slave[0])>>8;

	// Store volume
	ite->ChannelCountTable[300+bx] = ah;

AllocateChannelCommonSample2:
	other++;
	cx--;
	if(cx != 0)
		goto AllocateChannelCommonSample1;

	// OK.. now search table for maximum
	// occurrence of sample...
	uint16_t di = 0;
	int16_t si = -1;

	// Find maximum count, has to be
	// greater than 2 channels
	ah = 2;
	cx = 100;

AllocateChannelCommonSample3:
	if(ah >= ite->ChannelCountTable[di])
		goto AllocateChannelCommonSample4;

	ah = ite->ChannelCountTable[di];
	si = (int8_t)ite->ChannelCountTable[di+di+100+0];

AllocateChannelCommonSample4:
	di++;
	cx--;
	if(cx != 0)
		goto AllocateChannelCommonSample3;

	// Pop     DI
	// Pop     BX

	if(si != -1)
		goto AllocateChannelInstrument;

	// Find out which host channel has the most
	// (disowned) slave channels
	// Then find the softest non-single sample
	// in that channel.

	memset(ite->ChannelCountTable, 0, 64);

	bx = 0;
	cx = ite->AllocateNumChannels;
	other = &ite->slave[ite->AllocateSlaveOffset];

AllocateChannelCount1:
	bx = other->HCN & 0x3F;
	ite->ChannelCountTable[bx]++;

AllocateChannelCount2:
	other++;
	cx--;
	if(cx != 0)
		goto AllocateChannelCount1;

AllocateChannelCountEnd:
	// OK.. search through and find
	// the most heavily used channel

	// AH = channel count
	// AL = channel
	// 64 = physical channels
	
	ah = 0x01;
	al = 0x00;
	bx = 0;
	cx = 64;

AllocateChannelCountSearch1:
	if(ah >= ite->ChannelCountTable[bx])
		goto AllocateChannelCountSearch2;

	ah = ite->ChannelCountTable[bx];
	al = bx & 0xFF;

AllocateChannelCountSearch2:
	bx++;
	cx--;
	if(cx != 0)
		goto AllocateChannelCountSearch1;

	// AH = channel to use.
	// ^ don't you mean AL, Jeff? AH is the volume. --GM
	if(ah <= 1)
		goto AllocateChannelSoftestSearch;

	// Search for disowned only
	al |= 0x80;

	// actually BH --GM
	uint8_t bh = chn->Smp-1;

	cx = ite->AllocateNumChannels;
	other = &ite->slave[ite->AllocateSlaveOffset];
	it_slave *sither = &ite->slave[ite->AllocateSlaveOffset];
	ah = 0xFF;

AllocateChannelNonSingle1:
	if(al != other->HCN)
		goto AllocateChannelNonSingle3;

	// Lower Volume?
	if(ah <= other->FV)
		goto AllocateChannelNonSingle3;

	// Now check if any other channel contains this sample
	if(bh == other->Smp)
		goto AllocateChannelNonSingle6;

	uint8_t bl = other->Smp;

	{
		//Push    CX
		//Push    SI

		other->Smp = 0xFF;

		it_slave *subslave = &ite->slave[ite->AllocateSlaveOffset];
		uint16_t subcx = ite->AllocateNumChannels;

	AllocateChannelNonSingle2:
		if(bh == subslave->Smp)
			goto AllocateChannelNonSingle5;

		// A second sample?
		if(bl == subslave->Smp)
			goto AllocateChannelNonSingle5;

	AllocateChannelNonSingle4:
		subslave++;
		cx--;
		if(cx != 0)
			goto AllocateChannelNonSingle2;

		other->Smp = bl;

		//Pop     SI
		//Pop     CX
	}

	goto AllocateChannelNonSingle3;

AllocateChannelNonSingle5:
	other->Smp = bl;

	//Pop     SI
	//Pop     CX

AllocateChannelNonSingle6:
	// OK found a second sample.
	// get offset
	si = other - &ite->slave[0];

	// Get volume
	ah = other->FV;

AllocateChannelNonSingle3:
	other++;
	cx--;
	if(cx != 0)
		goto AllocateChannelNonSingle1;

	if(si != -1)
		goto AllocateChannelSampleSearch;

	si = al & 0x3F;
	ite->ChannelCountTable[si] = 0;
	goto AllocateChannelCountEnd; // Next cycle...

AllocateChannelSampleSearch:
	//Push    DI

	al = slave->Smp;

	cx = ite->AllocateNumChannels;
	other = &ite->slave[ite->AllocateSlaveOffset];
	ah = 0xFF;

AllocateChannelSampleSearch1:
	// Same sample?
	if(al != chn->Smp)
		goto AllocateChannelSampleSearch2;

	// Disowned channel?
	if((other->HCN & 0x80) == 0)
		goto AllocateChannelSampleSearch2;

	// Lower Volume?
	if(ah <= other->FV)
		goto AllocateChannelSampleSearch2;

	// get offset.
	si = other - &ite->slave[0];

	// Get volume
	ah = other->FV;

AllocateChannelSampleSearch2:
	other++;
	cx--;
	if(cx != 0)
		goto AllocateChannelSampleSearch1;

	//Pop     DI

	goto AllocateChannelInstrument;

AllocateChannelSoftestSearch:
	//Push    DI

	// Now search for softest
	// disowned sample
	// (not non-single)

	cx = ite->AllocateNumChannels;
	other = &ite->slave[ite->AllocateSlaveOffset];

	// Offset
	si = 0;
	ah = 0xFF;

AllocateChannel18:
	if((other->HCN & 0x80) == 0)
		goto AllocateChannel19; // No.. then look for next

	// Volume set...
	if(ah < other->FV)
		goto AllocateChannel19;

	// get offset.
	si = other - &ite->slave[0];

	// Get volume
	ah = other->FV;

AllocateChannel19:
	other++;
	cx--;
	if(cx != 0)
		goto AllocateChannel18;

	// Pop     DI

	if(si != -1)
		goto AllocateChannelInstrument;

	*ch &= ~4;
	return NULL;

AllocateMIDIChannelFound:
	//Pop     DI

AllocateChannelInstrument:
	chn->SCOffst = slave - &ite->slave[0];

	slave->HCN = chn->HCN;
	slave->HCOffst = chn - &ite->chn[0];

	// Reset vibrato info
	slave->Bit = 0;
	slave->ViP = 0;
	slave->ViDepth = 0;

	// Reset loop dirn
	slave->LpD = 0;

	InitPlayInstrument(ite, chn, slave, ins - &ite->ins[0]);

	slave->SVl = ins->GbV;

	//Pop     CX

	// FadeOut, VolEnv&Pos
	slave->FadeOut = 0x0400;

	al = chn->Nte;
	ah = chn->Ins;
	if(chn->Smp == 101)
		al = chn->Nt2;

	slave->Nte = al;
	slave->Ins = ah;

	if(chn->Smp == 0)
	{
		slave->Flags = 0x200;
		*ch &= ~4;
		return NULL;
	}

	I_TagSample(ite, chn->Smp-1);
	slave->Smp = chn->Smp-1;

	// Sample memory offset.
	slave->SmpOffs = chn->Smp-1;
	it_sample *smp = &ite->smp[slave->SmpOffs];

	if(smp->Length == 0 || (smp->Flg & 1) == 0)
	{
		// No sample!
		slave->Flags = 0x200;
		*ch &= ~4;
		return NULL;
	}

	slave->Bit = (smp->Flg & 2);
	slave->SVl = (smp->GvL * (uint16_t)slave->SVl) >> 6; // SI = 0->128
	//printf("INS VOL %i SMP %i %i\n", slave->SVl, slave->Smp, chn->Smp);

	return slave;
}

uint16_t Random(it_engine *ite)
{
	uint16_t ax = ite->Seed1;
	uint16_t bx = ite->Seed2;
	uint16_t cx = bx;
	uint16_t dx = bx;

	ax += bx;
	ax = (ax<<(cx&15)) | (ax>>(15-(cx&15)));
	ax ^= dx;
	cx = (cx>>8)|(cx<<8);
	bx += cx;
	dx += bx;
	cx += ax;
	ax -= dx - ((bx&1) != 0 ? 1 : 0); // SBB - TODO: confirm I have this right!
	bx = (bx>>1) | (bx<<15);
	ite->Seed2 = dx;
	ite->Seed1 = ax;

	return ax;
}

void GetLoopInformation(it_engine *ite, it_slave *slave)
{
	// TODO: verify
	// Destroys AX, BX, CX, DX
	it_sample *smp = &ite->smp[slave->Smp];
	uint8_t ah;
	int32_t ecx;
	int32_t edx;

	// TODO: fix this crap (there's a lot of guesswork here!)
	if((smp->Flg & ((slave->Flags & 4) == 0 ? 0x30 : 0x10)) == 0)
	{
		ecx = 0;
		edx = smp->Length;
		ah = 0;
	} else {
		ecx = smp->Loop_Begin;
		edx = smp->Loop_End;
		ah = smp->Flg;

		if((smp->Flg & 0x20) != 0 && (slave->Flags & 0x4) == 0) // SusLoop?
		{
			ecx = smp->SusLoop_Begin;
			edx = smp->SusLoop_End;
			ah >>= 1;
		}

		if((ah & 0x40) == 0) {
			ah = 8;
		} else {
			ah = 24;
		}
	}

	if(slave->LpM == ah)
	if(slave->Loop_Beginning == ecx)
	if(slave->Loop_End == edx)
		return;

	slave->LpM = ah;
	slave->Loop_Beginning = ecx;
	slave->Loop_End = edx;
	slave->Flags |= 0x0400; // Loop changed.
}

// include it_m_eff.inc

void PitchSlideDown(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx)
{
	// do NOT blame me for this, it's how it works in the real thing
	// except it actually falls through to procedures so it's worse --GM
#if USEFPUCODE
	bx = -bx;
#else
	if((ite->hdr.Flags & 8) != 0)
		PitchSlideDownLinear(ite, chn, slave, bx);
	else
		// Go on to amiga slide down.
		PitchSlideDownAmiga(ite, chn, slave, bx);
}

void PitchSlideDownAmiga(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx)
{
	slave->Flags |= 32; // recalculate pitch!

	uint64_t m64 = ((uint64_t)slave->Frequency)*((uint64_t)(uint16_t)bx);
	// EDX:EAX = cmd*InitialFreq

	// CX = counter.
	int cx = 0;

	m64 += (uint64_t)(((uint64_t)1712)*(uint64_t)8363);
	while((m64>>(uint64_t)32) != 0)
	{
		m64 >>= (uint64_t)1;
		cx++;
	}

	uint32_t ebx = m64; // EBX = 1712*8363+Cmd*InitialFreq

	m64 = ((uint64_t)slave->Frequency)*(uint64_t)(((uint64_t)1712)*(uint64_t)8363);

	while(cx > 0)
	{
		m64 >>= (uint64_t)1;
		cx--;
	}

	if(ebx > 0)
		slave->Frequency = (m64 / (uint64_t)ebx);

}

void PitchSlideDownLinear(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx)
{
	// Given BX = slide down value = 0->1024

	slave->Flags |= 32; // recalculate pitch!

	const uint16_t *tab;
	if(bx <= 0x0F)
	{
		tab = FineLinearSlideDownTable;
	} else {
		tab = LinearSlideDownTable;
		bx >>= 2;
	}

	uint64_t m64 = tab[bx];
	m64 *= (uint64_t)slave->Frequency;
	m64 >>= (uint64_t)16;
	slave->Frequency = m64;

#endif /* USEFPUCODE */
}

void PitchSlideUp(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx)
{
	if((ite->hdr.Flags & 8) == 0)
		PitchSlideUpAmiga(ite, chn, slave, bx);
	else
		PitchSlideUpLinear(ite, chn, slave, bx);
		// Go on to linear slide
}

void PitchSlideUpLinear(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx)
{
#if USEFPUCODE
	Mov     [CS:SlideValue], BX
	FILD    Word Ptr [CS:SlideValue]
	FMul    [CS:Const1_On_768]      ; Have SlideValue/768.0
	FLd     ST
	FRndInt
	FSub    ST(1), ST
	FXCh
	F2XM1
	FLd1
	FAdd
	FScale
	FIMul   DWord Ptr [SI+10h]
	FIStP   DWord Ptr [SI+10h]
	FStP    ST

PitchSlideUpFPUFreqCheck:
	Or      Byte Ptr [SI], 32       ; recalculate pitch!
	Cmp     DWord Ptr [SI+10h], 07FFFFFFFh
	JAE     PitchSlideUpLinear1
	Ret

PitchSlideUpLinear1:                                    ; Turn off channel
	Or      Word Ptr [SI], 200h
	And     Byte Ptr [DI], Not 4
#else
	slave->Flags |= 32; // recalculate pitch!

	const uint32_t *tab;
	if(bx <= 0x0F)
	{
		tab = FineLinearSlideUpTable;
	} else {
		tab = LinearSlideUpTable;
		bx >>= 2;
	}

	uint64_t m64 = tab[bx];
	m64 *= (uint64_t)slave->Frequency;
	m64 >>= (uint64_t)16;

	if((m64 & (uint64_t)0xFFFF00000000LL) == 0)
	{
		slave->Frequency = m64;
	} else {
		// Turn off channel
		slave->Flags |= 0x0200;
		chn->Flags &= ~4;
		// slave->Flags &= ~1;
		// slave->Flags |= 2<<8; // Cut!
		// chn->Flags &= ~4;
	}
#endif
}

void PitchSlideUpAmiga(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx)
{
#if USEFPUCODE
	Mov     [CS:SlideValue], BX
	FILD    Word Ptr [CS:SlideValue]
	FILD    DWord Ptr [SI+10h]      ; InitFreq, Cmd
	FMul    ST(1), ST               ; InitFreq, Cmd.InitFreq
	FLd     [CS:Const_14317456]     ; 1712*8363, InitFreq, Cmd.InitFreq
	FSubR   ST(2), ST               ; 1712*8363, InitFreq, 1712*8363-Cmd.InitFreq
	FMul                            ; 1712*8363*InitFreq, 1712*8363-Cmd.InitFreq
	FDivRP  ST(1), ST               ; FinalFreq
	FIStP   DWord Ptr [SI+10h]
	Jmp     PitchSlideUpFPUFreqCheck

#else
	slave->Flags |= 32; // recalculate pitch!

	uint64_t m64 = ((uint64_t)slave->Frequency)*((uint64_t)(uint16_t)bx);
	// EDX:EAX = InitialFreq*Cmd

	if((m64 & (uint64_t)0xFFFFFFFF00000000LL) == (uint64_t)0)
	{
		if(m64 < (1712*8363))
		{
			uint32_t ecx = (1712*8363) - m64;

			m64 = slave->Frequency * (uint64_t)(1712*8363);
			if((m64>>(uint64_t)32) < (1712*8363))
			{
				// TODO: verify
				m64 /= (uint64_t)ecx;
				slave->Frequency = (uint32_t)m64;
				return;
			}
		}
	}

	// Turn off channel
	slave->Flags |= 0x0200;
	chn->Flags &= ~4;
	// slave->Flags &= ~1;
	// slave->Flags |= 2<<8; // Cut!
	// chn->Flags &= ~4;
#endif
}

int Music_GetWaveForm(it_engine *ite)
{
	// TODO: work out exactly what to do
	if((ite->d.DriverFlags & 4) == 0)
		return -1;
	
	// TODO: find out the arguments for this
	return ite->d.DriverGetWaveform(ite);
}

void Music_Poll(it_engine *ite)
{
	// AX = CS:PlayMode
	// BX = CS:CurrentPattern
	ite->d.DriverPoll(ite, ite->PlayMode, ite->CurrentPattern);
}

void Music_InitTempo(it_engine *ite)
{
	ite->d.DriverSetTempo(ite, Music_GetTempo(ite));
}

void GetChannels(it_engine *ite)
{
	// Returns min of NumChannels & DriverMaxChannels
	// Also uses default channels if num channels
	// = 0ffffh
	int16_t ax = ite->CmdLineNumChannels;

	if(ax == -1)
		ax = (int16_t)ite->d.DefaultChannels;

	if(ax > (int16_t)ite->d.DriverMaxChannels)
		ax = (int16_t)ite->d.DriverMaxChannels;

	if(ax >= MAXSLAVECHANNELS)
		ax = MAXSLAVECHANNELS; // MC4

	ite->NumChannels = ax;
}

void Music_ReinitSoundCard(it_engine *ite)
{
	GetChannels(ite);
	ite->d.DriverReinitSound(ite);
	Music_SoundCardLoadAllSamples(ite);
}

void Music_UnInitSoundCard(it_engine *ite)
{
	ite->d.DriverUninitSound(ite);
}

void Music_InitMusic(it_engine *ite)
{
#if ENABLEINT3
#else
	// This puts Music_UpdateSampleLocation in the INT3 TSR.
	// Because we are not in x86 Real Mode, we cannot do this.
#endif

	Trace(" - Initialising SoundDriver Tables");

	Music_ClearDriverTables(ite);
	D_GotoStartingDirectory(ite);

	Trace(" - Loading MIDI configuration");

	// Open MIDI Config file.
	FILE *fp = NULL;//fopen(MIDIConfigFileName, "rb");

	if(fp != NULL)
	{
		// TODO: serialise this properly
		// TODO: actually create a MIDIDataArea
		//fread(ite->MIDIDataArea, 1, (128+16+9)*32, fp);

		fclose(fp);
	}

	Trace(" - Initialising playback tables");

	Music_Stop(ite);
}

void Music_ReleasePattern(it_engine *ite, uint16_t ax)
{
	// AX = pattern number

	// Complete rewrite.
	if(ite->pat[ax] != NULL)
	{
		free(ite->pat[ax]);
		ite->pat[ax] = NULL;
	}
}

it_pattern *Music_GetPattern(it_engine *ite, uint16_t ax)
{
	// This is much simpler.
	it_pattern *pat = ite->pat[ax]; // SI

	if(pat == NULL)
		pat = (it_pattern *)EmptyPattern;

	return pat;

	// Here's what happens in the actual code.
	/*
	if(AL < 1)
	{
		// Empty pattern
		Push    CS
		Pop     DS
		Mov     SI, Offset EmptyPattern

		Pop     AX
		Ret

	} else if(AL == 1) {
		// Start of segment, conventional memory
		LodsW
		Mov     DS, AX
		Xor     SI, SI

		Pop     AX
		Ret

	} else if(AL < 3) {
		Push    CX
		Push    DX

		MovZX   CX, AH
		Mov     DX, [SI]
		Call    E_MapEMSMemory
		Call    E_GetEMSPageFrame
		Mov     DS, AX
		Xor     SI, SI

		Pop     DX
		Pop     CX

		Pop     AX
		Ret
	} else {
		// Start of EMS block
		LodsW
		Call    E_MapAlignedBlockEMS

		Pop     AX
		Ret
	}
	*/

	// Basically, a bunch of conventional-vs-EMS crap.
}

it_pattern *Music_GetPatternLocationNoCount(it_engine *ite, uint16_t ax)
{
	// Because we don't use the x86 realmode segment:offset crap,
	// this is identical to Music_GetPattern.
	it_pattern *pat = ite->pat[ax];

	if(pat == NULL)
		pat = (it_pattern *)EmptyPattern;

	return pat;
}

it_pattern *Music_GetPatternLocation(it_engine *ite, uint16_t ax, uint16_t *len)
{
	// AX = pattern number
	// Returns AX = handle
	//         EBX = page/offset or
	//               seg/offset
	//         CX = length

	it_pattern *pat = Music_GetPattern(ite, ax);
	*len = pat->Length;

	return Music_GetPatternLocationNoCount(ite, ax);
}

it_pattern *Music_AllocatePattern(it_engine *ite, uint16_t dx)
{
	// DX = length.
	// SI = Pattern
	// ES:DI points to pattern area

	// I suspect this is the C version:
	return malloc(dx);

	// Yeah.
}

uint8_t *Music_AllocateSample(it_engine *ite, uint16_t ax, size_t edx)
{
	// AX = Sample number, 0 based
	// EDX = length
	// Returns ES:DI, ES = 0 if not.

	it_sample *smp = &ite->smp[ax];
	int real_len = smp->Length;

	Music_ReleaseSample(ite, ax, 2);
	edx += 8; // Extra 8 bytes allocated..

	if(edx < 1048576)
	{
		smp->Flg |= 1;

		// not in the actual code, just trying to prevent leaks --GM
		if(ite->SamplePointer[ax] != NULL)
		{
			free(ite->SamplePointer[ax]);
			ite->SamplePointer[ax] = NULL;
		}

		// XXX: not sure where this actually goes --GM
		smp->Length = real_len;

		ite->SamplePointer[ax] = malloc(edx);
		return ite->SamplePointer[ax];
	}

	smp->Flg &= 0xF0;
	return NULL;
}

void Music_ReleaseSample(it_engine *ite, uint8_t al, uint8_t ah)
{
	// AX = sample number, 0 based
	// AH = 1 = called from network
	//    = 2 = called from allocate

	it_sample *smp = &ite->smp[al]; // DS:BX points to sample

	ite->d.DriverReleaseSample(ite, smp);

	if((smp->Flg & 1) != 0)
	{
		if(ite->SamplePointer[al] != NULL)
		{
			free(ite->SamplePointer[al]);
			ite->SamplePointer[al] = NULL;
		}
	}

	smp->Length = 0;
	//smp->SamplePointer = 0; // TODO: find out what to do with this --GM

	if(ah > 1)
		return;

	if(ah != 1)
	{
#if NETWORKENABLED
		Network_AddWordToQueue(ite, (ah<<8) | NETWORK_DELETESAMPLEOBJECT);
#endif
	}

	smp->Flg &= 0xFE;

	// this is where we do a good thing and actually use our struct properly
	// IT itself just does memset(smp->Name, 0, 0x3C). --GM

	// also, why isn't the filename scrubbed? --GM
	memset(smp->Name, 0, 26);
	smp->Cvt = 0;
	smp->DfP = 0;
	smp->Length = 0;
	smp->Loop_Begin = 0;
	smp->Loop_End = 0;
	smp->C5Speed = 0;
	smp->SusLoop_Begin = 0;
	smp->SusLoop_End = 0;
	smp->ViS = 0;
	smp->ViD = 0;
	smp->ViR = 0;
	smp->ViT = 0;
}

void Music_ClearSampleName(it_engine *ite, uint16_t ax)
{
	// AX = Sample number (0 based)

	memcpy(&ite->smp[ax], SampleHeader, 80);

	// not in the actual code, just here to prevent a leak --GM
	if(ite->SamplePointer[ax] != NULL)
	{
		free(ite->SamplePointer[ax]);
		ite->SamplePointer[ax] = NULL;
	}
}

void Music_ClearAllSampleNames(it_engine *ite)
{
	int i;

	for(i = 99; i >= 0; i--)
		Music_ClearSampleName(ite, i);
}

void Music_ReleaseAllSamples(it_engine *ite)
{
	int i;

	for(i = 99; i >= 0; i--)
		Music_ReleaseSample(ite, i, 0);
}

void Music_ReleaseAllPatterns(it_engine *ite)
{
	int i;

	for(i = 99; i >= 0; i--)
		Music_ReleasePattern(ite, i);
}

void Music_ClearInstrument(it_engine *ite, uint16_t ax)
{
	// AX = Instrument number
	// (0 based)

	memcpy(&ite->ins[ax], InstrumentHeader, 554);
}

void Music_ClearAllInstruments(it_engine *ite)
{
	int i;

	for(i = 99; i >= 0; i--)
		Music_ClearInstrument(ite, i);
}

void Music_UnInitMusic(it_engine *ite)
{
	Music_UnInitSoundCard(ite);
	Music_UnloadDriver(ite);

	Music_ReleaseAllPatterns(ite);
	Music_ReleaseAllSamples(ite);
}

// NOT IMPLEMENTING: Music_GetSongSegment
// BECAUSE: it's legacy DOS crap that doesn't apply --GM

void Music_UnloadDriver(it_engine *ite)
{
	// really not sure how to go about this one.
	// might just opt for dlclose() or something like that --GM

	/*
	Xor     AX, AX
	XChg    AX, CS:SoundDriverSegment
	Test    AX, AX
	JZ      Music_UnloadDriver1

	Mov     ES, AX
	Mov     AH, 49h
	Int     21h

Music_UnloadDriver1:
	Ret
	*/
}

// these have been reworked a bit
int NoFunction(it_engine *ite)
{
	return -1;
}

int NoFunctionB(it_engine *ite, uint8_t w1)
{
	return -1;
}

int NoFunctionW(it_engine *ite, uint16_t w1)
{
	return -1;
}

int NoFunctionWW(it_engine *ite, uint16_t w1, uint16_t w2)
{
	return -1;
}

int NoFunctionPs(it_engine *ite, it_sample *ps1)
{
	return -1;
}

const char *NoFunction2(it_engine *ite)
{
	return NoSoundCardMsg;
}

const char *NoFunction2PccWW(it_engine *ite, const char *c1, uint16_t w2, uint16_t w3)
{
	return NoSoundCardMsg;
}

void Music_ClearDriverTables(it_engine *ite)
{
	// Makes all of them point to
	// Xor AX, AX, StC, RetF
	ite->d.DriverDetectCard = NoFunction2PccWW;
	ite->d.DriverInitSound = NoFunction2;

	// we have to serialise this, really,
	// thus, no OLDDRIVER distinction --GM
	ite->d.DriverReinitSound = NoFunction;
	ite->d.DriverUninitSound = NoFunction;
	ite->d.DriverPoll = NoFunctionWW;
	ite->d.DriverSetTempo = NoFunctionW;
	ite->d.DriverSetMixVolume = NoFunctionW;
	ite->d.DriverSetStereo = NoFunctionW;
	ite->d.DriverLoadSample = NoFunctionW;
	ite->d.DriverReleaseSample = NoFunctionPs;
	ite->d.DriverResetMemory = NoFunction;
	ite->d.DriverGetStatus = NoFunction;

	ite->d.DriverSoundCardScreen = NoFunction;
	ite->d.DriverGetVariable = NoFunctionW;
	ite->d.DriverSetVariable = NoFunctionWW;

	ite->d.DriverMIDIOut = NoFunctionB;
	ite->d.DriverGetWaveform = NoFunction;

	ite->d.DriverMaxChannels = 32;
}

int Music_LoadDriver(it_engine *ite, const char *fname)
{
	// pretty much a complete rewrite! let's go! --GM

	// dlopen() is handled on every modern OS that isn't Windows.
	// of course, it's easier to just statically link in a driver.
	// so, no dlopen(). yet.

	it_drvdata *drv = DriverSys_GetByName(ite, fname);
	if(drv == NULL)
	{
		Music_ClearDriverTables(ite);
		return -1;
	}

	memcpy(&ite->d, drv, sizeof(it_drvdata));

	// There's no need to worry about "required variables"
	// or "required functions" - driver has full it_engine access.

	return 0;
}

const char *Music_AutoDetectSoundCard(it_engine *ite)
{
	int dord = 0;
	const char *reply = NULL;

	// Returns DS:SI = string
	// AX, BX, CX, DX, DI = parameters

	D_GotoStartingDirectory(ite);

	if(ite->DriverName != NULL)
	{
		Trace(" - Loading specific soundcard driver");

		if(ite->DriverName[0] != '\x00')
		{
			if(!Music_LoadDriver(ite, ite->DriverName))
			{
				reply = ite->d.DriverDetectCard(
					ite, ite->DriverName, 1, TRACKERVERSION); // Forced

				// Unfortunately these drivers don't have to return 'Jeff'.
				if(reply != NULL)
				{
					Music_UnloadDriver(ite);
					Music_ClearDriverTables(ite);
				}
			}
		}
	} else for(;;) {
	Music_AutoDetectSoundCard1:
		Trace(" - Testing soundcard driver");

		int didx = DriverDetectionOrder[dord++];
		if(didx == 0xFFFF)
			break;

		if(!Music_LoadDriver(ite, DriverNameTable[didx]))
		{
			const char *reply = ite->d.DriverDetectCard(
				ite, DriverNameTable[didx], 1, TRACKERVERSION); // Not forced

			if(reply == NULL)
				break;

			Music_UnloadDriver(ite);
			Music_ClearDriverTables(ite);
		}
	}

	GetChannels(ite);
	ite->d.DriverInitSound(ite);

	// Not sure what these are for!
	/*
	Mov     [CS:ADSCParams+2], DS
	Mov     [ADSCParams], SI
	Mov     [ADSCParams+12], DI
	Mov     [ADSCParams+10], DX
	Mov     [ADSCParams+8], CX
	Mov     [ADSCParams+6], BX
	Mov     [ADSCParams+4], AX
	*/

	Music_InitStereo(ite);
	Music_InitMixTable(ite);

	return reply;
}

void Music_ShowAutoDetectSoundCard(it_engine *ite)
{
	// HELP --GM
	/*
	Push    DWord Ptr [ADSCParams+10]
	Push    DWord Ptr [ADSCParams+6]
	Push    [ADSCParams+4]

	LDS     SI, DWord Ptr [ADSCParams]

	Mov     AH, 20h
	Mov     DI, (26+28*80)*2
	Call    S_DrawString

	Add     SP, 10
	Ret
	*/
}

uint16_t Music_GetInstrumentMode(it_engine *ite)
{
	return (ite->hdr.Flags & 4);
}

void UpdateGOTONote(it_engine *ite)
{
	// Get offset & arrayed flag.

	uint16_t patnum;
	uint16_t maxrow;

	it_pattern *pat = PE_GetCurrentPattern(ite, &patnum, &maxrow);
	// AX = Pattern number
	// BX = MaxRow
	// DS = PatternDataSegment

	if(patnum == ite->CurrentPattern)
	{
		ite->DecodeExpectedPattern = patnum;
		//Mov     CS:PatternSegment, DS

		ite->NumberOfRows = maxrow;

		if(ite->ProcessRow >= ite->NumberOfRows)
		{
			ite->ProcessRow = 0;
			ite->CurrentRow = 0;
		}

		ite->DecodeExpectedRow = ite->ProcessRow;

		ite->PatternOffset = ite->ProcessRow * (64*5);
		ite->PatternArray = 1;

		return;
	}

	ite->DecodeExpectedPattern = ite->CurrentPattern;
#if NETWORKENABLED
	Network_UpdatePatternIfIdle(ite);
#endif
	pat = Music_GetPattern(ite, ite->CurrentPattern);
	// DS:SI points to pattern.

	// AX = number of rows
	if(ite->ProcessRow >= pat->Rows)
		ite->CurrentRow = 0;
	else
		ite->CurrentRow = ite->ProcessRow;

	ite->ProcessRow = ite->CurrentRow;
	ite->DecodeExpectedRow = ite->CurrentRow;
	ite->NumberOfRows = pat->Rows;

	uint8_t *data = pat->data;
	uint16_t cx = ite->ProcessRow;
	uint8_t al;
	cx++;

	while((--cx) != 0)
	{
		for(;;)
		{
			// OK.. now to find the right
			// offset & update tables.
			al = *(data++);
			if(al == 0)
				break;

			uint8_t dl = al;
			al = (al & 0x7F) - 1;
			it_host *chn = &ite->chn[al];
			// CS:DI points.

			if((dl & 0x80) != 0)
				chn->Msk = *(data++);

			if((chn->Msk & 1) != 0)
				chn->Nte = *(data++);
			if((chn->Msk & 2) != 0)
				chn->Ins = *(data++);
			if((chn->Msk & 4) != 0)
				chn->Vol = *(data++);
			if((chn->Msk & 8) != 0)
			{
				// warning, when reading the code this can catch you out.
				// it loads then stores a *word*.
				// the IT code does this kind of stuff a lot. --GM
				chn->Cmd = *(data++);
				chn->CVal = *(data++);
			}
		}
	}

	ite->PatternOffset = data - pat->data;
	ite->PatternArray = 0;
}

void PreInitCommand(it_engine *ite, it_host *chn)
{
	if((chn->Msk & 0x33) != 0)
	{
		if((ite->hdr.Flags & 4) == 0 || chn->Nte >= 120 || chn->Ins == 0)
		{
			chn->Nt2 = chn->Nte;
			chn->Smp = chn->Ins;
		} else {
			// Have to xlat.

			it_instrument *ins = &ite->ins[chn->Ins-1];

			if(ins->MCh == 0)
			{
				chn->Nt2 = ins->NoteSamp[chn->Nte][0];
				chn->Smp = ins->NoteSamp[chn->Nte][1];
				// This part is fine.
				//printf("XLAT %i -> %i %i\n", chn->Nte, chn->Nt2, chn->Smp);
			} else {

				if(ins->MCh == 17)
					chn->MCh = chn->HCN+1;
				else
					chn->MCh = ins->MCh;

				chn->MPr = ins->MPr;

				chn->Nt2 = ins->NoteSamp[chn->Nte][0];
				chn->Smp = 101;
			}

			if(chn->Smp == 0) // No sample?
				return;
		}
	}

	InitCommandTable[chn->Cmd & 0x1F](ite, chn); // Init note
	chn->Flags |= 64;

	// Check whether chn is on
	if((chn->HCN & 0x80) == 0)
		return;

	if((chn->Flags & 32) != 0)
		return;

	if((chn->Flags & 4) == 0)
		return; // Channel was off.

	it_slave *slave = &ite->slave[chn->SCOffst];
	slave->Flags |= 0x0800;
}

void UpdateNoteData(it_engine *ite)
{
	ite->PatternLooping = 0;

	if(ite->CurrentPattern == ite->DecodeExpectedPattern)
	{
		UpdateGOTONote(ite);
	} else {
		ite->DecodeExpectedRow++;
		if(ite->CurrentRow != ite->DecodeExpectedRow)
			UpdateGOTONote(ite);
	}

	uint16_t cx = 64; // 64 channels
	// Xor     AX, AX                  // Just to get rid of "jumps"
	it_host *chn = &ite->chn[0]; // DI --GM

	if(ite->PatternArray != 1)
	{
		// First clear all old command&value.
		// Mov     CX, 64                 // Done above

		for(; cx != 0; cx--)
		{
			chn->Flags &= ~(3+32+64+256);
			chn++;
		}

		//Mov     AX, CurrentPattern
#if NETWORKENABLED
		// Network_UpdatePatternIfIdle(ite);
#endif
		it_pattern *pat = Music_GetPattern(ite, ite->CurrentPattern); // Gets DS
		uint8_t *data = pat->data + ite->PatternOffset;

		for(;;)
		{
			uint8_t al;
			al = *(data++);
			if(al == 0) break; // No more!

			// else... go through decoding.

			uint8_t dl = al;
			assert((al & 0x7F) != 0);
			al = (al & 0x7F) - 1;
			chn = &ite->chn[al];

			uint8_t dh = chn->Msk; // mask.
			if((dl & 0x80) != 0)
			{
				dh = *(data++);
				chn->Msk = dh;
			}

			if((dh & 1) != 0)
				chn->Nte = *(data++);
			if((dh & 2) != 0)
				chn->Ins = *(data++);
			if((dh & 4) != 0)
				chn->Vol = *(data++);

			uint8_t ah;
			if((dh & 8) != 0)
			{
				al = chn->OCm = *(data++);
				ah = chn->OVal = *(data++);
			} else if((dh & 0x80) != 0) {
				al = chn->OCm;
				ah = chn->OVal;
			} else {
				al = ah = 0;
			}

			chn->Cmd  = al;
			chn->CVal = ah;
			PreInitCommand(ite, chn);
		}

		ite->PatternOffset = data - pat->data;
	} else {

		uint8_t *data = (&ite->patspace[0]) + ite->PatternOffset;

		// Mov     CX, 64                  ; 64 channels
		// Mov     DI, Offset HostChannelInformationTable

		for(; cx != 64; cx--)
		{
			uint8_t dl = 0; // DL = mask.
			chn->Flags &= ~(3+32+64+256); // Turn off all calling...

			uint8_t al;
			al = *(data++); 
			if(al != NONOTE)
			{
				chn->Nte = al;
				dl |= 1;
			}

			al = *(data++);
			if(al != 0)
			{
				chn->Ins = al;
				dl |= 2;
			}

			al = *(data++);
			if(al != 0xFF)
			{
				chn->Vol = al;
				dl |= 4;
			}

			al = *(data++);
			uint8_t ah = *(data++);
			chn->Cmd  = chn->OCm  = al;
			chn->CVal = chn->OVal = ah;

			if(al != 0 || ah != 0)
				dl |= 8;

			if(dl != 0)
			{
				chn->Msk = dl;
				PreInitCommand(ite, chn);
			}

			chn++;
		}

		ite->PatternOffset = data - ite->patspace;
	}
}

void UpdateVibrato(it_engine *ite, it_slave *slave)
{
	// DS:SI points to slavechannelstruct.

	it_sample *smp = &ite->smp[slave->SmpOffs]; // ES:BX points to sample
	if(smp->ViD == 0)
		return;

	// ITTECH.TXT lied to you - check the original source.
	// it misses a crucial step... and IT uses CX not AX --GM
	slave->ViDepth += smp->ViR;
	if((slave->ViDepth>>8) > smp->ViD)
		slave->ViDepth = (slave->ViDepth & 0xFF) | (((uint16_t)smp->ViD)<<8);

	if(smp->ViS == 0)
		return;

	int8_t al = 0;
	if(smp->ViT != 3)
	{
		slave->ViP += smp->ViS; // Update pointer.

		// probably not wise to try to emulate out-of-range vibrato types.
		// well, at least for now. we can research these later. --GM
		switch(smp->ViT)
		{
			case 0:
				al = FineSineData[slave->ViP];
				break;
			case 1:
				al = FineRampDownData[slave->ViP];
				break;
			case 2:
				al = FineSquareWave[slave->ViP];
				break;
			default:
				printf("PANIC: out of range vibrato types not emulated!\n");
				abort();
		}
	} else {
		al = ((Random(ite) & 0x7F) - 64);
	}

	int16_t ax = ((int16_t)al) * (int16_t)(slave->ViDepth>>8);
	ax >>= 6; // SAL 2, take high (SAR 8) .: SAR 6

#if !USEFPUCODE
	if(ax < 0)
	{
		// strictly speaking this branch doesn't look for the host channel,
		// but it's included for completeness --GM
		PitchSlideDownLinear(ite, &ite->chn[slave->HCOffst], slave, -ax);
	} else {
#endif
		PitchSlideUpLinear(ite, &ite->chn[slave->HCOffst], slave, ax);
#if !USEFPUCODE
	}
#endif
}

void Update(it_engine *ite, uint16_t *rcx, it_slave **si, uint16_t *ax)
{
	uint16_t cx = MAXSLAVECHANNELS;
	it_slave *slave = &ite->slave[0];
	it_host *chn = &ite->chn[slave->HCOffst];

	MIDITranslate(ite, chn, slave, MIDICOMMAND_TICK);

	for(; cx != 0; cx--)
	{
		if((slave->Flags & 1) != 0)
		{
			// reset volume
			if(slave->VS != slave->Vol)
			{
				slave->Vol = slave->VS;
				slave->Flags |= 0x10;
			}

			// Freq
			if(slave->Frequency_Set != slave->Frequency)
			{
				slave->Frequency = slave->Frequency_Set;
				slave->Flags |= 0x20; // Recalc freq.
			}
		}

		slave++;
	}

	UpdateData(ite);

	if((ite->hdr.Flags & 4) != 0)
	{
		UpdateInstruments(ite);
	} else {
		UpdateSamples(ite);
	}
	
	*rcx = ite->NumChannels;
	*si = &ite->slave[0];
	*ax = ite->PlayMode;
}

void UpdateSamples(it_engine *ite)
{
	uint16_t i = ite->NumChannels;
	it_slave *slave = &ite->slave[0];

	for(; i != 0 ; i--, slave++)
	{
		if((slave->Flags & 1) == 0)
			continue;

		// OK... if recalc volume is on.
		// then recalc volume! :)
		if((slave->Flags & 16) != 0)
		{
			slave->Flags &= ~16;
			slave->Flags |= 64; // Recalc final vol

			if(ite->SoloSample != 0xFF && ite->SoloSample != slave->Smp)
				slave->Flags |= 0x0800;

			uint32_t ax = ((uint32_t)slave->Vol)*((uint32_t)slave->CVl)
				*((uint32_t)slave->SVl); // AX = 0->64*64*128
			ax >>= 4; // AX = 0->32768

			ax *= (uint32_t)ite->GlobalVolume;
			// AX = 0->32768*128

			ax >>= 7;

			// Final vol stored.
			slave->FV = ax>>8;
			slave->_16bVol = ax;
		}

		if((slave->Flags & 2) != 0)
		{
			slave->Flags &= ~2;
			slave->Flags |= 0x8000;

			slave->FP = slave->Pan;
			if(slave->Pan != 100)
			{
				int16_t ax = slave->Pan;
				ax -= 32;
				// Pan = (Pan-32)* Separation/128 + 32

				ax *= (int16_t)ite->hdr.Sep;
				// 0->64 (Separation) => AX = -2048->+2048
				// ie. AH = -8->+8

				ax >>= 7;
				// AL = -32->+32

				if(ite->d.ReverseChannels != 0)
					ax = -ax;

				ax += 32;

				slave->FPP = ax;
			} else {
				slave->FPP = slave->Pan;
			}
		}

		UpdateVibrato(ite, slave);
	}
}

int UpdateEnvelope(it_engine *ite, it_slen *slen, it_envelope *env, uint16_t bp)
{
	// Returns Carry if envelope needs
	// to be turned off
	// Reqs ES:DI points to envelope
	// DS:SI points to slave channel envelope structure
	// Called only if envelope is ON
	// BP != 0 if sustain points released, 0 otherwise

	int32_t dx = slen->EnvPos; // DX = current pos
	if(dx < slen->NextET)
	{
		// Increase position;
		slen->EnvPos = ++dx;

		// Update value
		slen->EnvelopeValue += slen->EnvelopeDelta;

		return 0;
	}

	// Procedure:
	// 1) Get current pos' value
	// 2) Figure out next pos (inc/loop)
	// 3) Figure out delta to next pos
	// 4) Terminate if no loop (with carry)
	//      or place new check in [SI+0Ch]

	uint16_t bx = slen->CurEnN; // BX = cur env node;
	uint16_t ax = bx+1;

	dx = (int8_t)env->Nodes[bx][0];
	dx <<= 16;
	slen->EnvelopeValue = dx; // Current pos value done.
	// AX = next cur env node

	if((env->Flg & 6) != 0) // Any loop at all?
	{
		// Normal Loop
		uint8_t bl = env->LpB;
		uint8_t bh = env->LpE;

		// the flow here is very weird in the original source --GM
		int use_sus = ((env->Flg & 4) != 0) && (bp != 0);
		int has_loop = (use_sus ? (env->Flg & 4) : (env->Flg & 2)) != 0;

		if(use_sus)
		{
			bl = env->SLB;
			bh = env->SLE;
		}

		// Loop
		if(has_loop && ax > bh)
		{
			// BL = New node
			slen->CurEnN = bl;
			uint16_t x = ((uint16_t)env->Nodes[bl][1])
				| (((uint16_t)env->Nodes[bl][2])<<8);

			slen->EnvPos = x;
			slen->NextET = x;

			return 0;
		}
	}

	// AX = new node
	if(ax >= env->Num)
		return 1;

	//slen->CurEnN = ax; // New node
	slen->CurEnN = (slen->CurEnN & 0xFF00) | (ax & 0x00FF); // New node

	// New node's tick
	uint16_t x1 = ((uint16_t)env->Nodes[ax][1])
		| (((uint16_t)env->Nodes[ax][2])<<8);
	slen->NextET = x1;
	uint16_t x0 = ((uint16_t)env->Nodes[ax-1][1])
		| (((uint16_t)env->Nodes[ax-1][2])<<8);

	uint16_t delta = x1 - x0;
	uint16_t base = x0+1;
	slen->EnvPos = base; // For security.

	int32_t yd = (int32_t)(((int8_t)env->Nodes[ax][0]) - ((int8_t)env->Nodes[ax-1][0]));
	int isneg = (yd < 0);

	if(isneg) yd = -yd;

	// Just to prevent div 0 errors
	if(delta == 0) delta = 1;

	// this was hell to translate --GM
	uint32_t eax = ((uint32_t)yd)/(uint32_t)delta;
	uint32_t edx = ((uint32_t)yd)%(uint32_t)delta;
	yd = (eax<<16) | ((edx<<16)/(uint32_t)delta);

	if(isneg) yd = -yd;

	slen->EnvelopeDelta = yd; // Delta done

	return 0;
}

void UpdateMIDI(it_engine *ite)
{
	uint16_t cx;
	it_slave *slave;
	it_host *chn;

	// Stop cycle
	for(cx = MAXSLAVECHANNELS, slave = &ite->slave[0]; cx != 0; cx--, slave++)
	{
		if((slave->Flags & 0x200) == 0) continue;
		if(slave->Smp != 100) continue;
		if((slave->Flags & 1) == 0) continue;

		slave->Flags = 0;
		chn = &ite->chn[slave->HCOffst];

		MIDITranslate(ite, chn, slave, MIDICOMMAND_STOPNOTE);
		
		if((slave->HCN & 0x80) != 0) continue;

		slave->HCN |= 0x80;
		chn->Flags &= ~4; // Signify channel off
	}

	// Play cycle
	for(cx = MAXSLAVECHANNELS, slave = &ite->slave[0]; cx != 0; cx--, slave++)
	{
		if(slave->Smp != 100) continue; // MIDI Instrument?

		if((slave->Flags & 1) == 0) continue;

		slave->OldSampleOffset = (slave->OldSampleOffset & ~0xFF)
			| (slave->Sample_Offset & 0xFF);
		slave->Sample_Offset = (slave->Sample_Offset & ~0xFF)
			| 1;

		if((slave->Smp & 0x0800) != 0) continue; // Muted?

		chn = &ite->chn[slave->HCOffst];
		slave->Flags &= 0x788D;
		// 0111100010001101b
		// 0111 1000 1000 1101
		// 7    8    8    D

		if((slave->Flags & 0x0100) != 0)
		{
			// Check if there's a bank select.
			uint16_t MBank = ((uint16_t)slave->FCut)
				| (((uint16_t)slave->FRes)<<8);

			if(MBank != 0xFFFF)
			{
				if(ite->MIDIBanks[slave->MCh-1] != MBank)
				{
					ite->MIDIBanks[slave->MCh-1] = MBank;
					ite->MIDIPrograms[slave->MCh-1] = 0xFF; // Reset program
					MIDITranslate(ite, chn, slave, MIDICOMMAND_BANKSELECT);
				}
			}

			// Check for a program specification
			if(((int8_t)slave->MPr) >= 0)
			{
				if(ite->MIDIPrograms[slave->MCh-1] != slave->MPr)
				{
					ite->MIDIPrograms[slave->MCh-1] = slave->MPr;
					MIDITranslate(ite, chn, slave, MIDICOMMAND_PROGRAMSELECT);
				}
			}

			// Check for MIDI pitch wheel..
			if((ite->hdr.Flags & 64) != 0)
			{
				if(ite->MIDIPitch[slave->MCh-1] != 0x2000)
				{
					ite->MIDIPitch[slave->MCh-1] = 0x2000;
					MIDISendFilter(ite, chn, (slave->MCh-1) | 0xE0);
					MIDISendFilter(ite, chn, 0); // Reset pitch wheel
					MIDISendFilter(ite, chn, 0x40);
				}
			}

			slave->RVol_MIDIFSet = slave->Frequency_Set;
			MIDITranslate(ite, chn, slave, MIDICOMMAND_PLAYNOTE);
		} else {
			// Change in volume?
			if((slave->Flags & 64) != 0)
				MIDITranslate(ite, chn, slave, MIDICOMMAND_CHANGEVOLUME);
		}

		// Pan changed?
		if((slave->Flags & 0x8000) != 0)
		{
			if(ite->MIDIPan[slave->MCh-1] != slave->FPP)
			{
				slave->FPP = ite->MIDIPan[slave->MCh-1];
				MIDITranslate(ite, chn, slave, MIDICOMMAND_CHANGEPAN);
			}
		}

		// Pitch changed?
		if((slave->Flags & 32) != 0)
			MIDITranslate(ite, chn, slave, MIDICOMMAND_CHANGEPITCH);
	}
}

void UpdateInstruments(it_engine *ite)
{
	// Things to update:
	//  1) Volume envelope
	//  2) Fadeout
	//  3) FinalVolume
	//  4) Vibrato.
	// Turn off channel if
	//  1) Volume envelope is off & VEV = 0 or
	//  2) Fadeout = 0

	uint16_t cx;
	it_slave *slave;
	ite->DoMIDICycle = 0;

	for(cx = MAXSLAVECHANNELS, slave = &ite->slave[0]; cx != 0; cx--, slave++)
	{
		if((slave->Flags & 1) != 0)
			UpdateInstruments16(ite, slave); // Channel on!!
	}

	if(ite->DoMIDICycle != 0)
		UpdateMIDI(ite);
}

void UpdateInstruments16(it_engine *ite, it_slave *slave)
{
	// Mov     CX, [SI]

	it_instrument *ins = &ite->ins[slave->InsOffs];

	if(slave->Ins == 0xFF) // No instrument?
		return UpdateInstruments5(ite, slave);
	
	uint16_t bp = (slave->Flags & 4); // BP = sustain for envelope calls

	if((slave->Flags & 0x4000) != 0)
		if(UpdateEnvelope(ite, &slave->Pt, &ins->PitchEnv, bp) != 0)
			slave->Flags &= ~0x4000;

	int do_filter = ((ins->PitchEnv.Flg & 0x80) != 0);
	
	if(do_filter && slave->Smp != 100)
	{
		int16_t fval = (slave->Pt.EnvelopeValue>>8);
		fval >>= 6; // Range -128 to +128
		fval += 128; // Range 0 -> 256

		uint16_t ufval = fval;

		if(fval >= 0x100) fval--;
		slave->FCut = fval;

		slave->Flags |= 64; // Recalculate final volume

	} else if(!do_filter) {
		int16_t ptval = (slave->Pt.EnvelopeValue>>8);
		ptval >>= 3;

		if(ptval != 0)
		{
#if !USEFPUCODE
			if(ptval >= 0)
			{
#endif
				PitchSlideUpLinear(ite, &ite->chn[slave->HCOffst], slave, ptval);
#if !USEFPUCODE
			} else {
				PitchSlideDownLinear(ite, &ite->chn[slave->HCOffst], slave, -ptval);
			}
#endif
			slave->Flags |= 32; // Recalculate freq
		}
	}

	if((slave->Flags & 0x2000) != 0)
	{
		slave->Flags |= 2; // Recalculate pan

		if(UpdateEnvelope(ite, &slave->P, &ins->PanEnv, bp) != 0)
			slave->Flags &= ~0x2000;
	}

	// Volume envelope on?
	int jump_ins17 = 0;
	if((slave->Flags & 0x1000) != 0)
	{
		slave->Flags |= 16; // Recalculate volume

		if(UpdateEnvelope(ite, &slave->P, &ins->PanEnv, bp) == 0)
		{
			// Note fade on?
			if((slave->Flags & 8) != 0)
			{
				// Now, check if loop + sustain
				// off
				// TODO: verify - i may have missed a bp modification --GM
				if(bp == 0)
					return UpdateInstruments5(ite, slave);

				// Normal vol env loop?
				if((ins->VolEnv.Flg & 2) == 0)
					return UpdateInstruments5(ite, slave); // Volume calculation
			}

		} else {
			// Envelope turned off...
			slave->Flags &= ~0x1000; // Turn off envelope flag

			// Turn off if end of loop is reached
			// TODO: verify - this looks weird
			if(((slave->V.EnvelopeValue>>16)&0xFF) == 0)
				jump_ins17 = 1;
		}

	} else {
		if((slave->Flags & 8) != 0) // Note fade??
		{
			// Also apply fade if No vol env
			// AND sustain off

			// Note off issued?
			if((slave->Flags & 4) == 0)
				return UpdateInstruments5(ite, slave);
		}
	}

	if(!jump_ins17)
	{
		// in the original, this line is skipped when the bit is set
		// maintaining that would just make for a horrible flow --GM
		slave->Flags |= 8;

		slave->FadeOut -= ins->FadeOut;
		if(slave->FadeOut <= 0)
		{
			slave->FadeOut = 0;
		} else {
			slave->Flags |= 16; // Recalc volume flag.
			return UpdateInstruments5(ite, slave);
		}
	}

	// Turn off channel
	if((slave->HCN & 0x80) == 0)
	{
		slave->HCN |= 0x80;
		// Host channel exists
		ite->chn[slave->HCOffst].Flags &= ~4;
	}

	slave->Flags |= 0x0200;
	slave->Flags |= 16; // Recalc volume flag.

	return UpdateInstruments5(ite, slave);
}

void UpdateInstruments5(it_engine *ite, it_slave *slave)
{
	if((slave->Flags & 16) != 0)
	{
		// Calculate volume
		slave->Flags &= ~16;
		//Mov     DX, Word Ptr [SoloSample]       ; DL = sample, DH = inst

		slave->Flags |= 64; // Recalc final volume
		uint32_t eax = slave->Vol; // Note volume...
		eax *= (uint32_t)slave->CVl; // Channel volume
		// AX = (0->4096)

		if(ite->SoloSample != 0xFF && ite->SoloSample != slave->Smp)
			slave->Flags |= 0x0800;
		else if(ite->SoloInstrument != 0xFF && ite->SoloInstrument != slave->Ins)
			slave->Flags |= 0x0800;

		eax *= slave->FadeOut;
		// Fadeout Vol (0->1024)
		// DX:AX = 0->4194304

		eax >>= 7; // AX = (0->32768)

		eax *= slave->SVl; // Sample volume
		// DX:AX = 0->4194304

		eax >>= 7; // AX = 0->32768

		eax *= (0xFFFF & (slave->V.EnvelopeValue>>8)); // VEV, 0->64*256
		// DX:AX = 0->536870912

		eax >>= 14; // AX = 0->32768

		eax *= ite->GlobalVolume; // DX:AX = 0->4194304

		eax >>= 7;

		//printf("VOL %04X %02X\n", slave->_16bVol, slave->FV);
		slave->FV = eax>>8;
		slave->_16bVol = eax;
	}

	// Change in panning?
	if((slave->Flags & 2) != 0)
	{
		slave->Flags &= ~2;
		slave->Flags |= 0x8000;

		int16_t ax = slave->Pan; // actual pan
		if(ax != 100)
		{
			// some really funky stuff going on here.
			// i could possibly simplify this but i'd like to be accurate.
			// so, here we go for now --GM
			ax = 32 - ax;
			ax ^= (ax>>8);
			ax = (ax & 0xFF00) | ((ax - (ax>>8)) & 0xFF);
			// AL = |32-ActualPan|

			ax = (ax & 0xFF00) | ((32 - ax) & 0xFF);

			ax = (ax & 0xFF) * (int16_t)(int8_t)(
				slave->Pt.EnvelopeValue>>16); // Pan envelope..
			ax >>= 5;
			ax = (ax & 0xFF00) + ((ax + slave->Pan) & 0xFF);

			// Value to show..
			slave->Pan = ax;

			ax = (ax & 0xFF00) + ((ax - 32) & 0xFF);

			// Pan = (Pan-32)* Separation/128 + 32
			uint8_t sep = ite->hdr.Sep; // 0->64 (Separation)
			sep >>= 1;
			ax = (ax & 0xFF) & (int8_t)sep;
			// AX = -2048->+2048
			// ie. AH = -8->+8
			ax >>= 6; // AL = -32->+32

			if(ite->d.ReverseChannels != 0)
				ax = -ax;

			ax += 32;

		} else {
			slave->FP = ax;
		}

		slave->FPP = ax;
	}

	UpdateVibrato(ite, slave);

	if(slave->Smp == 100) // MIDI?
		ite->DoMIDICycle = 1;
}

void UpdateData(it_engine *ite)
{
	if(ite->PlayMode == 1)
		return UpdateData_PlayMode1(ite);
	else if(ite->PlayMode < 1)
		return UpdateData_PlayMode0(ite);
	else
		return UpdateData_PlayMode2(ite);
}

void UpdateData_PlayMode0(it_engine *ite)
{
	uint16_t cx;
	it_host *chn;

	// Step through each channel.
	// Check if on/updatemode OK
	for(cx = 64, chn = &ite->chn[0]; cx != 0; cx--, chn++)
	{
		if(chn->CUC == 0) continue;

		chn->CUC--; // Handle counter

		if(chn->CUC != 0xFF)
			chn->Flags &= ~0x303; // Turn off mode.

		// Mov     AX, [DI]

		// Channel on? / Don't update effect?
		if((chn->Flags & 4) != 0 && (chn->Flags & 0x100) != 0)
			VolumeEffectTable[chn->VCm & 7](ite, chn);

		if((chn->Flags & 2) != 0 // Update effect regardless..
			|| ((chn->Flags & 4) != 0 && (chn->Flags & 1) != 0))
		{
			// OK. now handle counter.
			CommandTable[chn->Cmd & 31](ite, chn);
		}

		// Progress to next channel
	}

	// That's all!
}

void UpdateData_NoNewRow(it_engine *ite)
{
	it_host *chn;
	uint16_t cx;

	// OK. call update command.

	for(chn = &ite->chn[0], cx = 64; cx != 0; cx--, chn++)
	{
		if((chn->Flags & 4) != 0 && (chn->Flags & 0x100) != 0)
			VolumeEffectTable[chn->VCm & 7](ite, chn);

		if((chn->Flags & 3) == 0) continue;
		if((chn->Flags & 2) == 0 && (chn->Flags & 4) == 0) continue;

		//printf("%i %04X\n", chn->Flags);
		CommandTable[chn->Cmd & 31](ite, chn);
	}
}

void UpdateData_PlayMode1(it_engine *ite)
{
	uint16_t cx;

	// Pattern stuff..
	ite->ProcessTick--;
	ite->CurrentTick--;
	if(ite->CurrentTick != 0)
	{
		UpdateData_NoNewRow(ite);
		return;
	}

	// OK... have to update row.
	ite->CurrentTick = ite->ProcessTick = ite->CurrentSpeed;

	ite->RowDelay--;
	if(ite->RowDelay != 0)
		return UpdateEffectData(ite);

	ite->RowDelay = 1;

	uint16_t ax = ite->ProcessRow+1; // Progress to new row.
	if(ax >= ite->NumberOfRows)
	{
		if(ite->d.StopEndOfPlaySection != 0)
		{
			Music_Stop(ite); // Optionally.. loop!
			return;
		}

		// Wrap row.
		ax = ite->BreakRow;
		ite->BreakRow = 0;
	}

	ite->ProcessRow = ite->CurrentRow = ax;

	// Gotta get note data.
	UpdateNoteData(ite);
	return;
}

void UpdateEffectData(it_engine *ite)
{
	uint16_t cx;
	it_host *chn;

	for(chn = &ite->chn[0], cx = 64; cx != 0; cx--, chn++)
	{
		if((chn->Flags & 64) == 0) continue;
		if((chn->Msk & 0x88) == 0) continue;

		uint8_t oldmsk = chn->Msk;
		chn->Msk &= 0x88;

		InitCommandTable[chn->Cmd & 0x1F](ite, chn); // Init note

		chn->Msk = oldmsk;
	}
}

void UpdateData_PlayMode2(it_engine *ite)
{
	ite->ProcessTick--;
	ite->CurrentTick--;
	if(ite->CurrentTick != 0)
	{
		UpdateData_NoNewRow(ite);
		return;
	}

	// Play song stuff...

	ite->CurrentTick = ite->ProcessTick = ite->CurrentSpeed;

	ite->RowDelay--;
	if(ite->RowDelay != 0)
		return UpdateEffectData(ite);

	ite->RowDelay = 1;

	uint16_t ax = ite->ProcessRow+1;
	if(ax < ite->NumberOfRows)
	{
		ite->ProcessRow = ite->CurrentRow = ax;
		UpdateNoteData(ite);
		return;
	}

	if((ite->OrderLockFlag & 1) == 0)
	{
		int dx = 0;

		// Get new pattern.
		int16_t bx = ite->ProcessOrder + 1;
		uint8_t cl = 64; // might as well emulate any side-effects we get --GM

		for(;;)
		{
			if(bx < 0x100)
			{
				// next pattern
				cl = ite->ord[bx]; // CL = next pattern.
				if(cl < 200)
					break;

				bx++;

				if(cl == 0xFE) continue;

				ite->StopSong = 1;
				if(ite->d.StopEndOfPlaySection != 0)
				{
					Music_Stop(ite); // Optionally.. loop!
					return;
				}
			}

			if(dx != 0)
			{
				Music_Stop(ite); // Optionally.. loop!
				return;
			}

			dx++;
			bx = 0;
		}

		ite->ProcessOrder = ite->CurrentOrder = bx;
		ite->CurrentPattern = cl;
	}

	ite->ProcessRow = ite->CurrentRow = ite->BreakRow;
	ite->BreakRow = 0;
	UpdateNoteData(ite);
}

uint16_t Music_GetNumberOfSamples(it_engine *ite)
{
	// Returns AX
	uint16_t ax = 99;
	it_sample *smp = &ite->smp[99-1];

	while(ax > 0)
	{
		if(memcmp(smp, SampleHeader, 80))
			break;

		ax--;
		smp--;
	}

	return ax;
}

uint16_t Music_GetNumberOfInstruments(it_engine *ite)
{
	// Returns AX
	uint16_t ax = 99;
	it_instrument *ins = &ite->ins[99-1];

	while(ax > 0)
	{
		if(memcmp(ins, InstrumentHeader, 80))
			break;

		ax--;
		ins--;
	}

	return ax;
}

it_sample *Music_GetSampleHeader(it_engine *ite, uint16_t ax)
{
	// AX = sample, 1 based
	return &ite->smp[ax-1];
}

uint8_t *Music_GetSampleLocation(it_engine *ite, uint16_t ax, uint32_t *rcx, int *is8bit)
{
	// AX = sample (1based)
	// CH = page.
	// Returns DS:ESI
	//   ECX = length
	// Carry if no sample. - (using NULL --GM)
	// Zero set if 8 bit     (using a flag --GM)

	ite->LastSample = ax;

	it_sample *smp = &ite->smp[ax-1];
	if((smp->Flg & 1) == 0)
		return NULL;

	if(ite->SamplePointer[ax-1] == NULL)
		return NULL;

	uint8_t *data = ite->SamplePointer[ax-1];

	*rcx = smp->Length;
	*is8bit = ((~smp->Flg) & 1);

	return data;
}

// Accessed via Int 3
uint8_t *Music_UpdateSampleLocation(it_engine *ite, uint32_t esi, int *is8bit)
{
	// Reqs ESI.
	// TODO: decipher this crap *properly* --GM

	uint16_t ax = ite->LastSample;
	uint32_t dummy_ecx;
	return Music_GetSampleLocation(ite, ax, &dummy_ecx, is8bit);
}

uint8_t *Music_FarUpdateSampleLocation(it_engine *ite, uint32_t esi, int *is8bit)
{
	return Music_UpdateSampleLocation(ite, esi, is8bit);
}

void Music_GetPlayMode(it_engine *ite, uint16_t *PlayMode, uint16_t *CurrentRow,
	uint16_t *CurrentPattern, uint16_t *CurrentOrder, uint16_t *NumberOfRows)
{
	*PlayMode = ite->PlayMode;
	*CurrentRow = ite->CurrentRow;
	*CurrentPattern = ite->CurrentPattern;
	*CurrentOrder = ite->CurrentOrder;
	*NumberOfRows = ite->NumberOfRows;
}

void Music_GetPlayMode2(it_engine *ite, uint16_t *PlayMode, uint16_t *CurrentRow,
	uint16_t *CurrentTick, uint32_t *CurrentOrder)
{
	*PlayMode = ite->PlayMode;
	*CurrentOrder = ite->CurrentOrder;
	*CurrentRow = ite->CurrentRow;
	*CurrentTick = ite->CurrentTick;
}

void Music_PlayPattern(it_engine *ite, uint16_t pidx, uint16_t numrows, uint16_t startrow)
{
	// AX = pattern, BX = number of rows
	// CX = row to start

	Music_Stop(ite);

	ite->MIDIPitchDepthSent = 0;
	ite->LastMIDIByte = 0xFF;
	ite->CurrentPattern = pidx;
	ite->CurrentRow = startrow;
	ite->NumberOfRows = numrows;
	ite->ProcessRow = startrow-1;
	ite->PlayMode = 1;
}

void Music_PlaySong(it_engine *ite, uint16_t oidx)
{
	// AX = Order

	Music_Stop(ite);

	ite->MIDIPitchDepthSent = 0;
	ite->LastMIDIByte = 0xFF;
	ite->CurrentOrder = oidx;
	ite->ProcessOrder = oidx-1;
	ite->ProcessRow = 0xFFFE;
	ite->PlayMode = 2;

	StartClock(ite);

	MIDITranslate(ite, ite->chn, ite->slave, MIDICOMMAND_START);
}

void Music_PlayPartSong(it_engine *ite, uint16_t oidx, uint16_t row)
{
	// AX = order, BX = row.

	Music_Stop(ite);

	ite->NumberOfRows = 200;
	ite->ProcessOrder = oidx;
	ite->CurrentOrder = oidx;
	ite->CurrentRow = row;
	ite->ProcessRow = row-1;

	ite->CurrentPattern = ite->ord[oidx];

	ite->PlayMode = 2;

	StartClock(ite);
}

void Music_KBPlaySong(it_engine *ite)
{
	if(ite->PlayMode != 2)
		Music_PlaySong(ite, 0);
}

void Music_StopChannels(it_engine *ite)
{
	uint16_t cx;
	it_host *chn;
	it_slave *slave;

	for(chn = &ite->chn[0], cx = 64; cx != 0; cx--, chn++)
	{
		chn->Flags = 0;
		chn->PLR = 0;
		chn->PLC = 0;
	}

	for(slave = &ite->slave[0], cx = MAXSLAVECHANNELS; cx != 0; cx--, slave++)
	{
		if((slave->Flags & 1) != 0 && slave->Smp == 100)
			MIDITranslate(ite, &ite->chn[slave->HCOffst], slave, MIDICOMMAND_STOPNOTE);

		slave->Flags = 0x200;
	}
}

void Music_Stop(it_engine *ite)
{
	uint16_t cx;
	it_host *chn;
	it_slave *slave;

	// Turn off MIDI channels first.
	if((ite->OrderLockFlag & 1) != 0)
		Music_ToggleOrderUpdate(ite);

	for(slave = &ite->slave[0], cx = MAXSLAVECHANNELS; cx != 0; cx--, slave++)
	{
		if((slave->Flags & 1) != 0 && slave->Smp == 100)
			MIDITranslate(ite, &ite->chn[slave->HCOffst], slave, MIDICOMMAND_STOPNOTE);
	}

	// Stop
	MIDITranslate(ite, &ite->chn[0], &ite->slave[0], MIDICOMMAND_STOP);

	ite->PlayMode = 0;

	ite->DecodeExpectedPattern = 0xFFFE;
	ite->DecodeExpectedRow = 0xFFFE;
	ite->RowDelay = 1;
	ite->CurrentRow = 0;
	ite->CurrentOrder = 0;
	ite->CurrentTick = 1;
	ite->BreakRow = 0;

	memset(ite->MIDIPrograms, 0xFF, 32*2);

	uint8_t dl;
	uint8_t dh;
	uint16_t ax;
	for(chn = &ite->chn[0], dl = 64, dh = 0; dl != 0; dl--, chn++, dh++)
	{
		chn->Flags = 0; chn->Msk = 0; chn->Nte = 0;
		chn->Ins = 0; chn->Vol = 0; chn->Cmd = 0; chn->CVal = 0;
		chn->OCm = 0; chn->OVal = 0; chn->VCm = 0; chn->VVal = 0;
		chn->MCh = 0; chn->MPr = 0; chn->Nt2 = 0; chn->Smp = 0;

		chn->DKL = 0; chn->EFG = 0; chn->O00 = 0; chn->I00 = 0;
		chn->J00 = 0; chn->M00 = 0; chn->N00 = 0; chn->P00 = 0;
		chn->Q00 = 0; chn->T00 = 0; chn->S00 = 0; chn->OxH = 0;
		chn->W00 = 0; chn->VCE = 0; chn->GOE = 0; chn->SFx = 0;

		chn->HCN = dh; chn->CUC = 0; chn->VSe = 0; chn->LTr = 0;
		chn->SCOffst = 0; chn->PLR = 0; chn->PLC = 0;
		chn->PWF = 0; chn->PPo = 0; chn->PDp = 0; chn->PSp = 0;
		chn->LPn = 0; chn->LVi = 0; 
		chn->CP = ite->hdr.Chnl_Pan[dh] & 0x7F; chn->CV = ite->hdr.Chnl_Vol[dh];
		
		chn->VCh = 0; chn->TCD = 0; chn->Too = 0; chn->RTC = 0;
		chn->Porta_Frequency = 0;
		chn->VWF = 0; chn->VPo = 0; chn->VDp = 0; chn->VSp = 0;
		chn->TWF = 0; chn->TPo = 0; chn->TDp = 0; chn->TSp = 0;

		chn->_40 = 0; chn->_42 = 0;
		chn->_44 = 0; chn->_46 = 0; chn->_47 = 0;
		chn->_48 = 0; chn->_49 = 0; chn->_4A = 0; chn->_4B = 0;
		chn->_4C = 0; chn->_4D = 0; chn->_4E = 0; chn->_4F = 0;
	}

	// Now clear SlaveChannel
	//Mov     DX, MAXSLAVECHANNELS
	for(slave = &ite->slave[0], cx = MAXSLAVECHANNELS; cx != 0; cx--, slave++)
	{
		// too much bloody effort, using memset --GM
		memset(slave, 0, 128);
	}

	ite->GlobalVolume = ite->hdr.GV;
	ite->CurrentSpeed = ite->hdr.IS;
	ite->ProcessTick = ite->hdr.IS;
	ite->Tempo = ite->hdr.IT;

	Music_InitTempo(ite);
	MIDI_ClearTable(ite);
}

void Music_UpdatePatternOffset(it_engine *ite)
{
	ite->DecodeExpectedPattern = 0xFFFE;
}

void Music_PlayNote(it_engine *ite, uint8_t *data, uint16_t cidx, uint8_t dh)
{
	//DS:SI points to 5-note struct
	// AX = channel
	// DH = +32 means ignore mute
	//      settings
	// DH = +128 means to use central
	//       pan and max volume.

	it_host *chn = &ite->chn[cidx&0xFF];

	uint8_t dl = 0; // DL = mask
	uint8_t al;

	al = *(data++);
	if(al != NONOTE)
	{
		dl |= 1;
		chn->Nte = al;
	}

	al = *(data++);
	if(al != 0)
	{
		dl |= 2;
		chn->Ins = al;
	}

	al = *(data++);
	if(al != 0xFF)
	{
		dl |= 4;
		chn->Vol = al;
	}

	al = *(data++);
	uint8_t ah = *(data++);
	if(al != 0 || ah != 0)
		dl |= 8;

	chn->Cmd  = chn->OCm  = al;
	chn->CVal = chn->OVal = ah;
	chn->Msk = dl;
	chn->Flags &= ~(3+32+64+256);
	chn->Flags |= dh & 0x7F; // Now for command update count

	chn->CUC = ite->CurrentSpeed;
	PreInitCommand(ite, chn);

	if((chn->Flags & 4) != 0 && (dh & 128) != 0)
	{
		it_slave *slave = &ite->slave[chn->SCOffst];

		slave->Pan = slave->PS = 0x20; // Pan and pan set.
		slave->CVl = 0x40; // Full channel volume.
	}

	ite->DecodeExpectedRow = 0xFFFE;
}

void Music_PlaySample(it_engine *ite, uint8_t note, uint8_t sidx, uint16_t cidx)
{
	// AL = Note
	// AH = sample number
	// CX = channel.

	it_host *chn = &ite->chn[cidx];

	chn->Msk = 3; // Note & Sample
	chn->Nte = note;
	chn->Vol = 0xFF;
	chn->Ins = 0xFF;
	chn->Nt2 = note;
	chn->Smp = sidx;
	chn->Flags |= 0x8020;

	InitNoCommand(ite, chn);

	if((chn->Flags & 4) != 0)
	{
		it_slave *slave = &ite->slave[chn->SCOffst];
		slave->Pan = slave->PS = 0x20;
		slave->CVl = 0x40; // Full channel volume.
		slave->NNA = 0; // Note cut.
		slave->DCT = 0;
	}

	chn->Flags &= ~0x8000;

	ite->DecodeExpectedRow = 0xFFFE;
}

it_host *Music_GetHostChannelInformationTable(it_engine *ite)
{
	return &ite->chn[0];
}

it_slave *Music_GetSlaveChannelInformationTable(it_engine *ite, uint16_t *count)
{
	if(count != NULL) *count = MAXSLAVECHANNELS;
	return &ite->slave[0];
}

void Music_NextOrder(it_engine *ite)
{
	if(ite->PlayMode != 2)
		return;

	ite->PlayMode = 0;
	Music_StopChannels(ite);
	ite->ProcessRow = 0xFFFE;
	ite->CurrentTick = 1;
	ite->RowDelay = 1;
	ite->PlayMode = 2;
}

void Music_LastOrder(it_engine *ite)
{
	if(ite->PlayMode != 2) return;
	if(((int16_t)ite->ProcessOrder) <= 0) return;

	ite->PlayMode = 0;
	Music_StopChannels(ite);
	ite->ProcessOrder = ite->ProcessOrder - 2;
	ite->ProcessRow = 0xFFFE;
	ite->CurrentTick = 1;
	ite->RowDelay = 1;
	ite->PlayMode = 2;
}

void Music_SetGlobalVolume(it_engine *ite, uint8_t al)
{
	ite->GlobalVolume = al;
	RecalculateAllVolumes(ite);
}

void Music_MuteChannel(it_engine *ite, uint16_t ax)
{
	// AX = channel number
	uint16_t cx = ite->NumChannels;
	it_slave *slave = &ite->slave[0];

	for(; cx != 0; cx--, slave++)
	{
		if((slave->Flags & 1) == 0) continue;
		if((slave->HCN & 0x7F) != (ax & 0xFF)) continue;
		slave->Flags |= 0x0840;
	}
}

void Music_UnmuteChannel(it_engine *ite, uint16_t ax)
{
	// AX = channel number
	uint16_t cx = ite->NumChannels;
	ite->SoloSample = 0xFF;
	ite->SoloInstrument = 0xFF;
	it_slave *slave = &ite->slave[0];

	for(; cx != 0; cx--, slave++)
	{
		if((slave->Flags & 1) == 0) continue;
		if((slave->HCN & 0x7F) != (ax & 0xFF)) continue;
		slave->Flags &= ~0x0800;
		slave->Flags |= 64;
	}
}

void Music_ToggleChannel(it_engine *ite, uint16_t ax)
{
	// AX = channel number.

	if((ite->hdr.Chnl_Vol[ax] & 0x80) != 0)
	{
		ite->hdr.Chnl_Vol[ax] &= 0x7F;
		Music_UnmuteChannel(ite, ax);
		ite->MuteChannelTable[ax] = 0;
	} else {
		// Mute channel
		ite->MuteChannelTable[ax] ^= 1;
		ite->hdr.Chnl_Vol[ax] |= 0x80;
		Music_MuteChannel(ite, ax);
	}
}

void Music_UnmuteAll(it_engine *ite)
{
	// solo pressed on already soloed
	// channel -> turn everything on.
	uint16_t cx = 64;

	for(; cx != 0; cx--)
		if(ite->MuteChannelTable[cx-1] == 1)
			Music_ToggleChannel(ite, cx-1);
}

void Music_SoloChannel(it_engine *ite, uint16_t ax)
{
	// AX = channel

	// Check & count whether any playing.
	uint8_t *si = &ite->hdr.Chnl_Vol[0];
	uint16_t cx = 64;
	uint16_t dx = 64;
	for(; cx != 0; cx--)
		dx -= ((*(si++))>>7)&1; // no way to represent this properly in C --GM

	// DX = num playing.
	// check whether it's the current
	if(dx == 1 && (ite->hdr.Chnl_Vol[ax] & 0x80) == 0)
		return Music_UnmuteAll(ite);

	// 64 channel to step through
	// turn 'em all off.
	cx = 64;
	for(; cx != 0; cx--)
	{
		if(cx-1 == ax)
		{
			if((ite->hdr.Chnl_Vol[cx-1] & 0x80) == 0)
				Music_ToggleChannel(ite, ax);
		} else {
			if((ite->hdr.Chnl_Vol[cx-1] & 0x80) != 0)
				Music_ToggleChannel(ite, ax);
		}
	}
}

void Music_InitMuteTable(it_engine *ite)
{
	memset(&ite->MuteChannelTable[0], 0, 64);
	ite->SoloSample = 0xFF;
	ite->SoloInstrument = 0xFF;
	ite->AllocateNumChannels = 0;
}

void Music_InitStereo(it_engine *ite)
{
	ite->d.DriverSetStereo(ite, ite->hdr.Flags & 1);
	RecalculateAllVolumes(ite);
}

// note, increase means go faster, not increase number! --GM
uint16_t Music_IncreaseSpeed(it_engine *ite)
{
	// Returns AX = speed
	uint16_t ax = ite->CurrentSpeed;
	if(ax != 1)
	{
		ax--;
		ite->CurrentSpeed = ax;
		ite->hdr.IS = ax;
	}

	return ax;
}

uint16_t Music_DecreaseSpeed(it_engine *ite)
{
	// Returns AX = speed
	uint16_t ax = ite->CurrentSpeed;
	if(ax != 0xFF)
	{
		ax++;
		ite->CurrentSpeed = ax;
		ite->hdr.IS = ax;
	}

	return ax;
}

void Music_SetSoundCard(it_engine *ite, uint8_t al)
{
	// AL contains sound card num
	if(ite->DriverName != NULL) free(ite->DriverName);
	ite->DriverName = strdup(DriverNameTable[DriverSoundCard[al]]);
}

void Music_SetSoundCardDriver(it_engine *ite, const char *dssi)
{
	// improvised a bit here --GM
	if(ite->DriverName != NULL) free(ite->DriverName);
	ite->DriverName = strdup(dssi);
}

// exactly why am I doing this stuff? --GM
void Music_SetDMA(it_engine *ite, uint8_t al)
{
	ite->d.DMA = al;
}

void Music_SetMixSpeed(it_engine *ite, uint16_t cx)
{
	ite->d.CmdLineMixSpeed = cx;
}

void Music_SetIRQ(it_engine *ite, uint16_t cx)
{
	ite->d.IRQ = cx;
}

void Music_SetAddress(it_engine *ite, uint16_t dx)
{
	ite->d.BasePort = dx;
}

void Music_GetDisplayVariables(it_engine *ite, uint16_t *CurrentSpeed,
	uint16_t *Tempo, uint16_t *GlobalVolume)
{
	*CurrentSpeed = ite->CurrentSpeed;
	*Tempo = ite->Tempo;
	*GlobalVolume = ite->GlobalVolume;
}

int16_t Music_AssignSampleToInstrument(it_engine *ite, uint16_t bx)
{
	uint16_t cx;
	int i;

	// BX = sample num
	// returns AX
	// (returns -1 instead of carry on error --GM)

	// Check for sample-number's instrument first.
	it_instrument *ins = &ite->ins[bx];

	uint16_t ax = bx+1;

	if(memcmp(ins, InstrumentHeader, 554))
	{
		// Search
		cx = 99;

		ins = &ite->ins[0]; // Points to first inst.
		ax = 1;

		for(; cx != 0; cx--, ax++, ins++)
			if(!memcmp(ins, InstrumentHeader, 554))
				break;

		if(cx == 0)
			return -1;
	}

#if NETWORKENABLED
	uint8_t *nsq = Network_GetSendQueue(ite);

	if(nsq != NULL)
	{
		*(nsq++) = 0x00;
		*(nsq++) = 0x04;
		*(nsq++) = ax-1;
	}

	Network_FinishedSendQueue(ite, nsq);
#endif

	ins = &ite->ins[ax-1];
	it_sample *smp = &ite->smp[bx];

	memcpy(ins->Name, smp->Name, 26);

	// Now to fill in table.
	bx++;
	for(cx = 120, i = 0; cx != 0; cx--, i++)
		ins->NoteSamp[i][1] = bx;

	return ax;
}

void Music_SetLimit(it_engine *ite, uint16_t cx)
{
	ite->CmdLineNumChannels = cx;
}

void Music_ReverseChannels(it_engine *ite, uint16_t cx)
{
	ite->d.ReverseChannels = 1;
}

void Music_IncreaseVolume(it_engine *ite)
{
	if(ite->GlobalVolume < 128)
	{
		ite->GlobalVolume++;
		RecalculateAllVolumes(ite);
	}
}
void Music_DecreaseVolume(it_engine *ite)
{
	if(ite->GlobalVolume != 0)
	{
		ite->GlobalVolume--;
		RecalculateAllVolumes(ite);
	}
}

void Music_RegetLoopInformation(it_engine *ite)
{
	uint16_t cx;
	it_slave *slave;
	
	for(cx = ite->NumChannels, slave = &ite->slave[0]; cx != 0; cx--, slave++)
	{
		if((slave->Flags & 1) == 0) continue;

		GetLoopInformation(ite, slave);
		slave->Flags |= 0x40;
	}
}

void ResetSoundCardMemory(it_engine *ite)
{
	ite->d.DriverResetMemory(ite);
}

int Music_SoundCardLoadSample(it_engine *ite, uint16_t sidx)
{
	// AX = sample number
	// (1 based)
	// Carry set if insuf mem

	// incorrect - it's CLEAR on insufficient memory
	// anyway, we're just returning -1 on fail, 0 on pass --GM

	if(ite->d.DriverLoadSample(ite, sidx) == -1)
	{
		M_Object1List(ite, O1_OutOfSoundCardMemoryList, 2);
		return -1;
	}

	return 0;
}

uint16_t Music_SoundCardLoadAllSamples(it_engine *ite)
{
	Music_Stop(ite);
	S_SaveScreen(ite);

	S_DrawBox(ite, 3, 30, 50, 28, 30);
	S_DrawString(ite, 32, 29, PrepareSamplesMsg, 0x20);
	S_UpdateScreen(ite);

	ResetSoundCardMemory(ite);

	uint16_t ax;
	for(ax = 1; ax <= 100; ax++)
		if(Music_SoundCardLoadSample(ite, ax) != 0)
			return 1;

	S_RestoreScreen(ite);

	return 1;
}

int Music_GetFreeSoundCardMemory(it_engine *ite)
{
	// TODO: find out how this works --GM
	return ite->d.DriverGetStatus(ite);
}

uint16_t Music_GetNumChannels(it_engine *ite)
{
	return ite->NumChannels;
}

const uint32_t *Music_GetPitchTable(it_engine *ite)
{
	// Returns ES:DI to pitch table
	return PitchTable;
}

void Music_ToggleReverse(it_engine *ite)
{
	ite->d.ReverseChannels ^= 1;
	RecalculateAllVolumes(ite);
	SetInfoLine(ite, ReverseMsg);
}

void Music_PatternStorage(it_engine *ite, uint8_t al)
{
	// this won't mean a damn thing --GM
	ite->PatternStorage = al;
}

void Music_InitMixTable(it_engine *ite)
{
	ite->d.DriverSetMixVolume(ite, ite->hdr.MV); // AL = 0->128
}

uint16_t Music_GetTempo(it_engine *ite)
{
	// returns in BX --GM
	return ite->Tempo;
}

uint16_t Music_GetLastChannel(it_engine *ite)
{
	// Returns AX
	uint16_t cx = 64;
	uint16_t ax = 0;
	uint16_t dx = 0;

	for(; cx != 0; cx--, dx++)
		if(((ite->hdr.Chnl_Vol[dx]>>7)^(ite->MuteChannelTable[dx])) == 0)
			ax = dx;
	
	return ax;
}

int Music_GetDriverScreen(it_engine *ite)
{
	return ite->d.DriverSoundCardScreen(ite);
}

int Music_GetDriverVariable(it_engine *ite, uint16_t Var)
{
	return ite->d.DriverGetVariable(ite, Var);
}

int Music_SetDriverVariable(it_engine *ite, uint16_t Var, uint16_t Thing)
{
	return ite->d.DriverSetVariable(ite, Var, Thing);
}

void Music_SetNextOrder(it_engine *ite, uint16_t order)
{
	ite->ProcessOrder = order-1;
}

uint16_t Music_GetDelay(it_engine *ite)
{
	// returns in DX --GM

	// in case you're wondering,
	// this generates SDx delay effect information for liveplay --GM

	if(ite->PlayMode == 0)
		return 0;

	uint16_t dx = ite->CurrentSpeed;

	if(dx == ite->CurrentRow)
	{
		dx -= ite->ProcessTick;
		if(dx == 0) return 0;

		if(dx >= 0x0F)
			dx = 0x0F;
	} else {
		dx--;

		if(dx >= 0x0F)
			dx = 0x0F;
	}

	dx <<= 8;
	dx |= 'S'-'@' + 0xD000;

	return dx;
}

int InternalTimer(it_engine *ite, uint16_t bx)
{
	// Ticks = (1193181/(2*0.4))/Tempo
	uint32_t eax = 0x16C214 / (uint32_t)bx;
	ite->TimerCounter = eax*2;

	return 0;
}

int Music_TimeSong(it_engine *ite)
{
	// Time song!

	S_SaveScreen(ite);

	S_SetDirectMode(ite, 1);
	S_DrawSmallBox(ite);

	S_DrawString(ite, 33, 26, PleaseWaitMsg, 0x20);

	Music_Stop(ite);

	/*
	Mov     CX, 0FFFFh

Music_TimeSong1:
	In      AL, 21h                 // Delay mechanism
	In      AL, 0A1h                 // Delay mechanism
	Loop    Music_TimeSong1
	*/

	ite->StopSong = 0;
	ite->TotalTimer = 0;
	ite->TotalTimerHigh = 0;

	// appears to disable interrupts --GM
	/*
	ClI
	In      AL, 0A1h
	Mov     AH, AL
	In      AL, 21h
	Push    AX

	Mov     AL, 0FFh
	Out     0A1h, AL
	Out     21h, AL
	*/

	int (*OldDriverSetTempo)(it_engine *ite, uint16_t Tempo) = ite->d.DriverSetTempo;
	ite->d.DriverSetTempo = InternalTimer;
	uint16_t OldNumChannels = ite->NumChannels;
	uint16_t OldFlags = ite->hdr.Flags;
	ite->hdr.Flags &= ~4;

	Music_PlaySong(ite, 0);

	for(;;)
	{
		uint16_t cx;
		it_slave *si;
		uint16_t ax;
		Update(ite, &cx, &si, &ax);

		if(ite->StopSong != 0) break;

		ite->TotalTimer += ite->TimerCounter;
		if(ite->TotalTimer < ite->TimerCounter) // cheeky way to do AdC 0 --GM
			ite->TotalTimerHigh++;
	}

	Music_Stop(ite);

	ite->hdr.Flags = OldFlags;
	ite->NumChannels = OldNumChannels;
	ite->d.DriverSetTempo = OldDriverSetTempo;

	/*
	Pop     AX
	Out     21h, AL
	Mov     AL, AH
	Out     0A1h, AL

	StI
	*/

	S_SetDirectMode(ite, 0);

	S_RestoreScreen(ite);

	M_Object1List(ite, O1_ShowTime, 0xFFFF);

	return 1;
}

void Music_ShowTime(it_engine *ite)
{
	S_GetDestination(ite); // TODO: work this out --GM

	uint32_t xtime = (ite->TotalTimerHigh<<16) | (ite->TotalTimer>>16);

	D_ShowTime(ite, 43, 27, xtime);
}

uint16_t Music_GetPatternLength(it_engine *ite)
{
	return ite->NumberOfRows;
}

uint16_t Music_SaveMIDIConfig(it_engine *ite)
{
	D_GotoStartingDirectory(ite);

	// TODO!
	/*
	Mov     AH, 3Ch
	Xor     CX, CX
	Mov     DX, Offset MIDIConfigFileName
	Int     21h
	JC      Music_SaveMIDIConfig1

	Mov     BX, AX

	Mov     AH, 40h
	Mov     DS, CS:MIDIDataArea
	Xor     DX, DX
	Mov     CX, (128+16+9)*32
	Int     21h

	Mov     AH, 3Eh
	Int     21h

Music_SaveMIDIConfig1:
	Xor     AX, AX
	*/
	return 0;
}

uint8_t *Music_GetMIDIDataArea(it_engine *ite)
{
	// TODO!
	//Mov     DS, CS:MIDIDataArea
	return NULL;
}

void Music_ToggleOrderUpdate(it_engine *ite)
{
	const char *msg = OrderUpdateEnabledMsg;
	ite->OrderLockFlag ^= 1;
	if(ite->OrderLockFlag != 0)
		msg = OrderUpdateDisabledMsg;
	
	SetInfoLine(ite, msg);
}

uint16_t Music_ToggleSoloInstrument(it_engine *ite)
{
	return Music_ToggleSolo(ite, SoloInstrumentMsg, &ite->SoloInstrument, 1);
}

uint16_t Music_ToggleSoloSample(it_engine *ite)
{
	return Music_ToggleSolo(ite, SoloSampleMsg, &ite->SoloSample, 0);
}

uint16_t Music_ToggleSolo(it_engine *ite, const char *msg, uint8_t *v, uint16_t bp)
{
	uint16_t bx = PE_GetLastInstrument(ite);
	uint16_t ax = bx;
	bx += bp;

	if(*v != (uint8_t)bx)
	{
		ite->SoloSample = 0xFF;
		ite->SoloInstrument = 0xFF;
		ax++;
		*v = bx;
	} else {
		ite->SoloSample = 0xFF;
		ite->SoloInstrument = 0xFF;
		msg = UnsoloMsg;
	}

	SetInfoLine(ite, msg);

	uint16_t cx;
	it_slave *slave;

	bx = 0;

	for(cx = ite->NumChannels, slave = &ite->slave[0]; cx != 0; cx--, slave++)
	{
		slave->Flags |= 18;
		bx = slave->HCN; // BX = channel

		if((ite->hdr.Chnl_Vol[bx] & 0x80) == 0)
			slave->Flags &= ~0x800;
	}

	RecalculateAllVolumes(ite);

	return 1;
}

