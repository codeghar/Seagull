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

#ifndef _C_TASK_CONTROL
#define _C_TASK_CONTROL

#include "iostream_t.hpp"
#include <pthread.h>

#include "GeneratorError.h"

class C_TaskControl {

public:

  typedef enum _enum_state {
    E_STATE_UNKNOWN = 0 ,
    E_STATE_INIT,
    E_STATE_RUNNING,
    E_STATE_STOPPING,
    E_STATE_STOPPED 
  } T_State ;

           C_TaskControl() ;
  virtual ~C_TaskControl() ;

  T_GeneratorError init () ;
  T_GeneratorError run  () ;
  T_GeneratorError run_task_once () ;
  T_GeneratorError run_all_once () ;

  void    stop () ;
  T_State get_state() ;

protected:
  
  virtual T_GeneratorError TaskProcedure () = 0 ;
  virtual T_GeneratorError InitProcedure () = 0 ;
  virtual T_GeneratorError EndProcedure () = 0 ;
  virtual T_GeneratorError StoppingProcedure () = 0 ;
  virtual T_GeneratorError ForcedStoppingProcedure () = 0 ;

  virtual T_GeneratorError InitTaskProcedure () { return (E_GEN_NO_ERROR); } ;

  T_State     M_state ;
  bool        m_end_executed ;

} ; // class C_TaskControl

iostream_output& operator<<(iostream_output&, C_TaskControl::T_State&);

// function to start a TaskControl within a thread
pthread_t* start_thread_control (C_TaskControl *P_taskControl) ;
void       wait_thread_control_end (pthread_t *P_thread);

#endif // _C_TASK_CONTROL
