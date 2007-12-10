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

#include "C_IncVarAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"


C_IncVarAction::C_IncVarAction(T_CmdAction        P_cmdAction,
                               T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}

C_IncVarAction::~C_IncVarAction() {
}


T_exeCode    C_IncVarAction::execute(T_pCmd_scenario P_pCmd,
                                    T_pCallContext  P_callCtxt,
                                    C_MessageFrame *P_msg,
                                    C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  static char         L_tmp_string [250]           ;
  T_pValueData        L_mem        = NULL          ;
  size_t              L_current_size               ;

  unsigned long       L_result     = 0             ;
  char               *L_value      = NULL          ;
  int                 L_value_size = 0             ; 
  char               *L_endstr     = NULL          ;


  L_mem = P_callCtxt->get_memory(m_mem_id);

  switch (L_mem->m_type) {
  case E_TYPE_NUMBER:
    L_mem->m_value.m_val_number += m_position ;
    break ;
    
  case E_TYPE_SIGNED:
    L_mem->m_value.m_val_signed += (T_Integer32)m_position ; 
    break ;
    
  case E_TYPE_STRING:
    memset(L_tmp_string, 0, 250) ;
    L_tmp_string[0] = '\0' ;

    L_value_size = L_mem->m_value.m_val_binary.m_size ; 
    ALLOC_TABLE(L_value,
		char*, 
		sizeof(char),
		L_value_size + 1 );

    memcpy(L_value, L_mem->m_value.m_val_binary.m_value , 
           L_mem->m_value.m_val_binary.m_size);
    L_value[L_value_size] = 0 ;
    
    L_result = strtoul_f (L_value, &L_endstr,10) ;
    if (L_endstr[0] != '\0') {
      L_result = strtoul_f (L_value, &L_endstr,16) ;
      if (L_endstr[0] != '\0') {
        GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
                  << L_value << "] bad format");
        L_exeCode    = E_EXE_ERROR ;
        break ;
      }
    }

    L_result += m_position ;
    sprintf(L_tmp_string, "%lu", L_result) ;
    L_current_size = strlen(L_tmp_string);
    FREE_TABLE(L_mem->m_value.m_val_binary.m_value);
    L_mem->m_value.m_val_binary.m_size = L_current_size ;
    ALLOC_TABLE(L_mem->m_value.m_val_binary.m_value,
		unsigned char*,
		sizeof(unsigned char),
		L_mem->m_value.m_val_binary.m_size);

    memcpy(L_mem->m_value.m_val_binary.m_value,
	   (unsigned char*) L_tmp_string,
	   L_mem->m_value.m_val_binary.m_size);
    FREE_TABLE(L_value);    
    break;
    
  case E_TYPE_NUMBER_64:
    L_mem->m_value.m_val_number_64 +=  (T_UnsignedInteger64) m_position ;
    break ;
    
  case E_TYPE_SIGNED_64:
    L_mem->m_value.m_val_signed_64 +=  (T_Integer64) m_position ;
    break ;


  default:
    GEN_FATAL(E_GEN_FATAL_ERROR, "Unsupported type [" 
              << L_mem->m_type 
              << "] for increment function");
    break ;
  }
  return (L_exeCode);
}





