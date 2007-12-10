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

#include "C_CheckValueAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"


C_CheckValueAction::C_CheckValueAction(T_CmdAction        P_cmdAction,
                                       T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}


C_CheckValueAction::~C_CheckValueAction() {
}


T_exeCode    C_CheckValueAction::execute(T_pCmd_scenario P_pCmd,
                                    T_pCallContext  P_callCtxt,
                                    C_MessageFrame *P_msg,
                                    C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  bool                L_check_result               ;

  L_check_result 
    = P_msg->check_field_value 
    (P_ref,
     m_id,
     m_check_behaviour,
     m_instance_id,
     m_sub_id);
  if (L_check_result == false) {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                  "Parameter value check error on call with session-id ["
                  << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
    
    switch (m_check_behaviour) {
    case E_CHECK_BEHAVIOUR_ERROR :
      L_exeCode = E_EXE_ERROR_CHECK ;
      break;
    case E_CHECK_BEHAVIOUR_ABORT:
      L_exeCode = E_EXE_ABORT_CHECK ;
      break;
    default:
      break;
    }
  }

  return (L_exeCode);
}





