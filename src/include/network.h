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

#if NETWORKENABLED
/*
Extrn   Network_GetSendQueue:Far
Extrn   Network_FinishedSendQueue:Far
Extrn   Network_AddWordToQueue:Far
Extrn   Network_EnsureNoNetwork:Far
Extrn   Network_SendSampleHeader:Far
Extrn   Network_SendInstrumentHeader:Far
Extrn   Network_QueueSampleData:Far
Extrn   Network_SendSongDataInformation:Far
*/

#define NETWORK_PARTIALPATTERNOBJECT    0
#define NETWORK_ENTIREPATTERNOBJECT     1
#define NETWORK_REQUESTPATTERNOBJECT    2
#define NETWORK_SONGDATAOBJECT          3
#define NETWORK_INSTRUMENTHEADEROBJECT  4
#define NETWORK_SAMPLEHEADEROBJECT      5
#define NETWORK_SETPATTERNLENGTH        6
#define NETWORK_DELETESAMPLEOBJECT      7

#define EnsureNoNetwork(ite)           Network_EnsureNoNetwork(ite)
#define NetworkSendSample(ite)         Network_SendSampleHeader(ite)
#define NetworkSendInstrument(ite)     Network_SendInstrumentHeader(ite)

#else

#define EnsureNoNetwork(ite)
#define NetworkSendSample(ite)
#define NetworkSendInstrument(ite)

#endif

