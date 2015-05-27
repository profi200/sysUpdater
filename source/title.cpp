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
#include <3ds.h>
#include "error.h"
#include "fs.h"
#include "misc.h"
#include "title.h"

#define _FILE_ "title.cpp" // Replacement for __FILE__ without the path



std::vector<TitleInfo> getTitleInfos(mediatypes_enum mediaType)
{
	char tmpStr[16];
	u32 count;
	Result res;
	TitleInfo tmpTitleInfo;



	if((res = AM_GetTitleCount(mediaType, &count))) throw error(_FILE_, __LINE__, res);


	std::vector<TitleInfo> titleInfos; titleInfos.reserve(count);
	Buffer<u64> titleIdList(count);
	Buffer<TitleList> titleList(count);



	if((res = AM_GetTitleIdList(mediaType, count, &titleIdList))) throw error(_FILE_, __LINE__, res);
	if((res = AM_ListTitles(mediaType, count, &titleIdList, &titleList))) throw error(_FILE_, __LINE__, res);
	for(u32 i=0; i<count; i++)
	{
		// Copy title ID, size and version directly
		memcpy(&tmpTitleInfo.titleID, &titleList[i].titleID, 18);
		AM_GetTitleProductCode(mediaType, titleIdList[i], tmpStr);
		tmpTitleInfo.productCode = tmpStr;

		titleInfos.push_back(tmpTitleInfo);
	}

	return titleInfos;
}


void installCia(mediatypes_enum mediaType, const std::u16string& path)
{
	fs::File ciaFile(path, 1), cia;
	Buffer<u8> buffer(MAX_BUF_SIZE);
	Handle ciaHandle;
	u32 blockSize;
	u64 ciaSize, offset = 0;
	Result res;



	ciaSize = ciaFile.size();
	if((res = AM_StartCiaInstall(mediaType, &ciaHandle))) throw error(_FILE_, __LINE__, res);
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
			} catch(error& e)
			{
				AM_CancelCIAInstall(&ciaHandle); // Abort installation
				cia.setFileHandle(0); // Reset the handle so it doesn't get closed twice
				printf("%s\n", e.what());
				throw error(_FILE_, __LINE__, res);
			}

			offset += blockSize;
		}
	}


	if((res = AM_FinishCiaInstall(mediaType, &ciaHandle))) throw error(_FILE_, __LINE__, res);
}


void deleteTitle(mediatypes_enum mediaType, u64 titleID)
{
	Result res;

	// System app
	if(titleID>>32 & 0xFFFF) {if((res = AM_DeleteTitle(mediaType, titleID))) throw error(_FILE_, __LINE__, res);} // Who likes ambiguous else?
	// Normal app
	else if((res = AM_DeleteAppTitle(mediaType, titleID))) throw error(_FILE_, __LINE__, res);
}
