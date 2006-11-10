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
#ifndef STOCC_H
#define STOCC_H

#include "randomc.h"

#ifndef RANDOM_GENERATOR
#define RANDOM_GENERATOR TRandomMersenne
#endif

/***********************************************************************
         System-specific user interface functions
***********************************************************************/
void FatalError(char * ErrorText);     // system-specific error reporting

/***********************************************************************
         Constants and tables
***********************************************************************/

// constant for LnFac function:
static const int FAK_LEN = 1024;       // length of factorial table


/***********************************************************************
         Class StochasticLib1
***********************************************************************/

class StochasticLib1 : public RANDOM_GENERATOR {
  // This class encapsulates the random variate generating functions.
  // May be derived from any of the random number generators.
  public:
  StochasticLib1 (int seed);           // constructor
  int32 Poisson (double L);            // poisson distribution

  // functions used internally
  protected:
  static double fc_lnpk(int32 k, int32 N_Mn, int32 M, int32 n); // used by Hypergeometric

  // subfunctions for each approximation method
  int32 PoissonInver(double L);                       // poisson by inversion
  int32 PoissonRatioUniforms(double L);               // poisson by ratio of uniforms
  int32 PoissonLow(double L);                         // poisson for extremely low L
  int32 BinomialInver (int32 n, double p);            // binomial by inversion
  int32 BinomialRatioOfUniforms (int32 n, double p);  // binomial by ratio of uniforms
  int32 HypInversionMod (int32 n, int32 M, int32 N);  // hypergeometric by inversion searching from mode
  int32 HypRatioOfUnifoms (int32 n, int32 M, int32 N);// hypergeometric by ratio of uniforms method

  // variables used by Normal distribution
  double normal_x2;  int normal_x2_valid;

  // define constants
  enum constants {
    // maximum value of 'colors' in multivariate distributions:
    MAXCOLORS = 20};         // you may change this number
  };
  
#endif
