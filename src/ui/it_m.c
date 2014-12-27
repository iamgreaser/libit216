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

void M_FunctionHandler(it_engine *ite, void *ObjectList)
{
	//ARG     ObjectList:DWord
	//Push    BP
	//Mov     BP, SP

#if 0
	MouseSaveEvents();

M_FunctionHandler1:
	// Draw all objects in list
	al = GetKeyboardLock();
	Cmp     AL, 2
	JE      MouseInput1

	Call    MouseClearEvents

	LDS     SI, ObjectList // DS:SI points to object list
	Add     SI, 6 // Skip pass list header
	Xor     AX, AX

M_FunctionHandler2:
	Cmp     Word Ptr [SI], 0   // DS:SI points to an offset of
	JE      M_FunctionHandler4 //  an object

M_FunctionHandler3:
	Push    AX                      ; AX = object number
	Push    DS
	Push    SI
	Push    BP

	LES     DI, ObjectList          ; ES:DI points to object list
	Mov     SI, [SI]                ; DS:SI points to object
	Mov     BX, [SI]                ; BX = object type
	Mov     CL, 2
	ShL     BX, CL

	Call    DWord Ptr [DrawObjectList+BX]

	Pop     BP
	Pop     SI                      ; DS:SI Points to an offset of
	Pop     DS                      ;  an object
	Pop     AX
	Inc     AX
	Add     SI, 2                   ; Advance Pointer
	Jmp     M_FunctionHandler2      ; Next Object

M_FunctionHandler4:                             ; Call Prefunction for object
	LDS     SI, ObjectList          ; DS:SI points to object list
	Mov     AX, [SI]                ; AX = Active object number
	LEA     SI, [ESI+EAX*2+6]       ; Skip pas list header (+6)
					; AX = object number

	Cmp     Word Ptr [SI], 0        ; if list doesn't point to
	JE      M_FunctionHandler21     ;  anything, don't call prefunc

M_FunctionHandler20:
	Mov     SI, [SI]                ; DS:SI points to object
	Mov     BX, [SI]                ; BX = object type
	ShL     BX, 2

	Push    BP
	Call    DWord Ptr [PreFunctionList+BX]
	Pop     BP

M_FunctionHandler21:

#if TUTORIAL
	Glbl_TutorialHandler(ite);
#endif

	Call    S_UpdateScreen

M_FunctionHandler5:                             ; Input...
;                Call    K_IsAnyKeyDown
;                And     AL, AL
;                JNZ     M_KeyBoardInput1

MouseInput1:
	Call    MouseInput              ; returns 0 if nothing
					; 1 if input (BX, CX, DX set)
					; 2 if no input, but keyboard
					;   locked
	Cmp     AX, 1
	JB      M_KeyBoardInput1
	JA      M_FunctionHandler6      ; IdleList
					; Handle postobject, of
					; object BX

	Mov     AX, DI
	Cmp     BX, 0FFFFh
	JE      M_FunctionHandler23

	LES     DI, ObjectList          ; ES:DI points to object list
	Push    ES
	Pop     DS

	Mov     [DI], BX                ; Object number in BX.
	LEA     BX, [EBX*2+EDI+6]       ; ES:BX points to offset of
					; active object.

	Mov     SI, [BX]                ; DS:SI points to object

	Mov     SI, [SI]                ; SI = object type of active obj
	ShL     SI, 2

	Push    BP
	Call    DWord Ptr [PostFunctionList+SI]
	Pop     BP
	Jmp     M_FunctionHandler23

M_KeyBoardInput1:
	Call    MIDIBufferEmpty
	JAE     M_FunctionHandler8

	Call    K_IsKeyWaiting

	Test    AX, AX
	JNZ     M_FunctionHandler8
					; Check for mouse info...

M_FunctionHandler6:                             ; IdleList
	Cmp     CS:ReleaseTimeSlice, 0
	JE      NoReleaseTimeSlice

	Mov     AX, 1680h
	Int     2Fh

NoReleaseTimeSlice:
;                StI

	LDS     SI, ObjectList          ; DS:SI points to object list
	Add     SI, 2                   ; DS:SI points to idle list

	Cmp     Word Ptr [SI], 0
	JE      M_FunctionHandler5      ; If Offset of IdleList = 0,
					;  check for another key

M_FunctionHandler7:
	Mov     SI, [SI]
	Xor     BX, BX                  ; Clear flag.

M_FunctionHandler19:
	Cmp     Word Ptr [SI], 0
	JNE     M_FunctionHandler18

	Cmp     Word Ptr [SI+2], 0
	JE      M_FunctionHandler29

M_FunctionHandler18:
	Push    SI
	Push    BX
	Push    DS
	Push    BP

	Call    DWord Ptr [SI]

	Pop     BP
	Pop     DS
	Pop     BX

	Cmp     AX, 5
	JE      M_FunctionHandlerIdleCommand

	Pop     SI

	Add     SI, 4

	Or      BX, AX
	Jmp     M_FunctionHandler19

M_FunctionHandlerIdleCommand:
	Pop     AX              ; Pull off SI from the stack
	Mov     AX, DI
	Jmp     M_FunctionHandler10
;                Mov     SI, 1
;                Jmp     M_FunctionHandler16

M_FunctionHandler29:
	Test    BX, BX
	JZ      M_FunctionHandler5

	Jmp     M_FunctionHandler1

M_FunctionHandler8:
	Call    K_GetKey                ; CX = Keyboard Input Data
					; DX = Translated Input Data
					; else MIDI input if CL = 0

M_FunctionHandler9:
	LES     DI, ObjectList          ; ES:DI points to object list
	Push    ES
	Pop     DS

	Mov     BX, [DI]                ; BX = active object number
	LEA     BX, [EDI+EBX*2+6]       ; ES:BX points to offset of
					;  active object.
	Xor     AX, AX

	Cmp     Word Ptr [BX], 0
	JE      M_FunctionHandler23

M_FunctionHandler22:
	Mov     SI, [BX]                ; DS:SI points to object

	Mov     SI, [SI]                ; SI = object type of active obj
	ShL     SI, 2

	Push    BP
	Call    DWord Ptr [PostFunctionList+SI]
	Pop     BP

M_FunctionHandler23:
	Push    DS
	Push    SI

	LDS     SI, ObjectList
	Add     SI, 4
	Mov     SI, [SI]
	Mov     Word Ptr CS:GlobalKeyList+2, DS
	Mov     Word Ptr CS:GlobalKeyList, SI

	Pop     SI
	Pop     DS

;                Xor     BX, BX                  ; Extra key list starts at 0

M_FunctionHandler10:
	Cmp     AX, 1
	JB      M_FunctionHandler11     ; If AX = 0
	JE      M_FunctionHandler1      ; Redraw screen if AX = 1

	Cmp     AX, 3
	JB      M_FunctionHandler4      ; Goto preobject if AX = 2
	JE      M_FunctionHandler5      ; Get next input

	Cmp     AX, 5                   ; New list
	JE      M_FunctionHandler16
	JA      M_FunctionHandler11     ; If > 5

	Call    MouseRestoreEvents

	Pop     BP
	Ret     4

M_FunctionHandler16:
	Mov     Word Ptr Offset ObjectList, DX
	Mov     Word Ptr Offset ObjectList+2, CX
	Mov     AX, SI

	Jmp     M_FunctionHandler10

M_FunctionHandler11:
	LDS     SI, CS:GlobalKeyList

	Cmp     Word Ptr [SI], 0
	JE      M_FunctionHandler4

M_FunctionHandler12:
	LodsB
	Mov     BX, DX
	Cmp     AL, 1
	JE      M_FunctionHandler13     ; Keycode compare (1)
	Mov     BX, CX
	JB      M_FunctionHandler13     ; Scancode compare (0)

	And     BX, 1FFh

	Cmp     AL, 3
	JB      M_FunctionHandlerAlt    ; Alt-keycode compare (2)
	JE      M_FunctionHandlerCtrl   ; Ctrl-keycode compare  (3)

	Cmp     AL, 5
	JB      M_FunctionHandlerForced ; Always call function (4)
	JE      M_FunctionHandlerNewListNear ; Chain to new list (5)

	Cmp     AL, 7
	JB      M_FunctionHandlerShift  ; Shift-keycode compare (6)
	JE      M_FunctionHandlerNewListFar  ; Chain to far list (7)

	Cmp     AL, 9
	JB      M_FunctionHandlerCapCheck ; Capitalised keycode (8)
	JE      M_FunctionHandlerMIDI   ; MIDI message (9)

;                Jmp     M_FunctionHandler1
	Jmp     M_FunctionHandler4      ; Undefined compare..
;                                                ; -> end of list

M_FunctionHandlerAlt:
	Test    CH, 60h
	JZ      M_FunctionHandler14
	Jmp     M_FunctionHandler13

M_FunctionHandlerCtrl:
	Test    CH, 18h
	JZ      M_FunctionHandler14
	Jmp     M_FunctionHandler13

M_FunctionHandlerShift:
	Test    CH, 6h
	JZ      M_FunctionHandler14
	Jmp     M_FunctionHandler13

M_FunctionHandlerNewListNear:
	LodsW
	Mov     Word Ptr CS:GlobalKeyList, AX
	Jmp     M_FunctionHandler11

M_FunctionHandlerNewListFar:
	LodsD
	Mov     CS:GlobalKeyList, EAX
	Jmp     M_FunctionHandler11

M_FunctionHandlerForced:
	LodsW
	Jmp     M_FunctionHandler26

M_FunctionHandlerCapCheck:                      ; Capital OK? (8)
	Mov     BX, DX
	Cmp     BX, 'A'
	JB      M_FunctionHandler14
	Cmp     BX, 'z'
	JA      M_FunctionHandler14
	Cmp     BX, 'a'
	JB      M_FunctionHandlerCapCheck1

	Sub     BL, 32

M_FunctionHandlerCapCheck1:
	Jmp     M_FunctionHandler13

M_FunctionHandlerMIDI:
	Test    CL, CL
	JNZ     M_FunctionHandler14
	Mov     BX, CX
	And     BX, 0F000h
	Jmp     M_FunctionHandlerCheck

M_FunctionHandler13:
	Test    CL, CL
	JZ      M_FunctionHandler14

M_FunctionHandlerCheck:
	LodsW
	Cmp     BX, AX
	JNE     M_FunctionHandler14

M_FunctionHandler26:
	LES     DI, ObjectList          ; ES:DI points to object list
	Push    BP
	Call    DWord Ptr [SI]
	Pop     BP
	Jmp     M_FunctionHandler15

M_FunctionHandler14:
	Xor     AX, AX

M_FunctionHandler15:
	Add     Word Ptr CS:GlobalKeyList, 7
	Jmp     M_FunctionHandler10
#endif
}

void M_Object1List(it_engine *ite, void *di, int cx)
{
	if(cx != -1)
		*(int *)di = cx;

	M_FunctionHandler(ite, di);
}

void M_Object1ListDefault(it_engine *ite, void *di)
{
	return M_Object1List(ite, di, -1);
}

