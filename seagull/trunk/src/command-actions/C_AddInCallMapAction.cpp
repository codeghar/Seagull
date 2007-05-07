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

#include "C_AddInCallMapAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_CallControl.hpp"

C_AddInCallMapAction::C_AddInCallMapAction(T_CmdAction        P_cmdAction,
                                           T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}

C_AddInCallMapAction::~C_AddInCallMapAction() {
}


T_exeCode    C_AddInCallMapAction::execute(T_pCmd_scenario P_pCmd,
                                           T_pCallContext  P_callCtxt,
                                           C_MessageFrame *P_msg,
                                           C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  
  T_pValueData        L_value_id   = NULL          ;
  C_CallContext::T_pCallMap   *L_map = P_callCtxt->m_call_control->get_call_map();
  
  L_value_id = P_msg -> get_session_id(P_callCtxt);
  if (L_value_id == NULL) {
    // TO DO
    GEN_ERROR(E_GEN_FATAL_ERROR, "session id is failed");
    L_exeCode = E_EXE_ERROR ;
  } else {
    L_value_id = P_callCtxt->set_id (m_id,L_value_id);
    L_map[m_id]
      ->insert(C_CallContext::T_CallMap::value_type(*L_value_id, P_callCtxt));
  }
  
  return (L_exeCode);
}





