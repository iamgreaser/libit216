#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include <assert.h>

#include "switch.h"
#include "network.h"

#define Trace(s) puts(s)

#define CREATENEWLOGFILE             0
#define HOSTCHANNELSIZE              80
#define SLAVECHANNELSIZE             128
#define MAXSLAVECHANNELS             256
#define NONOTE                       0xFD

#define MIDICOMMAND_START                    0x0000
#define MIDICOMMAND_STOP                     0x0020
#define MIDICOMMAND_TICK                     0x0040
#define MIDICOMMAND_PLAYNOTE                 0x0060
#define MIDICOMMAND_STOPNOTE                 0x0080
#define MIDICOMMAND_CHANGEVOLUME             0x00A0
#define MIDICOMMAND_CHANGEPAN                0x00C0
#define MIDICOMMAND_BANKSELECT               0x00E0
#define MIDICOMMAND_PROGRAMSELECT            0x0100
#define MIDICOMMAND_CHANGEPITCH              0xFFFF

typedef struct it_host_s
{
	// 0x0000
	uint16_t Flags; uint8_t Msk, Nte;
	uint8_t Ins, Vol, Cmd, CVal;
	uint8_t OCm, OVal, VCm, VVal;
	uint8_t MCh, MPr, Nt2, Smp;

	// 0x0010
	uint8_t DKL, EFG, O00, I00;
	uint8_t J00, M00, N00, P00;
	uint8_t Q00, T00, S00, OxH;
	uint8_t W00, VCE, GOE, SFx;

	// 0x0020
	uint8_t HCN, CUC, VSe, LTr;
	uint16_t SCOffst; uint8_t PLR, PLC;
	uint8_t PWF, PPo, PDp, PSp;
	uint8_t LPn, LVi, CP, CV;

	// 0x0030
	uint8_t VCh, TCD, Too, RTC;
	uint32_t Porta_Frequency;
	uint8_t VWF, VPo, VDp, VSp;
	uint8_t TWF, TPo, TDp, TSp;

	// 0x0040
	// Misc Effect Data......
	int16_t _40, _42;
	int16_t _44; uint8_t _46, _47;
	uint8_t _48, _49, _4A, _4B;
	uint8_t _4C, _4D, _4E, _4F;

} __attribute__((__packed__)) it_host;

typedef struct it_slen_s
{
	uint32_t EnvelopeValue;
	uint32_t EnvelopeDelta;
	uint16_t EnvPos, CurEnN;
	uint16_t NextET, filter;
} __attribute__((__packed__)) it_slen;

typedef struct it_slave_s
{
	// 0x0000
	uint16_t Flags;
	uint8_t DeviceSpecific[8];
	uint8_t LpM, LpD;
	uint32_t Left_Volume;

	// 0x0010
	uint32_t Frequency;
	uint32_t Frequency_Set;
	uint8_t Bit, ViP; uint16_t ViDepth;
	uint32_t RVol_MIDIFSet;

	// 0x0020
	uint8_t FV, Vol, VS, CVl;
	uint8_t SVl, FP; int16_t FadeOut;
	uint8_t DCT, DCA, Pan, PS;
	uint32_t OldSampleOffset;

	// 0x0030
	uint16_t InsOffs; uint8_t Nte, Ins;
	uint16_t SmpOffs; uint8_t Smp, FPP;
	int16_t HCOffst;  uint8_t HCN, NNA;
	uint8_t MCh, MPr; uint8_t FCut, FRes; // 0x003E also 16-bit MBank

	// 0x0040
	uint32_t Loop_Beginning;
	uint32_t Loop_End;
	uint16_t SmpErr, _16bVol;
	uint32_t Sample_Offset;

	// 0x0050
	it_slen V;

	// 0x0060
	it_slen P;

	// 0x0070
	it_slen Pt;
} __attribute__((__packed__)) it_slave;

typedef struct it_header_s
{
	// 0x0000
	uint8_t magic[4]; // "IMPM"
	uint8_t SongName[26];

	// 0x001E
	uint16_t PHiligt;

	// 0x0020
	uint16_t OrdNum, InsNum;
	uint16_t SmpNum, PatNum;
	uint16_t Cwt_v, Cmwt;
	uint16_t Flags, Special;

	// 0x0030
	uint8_t GV, MV, IS, IT;
	uint8_t Sep, PWD; uint16_t MsgLgth;
	uint32_t Message_Offset;
	uint32_t Time_Stamp;

	// 0x0040
	uint8_t Chnl_Pan[64];

	// 0x0080
	uint8_t Chnl_Vol[64];
} __attribute__((__packed__)) it_header;

typedef struct it_envelope_s
{
	// 0x0000
	uint8_t Flg, Num, LpB, LpE;
	uint8_t SLB, SLE;

	// 0x0006
	uint8_t Nodes[25][3];

	//
	uint8_t pad1;
	
} __attribute__((__packed__)) it_envelope;

typedef struct it_instrument_s
{
	// 0x0000
	uint8_t magic[4]; // "IMPI"
	uint8_t FileName[13];

	// 0x0011
	uint8_t NNA, DCT, DCA;
	uint16_t FadeOut; int8_t PPS; uint8_t PPC;
	uint8_t GbV, DfP, RV, RP;
	uint16_t TrkVers; uint8_t NoS, _1F;

	// 0x0020
	uint8_t Name[26];

	// 0x003A
	uint8_t IFC, IFR;
	uint8_t MCh, MPr; uint16_t MIDIBnk;

	// 0x0040
	uint8_t NoteSamp[120][2];
	
	// 0x0130
	it_envelope VolEnv;

	// 0x0182
	it_envelope PanEnv;

	// 0x01D4
	it_envelope PitchEnv;

	//
	uint8_t pad1[7];
} __attribute__((__packed__)) it_instrument;

typedef struct it_sample_s
{
	// 0x0000
	uint8_t magic[4]; // "IMPS"
	uint8_t FileName[13];

	// 0x0011
	uint8_t GvL, Flg, Vol;
	uint8_t Name[26];

	// 0x002E
	uint8_t Cvt, DfP;

	// 0x0030
	uint32_t Length;
	uint32_t Loop_Begin;
	uint32_t Loop_End;
	uint32_t C5Speed;

	// 0x0040
	uint32_t SusLoop_Begin;
	uint32_t SusLoop_End;
	uint32_t SamplePointer;
	uint8_t ViS, ViD, ViR, ViT;

} __attribute__((__packed__)) it_sample;

typedef struct it_pattern_s
{
	uint16_t Length;
	uint16_t Rows;
	uint8_t pad1[4];
	uint8_t data[];
} __attribute__((__packed__)) it_pattern;

typedef struct it_engine_s it_engine;

typedef struct it_drvdata_s
{
	uint16_t BasePort                 ;//= 0xFFFF;          // * ORDER IS IMPORTANT
	uint16_t IRQ                      ;//= 0xFFFF;          // * ORDER IS IMPORTANT
	uint16_t DMA                      ;//= 0xFFFF;          // * ORDER IS IMPORTANT
	uint16_t CmdLineMixSpeed          ;//= 0;               // * ORDER IS IMPORTANT
	//uint16_t SongDataArea             = SongData;        // * ORDER IS IMPORTANT
	//uint16_t MIDIDataArea             = SongData + 4076;

	uint16_t CmdLineDMASize           ;//= 1024;           // default
	uint8_t  ReverseChannels          ;//= 0;
	uint16_t DriverMaxChannels;       // = 32;
	uint16_t StopEndOfPlaySection;    // = 0;
	uint16_t DefaultChannels;         // = 32;
	uint16_t DriverFlags;             // = 0;
	// Bit 1 = MIDI Out supported
	// Bit 2 = Hiqual
	// Bit 3 = Output waveform data available

	// Driver vectors
	const char *(*DriverDetectCard)(it_engine *ite, const char *fname, uint16_t AL, uint16_t version);
	const char *(*DriverInitSound)(it_engine *ite);
	int (*DriverReinitSound)(it_engine *ite);
	int (*DriverUninitSound)(it_engine *ite);

	int (*DriverPoll)(it_engine *ite, uint16_t PlayMode, uint16_t CurrentPattern); 
	int (*DriverSetTempo)(it_engine *ite, uint16_t Tempo);
	int (*DriverSetMixVolume)(it_engine *ite, uint16_t MixVolume);
	int (*DriverSetStereo)(it_engine *ite, uint16_t Stereo);

	int (*DriverLoadSample)(it_engine *ite, uint16_t sidx);
	int (*DriverReleaseSample)(it_engine *ite, it_sample *smp);
	int (*DriverResetMemory)(it_engine *ite);
	int (*DriverGetStatus)(it_engine *ite);

	int (*DriverSoundCardScreen)(it_engine *ite);
	int (*DriverGetVariable)(it_engine *ite, uint16_t Var);
	int (*DriverSetVariable)(it_engine *ite, uint16_t Var, uint16_t Thing); // TODO!

	// 14?
	int (*DriverMIDIOut)(it_engine *ite, uint8_t al);
	int (*DriverGetWaveform)(it_engine *ite);
} it_drvdata;

// This does not pertain to any specific IT structure
struct it_engine_s
{
	it_header hdr;
	it_host chn[64];
	it_slave slave[256];
	it_instrument ins[99];
	it_sample smp[99];
	it_pattern *pat[199];
	uint8_t ord[0x100];
	uint8_t patspace[64000];
	uint8_t *SamplePointer[99];

	uint16_t LastSample       ;//= 0;
	uint16_t PlayMode         ;//= 0;

	uint8_t  SaveFormat       ;//= DEFAULTFORMAT;
	uint16_t TimerData        ;//= 0;
	uint16_t NumTimerData     ;//= 0;
	uint16_t TopTimerData     ;//= 0;

	uint32_t EditTimer        ;//= 0;

	// Playmode 0 = Freeplay
	// Playmode 1 = Pattern
	// Playmode 2 = Song
	uint16_t CurrentOrder     ;//= 0;      // } Must follow
	uint16_t CurrentPattern   ;//= 0;      // }
	uint16_t CurrentRow       ;//= 0;      // }
	uint16_t ProcessOrder     ;//= 0;
	uint16_t ProcessRow       ;//= 0;
	uint16_t BytesToMix       ;//= 0;      // = Bytes per frame
	uint16_t PatternOffset    ;//= 0;
	uint16_t PatternSegment   ;//= 0;
	uint16_t BreakRow         ;//= 0;
	uint8_t  RowDelay         ;//= 0;      // } Must join on.
	uint8_t  RowDelayOn       ;//= 0;      // }
	uint8_t  PatternArray     ;//= 0;

	uint16_t PatternDataSegment;
	uint16_t CurrentEditPattern;
	uint16_t PatternEditMaxRow;

	uint16_t DecodeExpectedPattern    ;//= 0xFFFE;
	uint16_t DecodeExpectedRow        ;//= 0xFFFE;

	int16_t CmdLineNumChannels       ;//= 0xFFFF;

	uint16_t NumberOfRows             ;//= 64;      // Non zero globals
	uint16_t CurrentTick              ;//= 6;
	uint16_t CurrentSpeed             ;//= 6;
	uint16_t ProcessTick              ;//= 0;
	uint8_t  Tempo                    ;//= 125;
	uint8_t  GlobalVolume             ;//= 128;
	uint16_t NumChannels              ;//= 256;
	uint8_t  SoloSample               ;//= 0xFF;            // * ORDER IS IMPORTANT
	uint8_t  SoloInstrument           ;//= 0xFF;            // * ORDER IS IMPORTANT
	uint16_t AllocateNumChannels      ;//= 0;
	uint16_t AllocateSlaveOffset      ;//= 0;
	uint16_t LastSlaveChannel         ;//= 0;

	uint8_t  PatternLooping           ;//= 0;
	uint8_t  PatternStorage           ;//= 1;
	// 0 = conventional only
	// 1 = selective
	// 2 = EMS only.

	uint8_t  OrderLockFlag            ;//= 0;

	uint8_t  MuteChannelTable[64];
	uint8_t  ChannelCountTable[400];

	char    *DriverName               ;//= NULL;


	uint8_t  LastMIDIByte             ;//= 0xFF;
	uint16_t MIDIPitchDepthSent       ;//= 0x0000;

	uint16_t Seed1                    ;//= 0x1234;
	uint16_t Seed2                    ;//= 0x5678;

	uint8_t  DoMIDICycle              ;//= 0;

	uint8_t  MIDIPrograms[16]         ;//= 0xFF; // Do NOT change order!
	uint16_t MIDIBanks[16]            ;//= 0xFFFF;
	uint8_t  MIDIPan[16]              ;//= 0xFF;
	uint16_t MIDIPitch[16]            ;//= 0x2000;

	uint16_t ADSCParams[7]            ;//= 0;

	uint8_t  StopSong                 ;//= 0;
	uint32_t TimerCounter             ;//= 0;
	uint32_t TotalTimer               ;//= 0;
	uint32_t TotalTimerHigh           ;//= 0;

	// PE stuff
	uint16_t TopOrder                ;//= 0
	uint16_t Order                   ;//= 0
	uint16_t OrderCursor             ;//= 0
	uint16_t PatternNumber           ;//= 0
	uint16_t TopRow                  ;//= 0
	uint16_t Row                     ;//= 0
	uint16_t MaxRow                  ;//= 63
	//uint16_t NumberOfRows            ;//= 64

	uint8_t LastNote                ;//= 60
	uint8_t LastInstrument          ;//= 1
	uint8_t LastVolume              ;//= 0FFh
	uint8_t LastCommand             ;//= 0
	uint8_t LastCommandValue        ;//= 0

	it_drvdata d;
};

// it_music.c
it_engine *ITEngineNew();
void RecalculateAllVolumes(it_engine *ite);
void InitPlayInstrument(it_engine *ite, it_host *chn, it_slave *slave, int bx);
void ApplyRandomValues(it_engine *ite, it_host *chn);
void SetFilterCutoff(it_engine *ite, it_slave *slave, uint8_t bl);
void SetFilterResonance(it_engine *ite, it_slave *slave, uint8_t bl);
void MIDITranslate(it_engine *ite, it_host *chn, it_slave *slave, uint16_t bx);
it_slave *AllocateChannel(it_engine *ite, it_host *chn, uint8_t *ch);
uint16_t Random(it_engine *ite);
void GetLoopInformation(it_engine *ite, it_slave *slave);
void PitchSlideDown(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx);
void PitchSlideDownAmiga(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx);
void PitchSlideDownLinear(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx);
void PitchSlideUp(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx);
void PitchSlideUpLinear(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx);
void PitchSlideUpAmiga(it_engine *ite, it_host *chn, it_slave *slave, int16_t bx);
int Music_GetWaveForm(it_engine *ite);
void Music_Poll(it_engine *ite);
void Music_InitTempo(it_engine *ite);
void Music_ReinitSoundCard(it_engine *ite);
void Music_UnInitSoundCard(it_engine *ite);
void Music_InitMusic(it_engine *ite);
it_pattern *Music_AllocatePattern(it_engine *ite, uint16_t dx);
uint8_t *Music_AllocateSample(it_engine *ite, uint16_t ax, size_t edx);
void Music_ReleaseSample(it_engine *ite, uint8_t al, uint8_t ah);
void Music_ClearAllSampleNames(it_engine *ite);
void Music_ReleaseAllSamples(it_engine *ite);
void Music_ReleaseAllPatterns(it_engine *ite);
void Music_ClearAllInstruments(it_engine *ite);
void Music_UnInitMusic(it_engine *ite);
void Music_UnloadDriver(it_engine *ite);
void Music_ClearDriverTables(it_engine *ite);
int Music_LoadDriver(it_engine *ite, const char *fname);
const char *Music_AutoDetectSoundCard(it_engine *ite);
void Update(it_engine *ite, uint16_t *rcx, it_slave **si, uint16_t *ax);
void UpdateSamples(it_engine *ite);
void UpdateInstruments(it_engine *ite);
void UpdateInstruments16(it_engine *ite, it_slave *slave);
void UpdateInstruments5(it_engine *ite, it_slave *slave);
void UpdateData(it_engine *ite);
void UpdateData_PlayMode0(it_engine *ite);
void UpdateData_PlayMode1(it_engine *ite);
void UpdateEffectData(it_engine *ite);
void UpdateData_PlayMode2(it_engine *ite);
uint8_t *Music_GetSampleLocation(it_engine *ite, uint16_t ax, uint32_t *rcx, int *is8bit);
void Music_PlayPattern(it_engine *ite, uint16_t pidx, uint16_t numrows, uint16_t startrow);
void Music_PlaySong(it_engine *ite, uint16_t oidx);
void Music_PlayPartSong(it_engine *ite, uint16_t oidx, uint16_t row);
void Music_StopChannels(it_engine *ite);
void Music_Stop(it_engine *ite);
void Music_InitStereo(it_engine *ite);
uint16_t Music_SoundCardLoadAllSamples(it_engine *ite);
void Music_InitMixTable(it_engine *ite);
uint16_t Music_GetTempo(it_engine *ite);
void Music_ToggleOrderUpdate(it_engine *ite);
uint16_t Music_ToggleSolo(it_engine *ite, const char *msg, uint8_t *v, uint16_t bp);

extern const uint32_t PitchTable[];
extern const int8_t FineSineData[];
extern const int8_t FineRampDownData[];
extern const int8_t FineSquareWave[];
extern const uint32_t *LinearSlideUpTable;

// it_m_eff.c
void InitNoCommand(it_engine *ite, it_host *chn);
void InitCommandA(it_engine *ite, it_host *chn);
void InitCommandB(it_engine *ite, it_host *chn);
void InitCommandC(it_engine *ite, it_host *chn);
void InitCommandD(it_engine *ite, it_host *chn);
void InitCommandE(it_engine *ite, it_host *chn);
void InitCommandF(it_engine *ite, it_host *chn);
void InitCommandG(it_engine *ite, it_host *chn);
void InitCommandH(it_engine *ite, it_host *chn);
void InitCommandI(it_engine *ite, it_host *chn);
void InitCommandJ(it_engine *ite, it_host *chn);
void InitCommandK(it_engine *ite, it_host *chn);
void InitCommandL(it_engine *ite, it_host *chn);
void InitCommandM(it_engine *ite, it_host *chn);
void InitCommandN(it_engine *ite, it_host *chn);
void InitCommandO(it_engine *ite, it_host *chn);
void InitCommandP(it_engine *ite, it_host *chn);
void InitCommandQ(it_engine *ite, it_host *chn);
void InitCommandR(it_engine *ite, it_host *chn);
void InitCommandS(it_engine *ite, it_host *chn);
void InitCommandT(it_engine *ite, it_host *chn);
void InitCommandU(it_engine *ite, it_host *chn);
void InitCommandV(it_engine *ite, it_host *chn);
void InitCommandW(it_engine *ite, it_host *chn);
void InitCommandX(it_engine *ite, it_host *chn);
void InitCommandY(it_engine *ite, it_host *chn);
void InitCommandZ(it_engine *ite, it_host *chn);

void NoCommand(it_engine *ite, it_host *chn);
void CommandD(it_engine *ite, it_host *chn);
void CommandE(it_engine *ite, it_host *chn);
void CommandF(it_engine *ite, it_host *chn);
void CommandG(it_engine *ite, it_host *chn);
void CommandH(it_engine *ite, it_host *chn);
void CommandI(it_engine *ite, it_host *chn);
void CommandJ(it_engine *ite, it_host *chn);
void CommandK(it_engine *ite, it_host *chn);
void CommandL(it_engine *ite, it_host *chn);
void CommandN(it_engine *ite, it_host *chn);
void CommandP(it_engine *ite, it_host *chn);
void CommandQ(it_engine *ite, it_host *chn);
void CommandR(it_engine *ite, it_host *chn);
void CommandS(it_engine *ite, it_host *chn);
void CommandT(it_engine *ite, it_host *chn);
void CommandW(it_engine *ite, it_host *chn);
void CommandY(it_engine *ite, it_host *chn);
void VolumeCommandC(it_engine *ite, it_host *chn);
void VolumeCommandD(it_engine *ite, it_host *chn);
void VolumeCommandE(it_engine *ite, it_host *chn);
void VolumeCommandF(it_engine *ite, it_host *chn);
void VolumeCommandG(it_engine *ite, it_host *chn);

// it_disk.c & co
int D_LoadIT(it_engine *ite, const char *fname);

// unsorted
extern void *O1_OutOfSoundCardMemoryList;
extern void *O1_ShowTime;

// these can be filled in later, they're really simple and afaik purely for the editor
// - look in IT_I.ASM for the code --GM
#define I_TagSample(ite, idx)
#define I_TagInstrument(ite, idx)

// here's the screen stuff that needs implementing --GM
#define S_SaveScreen(ite)
#define S_DrawBox(ite, col, x1, y1, w, h) /* args unknown right now */
#define S_DrawString(ite, x, y, str, col)
#define S_UpdateScreen(ite)
#define S_RestoreScreen(ite)
#define S_SetDirectMode(ite, b)
#define S_DrawSmallBox(ite)
#define S_GetDestination(ite)

#define M_Object1List(ite, list, typ)
#define SetInfoLine(ite, str) printf("%s\n", (str));

#define D_GotoStartingDirectory(ite)
#define D_ShowTime(ite, x, y, time)

#define MIDI_ClearTable(ite)

#define StartClock(ite)

