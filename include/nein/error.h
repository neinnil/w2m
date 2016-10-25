/****
    nein_error.h: 
    Copyright (C) 2016  Park, Sangjun

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/
#ifndef _NEIN_ERROR_H
#define _NEIN_ERROR_H

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SET_NEINERROR(n,e)	((n)<<8 | (e))
#define GET_NEINERROR(x)	(((x)>>8)&0xFF)
#define GET_ERROR(x)		((x)&0xFF)

#define NEIN_NOT_SUPPORTED	0x04
#define NEIN_IS_SYMLINK		0x08

extern void error_print(arg...);

#ifdef __cplusplus
}
#endif

#endif /* _NEIN_ERROR_H */
