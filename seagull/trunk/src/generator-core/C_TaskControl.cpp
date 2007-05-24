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

#include "C_TaskControl.hpp"
#include "Utils.hpp"
#include "GeneratorTrace.hpp"

C_TaskControl::C_TaskControl() {
  GEN_DEBUG(0, "C_TaskControl::C_TaskControl() start");
  M_state = C_TaskControl::E_STATE_UNKNOWN ;
  m_end_executed = false ;
  GEN_DEBUG(0, "C_TaskControl::C_TaskControl() end");
}

C_TaskControl::~C_TaskControl() {
  GEN_DEBUG(0, "C_TaskControl::~C_TaskControl() start");
  M_state = C_TaskControl::E_STATE_UNKNOWN ;
  m_end_executed = false ;
  GEN_DEBUG(0, "C_TaskControl::~C_TaskControl() end");
}

T_GeneratorError C_TaskControl::init () {

  T_GeneratorError L_error_code ;
  GEN_DEBUG(0, "C_TaskControl::init() start");
  L_error_code = InitProcedure();
  if (L_error_code == E_GEN_NO_ERROR) {
    M_state = C_TaskControl::E_STATE_INIT ;
  }
  GEN_DEBUG(0, "C_TaskControl::init() end");
  return  (L_error_code);
}

T_GeneratorError C_TaskControl::run_task_once  () {

  T_GeneratorError L_error_code = E_GEN_NO_ERROR;

  GEN_DEBUG(0, "C_TaskControl::run_task_once() start State:" 
               << M_state);

  switch (M_state) {
    
  case C_TaskControl::E_STATE_INIT:
    M_state = C_TaskControl::E_STATE_RUNNING ;
    L_error_code = TaskProcedure () ;
    break ;

  case C_TaskControl::E_STATE_RUNNING:
  case C_TaskControl::E_STATE_STOPPING:
    L_error_code = TaskProcedure () ;
    break ;

  case C_TaskControl::E_STATE_STOPPED:
    if (m_end_executed == false) {
      L_error_code = EndProcedure() ;
      m_end_executed = true ;
    }
    break ;

  default:
    GEN_ERROR (E_GEN_FATAL_ERROR, "Incorrect state " << M_state);
    L_error_code = E_GEN_FATAL_ERROR ;
    break ;

  } // switch M_state

  GEN_DEBUG(0, "C_TaskControl::run_task_once() end State " 
               << M_state);

  return (L_error_code);
}

T_GeneratorError C_TaskControl::run_all_once  () {

  T_GeneratorError L_error_code = E_GEN_NO_ERROR;

  GEN_DEBUG(0, "C_TaskControl::run_all_once() start");
  if (M_state == C_TaskControl::E_STATE_INIT) {
    M_state = C_TaskControl::E_STATE_RUNNING ;
    L_error_code = TaskProcedure () ;
    M_state = C_TaskControl::E_STATE_STOPPED ;
    L_error_code = EndProcedure() ;
  } else {
    GEN_ERROR (E_GEN_FATAL_ERROR, "Incorrect C_Generator state " << M_state);
    L_error_code = E_GEN_FATAL_ERROR ;
  }
  return (L_error_code);
  GEN_DEBUG(0, "C_TaskControl::run_all_once() end");
}

T_GeneratorError C_TaskControl::run  () {

  T_GeneratorError L_error_code = E_GEN_NO_ERROR;

  GEN_DEBUG(0, "C_TaskControl::run() start");
  if (M_state == C_TaskControl::E_STATE_INIT) {
    M_state = C_TaskControl::E_STATE_RUNNING ;
    while (M_state != C_TaskControl::E_STATE_STOPPED) {
      L_error_code = TaskProcedure () ;
    }
    if (L_error_code == E_GEN_NO_ERROR) {
      L_error_code = EndProcedure() ;
    }
  } else {
    GEN_ERROR (E_GEN_FATAL_ERROR, "Incorrect C_Generator state " << M_state);
    L_error_code = E_GEN_FATAL_ERROR ;
  }
  return (L_error_code);
  GEN_DEBUG(0, "C_TaskControl::run() end");
}

void C_TaskControl::stop () {
  
  // T_GeneratorError L_error_code ;

  GEN_DEBUG(0, "C_TaskControl::stop() start");

  switch (M_state) {
  case E_STATE_RUNNING:
    GEN_DEBUG(0, "C_TaskControl::stop() RUNNING=>STOPPING");
    M_state = C_TaskControl::E_STATE_STOPPING ;
   (void) StoppingProcedure();
    break ;
  case E_STATE_STOPPING:
    GEN_DEBUG(0, "C_TaskControl::stop() STOPPING");
    (void) ForcedStoppingProcedure ();
    break ;
  default:
    break ;
  }
  GEN_DEBUG(0, "C_TaskControl::stop() end");

}

C_TaskControl::T_State C_TaskControl::get_state() {
  GEN_DEBUG(1, "C_TaskControl::get_state()");
  return (M_state);
}

iostream_output& operator<<(iostream_output& P_ostream, 
			    C_TaskControl::T_State& P_state) {
  const char* c_string_state [] = {
    "E_STATE_UNKNOWN",
    "E_STATE_INIT",
    "E_STATE_RUNNING",
    "E_STATE_STOPPING",
    "E_STATE_STOPPED" 
  } ;

  P_ostream << c_string_state[P_state] ;

  return (P_ostream);
}


static void* call_run (void* P_data) {

  C_TaskControl    *L_taskControl ;
  T_GeneratorError  L_errorCode ;

  GEN_DEBUG(0, "call_run() start");
  
  L_taskControl = (C_TaskControl*) P_data ;

  L_errorCode = L_taskControl -> run () ;

  pthread_exit ((void*)L_errorCode) ;

  GEN_DEBUG(0, "call_run() end");

  return (NULL) ;
}


pthread_t* start_thread_control (C_TaskControl *P_taskControl) {

  pthread_t      *L_thread     ;
  int             L_return     ;

  GEN_DEBUG(0, "start_thread_control() start");
  ALLOC_VAR(L_thread, pthread_t*, sizeof(pthread_t));

  L_return = pthread_create (L_thread, 
 			     NULL, 
 			     call_run,
 			     (void*) P_taskControl) ;

  if (L_return != 0) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "pthread_create() error");
  }

  GEN_DEBUG(0, "start_controller() end");
  return (L_thread);
}

void wait_thread_control_end (pthread_t *P_thread) {

  void *L_return ;
  pthread_join (*P_thread, &L_return);

  // Delete thread
  FREE_VAR(P_thread);
}

