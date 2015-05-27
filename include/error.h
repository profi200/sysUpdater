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


#ifndef _ERROR_H_
#define _ERROR_H_

#include <cstdio>
#include <exception>

#define ERR_NULL_PTR       (-1)
#define ERR_NOT_ENOUGH_MEM (-2)
#define ERR_PATH_TOO_LONG  (-3)



struct error: public std::exception
{
	char errStr[256];


	error(const char *file, const int line, const Result res) {snprintf(errStr, 255, "%s:%d: Error: 0x%X", file, line, (unsigned int)res);}

	virtual const char* what() {return errStr;}
};

#endif // _ERROR_H_
