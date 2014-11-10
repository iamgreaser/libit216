#include <unistd.h>
#include "it_struc.h"

int main(int argc, char *argv[])
{
	const char *fname = argv[1];

	it_engine *ite = ITEngineNew();

	Music_InitMusic(ite);
	const char *dname = Music_AutoDetectSoundCard(ite);
	printf("Driver detected: \"%s\"\n", dname);

	D_LoadIT(ite, argv[1]);
	Music_PlaySong(ite, 0);
	//Music_PlayPattern(ite, 0, 64, 0);

	for(;;)
	{
		Music_Poll(ite);
	}

	return 0;
}

