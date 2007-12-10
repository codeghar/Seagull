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

#include <ctype.h>
#include "integer_t.hpp"

#if defined(__hpux)

// Define the strtoll and strtoull function 

// The table below is used to convert from ASCII digits to a
// numerical equivalent.  It maps from '0' through 'z' to integers
// (100 for non-digit characters).
static char cvtIn[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,		/* '0' - '9' */
    100, 100, 100, 100, 100, 100, 100,		/* punctuation */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,	/* 'A' - 'Z' */
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35,
    100, 100, 100, 100, 100, 100,		/* punctuation */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,	/* 'a' - 'z' */
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35};

unsigned long long int strtoull(const char *P_nPtr, char **P_endPtr, int P_base)
{
  register const char *           L_pt        = NULL;
  register unsigned long long int L_result    = 0LL;
  register unsigned               L_digit;
  register unsigned long long int L_shifted;
  int                             L_anyDigits = 0;
  int                             L_negative  = 0;

  // Skip leading blanks
  L_pt = P_nPtr;
  while (isspace((unsigned char)(*L_pt))) {
    L_pt += 1;
  }

  // Check for a sign
  if (*L_pt == '-') {
    L_pt += 1;
    L_negative = 1;
  } else {
    if (*L_pt == '+') {
      L_pt += 1;
    }
  }

  // If no base was provided, pick one from the leading characters
  // of the string.
  if (P_base == 0) {
    if (*L_pt == '0') {
      L_pt += 1;
      if (*L_pt == 'x' || *L_pt == 'X') {
        L_pt += 1;
        P_base = 16;
      } else {
        // Must set L_anyDigits here, otherwise "0" produces a
        // "no digits" error.
        L_anyDigits = 1;
        P_base = 8;
      }
    } else {
      P_base = 10;
    }
  } else if (P_base == 16) {
    // Skip a leading "0x" from hex numbers.
    if ((L_pt[0] == '0') && (L_pt[1] == 'x' || *L_pt == 'X')) {
      L_pt += 2;
    }
  }

  // Sorry this code is so messy, but speed seems important.  Do
  // different things for base 8, 10, 16, and other.
  if (P_base == 8) {
    for ( ; ; L_pt += 1) {
      L_digit = *L_pt - '0';
      if (L_digit > 7) {
        break;
      }
      L_shifted = L_result << 3;
      if ((L_shifted >> 3) != L_result) {
        goto overflow;
      }
      L_result = L_shifted + L_digit;
      if ( L_result < L_shifted ) {
        goto overflow;
      }
      L_anyDigits = 1;
    }
  } else if (P_base == 10) {
    for ( ; ; L_pt += 1) {
      L_digit = *L_pt - '0';
      if (L_digit > 9) {
        break;
      }
      L_shifted = 10 * L_result;
      if ((L_shifted / 10) != L_result) {
        goto overflow;
      }
      L_result = L_shifted + L_digit;
      if ( L_result < L_shifted ) {
        goto overflow;
      }
      L_anyDigits = 1;
    }
  } else if (P_base == 16) {
    for ( ; ; L_pt += 1) {
      L_digit = *L_pt - '0';
      if (L_digit > ('z' - '0')) {
        break;
      }
      L_digit = cvtIn[L_digit];
      if (L_digit > 15) {
        break;
      }
      L_shifted = L_result << 4;
      if ((L_shifted >> 4) != L_result) {
        goto overflow;
      }
      L_result = L_shifted + L_digit;
      if ( L_result < L_shifted ) {
        goto overflow;
      }
      L_anyDigits = 1;
    }
  } else if ( P_base >= 2 && P_base <= 36 ) {
    for ( ; ; L_pt += 1) {
      L_digit = *L_pt - '0';
      if (L_digit > ('z' - '0')) {
        break;
      }
      L_digit = cvtIn[L_digit];
      if (L_digit >= (unsigned) P_base) {
        break;
      }
      L_shifted = L_result * P_base;
      if ((L_shifted/P_base) != L_result) {
        goto overflow;
      }
      L_result = L_shifted + L_digit;
      if ( L_result < L_shifted ) {
        goto overflow;
      }
      L_anyDigits = 1;
    }
  }

  // Negate if we found a '-' earlier.
  if (L_negative) {
    L_result = (unsigned long long int)(-((long long int)L_result));
  }

  // See if there were any digits at all.
  if (!L_anyDigits) {
    L_pt = P_nPtr;
  }

  if (P_endPtr != 0) {
    *P_endPtr = (char *) L_pt;
  }

  return L_result;

  // On overflow generate the right output
overflow:
  errno = ERANGE;
  if (P_endPtr != 0) {
    for ( ; ; L_pt += 1) {
      L_digit = *L_pt - '0';
      if (L_digit > ('z' - '0')) {
        break;
      }
      L_digit = cvtIn[L_digit];
      if (L_digit >= (unsigned) P_base) {
        break;
      }
    }
    *P_endPtr = (char *) L_pt;
  }
  return ((unsigned long long)(-1LL));
}

// to add << for unsigned long long
iostream_output& operator<<(iostream_output&    P_ostream,
                            unsigned long long& P_value) {

  char L_buffer[25];

  sprintf(L_buffer,"%llu", P_value);
  P_ostream << L_buffer;

  return (P_ostream);
}

long long int strtoll(const char *P_nPtr, char **P_endPtr, int P_base)
{
  register const char *  L_pt              = NULL;
  long long int          L_result          = 0LL;
  unsigned long long int L_unsigned_result = 0LL;

  // Skip leading blanks
  L_pt = P_nPtr;
  while (isspace((unsigned char)(*L_pt))) {
    L_pt += 1;
  }

  // Check for a sign.
  errno = 0;
  if ((*L_pt) == '-') {
    L_pt += 1;
    L_unsigned_result = strtoull(L_pt, P_endPtr, P_base);
    if (errno != ERANGE) {
      if (L_unsigned_result > LONG_LONG_MAX+1) {
        errno = ERANGE;
        return ((long long int)(-1LL));
      } else if (L_unsigned_result > LONG_LONG_MAX) {
        return ~((long long int)LONG_LONG_MAX);
      } else {
        L_result = -((long long int)L_unsigned_result);
      }
    }
  } else {
    if (*L_pt == '+') {
      L_pt += 1;
    }
    L_unsigned_result = strtoull(L_pt, P_endPtr, P_base);
    if (errno != ERANGE) {
      if (L_unsigned_result > LONG_LONG_MAX) {
        errno = ERANGE;
        return ((long long int)(-1LL));
      } else {
        L_result = L_unsigned_result;
      }
    }
  }

  if ((L_result == 0) && (P_endPtr != 0) && ((*P_endPtr) == L_pt)) {
    (*P_endPtr) = (char *) P_nPtr;
  }
  return L_result;
}

// to add << for long long
iostream_output& operator<<(iostream_output&   P_ostream,
                            long long&         P_value) {

  char L_buffer[25];

  sprintf(L_buffer,"%lld", P_value);
  P_ostream << L_buffer;

  return (P_ostream);
}

#endif

// Define new ntohll and htonll function
T_UnsignedInteger64 ntohll(T_UnsignedInteger64 P_val) {

#if BYTE_ORDER == BIG_ENDIAN
  return P_val;
#else
  return (((T_UnsignedInteger64)ntohl(P_val)) << 32) + ntohl(P_val >> 32);
#endif
}

T_UnsignedInteger64 htonll(T_UnsignedInteger64 P_val) {

#if BYTE_ORDER == BIG_ENDIAN
  return P_val;
#else
  return (((T_UnsignedInteger64)htonl(P_val)) << 32) + htonl(P_val >> 32);
#endif
}

// End of file
