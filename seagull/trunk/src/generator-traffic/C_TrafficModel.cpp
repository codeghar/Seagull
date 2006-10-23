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

#include "C_TrafficModel.hpp"
#include "GeneratorTrace.hpp"
#include "Utils.hpp" 

#include <cmath>

//#include <math.h> // for floor


#define GET_TIME(clock) \
        {\
                struct timezone tzp;\
                gettimeofday (clock, &tzp);\
        }


/**
** Update internal variables
**
** The following global variables are used :
**
** - m_currentTimeval
**
** The following internal variables are used :
**
** - m_currentTrafficDuration  (out)
** - m_startTrafficDate        (in)
** - m_currentPeriodDuration   (out)
** - m_startPeriodDate         (in)
** - m_periodDuration          (in)
** - m_nbCallCreatedInPeriod   (out)
**
**/

void C_TrafficModel::update ()
{
  GET_TIME(&m_currentTimeval);
  // Period duration : ti - tp in mili-seconds
  m_sem_created_call->P();
  m_currentPeriodDuration = (m_currentTimeval.tv_sec - 
                             m_startPeriodDate.tv_sec)*1000 + 
                            (m_currentTimeval.tv_usec - 
                             m_startPeriodDate.tv_usec)/1000;


  //std::cerr <<  "update  m_currentPeriodDuration " << m_currentPeriodDuration << std::endl;

  // Traffic duration : ti - T0 in mili-seconds
  m_currentTrafficDuration = (m_currentTimeval.tv_sec - 
                             m_startTrafficDate.tv_sec ) * 1000 + 
                            (m_currentTimeval.tv_usec - 
                             m_startTrafficDate.tv_usec ) / 1000;

  m_sem_created_call->V();

} /* end of update */



void C_TrafficModel::reset ()
{

  m_sem_created_call->P();
  if (m_currentPeriodDuration > m_periodDuration) {
    m_startPeriodDate.tv_sec  = m_currentTimeval.tv_sec;
    m_startPeriodDate.tv_usec = m_currentTimeval.tv_usec;

    m_nbCallCreatedInPeriod  = 0;
    m_currentPeriodDuration  = 0;
  }
  m_sem_created_call->V();
 

} /* end of reset */



int C_TrafficModel::init (long P_averageRate, 
			  long P_maxCreationPerPeriod, 
			  long P_setupTime,
			  long P_rateScale)
{
   m_desiredAverageRate = P_averageRate;
   m_maxNbCreationPerPeriod = P_maxCreationPerPeriod; 
   m_periodDuration = 1000;
   m_nbCallCreatedInPeriod = 0;
   m_currentTrafficDuration = 0;
   m_createdCallNb = 0;
   m_currentPeriodDuration = 0;
   ms_setup_time = P_setupTime * 1000;
   m_rateScale = P_rateScale ;

   return (0);
}


void C_TrafficModel::start ()
{
   GET_TIME (&m_startTrafficDate);
   GET_TIME (&m_startPeriodDate);
   GET_TIME (&m_currentTimeval);
}


void C_TrafficModel::call_created ()
{
  m_sem_created_call->P();
  m_createdCallNb++;
  m_sem_created_call->V();
  m_sem_created_call_period->P();
  m_nbCallCreatedInPeriod++;
  m_sem_created_call_period->V();
}


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
//  int C_TrafficModel::authorize_new_call ()
//  {
//    int           L_callLimit;
//    int           L_burstLimit;
//    float         L_NB1, L_S1;
//    float         L_NB2, L_NB3, L_S2;
//    unsigned long L_outgoingTraffic;
//    static bool   L_burst = false;

//    long L_maxNbCreationPerPeriod ;
//    long L_desiredAverageRate ;
//    long L_createdCallNb ;
//    long L_nbCallCreatedInPeriod ;
  
//    update ();
//    reset () ;

//    m_sem_max->P();
//    L_maxNbCreationPerPeriod = m_maxNbCreationPerPeriod ;
//    m_sem_max->V();
//    m_sem_desired->P();
//    L_desiredAverageRate = m_desiredAverageRate ;
//    m_sem_desired->V();
//    m_sem_created_call->P();
//    L_createdCallNb = m_createdCallNb ;
//    m_sem_created_call->V();
//    m_sem_created_call_period->P();
//    L_nbCallCreatedInPeriod = m_nbCallCreatedInPeriod ;
//    m_sem_created_call_period->V();
  
  
//    // check m_currentTrafficDuration
//    if (m_currentTrafficDuration <= ms_setup_time) {
//      return (0);
//    }
//    L_outgoingTraffic = m_currentTrafficDuration - ms_setup_time;

//    // compute L_callLimit

//    GEN_DEBUG(0, "Outgoing Traffic Duration (ti-TO): " << (long) L_outgoingTraffic);
//    GEN_DEBUG(0, "Desired Call Rate (VO)           : " << (long) L_desiredAverageRate);
//    GEN_DEBUG(0, "Calls Already Created (Ni)       : " << (long) L_createdCallNb);

//    L_NB1 = ( (float) ( (float) L_outgoingTraffic / (float) 1000.0) * 
//  	    (float) L_desiredAverageRate);

//    GEN_DEBUG(0, "NB1 = [ (ti - T0) * V0 ]         : " << L_NB1);

//    if ( (float) L_createdCallNb >= L_NB1 ) {
//      L_callLimit = 0;
//    } else {
//      L_S1 = L_NB1 - (float) L_createdCallNb;
//      L_callLimit = (int) (floor (L_S1) + (fmodf (L_S1, 1.0) > 0 ? 1 : 0));
//    }
//    //GEN_DEBUG (C_Debug::E_LEVEL_5, "INFO", "L_callLimit = %d ", L_callLimit);
   
//    if (!L_maxNbCreationPerPeriod) {
//      return (L_callLimit); // no burstLimit
//    }

//    GEN_DEBUG(0, "Current Period Duration (ti-tp)           : " << m_currentPeriodDuration << " ms");
//    GEN_DEBUG(0, "Max Call Creation per Period (Vp)         : " << L_maxNbCreationPerPeriod);
//    GEN_DEBUG(0, "Period Duration (D0)                      : " << m_periodDuration << " ms");
//    GEN_DEBUG(0, "Calls Already Created during Period (Np)  : " << m_nbCallCreatedInPeriod);

//    // compute L_burstLimit
//    L_NB2 = (float) L_nbCallCreatedInPeriod;
//    //L_NB3 = (((float) m_currentPeriodDuration * (float) m_maxNbCreationPerPeriod) /
//    // (P1)	((float) m_periodDuration * (float) 1000.0));

//    L_NB3 = (((float) m_currentPeriodDuration * 
//  	    (float) L_maxNbCreationPerPeriod) /
//  	   ((float) m_periodDuration));

//    GEN_DEBUG(0, "NB3 = [ ( (ti - tp) / D0 ) * Vp ] : " << L_NB3);

//    if (L_NB2 >= L_NB3) {
//      L_burstLimit = 0;
//    } else {
//      L_S2 = L_NB3 - L_NB2;
//      L_burstLimit = (int) (floor (L_S2) + (fmodf (L_S2, 1.0) > 0 ? 1 : 0));
//    }

//    GEN_DEBUG(0, "L_burstLimit = " << L_burstLimit);
//    if (!L_callLimit) {
//      GEN_DEBUG(0, "End of burst mode : reached expected call rate");
//      L_burst = false;
//    } else {
//      if (!L_burstLimit) {
//        // L_burstLimit may be null either if burst limit has been reached
//        // or at the beginning of a new period
//        if ((!L_burst) && (L_nbCallCreatedInPeriod)) {
//  	GEN_DEBUG(0, "Burst control : reached burst limit");
//  	L_burst = true;
//        }
//      }
//    }
//    return ((L_callLimit < L_burstLimit) ? L_callLimit : L_burstLimit);
//  } /* end of authorize_new_call */


C_TrafficModel::C_TrafficModel () 
{
  NEW_VAR(m_sem_desired, C_Semaphore());
  NEW_VAR(m_sem_max, C_Semaphore());
  NEW_VAR(m_sem_created_call, C_Semaphore());
  NEW_VAR(m_sem_created_call_period, C_Semaphore());
}


C_TrafficModel::~C_TrafficModel()
{
  DELETE_VAR(m_sem_desired);
  DELETE_VAR(m_sem_max);
  DELETE_VAR(m_sem_created_call);
  DELETE_VAR(m_sem_created_call_period);
}


unsigned long C_TrafficModel::get_desired_rate() {
  unsigned long L_return ;
  m_sem_desired->P();
  L_return = m_desiredAverageRate ;
  m_sem_desired->V();
  return (L_return);
}

void C_TrafficModel::change_desired_rate (unsigned long P_rate) {
  m_sem_desired->P();
  m_desiredAverageRate = P_rate ;
  re_init();
  m_sem_desired->V();
}

void C_TrafficModel::change_rate_scale (long P_rateScale) {
  m_rateScale = P_rateScale ;
}


void C_TrafficModel::increase_desired_rate () {
  m_sem_desired->P();
  m_desiredAverageRate += m_rateScale ;
  re_init();
  m_sem_desired->V();
}

void C_TrafficModel::decrease_desired_rate () {
  m_sem_desired->P();
  if (m_desiredAverageRate > m_rateScale) {
    m_desiredAverageRate -= m_rateScale ;
    re_init();
  }
  m_sem_desired->V();
}

void C_TrafficModel::change_max_created_calls (unsigned long P_max) {
  m_sem_max->P();
  m_maxNbCreationPerPeriod = P_max ;
  m_sem_max->V();
}

void C_TrafficModel::re_init() {
  m_sem_created_call->P();
  m_createdCallNb = 0;
  GET_TIME (&m_startTrafficDate);
  GET_TIME (&m_startPeriodDate);
  m_sem_created_call->V();
}


long C_TrafficModel::get_current_period_duration() {
  long          L_currentPeriodDuration ;
  
  m_sem_created_call->P();
  L_currentPeriodDuration = m_currentPeriodDuration ;
  m_sem_created_call->V();

  return (L_currentPeriodDuration);
}
