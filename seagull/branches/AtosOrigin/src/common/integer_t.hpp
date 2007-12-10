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

#ifndef _INTEGER_T_HPP
#define _INTEGER_T_HPP

// For strtoul and strtol define
#include <stdlib.h>

// For byte order resolution
#include <sys/param.h>

// Special HP-UX resolution
#if defined (__hpux)
#if defined (__hppa)
#ifndef BYTE_ORDER
#define BYTE_ORDER BIG_ENDIAN
#endif
#elif defined(__ia64)
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif
#endif


// For max long size definition
#include <limits.h>

// For ntohl and htonl definition
#if defined(__hpux)
#include <arpa/inet.h>
#else
#include <netinet/in.h>
#endif

// Define the Integer 64 bits and Unsigned Integer 64 bits types 
// depending on platform capability
#if LONG_LONG_MAX<9223372036854775807LL
#error "long long type definition is not 64 bits"
#else

// long long is coded on 8 bytes use it for Integer64 types
typedef unsigned long long T_UnsignedInteger64, *T_pUnsignedInteger64;
typedef long long          T_Integer64, *T_pInteger64;

#define strtoull_f strtoull
#define strtoll_f  strtoll

#define ntohll_f ntohll
#define htonll_f htonll

#endif // LONG_LONG_MAX

// Define the Integer 32bits and Unsigned Integer 32bits  types
typedef unsigned long T_UnsignedInteger32, *T_pUnsignedInteger32;
typedef long          T_Integer32, *T_pInteger32;

// Check the size of long type
#if LONG_MAX==9223372036854775807L
// long is coded on 8 bytes 
#define strtoul_f strtoull
#define strtol_f  strtoll

#define ntohl_f ntohll
#define htonl_f htonll

#else
// long is coded on 4 bytes 
#define strtoul_f strtoul
#define strtol_f  strtol

#define ntohl_f ntohl
#define htonl_f htonl

#endif // LONG_MAX

// Define the ntoh and hton function for long long type
T_UnsignedInteger64 ntohll(T_UnsignedInteger64 P_val);
T_UnsignedInteger64 htonll(T_UnsignedInteger64 P_val);

#if defined(__hpux)
#include "iostream_t.hpp"

// Check the strtoll and strtoull implemenation
#ifndef strtoull
unsigned long long int strtoull(const char *P_nPtr, char **P_endPtr, int P_base);

iostream_output& operator<<(iostream_output&    P_ostream,
			    unsigned long long& P_value);
#endif

#ifndef strtoll
long long int strtoll(const char *P_nPtr, char **P_endPtr, int P_base);

iostream_output& operator<<(iostream_output&    P_ostream,
			    long long&          P_value);
#endif

#endif // defined(__hpux)


#endif // _INTEGER_T_HPP
