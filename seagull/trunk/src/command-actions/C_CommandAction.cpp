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

#include "C_CommandAction.hpp"
#include "C_ScenarioControl.hpp"

C_CommandAction::C_CommandAction(T_CmdAction        P_ActionData,
                                 T_pControllers     P_controllers) {

  m_type = P_ActionData.m_type            ;
  m_args = P_ActionData.m_args            ;
  m_id = P_ActionData.m_id              ;
  m_mem_id = P_ActionData.m_mem_id          ;
  m_instance_id = P_ActionData.m_instance_id     ; 
  m_sub_id = P_ActionData.m_sub_id          ; 
  m_begin = P_ActionData.m_begin           ; 
  m_size = P_ActionData.m_size            ; 
  m_pattern = P_ActionData.m_pattern         ;
  m_pattern_size = P_ActionData.m_pattern_size    ;
  m_position = P_ActionData.m_position        ;
  m_string_expr = P_ActionData.m_string_expr     ;
  m_check_behaviour = P_ActionData.m_check_behaviour ;
  m_field_data_num = P_ActionData.m_field_data_num  ;
  m_regexp_data = P_ActionData.m_regexp_data     ;
  m_external_method  = P_ActionData.m_external_method ;


  m_controllers = *P_controllers ;

} 

void C_CommandAction::update_log_controller(C_DataLogControl *P_log) {
  m_controllers.m_log = P_log ;
}


C_CommandAction::~C_CommandAction() {
  DELETE_VAR(m_regexp_data);
}










