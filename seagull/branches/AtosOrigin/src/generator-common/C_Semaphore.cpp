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

#include "Utils.hpp"
#include "C_Semaphore.hpp"
#include "SemaphoreImpl.h"

#include "GeneratorTrace.hpp"


C_Semaphore::C_Semaphore() {
  GEN_DEBUG (0, "C_Semaphore::C_Semaphore() start");
  ALLOC_VAR (m_impl, _pc_semaphore, sizeof(_c_semaphore)); 
  init () ;
  GEN_DEBUG (0, "C_Semaphore::C_Semaphore() end");
}

C_Semaphore::~C_Semaphore() {
  GEN_DEBUG(0,"C_Semaphore::~C_Semaphore() start");
  destroy () ;
  FREE_VAR (m_impl) ;
  GEN_DEBUG(0,"C_Semaphore::~C_Semaphore() end");
}

void C_Semaphore::init () {
  pthread_mutexattr_t L_mxAttr;
  pthread_condattr_t  L_cdAttr;

  SemCounter = COUNTER_INIT_VALUE ;

  pthread_mutexattr_init(&L_mxAttr);
  pthread_mutexattr_settype(&L_mxAttr, PTHREAD_MUTEX_NORMAL);
  pthread_mutex_init (SemMutex, &L_mxAttr) ;

  pthread_condattr_init(&L_cdAttr);
  pthread_cond_init (SemCond, &L_cdAttr) ;
} // init ()

void C_Semaphore::destroy () {
  GEN_DEBUG(0, "C_Semaphore::destroy() start");
  SemCounter = COUNTER_INIT_VALUE ;
  pthread_mutex_destroy (SemMutex) ;
  pthread_cond_destroy (SemCond) ;
  GEN_DEBUG(0, "C_Semaphore::destroy() end");
} // destroy ()

T_CounterValue C_Semaphore::V() {

  T_CounterValue L_value ;

  pthread_mutex_lock (SemMutex) ;
  SemCounter ++ ;
  L_value = SemCounter ;
  pthread_mutex_unlock (SemMutex);
  pthread_cond_signal (SemCond) ;

  return (L_value);
  
} // V()

T_CounterValue C_Semaphore::P() {

  T_CounterValue L_value = 0 ;

  pthread_mutex_lock (SemMutex) ;
  while (SemCounter <= COUNTER_COMP_VALUE) {
    pthread_cond_wait (SemCond, SemMutex);
  }
  SemCounter -- ;
  L_value = SemCounter ;
  pthread_mutex_unlock (SemMutex);

  return (L_value) ;

} // P()
