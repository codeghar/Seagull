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

#include "C_TrafficDistribUniform.hpp"
#include "GeneratorTrace.hpp"
#include "Utils.hpp" 

#include <cmath>


C_TrafficDistribUniform::C_TrafficDistribUniform () : C_TrafficModel() { 
}


C_TrafficDistribUniform::~C_TrafficDistribUniform() {
}

int C_TrafficDistribUniform::authorize_new_call()
{
  int           L_callLimit;
  float         L_NB1;

  long          L_desiredAverageRate    ;
  long          L_createdCallNb         ;
  long          L_nbCallCreatedInPeriod ;
  long          L_currentPeriodDuration ;


  update ();

  m_sem_desired->P();
  L_desiredAverageRate = m_desiredAverageRate ;
  m_sem_desired->V();

  m_sem_created_call->P();
  L_createdCallNb = m_createdCallNb ;
  L_currentPeriodDuration = m_currentPeriodDuration ;

  m_sem_created_call->V();

  m_sem_created_call_period->P();
  L_nbCallCreatedInPeriod = m_nbCallCreatedInPeriod ;
  m_sem_created_call_period->V();

  if (m_currentTrafficDuration <= ms_setup_time) {
    return (0);
  }

  L_NB1 = ( (L_currentPeriodDuration * L_desiredAverageRate) /
            (float) 1000.0) ;


  L_callLimit = (int) (floor (L_NB1)) - L_nbCallCreatedInPeriod ;

  reset () ;

  return ((L_callLimit < 0) ? 0 : L_callLimit );
  
} /* end of authorize_new_call */




