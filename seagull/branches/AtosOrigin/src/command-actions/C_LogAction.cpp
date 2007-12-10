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

#include "C_LogAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_ScenarioControl.hpp"
#include "BufferUtils.hpp"


C_LogAction::C_LogAction(T_CmdAction        P_cmdAction,
			 T_pControllers     P_controllers)
  : C_SetValueAction (P_cmdAction, P_controllers) {
}


C_LogAction::~C_LogAction() {
}


T_ValueData    C_LogAction::retrieve_memory(T_pCallContext  P_callCtxt) {

  static char  L_string_mem [250]           ;
  T_ValueData  L_mem                        ;
  int          L_j                          ;
  char        *L_ptr                        ;
  size_t       L_size, L_current_size       ;

  L_string_mem[0] = '\0' ;
  L_ptr = L_string_mem ;
  L_size = 0 ;
  for (L_j = 0 ;
       L_j < m_string_expr->m_nb_portion;
       L_j ++) {
    switch (m_string_expr->m_portions[L_j].m_type) {
    case E_STR_STATIC:
      strcpy(L_ptr, 
	     m_string_expr->m_portions[L_j].m_data.m_value);
      L_current_size = strlen(L_ptr) ;
      L_size += L_current_size ;
      L_ptr += L_current_size ;
      break ;
    case E_STR_COUNTER:
      sprintf(L_ptr, "%ld", 
	      m_controllers.m_scenario_control->get_counter_value
	      (m_string_expr
	       ->m_portions[L_j].m_data.m_id)) ;
      L_current_size = strlen(L_ptr);
      L_size += L_current_size;
      L_ptr += L_current_size;
      break ;
      
    case E_STR_MEMORY: {
      T_pValueData L_memory ;
      L_memory = P_callCtxt->get_memory(m_string_expr
					->m_portions[L_j].m_data.m_id);
      L_current_size = 250 - L_size ;
      valueToString(*L_memory, L_ptr, L_current_size);
      L_size += L_current_size ;
      L_ptr += L_current_size ;
    }
      break ;
    }
  }
  L_mem.m_value.m_val_binary.m_size = L_size ;
  L_mem.m_value.m_val_binary.m_value 
    = (unsigned char *) L_string_mem;

  return (L_mem);
}


void C_LogAction::print_data(T_pValueData  P_data) {
  
  static char L_ascii_buf [50] ;
  static char L_line_buf [100] ;
  const  size_t L_cNum = 48 ; 
  size_t L_j, L_nb ;
  unsigned char*L_cur ;
  size_t L_buffer_size = P_data->m_value.m_val_binary.m_size;
  
  L_nb = L_buffer_size / L_cNum ;
  L_cur = P_data->m_value.m_val_binary.m_value ;
  if (L_cur != NULL) {
    for (L_j = 0 ; L_j < L_nb; L_j++) {
      pretty_ascii_printable_buffer(L_cur, L_cNum, L_ascii_buf);
      sprintf(L_line_buf, "[%-48.48s] ", L_ascii_buf);
      GEN_LOG_EVENT (LOG_LEVEL_USER,
		     L_line_buf);
      L_cur += L_cNum ;
    }

    L_nb = L_buffer_size % L_cNum ;
    if (L_nb != 0) {
      pretty_ascii_printable_buffer(L_cur, L_nb, L_ascii_buf);
      sprintf(L_line_buf, "[%-48.48s] ",  L_ascii_buf);
      GEN_LOG_EVENT (LOG_LEVEL_USER,
		     L_line_buf);
    }
  } else {
    GEN_LOG_EVENT (LOG_LEVEL_USER,"NULL(empty)");
  }
  
  GEN_DEBUG(1, "C_ProtocolText::print_data() end");

}



T_exeCode    C_LogAction::execute(T_pCmd_scenario P_pCmd,
				  T_pCallContext  P_callCtxt,
				  C_MessageFrame *P_msg,
				  C_MessageFrame *P_ref) {
  
  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  T_ValueData         L_mem                        ;

  L_mem = retrieve_memory(P_callCtxt);
  print_data(&L_mem);
  
  return (L_exeCode);
}





