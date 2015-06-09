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


#ifndef _FS_H_
#define _FS_H_

#include <exception>
#include <string>
#include <vector>
#include <cstdio>
#include <3ds.h>

#define FS_PATH_MAX_LENGTH         (0x106)
#define MAX_BUF_SIZE               (0x200000) // 2 MB
#define FS_ERR_DOESNT_EXIST        ((Result)0xC8804478)
#define FS_ERR_DOES_ALREADY_EXIST  ((Result)0xC82044BE) // Sometimes the API returns 0xC82044B9 instead


extern FS_archive sdmcArchive;

class fsException : public std::exception
{
	char errStr[256];
	Result res;


public:
	fsException(const char *file, const int line, const Result res, const char *desc)
	{
    this->res = res;
		snprintf(errStr, 255, "fsException:\n%s:%d: Result: 0x%X\n%s", file, line, (unsigned int)res, desc);
	}

	virtual const char* what() {return errStr;}
	const Result getErrCode() {return res;}
};

typedef enum
{
	FS_SEEK_SET = 0,
	FS_SEEK_CUR,
	FS_SEEK_END,
} fsSeekMode;



namespace fs
{
	class File
	{
		u64 _offset_;
		std::u16string _path_;
		u32 _openFlags_;
		FS_archive *_archive_;
		Handle _fileHandle_ = 0;


	public:
		File(const std::u16string& path, u32 openFlags, FS_archive& archive=sdmcArchive) {open(path, openFlags, archive);}
		File() {}
		~File() {close();}

		void open(const std::u16string& path, u32 openFlags, FS_archive& archive=sdmcArchive);
		u32  read(void *buf, u32 size);
		u32  write(const void *buf, u32 size);
		void close() {if(_fileHandle_) FSFILE_Close(_fileHandle_); _fileHandle_ = 0;}
		void seek(const u64 offset, fsSeekMode mode);
		u64  tell() {return _offset_;}
		u64  size();
		void setSize(const u64 size);
		void move(const std::u16string& dst, FS_archive& dstArchive=sdmcArchive);
		void copy(const std::u16string& dst, FS_archive& dstArchive=sdmcArchive);
		void remove(const std::u16string& path, FS_archive& archive=sdmcArchive);
		void remove(); // Removes the currently opened file

		// Don't use setFileHandle() for normal files! Only for AM file handles or similar.
		Handle getFileHandle() {return _fileHandle_;}
		void   setFileHandle(Handle fileHandle) {_fileHandle_ = fileHandle; _offset_ = 0;}
	};


	struct DirEntry
	{
		std::u16string name;
		bool isDir;
		u64 size;

		DirEntry(std::u16string name, bool isDir, u64 size) : name(name), isDir(isDir), size(size) {}
	};


	// Directory functions
	void makeDir(const std::u16string& path, FS_archive& archive=sdmcArchive);
	void makePath(const std::u16string& path, FS_archive& archive=sdmcArchive);
	std::vector<DirEntry> listDirContents(const std::u16string& path, FS_archive& archive=sdmcArchive);
	void copyDir(const std::u16string& src, const std::u16string& dst, FS_archive& srcArchive=sdmcArchive, FS_archive& dstArchive=sdmcArchive);
	void removeDir(const std::u16string& path, FS_archive& archive=sdmcArchive);


	// snipped zip functions


	// Misc functions
	void addToPath(std::u16string& path, const std::u16string& dirOrFile);
	void removeFromPath(std::u16string& path);
} // namespace fs


void sdmcArchiveInit();
void sdmcArchiveExit();

#endif // _FS_H_
