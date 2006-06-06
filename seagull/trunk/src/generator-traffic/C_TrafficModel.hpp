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

#ifndef _C_TRAFFIC_MODEL_H
#define _C_TRAFFIC_MODEL_H

#include "C_Semaphore.hpp"

#include <sys/time.h>

class C_TrafficModel {
public:

  C_TrafficModel ();
  ~C_TrafficModel ();

  /**
   * Initialization
   *
   * This method initializes the traffic model manager.
   *
   * @param P_averageRate          : desired average call creation rate per second
   * @param P_maxCreationPerPeriod : maximum number of calls created per period (1s)
   * @param P_cotRate              : percentage of COT among created calls
   *
   * Note : in order to manage properly burst mechanism, the max number of call
   * creation within a period shall be superior to the average rate, or 0 for no limit
   *
   * @return 0 if success and -1 if failure
   *
   */
  int init (long P_averageRate, 
	    long P_maxCreationPerPeriod, 
	    long P_setupTime,
	    long P_rateScale);
  
  /**
   * Start
   *
   * This method must be called just before traffic start.
   *
   */
  void start ();
  
  /**
   * Indicates a new call has effectively been created
   */
  void call_created (); 
  
  /**
   * authorize_new_call.
   *
   * @param P_cot : (out)
   *
   * @return a positive value if new calls are allowed, and zero if not
   */
  int  authorize_new_call ();

  void change_desired_rate (unsigned long P_rate);
  void increase_desired_rate () ;
  void decrease_desired_rate () ;
  void change_max_created_calls (unsigned long P_max);

  void change_rate_scale (long P_rateScale) ;
  
private:

  void update() ;
  void re_init() ;

  C_Semaphore   *m_sem_desired ;
  C_Semaphore   *m_sem_max ;
  C_Semaphore   *m_sem_created_call ;
  C_Semaphore   *m_sem_created_call_period ;

  long           m_currentTrafficDuration;
  long           m_desiredAverageRate;
  long           m_createdCallNb;
  long           m_currentPeriodDuration;
  long           m_maxNbCreationPerPeriod;
  long           m_periodDuration;
  long           m_nbCallCreatedInPeriod;
  struct timeval m_startTrafficDate;
  struct timeval m_startPeriodDate;
  struct timeval m_currentTimeval;
  long           ms_setup_time;

  long           m_rateScale;

};

typedef C_TrafficModel *T_pTrafficModel ;

#endif // __TRAFFIC_MODEL_H__



