/*
 *  sysUpdater is an update app for the Nintendo 3DS.
 *  Copyright (C) 2015 profi200
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/
 */


#include <cstdio>
#include <string>
#include <vector>
#include <3ds.h>
#include "error.h"
#include "fs.h"
#include "misc.h"
#include "title.h"

#define _FILE_ "main.cpp" // Replacement for __FILE__ without the path



// Override the default service init/exit functions
extern "C"
{
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
}


// Find title and compare versions. Returns CIA file version - installed title version
int versionCmp(std::vector<TitleInfo>& installedTitles, u64& titleID, u16 version)
{
	for(auto it : installedTitles)
	{
		if(it.titleID == titleID)
		{
			return (version - it.version);
		}
	}

	return 1; // The title is not installed
}


// If downgrade is true we don't care about versions (except equal versions) and uninstall newer versions
void installUpdates(bool downgrade)
{
	std::vector<fs::DirEntry> filesDirs = fs::listDirContents(u"/updates");
	std::vector<TitleInfo> installedTitles = getTitleInfos(mediatype_NAND);
	std::vector<std::u16string> remainingCias;
	Buffer<char> tmpStr(256);
	Result res;
	TitleList ciaFileInfo;
	fs::File f;



	printf("Getting CIA file informations...\n\n");

	for(auto it : filesDirs)
	{
		if(!it.isDir && (it.name.compare(it.name.length()-4, 4, u".cia") == 0))
		{
			f.open(u"/updates/" + it.name, FS_OPEN_READ);
			if((res = AM_GetCiaFileInfo(mediatype_NAND, &ciaFileInfo, f.getFileHandle()))) throw error(_FILE_, __LINE__, res);

			int cmpResult = versionCmp(installedTitles, ciaFileInfo.titleID, ciaFileInfo.titleVersion);
			if((downgrade && cmpResult != 0) || (cmpResult > 0))
			{
				if(cmpResult < 0) deleteTitle(mediatype_NAND, ciaFileInfo.titleID);
				if(ciaFileInfo.titleID == 0x0004013800000002LL || ciaFileInfo.titleID == 0x0004013820000002LL)
				{
					printf("NATIVE_FIRM         ");
					installCia(mediatype_NAND, u"/updates/" + it.name);
					if((res = AM_InstallNativeFirm())) throw error(_FILE_, __LINE__, res);
					printf("\x1b[32m  Installed\x1b[0m\n");
				}
				else remainingCias.push_back(it.name);
			}
		}
	}


	for(auto it : remainingCias)
	{
		tmpStr.clear();
		utf16_to_utf8((u8*)&tmpStr, (u16*)it.c_str(), 255);

		printf("%s", &tmpStr);
		installCia(mediatype_NAND, u"/updates/" + it);
		printf("\x1b[32m  Installed\x1b[0m\n");
	}
}


int main()
{
	bool once = false;



	consoleInit(GFX_TOP, NULL);
	printf("sysUpdater 0.4 by profi200\n\n\n");
	printf("(A) update\n(Y) downgrade\n(B) exit\n\n");
	printf("Use the HOME button if you run the CIA version.\n");
	printf("If you started the update you can't abort it!\n\n");
	printf("\x1b[31mIMPORTANT: Don't run this in sysNAND Gateway mode or it will brick!\x1b[0m\n\n\n");


	while(aptMainLoop())
	{
		hidScanInput();


		if(hidKeysDown() & KEY_B)
			break;
		if(!once)
		{
			if(hidKeysDown() & (KEY_A | KEY_Y))
			{
				try
				{
					installUpdates((bool)(hidKeysDown() & KEY_Y));

					printf("\n\nUpdates installed. Rebooting in 10 seconds...\n");
					svcSleepThread(10000000000LL);

					aptOpenSession();
					APT_HardwareResetAsync(NULL);
					aptCloseSession();
					once = true;
				}
				catch(error& e)
				{
					printf("%s\n", e.what());
					once = true;
				}
			}
		}


		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}


	return 0;
}
