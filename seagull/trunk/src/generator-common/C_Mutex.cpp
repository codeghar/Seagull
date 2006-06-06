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

#include "MutexImpl.h"

#include "Utils.hpp"
#include "C_Mutex.hpp"


C_Mutex::C_Mutex() {
  ALLOC_VAR (m_impl, _pc_mutex, sizeof(_c_mutex)); 
  init () ;
}

C_Mutex::~C_Mutex() {
  destroy () ;
  FREE_VAR (m_impl) ;
}

void C_Mutex::init () {

  pthread_mutexattr_t L_mxAttr;

  pthread_mutexattr_init(&L_mxAttr);
  pthread_mutexattr_settype(&L_mxAttr, PTHREAD_MUTEX_NORMAL);
  pthread_mutex_init (MutexPt, &L_mxAttr) ;

} // init ()

void C_Mutex::destroy () {

  pthread_mutex_destroy (MutexPt) ;

} // destroy ()

void C_Mutex::unlock() {

  pthread_mutex_unlock (MutexPt);

} // unlock()

void C_Mutex::lock() {

  pthread_mutex_lock (MutexPt) ;

} // lock()
