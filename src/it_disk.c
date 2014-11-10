#include "it_struc.h"

const char *HeaderMsg = "File Header";
const char *InstrumentMsg = "Instrument \375D";
const char *SampleMsg = "Sample \375D";
const char *SHLoadMsg = "Sample Header \375D";
const char *PatternMsg = "Pattern \375D";

//
int D_LoadSampleData(it_engine *ite, FILE *fp, uint16_t ax)
{
	// DS:SI points to sample header
	// AX = sample number (0 based)
	
	it_sample *smp = &ite->smp[ax];

	uint32_t edx = smp->Length;
	if(edx == 0) return 0;

	uint8_t cl = smp->Flg;
	uint16_t bp = ((uint16_t)smp->Cvt) | (((uint16_t)smp->DfP)<<8);

	smp->Flg &= ~0x0C;
	smp->Cvt = 1;
	bp &= 0xFF;

	bp <<= 1;
	uint8_t ch = cl;
	ch &= 0x8; cl &= 0x02;
	ch <<= 4;
	cl >>= 1;
	bp |= ((uint16_t)ch)<<8;
	bp |= ((uint16_t)cl);

	// BP flags:
	// 1: 16 bit
	// 2: Convert unsigned->signed
	// 4: Swap bytes
	// 8: Delta values.
	// 16: Byte delta values
	// 32: 12-bit sample.
	// 64: Stereo prompt.
	// 8000h: Compressed.


	if((bp & 1) != 0)
		edx *= 2;

	if((bp & 64) != 0)
	{
		/*
		Cmp     CS:DisableStereoMenu, 0
		JNE     D_NoStereoMenu
		{

			PushAD
			Push    DS ES

			Mov     DI, Offset O1_StereoSampleList
			Mov     CX, 0FFFFh
			Call    M_Object1List

			Mov     CS:TempData, DX

			Pop     ES DS
			PopAD

			Or      BP, CS:TempData                 // 64 = left
								// 64+128 = right
		}
		D_NoStereoMenu:
		*/
		edx *= 2;
	}

	uint8_t *data = Music_AllocateSample(ite, ax, edx);
	ax++;

	int32_t edi = edx;

	if(data == NULL)
	{
		fseek(fp, smp->Length, SEEK_CUR); // Advance file ptr.
		// (pretty sure there's a bug somewhere --GM)

		//PEFunction_OutOfMemoryMessage(ite);

		return 1;
	}

	// AX = sample no.
	// CH = page no.
	// EDI = bytes remaining
	// SI = delta accumulator
	// BX = file pointer // using variable "fp" instead --GM

	ch = 0; // Start with page 0
	uint16_t si = 0;

	uint8_t *srcdata = data;
	for(; edi >= 0; edi -= 32768)
	{
		uint32_t ecx;
		int is8bit;
		//uint8_t *srcdata = Music_GetSampleLocation(ite, ax, &ecx, &is8bit);

		uint32_t buflen = 32768;

		if(edi < buflen)
			buflen = edi;

		if((bp & 32) != 0) // 12 bit format?
		{
			buflen &= 0xFFFF;
			buflen *= 3;
			buflen += 3;
			buflen >>= 2; // = bytes to read.
		}

		// DS:DX point to buffer. For compressed samples, use patterndata area.
		/*
		Test    BP, BP
		JNS     LoadCompressedSample1
		{
			Push    DS
			Push    CX
			Push    DX

			Mov     DX, Pattern
			Mov     DS, DX
				Assume DS:Pattern

			Mov     DS, Word Ptr [PatternDataArea]
				Assume DS:Nothing

			Xor     DX, DX
			Mov     CX, 2
			Mov     AH, 3Fh
			Int     21h
			Mov     CX, [DS:0]              // Bytes to read.
			Xor     DX, DX                  // Compressed chunk.
		}
		LoadCompressedSample1:
		*/

		uint8_t *newsrcdata = srcdata + buflen;
		buflen = fread(srcdata, 1, buflen, fp);

		// Now to decompress samples, if required.
		/*
		Test    BP, BP
		JNS     LoadCompressedSample3
		{
			Pop     DI
			Pop     CX
			Pop     ES
			Xor     SI, SI

			Test    BP, 1
			JNZ     LoadCompressedSample2

			Call    D_Decompress8BitData    // 8 bit decode.
			Jmp     LoadCompressedSample4

		LoadCompressedSample2:                          // 16 bit decode
			Call    D_Decompress16BitData

		LoadCompressedSample4:
			Push    ES
			Pop     DS

			Xor     SI, SI
			Jmp     SecondDelta
		}
		LoadCompressedSample3:
		*/

		if((bp & 32) != 0) // 12-bit sample?
		{
			// TODO!
			/*
			// CX = number of bytes read.
			//    = 3*2 number of sample read
			buflen = (buflen + 2) / 3;

			// SI = AX * 3
			LEA     SI, [EAX*2+EAX]         // SI = AX * 3
			LEA     DI, [EAX*4+EDX]
			Add     SI, DX

			Test    CX, CX
			JZ      ConvertTXSample2

			Push    CX

		ConvertTXSample1:
			Sub     SI, 3
			Sub     DI, 4

			Mov     AX, [SI+1]
			ShL     AL, 4
			ShL     EAX, 16
			Mov     AH, [SI]
			Mov     AL, [SI+1]
			And     AL, 0F0h
			Mov     [DI], EAX

			Loop    ConvertTXSample1

			Pop     CX
			Pop     SI
			ShL     CX, 1
			Jmp     D_LoadSampleData10

		ConvertTXSample2:
			Pop     SI
			*/
		} else {
			// CX = number of bytes read

		//SecondDelta:
			if((bp & 1) != 0) // 16 bit?
				buflen >>= 1;

		//D_LoadSampleData10:
			if((bp & 5) == 5) // 16 bit and BSwap?
			{
				uint16_t ctr = buflen;
				uint8_t *dfol = srcdata;

				while(ctr-- != 0)
				{
					uint8_t datl = dfol[0];
					uint8_t dath = dfol[1];
					dfol[0] = dath;
					dfol[1] = datl;

					dfol += 2;
				}
			}

			if((bp & 24) != 0) // Delta values?
			{
				if((bp & 1) != 0 && (bp & 16) == 0)
				{
					// 16 bit delta
					uint16_t ctr = buflen;
					uint16_t *dfol = (uint16_t *)srcdata;

					while(ctr-- != 0)
					{
						si += *dfol;
						*(dfol++) = si;
					}

				} else {
					// 8 bit delta
					uint16_t ctr = buflen;
					uint8_t *dfol = srcdata;

					if((bp & 1) != 0)
						ctr *= 2;

					while(ctr-- != 0)
					{
						si += ((uint16_t)(*dfol))<<8;
						*(dfol++) = (si>>8);
					}
				}
			}

			if((bp & 2) == 0) // unsigned->signed?
			{
				if((bp & 1) == 0)
				{
					// 8 bit
					uint16_t ctr = buflen;
					uint8_t *dfol = srcdata;

					while(ctr-- != 0)
						*(dfol++) ^= 0x80;
				} else {
					// 16 bit..
					uint16_t ctr = buflen;
					uint16_t *dfol = (uint16_t *)srcdata;

					while(ctr-- != 0)
						*(dfol++) ^= 0x8000;
				}
			}
		}

	D_LoadSampleData6:
		if((bp & 64) != 0) // Stereo?
		{
			// TODO!
			abort();
			/*
			Push    SI ES

			Push    DS
			Pop     ES

			Mov     SI, DX
			Mov     DI, DX

			ShR     CX, 1
			JZ      D_LoadSampleDataEndStereo

			Test    BP, 1           // 8/16 bit?
			JNZ     D_LoadSampleDataStereo16BitStart

			Test    BP, 128
			JZ      D_LoadSampleDataStereo8Bit

			Inc     SI

		D_LoadSampleDataStereo8Bit:
			MovsB
			Inc     SI
			Loop    D_LoadSampleDataStereo8Bit
			Jmp     D_LoadSampleDataEndStereo

		D_LoadSampleDataStereo16BitStart:
			Test    BP, 128
			JZ      D_LoadSampledataStereo16Bit

			LodsW

		D_LoadSampleDataStereo16Bit:
			MovsW
			LodsW
			Loop    D_LoadSampleDataStereo16Bit

		D_LoadSampleDataEndStereo:
			Pop     ES SI

			Pop     EDI
			Pop     CX
			Pop     AX

			Inc     CH
			Jmp     D_LoadSampleDataNextChain
			*/
		}

		srcdata = newsrcdata;
		ch += 2;

	//D_LoadSampleDataNextChain:
	}

	return 0;
}

FILE *D_PreLoadModule(it_engine *ite, const char *fname, uint8_t al)
{
	// Returns ES = song segment
	// BX = file handle
	// DS = Diskdata area
	// AX = SaveFormat

#if DEFAULTFORMAT
	if(al == 0)
		al = DEFAULTFORMAT;
#endif

	ite->SaveFormat = al;

	//I_ClearTables(ite);
	Music_ReleaseAllPatterns(ite);
	Music_ReleaseAllSamples(ite);
	//Music_ClearAllSampleNames(ite);
	Music_ClearAllInstruments(ite);
	//Msg_ResetMessage(ite);
	//ReleaseTimerData(ite);

	// this is ridiculous, let's write something simpler for now --GM
	FILE *fp = fopen(fname, "rb");

	if(fp == NULL) return NULL;

	return fp;

	/*
	Mov     DS, CS:DiskDataArea

	Mov     BX, CS:CurrentFile
	Add     BX, BX
	Mov     BX, [BX]
	Add     BX, 8

	Push    CS
	Pop     ES
	Mov     DI, Offset FileName             // OK...
	Mov     SI, BX                          // Data area no longer
	Mov     CX, 13                          //  reqd
	Rep     MovsB

	Mov     AX, 3D00h
	Mov     DX, BX
	Int     21h
	JC      D_LoadFileOpenErrorMsg

	Mov     BX, AX                          // BX = file handle.

	Call    Music_GetSongSegment
	Mov     ES, AX
	*/
}

void D_PostLoadModule(it_engine *ite, FILE *fp)
{
	fclose(fp); // Close file.

	//CheckTimerData(ite);
	//GetCurrentTime(ite);
	ite->TopTimerData = 0;
	//ite->EditTimer = GetTimerCounter();

	//PE_ResetOrderPattern(ite);
}

int D_LoadIT(it_engine *ite, const char *fname)
{
	FILE *fp = D_PreLoadModule(ite, fname, 0);
	printf("loading %p\n", fp);

	S_DrawString(ite, 4, 16, HeaderMsg, 5);

	// why does this load 2KB i mean what --GM
	// TODO: serialise this --GM
	fread(&ite->hdr, 1, 0xC0, fp);

	printf("ord=%-3i ins=%-3i smp=%-3i pat=%-3i\n",
		ite->hdr.OrdNum,
		ite->hdr.InsNum,
		ite->hdr.SmpNum,
		ite->hdr.PatNum);
	/*
	Mov     AH, 3Fh
	Mov     CX, 2048
	Xor     DX, DX
	Int     21h
	*/

	if(ite->hdr.Cwt_v >= 0x0208)
	{
		ite->hdr.Time_Stamp ^= 0x4B525449; // 'ITRK' - TODO CONFIRM BYTE ORDER
		ite->hdr.Time_Stamp = (ite->hdr.Time_Stamp >> 7) | (ite->hdr.Time_Stamp << (32-7));
		ite->hdr.Time_Stamp = -ite->hdr.Time_Stamp;
		ite->hdr.Time_Stamp = (ite->hdr.Time_Stamp << 4) | (ite->hdr.Time_Stamp >> (32-4));
		ite->hdr.Time_Stamp ^= 0x4C48544A; // 'JTHL' - TODO CONFIRM BYTE ORDER
	}

	if((ite->hdr.Special & 2) != 0) // Time data?
	{
		// Seek to 0C0+Orders+
		// (ins+samp+pat)*4
		
		// TODO!
		/*
		Mov     DX, [DS:22h]
		Add     DX, [DS:24h]
		Add     DX, [DS:26h]
		ShL     DX, 2
		Add     DX, [DS:20h]
		Add     DX, 0C0h
		Xor     CX, CX
		Mov     AX, 4200h
		Int     21h

		Push    DS

		Push    CS
		Pop     DS

		Mov     AH, 3Fh
		Mov     CX, 2
		Mov     DX, Offset NumTimerData
		Int     21h

		Push    BX              // Allocate data for timedata
		Mov     BX, NumTimerData
		Cmp     BX, 0FFFFh
		JNE     D_NoTimerDataOverFlow

		Dec     BX
		Mov     NumTimerData, BX

	D_NoTimerDataOverFlow:
		Mov     CX, BX
		ShR     BX, 1
		Inc     BX
		Mov     AH, 48h
		Int     21h
		Pop     BX
		JC      D_LoadTimeDataEnd

		Mov     TimerData, AX
		Mov     DS, AX
		ShL     CX, 3
		Xor     DX, DX
		Mov     AH, 3Fh
		Int     21h

	D_LoadTimeDataEnd:
		Pop     DS
		*/
	}

	if((ite->hdr.Special & 8) != 0)
	{
		// TODO: MIDI --GM

		/*
		PushA
		Push    DS

		Call    Music_GetMIDIDataArea
		Xor     DX, DX
		Mov     CX, 4896
		Mov     AH, 3Fh
		Int     21h

		Pop     DS
		PopA
		*/
	}

	if((ite->hdr.Special & 1) != 0)
	{
		// Load the message
		// Move to offset first.

		// TODO
		/*
		Mov     AX, 4200h
		Mov     CX, [DS:3Ah]
		Mov     DX, [DS:38h]
		Int     21h             // Seek to position

		Push    DS

		Mov     CX, [DS:36h]
		Call    Msg_GetMessageOffset
		Mov     AH, 3Fh
		Int     21h

		Pop     DS
		*/
	}

	// Actually, load row hilights first...
	if((ite->hdr.Special & 4) != 0)
	{
		// TODO --GM
		//ite->RowHilight1 = ite->hdr.PHiligt;
	}

	/*
	Xor     SI, SI
	Xor     DI, DI
	Mov     CX, 192
	Rep     MovsB                           // Header
	*/

	// TODO: verify
	// Orders
	fseek(fp, 0x00C0, SEEK_SET);
	fread(ite->ord, 1, ite->hdr.OrdNum, fp); // XXX: limit OrdNum to >= 1
	ite->ord[ite->hdr.OrdNum] = 0xFF; // TODO: verify

	// SI points to first pointer
	// this is different from the actual code --GM
	uint32_t iptrs[100];
	uint32_t sptrs[100];
	uint32_t pptrs[100];
	assert(ite->hdr.InsNum <= 99);
	assert(ite->hdr.SmpNum <= 99);
	assert(ite->hdr.PatNum <= 99);

	fread(iptrs, 4, ite->hdr.InsNum, fp);
	fread(sptrs, 4, ite->hdr.SmpNum, fp);
	fread(pptrs, 4, ite->hdr.PatNum, fp);

	// Instrument time.
	uint16_t bp;
	it_instrument *ins;
	it_sample *smp;
	it_pattern *pat;

	for(bp = 0, ins = &ite->ins[0]; bp < ite->hdr.InsNum; bp++, ins++)
	{
		// TODO: num args
		S_DrawString(ite, 4, 17, InstrumentMsg, 5);

		// Move to offset..
		fseek(fp, iptrs[bp], SEEK_SET);

		fread(ins, 1, 554, fp);

		if(ite->hdr.Cmwt < 0x200)
		{
			// TODO!
			abort();
			/*
			Mov     SI, DX
			Call    ConvertOldInstrument
			*/
		}
	}

	// Sample header time.
	for(bp = 0, smp = &ite->smp[0]; bp < ite->hdr.SmpNum; bp++, smp++)
	{
		// TODO: num args
		S_DrawString(ite, 4, 18, SHLoadMsg, 5);

		// Move to offset..
		fseek(fp, sptrs[bp], SEEK_SET);

		fread(smp, 1, 80, fp);
		//printf("smp len %i = %i\n", bp, smp->Length);
	}

	// DS now points to song data.
	for(bp = 0, smp = &ite->smp[0]; bp < 99; bp++, smp++)
	{
		if((smp->Flg & 1) != 0)
		{
			S_DrawString(ite, 4, 19, SampleMsg, 5);

			// Move file pointer.
			fseek(fp, smp->SamplePointer, SEEK_SET);
			ite->SamplePointer[bp] = NULL;
			smp->SamplePointer = 0;

			D_LoadSampleData(ite, fp, bp);
			printf("smp %2i flg=%02X len=%i\n", bp, smp->Flg, smp->Length);

		} else {
			smp->SamplePointer = 0;
			ite->SamplePointer[bp] = NULL;
		}
	}

	// Pattern time.
	for(bp = 0; bp < ite->hdr.PatNum; bp++)
	{
		S_DrawString(ite, 4, 20, PatternMsg, 5);

		// Move to offset..
		if(pptrs != 0)
		{
			fseek(fp, pptrs[bp], SEEK_SET);

			fread(ite->patspace, 1, 8, fp);

			uint16_t length = ((uint16_t)ite->patspace[0])
				| (((uint16_t)ite->patspace[1])<<8);

			printf("pat %-3i %-3i %-5i\n", bp, ite->patspace[2], length);
			ite->pat[bp] = pat = Music_AllocatePattern(ite, length + 8);
			if(pat != NULL)
			{
				memcpy((uint8_t *)pat, (uint8_t *)ite->patspace, 8);
				fread(pat->data, 1, length, fp);
			} else {
				//PEFunction_OutOfMemoryMessage(ite);
				abort();
			}
		}
	}

	D_PostLoadModule(ite, fp);
	return 0;
}

