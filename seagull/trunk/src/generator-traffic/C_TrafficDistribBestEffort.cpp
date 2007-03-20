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

#include "C_TrafficDistribBestEffort.hpp"
#include "GeneratorTrace.hpp"
#include "Utils.hpp" 

#include <cmath>


/**
** authorize_new_call
**
** The following internal variables are used (see also update()):
**
** - m_desiredAverageRate      (in)
** - m_createdCallNb           (in)
** - m_maxNbCreationPerPeriod  (in)
** - m_periodDuration          (in)
** - m_currentTrafficDuration  (in)
**
**/

int C_TrafficDistribBestEffort::authorize_new_call ()
{
  int           L_callLimit;
  int           L_burstLimit;
  float         L_NB1, L_S1;
  float         L_NB2, L_NB3, L_S2;
  unsigned long L_outgoingTraffic;
  static bool   L_burst = false;

  long L_maxNbCreationPerPeriod ;
  long L_desiredAverageRate ;
  long L_createdCallNb ;
  long L_nbCallCreatedInPeriod ;
  
  update ();
  reset () ;

  m_sem_max->P();
  L_maxNbCreationPerPeriod = m_maxNbCreationPerPeriod ;
  m_sem_max->V();
  m_sem_desired->P();
  L_desiredAverageRate = m_desiredAverageRate ;
  m_sem_desired->V();
  m_sem_created_call->P();
  L_createdCallNb = m_createdCallNb ;
  m_sem_created_call->V();
  m_sem_created_call_period->P();
  L_nbCallCreatedInPeriod = m_nbCallCreatedInPeriod ;
  m_sem_created_call_period->V();
  
  
  // check m_currentTrafficDuration
  if (m_currentTrafficDuration <= ms_setup_time) {
    return (0);
  }

  L_outgoingTraffic = m_currentTrafficDuration - ms_setup_time;

  GEN_DEBUG(0, "Outgoing Traffic Duration (ti-TO): " << (long) L_outgoingTraffic);
  GEN_DEBUG(0, "Desired Call Rate (VO)           : " << (long) L_desiredAverageRate);
  GEN_DEBUG(0, "Calls Already Created (Ni)       : " << (long) L_createdCallNb);


  L_NB1 = ( (float) ( (float)  L_outgoingTraffic / (float) 1000.0) * 
	    (float) L_desiredAverageRate);

  GEN_DEBUG(0, "NB1 = [ (ti - T0) * V0 ]         : " << L_NB1);

  if ( (float) L_createdCallNb >= L_NB1 ) {
    L_callLimit = 0;
  } else {
    L_S1 = L_NB1 - (float) L_createdCallNb;
    L_callLimit = (int) (floor (L_S1) + (fmodf (L_S1, 1.0) > 0 ? 1 : 0));
  }
  
  if (!L_maxNbCreationPerPeriod) {
    return (L_callLimit); // no burstLimit
  }

  GEN_DEBUG(0, "Current Period Duration (ti-tp)           : " << m_currentPeriodDuration << " ms");
  GEN_DEBUG(0, "Max Call Creation per Period (Vp)         : " << L_maxNbCreationPerPeriod);
  GEN_DEBUG(0, "Period Duration (D0)                      : " << m_periodDuration << " ms");
  GEN_DEBUG(0, "Calls Already Created during Period (Np)  : " << m_nbCallCreatedInPeriod);

  // compute L_burstLimit
  L_NB2 = (float) L_nbCallCreatedInPeriod;
  //L_NB3 = (((float) m_currentPeriodDuration * (float) m_maxNbCreationPerPeriod) /
  // (P1)	((float) m_periodDuration * (float) 1000.0));

  L_NB3 = (((float) m_currentPeriodDuration * 
	    (float) L_maxNbCreationPerPeriod) /
	   ((float) m_periodDuration));

  GEN_DEBUG(0, "NB3 = [ ( (ti - tp) / D0 ) * Vp ] : " << L_NB3);

  if (L_NB2 >= L_NB3) {
    L_burstLimit = 0;
  } else {
    L_S2 = L_NB3 - L_NB2;
    L_burstLimit = (int) (floor (L_S2) + (fmodf (L_S2, 1.0) > 0 ? 1 : 0));
  }

  GEN_DEBUG(0, "L_burstLimit = " << L_burstLimit);
  if (!L_callLimit) {
    GEN_DEBUG(0, "End of burst mode : reached expected call rate");
    L_burst = false;
  } else {
    if (!L_burstLimit) {
      // L_burstLimit may be null either if burst limit has been reached
      // or at the beginning of a new period
      if ((!L_burst) && (L_nbCallCreatedInPeriod)) {
	GEN_DEBUG(0, "Burst control : reached burst limit");
	L_burst = true;
      }
    }
  }
  return ((L_callLimit < L_burstLimit) ? L_callLimit : L_burstLimit);
} /* end of authorize_new_call */


C_TrafficDistribBestEffort::C_TrafficDistribBestEffort () : C_TrafficModel() { 
}


C_TrafficDistribBestEffort::~C_TrafficDistribBestEffort() {
}

