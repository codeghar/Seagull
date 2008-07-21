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

#ifndef _C_SCENARIO_H
#define _C_SCENARIO_H

#include "iostream_t.hpp"
#include "list_t.hpp"
#include "map_t.hpp"
#include "string_t.hpp"

#include "C_MessageFrame.hpp"

#include "C_CallContext.hpp"

#include "C_ProtocolFrame.hpp"

#include "C_GeneratorStats.hpp"
#include "C_DataLogControl.hpp"

#include "C_GeneratorConfig.hpp"

#include "C_ChannelControl.hpp"

#include "C_ExternalDataControl.hpp"
#include "C_ScenarioStats.hpp"

#include "C_RegExp.hpp"
#include "ExternalMethod.h"

#include "message_header_body_t.h"
#include "exe_code_t.h"

typedef enum _action_scenario_type {
   E_ACTION_SCEN_OPEN= 0,
   E_ACTION_SCEN_CLOSE,
   E_ACTION_SCEN_MEMORY_STORE,
   E_ACTION_SCEN_MEMORY_RESTORE,
   E_ACTION_SCEN_START_TIMER,
   E_ACTION_SCEN_STOP_TIMER,
   E_ACTION_SCEN_SET_VALUE,
   E_ACTION_SCEN_INC_COUNTER,
   E_ACTION_SCEN_CHECK_PRESENCE,
   E_ACTION_SCEN_GET_EXTERNAL_DATA,
   E_ACTION_SCEN_INC_VAR,
   E_ACTION_SCEN_CHECK_VALUE,
   E_ACTION_SCEN_CHECK_ORDER,
   E_ACTION_SCEN_SET_NEW_SESSION_ID,
   E_ACTION_SCEN_TRANSPORT_OPTION,
   E_ACTION_SCEN_SET_BIT,
   E_ACTION_SCEN_SET_VALUE_BIT,
   E_ACTION_SCEN_INSERT_IN_MAP,
   E_ACTION_SCEN_LOG,

   E_NB_ACTION_SCEN,  // internal actions after this value
   E_ACTION_SCEN_INTERNAL_INIT_DONE,
   E_ACTION_SCEN_CHECK_ALL_MSG,
   E_ACTION_SCEN_ADD_IN_CALL_MAP,
   E_ACTION_SCEN_SELECT_EXTERNAL_DATA_LINE,
   E_ACTION_SCEN_GET_EXTERNAL_DATA_TO_MEM,
   E_ACTION_SCEN_SET_VALUE_METHOD_EXTERN,
   E_ACTION_SCEN_INSERT_IN_MAP_FROM_MEM,
   E_ACTION_SCEN_ADD_DEFAULT_IN_CALL_MAP

}  T_action_type,
  *T_pAction_type ;

typedef enum _enum_string_type {
  E_STR_STATIC,
  E_STR_COUNTER,
  E_STR_MEMORY
} T_ValueStringType ;

typedef struct _union_string_data {
  int   m_id ;
  char *m_value ;
} T_ValueStringData, *T_pValueStringData ;

typedef struct _struct_string_portion {
  T_ValueStringType m_type ;
  T_ValueStringData m_data ;
} T_StringValue, *T_pStringValue ;

typedef struct _struct_string_express {
  int            m_nb_portion ;
  T_pStringValue m_portions ;
} T_StringExpression, *T_pStringExpression ;

typedef struct _xml_cmd_action {
  T_action_type       m_type            ;
  char               *m_args            ;
  int                 m_id              ;
  int                 m_occurence       ;
  int                 m_mem_id          ;
  int                 m_instance_id     ; // instance number 
  int                 m_sub_id          ; // id used when header body access
  int                 m_begin           ; // start of storage
  int                 m_size            ; // size of storage
  unsigned char      *m_pattern         ;
  int                 m_pattern_size    ;
  int                 m_position        ;
  T_pStringExpression m_string_expr     ;
  T_CheckBehaviour    m_check_behaviour ;
  int                 m_field_data_num  ;
  C_RegExp           *m_regexp_data     ;
  T_ExternalMethod    m_external_method ;
  T_MessagePartType   m_msg_part_type   ;
} T_CmdAction, *T_pCmdAction ;

typedef list_t<T_pCmdAction> T_CmdActionList, 
                               *T_pCmdActionList ;


typedef enum _enum_behaviour_scenario {

  E_BEHAVIOUR_SCEN_SUCCESS,
  E_BEHAVIOUR_SCEN_FAILED,
  E_BEHAVIOUR_SCEN_IGNORE
}  T_BehaviourScenario, 
  *T_pBehaviourScenario ;


typedef enum _cmd_scenario_type {
   E_CMD_SCEN_SEND = 0,
   E_CMD_SCEN_RECEIVE,
   E_CMD_SCEN_WAIT,
   E_CMD_SCEN_END,
   E_NB_CMD_SCEN
}  T_cmd_type,
  *T_pCmd_type ;


extern const char* command_name_table [] ;
extern const char* action_name_table [] ;

typedef struct _action_regexp_str_t {
  // char *m_name ;
  char *m_expr      ;
  int   m_nb_match  ; 
  int   m_sub_match ; 
  int   m_line      ;     
} T_ActionRegExpStr, *T_pActionRegExpStr ;

class C_CommandAction ;

typedef struct _xml_cmd_scenario {

  T_cmd_type           m_type           ;
  C_MessageFrame      *m_message        ;
  unsigned long        m_duration       ;
  unsigned long        m_duration_index ;

  int                  m_channel_id     ;

  int                  m_pre_action     ;
  int                  m_post_action    ;

  C_CommandAction**    m_pre_action_table ;
  C_CommandAction**    m_post_action_table ;

  unsigned long        m_retrans_delay  ;
  unsigned long        m_retrans_index  ;
  unsigned long        m_retrans_delay_index  ;
  unsigned long        m_id             ;

}  T_cmd_scenario,
  *T_pCmd_scenario ;

class C_ScenarioControl ;

class C_Scenario {

public:

  C_Scenario(C_ScenarioControl     *P_scenario_control,
	     C_ChannelControl      *P_channel_control,
	     C_ExternalDataControl *P_external_data_control,
	     T_exeCode              P_exe_code,	     
	     char                  *P_behaviour,
             bool                   P_retrans_enabled,
	     unsigned int           P_check_mask     = 0, 
	     T_CheckBehaviour       P_checkBehaviour 
	     = E_CHECK_BEHAVIOUR_WARNING
	     );
  ~C_Scenario();

  void set_size (size_t P_size);

  void set_data_log_controller (C_DataLogControl *P_log);

  size_t add_cmd  (T_cmd_type          P_type,
		   int                 P_channel_id,
		   T_pC_MessageFrame   P_msg,
		   int                 P_nb_pre_action,
		   C_CommandAction**   P_pre_act_table,
                   unsigned long       P_retrans_delay);


  // define post actions for the last command added
  size_t define_post_actions (int          P_nb_post_action,
			      C_CommandAction** P_post_act_table) ;
  size_t define_pre_actions (int          P_nb_pre_action,
			     C_CommandAction** P_pre_act_table) ;

  void delete_post_actions (int P_cmd_index);
  void delete_pre_actions  (int P_cmd_index) ;

  void update_post_actions (int               P_nb_post_action,
                            C_CommandAction** P_post_act_table,
                            int               P_cmd_index) ;

  void update_pre_actions  (int               P_nb_pre_action,
                            C_CommandAction** P_pre_act_table,
                            int               P_cmd_index) ;

  size_t add_cmd  (T_cmd_type    P_type,
		   unsigned long P_duration);
  size_t add_cmd  (T_cmd_type    P_type);

  bool check_msg_received (T_pReceiveMsgContext P_rcvMsg) ;

  T_exeCode          execute_cmd (T_pCallContext P_callCtxt, 
                                  bool P_resume);

  T_CallContextState first_state();

  T_exeCode          execute_cmd_retrans (int P_index, T_pCallContext P_callCtxt);
  void update_retrans_delay_cmd (size_t P_nb, unsigned long *P_table) ;

  void update_wait_cmd (size_t P_nb, unsigned long *P_table) ;

  friend iostream_output& operator<<(iostream_output&, C_Scenario&);

  void set_stats(C_ScenarioStats *P_scenStat) ;
  C_ScenarioStats* get_stats();
  void delete_stats () ;

  T_exeCode    get_exe_end_code()  ;

  int                  get_nb_retrans();

  int                  get_nb_recv_per_scen () ;

  int                  get_nb_send_per_scen () ;



  T_pCmd_scenario      get_commands() ;

  friend class C_ScenarioStats ;

private:

  typedef map_t<string_t, int> T_MemoryIdMap, *T_pMemoryIdMap ; 

  int              m_sequence_max  ;
  int              m_sequence_size ;
  T_pCmd_scenario  m_cmd_sequence  ;
  C_GeneratorStats *m_stat          ;

  C_DataLogControl *m_log            ;
  C_ExternalDataControl  *m_external_data ;

  unsigned int     m_check_mask      ;
  T_CheckBehaviour m_check_behaviour ;

  C_ScenarioControl *m_controller    ;
  C_ChannelControl  *m_channel_ctrl  ;
  C_ScenarioStats   *m_stats         ;
  T_exeCode          m_exe_end_code  ;


  T_exeCode          execute_action (T_pCmd_scenario      P_pCmd,
				     T_pCallContext       P_callCtxt,
				     C_MessageFrame      *P_msg,
				     int                  P_nbActions,
				     C_CommandAction**    P_actions,
				     C_MessageFrame      *P_ref);


  T_BehaviourScenario m_behaviour ;

  bool                  m_retrans_enabled  ;
  int                   m_nb_retrans            ;


  int                   m_nb_send_per_scene   ;
  int                   m_nb_recv_per_scene   ;

} ;

typedef C_Scenario *T_pC_Scenario ;

#endif // _C_SCENARIO_H






