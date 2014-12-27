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

/*
ESCContinueList         DB      0               ; ESC
                        DW      101h
                        DD      Glbl_F2

                        DB      0FFh
*/
it_object AboutBox = {
	.box = {
		0, // Object type 0
		11, 16, 68, 34, // Coordinates
		0, // Box style
	}
};

it_object AutoMiniBox = {
	.box = {
		0,
		25, 25, 55, 30,
		0,
	}
};

const char AboutTextData[] = {
	0xFF, 1, 0, 4, 8, 0xFF, 8, 55, 37, 41, 0xFF, 5, 55, 56, 58, 62, 66, 0xFF, 6, 55, 88, 92, 13,
	1, 5, 9, 12, 15, 18, 22, 25, 28, 31, 34, 38, 42, 45, 48, 51, 55, 55, 57, 59, 63, 67, 70, 73, 76, 79, 82, 85, 89, 93, 96, 99, 102, 105, 13,
	2, 6, 0xFF, 1, 10, 0xFF, 1, 13, 16, 19, 23, 26, 29, 32, 35, 39, 43, 46, 49, 52, 54, 55, 55, 60, 64, 68, 71, 74, 77, 80, 83, 86, 90, 94, 97, 100, 103, 106, 13,
	3, 7, 11, 14, 17, 20, 24, 27, 30, 33, 36, 40, 44, 47, 50, 53, 55, 55, 55, 61, 65, 69, 72, 75, 78, 81, 84, 87, 91, 95, 98, 101, 104, 107, 13,
	0xFF, 5, 55, 21, 0
};

it_object AboutText = {
	.text = {
		1, // Object type 1
		24, 19,
		0x2B,
		AboutTextData,
	}
};

const uint8_t ImpulseLogoChars[] = {
	0, 0, 0, 0, 1, 7, 15, 31,                // 0
	63, 127, 127, 254, 252, 255, 255, 126,   // 1
	24, 0, 0, 0, 0, 0, 0, 0,                 // 2
	0, 1, 3, 3, 3, 1, 0, 0,                  // 3
	7, 31, 63, 255, 255, 254, 248, 240,      // 4
	192, 128, 0, 1, 1, 131, 3, 7,            // 5
	7, 15, 14, 30, 60, 60, 120, 240,         // 6
	240, 224, 224, 224, 224, 192, 0, 0,      // 7
	240, 240, 248, 248, 248, 120, 56, 120,   // 8
	112, 240, 240, 224, 224, 192, 192, 131,  // 9
	7, 15, 31, 31, 62, 60, 61, 63,           // 10
	63, 63, 62, 28, 8, 0, 0, 0,              // 11
	0, 0, 0, 0, 0, 0, 193, 227,              // 12
	199, 223, 191, 127, 247, 239, 207, 159,  // 13
	31, 31, 63, 60, 24, 0, 0, 0,             // 14
	0, 0, 0, 0, 0, 240, 240, 240,            // 15
	241, 227, 239, 223, 191, 251, 247, 231,  // 16
	159, 159, 31, 31, 14, 0, 0, 0,           // 17
	0, 0, 0, 0, 0, 48, 112, 248,             // 18
	248, 240, 224, 192, 193, 131, 7, 30,     // 19
	252, 240, 225, 131, 3, 7, 7, 15,         // 20
	15, 30, 30, 28, 8, 0, 0, 0,              // 21
	0, 0, 0, 1, 3, 3, 7, 15,                 // 22
	31, 63, 127, 255, 191, 190, 124, 126,    // 23
	255, 255, 255, 255, 199, 128, 128, 0,    // 24
	60, 124, 248, 240, 224, 192, 135, 159,   // 25
	127, 255, 223, 143, 30, 60, 56, 113,     // 26
	255, 255, 252, 240, 192, 0, 0, 0,        // 27
	0, 0, 0, 0, 0, 0, 128, 129,              // 28
	131, 135, 15, 15, 30, 60, 124, 253,      // 29
	191, 31, 31, 14, 0, 0, 0, 0,             // 30
	0, 0, 0, 0, 0, 112, 248, 240,            // 31
	225, 195, 135, 15, 31, 63, 127, 255,     // 32
	239, 223, 159, 15, 14, 0, 0, 0,          // 33
	0, 0, 0, 0, 24, 56, 124, 248,            // 34
	248, 241, 225, 193, 129, 131, 15, 31,    // 35
	249, 241, 225, 192, 0, 0, 0, 0,          // 36
	0, 0, 0, 0, 0, 0, 1, 3,                  // 37
	7, 15, 30, 61, 57, 123, 119, 254,        // 38
	252, 248, 240, 224, 192, 192, 192, 225,  // 39
	247, 255, 254, 252, 0, 0, 0, 0,          // 40
	0, 0, 24, 56, 120, 248, 248, 240,        // 41
	112, 96, 224, 192, 192, 128, 0, 1,       // 42
	3, 7, 14, 28, 56, 112, 248, 252,         // 43
	255, 127, 63, 31, 7, 0, 0, 0,            // 44
	0, 0, 0, 48, 120, 120, 248, 248,         // 45
	252, 124, 124, 120, 120, 112, 241, 231,  // 46
	142, 252, 248, 224, 128, 0, 0, 0,        // 47
	0, 0, 0, 0, 0, 0, 3, 7,                  // 48
	15, 30, 60, 63, 127, 254, 252, 60,       // 49
	126, 63, 63, 31, 6, 0, 0, 0,             // 50
	0, 0, 0, 0, 60, 254, 254, 222,           // 51
	30, 60, 248, 224, 128, 1, 7, 14,         // 52
	124, 248, 224, 192, 0, 0, 0, 0,          // 53
	0, 0, 0, 0, 128, 128, 0, 0,              // 54
	0, 0, 0, 0, 0, 0, 0, 0,                  // 55
	0, 0, 7, 31, 63, 127, 127, 255,          // 56
	255, 252, 127, 0, 0, 0, 0, 0,            // 57
	3, 255, 255, 255, 255, 255, 255, 252,    // 58
	128, 0, 0, 0, 0, 0, 0, 0,                // 59
	0, 0, 0, 1, 1, 3, 3, 7,                  // 60
	7, 15, 31, 31, 31, 30, 0, 0,             // 61
	255, 255, 255, 255, 255, 255, 255, 0,    // 62
	0, 1, 3, 7, 7, 15, 31, 62,               // 63
	60, 124, 248, 248, 240, 240, 224, 224,   // 64
	192, 192, 128, 128, 0, 0, 0, 0,          // 65
	128, 224, 240, 240, 248, 248, 248, 112,  // 66
	240, 224, 192, 192, 128, 6, 15, 31,      // 67
	63, 127, 127, 112, 33, 97, 195, 131,     // 68
	7, 7, 7, 3, 1, 0, 0, 0,                  // 69
	0, 0, 0, 0, 0, 0, 252, 254,              // 70
	254, 252, 248, 240, 224, 193, 195, 199,  // 71
	223, 253, 240, 224, 128, 0, 0, 0,        // 72
	0, 0, 0, 0, 0, 0, 0, 3,                  // 73
	15, 31, 63, 124, 248, 240, 225, 195,     // 74
	199, 254, 252, 120, 48, 0, 0, 0,         // 75
	0, 0, 0, 0, 0, 31, 255, 255,             // 76
	255, 255, 191, 63, 126, 252, 248, 184,   // 77
	63, 127, 127, 126, 124, 48, 0, 0,        // 78
	0, 0, 0, 0, 0, 0, 128, 128,              // 79
	129, 3, 7, 15, 15, 31, 62, 126,          // 80
	239, 207, 143, 7, 3, 0, 0, 0,            // 81
	0, 0, 0, 0, 0, 15, 63, 255,              // 82
	255, 255, 238, 204, 0, 0, 0, 3,          // 83
	15, 255, 254, 248, 224, 0, 0, 0,         // 84
	0, 0, 0, 0, 0, 193, 195, 131,            // 85
	135, 15, 15, 31, 63, 127, 255, 255,      // 86
	190, 60, 28, 24, 0, 0, 0, 0,             // 87
	0, 0, 0, 0, 0, 1, 3, 7,                  // 88
	15, 31, 63, 126, 252, 248, 240, 227,     // 89
	199, 223, 188, 112, 225, 199, 254, 252,  // 90
	252, 126, 127, 63, 31, 0, 0, 0,          // 91
	0, 12, 60, 124, 248, 248, 240, 224,      // 92
	192, 128, 0, 0, 0, 48, 248, 248,         // 93
	248, 120, 241, 225, 195, 3, 15, 31,      // 94
	59, 243, 227, 129, 0, 0, 0, 0,           // 95
	0, 0, 0, 0, 1, 15, 31, 62,               // 96
	120, 241, 231, 239, 252, 240, 192, 192,  // 97
	227, 255, 255, 252, 112, 0, 0, 0,        // 98
	0, 0, 0, 0, 192, 240, 241, 243,          // 99
	231, 207, 143, 14, 12, 28, 56, 112,      // 100
	225, 193, 1, 0, 0, 0, 0, 0,              // 101
	0, 0, 0, 0, 0, 192, 255, 255,            // 102
	255, 255, 254, 60, 120, 112, 240, 241,   // 103
	247, 255, 252, 248, 96, 0, 0, 0,         // 104
	0, 0, 0, 0, 0, 0, 0, 128,                // 105
	128, 0, 0, 0, 0, 0, 0, 192,              // 106
	192, 0, 0, 0, 0, 0, 0, 0,                // 107
};

it_object ImpulseLogo = {
	.redefine_characters = {
		6, // Object type 6
		256, // First char to define - label: LogoCharacter
		108,
		ImpulseLogoChars,
	}
};

it_object AutoDetectText = {
	.text = {
		1,
		32, 26,
		0x20,
		"Sound Card Setup",
	}
};

it_object CallAutoDetect = {
	.call_far_function = {
		8,
		Music_ShowAutoDetectSoundCard,
	}
};

it_object AutoContinueButton = {
	.button = {
		2,                       // Object type 2
		-1, -1, -1, -1,
		0,                       // Button usage type
		0, 0,                    // ????
		4,                       // New List
		//0, 0, Glbl_F9, NULL,
		0, 0, NULL, NULL,
		32, 31, 47, 33,          // Left/Top/Right/Bottom
		8,                       // Box initial style
		0,                       // Button Up
		"   Continue",
	}
};

it_object NBMBox = {
	.box = {
		0,
		25, 25, 54, 32,
		3,
	}
};

it_object OKButton = {
	.button = {
		2,
		-1, -1, -1, -1,
		0,
		0, 0,
		0,
		0, // Returns 0
		0, NULL, NULL,
		35, 29, 44, 31,
		8,
		0,
		"   OK",
	}
};

it_object OOSoundCardMemoryText = {
	.text = {
		1,
		27, 27,
		0x20,
		"Insufficient Soundcard RAM",
	}
};

it_object FillHeader = {
	.call_far_function = {
		8,
		NULL,//PE_FillHeader,
	}
};

it_object SongLengthText = {
	.text = {
		1,
		27, 27,
		0x20,
		"Total song time: ",
	}
};

it_object ShowTime = {
	.call_far_function = {
		8,
		Music_ShowTime,
	}
};

it_objlist_6 O1_AutoDetectList = {
	6, 0, //ESCContinueList,
	{
		&AboutBox,
		&ImpulseLogo,
		&AutoMiniBox,
		&AboutText,
		&AutoDetectText,
		&CallAutoDetect,
		&AutoContinueButton,
		NULL,
	},
};

it_objlist_2 O1_OutOfSoundCardMemoryList = {
	2, 0, //ESCReturnList,
	{
		&NBMBox,
		&OOSoundCardMemoryText,
		&OKButton,
		&FillHeader,
		NULL,
	}
};

it_objlist_2 O1_ShowTime = {
	2, 0, //ESCReturnList,
	{
		&NBMBox,
		&ShowTime,
		&OKButton,
		&SongLengthText,
		NULL,
	}
};

