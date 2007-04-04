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

#include "C_AddDefaultInCallMapAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_CallControl.hpp"
#include "map_t.hpp"

C_AddDefaultInCallMapAction::C_AddDefaultInCallMapAction(T_CmdAction        P_cmdAction,
                                           T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}

C_AddDefaultInCallMapAction::~C_AddDefaultInCallMapAction() {
}


T_exeCode    C_AddDefaultInCallMapAction::execute(T_pCmd_scenario P_pCmd,
                                           T_pCallContext  P_callCtxt,
                                           C_MessageFrame *P_msg,
                                           C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  
  T_pValueData  L_value_id ;
  C_CallContext::T_pCallMap   *L_map = P_callCtxt->m_call_control->get_call_map();
  
  L_value_id = P_msg -> get_session_id(P_callCtxt);
  if (L_value_id == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "session id is failed");
    L_exeCode = E_EXE_ERROR ;
  } else {
    pair_t<C_CallContext::T_CallMap::iterator,bool> L_pr ;
    C_CallContext::T_contextMapData L_data ;

    L_pr =
      L_map[m_id]
      ->insert(C_CallContext::T_CallMap::value_type(*L_value_id, P_callCtxt));
    L_data.m_channel = m_id ;
    L_data.m_iterator = L_pr.first ;
    (P_callCtxt->m_map_data_list)->push_back(L_data);
  }
  
  return (L_exeCode);
}





