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

#include "SemaphoreImpl.h"
#include "C_SemaphoreTimed.hpp"
#include "GeneratorTrace.hpp"

#include <sys/time.h>
#include <unistd.h>
#include <cerrno>

C_SemaphoreTimed::C_SemaphoreTimed(long P_timeOut):C_Semaphore() {

  GEN_DEBUG(0, "C_SemaphoreTimed::C_SemaphoreTimed(" << P_timeOut << ") start");
  m_timeOut = P_timeOut ;
  GEN_DEBUG(0, "C_SemaphoreTimed::C_SemaphoreTimed() end");
}

C_SemaphoreTimed::~C_SemaphoreTimed() {
  GEN_DEBUG(0, "C_SemaphoreTimed::~C_SemaphoreTimed() start");
  m_timeOut = 0 ;
  GEN_DEBUG(0, "C_SemaphoreTimed::~C_SemaphoreTimed() end");
}

T_CounterValue C_SemaphoreTimed::P() {

  T_CounterValue L_value   ;
  struct timeval    L_now     ;
  struct timespec   L_timeOut ;
  struct timezone   L_tzp     ;
  int               L_return  = -1; // do not forget to initialize
                                    // for re-entry call

  gettimeofday (&L_now,&L_tzp);
  L_timeOut.tv_sec = L_now.tv_sec + (time_t) m_timeOut ;
  L_timeOut.tv_nsec = L_now.tv_usec * 1000 ;

  pthread_mutex_lock (SemMutex) ;
  while (   (SemCounter <= COUNTER_COMP_VALUE) 
  	 && (L_return != ETIMEDOUT)) {

    L_return = pthread_cond_timedwait (SemCond, SemMutex, &L_timeOut);

  }
  if (L_return != ETIMEDOUT) {
    SemCounter -- ;
  } 
  L_value = SemCounter ;
  pthread_mutex_unlock (SemMutex);

  return (L_value) ;

} // P()


void C_SemaphoreTimed::change_display_period(long P_period) {
  m_timeOut = P_period;
}




















