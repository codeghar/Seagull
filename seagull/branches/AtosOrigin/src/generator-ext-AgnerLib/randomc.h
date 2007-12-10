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

#ifndef RANDOMC_H
#define RANDOMC_H

#ifdef __INTEL_COMPILER
  #include <mathimf.h>       // Intel math function library
#else
  #include <math.h>          // default math function linrary
#endif

#include <assert.h>
#include <stdio.h>

// Define 32 bit signed and unsigned integers.
// Change these definitions, if necessary, on 64 bit computers
typedef   signed long int32;
typedef unsigned long uint32;

class TRandomMersenne {                // encapsulate random number generator
  #if 0
    // define constants for MT11213A:
    // (32 bit constants cannot be defined as enum in 16-bit compilers)
    #define MERS_N   351
    #define MERS_M   175
    #define MERS_R   19
    #define MERS_U   11
    #define MERS_S   7
    #define MERS_T   15
    #define MERS_L   17
    #define MERS_A   0xE4BD75F5
    #define MERS_B   0x655E5280
    #define MERS_C   0xFFD58000
  #else
    // or constants for MT19937:
    #define MERS_N   624
    #define MERS_M   397
    #define MERS_R   31
    #define MERS_U   11
    #define MERS_S   7
    #define MERS_T   15
    #define MERS_L   18
    #define MERS_A   0x9908B0DF
    #define MERS_B   0x9D2C5680
    #define MERS_C   0xEFC60000
  #endif
  public:
  TRandomMersenne(uint32 seed) {       // constructor
    RandomInit(seed);}
  void RandomInit(uint32 seed);        // re-seed
  double Random();                     // output random float
  uint32 BRandom();                    // output random bits
  private:
  uint32 mt[MERS_N];                   // state vector
  int mti;                             // index into mt
  enum TArch {LITTLE_ENDIAN1, BIG_ENDIAN1, NONIEEE};
  TArch Architecture;                  // conversion to float depends on computer architecture
  };

#endif
