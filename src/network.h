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

