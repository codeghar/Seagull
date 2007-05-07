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

#include "C_InsertInMapAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_CallControl.hpp"
#include "map_t.hpp"


C_InsertInMapAction::C_InsertInMapAction(T_CmdAction        P_cmdAction,
                                                 T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}

C_InsertInMapAction::~C_InsertInMapAction() {
}


T_exeCode    C_InsertInMapAction::execute(T_pCmd_scenario P_pCmd,
                                          T_pCallContext  P_callCtxt,
                                          C_MessageFrame *P_msg,
                                          C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;

  T_ValueData         L_val      ;
  T_pValueData        L_value_id   = NULL          ;

  
  C_CallContext::T_pCallMap *L_map = P_callCtxt->m_call_control->get_call_map();
    
  L_val.m_type = E_TYPE_NUMBER ;
  
  if (P_msg -> get_field_value(m_id, 
                               m_instance_id,
                               m_sub_id,
                               &L_val) == true) {

    pair_t<C_CallContext::T_CallMap::iterator,bool> L_pr ;
    C_CallContext::T_contextMapData L_data ;

    L_value_id = &L_val ;
    L_value_id = P_callCtxt->set_id (m_position, L_value_id);

    L_pr = L_map[m_position]
      ->insert(C_CallContext::T_CallMap::value_type(*L_value_id, P_callCtxt));

    L_data.m_channel = m_position ;
    L_data.m_iterator = L_pr.first ;
    (P_callCtxt->m_map_data_list)->push_back(L_data);
    
    resetMemory(L_val);

  } else {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                  action_name_table[m_type] 
                  << ": the value of the field asked is incorrect or not found");

    L_exeCode = E_EXE_ERROR;
  }
  
  return (L_exeCode);
}






