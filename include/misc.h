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


#ifndef _MISC_H_
#define _MISC_H_

#include <cstring>
#include <3ds.h>
#include "fs.h"



template<class T>
class Buffer
{
	const u32 elements;
	T *ptr;

public:
	// Clears mem by default to avoid problems
	Buffer(u32 elementCnt, bool clearMem=true) : elements(elementCnt) {ptr = new T[elementCnt]; if(clearMem) clear();}
	~Buffer() {delete[] ptr;}

	void clear() {memset(ptr, 0, size());}
	u32 size() {return elements*sizeof(T);}

	T* operator &() {return ptr;}
	T& operator [](u32 element) {return ptr[element];}
};

bool fileNameCmp(fs::DirEntry& first, fs::DirEntry& second);

#endif // _MISC_H_
