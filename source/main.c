#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <fs.h>
#include <title.h>



// Override the default service init/exit functions
void __appInit()
{
	// Initialize services
	srvInit();
	aptInit();
	gfxInit(GSP_RGB565_OES, GSP_RGB565_OES, false);
	hidInit(NULL);
	fsInit();
	sdmcArchiveInit();
	amInit();
}


void __appExit()
{
	// Exit services
	amExit();
	sdmcArchiveExit();
	fsExit();
	hidExit();
	gfxExit();
	aptExit();
	srvExit();
}


void printResult(Result res)
{
	switch(res)
	{
		case 0:
			printf("\x1b[32m   Installed\n");
			break;
		case 0xD8E08027:
			printf("\x1b[33m   Installed version is newer\n");
			break;
		case 0xC8E083FC:
			printf("\x1b[36m   Already installed\n");
			break;
		default:
			printf("\x1b[31m   0x%X\n", (unsigned int)res);
	}
	printf("\x1b[0m");
}


int main()
{
	DirEntry *entries = (DirEntry*)malloc(sizeof(DirEntry)*512);
	u32 i, folderCount, fileCount;
	char basePath[1024] = "/updates";
	Result res;
	bool done = false;



	res = getFilesFolders(basePath, &sdmcArchive, &folderCount, &fileCount, entries);
	consoleInit(GFX_TOP, NULL);
	printf("sysUpdater 0.3 by profi200\n\n\n");
	printf("Press (A) to update or (B) to exit.\n");
	printf("If you run the CIA version you can just close this app now. ");
	printf("If you started the update you can't abort it!\n\n");
	printf("\x1b[31mIMPORTANT: Don't run this in sysNAND Gateway mode or it will brick!\n\n");
	if(res) printf("Error: '/updates' doesn't exist!\n");
	if(!fileCount) printf("Nothing to install.\n");
	printf("\x1b[0m"); // Reset colors


	while(aptMainLoop())
	{
		hidScanInput();

		if(hidKeysDown() & KEY_B)
			break;

		if(!done && !res && fileCount && (hidKeysDown() & KEY_A))
		{
			for(i=0; i<fileCount; i++)
			{
				if(strncmp(&entries[folderCount+i].name[strlen(entries[folderCount+i].name)-4], ".cia", 4) == 0)
				{
					addToPath(basePath, entries[folderCount+i].name);
					printf("%s", entries[folderCount+i].name);
					res = installCia(0, basePath);
					printResult(res);
					removeFromPath(basePath);
				}
			}


			printf("\n  Installing FIRM...");
			res = AM_InstallNativeFirm();
			printResult(res);


			printf("\n\nUpdates installed. Rebooting in 10 seconds...\n");
			svcSleepThread(10000000000LL);


			aptOpenSession();
			APT_HardwareResetAsync(NULL);
			aptCloseSession();

			done = true;
		}


		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	free(entries);


	return 0;
}
