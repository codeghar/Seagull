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

#include "C_StopTimerAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"


C_StopTimerAction::C_StopTimerAction(T_CmdAction        P_cmdAction,
                                     T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}


C_StopTimerAction::~C_StopTimerAction() {
}


T_exeCode    C_StopTimerAction::execute(T_pCmd_scenario P_pCmd,
                                        T_pCallContext  P_callCtxt,
                                        C_MessageFrame *P_msg,
                                        C_MessageFrame *P_ref) {
  
  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  long                L_time_ms                    ;

  GEN_DEBUG(1, "C_Scenario::execute_action() E_ACTION_SCEN_STOP_TIMER " << 
            m_controllers.m_log);
  L_time_ms = 
    ms_difftime(&P_callCtxt->m_current_time, 
                &P_callCtxt->m_start_time);
  m_controllers.m_stat->executeStatAction (C_GeneratorStats::E_ADD_RESPONSE_TIME_DURATION,
                             L_time_ms) ;
  if (m_controllers.m_log) {
    m_controllers.m_log->time_data(&P_callCtxt->m_start_time, 
                     &P_callCtxt->m_current_time);
  }
  
  return (L_exeCode);
}





