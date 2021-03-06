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

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

static it_drvdata drv;
static int is_initialised = 0;
static int tempo = 125;
static int ossfmt = AFMT_S16_LE;
static int freq = 44100;
static int stereo = 1;
static int tper = 882;
static int dsp = -1;
static int32_t mixbuf[44100*2];
static int16_t outbuf[44100*2];

static const char *drv_oss_DriverDetectCard(it_engine *ite, const char *dname, uint16_t AL, uint16_t version)
{
	return NULL;
}

static const char *drv_oss_DriverInitSound(it_engine *ite)
{
	return NULL;
}

int drv_oss_DriverUninitSound(it_engine *ite)
{
	return 0;
}

static inline void kill_channel(it_engine *ite, it_slave *slave)
{
	//slave->Flags &= 0x788D;
	slave->Flags = 0x0200;
	slave->LpD = 0;

	if((slave->HCN & 0x80) == 0)
		ite->chn[slave->HCN].Flags &= ~4;
}

static inline int update_offs(it_engine *ite, it_slave *slave, int32_t *offs, int32_t *oferr, int32_t *nfreq, const int32_t lpbeg, const int32_t lpend)
{
	// TODO: use the actual mixer code (it's faster than this approach)
	// it's also possible that the ping pong stuff is completely broken now D:
	// --GM
	*oferr += *nfreq;
	*offs += *oferr>>16;
	*oferr &= 0xFFFF;

	if((slave->LpM & 24) == 24 && slave->LpD != 0 && (*offs < lpbeg))
	{
		*offs = lpbeg - *offs;
		if(lpend <= lpbeg)
		{
			// IT mixer doesn't kill here
			// Temporary measure pending IT mixer port
			kill_channel(ite, slave);
			return 1;
		}

		*offs %= (lpend-lpbeg)*2;
		if(*offs < (lpend-lpbeg))
		{
			slave->LpD = 0;
			if(*nfreq < 0) *nfreq = -*nfreq;
			*offs += lpbeg;
		} else {
			*offs = (lpend - lpbeg) - *offs;
			*offs += lpend - 1;
		}
	}

	if(*offs < 0)
	{
		if((slave->LpM & 24) != 24)
		{
			kill_channel(ite, slave);
			slave->LpD = 0;
			return 1;
		}

		*offs = 0;
		if(*nfreq < 0) *nfreq = -*nfreq;
		slave->LpD = 0;
	}

	if(*offs >= lpend)
	{
		if((slave->LpM & 8) == 0)
		{
			kill_channel(ite, slave);
			return 1;
		}

		if((slave->LpM & 24) == 24)
		{
			if(lpend <= lpbeg)
			{
				// IT mixer doesn't kill here
				// Temporary measure pending IT mixer port
				kill_channel(ite, slave);
				return 1;
			}

			*offs -= lpend;
			*offs %= (lpend-lpbeg)*2;
			if(*offs >= (lpend-lpbeg))
			{
				*offs -= (lpend - lpbeg);
				*offs += lpbeg;
			} else {
				slave->LpD = 1;

				*offs = lpend - *offs - 1;

				if(*nfreq > 0) *nfreq = -*nfreq;
			}

		} else {
			if(lpend <= lpbeg)
			{
				// IT mixer doesn't kill here
				// Temporary measure pending IT mixer port
				kill_channel(ite, slave);
				return 1;
			}

			*offs -= lpend;
			*offs %= (lpend-lpbeg);
			*offs += lpbeg;
		}
	}

	if(*offs < 0 || *offs >= lpend)
	{
		kill_channel(ite, slave);
		return 1;
	}

	return 0;

}

static int drv_oss_DriverPoll(it_engine *ite, uint16_t PlayMode, uint16_t CurrentPattern)
{
	it_host *chn;
	it_slave *slave;
	int i, j;

	uint16_t cx;
	it_slave *si;
	uint16_t ax;

	Update(ite, &cx, &si, &ax);
	//printf("%i %i %i %i\n", cx, &ite->slave[0] - si, ax, ite->CurrentRow);
	printf("%i %i %i\n", ite->ProcessOrder, ite->CurrentPattern, ite->CurrentRow);

	int tsleep = (1000000*10)/(tempo*4);
	tper = (freq*10)/(tempo*4);

	if(dsp == -1)
	{
		dsp = open("/dev/dsp", O_WRONLY);
		assert(dsp > 2);
		ioctl(dsp, SNDCTL_DSP_SETFMT, &ossfmt);
		assert(ossfmt == AFMT_S16_LE);
		ioctl(dsp, SNDCTL_DSP_SPEED, &freq);
		ioctl(dsp, SNDCTL_DSP_STEREO, &stereo);
		printf("OSS opened: fmt=%i, freq=%i, chns=%i\n", ossfmt, freq,
			(stereo ? 2 : 1));
	}

	memset(mixbuf, 0, tper*(stereo ? 2 : 1)*4);
	memset(outbuf, 0, tper*(stereo ? 2 : 1)*2);

	for(i = 0; i < ite->NumChannels; i++)
	{
		slave = &ite->slave[i];
		if((slave->Flags & 1) != 0 && (slave->Flags & 0x0200) == 0)
		{
			// TODO: get the damn thing to behave

			//it_sample *smp = &ite->smp[slave->Smp];
			//printf("smp %i %i %02X\n", slave->Smp, smp->Length, smp->Flg);
			int32_t offs = (int32_t)slave->Sample_Offset;
			int32_t oferr = (int32_t)slave->SmpErr;
			int32_t lpbeg = (int32_t)slave->Loop_Beginning;
			int32_t lpend = (int32_t)slave->Loop_End;
			int32_t nfreq = (int32_t)slave->Frequency;

			int32_t vol = slave->_16bVol;
			vol *= ite->hdr.MV;
			vol >>= 8;

			int32_t lvol = vol;
			int32_t rvol = vol;

			//printf("pan %i\n", slave->FPP);
			if(slave->FPP == 100)
				rvol = -rvol;
			else if(slave->FPP < 32)
				rvol = (rvol * slave->FPP) >> 5;
			else
				lvol = (lvol * (64 - slave->FPP)) >> 5;

			// TODO: fix bugs in actual thing
			//printf("%i %i %i %i\n", slave->Vol, slave->CVl, slave->SVl, ite->GlobalVolume);

			nfreq = (((int64_t)nfreq) << (int64_t)16) / (int64_t)freq;

			if(slave->LpD == 1) nfreq = -nfreq;

			//if(lpend == 0) lpend = smp->Length;

			//lpend = 10000;

			if(ite->SamplePointer[slave->Smp] == NULL)
			{
				kill_channel(ite, slave);
				break;

			} else if(stereo != 0 && slave->Bit != 0) {
				int16_t *data = (int16_t *)ite->SamplePointer[slave->Smp];

				for(j = 0; j < tper*2; j+=2)
				{
					if(update_offs(ite, slave, &offs, &oferr, &nfreq, lpbeg, lpend) != 0)
						break;

					mixbuf[j+0] -= (lvol*(int32_t)data[offs])>>14;
					mixbuf[j+1] -= (rvol*(int32_t)data[offs])>>14;
				}

			} else if(stereo != 0 && slave->Bit == 0) {

				int8_t *data = (int8_t *)ite->SamplePointer[slave->Smp];
				for(j = 0; j < tper*2; j+=2)
				{
					if(update_offs(ite, slave, &offs, &oferr, &nfreq, lpbeg, lpend) != 0)
						break;

					mixbuf[j+0] -= (lvol*(int32_t)data[offs])>>(14-8);
					mixbuf[j+1] -= (rvol*(int32_t)data[offs])>>(14-8);
				}
			} else if(stereo == 0 && slave->Bit != 0) {
				int16_t *data = (int16_t *)ite->SamplePointer[slave->Smp];

				for(j = 0; j < tper; j++)
				{
					if(update_offs(ite, slave, &offs, &oferr, &nfreq, lpbeg, lpend) != 0)
						break;

					mixbuf[j] -= (vol*(int32_t)data[offs])>>14;
				}

			} else {
				int8_t *data = (int8_t *)ite->SamplePointer[slave->Smp];
				for(j = 0; j < tper; j++)
				{
					if(update_offs(ite, slave, &offs, &oferr, &nfreq, lpbeg, lpend) != 0)
						break;

					mixbuf[j] -= (vol*(int32_t)data[offs])>>(14-8);
				}
			}

			slave->Sample_Offset = offs;
			slave->SmpErr = oferr;

		}

		// test
		if(0)
		{
			if((slave->Flags & 0x0200) != 0 && (slave->Flags & 1) != 0)
			{
				printf("Drop channel\n");
				slave->Flags &= ~1;
			}

		}

		slave->Flags &= 0x788D; // no idea why this does it but anyway --GM

		//printf("%i %i %i\n", offs, lpbeg, lpend);

		/*
		if(slave->Flags != 0)
			printf(" %02X:%04X/%p", i, slave->Flags, ite->SamplePointer[slave->Smp]);
		*/
	}
	//printf("\n");

	for(i = 0; i < tper*(stereo != 0 ? 2 : 1); i++)
	{
		int32_t v = mixbuf[i];

		if(v >  0x7FFF) v =  0x7FFF;
		if(v < -0x7FFF) v = -0x7FFF;

		outbuf[i] = v;
	}

	write(dsp, outbuf, tper*2*(stereo != 0 ? 2 : 1));
	//usleep(tsleep);
	return 0;
}

static int drv_oss_DriverSetTempo(it_engine *ite, uint16_t Tempo)
{
	printf("tempo %i\n", Tempo);
	tempo = Tempo;
	return 0;
}

static int drv_oss_DriverSetMixVolume(it_engine *ite, uint16_t MixVolume)
{
	return 0;
}

static int drv_oss_DriverSetStereo(it_engine *ite, uint16_t Stereo)
{
	return 0;
}

static int drv_oss_DriverReleaseSample(it_engine *ite, it_sample *smp)
{
	return 0;
}

static int drv_oss_DriverMIDIOut(it_engine *ite, uint8_t al)
{
	printf("MIDI %02X\n", al);
	return 0;
}

static int drv_oss_DriverGetWaveform(it_engine *ite)
{
	return 0;
}

it_drvdata *drv_oss_init(it_engine *ite)
{
	if(is_initialised)
		return &drv;

	drv.BasePort = 0xFFFF;
	drv.IRQ = 0xFFFF;
	drv.DMA = 0xFFFF;
	drv.CmdLineMixSpeed = 0;

	drv.CmdLineDMASize = 1024;
	drv.ReverseChannels = 0;
	drv.DriverMaxChannels = 256;
	drv.StopEndOfPlaySection = 0;
	drv.DefaultChannels = 256;
	//drv.DefaultChannels = 10;

	drv.DriverFlags = 0; // no midi out, no hiqual (at least for now)
	// both do appear to be supported however (well, to some extent)

	drv.DriverDetectCard = drv_oss_DriverDetectCard;
	drv.DriverInitSound = drv_oss_DriverInitSound;

	drv.DriverUninitSound = drv_oss_DriverUninitSound;

	drv.DriverPoll = drv_oss_DriverPoll;
	drv.DriverSetTempo = drv_oss_DriverSetTempo;
	drv.DriverSetMixVolume = drv_oss_DriverSetMixVolume;
	drv.DriverSetStereo = drv_oss_DriverSetStereo;

	drv.DriverReleaseSample = drv_oss_DriverReleaseSample;

	drv.DriverMIDIOut = drv_oss_DriverMIDIOut;
	drv.DriverGetWaveform = drv_oss_DriverGetWaveform;

	return &drv;
}


