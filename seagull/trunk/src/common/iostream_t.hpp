/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * (c)Copyright 2006 Hewlett-Packard Development Company, LP.
 *
 */

#ifndef _IOSTREAM_T_HPP
#define _IOSTREAM_T_HPP

#if defined(__hpux)

#include <iostream.h>
#include <iomanip.h>

#define iostream_error cerr
#define iostream_flush flush
#define iostream_endl  endl
#define iostream_output ostream

#define iostream_hex hex
#define iostream_dec dec
#define iostream_setw setw
#define iostream_setfill setfill

#else

#include <iostream>
#include <iomanip>

#define iostream_error std::cerr
#define iostream_flush std::flush
#define iostream_endl  std::endl
#define iostream_output std::ostream

#define iostream_hex std::hex
#define iostream_dec std::dec
#define iostream_setw std::setw
#define iostream_setfill std::setfill

#endif

#endif
