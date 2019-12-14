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

#define TRACEENABLED          0

#define TUTORIAL         0

#define  EMSUSE41         0

#define  SHOWVERSION          0
#define  SHOWREGISTERNAME     1

#define  USE32BITSCREENCOPY   0

#define  SORTENABLED          1
#define  DDCOMPRESS           1
#define  ORDERSORT            1
#define  FILTERENVELOPES      1
#define  CHORDENTRY           1
#define  SPECTRUMANALYSER     1
#define  SAVESAMPLEWAV        1
#define  ENABLEPRESETENVELOPES  1
#define  ENABLESOLO           1

#define    DEFAULTFORMAT      3 /* 0  IT214, 1  S3M, 2  IT2xx, 3  IT215 */

// USEFPUCODE disabled for the time being --GM
#define    USEFPUCODE         0 /* For IT_MUSIC, this will change from LUT to FPU code */

#define    OLDDRIVER          0

#define    MUSICDEBUG         0
#define    EMSDEBUG           0
#define    MEMORYDEBUG        0
#define ENABLEINT3         0 /* For debugging. */

#define    TIMERSCREEN        1

//define    NETWORKENABLED     1
#define    NETWORKENABLED     0
#define    SHOWPATTERNLENGTH  0

#if TUTORIAL
#define SORTENABLED      1
#define DDCOMPRESS       1
#endif

#define TRACKERVERSION  0x217 /* Still have to change text in IT.ASM, IT_F.ASM */

