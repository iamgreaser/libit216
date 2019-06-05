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

const char *HeaderMsg = "File Header";
const char *InstrumentMsg = "Instrument \375D";
const char *SampleMsg = "Sample \375D";
const char *SHLoadMsg = "Sample Header \375D";
const char *PatternMsg = "Pattern \375D";

// TODO: prevent buffer read overruns
// note, it's entirely likely that I'm slightly off for exceptional behaviour --GM
void D_Decompress16BitData(uint16_t *dest, uint8_t *src, uint16_t len)
{
	// Register usage:
	// BX = LastValue
	// CH = Bitdepth
	// CL = 16-Bitdepth, 0 for Bitdepth > 16
	// DL = Bitsread
	// DH = scratch

	int counter = len>>1;
	uint8_t bitdepth = 17;
	uint8_t ibitdepth = 0;
	uint16_t lastvalue = 0;
	uint8_t bitsread = 0;
	uint8_t scratch = 0;

	while(counter != 0)
	{
		// Push    CX

		uint32_t eax = *(uint32_t *)src;
		eax >>= bitsread;

		bitsread += bitdepth;
		src += bitsread>>3;
		bitsread &= 7;

		// Pop     CX

		if(bitdepth <= 6)
		{
			eax <<= ibitdepth;

			if((eax & 0xFFFF) != 0x8000)
			{
				lastvalue += (uint16_t)(((int16_t)eax)>>ibitdepth);
				*(dest++) = lastvalue;
				counter--;
				continue;
			}

			uint8_t newbits = (eax>>16) & 15;
			newbits++;

			// Advance bits
			bitsread += 4;
			if(newbits >= bitdepth)
				newbits += 1;

			ibitdepth = 16;
			bitdepth = newbits;
			if(ibitdepth < bitdepth)
			{
				ibitdepth -= bitdepth;
				ibitdepth++;
			} else {
				ibitdepth -= bitdepth;
			}

		} else if(bitdepth <= 16) {

			// Push    DX

			uint16_t tmp_dx = 0xFFFF;
			tmp_dx >>= ibitdepth;
			uint16_t tmp_ax = (uint16_t)eax;
			tmp_ax &= (uint16_t)tmp_dx;
			tmp_dx >>= 1;
			tmp_dx += 8;
			tmp_dx -= 16;
			if(tmp_ax > (uint16_t)(tmp_dx+16) || tmp_ax <= (uint16_t)tmp_dx)
			{
				eax <<= ibitdepth;
				lastvalue += (uint16_t)(((int16_t)eax)>>ibitdepth);
				*(dest++) = lastvalue;
				counter--;
				continue;
			}

			// Pop     DX

			uint8_t newbits = (uint8_t)(tmp_ax - tmp_dx);
			if(newbits >= bitdepth)
				newbits += 1;

			ibitdepth = 16;
			bitdepth = newbits;
			if(ibitdepth < bitdepth)
			{
				ibitdepth -= bitdepth;
				ibitdepth++;
			} else {
				ibitdepth -= bitdepth;
			}
		} else {

			if((eax & 0x10000) == 0)
			{
				lastvalue += (uint16_t)eax;
				*(dest++) = lastvalue;
				counter--;
				continue;
			}

			eax++; // Inc AX actually
			bitdepth = (uint8_t)(eax&0xFF);
			ibitdepth = 16 - bitdepth;
		}
	}

}

void D_Decompress8BitData(uint8_t *dest, uint8_t *src, uint16_t len)
{
	// DS:SI = source
	// ES:DI = destination
	// CX = count.

	// Register usage:
	// BH = Bitdepth
	// BL = lastvalue
	// CL = 8-bitdepth, undefined for bitdepth > 8
	// CH = Bitsread

	// DX = scratch

	int counter = len; // BP = counter;
	uint8_t bitdepth = 9;
	uint8_t ibitdepth = 0;
	uint8_t lastvalue = 0;
	uint8_t bitsread = 0;
	uint16_t scratch = 0;

	while(counter != 0)
	{
		// Get bits loaded into AX properly.
		uint16_t ax = *(uint16_t *)src;
		ax >>= bitsread;

		// Advance SI as necessary.
		bitsread += bitdepth;
		src += bitsread>>3;
		bitsread &= 7;

		uint8_t tmp_al;
		if(bitdepth <= 6)
		{
			ax <<= ibitdepth;

			if((ax & 0xFF) != 0x80)
			{
				lastvalue += (uint8_t)(((int8_t)ax)>>ibitdepth);
				*(dest++) = lastvalue;
				counter--;
				continue;
			}

			tmp_al = (ax>>8);
			bitsread += 3;

			tmp_al &= 7;
			scratch = bitsread;
			bitsread &= 7;
			scratch >>= 3;

			src += scratch;

		} else {
			tmp_al = (ax & 0xFF);

			if(bitdepth > 8)
			{
				// 9 bit representation
				ax &= 0x1FF;

				if((ax & 0x100) == 0)
				{
					lastvalue += (uint8_t)tmp_al;
					*(dest++) = lastvalue;
					counter--;
					continue;
				}

			} else if(bitdepth == 8) {
				if(tmp_al < 0x7C || tmp_al > 0x83)
				{
					lastvalue += (uint8_t)tmp_al;
					*(dest++) = lastvalue;
					counter--;
					continue;
				}

				tmp_al -= 0x7C;

			} else {

				tmp_al <<= 1;
				if(tmp_al < 0x78 || tmp_al > 0x86)
				{
					lastvalue += (uint8_t)(((int8_t)tmp_al)>>1);
					*(dest++) = lastvalue;
					counter--;
					continue;
				}

				tmp_al >>= 1;
				tmp_al -= 0x3C;

			}
		}

		ibitdepth = 8;
		tmp_al++;

		//Cmp     AL, BH
		//SBB     AL, 0FFh
		//Mov     BH, AL
		//Sub     CL, AL
		//AdC     CL, 0
		if(tmp_al >= bitdepth)
			tmp_al++;
		bitdepth = tmp_al;

		if(ibitdepth < tmp_al)
		{
			ibitdepth -= tmp_al;
			ibitdepth++;
		} else {
			ibitdepth -= tmp_al;
		}

		continue;

	}
}

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
	for(; edi > 0; edi -= 32768)
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

		uint8_t *newsrcdata = srcdata + buflen;
		uint16_t packed_len;
		uint8_t packed_data[0x10000];
		size_t packed_len_long;

		// DS:DX point to buffer. For compressed samples, use patterndata area.
		if((bp & 0x8000) != 0)
		{
			/*
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
			*/
			fread(&packed_len, 2, 1, fp);
			packed_len_long = packed_len;
			packed_len_long = fread(packed_data, 1, packed_len_long, fp);

		} else {
			buflen = fread(srcdata, 1, buflen, fp);
		}

		// Now to decompress samples, if required.
		if((bp & 0x8000) != 0)
		{
			// TODO
			if((bp & 1) == 0)
				D_Decompress8BitData(srcdata, packed_data, buflen); // 8 bit decode.
			else
				D_Decompress16BitData((uint16_t *)srcdata, packed_data, buflen); // 16 bit decode

			si = 0;
		}

		// flag skipped if sample compressed
		if((bp & 0x8000) == 0 && (bp & 32) != 0) // 12-bit sample?
		{
			// TODO!
			abort();
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
		printf("Timestamp: %08X\n", ite->hdr.Time_Stamp);
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
	uint32_t pptrs[200];
	assert(ite->hdr.InsNum <= 99);
	assert(ite->hdr.SmpNum <= 99);
	assert(ite->hdr.PatNum <= 199);

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

