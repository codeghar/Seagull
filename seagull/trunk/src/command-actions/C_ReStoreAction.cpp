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

#include "C_ReStoreAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"


C_ReStoreAction::C_ReStoreAction(T_CmdAction        P_cmdAction,
                                 T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}


C_ReStoreAction::~C_ReStoreAction() {
}


T_exeCode    C_ReStoreAction::execute(T_pCmd_scenario P_pCmd,
                                      T_pCallContext  P_callCtxt,
                                      C_MessageFrame *P_msg,
                                      C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;

  T_pValueData        L_mem                        ;
  int                 L_size                       ;
  
  GEN_DEBUG(2, "restore memory id = " << m_mem_id);
  GEN_DEBUG(2, "restore field id = "  << m_id);
  
  L_mem = P_callCtxt->get_memory(m_mem_id);
  
  L_size = m_size ;
  
  if (L_size != -1) {
    T_ValueData           L_val ;
    L_val.m_type = E_TYPE_NUMBER ;
    
    if (L_mem->m_value.m_val_binary.m_size == 0) {
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                    action_name_table[m_type] 
                    << " memory zone that was not previously stored");

      return (L_exeCode);
    }
    
    // RETRIEVE the data 
    if (P_msg -> get_field_value(m_id, 
				 m_instance_id,
				 m_sub_id,
                                 &L_val) == true) {
      
          if (L_size > (int)L_val.m_value.m_val_binary.m_size) {
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                          action_name_table[m_type] 
                          << ": the size desired  ["
                          << L_size << "] exceed the length of buffer ["
                          << L_val.m_value.m_val_binary.m_size << "]");
            L_size = L_val.m_value.m_val_binary.m_size ;
          }

          copyBinaryVal(L_val, m_begin, L_size,
                        *L_mem);

          if (P_msg -> set_field_value(&L_val, 
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
          resetMemory(L_val);
        } else {
	  GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                        action_name_table[m_type] 
                        << ": the value of the field asked is incorrect or not found");
	  
	  GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
			"error on call with session-id ["
			<< P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
          
	  L_exeCode = E_EXE_ERROR;
        }
      } else {
        if (P_msg -> set_field_value(L_mem, 
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
      }

  return (L_exeCode);
}





