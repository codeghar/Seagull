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
 * This file is extracted from the random generator library provided by Agner Fog
 * (http://www.agner.org/random/) and has been modified for Seagull.
 * (c) 2002 Agner Fog. GNU General Public License www.gnu.org/copyleft/gpl.html
 *
 * (c)Copyright 2006 Hewlett-Packard Development Company, LP.
 *
 */

#include "randomc.h"


void TRandomMersenne::RandomInit(uint32 seed) {
  // re-seed generator
  mt[0]= seed;
  for (mti=1; mti < MERS_N; mti++) {
    mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);}

  // detect computer architecture
  union {double f; uint32 i[2];} convert;
  convert.f = 1.0;
  // Note: Old versions of the Gnu g++ compiler may make an error here,
  // compile with the option  -fenum-int-equiv  to fix the problem
  if (convert.i[1] == 0x3FF00000) Architecture = LITTLE_ENDIAN1;
  else if (convert.i[0] == 0x3FF00000) Architecture = BIG_ENDIAN1;
  else Architecture = NONIEEE;}

uint32 TRandomMersenne::BRandom() {
  // generate 32 random bits
  uint32 y;

  if (mti >= MERS_N) {
    // generate MERS_N words at one time
    const uint32 LOWER_MASK = (1LU << MERS_R) - 1;         // lower MERS_R bits
    const uint32 UPPER_MASK = 0xFFFFFFFF << MERS_R;        // upper (32 - MERS_R) bits
    static const uint32 mag01[2] = {0, MERS_A};

    int kk;
    for (kk=0; kk < MERS_N-MERS_M; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[kk+MERS_M] ^ (y >> 1) ^ mag01[y & 1];}

    for (; kk < MERS_N-1; kk++) {
      y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[kk+(MERS_M-MERS_N)] ^ (y >> 1) ^ mag01[y & 1];}

    y = (mt[MERS_N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[MERS_N-1] = mt[MERS_M-1] ^ (y >> 1) ^ mag01[y & 1];
    mti = 0;}

  y = mt[mti++];

  // Tempering (May be omitted):
  y ^=  y >> MERS_U;
  y ^= (y << MERS_S) & MERS_B;
  y ^= (y << MERS_T) & MERS_C;
  y ^=  y >> MERS_L;
  return y;}



double TRandomMersenne::Random() {
  // output random float number in the interval 0 <= x < 1
  union {double f; uint32 i[2];} convert;
  uint32 r = BRandom(); // get 32 random bits
  // The fastest way to convert random bits to floating point is as follows:
  // Set the binary exponent of a floating point number to 1+bias and set
  // the mantissa to random bits. This will give a random number in the
  // interval [1,2). Then subtract 1.0 to get a random number in the interval
  // [0,1). This procedure requires that we know how floating point numbers
  // are stored. The storing method is tested in function RandomInit and saved
  // in the variable Architecture. The following switch statement can be
  // omitted if the architecture is known. (A PC running Windows or Linux uses
  // LITTLE_ENDIAN1 architecture):
  switch (Architecture) {
  case LITTLE_ENDIAN1:
    convert.i[0] =  r << 20;
    convert.i[1] = (r >> 12) | 0x3FF00000;
    return convert.f - 1.0;
  case BIG_ENDIAN1:
    convert.i[1] =  r << 20;
    convert.i[0] = (r >> 12) | 0x3FF00000;
    return convert.f - 1.0;
  case NONIEEE: default:
  ;}
  // This somewhat slower method works for all architectures, including
  // non-IEEE floating point representation:
  return (double)r * (1./((double)(uint32)(-1L)+1.));}

