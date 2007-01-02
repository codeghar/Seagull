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

#include "C_SetValueActionMethodExtern.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_ScenarioControl.hpp"


C_SetValueActionMethodExtern::C_SetValueActionMethodExtern(T_CmdAction        P_cmdAction,
                                                           T_pControllers P_controllers)
  : C_SetValueAction (P_cmdAction, P_controllers) {
}


C_SetValueActionMethodExtern::~C_SetValueActionMethodExtern() {

}


T_exeCode    C_SetValueActionMethodExtern::execute(T_pCmd_scenario P_pCmd,
                                                   T_pCallContext  P_callCtxt,
                                                   C_MessageFrame *P_msg,
                                                   C_MessageFrame *P_ref) {
  

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  int                 L_ret        = 0             ;

  T_ValueData         L_mem                        ;
  T_ValueData         L_result                     ;
  T_ValueData         L_value                      ;

  bool                L_reset_value = false        ;


  L_mem = search_memory(P_callCtxt, P_msg, L_reset_value) ;

  
  L_value.m_type = E_TYPE_NUMBER ;
  L_result.m_type = E_TYPE_NUMBER ;

  if (P_msg->get_buffer(&L_value,E_BODY_TYPE) == -1) {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                  action_name_table[m_type] 
                  << ": error in body of message");
    
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                  "error on call with session-id ["
                  << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
    
    L_exeCode = E_EXE_ERROR;
    return (L_exeCode);
  } else {
    L_ret = (*m_external_method)(&L_value, 
                                 &L_mem,
                                 &L_result);
    
    if (L_ret != -1) {
      if (P_msg->set_field_value(&L_result,
                                 m_id,
                                 m_instance_id,
                                 m_sub_id) == false ) {
        
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                      action_name_table[m_type] 
                      << ": problem when set field value");
        
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                      "error on call with session-id ["
                      << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
        
        L_exeCode = E_EXE_ERROR;
      }
    } else {
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                    action_name_table[m_type] 
                    << ": problem when using external method (md5 or AKA)");
      
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                    "error on call with session-id ["
                    << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
      
      L_exeCode = E_EXE_ERROR;
    }
    resetMemory(L_result);
    resetMemory(L_value);
  }

  if (L_reset_value == true) {
    L_mem.m_type = E_TYPE_NUMBER ;
  }
    
  return (L_exeCode);
}









