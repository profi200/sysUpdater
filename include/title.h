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


#ifndef _TITLE_H_
#define _TITLE_H_

#include <exception>
#include <string>
#include <vector>
#include <cstdio>
#include <3ds.h>

class titleException : public std::exception
{
	char errStr[256];
	Result res;


public:
	titleException(const char *file, const int line, const Result res, const char *desc)
	{
    this->res = res;
		snprintf(errStr, 255, "titleException:\n%s:%d: Result: 0x%X\n%s", file, line, (unsigned int)res, desc);
	}

	virtual const char* what() const noexcept {return errStr;}
	const Result getErrCode() {return res;}
};



struct TitleInfo
{
	u16 icon[0x900];
	std::u16string title;
	std::u16string publisher;
	std::string productCode;
	u64 titleID;
	u64 size;
	u16 version;
};


struct Icon
{
	u32 magic;
	u16 version;
	u16 reserved;
	struct
	{
		char16_t shortDesc[0x40];
		char16_t longDesc[0x80];
		char16_t publisher[0x40];
	} appTitles[0x10];
	struct
	{
		u8 ageRatings[0x10];
		u32 regionLock;
		struct
		{
			u32 matchMakerID;
			u64 matchMakerBitID;
		// gcc adds unnecessary padding here so we force it to not use padding.
		// Surprisingly this doesn't increase code size.
		} __attribute__((packed)) matchMakerIDs;
		u32 flags;
		u16 eulaVersion;
		u16 reserved;
		u32 animationDefaultFrame;
		u32 cecID;
	} appSettings;
	u64 reserved2;
	u16 icon24[0x240];
	u16 icon48[0x900];
};


std::vector<TitleInfo> getTitleInfos(FS_MediaType mediaType);
void installCia(const std::u16string& path, FS_MediaType mediaType, std::function<void (const std::u16string& file, u32 percent)> callback=nullptr);
void deleteTitle(FS_MediaType mediaType, u64 titleID);
//bool launchTitle(FS_MediaType mediaType, u8 flags, u64 titleID); // On applet launch it returns false if the applet can't be lauched
#define relaunchApp() launchTitle(mediatype_SDMC, 2, 0)

#endif // _TITLE_H_
