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

