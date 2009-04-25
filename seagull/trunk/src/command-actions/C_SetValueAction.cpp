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

#include "C_SetValueAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_ScenarioControl.hpp"



C_SetValueAction::C_SetValueAction(T_CmdAction        P_cmdAction,
                                     T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}


C_SetValueAction::~C_SetValueAction() {

  int L_k ;

  if (m_string_expr != NULL) {
    for(L_k = 0; 
        L_k < m_string_expr->m_nb_portion; L_k++) {
      if (m_string_expr->m_portions[L_k].m_type
          == E_STR_STATIC) {
        FREE_TABLE(m_string_expr->m_portions[L_k].m_data.m_value);
      }
    }
    FREE_TABLE(m_string_expr->m_portions);
    FREE_VAR(m_string_expr);
  }
}

T_ValueData  C_SetValueAction::search_memory(T_pCallContext  P_callCtxt,
                                             C_MessageFrame *P_msg,
                                             bool           &P_reset_value) {

  T_ValueData         L_mem                        ;
  char               *L_values, *L_chr, *L_end_ptr ;
  unsigned long       L_value                      ;
  int                 L_j                          ;
  char               *L_ptr                        ;
  size_t              L_size, L_current_size       ;

  static char         L_tmp_string [250]           ;
  static char         L_tmp_string_filled [250]    ;


///////////////////////////////////
  L_mem.m_type = E_TYPE_NUMBER ;

  L_mem.m_type = P_msg->get_field_type(m_id,0);

  if ((L_mem.m_type == E_TYPE_NUMBER)
    && (m_string_expr->m_portions[0].m_type == E_STR_COUNTER)) {
        L_mem.m_value.m_val_number = m_controllers.m_scenario_control->get_counter_value
          (m_string_expr->m_portions[0].m_data.m_id) ;
    }
  else if ((L_mem.m_type == E_TYPE_SIGNED)
    && (m_string_expr->m_portions[0].m_type == E_STR_COUNTER)) {
        L_mem.m_value.m_val_signed = (T_Integer32) m_controllers.m_scenario_control->get_counter_value
          (m_string_expr->m_portions[0].m_data.m_id) ;
    }
  else if ((L_mem.m_type == E_TYPE_NUMBER_64)
    && (m_string_expr->m_portions[0].m_type == E_STR_COUNTER)) {
        L_mem.m_value.m_val_number_64 = (T_UnsignedInteger64) m_controllers.m_scenario_control->get_counter_value
          (m_string_expr->m_portions[0].m_data.m_id) ;
    }
  else if ((L_mem.m_type == E_TYPE_SIGNED_64)
    && (m_string_expr->m_portions[0].m_type == E_STR_COUNTER)) {
    L_mem.m_value.m_val_signed_64 = (T_Integer64) m_controllers.m_scenario_control->get_counter_value
      (m_string_expr->m_portions[0].m_data.m_id) ;
    }
  else if ((L_mem.m_type == E_TYPE_STRING)  ||
           (L_mem.m_type == E_TYPE_NUMBER)  ||
           (L_mem.m_type == E_TYPE_SIGNED)  ||
           (L_mem.m_type == E_TYPE_NUMBER_64)  ||
           (L_mem.m_type == E_TYPE_SIGNED_64)  ) {
    L_mem.m_type = E_TYPE_STRING ;
    L_tmp_string[0] = '\0' ;
    L_ptr = L_tmp_string ;
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
		// Trying to remove '\r' inserted in between two concatenated values, if first value is integer. (useful in case of set-value with a format))
		if((m_string_expr->m_nb_portion == 2) && (L_j == 0) && ((*(L_ptr -2) >= '0') && (*(L_ptr -2) <= '9') )){
        *(L_ptr -1) = ' ';
      }
      }
      break ;
      }
    }
    L_mem.m_value.m_val_binary.m_size = L_size ;
    L_mem.m_value.m_val_binary.m_value
      = (unsigned char *) L_tmp_string;


    if (m_size != -1) {
      if (m_size > (int)L_size) {
        int L_filled_size = m_size - L_size ;
        int L_fill_idx = 0 ;
        memcpy(L_tmp_string_filled+L_filled_size,
               L_tmp_string,
               L_size);

        while ((L_fill_idx+(m_pattern_size)) > L_filled_size) {
          memcpy(L_tmp_string_filled+L_fill_idx,
                 m_pattern,
                 m_pattern_size);
          L_fill_idx += m_pattern_size ;
        }
        L_mem.m_value.m_val_binary.m_size = m_size ;
        L_mem.m_value.m_val_binary.m_value
          = (unsigned char *) L_tmp_string_filled;

      }
    }

    P_reset_value = true ;

    }
    else if ((L_mem.m_type == E_TYPE_STRUCT) )  {

    P_msg -> get_field_value(m_id, m_occurence,
                             0,0,
                             &L_mem);
    switch (m_string_expr->m_nb_portion) {
    case 1:
      L_values = m_string_expr->m_portions[0].m_data.m_value ;
      L_chr = strrchr(L_values, ';');
      if (L_values == L_chr) {
        // L_mem.m_value.m_val_struct.m_id_1 not setted
        if (*(L_chr+1) != '\0') {
          L_value = strtoul_f (L_chr+1, &L_end_ptr, 10);
          if (L_end_ptr[0] == '\0') { // good format
            L_mem.m_value.m_val_struct.m_id_2
              = L_value ;
          }
        }

      } else {
        *L_chr = '\0';
        L_value = strtoul_f (L_values, &L_end_ptr, 10);
        if (L_end_ptr[0] == '\0') { // good format
          L_mem.m_value.m_val_struct.m_id_1
            = L_value ;
        }
        if (*(L_chr+1) != '\0') {
          L_value = strtoul_f (L_chr+1, &L_end_ptr, 10);
          if (L_end_ptr[0] == '\0') { // good format
            L_mem.m_value.m_val_struct.m_id_2
              = L_value ;
          }
        }
        *L_chr = ';' ;
      }
      break ;

    case 2:
      if (m_string_expr->m_portions[0].m_type == E_STR_COUNTER){
        L_mem.m_value.m_val_struct.m_id_1 = m_controllers.m_scenario_control->get_counter_value
          (m_string_expr->m_portions[0].m_data.m_id) ;
        L_values = m_string_expr->m_portions[1].m_data.m_value ;
        L_chr = strrchr(L_values, ';');
        if (*(L_chr+1) != '\0') {
          L_value = strtoul_f (L_chr+1, &L_end_ptr, 10);
          if (L_end_ptr[0] == '\0') { // good format
            L_mem.m_value.m_val_struct.m_id_2
              = L_value ;
          }
        }

      } else {

        L_values = m_string_expr->m_portions[0].m_data.m_value ;
        L_chr = strrchr(L_values, ';');
        if (L_values != L_chr) {
          *L_chr = '\0' ;
          L_value = strtoul_f (L_values, &L_end_ptr, 10);
          if (L_end_ptr[0] == '\0') { // good format
            L_mem.m_value.m_val_struct.m_id_1
              = L_value ;
          }
          *L_chr = ';' ;
        }
        L_mem.m_value.m_val_struct.m_id_2 = m_controllers.m_scenario_control->get_counter_value
          (m_string_expr->m_portions[1].m_data.m_id) ;

      }

      break ;
    case 3:
      L_mem.m_value.m_val_struct.m_id_1 = m_controllers.m_scenario_control->get_counter_value
        (m_string_expr->m_portions[0].m_data.m_id) ;
      L_mem.m_value.m_val_struct.m_id_2 = m_controllers.m_scenario_control->get_counter_value
        (m_string_expr->m_portions[2].m_data.m_id) ;
    }
    }
  else {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Unsupported type for action execution");
  }
  return (L_mem);
}


T_exeCode    C_SetValueAction::execute(T_pCmd_scenario P_pCmd,
                                       T_pCallContext  P_callCtxt,
                                       C_MessageFrame *P_msg,
                                       C_MessageFrame *P_ref) {
  




  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  T_ValueData         L_mem                        ;
  bool                L_reset_value = false        ;

  L_mem = search_memory(P_callCtxt, P_msg, L_reset_value) ;


  
  if (P_msg->set_field_value(&L_mem, 
                             m_id, m_occurence,
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
  
  if (L_reset_value == true) {
    L_mem.m_type = E_TYPE_NUMBER ;
  }
  
  return (L_exeCode);
}





