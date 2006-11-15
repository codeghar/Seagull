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

#include <math.h>      // math functions
#include "stocc.h"     // class definition

/***********************************************************************
                     Log factorial function
***********************************************************************/
double LnFac(int32 n) {
  // log factorial function. gives natural logarithm of n!

  // define constants
  static const double        // coefficients in Stirling approximation
    C0 =  0.918938533204672722,   // ln(sqrt(2*pi))
    C1 =  1./12.,
    C3 = -1./360.;
    // C5 =  1./1260.,  // use r^5 term if FAK_LEN < 50
    // C7 = -1./1680.;  // use r^7 term if FAK_LEN < 20
  // static variables
  static double fac_table[FAK_LEN]; // table of ln(n!):
  static int initialized = 0;   // remember if fac_table has been initialized

  if (n < FAK_LEN) {
    if (n <= 1) {
      if (n < 0) FatalError("Parameter negative in LnFac function");
      return 0;}
    if (!initialized) { // first time. Must initialize table
      // make table of ln(n!)
      double sum = fac_table[0] = 0.;
      for (int i=1; i<FAK_LEN; i++) {
        sum += log(float(i));
        fac_table[i] = sum;}
      initialized = 1;}
    return fac_table[n];}

  // not found in table. use Stirling approximation
  double  n1, r;
  n1 = n;  r  = 1. / n1;
  return (n1 + 0.5)*log(n1) - n1 + C0 + r*(C1 + r*r*C3);}


/***********************************************************************
                      constants
***********************************************************************/
const double SHAT1 = 2.943035529371538573;    // 8/e
const double SHAT2 = 0.8989161620588987408;   // 3-sqrt(12/e)

/***********************************************************************
                      Poisson distribution
***********************************************************************/
int32 StochasticLib1::Poisson (double L) {
/*
   This function generates a random variate with the poisson distribution.

   Uses inversion by chop-down method for L < 17, and ratio-of-uniforms
   method for L >= 17.

   For L < 1.E-6 numerical inaccuracy is avoided by direct calculation.
*/

  //------------------------------------------------------------------
  //                 choose method
  //------------------------------------------------------------------
  if (L < 17) {
    if (L < 1.E-6) {
      if (L == 0) return 0;
      if (L < 0) FatalError("Parameter negative in poisson function");

      //--------------------------------------------------------------
      // calculate probabilities
      //--------------------------------------------------------------
      // For extremely small L we calculate the probabilities of x = 1
      // and x = 2 (ignoring higher x). The reason for using this
      // method is to prevent numerical inaccuracies in other methods.
      //--------------------------------------------------------------
      return PoissonLow(L);}

    else {

      //--------------------------------------------------------------
      // inversion method
      //--------------------------------------------------------------
      // The computation time for this method grows with L.
      // Gives overflow for L > 80
      //--------------------------------------------------------------
      return PoissonInver(L);}}

  else {
    if (L > 2.E9) FatalError("Parameter too big in poisson function");

    //----------------------------------------------------------------
    // ratio-of-uniforms method
    //----------------------------------------------------------------
    // The computation time for this method does not depend on L.
    // Use where other methods would be slower.
    //----------------------------------------------------------------
    return PoissonRatioUniforms(L);}}


/***********************************************************************
                      Subfunctions used by poisson
***********************************************************************/
int32 StochasticLib1::PoissonLow(double L) {
/*
   This subfunction generates a random variate with the poisson
   distribution for extremely low values of L.

   The method is a simple calculation of the probabilities of x = 1
   and x = 2. Higher values are ignored.

   The reason for using this method is to avoid the numerical inaccuracies
   in other methods.
*/
  double d, r;
  d = sqrt(L);
  if (Random() >= d) return 0;
  r = Random() * d;
  if (r > L * (1.-L)) return 0;
  if (r > 0.5 * L*L * (1.-L)) return 1;
  return 2;}

int32 StochasticLib1::PoissonInver(double L) {
/*
   This subfunction generates a random variate with the poisson
   distribution using inversion by the chop down method (PIN).

   Execution time grows with L. Gives overflow for L > 80.

   The value of bound must be adjusted to the maximal value of L.
*/
  const int bound = 130;             // safety bound. Must be > L + 8*sqrt(L).
  static double p_L_last = -1.;      // previous value of L
  static double p_f0;                // value at x=0
  double r;                          // uniform random number
  double f;                          // function value
  int32 x;                           // return value

  if (L != p_L_last) {               // set up
    p_L_last = L;
    p_f0 = exp(-L);}                 // f(0) = probability of x=0

  while (1) {
    r = Random();  x = 0;  f = p_f0;
    do {                        // recursive calculation: f(x) = f(x-1) * L / x
      r -= f;
      if (r <= 0) return x;
      x++;
      f *= L;
      r *= x;}                       // instead of f /= x
    while (x <= bound);}}

int32 StochasticLib1::PoissonRatioUniforms(double L) {
/*
   This subfunction generates a random variate with the poisson
   distribution using the ratio-of-uniforms rejection method (PRUAt).

   Execution time does not depend on L, except that it matters whether L
   is within the range where ln(n!) is tabulated.

   Reference: E. Stadlober: "The ratio of uniforms approach for generating
   discrete random variates". Journal of Computational and Applied Mathematics,
   vol. 31, no. 1, 1990, pp. 181-189.
*/
  static double p_L_last = -1.0;            // previous L
  static double p_a;                       // hat center
  static double p_h;                       // hat width
  static double p_g;                       // ln(L)
  static double p_q;                       // value at mode
  static int32 p_bound;                    // upper bound
  int32 mode;                              // mode
  double u;                                // uniform random
  double lf;                               // ln(f(x))
  double x;                                // real sample
  int32 k;                                 // integer sample

  if (p_L_last != L) {
    p_L_last = L;                           // Set-up
    p_a = L + 0.5;                          // hat center
    mode = (int32)L;                        // mode
    p_g  = log(L);
    p_q = mode * p_g - LnFac(mode);         // value at mode
    p_h = sqrt(SHAT1 * (L+0.5)) + SHAT2;    // hat width
    p_bound = (int32)(p_a + 6.0 * p_h);}    // safety-bound

  while(1) {
    u = Random();
    if (u == 0) continue;                   // avoid division by 0
    x = p_a + p_h * (Random() - 0.5) / u;
    if (x < 0 || x >= p_bound) continue;    // reject if outside valid range
    k = (int32)(x);
    lf = k * p_g - LnFac(k) - p_q;
    if (lf >= u * (4.0 - u) - 3.0) break;   // quick acceptance
    if (u * (u - lf) > 1.0) continue;       // quick rejection
    if (2.0 * log(u) <= lf) break;}         // final acceptance
  return(k);}


/***********************************************************************
                      Constructor
***********************************************************************/
StochasticLib1::StochasticLib1 (int seed)
  : RANDOM_GENERATOR(seed) {
  normal_x2_valid = 0;}



