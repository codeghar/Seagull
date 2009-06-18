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

#ifndef  _C_COMMANDACTION_H
#define  _C_COMMANDACTION_H


#include "C_MessageFrame.hpp"
#include "C_DataLogControl.hpp"
#include "C_GeneratorStats.hpp"
#include "C_ExternalDataControl.hpp"
#include "C_CallContext.hpp"
#include "C_ChannelControl.hpp"
#include "C_Scenario.hpp"

class C_ScenarioControl ;

typedef struct _controllers_struct {
  C_DataLogControl      *m_log ;
  C_ScenarioControl     *m_scenario_control;
  C_GeneratorStats      *m_stat ;
  C_ExternalDataControl *m_external_data ;
  unsigned int           m_check_mask      ;
  T_CheckBehaviour       m_check_behaviour ;
  C_ChannelControl      *m_channel_ctrl ;
} T_Controllers, *T_pControllers ;

class C_CommandAction { 

public:

  C_CommandAction(T_CmdAction        P_ActionData,
                  T_pControllers     P_controllers) ;

  void update_log_controller(C_DataLogControl *P_log) ;

  virtual ~C_CommandAction() ;

  virtual T_exeCode execute (T_pCmd_scenario P_pCmd,
                             T_pCallContext  P_callCtxt,
                             C_MessageFrame *P_msg,
                             C_MessageFrame *P_ref) = 0 ;

  T_action_type     get_type();

protected:

  T_action_type       m_type            ;
  char               *m_args            ;
  int                 m_id              ;
   int                 m_occurence       ;
  int                 m_look_ahead      ;
  int                 m_look_back       ; 
  int                 m_mem_id          ;
  int                 m_instance_id     ; // instance number 
  int                 m_sub_id          ; // id used when header body access
  int                 m_begin           ; // start of storage
  int                 m_size            ; // size of storage
  char               *m_branch_on       ;
  unsigned char      *m_pattern         ;
  int                 m_pattern_size    ;
  int                 m_position        ;
  T_pStringExpression m_string_expr     ;
  T_CheckBehaviour    m_check_behaviour ;
  int                 m_field_data_num  ;
  C_RegExp           *m_regexp_data     ;
  T_ExternalMethod    m_external_method ;
  T_MessagePartType   m_msg_part_type   ;
  
  T_Controllers       m_controllers     ;
  


} ;

typedef C_CommandAction *T_pC_CommandAction ;


#endif  // _C_COMMANDACTION_H




