#include "it_struc.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

static it_drvdata drv;
static int is_initialised = 0;
static int tempo = 125;
static int ossfmt = AFMT_S16_LE;
static int freq = 44100;
static int stereo = 0;
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
		assert(stereo == 0);
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

			// TODO: stereo
			int32_t lvol = ((uint16_t)slave->FV)<<8;
			int32_t rvol = slave->_16bVol;
			int32_t vol = rvol;
			//vol = (vol * slave->CVl) >> 6; // TODO: confirm this

			// TODO: fix bugs in actual thing
			//printf("%i %i %i %i\n", slave->Vol, slave->CVl, slave->SVl, ite->GlobalVolume);
			vol *= ite->hdr.MV;
			vol >>= 8;

			nfreq = (((int64_t)nfreq) << (int64_t)16) / (int64_t)freq;

			if(slave->LpD == 1) nfreq = -nfreq;

			//if(lpend == 0) lpend = smp->Length;

			//lpend = 10000;
			
			// TODO: ping-pong loops
			// TODO: stereo
			if(ite->SamplePointer[slave->Smp] == NULL)
			{
				slave->Flags |= 0x0200;

			} else if(slave->Bit != 0) {
				int16_t *data = (int16_t *)ite->SamplePointer[slave->Smp];

				for(j = 0; j < tper; j++)
				{
					oferr += nfreq;
					offs += oferr>>16;
					oferr &= 0xFFFF;

					if(offs < 0)
					{
						if(slave->LpM != 24)
						{
							slave->Flags |= 0x0200;
							slave->LpD = 0;
							break;
						}

						offs = 0;
						if(nfreq < 0) nfreq = -nfreq;
						slave->LpD = 0;
					}

					if(offs >= lpend)
					{
						if(slave->LpM == 0)
						{
							slave->Flags |= 0x0200;
							break;
						}

						if(slave->LpM == 24)
						{
							offs = lpend-1;
							if(nfreq > 0) nfreq = -nfreq;
							slave->LpD = 1;

						} else {
							offs = lpbeg;
						}
					}

					if(offs < 0 || offs >= lpend)
					{
						slave->Flags |= 0x0200;
						break;
					}

					mixbuf[j] -= (vol*(int32_t)data[offs])>>14;
				}

			} else {
				int8_t *data = (int8_t *)ite->SamplePointer[slave->Smp];
				for(j = 0; j < tper; j++)
				{
					oferr += nfreq;
					offs += oferr>>16;
					oferr &= 0xFFFF;

					if(offs < 0)
					{
						if(slave->LpM != 24)
						{
							slave->Flags |= 0x0200;
							slave->LpD = 0;
							break;
						}

						offs = 0;
						if(nfreq < 0) nfreq = -nfreq;
						slave->LpD = 0;
					}

					if(offs >= lpend)
					{
						if(slave->LpM == 0)
						{
							slave->Flags |= 0x0200;
							break;
						}

						if(slave->LpM == 24)
						{
							offs = lpend-1;
							if(nfreq > 0) nfreq = -nfreq;
							slave->LpD = 1;

						} else {
							offs = lpbeg;
						}
					}

					if(offs < 0 || offs >= lpend)
					{
						slave->Flags |= 0x0200;
						break;
					}

					mixbuf[j] -= (vol*(int32_t)data[offs])>>(14-8);
				}
			}

			slave->Sample_Offset = offs;
			slave->SmpErr = oferr;
		}
		//printf("%i %i %i\n", offs, lpbeg, lpend);

		/*
		if(slave->Flags != 0)
			printf(" %02X:%04X/%p", i, slave->Flags, ite->SamplePointer[slave->Smp]);
		*/
	}
	//printf("\n");

	for(i = 0; i < tper; i++)
	{
		int32_t v = mixbuf[i];

		if(v >  0x7FFF) v =  0x7FFF;
		if(v < -0x7FFF) v = -0x7FFF;

		outbuf[i] = v;
	}

	write(dsp, outbuf, tper*2);
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
	drv.DriverMaxChannels = 128;
	drv.StopEndOfPlaySection = 0;
	drv.DefaultChannels = 128;
	drv.DriverFlags = 0; // no midi out, no hiqual (at least for now)

	drv.DriverDetectCard = drv_oss_DriverDetectCard;
	drv.DriverInitSound = drv_oss_DriverInitSound;

	drv.DriverPoll = drv_oss_DriverPoll;
	drv.DriverSetTempo = drv_oss_DriverSetTempo;
	drv.DriverSetMixVolume = drv_oss_DriverSetMixVolume;
	drv.DriverSetStereo = drv_oss_DriverSetStereo;

	drv.DriverReleaseSample = drv_oss_DriverReleaseSample;

	return &drv;
}


