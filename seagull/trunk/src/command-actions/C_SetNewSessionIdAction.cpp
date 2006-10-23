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

#include "C_SetNewSessionIdAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_CallControl.hpp"

C_SetNewSessionIdAction::C_SetNewSessionIdAction(T_CmdAction        P_cmdAction,
                                                 T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}

C_SetNewSessionIdAction::~C_SetNewSessionIdAction() {
}


T_exeCode    C_SetNewSessionIdAction::execute(T_pCmd_scenario P_pCmd,
                                              T_pCallContext  P_callCtxt,
                                              C_MessageFrame *P_msg,
                                              C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;

  T_pValueData          L_mem      ;
  T_ValueData           L_val      ;
  
  T_CallMap::iterator   L_call_it  ;
  T_pValueData          L_value_id ;
  
  T_pCallMap *L_map = P_callCtxt->m_call_control->get_call_map();
    
  L_val.m_type = E_TYPE_NUMBER ;
  
  L_mem = P_callCtxt->get_memory(m_mem_id);
  if (L_mem->m_value.m_val_binary.m_size == 0) {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                  action_name_table[m_type] 
                  << " action: Memory zone used without data stored" );
    //        L_exeCode = E_EXE_ERROR;
    // break ;
  }
  
  if (P_msg -> get_field_value(m_id, 
                               m_instance_id,
                               m_sub_id,
                               &L_val) == true) {
    if (*L_mem == L_val ) {
      //break;
    } else {
      L_call_it = L_map[P_pCmd->m_channel_id]
        ->find (T_CallMap::key_type(P_callCtxt->m_id_table[P_pCmd->m_channel_id]));
      if (L_call_it != L_map[P_pCmd->m_channel_id]->end()) {
        L_map[P_pCmd->m_channel_id]->erase (L_call_it);
        P_callCtxt->reset_id (P_pCmd->m_channel_id);
        L_value_id = P_callCtxt->set_id (P_pCmd->m_channel_id,&L_val);
        L_map[P_pCmd->m_channel_id]
          ->insert(T_CallMap::value_type(*L_value_id, P_callCtxt));
        resetMemory(L_val);
      } 
    }
    
  } else {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                  action_name_table[m_type] 
                  << ": the value of the field asked is incorrect or not found");
    
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                  "error on call with session-id ["
                  << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
    
    L_exeCode = E_EXE_ERROR;
  }
  
  
  return (L_exeCode);
}





