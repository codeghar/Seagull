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

#include "C_TrafficDistribPoisson.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "Utils.hpp" 

#include <cmath>

#include "randomc.h"                   // define classes for random number generators
#define RANDOM_GENERATOR TRandomMersenne // define which random number generator to use
#include "stocc.h"

// implementation of virtual function of the AgnerLib
void FatalError(char * ErrorText) {
  GEN_FATAL(E_GEN_FATAL_ERROR,"Error in AgnerLib Poisson distribution: " << ErrorText);
}

C_TrafficDistribPoisson::C_TrafficDistribPoisson () : C_TrafficModel() { 

  int L_seed;

  m_LastFctCallTS = 0;

  L_seed = time(0); 
  NEW_VAR(m_sto, StochasticLib1(L_seed));

}


C_TrafficDistribPoisson::~C_TrafficDistribPoisson() {
}



int C_TrafficDistribPoisson::authorize_new_call ()
{
  long L_desiredAverageRate ;

  float L_poissonMean;
  float L_poissonVal = 0.0;
  long  L_DeltaSinceLastFuncCall;   // ms

  reset ();
  update ();

  m_sem_desired->P();
  L_desiredAverageRate = m_desiredAverageRate ;
  m_sem_desired->V();

  if (m_currentTrafficDuration <= ms_setup_time) {
    return (0);
  }

  L_DeltaSinceLastFuncCall = m_currentTrafficDuration - m_LastFctCallTS;
//  L_DeltaSinceLastFuncCall = m_currentPeriodDuration ;
  L_poissonMean =( (float)  ( (float) L_DeltaSinceLastFuncCall / (float) 1000.0)  *
     (float) L_desiredAverageRate);
  L_poissonVal = m_sto->Poisson(L_poissonMean);
  
  m_LastFctCallTS = m_currentTrafficDuration;

  return ((int) L_poissonVal);

} /* end of authorize_new_call */


