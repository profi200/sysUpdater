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


#include <string>
#include <vector>
#include <cstring>
#include <3ds.h>
#include "fs.h"
#include "misc.h"
#include "title.h"

#define _FILE_ "title.cpp" // Replacement for __FILE__ without the path



std::vector<TitleInfo> getTitleInfos(mediatypes_enum mediaType)
{
	char tmpStr[16];
	extern u8 sysLang; // We got this in main.c
	u32 count, bytesRead;
	Result res;
	Handle fileHandle;
	TitleInfo tmpTitleInfo;

	u32 archiveLowPath[4] = {0, 0, mediaType, 0};
	const FS_archive iconArchive = {0x2345678A, {PATH_BINARY, 0x10, (u8*)archiveLowPath}};
	const u32 fileLowPath[5] = {0, 0, 2, 0x6E6F6369, 0};
	const FS_path filePath = {PATH_BINARY, 0x14, (const u8*)fileLowPath};


	if((res = AM_GetTitleCount(mediaType, &count))) throw titleException(_FILE_, __LINE__, res, "Failed to get title count!");


	std::vector<TitleInfo> titleInfos; titleInfos.reserve(count);
	Buffer<u64> titleIdList(count, false);
	Buffer<TitleList> titleList(count, false);
	Buffer<Icon> icon(1, false);



	if((res = AM_GetTitleIdList(mediaType, count, &titleIdList))) throw titleException(_FILE_, __LINE__, res, "Failed to get title ID list!");
	if((res = AM_ListTitles(mediaType, count, &titleIdList, &titleList))) throw titleException(_FILE_, __LINE__, res, "Failed to get title list!");
	for(u32 i=0; i<count; i++)
	{
		// Copy title ID, size and version directly
		memcpy(&tmpTitleInfo.titleID, &titleList[i].titleID, 18);
		if(AM_GetTitleProductCode(mediaType, titleIdList[i], tmpStr)) memset(tmpStr, 0, 16);
		tmpTitleInfo.productCode = tmpStr;

		// Copy the title ID into our archive low path
		memcpy(archiveLowPath, &titleIdList[i], 8);
		icon.clear();
		if(!FSUSER_OpenFileDirectly(nullptr, &fileHandle, iconArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE))
		{
			// Nintendo decided to release a title with an icon entry but with size 0 so this will fail.
			// Ignoring errors because of this here.
			FSFILE_Read(fileHandle, &bytesRead, 0, &icon, sizeof(Icon));
			FSFILE_Close(fileHandle);
		}

		tmpTitleInfo.title = icon[0].appTitles[sysLang].longDesc;
		tmpTitleInfo.publisher = icon[0].appTitles[sysLang].publisher;
		memcpy(tmpTitleInfo.icon, icon[0].icon48, 0x1200);

		titleInfos.push_back(tmpTitleInfo);
	}

	return titleInfos;
}


void installCia(const std::u16string& path, mediatypes_enum mediaType, std::function<void (const std::u16string& file, u32 percent)> callback)
{
	fs::File ciaFile(path, FS_OPEN_READ), cia;
	Buffer<u8> buffer(MAX_BUF_SIZE, false);
	Handle ciaHandle;
	u32 blockSize;
	u64 ciaSize, offset = 0;
	Result res;



	ciaSize = ciaFile.size();
	if((res = AM_StartCiaInstall(mediaType, &ciaHandle))) throw titleException(_FILE_, __LINE__, res, "Failed to start CIA installation!");
	cia.setFileHandle(ciaHandle); // Use the handle returned by AM


	for(u32 i=0; i<=ciaSize / MAX_BUF_SIZE; i++)
	{
		blockSize = ((ciaSize - offset<MAX_BUF_SIZE) ? ciaSize - offset : MAX_BUF_SIZE);

		if(blockSize>0)
		{
			try
			{
				ciaFile.read(&buffer, blockSize);
				cia.write(&buffer, blockSize);
			} catch(fsException& e)
			{
				AM_CancelCIAInstall(&ciaHandle); // Abort installation
				cia.setFileHandle(0); // Reset the handle so it doesn't get closed twice
				throw;
			}

			offset += blockSize;
			if(callback) callback(path, offset * 100 / ciaSize);
		}
	}

	if((res = AM_FinishCiaInstall(mediaType, &ciaHandle))) throw titleException(_FILE_, __LINE__, res, "Failed to finish CIA installation!");
}


void deleteTitle(mediatypes_enum mediaType, u64 titleID)
{
	Result res;

	// System app
	if(titleID>>32 & 0xFFFF) {if((res = AM_DeleteTitle(mediaType, titleID))) throw titleException(_FILE_, __LINE__, res, "Failed to delete system title!");} // Who likes ambiguous else?
	// Normal app
	else if((res = AM_DeleteAppTitle(mediaType, titleID))) throw titleException(_FILE_, __LINE__, res, "Failed to delete app title!");
}


// TODO: Find a way to translate the title ID to an appID without lookup tables (this looks ugly :|)
//       Fix the weird freeze which sometimes happens at applet launch
bool launchTitle(mediatypes_enum mediaType, u8 flags, u64 titleID)
{
	Result res;
	u8 isN3DS;
	u32 appID;
	const u32 appIDTable[9] = {0x101, 0x103, 0x110, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117};
	const u32 tIDTableOld3DS[9][3] = {
		{0x8202, 0x8F02, 0x9802}, // Home menu
		{0x8102, 0x8102, 0x8102}, // Alternate Menu
		{0x8402, 0x9002, 0x9902}, // Camera Applet
		{0x8D02, 0x9602, 0x9F02}, // Friends list applet
		{0x8702, 0x9302, 0x9C02}, // Game notes applet
		{0x8802, 0x9402, 0x9D02}, // Internet browser
		{0x8602, 0x9202, 0x9B02}, // Instruction manual applet
		{0x8E02, 0x9702, 0xA002}, // Notifications applet
		{0xBC02, 0xBD02, 0xBE02}}; // Miiverse applet
	const u32 tIDTableNew3DS[9][3] = {
		{0x8202, 0x8F02, 0x9802}, // Home menu
		{0x8102, 0x8102, 0x8102}, // Alternate Menu
		{0x8402, 0x9002, 0x9902}, // Camera Applet
		{0x8D02, 0x9602, 0x9F02}, // Friends list applet
		{0x8702, 0x9302, 0x9C02}, // Game notes applet
		{0x20008802, 0x20009402, 0x20009D02}, // New 3DS internet browser
		{0x8602, 0x9202, 0x9B02}, // Instruction manual applet
		{0x8E02, 0x9702, 0xA002}, // Notifications applet
		{0xBC02, 0xBD02, 0xBE02}}; // Miiverse applet



	APT_CheckNew3DS(nullptr, &isN3DS);
	const u32 (*tIDTable)[3] = ((isN3DS) ? tIDTableNew3DS : tIDTableOld3DS);


	aptOpenSession();

	if(titleID>>32 == 0x40030) // Applet
	{
		// Search the the title ID lower word in our title ID table
		for(u32 i=0; i<9; i++)
		{
			for(u32 j=0; j<3; j++)
			{
				if(tIDTable[i][j] == ((u32)titleID))
				{
					appID = appIDTable[i];
					goto found;
				}
			}
		}
		return false; // Wrong title ID or applet can't be launched/is not in our list

		found:


		if((res = APT_PrepareToStartSystemApplet(nullptr, (NS_APPID)appID)))
		{
			aptCloseSession();
			throw titleException(_FILE_, __LINE__, res, "Failed to prepare for system applet start!");
		}
		if((res = APT_StartSystemApplet(nullptr, (NS_APPID)appID, 0, 0, nullptr)))
		{
			aptCloseSession();
			throw titleException(_FILE_, __LINE__, res, "Failed to start system applet!");
		}
		aptSetStatus(APP_EXITING);
	}
	else // App
	{
		if((res = APT_PrepareToDoAppJump(nullptr, flags, titleID, mediaType)))
		{
			aptCloseSession();
			throw titleException(_FILE_, __LINE__, res, "Failed to prepare for app start!");
		}
		if((res = APT_DoAppJump(nullptr, 0, 0, nullptr, nullptr)))
		{
			aptCloseSession();
			throw titleException(_FILE_, __LINE__, res, "Failed to start app!");
		}
	}

	aptCloseSession();
	return true; // Applet launch was successful
}
