#include "it_struc.h"

uint8_t SlideTable[9] = { 1, 4, 8, 16, 32, 64, 96, 128, 255 };

void InitVolumeEffectH(it_engine *ite, it_host *chn, uint8_t ah);
void InitCommandD7(it_engine *ite, it_host *chn, it_slave *slave);
void InitCommandG11(it_engine *ite, it_host *chn);
void InitCommandX2(it_engine *ite, it_host *chn, uint8_t al);
void CommandD2(it_engine *ite, it_host *chn, it_slave *slave, uint8_t al);
void CommandEChain(it_engine *ite, it_host *chn, int16_t bx);
void CommandFChain(it_engine *ite, it_host *chn, int16_t bx);
void InitVibrato(it_engine *ite, it_host *chn);
void CommandH5(it_engine *ite, it_host *chn, it_slave *slave, int8_t al);
void InitTremelo(it_engine *ite, it_host *chn);
void CommandR2(it_engine *ite, it_host *chn, it_slave *slave, int8_t al);

/*
// Not transcribing further unless there is a good reason to.
int32_t GetC5Speed(uint8_t *ds, uint8_t *es, uint16_t bx)
{
	if(*(uint8_t *)(ds + di + 0x0F) == 101)
	{
		// OK.. we have a MIDI sample
		// Check whether the midi sample points to a valid sample
		// And if so, use that sample's C5 speed.

		uint16_t si = *(uint8_t *)(ds + di + 0x03); // Not a note?
		uint32_t ebp = *(uint8_t *)(ds + di + 0x04);

		if(ebp != 0) // No sample?
		{
			si += si;
			ebp = *(uint16_t *)(es + 64710 + ebp + ebp);
			uint8_t al = *(uint8_t *)(es + ebp + si + 0x40) // AL = note
			uint8_t ah = *(uint8_t *)(es + ebp + si + 0x41) // AH = sample.

			if(ah != 0) // No sample?
			{
				si = ah*2;
				si = *(uint16_t *)(es + 64910 + si) // Sample offset

				if(*(uint8_t *)(es + si + 0x12) == 1)
				{
					bx = si;
				}
			}
		}
	}

	return *(uint8_t *)(es + bx + 0x3C); // EAX = C5Spd
}
*/

void InitVolumeEffect(it_engine *ite, it_host *chn) // Done A, B, H
{
	if((chn->Msk & 0x44) == 0)
		return;

	uint8_t al = chn->Vol;
	uint8_t ah = al;

	al &= 0x7F;
	if(al < 65)
		return;
	al -= 65;

	if((ah & 0x80) != 0)
	{
		al += 60;
	}

	uint8_t dl = 10;
	ah = 0;
	uint8_t tmpquo = al / dl;
	uint8_t tmprem = al % dl;
	al = tmpquo; // Effect number
	ah = tmprem; // Effect parameter
	chn->VCm = al; // Store effect number

	// Memory for effects A->D, (EFG)/H dont' share.

	// Effects Ax and Bx (fine volume slide up and down) require immediate
	// handling. No flags required. (effect 0 and 1)

	// Effects Cx, Dx, Ex, Fx (volume/pitch slides) require flag to be
	// set   (effects 2->5)

	// Effects Gx and Hx need init (handling) code + flags.
	// (effects 6 and 7).

	if(ah != 0)
	{
		if(al >= 4)
		{
			if(al < 6)
			{
				// Ex Fx
				chn->EFG = ah<<2;
			} else if(al <= 6) {
				// Gx
				if((ite->hdr.Flags & 0x0020) != 0) // Link command G?
				{
					chn->GOE = SlideTable[ah-1];
				} else {
					chn->EFG = SlideTable[ah-1];
				}
			}
		} else {
			// Ax Bx Cx Dx
			chn->VVal = ah;
		}
	}

	if((chn->Flags & 0x04) != 0) 
	{
		// Channel not on!
		// TODO: work out what this comment means and which it applies to
		// (I think it applies to this block)

		it_slave *slave = &ite->slave[chn->SCOffst];

		if(al <= 1)
		{
			if(al != 1)
			{
				al = chn->VVal;
				al += slave->VS;
				if(al > 64)
					al = 64;
			} else {
				al = chn->VVal;
				al -= slave->VS;
				if((al & 0x80) != 0)
					al = 0;
			}

			CommandD2(ite, chn, slave, al);
			return;
		} else {
			chn->Flags |= 0x0100;

			if(al > 6)
			{
				InitVolumeEffectH(ite, chn, ah);
				return;
			} else if(al == 6) {
				InitCommandG11(ite, chn);
				return;
			}
			return;
		}
	} else {
		if(al == 7)
			InitVolumeEffectH(ite, chn, ah);
	}
}

void InitVolumeEffectH(it_engine *ite, it_host *chn, uint8_t ah)
{
	ah <<= 2;
	if(ah != 0)
		chn->VDp = ah;
	
	if((chn->Flags & 4) != 0)
		InitVibrato(ite, chn);
}

void VolumeCommandC(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	uint8_t al = chn->VVal + slave->VS;

	if(al > 64)
	{
		chn->Flags &= ~0x0100; // Turn off effect calling
		al = 64;
	}

	CommandD2(ite, chn, slave, al);
}

void VolumeCommandD(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	uint8_t al = chn->VVal - slave->VS;

	if((al & 0x80) != 0)
	{
		chn->Flags &= ~0x0100; // Turn off effect calling
		al = 0;
	}

	CommandD2(ite, chn, slave, al);
}

void VolumeCommandE(it_engine *ite, it_host *chn) // Pitch slide down
{
	CommandEChain(ite, chn, ((int16_t)(uint16_t)chn->EFG)<<2);
}

void VolumeCommandF(it_engine *ite, it_host *chn)
{
	CommandFChain(ite, chn, ((int16_t)(uint16_t)chn->EFG)<<2);
}

void VolumeCommandG(it_engine *ite, it_host *chn)
{
	if((chn->Flags & 0x16) == 0)
		return;

	uint16_t bx = chn->EFG;
	if((ite->hdr.Flags & 0x0020) != 0) // Link command G?
		bx = chn->GOE;

	if(bx == 0)
		return;

	bx <<= 2;
	it_slave *slave = &ite->slave[chn->SCOffst];

	if(chn->_42 != 1)
	{
		// Slide down
		PitchSlideDown(ite, chn, slave, bx);

		// Check that frequency is above porta
		//  to frequency.
		int32_t eaxs = slave->Frequency;
		if(eaxs <= chn->Porta_Frequency)
		{
			chn->Flags &= ~0x0110; // Turn off calling
			slave->Frequency = eaxs = chn->Porta_Frequency;
		}

		slave->Frequency_Set = eaxs;

	} else {
		// Slide up!
		PitchSlideUp(ite, chn, slave, bx);

		// Check that
		//  1) Channel is on
		//  2) Frequency (set) is below porta to
		//       frequency
		int32_t eaxs = slave->Frequency;

		if((slave->Flags & 0x0200) != 0 || eaxs > chn->Porta_Frequency)
		{
			slave->Flags &= ~0x0200;
			chn->Flags |= 0x0004; // Turn on.
			chn->Flags &= ~0x0110; // Turn off calling
			slave->Frequency = eaxs = chn->Porta_Frequency;
		}

		slave->Frequency_Set = eaxs;

	}
}

void InitNoCommand(it_engine *ite, it_host *chn) // DS:DI points to CIT area.
{
	it_slave *slave;
	uint8_t cl = chn->Msk;   // CL = mask
	uint8_t ch = chn->Flags; // CH = channel info.

	if((cl & 0x33) == 0)
		goto NoOldEffect; // InitCommand1

	uint8_t al = chn->Nt2;

	// Note here!
	// Check for noteoff.
	if(al >= 120)
	{
		if((ch & 0x04) == 0) // Taken if the channel's off.
		{
			// Jump point.
			goto NoOldEffect; // InitNoCommand1
		}

		slave = &ite->slave[chn->SCOffst];

		if(al > 0xFE) // Noteoff
		{
			slave->Flags |= 0x0004; // Note off
			// if(slave->VS == 0) // Volume = 0???
			// 	goto InitNoCommand13;
		} else {
			if(al == 0xFE)
			{
				ch &= ~4;

				if(chn->Smp != 100 && (ite->d.DriverFlags & 2) == 0)
				{
					slave->Flags = 0x0200;
					goto NoOldEffect;
				}

				// MIDINoteCut:
				// ch &= ~4;
				slave->Flags |= 0x0200;
			} else {
				slave->Flags |= 8; // Note fade
				goto NoOldEffect;
			}
		}
	} else {
		if((ch & 4) != 0 && (cl & 0x11) == 0 && ite->slave[chn->SCOffst].Nte != chn->Nte)
			goto NoOldEffect; // InitNoCommand1

		if((cl & 0x44) != 0 && chn->Vol >= 193 && chn->Vol <= 202 && (chn->Flags & 4) != 0)
			return InitVolumeEffect(ite, chn);

		slave = AllocateChannel(ite, chn, &ch);
		if(slave == NULL)
			goto NoOldEffect;

		// Channel allocated.
		// put volume
		slave->Vol = slave->VS = chn->VSe;
		it_sample *smp = &ite->smp[slave->SmpOffs];

		if((ite->hdr.Flags & 4) == 0 && (smp->DfP & 0x80) == 0)
			slave->PS = slave->Pan = chn->CP = smp->DfP & 0x7F;

		uint32_t eax = smp->C5Speed;

		slave->OldSampleOffset = 0;
		slave->SmpErr = 0;
		slave->Sample_Offset = 0;

		// Calculate frequency.
		uint64_t da = (uint64_t)eax * (uint64_t)PitchTable[chn->Nt2];
		eax = (uint32_t)(((uint64_t)da)>>(uint64_t)16);
		slave->Frequency = slave->Frequency_Set = eax;

		ch |= 4;
		ch &= ~16;
	}

	GetLoopInformation(ite, slave);

InitNoCommand1:
	if((cl & (0x22 + 0x44)) == 0)
		goto InitNoCommand3;

	// Instrument mode and old effects?
	if((ite->hdr.Flags & 0x14) != 0x14)
		goto NoOldEffect;
	
	if(cl & 0x22)
		goto NoOldEffect;

	if(chn->Ins == 0xFF)
		goto NoOldEffect;

	slave->FadeOut = 0x0400;
	InitPlayInstrument(ite, chn, slave, chn->Ins);

NoOldEffect:
	// TODO: fix this dreadful flow code once everything works --GM
	if((cl & 0x44) == 0)
		goto InitNoCommand7;

	if(chn->Vol <= 64)
	{
		chn->VSe = chn->Vol;
		goto InitNoCommand8; // Volume set...
	}

	if((chn->Vol & 0x7F) >= 65)
		goto InitNoCommand7;

	// Panning set!
InitNoCommandPanning:
	chn->Flags = (chn->Flags & 0xFF00) | (0xFF & (uint16_t)ch);

	InitCommandX2(ite, chn, chn->Vol - 128); // Destroys (SI), AX

InitNoCommand7:
	if((cl & 0x22) == 0) // Instrument present? Change volume
		goto InitNoCommand3;

	// Get instrument offset.
	if(chn->Smp == 0xFF)
		goto InitNoCommand3;

	chn->VSe = ite->smp[chn->Smp-1].Vol; // Default volume

InitNoCommand8:
	if((ch & 4) == 0)
		goto InitNoCommand3;
	
	slave = &ite->slave[chn->SCOffst];

	slave->Vol = slave->VS = chn->VSe;
	slave->Flags |= 0x10; // recalc volume

InitNoCommand3:
	// Randomise volume if required.
	if((chn->Flags & 0x80) == 0)
	{
		chn->Flags = (chn->Flags & 0xFF00) | (0xFF & (uint16_t)ch);
		return InitVolumeEffect(ite, chn);
	}

	chn->Flags = (chn->Flags & 0xFF00) | (0xFF & (uint16_t)ch);
	ApplyRandomValues(ite, chn);
	return InitVolumeEffect(ite, chn);
}

void InitCommandA(it_engine *ite, it_host *chn)
{
	uint16_t ax = chn->CVal;

	if(ax != 0)
	{
		ite->CurrentTick -= ite->CurrentSpeed;
		ite->ProcessTick -= ite->CurrentSpeed;
		ite->CurrentTick += ax;
		ite->ProcessTick += ax;
		ite->CurrentSpeed = ax;
	}

	return InitNoCommand(ite, chn);
}

void InitCommandB(it_engine *ite, it_host *chn)
{
	uint16_t ax = chn->CVal;

	if(ax < ite->CurrentOrder)
		ite->StopSong = 1;
	
	ax--;
	ite->ProcessOrder = ax;
	ite->ProcessRow = 0xFFFE;

	return InitNoCommand(ite, chn);
}

void InitCommandC(it_engine *ite, it_host *chn)
{
	if(ite->PatternLooping == 0)
	{
		ite->BreakRow = chn->CVal;
		ite->ProcessRow = 0xFFFE;
	}

	return InitNoCommand(ite, chn);
}

void InitCommandD(it_engine *ite, it_host *chn)
{
	InitNoCommand(ite, chn);

	if(chn->CVal != 0)
		chn->DKL = chn->CVal;

	if((chn->Flags & 4) == 0)
		return;

	it_slave *slave = &ite->slave[chn->SCOffst];

	return InitCommandD7(ite, chn, slave);
}

void InitCommandD7(it_engine *ite, it_host *chn, it_slave *slave)
{
	// Jmp point for Lxx

	slave->Flags |= 16; // Recalc vol

	// TODO: verify
	if((chn->DKL & 0x0F) == 0) {
		// Slide up.
		chn->VCh = chn->DKL>>4;
		chn->Flags |= 1;

		if(chn->VCh == 0x0F)
			return CommandD(ite, chn);
	} else if((chn->DKL & 0xF0) == 0) {
		// Slide down
		chn->VCh = -chn->DKL;
		chn->Flags |= 1;

		if(chn->VCh == (uint8_t)-0x0F)
			return CommandD(ite, chn);
	} else if((chn->DKL & 0x0F) == 0x0F) {
	InitCommandD5:
		// Slide up (fine)
		chn->VCh = 0;

		uint8_t al = (chn->DKL >> 4) + slave->VS;
		if(al > 64)
			al = 64;

		slave->Vol = slave->VS = chn->VSe = al;

	} else if((chn->DKL & 0xF0) == 0xF0) {
		// Slide down (fine)
		chn->VCh = 0;

		uint8_t al = slave->VS - (chn->DKL & 0x0F);

		if((al & 0x80) != 0)
			al = 0;

		slave->Vol = slave->VS = chn->VSe = al;
	}
}

void InitCommandE(it_engine *ite, it_host *chn)
{
	InitNoCommand(ite, chn);

	if(chn->CVal != 0)
		chn->EFG = chn->CVal;

	if((chn->Flags & 4) == 0)
		return;

	it_slave *slave = &ite->slave[chn->SCOffst];

	// OK.. now processing is dependent
	// upon slide mode.

	if(chn->EFG == 0) // still no slide??
		return;

	if((chn->EFG & 0xF0) >= 0xE0)
	{
		if((chn->EFG & 0x0F) == 0)
			return;

		uint8_t al = chn->EFG & 0x0F;
		if((chn->EFG & 0xF0) != 0xE0)
			al <<= 2;

		PitchSlideDown(ite, chn, slave, al);

		slave->Frequency_Set = slave->Frequency;
	} else {
		chn->_40 = ((int16_t)chn->EFG)<<2;

		// call update only if necess.
		chn->Flags |= 1;
	}
}

void InitCommandF(it_engine *ite, it_host *chn)
{
	// TODO: verify
	InitNoCommand(ite, chn);

	if(chn->CVal != 0)
		chn->EFG = chn->CVal;

	if((chn->Flags & 4) == 0)
		return;

	it_slave *slave = &ite->slave[chn->SCOffst];

	// OK.. now processing is dependent
	// upon slide mode.

	if(chn->EFG == 0) // still no slide??
		return;

	if((chn->EFG & 0xF0) >= 0xE0)
	{
		if((chn->EFG & 0x0F) == 0)
			return;

		uint8_t al = chn->EFG & 0x0F;
		if((chn->EFG & 0xF0) != 0xE0)
			al <<= 2;

		PitchSlideUp(ite, chn, slave, al);

		slave->Frequency_Set = slave->Frequency;
	} else {
		// note: not negative. this point proves crucial for some things --GM
		chn->_40 = (((int16_t)chn->EFG)<<2);

		// call update only if necess.
		chn->Flags |= 1;
	}
}

// InitCommandG15 = InitNoCommand
void InitCommandG(it_engine *ite, it_host *chn)
{
	// Check whether channel on/owned
	if(chn->CVal != 0)
	{
		// Compatibility Gxx?
		if((ite->hdr.Flags & 0x20) != 0)
			chn->GOE = chn->CVal;
		else
			chn->EFG = chn->CVal;
	}

	if((chn->Flags & 4) == 0)
		InitNoCommand(ite, chn);
	else
		InitCommandG11(ite, chn);
}

void InitCommandG11(it_engine *ite, it_host *chn)
{
	// Jumped to from Lxx
	it_slave *slave = &ite->slave[chn->SCOffst];

	int skipnote = 0;

	if((chn->Msk & 0x22) != 0 && (chn->Smp != 0))
	{
		// Checking for change of Sample or instrument.
		if((ite->hdr.Flags & 0x20) == 0)
		{
			// Don't overwrite note if MIDI!
			if(chn->Smp == 101)
			{
				// Ins the same?

				uint8_t oldsmp = slave->Smp;
				uint8_t oldins = slave->Ins;
				slave->Nte = chn->Nte;
				slave->Ins = chn->Ins;
				if(oldins != chn->Ins // Ins the same?
					|| oldsmp != chn->Smp) // Sample the same?
				{
					it_sample *smp = &ite->smp[chn->Smp-1];
					slave->Flags = (slave->Flags & 0xFF) | 0x0100;

					// Now to update sample info.
					slave->SmpOffs = chn->Smp-1;
					slave->Smp = chn->Smp-1;

					slave->ViDepth = 0; // Reset vibrato..
					slave->LpD = 0; // Reset loop direction.
					slave->OldSampleOffset = 0;
					slave->SmpErr = 0;
					slave->Sample_Offset = 0;

					slave->SVl = smp->GvL*2;

					if((smp->Flg & 1) == 0)
					{
						slave->Flags = 0x0200;
						chn->Flags &= ~4;
						return;
					}

					// 16 bit...
					slave->Bit = (smp->Flg & 2);

					GetLoopInformation(ite, slave);
				}
			}

		} else {
			chn->Smp = slave->Smp+1;

			it_sample *smp = &ite->smp[slave->Smp];
			slave->SVl = smp->GvL*2;

			skipnote = 1;
			if((ite->hdr.Flags & 4) != 0) // Instrument/sample mode?
			{
				// Now for instruments
				slave->FadeOut = 0x0400;
				it_instrument *ins = &ite->ins[chn->Ins];

				uint16_t oldflags = slave->Flags;
				InitPlayInstrument(ite, chn, slave, chn->Ins);

				if((oldflags & 1) != 0)
					slave->Flags &= ~0x100;

				slave->SVl = ((uint16_t)ins->GbV * (uint16_t)slave->SVl)>>7;
			}
		}
	}

	if((skipnote != 0) || (chn->Msk & 0x11) != 0)
	{
		// OK. Time to calc freq.
		if(chn->Nt2 > 119)
		{
			if((chn->Flags & 4) != 0)
			{
				if(chn->Nt2 > 0xFE)
				{ 
					// Note off
					slave->Flags |= 4;
					GetLoopInformation(ite, slave);

				} else if(chn->Nt2 != 0xFE) {
					slave->Flags |= 8;

				} else {
					// Note cut!
					chn->Flags &= ~4;
					slave->Flags = 0x200; // Cut.
				}
			}

		} else {
			// Don't overwrite note if MIDI!
			if(chn->Smp != 101)
				slave->Nte = chn->Nt2;

			it_sample *smp = &ite->smp[slave->SmpOffs];

			uint64_t c5 = ((uint64_t)smp->C5Speed) * (uint64_t)PitchTable[chn->Nt2];

			c5 >>= (uint64_t)16;

			chn->Porta_Frequency = c5;
			chn->Flags |= 16;
		}
	}

	int revol = -1;
	if((chn->Msk & 0x44) != 0)
	{
		if(chn->Vol <= 64)
		{
			revol = chn->Vol;
		} else if((chn->Vol & 0x7F) < 65) {
			// Panning set...
			InitCommandX2(ite, chn, chn->Vol - 128);
		}
	}

	if(revol == -1 && (chn->Msk & 0x22) != 0)
		revol = ite->smp[slave->SmpOffs].Vol;

	if(revol != -1)
	{
		slave->Flags |= 16; // recalc volume.
		chn->VSe = slave->Vol = slave->VS = revol;
	}

	if((chn->Flags & 16) != 0)       // Slide on???
	{
		// Work out magnitude + dirn
		int16_t ax = chn->EFG;

		if((ite->hdr.Flags & 0x20) != 0)   // Command G memory
			ax = chn->GOE;

		ax <<= 2;
		if(ax != 0)
		{
			chn->_40 = ax;

			if(chn->Porta_Frequency > slave->Frequency_Set)
			{
				// slide up
				chn->_42 = 1 | (chn->_42 & 0xFF00);

				if((chn->Flags & 0x100) == 0)
					chn->Flags |= 1; // Update effect if necess.

			} else if(chn->Porta_Frequency < slave->Frequency_Set) {
				// slide down.
				chn->_42 = 0 | (chn->_42 & 0xFF00);

				if((chn->Flags & 0x100) == 0)
					chn->Flags |= 1; // Update effect if necess.

			}

			// equal?!?!?
			// then don't update effect.
		}
	}

	// Don't call volume effects if it has a Gxx!
	if((chn->Flags & 0x100) == 0) // comment this out and you'll get a stack overflow on a Gx VE --GM
		InitVolumeEffect(ite, chn);
}

void InitCommandH(it_engine *ite, it_host *chn)
{
	if((chn->Msk & 0x11) != 0 && chn->Nte <= 119)
	{
		chn->VPo = 0;
		chn->LVi = 0;
	}

	uint8_t ah = (chn->CVal & 0x0F); // AH = depth
	uint8_t al = (chn->CVal & 0xF0); // AL = speed.

	if(ah != 0 || al != 0)
	{
		al >>= 2;
		if(al != 0)
			chn->VSp = al;

		ah <<= 2;
		if(ah != 0)
		{
			if((ite->hdr.Flags & 0x10) != 0)
				ah *= 2;

			chn->VDp = ah;
		}
	}

	InitNoCommand(ite, chn);

	if((chn->Flags & 4) != 0)
	{
		// Update mode.
		chn->Flags |= 1;
		InitVibrato(ite, chn);
	}
}

void InitCommandI(it_engine *ite, it_host *chn)
{
	InitNoCommand(ite, chn);

	if(chn->CVal != 0)
		chn->I00 = chn->CVal;

	if((chn->Flags & 4) != 0)
	{
		// OK.. now to handle tremor
		chn->Flags |= 1;

		uint8_t al = chn->I00 & 0x0F; // AL = Offtime
		uint8_t ah = chn->I00 >> 4;   // AH = ontime

		if((ite->hdr.Flags & 16) != 0)
		{
			al++;
			ah++;
		}

		uint16_t ax = (uint16_t)al | (((uint16_t)ah)<<8);
		chn->_40 = ax; // fucking hell Jeff ;_; --GM

		CommandI(ite, chn);
	}
}

void InitCommandJ(it_engine *ite, it_host *chn)
{
	InitNoCommand(ite, chn);

	chn->_40 = 0;
	if(chn->CVal != 0)
		chn->J00 = chn->CVal;

	if((chn->Flags & 4) != 0)
	{
		chn->Flags |= 1; // Update when channel on

		// TODO: verify - i think this actually stores the freq to mul by,
		// which is 32-bit --GM
		chn->_44 = (60+(chn->J00&0x0F))*4;
		chn->_42 = (60+(chn->J00>>4))*4;
	}
}

void InitCommandK(it_engine *ite, it_host *chn)
{
	if(chn->CVal != 0)
		chn->DKL = chn->CVal;

	InitNoCommand(ite, chn);

	if((chn->Flags & 4) != 0)
	{
		InitVibrato(ite, chn);
		it_slave *slave = &ite->slave[chn->SCOffst];
		InitCommandD7(ite, chn, slave);

		chn->Flags |= 2; // Always update.
	}
}

void InitCommandL(it_engine *ite, it_host *chn)
{
	if(chn->CVal != 0)
		chn->DKL = chn->CVal;

	if((chn->Flags & 4) != 0)
	{
		InitCommandG11(ite, chn);
		it_slave *slave = &ite->slave[chn->SCOffst];
		InitCommandD7(ite, chn, slave);

		chn->Flags |= 2; // Always update.
	}
}

void InitCommandM2(it_engine *ite, it_host *chn, uint8_t al)
{
	if((chn->Flags & 4) != 0)
	{
		it_slave *slave = &ite->slave[chn->SCOffst];

		slave->CVl = al;
		slave->Flags |= 16; // recalc volume
	}

	chn->CV = al;
}

void InitCommandM(it_engine *ite, it_host *chn)
{
	InitNoCommand(ite, chn);

	if(chn->CVal > 0x40) return;

	InitCommandM2(ite, chn, chn->CVal);
}

void InitCommandN(it_engine *ite, it_host *chn)
{
	if(chn->CVal != 0)
		chn->N00 = chn->CVal;

	InitNoCommand(ite, chn);

	if((chn->N00 & 0x0F) == 0)
	{
		chn->_40 = (chn->_40 & 0xFF00) | ((((int16_t)chn->N00)>>4) & 0xFF);
		chn->Flags |= 2; // Always update effect

	} else if((chn->N00 & 0xF0) == 0) {
		chn->_40 = (chn->_40 & 0xFF00) | ((-(int16_t)chn->N00) & 0xFF);
		chn->Flags |= 2;

	} else if((chn->N00 & 0x0F) == 0x0F) {
		uint8_t al = (chn->N00>>4) + chn->CV;

		if(al > 64) al = 64;

		return InitCommandM2(ite, chn, al);

	} else if((chn->N00 & 0xF0) == 0xF0) {
		//
		uint8_t al = (chn->N00>>4) - chn->CV;

		if((chn->N00>>4) < chn->CV) al = 0;

		return InitCommandM2(ite, chn, al);

	}
}

void InitCommandO(it_engine *ite, it_host *chn)
{
	if(chn->CVal != 0)
		chn->O00 = chn->CVal;

	InitNoCommand(ite, chn);

	if((chn->Msk & 0x33) == 0) return;
	if(chn->Nt2 >= 120) return;
	if((chn->Flags & 4) == 0) return;

	uint32_t eax;
	it_slave *slave = &ite->slave[chn->SCOffst];
	eax = ((uint32_t)chn->O00) + (((uint32_t)chn->OxH)<<8);
	eax <<= 8;

	if(eax >= slave->Loop_End)
	{
		if((ite->hdr.Flags & 16) == 0) return;
		eax = slave->Loop_End-1;
	}

	slave->OldSampleOffset = slave->Sample_Offset = eax;
	slave->SmpErr = 0;
}

void InitCommandP(it_engine *ite, it_host *chn)
{
	if(chn->CVal != 0)
		chn->P00 = chn->CVal;

	InitNoCommand(ite, chn);

	uint8_t dl = chn->CP;
	if((chn->Flags & 4) != 0)
	{
		it_slave *slave = &ite->slave[chn->SCOffst];
		dl = slave->PS; // Pan set
	}

	if(dl == 100) // Surround??
		return;

	uint8_t al = chn->P00;

	if((al & 0x0F) == 0)
	{
		chn->_40 = (chn->_40 & 0xFF00) + ((-(int16_t)(al >> 4)) & 0x00FF);
		chn->Flags |= 2; // Always update effect

	} else if((al & 0xF0) == 0) {
		chn->_40 = (chn->_40 & 0xFF00) + (((int16_t)al) & 0x00FF);
		chn->Flags |= 2;

	} else if((al & 0x0F) == 0x0F) {
		uint8_t xal = dl - (al>>4);

		if(dl < (al>>4)) xal = 0;

		InitCommandX2(ite, chn, xal);

	} else if((al & 0xF0) == 0xF0) {
		al = al + dl;

		if(al > 64) al = 64;

		InitCommandX2(ite, chn, al);

	}

}

void InitCommandQ(it_engine *ite, it_host *chn)
{
	InitNoCommand(ite, chn);

	if(chn->CVal != 0)
		chn->Q00 = chn->CVal;

	if((chn->Flags & 4) != 0)
	{
		chn->Flags |= 1;

		if((chn->Msk & 0x11) != 0)
		{
			chn->RTC = chn->CVal & 0x0F; // retrig countdown

		} else {
			CommandQ(ite, chn);

		}
	}

}

void InitCommandR(it_engine *ite, it_host *chn)
{
	uint8_t ah = chn->CVal & 0x0F; // AH = depth
	uint8_t al = chn->CVal & 0xF0; // AL = speed.

	if(ah != 0 || al != 0)
	{
		al >>= 2;
		if(al != 0) chn->TSp = al;

		ah <<= 1;
		if(ah != 0) chn->TDp = ah;
	}

	InitNoCommand(ite, chn);

	if((chn->Flags & 4) != 0)
	{
		chn->Flags |= 1; // Update mode.
		InitTremelo(ite, chn);
	}
}

void InitCommandS(it_engine *ite, it_host *chn)
{
	uint8_t al = chn->CVal;
	it_slave *slave;
	uint16_t cx;

	if(al != 0)
		chn->S00 = al;

	al = chn->S00;

	uint8_t ah = al;
	ah &= 0xF0;
	al &= 0x0F;

	chn->_40 = (ah<<8) | al; // Misc effects data.

	switch(ah>>4)
	{
		case 0x0: // 0
		case 0x1: // 1
		case 0x2: // 2
			break;

		case 0x3: // 3 - set vibrato waveform
			if(al <= 3) chn->VWF = al;
			break;

		case 0x4: // 4 - set tremelo waveform
			if(al <= 3) chn->TWF = al;
			break;

		case 0x5: // 5 - set panbrello waveform
			if(al <= 3) chn->PWF = al;
			break;

		case 0x6: // 6 - extra delay of x frames
			ite->CurrentTick += al;
			ite->ProcessTick += al;
			break;

		case 0x7: // 7 - instrument functions
		switch(al)
		{
			case 0x0: // Past note cut
				InitNoCommand(ite, chn);
				al = chn->HCN | 0x80;

				for(slave = &ite->slave[0], cx = ite->NumChannels; cx != 0; cx--, slave++)
				{
					if(al != slave->HCN) continue;

					if((ite->d.DriverFlags & 2) != 0)
						slave->Flags |= 0x200;
					else
						slave->Flags  = 0x200;
				}
				return;

			case 0x1: // Past note off
			case 0x2: // Past note fade
				InitNoCommand(ite, chn);
				ah = (al == 0x1 ? 4 : 8);
				for(slave = &ite->slave[0], cx = ite->NumChannels; cx != 0; cx--, slave++)
				{
					if(al == slave->HCN)
					{
						slave->Flags |= ah;
						GetLoopInformation(ite, slave);
					}
				}
				return;

			case 0x3: // Set NNA to cut
			case 0x4: // Set NNA to continue
			case 0x5: // Set NNA to off
			case 0x6: // Set NNA to fade
				InitNoCommand(ite, chn);
				if((chn->Flags & 4) != 0)
				{
					it_slave *slave = &ite->slave[chn->SCOffst];
					slave->NNA = al-3;
				}

				return;

			case 0x7: // Set volume envelope on
				InitNoCommand(ite, chn);

				if((chn->Flags & 4) != 0)
				{
					it_slave *slave = &ite->slave[chn->SCOffst];
					slave->Flags &= ~0x1000;
				}

				return;

			case 0x8: // Set volume envelope off
				InitNoCommand(ite, chn);

				if((chn->Flags & 4) != 0)
				{
					it_slave *slave = &ite->slave[chn->SCOffst];
					slave->Flags |= 0x1000;
				}

				return;

			case 0x9: // Set panning envelope on
				InitNoCommand(ite, chn);

				if((chn->Flags & 4) != 0)
				{
					it_slave *slave = &ite->slave[chn->SCOffst];
					slave->Flags &= ~0x2000;
				}
				break;

			case 0xA: // Set panning envelope off
				InitNoCommand(ite, chn);

				if((chn->Flags & 4) != 0)
				{
					it_slave *slave = &ite->slave[chn->SCOffst];
					slave->Flags |= 0x2000;
				}
				break;

			case 0xB: // Set pitch envelope on
				InitNoCommand(ite, chn);

				if((chn->Flags & 4) != 0)
				{
					it_slave *slave = &ite->slave[chn->SCOffst];
					slave->Flags |= ~0x4000;
				}
				break;

			case 0xC: // Set pitch envelope off
				InitNoCommand(ite, chn);

				if((chn->Flags & 4) != 0)
				{
					it_slave *slave = &ite->slave[chn->SCOffst];
					slave->Flags |= 0x4000;
				}
				break;

			case 0xD:
			case 0xE:
			case 0xF:
				break;
		} break;

		case 0x8: // 8 - set pan
			ah = al;
			ah <<= 4;
			al |= ah;
			ah = 0;

			if(((al+2)&0xFF) < al) ah++;
			al += 2;
			al >>= 2;
			al |= (ah << 6);
			// TODO: verify

			InitNoCommand(ite, chn);
			InitCommandX2(ite, chn, al);
			return;

		case 0x9: // 9 - set surround
			if(al == 1)
			{
				al = 100;
				InitNoCommand(ite, chn);
				InitCommandX2(ite, chn, al);
				return;
			}

			break;

		case 0xA: // A - Set high order offset
			chn->OxH = al;
			break;

		case 0xB: // B - loop control
			InitNoCommand(ite, chn);

			if(al == 0)
			{
				chn->PLR = ite->CurrentRow;

			} else if(chn->PLC == 0) {
				chn->PLC = al;
				ite->ProcessRow = ((uint16_t)chn->PLR)-1;
				ite->PatternLooping = 1;

			} else if((--chn->PLC) == 0) {
				ite->ProcessRow = ((uint16_t)chn->PLR)-1;
				ite->PatternLooping = 1;

			} else {
				chn->PLR = ++ite->CurrentRow;

			}
			return;

		case 0xC: // C - note cut
			chn->Flags |= 1;
			break;

		case 0xD: // D - note delay
			chn->Flags |= 2;
			return;

		case 0xE: // E - pattern delay
			if(ite->RowDelayOn == 0)
			{
				ite->RowDelay = al+1;
				ite->RowDelayOn = 1;
			}

			break;

		case 0xF: // F - MIDI Macro select
			chn->SFx = al;
			break;
	}

	InitNoCommand(ite, chn);
}

void InitCommandT(it_engine *ite, it_host *chn)
{
	if(chn->CVal != 0)
		chn->T00 = chn->CVal;

	if(chn->T00 >= 0x20)
	{
		ite->Tempo = chn->T00;
		Music_InitTempo(ite);
		InitNoCommand(ite, chn);

	} else {
		InitNoCommand(ite, chn);
		chn->Flags |= 2; // Update mode

	}
}

void InitCommandU(it_engine *ite, it_host *chn)
{
	if((chn->Msk & 0x11) != 0)
	{
		chn->VPo = 0;
		chn->LVi = 0;
	}

	uint8_t ah = chn->CVal & 0x0F; // AH = depth
	uint8_t al = chn->CVal & 0xF0; // AL = speed.

	if(al != 0)
	{
		al >>= 2;
		chn->VSp = al;
	}

	if(ah != 0)
	{
		if((ite->hdr.Flags & 16) != 0)
			ah *= 2;

		chn->VDp = ah;
	}

	InitNoCommand(ite, chn);

	if((chn->Flags & 4) != 0)
	{
		chn->Flags |= 1; // Update mode.
		InitVibrato(ite, chn);
	}

}

void InitCommandV(it_engine *ite, it_host *chn)
{
	if(chn->CVal <= 0x80)
	{
		ite->GlobalVolume = chn->CVal;
		RecalculateAllVolumes(ite);
	}

	InitNoCommand(ite, chn);
}

void InitCommandW(it_engine *ite, it_host *chn)
{
	// Global volume slides!

	InitNoCommand(ite, chn);

	if(chn->CVal != 0)
		chn->W00 = chn->CVal;

	if(chn->W00 == 0)
	{
		return;
	
	} else if((chn->W00 & 0xF0) == 0) {
		chn->_40 = (chn->_40 & 0xFF00) | (0xFF & (int16_t)(-chn->W00));
		chn->Flags |= 2;

	} else if((chn->W00 & 0x0F) == 0) {
		chn->_40 = (chn->_40 & 0xFF00) | (0xFF & (int16_t)(chn->W00>>4));
		chn->Flags |= 2;

	} else if((chn->W00 & 0xF0) == 0xF0) {
		if(ite->GlobalVolume >= (chn->CVal & 0x0F))
			ite->GlobalVolume -= (chn->CVal & 0x0F);
		else
			ite->GlobalVolume = 0;

		RecalculateAllVolumes(ite);

	} else if((chn->W00 & 0x0F) == 0x0F) {
		ite->GlobalVolume += (chn->CVal>>4);
		if((ite->GlobalVolume & 0x80) != 0)
			ite->GlobalVolume = 128;

		RecalculateAllVolumes(ite);

	}
}

void InitCommandX(it_engine *ite, it_host *chn)
{
	InitNoCommand(ite, chn);

	uint16_t ax = chn->CVal;
	ax += 2;
	ax >>= 2;
	InitCommandX2(ite, chn, (uint8_t)ax);
}

void InitCommandX2(it_engine *ite, it_host *chn, uint8_t al)
{
	if((chn->Flags & 4) != 0)
	{
		it_slave *slave = &ite->slave[chn->SCOffst];
		slave->PS = slave->Pan = al;
		slave->Flags |= 64+2; // Recalculate pan
	}

	chn->CP = al;
}

void InitCommandY(it_engine *ite, it_host *chn)
{
	uint8_t ah = chn->CVal & 0x0F; // AH = depth
	uint8_t al = chn->CVal & 0xF0;

	if(ah != 0 || al != 0)
	{
		al >>= 4;
		if(al != 0)
			chn->PSp = al;

		ah <<= 1;
		if(ah != 0)
			chn->PDp = ah;
	}

	InitNoCommand(ite, chn);

	if((chn->Flags & 4) != 0)
	{
		chn->Flags |= 1; // Update mode.
		CommandY(ite, chn);
	}
}

void InitCommandZ(it_engine *ite, it_host *chn)
{
	// Macros start at 120h, 320h
	InitNoCommand(ite, chn);

	uint16_t bx = chn->CVal;
	it_slave *slave = &ite->slave[chn->SCOffst];

	if((bx & 0x80) == 0)
	{
		bx = chn->SFx & 0x0F; // 0->7Fh - BX = SFx number
		bx <<= 5;
		bx += 0x120;
		MIDITranslate(ite, chn, slave, bx);

	} else {
		// Macros!
		bx &= 0x7F;
		bx <<= 5; // BX = (xx-80x)*20h
		bx += 0x320;
		MIDITranslate(ite, chn, slave, bx);

	}
}

void NoCommand(it_engine *ite, it_host *chn)
{
	// DS:DI points to CIT Area
}

void CommandD(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	uint8_t al = chn->VCh + slave->VS; // Volset.

	if((al & 0x80) == 0)
	{
		if(al > 64)
		{
			chn->Flags &= ~1;
			al = 64;
		}

	} else {
		chn->Flags &= ~1;
		al = 0;
	}

	CommandD2(ite, chn, slave, al);
}

void CommandD2(it_engine *ite, it_host *chn, it_slave *slave, uint8_t al)
{
	slave->Vol = al;
	slave->VS = al;
	chn->VSe = al;
	slave->Flags |= 16; // Recalc vol
}

void CommandE(it_engine *ite, it_host *chn)
{
	CommandEChain(ite, chn, chn->_40);
}

void CommandEChain(it_engine *ite, it_host *chn, int16_t bx)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	PitchSlideDown(ite, chn, slave, bx);
	slave->Frequency_Set = slave->Frequency;
}

void CommandF(it_engine *ite, it_host *chn)
{
	CommandFChain(ite, chn, chn->_40);
}

void CommandFChain(it_engine *ite, it_host *chn, int16_t bx)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	PitchSlideUp(ite, chn, slave, bx);
	slave->Frequency_Set = slave->Frequency;
}

void CommandG(it_engine *ite, it_host *chn)
{
	if((chn->Flags & 16) == 0)
		return;

	int16_t bx = chn->_40;
	it_slave *slave = &ite->slave[chn->SCOffst];

	if((chn->_42 & 0xFF) != 1)
	{
		// Slide down
		PitchSlideDown(ite, chn, slave, bx);

		// Check that frequency is above porta
		//  to frequency.
		uint32_t eax = slave->Frequency;
		if(eax <= chn->Porta_Frequency)
		{
			eax = chn->Porta_Frequency;
			chn->Flags &= ~(3 | 16); // Turn off calling
			slave->Frequency = eax;
		}

		slave->Frequency_Set = eax;

	} else {
		// Slide up!
		PitchSlideUp(ite, chn, slave, bx);

		// Check that
		//  1) Channel is on
		//  2) Frequency (set) is below porta to
		//       frequency
		uint32_t eax = slave->Frequency;

		if((slave->Flags & 0x200) == 0 && eax < chn->Porta_Frequency)
		{
			slave->Frequency_Set = eax;
			return;
		}

		slave->Flags &= ~0x200;
		chn->Flags |= 4; // Turn on.
		eax = chn->Porta_Frequency;
		chn->Flags &= ~(3 | 16); // Turn off calling
		slave->Frequency = eax;
		slave->Frequency_Set = eax;
	}

}

void InitVibrato(it_engine *ite, it_host *chn)
{
	if((ite->hdr.Flags & 0x10) == 0)
	{
		CommandH(ite, chn);

	} else {
		it_slave *slave = &ite->slave[chn->SCOffst];
		slave->Flags |= 32; // Freq change...
		CommandH5(ite, chn, slave, chn->LVi);
	}
}

void CommandH(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	slave->Flags |= 32; // Freq change...

	// Add speed. / Save value
	chn->VPo += chn->VSp;

	// Mov     BH, [DI+38h]            // AL = waveform
	// gg wp --GM

	int8_t al;
	if(chn->VWF != 3)
	{
		// probably not wise to try to emulate out-of-range vibrato types.
		// well, at least for now. we can research these later. --GM
		switch(chn->VWF)
		{
			case 0:
				al = FineSineData[chn->VPo];
				break;
			case 1:
				al = FineRampDownData[chn->VPo];
				break;
			case 2:
				al = FineSquareWave[chn->VPo];
				break;
			default:
				printf("PANIC: out of range vibrato types not emulated!\n");
				abort();
		}

	} else {
		al = (Random(ite)&127)-64; // Random.

	}

	chn->LVi = al; // Save last vibrato.

	CommandH5(ite, chn, slave, al);
}

void CommandH5(it_engine *ite, it_host *chn, it_slave *slave, int8_t al)
{
	int16_t ax = ((int16_t)al) * (int16_t)(int8_t)(chn->VDp);
	ax <<= 2;
	ax += 0x80;
	ax >>= 8; // actual code then simply uses AH --GM

	if((ite->hdr.Flags & 16) != 0)
	{
		// don't ask me why sackit does weird things here,
		// i could have sworn Jeff had done a one's complement
		// but that seems to not be the case (in IT 2.14 at least) --GM

		ax = -ax;
	}

	//MovZX   BX, AH

	// AH = EEx/FEx command value
	if(ax < 0)
		PitchSlideDown(ite, chn, slave, -ax);
	else
		PitchSlideUp(ite, chn, slave, ax);
}

void CommandI(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];

	slave->Flags |= 16; // recalc volume;

	chn->TCD--;

	if((chn->TCD & 0x80) != 0 || chn->TCD == 0)
	{
		chn->Too ^= 1;
		chn->TCD = (uint8_t)(chn->Too == 0 ? (chn->_40>>8) : (chn->_40&0xFF));
	}

	if(chn->Too != 0)
		slave->Vol = 0;

}

void CommandJ(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	int16_t bx = chn->_40;

	slave->Flags |= 32;

	bx += 2;
	if(bx >= 6)
	{
		chn->_40 = 0;
		return;
	}

	chn->_40 = bx;
	uint64_t rad = (uint64_t)slave->Frequency;
	rad *= (uint64_t)PitchTable[(bx == 2 ? chn->_42 : chn->_44)/4];

	//printf("honk %lX\n", rad);
	// FIXME work out how this damn thing works
	rad >>= (uint64_t)16;
	/*
	if((rad & (uint64_t)0xFFFF000000000000LLU) != 0)
		rad &= ~(uint64_t)0xFFFFFFFFLLU;
	else
		rad >>= (uint64_t)rad;
	*/

	slave->Frequency = (uint32_t)rad;
}

void CommandK(it_engine *ite, it_host *chn)
{
	CommandH(ite, chn);
	CommandD(ite, chn);
}

void CommandL(it_engine *ite, it_host *chn)
{
	if((chn->Flags & 16) != 0)
	{
		CommandG(ite, chn);
		chn->Flags |= 1;
	}

	CommandD(ite, chn);
}

void CommandN(it_engine *ite, it_host *chn)
{
	uint8_t al = chn->CV + chn->_40;

	if((al & 0x80) == 0)
		al = 0;
	else if(al > 64)
		al = 64;

	InitCommandM2(ite, chn, al);
}

void CommandP(it_engine *ite, it_host *chn)
{
	int8_t al = chn->CP;

	if((chn->Flags & 4) != 0)
	{
		it_slave *slave = &ite->slave[chn->SCOffst];
		al = slave->PS;
	}

	al += (int8_t)(chn->_40 & 0xFF);

	if(al < 0)
		al = 0;
	else if(al > 64)
		al = 64;

	InitCommandX2(ite, chn, al);
}

void CommandQ(it_engine *ite, it_host *chn)
{
	chn->RTC--;
	if(chn->RTC != 0 && chn->RTC != 0xFF)
		return;

	// OK... reset counter.
	chn->RTC = chn->Q00 & 0x0F; // retrig count done.
	//And     BX, 0F00Fh

	it_slave *slave = &ite->slave[chn->SCOffst];

	if((ite->d.DriverFlags & 2) != 0) // Hiqual?
	{
		if((ite->hdr.Flags & 4) == 0) // Instrument mode?
		{
			// Sample mode
			memcpy(slave+64, slave, sizeof(it_slave));

			(slave+64)->Flags |= 0x200; // Cut
			(slave+64)->HCN |= 0x80;    // Disowned
		} else {
			// Instrument mode

			it_slave *dest = &ite->slave[0];
			uint16_t cx = ite->NumChannels;

			for(; cx != 0; cx--, dest++)
			{
				if((dest->Flags & 1) == 0)
				{
					memcpy(dest, slave, sizeof(it_slave));
					dest->Flags |= 0x200; // Cut
					dest->HCN |= 0x80;    // Disowned
					slave = dest;

					// TODO: verify C behaviour
					chn->SCOffst = (dest - &ite->slave[0]);
					break;
				}
			}
		}
	}

	slave->OldSampleOffset = 0;
	slave->SmpErr = 0;
	slave->Sample_Offset = 0;

	slave->Flags |= 0x540;

	uint8_t al = slave->VS;
	int check = 0;
	switch(chn->Q00>>4)
	{
		case 0x0:
			return;

		case 0x1:
			al--;
			check = -1;
			break;

		case 0x2:
			al -= 2;
			check = -1;
			break;

		case 0x3:
			al -= 4;
			check = -1;
			break;

		case 0x4:
			al -= 8;
			check = -1;
			break;

		case 0x5:
			al -= 16;
			check = -1;
			break;

		case 0x6:
			al <<= 1;
			al /= 3;
			break;

		case 0x7:
			al >>= 1;
			break;

		case 0x8:
			return;

		case 0x9:
			al++;
			check = 1;
			break;

		case 0xA:
			al += 2;
			check = 1;
			break;

		case 0xB:
			al += 4;
			check = 1;
			break;

		case 0xC:
			al += 8;
			check = 1;
			break;

		case 0xD:
			al += 16;
			check = 1;
			break;

		case 0xE:
			al = (al + al + al) >> 1;
			check = 1;
			break;

		case 0xF:
			al <<= 1;
			check = 1;
			break;
	}

	if(check < 0 && (al & 0x80) != 0)
		al = 0;
	else if(check > 0 && al > 64)
		al = 64;

	slave->VS = slave->Vol = al;
	chn->VSe = al;
	slave->Flags |= 16; // recalc volume flag

	if(chn->Smp == 101) // MIDI sample
		MIDITranslate(ite, chn, slave, MIDICOMMAND_STOPNOTE);

}

void InitTremelo(it_engine *ite, it_host *chn)
{
	if((ite->hdr.Flags & 0x10) == 0)
	{
		CommandR(ite, chn);

	} else {
		it_slave *slave = &ite->slave[chn->SCOffst];
		slave->Flags |= 64; // Volume change...
		CommandR2(ite, chn, slave, chn->LTr);

	}
}

void CommandR(it_engine *ite, it_host *chn)
{
	it_slave *slave = &ite->slave[chn->SCOffst];
	slave->Flags |= 64; // Volume change

	// TODO: verify

	// Add speed. / Save value
	chn->TPo += chn->TSp;

	int8_t al;
	if(chn->TWF != 3)
	{
		// probably not wise to try to emulate out-of-range vibrato types.
		// well, at least for now. we can research these later. --GM
		switch(chn->TWF)
		{
			case 0:
				al = FineSineData[chn->TPo];
				break;
			case 1:
				al = FineRampDownData[chn->TPo];
				break;
			case 2:
				al = FineSquareWave[chn->TPo];
				break;
			default:
				printf("PANIC: out of range vibrato types not emulated!\n");
				abort();
		}

	} else {
		al = (Random(ite)&127)-64; // Random.

	}

	CommandR2(ite, chn, slave, al);
}

void CommandR2(it_engine *ite, it_host *chn, it_slave *slave, int8_t al)
{
	int16_t ax;
	ax = al;
	ax *= (int16_t)(int8_t)(chn->TDp);
	ax <<= 2;
	ax += 0x80;
	ax >>= 8;

	al = slave->Vol + ax;
	
	if((al & 0x80) != 0)
		al = 0;
	if(al > 64)
		al = 64;

	slave->Vol = al;
}

void CommandS(it_engine *ite, it_host *chn)
{
	// Have to handle SDx, SCx
	// AH = command, AL = value.
	uint8_t ah = (chn->_40>>8);
	uint8_t al = (chn->_40&0xFF);

	if(ah == 0xD0)
	{
		// this is just an 8-bit dec in a 16-bit value --GM
		if(chn->_40 == 0) chn->_40 |= 0xFF;
		else chn->_40--;

		if((chn->_40 & 0x80) == 0 && (chn->_40 & 0xFF) != 0)
			return;

		chn->Flags &= ~3;
		InitNoCommand(ite, chn);

		chn->Flags |= 64;

		// Check whether chn is on
		if((ite->hdr.Chnl_Vol[chn->HCN] & 0x80) == 0) return;
		if((chn->Flags & 32) != 0) return;
		if((chn->Flags & 4) == 0) return; // Channel was off.

		it_slave *slave = &ite->slave[chn->SCOffst];
		slave->Flags |= 0x800;

	} else if(ah == 0xC0) {
		if((chn->Flags & 4) == 0) return;

		// this is just an 8-bit dec in a 16-bit value --GM
		if(chn->_40 == 0) chn->_40 |= 0xFF;
		else chn->_40--;

		if((chn->_40 & 0x80) == 0 && (chn->_40 & 0xFF) != 0)
			return;

		it_slave *slave = &ite->slave[chn->SCOffst]; // Note cut.

		chn->Flags &= ~4;

		if(slave->Smp != 100 && (ite->d.DriverFlags & 2) == 0)
			slave->Flags  = 0x200;
		else
			slave->Flags |= 0x200;
	}
}

void CommandT(it_engine *ite, it_host *chn)
{
	// TODO: verify - flow somewhat different from original source --GM

	if((chn->T00 & 0xF0) != 0)
	{
		// Slide up
		ite->Tempo += chn->T00;
		ite->Tempo -= 0x10;
		if(ite->Tempo > 0xFF)
			ite->Tempo = 0xFF;

	} else {
		// Slide down
		ite->Tempo -= 0x10;
		if(ite->Tempo < 0x20)
			ite->Tempo = 0x20;
	}

	ite->d.DriverSetTempo(ite, ite->Tempo);
}

void CommandW(it_engine *ite, it_host *chn)
{
	// Global volume slide!
	int16_t ax = (int16_t)(int8_t)(chn->_40 & 0xFF);
	ax += ite->GlobalVolume;

	if((ax & 0x8000) != 0)
		ax = 0;
	
	if((ax & 0xFF) > 128)
		ax = 128;

	ite->GlobalVolume = ax;

	RecalculateAllVolumes(ite);
}

void CommandY(it_engine *ite, it_host *chn)
{
	// TODO!
}

#if 0
void CommandY(it_engine *ite, it_host *chn)
{
	Test    Byte Ptr [DI], 4
	JZ      CommandY5

	it_slave *slave = &ite->slave[chn->SCOffst];

	Mov     BH, [DI+28h]            // AL = waveform
	Cmp     BH, 3
	JAE     CommandY1

	Mov     BL, [DI+29h]            // Pos
	Add     BL, [DI+2Bh]            // Add speed.
	Mov     [DI+29h], BL            // Save value

	Mov     AL, [FineSineData+BX]       // AL = -64 -> 64

	Jmp     CommandY2

CommandY1:                                      // Random panning make
					// speed the delay time.
	Dec     Byte Ptr [DI+29h]
	JZ      CommandY6
	JS      CommandY6

	Mov     AL, [DI+2Ch]
	Jmp     CommandY2

CommandY6:
	Mov     BL, [DI+2Bh]
	Mov     [DI+29h], BL            // reset countdown.

	Call    Random
	And     AL, 127
	Sub     AL, 64

	Mov     [DI+2Ch], AL

CommandY2:
	IMul    Byte Ptr [DI+2Ah]
	SAL     AX, 2
	Add     AX, 80h
	MovZX   BX, AH
					// AH = panning change
	Mov     AL, [SI+2Bh]            // AL = panning
	Cmp     AL, 100                 // Surround?
	JE      CommandY5

	Add     AL, AH
	JNS     CommandY3

	Xor     AL, AL

CommandY3:
	Cmp     AL, 64
	JBE     CommandY4

	Mov     AL, 64

CommandY4:
	Or      Byte Ptr [SI], 2        // Panning change
	Mov     [SI+2Ah], AL

CommandY5:
	Ret

}
#endif

