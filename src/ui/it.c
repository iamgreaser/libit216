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

// Externals

/*
Segment         Object1 BYTE Public 'Data'
                Extrn   HelpKeyValue:Word, OrderKeyValue:Word
EndS

Segment         InfoLine BYTE Public 'Code' USE16
                Extrn   ShowUsageTime:Byte
EndS

Segment         Disk BYTE Public 'Code' USE16
                Extrn   DiskOptions:Byte
EndS

Segment         Screen BYTE Public 'Code'
                Extrn   CharacterGenerationOffset:Word
                Extrn   VGAFlags:Byte
EndS

Segment         Mouse BYTE Public 'Code'
                Extrn   MouseCharacterGenerationOffset:Word
EndS

Segment         Main DWORD Public 'Code' USE16
                Extrn   ReleaseTimeSlice:Byte
EndS

                Extrn   D_InitDisk:Far
                Extrn   D_UnInitDisk:Far
                Extrn   D_DisableFileColours:Far

                Extrn   E_InitEMS:Far
                Extrn   E_UnInitEMS:Far

                Extrn   Error_InitHandler:Far
                Extrn   Error_UnInitHandler:Far

                Extrn   K_InitKeyBoard:Far
                Extrn   K_UnInitKeyBoard:Far
                Extrn   K_InstallKeyboardType:Far
                Extrn   K_RemoveKeyboardType:Far

                Extrn   K_InstallDOSHandler:Far
                Extrn   K_UnInstallDOSHandler:Far
                Extrn   K_SwapKeyBoard:Far

                Extrn   O1_AutoDetectList:Far
                Extrn   O1_ConfirmQuit:Far
                Extrn   O1_PatternEditList:Far
                Extrn   O1_CrashRecovery:Far
                Extrn   O1_KeyboardList:Far

                Extrn   M_Object1List:Far

                Extrn   S_InitScreen:Far
                Extrn   S_ClearScreen:Far
                Extrn   S_UnInitScreen:Far
                Extrn   S_SetDirectMode:Far
                Extrn   S_DrawString:Far

                Extrn   Music_InitMusic:Far
                Extrn   Music_UnInitMusic:Far

                Extrn   Music_SetLimit:Far
                Extrn   Music_SetSoundCard:Far
                Extrn   Music_SetDMA:Far
                Extrn   Music_SetIRQ:Far
                Extrn   Music_SetMixSpeed:Far
                Extrn   Music_SetAddress:Far
                Extrn   Music_ReverseChannels:Far
                Extrn   Music_PatternStorage:Far
                Extrn   Music_SetSoundCardDriver:Far
                Extrn   Music_Stop:Far
                Extrn   Music_AutoDetectSoundCard:Far

IF NETWORKENABLED
                Extrn   Network_Shutdown:Far
ENDIF

                Extrn   PE_InitPatternEdit:Far
                Extrn   PE_UnInitPatternEdit:Far
                Extrn   PECheckModified:Far

                Extrn   D_RestorePreShellDirectory:Far
                Extrn   D_GetPreShellDirectory:Far

                Extrn   MMTSR_InstallMMTSR:Far
                Extrn   MMTSR_UninstallMMTSR:Far

                Extrn   InitMouse:Far, UnInitMouse:Far
                Extrn   CmdLineDisableMouse:Far

                Extrn   InitTimerHandler:Far, UnInitTimerHandler:Far

                Global  Quit:Far, Refresh:Far
                Global  DOSShell:Far, GetEnvironment:Far
                Global  CrashRecovery:Far

                Public  IsStartupKeyList
                Public  GetStartupKeyList

*/

/*
Segment                 StartUp BYTE Public 'Code' USE16
                        Assume  CS:StartUp, DS:Nothing, ES:Nothing
*/

/*
CREATENEWLOGFILE        EQU     1
include debug.inc
*/

// Variables

#define StackSize 0x1000

// these are totally going to be useful, har har --GM
const char No386Msg[] = "Sorry, Impulse Tracker requires a 386+ processor to run.\n$";
const char WindowsMsg[] = "Microsoft Windows detected.\n\n"
	"Due to instabilities in Windows, it is highly recommended that you run\n"
	"Impulse Tracker in DOS instead.\n\n"
	"Press any key to continue...";

uint16_t PSP;
uint8_t LoadMMTSR = 1;
uint8_t Pause = 0;
uint8_t COMSPECFound = 0;
uint32_t COMSPEC = 0;
uint8_t Control = 0;
uint8_t CommandTail[] = {1, 0, 13};

uint8_t FCB1[] = {0, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }; 
uint8_t FCB2[] = {0, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, }; 

const char COMSPECString[] = "COMSPEC";
const char DefaultShell[] = "C:\\COMMAND.COM";

// I AM NOT PORTING THIS BIT. --GM
/*
EXECData        DW      0               ; Inherit same environment block
                DW      Offset CommandTail, Startup
                DW      Offset FCB1, Startup
                DW      Offset FCB2, Startup
*/
const char ShellMsg[] = "Type EXIT to return to Impulse Tracker";

#ifdef SHOWREGISTERNAME
#include "wavswitc.h"
#include "username.h"
#endif

const char CmdLineHelp[] =
#if SHOWVERSION
                "Impulse Tracker 2.14, Copyright (C) 1995-2000 Jeffrey Lim\n"
		"\n"
                "  Usage: IT.EXE [Switches]\n"
#else
                "Impulse Tracker, Copyright (C) 1995-2000 Jeffrey Lim\n"
#if SHOWREGISTERNAME
                "Registered to: "
                REGISTERNAME
		"\n"
#endif
#endif
		"\n"
                "Switches:\n"
                "  SFilename.Drv  Select sound card driver\n"
                "  S#\t\t Quick select sound card\n"
                "\tS0: No sound card\t\t S9: Pro Audio Spectrum\n"
                "\tS1: PC Speaker\t\t\tS10: Pro Audio Spectrum 16\n"
                "\tS2: Sound Blaster 1.xx\t\tS11: Windows Sound System\n"
                "\tS3: Sound Blaster 2.xx\t\tS12: ESS ES1868 AudioDrive\n"
                "\tS4: Sound Blaster Pro\t\tS13: EWS64 XL Codec\n"
                "\tS5: Sound Blaster 16\t\tS14: Ensoniq SoundscapeVIVO\n"
                "\tS6: Sound Blaster AWE 32\tS19: Generic MPU401\n"
                "\tS7: Gravis UltraSound\t\tS20: Disk Writer (WAV)\n"
                "\tS8: AMD Interwave\t\tS21: Disk Writer (MID)\n"
		"\n"
                "  Axxx\tSet Base Address of sound card (hexadecimal)\n"
                "  I###\tSet IRQ of sound card (decimal)\n"
                "  D###\tSet DMA of sound card (decimal)\n"
		"\n"
                "  M###\tSet Mixspeed\n"
                "  L###\tLimit number of channels (4-256)\n"
		;

const char MixErrorMsg[]     = "Error: Mixing speed invalid - setting ignored\n";
const char AddressErrorMsg[] = "Error: Base Address invalid - setting ignored\n";
const char IRQErrorMsg[]     = "Error: IRQ number invalid - setting ignored\n";
const char LimitErrorMsg[]   = "Error: Channel limit number invalid - setting ignored\n";
const char ContinueMsg[]     = "Press any key to continue...";

#if 0
StartupList             DB      0
StartupMode             DB      0       ; Load? Or Load/Play? or Load/Save
StartupFileOffset       DW      0
StartupFileSegment      DW      0
StartupKeyListFunction  DW      Offset GetStartupKeyList1

                                ; CX,DX
StartupInformation      DW      11Ch, 0         ; Enter
                        DW      1Ch,  0         ; Release Enter
                        DW      10Fh, 0         ; Tab
                        DW      10Fh, 0         ; Tab
                        DW      10Fh, 0         ; Tab
                        DW      90Eh, 127       ; Ctrl-Backspace

ENDSTARTUPINFORMATION   EQU     $

SaveInformation         DW      0, 0            ; Loading screen key-loss
                        DW      1FFh, 13h       ; Ctrl-S
                        DW      0, 0            ; Saving screen key-loss
                        DW      1FFh, 11h       ; Ctrl-Q
                        DW      1FFh, 'Y'       ; 'Y'

ENDSAVEINFORMATION      EQU     $

StartupQueueOffset      DW      Offset StartupInformation
StartupQueueEnd         DW      Offset ENDSTARTUPINFORMATION
StartupQueueNextFunction DW     Offset GetStartupKeyList2

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³ Functions                                                                   ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

#endif
uint8_t CapitaliseAL(uint8_t c)
{
	if(c >= 'a' && c <= 'z')
		c += 'A'-'a';
	
	return c;
}

#if 0
Proc            GetDecimalNumber        ; Returns CX

	LodsB
	Cmp     AL, '0'
	JB      GetDecimalNumber1
	Cmp     AL, '9'
	JBE     GetDecimalNumber2

GetDecimalNumber1:
	StC
	Ret

GetDecimalNumber2:
	Xor     CX, CX

GetDecimalNumber3:
	Mov     BL, AL
	Sub     BL, '0'
	Xor     BH, BH
	Mov     AX, 10
	Mul     CX
	Add     AX, BX
	Mov     CX, AX
	LodsB
	And     DX, DX
	JNZ     GetDecimalNumber1

	Cmp     AL, '0'
	JB      GetDecimalNumber4
	Cmp     AL, '9'
	JA      GetDecimalNumber4
	Jmp     GetDecimalNumber3

GetDecimalNumber4:
	ClC
	Ret

EndP            GetDecimalNumber
#endif

void Quit1(it_engine *ite)
{
	//PECheckModified();
	Music_Stop(ite);

#if NETWORKENABLED
	Network_Shutdown();
#endif

	//MMTSR_UninstallMMTSR();
	//PE_UnInitPatternEdit();
	Music_UnInitMusic(ite);
	//UnInitMouse();
	//S_UnInitScreen();
	//E_UnInitEMS();
	//K_UnInitKeyBoard();
	//Error_UnInitHandler();
	//D_UnInitDisk();
	//K_RemoveKeyboardType();
	//UnInitTimerHandler();

	exit(0);
}

void Start(it_engine *ite, const char *cmdtail)
{
	const char *v;
	char c;

	// 386 check.

	printf("Impulse Tracker Startup\n");

	// if(do_some_386_check()) { printf(No386Msg); exit(2) };

	//Found386:
	// Push    0B800h                          ; DEBUG!!!!!
	// Pop     GS                              ; DEBUG!!!!!

	/*
	Push    CX
	PopF

	ClD

	Mov     [CS:PSP], ES

	Mov     AX, ES
	Mov     BX, SS
	Sub     BX, AX
	Add     BX, StackSize / 16       ; Add BX, <Size reqd for Stack in para>
	Mov     AH, 4Ah                 ; Re-allocate memory
	Int     21h
	*/

	// Check for 386 here.

	/*
					; Do command line stuff.
	Mov     SI, 81h                 ; DS:SI points to cmdtail
	*/

	v = cmdtail;
	for(;;)
	{
		/*
		Push    ES
		Pop     DS
		*/

		c = *(v++);

	CmdLine3:
		if(c == '\r' || c == '\x00')
			goto CmdLineEnd;

		c = CapitaliseAL(c);

	CmdLine2:
		switch(c)
		{
			case 'K':
				// KeyboardSwap
				break;

			case 'F':
				// DisableColours
				break;

			case 'C':
				// SetControl
				break;

			case 'H':
			case '?':
				// ShowCmdLineHelp
				break;

			case 'S':
				// SetSoundCard1
				break;

			case 'D':
				// SetDMA1
				break;

			case 'M':
				// SetMixSpeed1
				break;

			case 'I':
				// SetIRQ1
				break;

			case 'A':
				// SetAddress1
				break;

			case 'V':
				// OverrideVGA
				break;

			case 'L':
				// Limit1
				break;

			case 'R':
				// Reverse1
				break;

			case 'X':
				// DisableFeatures
				break;

			case 'P':
				// PatternStorage
				break;

			case 'T':
				// NoShowUsageTime
				break;

			case 'W':
				// ConvertModule
				break;
		}

/*
NoShowUsageTime:
	LodsB
	Cmp     AL, '1'
	JNE     NoReleaseTimeSlice

	Push    DS

	Push    InfoLine
	Pop     DS
		Assume DS:InfoLine

	Mov     [ShowUsageTime], 0

	Pop     DS
	Jmp     CmdLine1
		Assume DS:Nothing

NoReleaseTimeSlice:
	Cmp     AL, '2'
	JNE     CmdLine3

NoReleaseTimeSlice2:
	Push    DS

	Push    Main
	Pop     DS
		Assume DS:Main

	Mov     [ReleaseTimeSlice], 1

	Pop     DS
	Jmp     CmdLine1

PatternStorage:
	LodsB
	Sub     AL, '0'
	JC      PatternStorageEnd
	Cmp     AL, 2
	JA      PatternStorageEnd
	Call    Music_PatternStorage
	Jmp     CmdLine1

PatternStorageEnd:
	Jmp     CmdLine3

DisableFeatures:
	LodsB
	Cmp     AL, '1'
	JB      CmdLine3
	JE      DisableMMTSR

	Cmp     AL, '3'
	JB      DisableMouse
	JE      DisableDetectDriveMap

	Cmp     AL, '5'
	JB      DisableCacheFiles
	Jmp     CmdLine3

DisableMouse:
	Call    CmdLineDisableMouse
	Jmp     CmdLine1

DisableMMTSR:
	Mov     [LoadMMTSR], 0
	Jmp     CmdLine1

DisableDetectDriveMap:
	Push    Disk
	Pop     DS
		Assume DS:Disk

	Or      [DiskOptions], 1
	Jmp     CmdLine1
		Assume DS:Nothing

DisableCacheFiles:
	Push    Disk
	Pop     DS
		Assume DS:Disk

	Or      [DiskOptions], 2
	Jmp     CmdLine1
		Assume DS:Nothing

KeyboardSwap:
	Mov     AX, Object1
	Mov     DS, AX
		Assume DS:Object1

	Mov     [HelpKeyValue], 157h
	Mov     [OrderKeyValue], 13Bh

	Jmp     CmdLine1
		Assume DS:Nothing

DisableColours:
	Call    D_DisableFileColours
	Jmp     CmdLine1

Reverse1:
	Call    Music_ReverseChannels
	Jmp     CmdLine1

OverrideVGA:
	LodsB

	Mov     CX, Screen
	Mov     DS, CX
		Assume DS:Screen

	Cmp     AL, '1'
	JE      OverrideVGA1
	Cmp     AL, '2'
	JE      Matrox
	Cmp     AL, '3'
	JE      WaitforRetrace
	Cmp     AL, '4'
	JE      Retrace
	Jmp     CmdLine3

OverrideVGA1:
	Or      [VGAFlags], 1
	Jmp     CmdLine1

WaitforRetrace:
	Or      [VGAFlags], 4
	Jmp     CmdLine1

Retrace:
	Or      [VGAFlags], 2
	Jmp     CmdLine1

Matrox:
	Mov     [CharacterGenerationOffset], 256*32

	Mov     AX, Mouse
	Mov     DS, AX
		Assume DS:Mouse
	Mov     [MouseCharacterGenerationOffset], 256*32

	Jmp     CmdLine1
		Assume DS:Nothing

SetControl:
	Mov     [CS:Control], 1
	Jmp     CmdLine1

ShowCmdLineHelp:
	Push    CS
	Pop     DS

	Mov     AH, 9
	Mov     DX, Offset CmdLineHelp
	Int     21h

	Mov     AX, 4C00h
	Int     21h

SetSoundCard1:
	Call    GetDecimalNumber
	JC      SetSoundCardDriver
	Cmp     CX, 21
	JA      SetSoundCard2

	Push    AX
	Mov     AX, CX
	Call    Music_SetSoundCard
	Pop     AX

SetSoundCard2:
	Jmp     CmdLine3

SetSoundCardDriver:
	Cmp     AL, 32
	JBE     CmdLine3

	Dec     SI
	Call    Music_SetSoundCardDriver

SetSoundCardDriver1:
	LodsB
	Cmp     AL, 32
	JA      SetSoundCardDriver1

	Mov     Byte Ptr [SI-1], 0
	Cmp     AL, 32
	JE      CmdLine1
	Jmp     CmdLineEnd

ConvertModule:
	Mov     [CS:StartupList], 1
	Mov     [CS:StartupFileOffset], SI
	Mov     [CS:StartupFileSegment], DS
	Jmp     SetSoundCardDriver1

SetDMA1:
	LodsB
	Cmp     AL, '0'
	JB      CmdLine3
	Cmp     AL, '7'
	JA      CmdLine3
	Sub     AL, '0'
	Call    Music_SetDMA
	Jmp     CmdLine1

SetMixSpeed1:
	Call    GetDecimalNumber
	JC      SetMixSpeedError
	Push    AX
	Call    Music_SetMixSpeed
	Pop     AX
	Jmp     CmdLine3

SetMixSpeedError:
	Push    CS
	Pop     DS
		Assume DS:StartUp

	Mov     AH, 9
	Mov     DX, Offset MixErrorMsg
	Int     21h

	Mov     [Pause], 1
	Jmp     CmdLine1
		Assume DS:Nothing

SetIRQ1:
	Call    GetDecimalNumber
	JC      IRQError
	Cmp     CX, 15
	JA      IRQError

	Push    AX
	Call    Music_SetIRQ
	Pop     AX
	Jmp     CmdLine3

IRQError:
	Push    CS
	Pop     DS
		Assume DS:StartUp

	Mov     AH, 9
	Mov     DX, Offset IRQErrorMsg
	Int     21h

	Mov     [Pause], 1
	Jmp     CmdLine1
		Assume DS:Nothing


SetAddress1:
	LodsB

	Xor     DX, DX
	Mov     CL, 4

	Cmp     AL, '0'
	JB      SetAddress2
	Cmp     AL, '9'
	JA      SetAddress2
	Sub     AL, '0'
	Jmp     SetAddress3

SetAddress2:
	Call    CapitaliseAL

	Cmp     AL, 'A'
	JB      CmdLine2
	Cmp     AL, 'F'
	JA      CmdLine2
	Sub     AL, '@'

SetAddress3:
	Test    DX, 0F000h
	JNZ     AddressError

	ShL     DX, CL
	Or      DL, AL

	LodsB
	Cmp     AL, '0'
	JB      SetAddress4
	Cmp     AL, '9'
	JA      SetAddress4
	Sub     AL, '0'
	Jmp     SetAddress3

SetAddress4:
	Call    CapitaliseAL
	Cmp     AL, 'A'
	JB      SetAddress5
	Cmp     AL, 'F'
	JA      SetAddress5

	Sub     AL, '@'
	Jmp     SetAddress3

SetAddress5:
	Call    Music_SetAddress
	Jmp     CmdLine3

AddressError:
	Push    CS
	Pop     DS
		Assume DS:StartUp

	Mov     AH, 9
	Mov     DX, Offset AddressErrorMsg
	Int     21h

	Mov     [Pause], 1
	Jmp     CmdLine1
		Assume DS:Nothing

Limit1:
	Call    GetDecimalNumber
	JC      LimitError
	Cmp     CX, 256
	JA      LimitError
	Cmp     CX, 4
	JB      LimitError

	Push    AX
	Call    Music_SetLimit
	Pop     AX
	Jmp     CmdLine3

LimitError:
	Push    CS
	Pop     DS
		Assume DS:StartUp

	Mov     AH, 9
	Mov     DX, Offset LimitErrorMsg
	Int     21h

	Mov     [Pause], 1
	Jmp     CmdLine1
		Assume DS:Nothing
*/
	}


CmdLineEnd:
	printf("Command Line Parsed\n");

	printf("Windows Detection\n");

	// if(windows_exists() && windows_version() < 4) { printf(WindowsMsg); press_a_key(); }

Start1:
	// Get Comspec
	if(Pause == 1)
	{
		/*
		Mov     DX, Offset ContinueMsg
		Mov     AH, 9
		Int     21h

		Xor     AX, AX
		Int     16h
		*/
	}

NoPause:
	printf("Retrieving Environment\n");

	/*
	Mov     SI, Offset COMSPECString
	Mov     CX, 7
	Call    GetEnvironment
	JC      Start2

	Mov     [COMSPECFound], 1
	Mov     [Word Ptr COMSPEC], DI
	Mov     [Word Ptr COMSPEC+2], ES

Start2:
	*/
	//Mov     FS, [CS:PSP]

	printf("Initialising Screen Module\n");

	//S_InitScreen();

	printf("Initialising Disk Module\n");

	//D_InitDisk();

	printf("Initialising Expanded Memory Module\n");

	//E_InitEMS();

	printf("Initialising Music Module\n");

	Music_InitMusic(ite);

	printf("Initialising Keyboard Module\n");

	//K_InitKeyBoard();

	printf("Initialising Edit Module\n");

	//PE_InitPatternEdit();

	printf("Initialising Custom Keyboard Layout\n");

	//K_InstallKeyboardType();

	printf("Initialising Error Handler Module\n");

	//Error_InitHandler();

	if(LoadMMTSR != 0)
	{
		printf("Initialising MMTSR Module\n");

		//MMTSR_InstallMMTSR();
	}

	printf("Initialising Mouse Module\n");

	//InitMouse();

	//S_ClearScreen();

	printf("Initialising Timer Module\n");

	//InitTimerHandler();

	printf("Initialising Soundcard\n");

	Music_AutoDetectSoundCard(ite);

	printf("Entering Main Message Loop\n");

	/*
	Mov     DI, Offset O1_AutoDetectList
	Mov     CX, 0FFFFh
	M_Object1List();
	Jmp     Quit1
	*/
	Quit1(ite);
}

int Quit(it_engine *ite)
{
	/*
	Mov     DI, Offset O1_ConfirmQuit
	Mov     CX, 3
	Call    M_Object1List

	And     DX, DX
	JNZ     Quit1

	Mov     AX, 1
	RetF
	*/
	Quit1(ite);
	return 0;
}

#if 0
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Proc            IsStartupKeyList Far

                Mov     AL,CS:StartupList

                Ret

EndP            IsStartupKeyList

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Proc            GetStartupKeyList Far

                Jmp     [CS:StartupKeyListFunction]

GetStartupKeyList1:
                Push    SI
                Mov     SI, [CS:StartupQueueOffset]

                Mov     CX, [CS:SI]
                Mov     DX, [CS:SI+2]

                Add     SI, 4
                Mov     [CS:StartupQueueOffset], SI

                Cmp     SI, [CS:StartupQueueEnd]
                
                Pop     SI
                JB      GetStartupKeyList1End

                Mov     AX, [CS:StartupQueueNextFunction]
                Mov     [CS:StartupKeyListFunction], AX

GetStartupKeyList1End:
                Ret

GetStartupKeyList2:
                Push    DS
                Push    SI

                LDS     SI, [DWord Ptr CS:StartupFileOffset]
                LodsB

                Mov     [CS:StartupFileOffset], SI

                Pop     SI
                Pop     DS

                Cmp     AL, 32
                JE      GetStartupKeyList2EndOfString
                And     AX, 0FFh
                JZ      GetStartupKeyList2EndOfString

                Mov     DX, AX
                Mov     CX, 1FFh

                Ret

GetStartupKeyList2EndOfString:
                Mov     [CS:StartupQueueOffset], Offset SaveInformation
                Mov     [CS:StartupQueueEnd], Offset ENDSAVEINFORMATION
                Mov     [CS:StartupQueueNextfunction], Offset GetStartupKeyList3
                Mov     [CS:StartupKeyListFunction], Offset GetStartupKeyList1

                Mov     CX, 11Ch        ; Enter
                Mov     DX, 0

                Ret

GetStartupKeyList3:                     ; Save module then quit
                Xor     CX, CX
                Xor     DX, DX
                Mov     [CS:StartupList], 0
                Ret

EndP            GetStartupKeyList

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Proc            CrashRecovery Far               ; CtrlAltDel location.

                ClD
                StI

                Call    S_ClearScreen
                Call    S_InitScreen
                Call    D_InitDisk
                Call    InitMouse

                Mov     DI, Offset O1_CrashRecovery
                Mov     CX, 0FFFFh
                Call    M_Object1List

                Mov     DI, Offset O1_PatternEditList
                Mov     CX, 0FFFFh
                Jmp     M_Object1List

EndP            CrashRecovery

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Proc            GetEnvironment Far              ; DS:SI points to string.
                                                ; CX = length of string.
                                                ; Returns ES:DI

                Mov     ES, [CS:PSP]
                Mov     ES, [ES:2Ch]            ; DS:00 points to environ.
                Xor     DI, DI

GetEnvironment1:
                Push    DI
                Push    CX
                Push    SI

                RepE    CmpSB

                Pop     SI
                Pop     CX
                Pop     DI

                JE      GetEnvironment3         ; From RepE

                Xor     AL, AL

GetEnvironment2:
                ScasB
                JNE     GetEnvironment2

                Cmp     [Byte Ptr ES:DI], AL
                JNE     GetEnvironment1

                StC
                Ret

GetEnvironment3:
                Add     DI, CX
                Inc     DI              ; Skip past '='
                ClC
                Ret

EndP            GetEnvironment

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Proc            Refresh Far

                Call    D_GetPreShellDirectory
                Call    S_InitScreen
                Call    D_InitDisk
                Call    InitMouse
                Call    D_RestorePreShellDirectory

                Mov     AX, 1
                Ret

EndP            Refresh

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Proc            DOSShell Far

                PushAD
                Push    DS
                Push    ES

                PushF

                Call    D_GetPreShellDirectory

                Cmp     [CS:Control], 0
                JNE     DOSShell2

                Call    K_UnInitKeyBoard
                Jmp     DOSShell3

DOSShell2:
                Call    K_InstallDOSHandler
DOSShell3:

                Mov     AX, 3
                Int     10h                     ; Clr screen.

                Call    UnInitMouse

                Push    CS
                Push    CS
                Pop     DS
                Pop     ES
                        Assume DS:StartUp

                Mov     AH, 9
                Mov     DX, Offset ShellMsg
                Int     21h

                Mov     AX, 4B00h
                Mov     BX, Offset ExecData
                Mov     DX, Offset DefaultShell
                Cmp     [COMSPECFound], 0
                JE      DOSShell1

                LDS     DX, [COMSPEC]

DOSShell1:
                ClI
                Int     21h
                        Assume DS:Nothing

                PopF

                Call    S_InitScreen
                Call    D_InitDisk
                Call    InitMouse

                Cmp     [CS:Control], 0
                JNE     DOSShell4

                Call    K_InitKeyBoard
                Jmp     DOSShell5

DOSShell4:
                Call    K_UnInstallDOSHandler

DOSShell5:
                Call    D_RestorePreShellDirectory

                Pop     ES
                Pop     DS
                PopAD

                Mov     AX, 1
                Ret

EndP            DOSShell
                Assume DS:Nothing

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ


EndS

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³ Stack Segment                                                               ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

Segment         StackSeg PARA Stack 'Stack'
StackData       DB      StackSize Dup(?)
EndS

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ

End             Start
#endif

int main(int argc, char *argv[])
{
	it_engine *ite = ITEngineNew();
	Start(ite, "");
	return 0;
}

