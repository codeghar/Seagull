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

#include "C_GetExternalDataActionToMem.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

C_GetExternalDataActionToMem::C_GetExternalDataActionToMem(T_CmdAction        P_cmdAction,
                                                           T_pControllers P_controllers)
  : C_GetExternalDataAction(P_cmdAction, P_controllers) {
}

C_GetExternalDataActionToMem::~C_GetExternalDataActionToMem() {
}


T_exeCode    C_GetExternalDataActionToMem::execute(T_pCmd_scenario P_pCmd,
                                              T_pCallContext  P_callCtxt,
                                              C_MessageFrame *P_msg,
                                              C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  T_pValueData        L_value      = NULL ;
  T_pValueData        L_mem        = NULL ;

  
  P_callCtxt->reset_memory(m_mem_id) ;
    
  L_mem = P_callCtxt->get_memory(m_mem_id);
      
  L_value = m_controllers.m_external_data->get_value(P_callCtxt->m_selected_line, 
                                       m_field_data_num);

  if (L_value != NULL) {
      L_mem->m_type = E_TYPE_NUMBER ; 
      copyValue(*L_mem,*L_value, false);
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







