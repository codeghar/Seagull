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

#include "C_Scenario.hpp"
#include "Utils.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "BufferUtils.hpp"
#include "TimeUtils.hpp"
#include "C_CommandAction.hpp"

#include "C_ScenarioControl.hpp"

#include "integer_t.hpp" // For strtoul_f

  

const char* command_name_table [] = {
  "send",
  "receive",
  "wait-ms",
  "end"} ;

const char* action_name_table [] = {
  "open",
  "close",
  "store",
  "restore",
  "start-timer",
  "stop-timer",
  "set-value",
  "inc-counter",
  "check-presence",
  "restore-from-external",
  "inc-var",
  "check-value",
  "check-order",
  "set-new-session-id",
  "transport-option",
  "set-bit",
  "set-value-bit",
  "insert-in-map",
  "log",
  "E_NB_ACTION_SCEN",  // internal actions after this value
  "E_ACTION_SCEN_INTERNAL_INIT_DONE",
  "E_ACTION_SCEN_CHECK_ALL_MSG",
  "E_ACTION_SCEN_ADD_IN_CALL_MAP",
  "E_ACTION_SCEN_SELECT_EXTERNAL_DATA_LINE",
  "E_ACTION_SCEN_GET_EXTERNAL_DATA_TO_MEM",
  "E_ACTION_SCEN_SET_VALUE_METHOD_EXTERN",
  "E_ACTION_SCEN_INSERT_IN_MAP_FROM_MEM",
  "E_ACTION_SCEN_ADD_DEFAULT_IN_CALL_MAP"

} ;


C_Scenario::C_Scenario (C_ScenarioControl     *P_scenario_control,
			C_ChannelControl      *P_channel_control,
			C_ExternalDataControl *P_external_data_control,
			T_exeCode              P_exe_code,
			char                  *P_behaviour,
                        bool                   P_retrans_enabled,
			unsigned int           P_check_mask, 
			T_CheckBehaviour       P_checkBehaviour
			) {

  GEN_DEBUG(1, "C_Scenario::C_Scenario() start");
  m_sequence_max = 0 ;
  m_sequence_size = 0 ;
  m_cmd_sequence = NULL ;
  m_stat = C_GeneratorStats::instance() ;
  m_check_mask = P_check_mask ;
  m_check_behaviour = P_checkBehaviour ;
  m_log = NULL ;
  m_channel_ctrl = P_channel_control ;
  m_controller = P_scenario_control ;
  m_external_data = P_external_data_control ;
  m_stats  = NULL ;

  m_exe_end_code = P_exe_code ;
  m_retrans_enabled        = P_retrans_enabled ;
  m_nb_retrans = 0 ;

  m_nb_send_per_scene = 0 ;
  m_nb_recv_per_scene = 0 ;

  if (P_behaviour == NULL) {
    m_behaviour = E_BEHAVIOUR_SCEN_SUCCESS ;
  } else {
    if (strcmp(P_behaviour,(char*)"failed") == 0) {
      m_behaviour = E_BEHAVIOUR_SCEN_FAILED ;
      m_exe_end_code = E_EXE_ERROR ;
    } else if (strcmp(P_behaviour,(char*)"ignore") == 0) { 
      m_behaviour = E_BEHAVIOUR_SCEN_IGNORE ;
      m_exe_end_code = E_EXE_IGNORE ;
    } else {
      m_behaviour = E_BEHAVIOUR_SCEN_SUCCESS ;
    }
  }

  GEN_DEBUG(1, "C_Scenario::C_Scenario() end");

}

C_Scenario::~C_Scenario () {

  int          L_i;
  int          L_j;
  GEN_DEBUG(1, "C_Scenario::~C_Scenario() start seq nb: " << m_sequence_max);

  if (m_sequence_max) {

    for (L_i = 0; L_i < m_sequence_max; L_i++) {

      // Do not delete sequence message here 
      // this is made during specific protocol clean

      if (m_cmd_sequence[L_i].m_pre_action != 0) {
	for (L_j = 0 ; L_j < m_cmd_sequence[L_i].m_pre_action; L_j ++) {
          DELETE_VAR(m_cmd_sequence[L_i].m_pre_action_table[L_j]);
	}
        FREE_TABLE(m_cmd_sequence[L_i].m_pre_action_table);
      }

      if (m_cmd_sequence[L_i].m_post_action != 0) {
	for (L_j = 0 ; L_j < m_cmd_sequence[L_i].m_post_action; L_j ++) {
          DELETE_VAR(m_cmd_sequence[L_i].m_post_action_table[L_j]);
	}
        FREE_TABLE(m_cmd_sequence[L_i].m_post_action_table);
      }

    }
  }
  FREE_TABLE(m_cmd_sequence);
  m_sequence_max = 0 ;
  m_sequence_size = 0 ;
  m_stat = NULL ;
  m_stats = NULL ;

  m_nb_retrans = 0 ;

  m_nb_send_per_scene = 0 ;
  m_nb_recv_per_scene = 0 ;


  GEN_DEBUG(1, "C_Scenario::~C_Scenario() end");
}

void C_Scenario::set_data_log_controller (C_DataLogControl *P_log) {
  int L_i, L_j ;
  GEN_DEBUG(1, "C_Scenario::set_data_log_controller start");
  m_log = P_log ;

  for(L_i=0; L_i < m_sequence_max; L_i++) {
    for (L_j = 0 ; L_j < m_cmd_sequence[L_i].m_pre_action; L_j ++) {
      m_cmd_sequence[L_i].m_pre_action_table[L_j]
        ->update_log_controller(P_log);
    }
    for (L_j = 0 ; L_j < m_cmd_sequence[L_i].m_post_action; L_j ++) {
      m_cmd_sequence[L_i].m_post_action_table[L_j]
        ->update_log_controller(P_log);
    }
  }
  GEN_DEBUG(1, "C_Scenario::set_data_log_controller m_log is" << m_log);
  GEN_DEBUG(1, "C_Scenario::set_data_log_controller end");
}

T_CallContextState C_Scenario::first_state () {
  T_CallContextState L_ret ;

  GEN_DEBUG(1, "C_Scenario::first_state() start");

  switch(m_cmd_sequence[0].m_type) {
  case E_CMD_SCEN_SEND:
    L_ret = E_CTXT_SEND ;
    break ;
  case E_CMD_SCEN_RECEIVE:
    L_ret = E_CTXT_RECEIVE ;
    break ;
  case E_CMD_SCEN_WAIT:
    L_ret = 
      E_CTXT_WAIT+m_cmd_sequence[0].m_duration_index ;
    break ; 
  default:
    L_ret = E_CTXT_NBSTATE ;
    break ;
  }
  GEN_DEBUG(1, "C_Scenario::first_state() end");
  return (L_ret);
}

T_exeCode C_Scenario::execute_cmd_retrans (int P_index, T_pCallContext P_callCtxt) {
  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  int                 L_cmdIdx                     ;
  T_pCmd_scenario     L_pCmd                       ;
  
  struct timezone     L_timeZone                   ;
  
  C_MessageFrame     *L_retransMsg = NULL          ;
  int                 L_channel_id = -1            ;

  GEN_DEBUG(1, "C_Scenario::execute_cmd_retrans() start");

  // retrieve command execution informations
  L_cmdIdx = P_callCtxt->m_retrans_cmd_idx[P_index];
  L_pCmd = &m_cmd_sequence[L_cmdIdx] ;

  switch (L_pCmd->m_type) {

  case E_CMD_SCEN_SEND: {
    
    L_channel_id = L_pCmd -> m_channel_id ;
    L_retransMsg = P_callCtxt->m_retrans_msg[P_index] ;
      
    if (m_channel_ctrl->
        send_to_channel(L_channel_id, P_callCtxt->m_channel_table, L_retransMsg)) {
      
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                    "Send error on call with session-id ["
                    << P_callCtxt->m_id_table[L_channel_id] << "]");
      
      L_exeCode = E_EXE_ERROR_SEND ;
    } else {
      // to modify use a new counter 
      m_stat -> executeStatAction (C_GeneratorStats::E_SEND_MSG);
      gettimeofday(&(P_callCtxt->m_retrans_time[P_index]), &L_timeZone) ;
      (P_callCtxt->m_nb_retrans_done[P_index]) ++;
      if (m_stats) {
        m_stats->updateStats(L_cmdIdx,C_ScenarioStats::E_RETRANS,0);
        // m_stats->updateStats(L_cmdIdx,C_ScenarioStats::E_MESSAGE,0);

      }
    }
  }
  break ;
  default:
    GEN_DEBUG (1, "C_Scenario::execute_cmd() Incorrect type "
               << L_pCmd->m_type
               << " command execution");
    L_exeCode = E_EXE_ERROR;
    break ;
  }
  
  GEN_DEBUG(1, "C_Scenario::execute_cmd_retrans() end");
  return (L_exeCode);
}


T_exeCode C_Scenario::execute_cmd (T_pCallContext P_callCtxt,
                                   bool P_resume) {

  T_exeCode           L_exeCode = E_EXE_NOERROR ;
  int                 L_cmdIdx  ;
  T_pCmd_scenario     L_pCmd ;

  struct timezone     L_timeZone ;

  C_MessageFrame                  *L_sendMsg = NULL ;
  int                              L_channel_id = -1    ;


  GEN_DEBUG(1, "C_Scenario::execute_cmd() start");

  // retrieve command execution informations
  L_cmdIdx = P_callCtxt->get_current_cmd_idx();
  L_pCmd = &m_cmd_sequence[L_cmdIdx] ;


  switch (L_pCmd->m_type) {

  case E_CMD_SCEN_SEND: {


    GEN_DEBUG (1, "C_Scenario::execute_cmd() send command execution");

    if (P_resume == false) {

      L_channel_id = L_pCmd -> m_channel_id ;

      L_sendMsg = (m_channel_ctrl->get_channel_protocol(L_channel_id))
	->create_new_message(L_pCmd->m_message);

      // now execution of the pre-actions
      if (L_pCmd->m_pre_action != 0) {
	L_exeCode = execute_action(L_pCmd, P_callCtxt, 
				   L_sendMsg, 
				   L_pCmd->m_pre_action,
				   L_pCmd->m_pre_action_table,
				   L_pCmd->m_message,false) ;
      }
    } else {
      L_channel_id = L_pCmd -> m_channel_id ;
      L_sendMsg = P_callCtxt->m_suspend_msg ;
    }

    if ((L_exeCode == E_EXE_NOERROR) && (L_exeCode != E_EXE_SUSPEND)) {  

      if (m_channel_ctrl->send_to_channel(L_channel_id, P_callCtxt->m_channel_table, L_sendMsg)) {

	GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		      "Send error on call with session-id ["
		      << P_callCtxt->m_id_table[L_channel_id] << "]");

	L_exeCode = E_EXE_ERROR_SEND ;
      } else {
	m_stat -> executeStatAction (C_GeneratorStats::E_SEND_MSG);
	gettimeofday(&P_callCtxt->m_current_time, &L_timeZone) ;
	
	if (m_stats) {
	  m_stats->updateStats(L_cmdIdx,C_ScenarioStats::E_MESSAGE,0);
	}


        if (m_retrans_enabled) {
          if (L_pCmd->m_retrans_delay > 0) {
            P_callCtxt->m_retrans_context.m_retrans_index = L_pCmd->m_retrans_index ;
            P_callCtxt->m_retrans_context.m_retrans_delay_index = L_pCmd->m_retrans_delay_index ;
            P_callCtxt->m_retrans_context.m_context = P_callCtxt ;
            P_callCtxt->m_retrans_time[L_pCmd->m_retrans_index] = P_callCtxt->m_current_time ;
            P_callCtxt->m_retrans_cmd_idx[L_pCmd->m_retrans_index] = L_cmdIdx;
            P_callCtxt->m_retrans_msg[L_pCmd->m_retrans_index] = L_sendMsg ;
            P_callCtxt->m_retrans_to_do = true ;
          }
        }
      }
    }

    if ((L_exeCode == E_EXE_NOERROR) && (L_exeCode != E_EXE_SUSPEND)) {  
      // now execute post actions
      if (L_pCmd->m_post_action != 0) {
	L_exeCode = execute_action(L_pCmd, P_callCtxt, 
				   L_sendMsg, 
				   L_pCmd->m_post_action,
				   L_pCmd->m_post_action_table,
				   L_pCmd->m_message,false) ;
      }
    }


    if (L_exeCode != E_EXE_SUSPEND) {
      if ((m_retrans_enabled) && (L_pCmd->m_retrans_delay > 0)) {
        L_sendMsg = NULL ;
      } else {
        DELETE_VAR(L_sendMsg);
      }
    }
  }
    
    break ;

  case E_CMD_SCEN_RECEIVE: {

    C_MessageFrame *L_msgReceived  = NULL ;

    GEN_DEBUG (1, "C_Scenario::execute_cmd() receive command execution");

    // => check channel first
    L_channel_id = L_pCmd -> m_channel_id ;
    if (L_channel_id != P_callCtxt->m_channel_received) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Channel error: expected channel ["
                << L_channel_id
                << "] received on ["
                << P_callCtxt->m_channel_received << "]");
      L_exeCode = E_EXE_ERROR_MSG ;
    } else {

      L_msgReceived = P_callCtxt->get_msg_received() ;

      if (L_msgReceived -> update_fields(L_pCmd->m_message)) {
        GEN_LOG_EVENT(LOG_LEVEL_MSG,
                      "Received [" << *L_msgReceived << "]");
      }

      if (L_msgReceived -> compare_types (L_pCmd->m_message)) {
        if (P_resume == false) {
          if (L_pCmd->m_pre_action != 0) {
            L_exeCode = execute_action(L_pCmd, P_callCtxt,
                                       L_msgReceived,
                                       L_pCmd->m_pre_action,
                                       L_pCmd->m_pre_action_table,
                                       L_pCmd->m_message, false) ;
          }
        }

        if (L_exeCode == E_EXE_NOERROR) {

          if (L_pCmd->m_post_action != 0) {
            L_exeCode = execute_action(L_pCmd, P_callCtxt,
                                       L_msgReceived,
                                       L_pCmd->m_post_action,
                                       L_pCmd->m_post_action_table,
                                       L_pCmd->m_message,false) ;
          }

        }
        L_msgReceived -> update_message_stats();

        if (m_stats) {
          m_stats->updateStats(L_cmdIdx,C_ScenarioStats::E_MESSAGE,0);
        }

      } else  {
         if (L_pCmd->m_post_action != 0) {
            L_exeCode = execute_action(L_pCmd, P_callCtxt,
                                       L_msgReceived,
                                       L_pCmd->m_post_action,
                                       L_pCmd->m_post_action_table,
                                       L_pCmd->m_message,true) ;
                      }
         if(L_pCmd->m_post_action == 0 || L_exeCode == E_EXE_ABORT_CHECK || L_exeCode == E_EXE_ERROR_CHECK){
        // unexpected message during call
// TO-DO
        GEN_ERROR(1,"Unexpexted message that doesn't match the scenario.");

        if (!(genTraceLevel & gen_mask_table[LOG_LEVEL_TRAFFIC_ERR])) {
          GEN_ERROR(1,"Activate 'T' log level");
        }

        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                      "Unexpected message on call with session-id ["
                      << P_callCtxt->m_id_table[L_channel_id] << "]");

        if ((L_msgReceived)->name() == NULL) {
          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                        "Received [Unknown Message] when expecting ["
                        << (L_pCmd->m_message)->name()
                        << "]");
        } else {
          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                        "Received ["
                        << (L_msgReceived)->name()
                        << "] when expecting ["
                        << (L_pCmd->m_message)->name()
                        << "]");
        }
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                      "Unexpected message received [ " << (*L_msgReceived) <<
                      GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "]" );

        // TO-DO
        L_exeCode = E_EXE_ERROR_MSG ;

        if (m_stats) {
          m_stats->updateStats(L_cmdIdx,C_ScenarioStats::E_UNEXPECTED,0);
        }
      }
    }
   }
  }
    break ;

 
  case E_CMD_SCEN_WAIT:
    GEN_DEBUG (1, "C_Scenario::execute_cmd() wait command execution");
    L_exeCode = E_EXE_NOERROR ;
    break ;


  case E_CMD_SCEN_END:
    GEN_DEBUG (1, "C_Scenario::execute_cmd() scenario end command execution");
    L_exeCode = m_exe_end_code;
    break ;

  default:
    GEN_DEBUG (1, "C_Scenario::execute_cmd() unknown "
                  << L_pCmd->m_type
                  << " command execution");
    break ;
  }
if(L_exeCode >19 || L_exeCode < 0){
    if(L_exeCode >19)
    P_callCtxt -> set_cmd(L_exeCode-1-20);//will increase with E_EXE_NOERROR
    else
    P_callCtxt -> set_cmd(L_exeCode -1); //will increase with E_EXE_NOERROR (by call to next_cmd())
  L_exeCode = E_EXE_NOERROR;
  }
  switch (L_exeCode) {
  case E_EXE_NOERROR:

    // Get the next command
    P_callCtxt -> next_cmd();
    L_cmdIdx = P_callCtxt->get_current_cmd_idx();

    // Get new command description
    L_pCmd = &m_cmd_sequence[L_cmdIdx] ;

    // Change call context state dependinf on new command
    switch (L_pCmd->m_type) {
    case E_CMD_SCEN_RECEIVE:
      P_callCtxt->set_state(E_CTXT_RECEIVE);
      break ;
    case E_CMD_SCEN_SEND:
      P_callCtxt->set_state(E_CTXT_SEND);
      break ;
    case E_CMD_SCEN_WAIT:
      P_callCtxt->set_state(E_CTXT_WAIT+L_pCmd->m_duration_index);
      break ;
    case E_CMD_SCEN_END :
      L_exeCode = E_EXE_NOERROR;
      if (L_pCmd->m_pre_action != 0) {
	L_exeCode = execute_action(L_pCmd, P_callCtxt, 
				   NULL,
				   L_pCmd->m_pre_action,
				   L_pCmd->m_pre_action_table,
				   L_pCmd->m_message,false) ;
      }
      if (L_exeCode == E_EXE_NOERROR) {
	if (L_pCmd->m_post_action != 0) {
	  L_exeCode = execute_action(L_pCmd,P_callCtxt, 
				     NULL, 
				     L_pCmd->m_post_action,
				     L_pCmd->m_post_action_table,
				     L_pCmd->m_message,false) ;
	}
      }
      if (L_exeCode == E_EXE_NOERROR) {
	L_exeCode = m_exe_end_code;
      }
      break ;
    default:
      break;
    }
    break ;
  case E_EXE_TRAFFIC_END:
  case E_EXE_DEFAULT_END:
  case E_EXE_ABORT_END:
  case E_EXE_INIT_END:
  case E_EXE_SUSPEND:
    break ;
  default:
    if (L_channel_id >= 0) {
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		    "Execution error on call with session-id ["
		    << P_callCtxt->m_id_table[L_channel_id] << "]");
    }
    break ;
  }

  GEN_DEBUG(1, "C_Scenario::execute_cmd() end");
  return (L_exeCode);
}

bool C_Scenario::check_msg_received (T_pReceiveMsgContext P_rcvMsg) {

  bool L_msgOk = false ;
  GEN_DEBUG(1, "C_Scenario::check_msg_received() start");

  // Don t forget to dump the message ?
  // GEN_DEBUG(1, "C_Scenario::check_msg_received the message is [" << (*P_msg) << "]");
  GEN_DEBUG(1, "C_Scenario::check_msg_received m_sequence_max [" << m_sequence_max << "]");

  if (m_sequence_max) {
    if (m_cmd_sequence[0].m_type == E_CMD_SCEN_RECEIVE) {
      L_msgOk = ((P_rcvMsg->m_msg) -> update_fields(m_cmd_sequence[0].m_message)) ;
       
      if (L_msgOk) {
        L_msgOk = (P_rcvMsg->m_msg) -> compare_types(m_cmd_sequence[0].m_message) &&
          (P_rcvMsg->m_channel == m_cmd_sequence[0].m_channel_id) ;
      }
    }
  }

  GEN_DEBUG(1, "C_Scenario::check_msg_received the result of compare type message is " << L_msgOk);
  GEN_DEBUG(1, "C_Scenario::check_msg_received() end");
  return (L_msgOk) ;
}

void C_Scenario::set_size (size_t P_size) {
  GEN_DEBUG(1, "C_Scenario::set_size() start");
  m_sequence_size = P_size ;
  ALLOC_TABLE(m_cmd_sequence, T_pCmd_scenario, 
	      sizeof(T_cmd_scenario), m_sequence_size);
  GEN_DEBUG(1, "C_Scenario::set_size() end");
}



size_t C_Scenario::define_post_actions  
(int          P_nb_post_action,
 C_CommandAction** P_post_act_table) {

  GEN_DEBUG(1, "C_Scenario::define_post_actions() start");

  if (m_sequence_max-1 >= 0) {
    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max-1]) ;

    if ((P_nb_post_action != 0) && (P_post_act_table != NULL)) {
      delete_post_actions(m_sequence_max-1);
      L_cmd_sequence->m_post_action = P_nb_post_action ;
      L_cmd_sequence->m_post_action_table = P_post_act_table ;
    }
  }

  GEN_DEBUG(1, "C_Scenario::define_post_actions() end");

  return (m_sequence_max);
}


size_t C_Scenario::define_pre_actions  
(int               P_nb_pre_action,
 C_CommandAction** P_pre_act_table) {

  GEN_DEBUG(1, "C_Scenario::define_pre_actions() start");

  if (m_sequence_max-1 >= 0) {
    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max-1]) ;
    if ((P_nb_pre_action != 0) && (P_pre_act_table != NULL)) {
      L_cmd_sequence->m_pre_action = P_nb_pre_action ;
      L_cmd_sequence->m_pre_action_table = P_pre_act_table ;
    }
  }

  GEN_DEBUG(1, "C_Scenario::define_pre_actions() end");

  return (m_sequence_max);
}




size_t C_Scenario::add_cmd  (T_cmd_type          P_type,
			     int                 P_channel_id,
	                     T_pC_MessageFrame   P_msg,
			     int                 P_nb_pre_action,
			     C_CommandAction**   P_pre_act_table,
                             unsigned long       P_retrans_delay) {

  GEN_DEBUG(1, "C_Scenario::add_cmd() start");

  if (m_sequence_max < m_sequence_size) {

    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max]) ;

    L_cmd_sequence->m_type = P_type ;
    switch (P_type) {

    case E_CMD_SCEN_SEND: 
      m_nb_send_per_scene++;
      break ;
      
    case E_CMD_SCEN_RECEIVE:
      m_nb_recv_per_scene++;
      break ;
    default :
      break;
    }

    L_cmd_sequence->m_message = P_msg ;
    L_cmd_sequence->m_channel_id = P_channel_id ;


    if (m_retrans_enabled == false) {
      L_cmd_sequence->m_retrans_delay = 0 ;
      L_cmd_sequence->m_retrans_index = 0 ;
      L_cmd_sequence->m_retrans_delay_index = 0 ;
    } else {
      L_cmd_sequence->m_retrans_delay = P_retrans_delay ;
      if (P_retrans_delay > 0) {
        L_cmd_sequence->m_retrans_index = m_nb_retrans ;
        m_nb_retrans++;
      } else {
        L_cmd_sequence->m_retrans_index = 0 ;
      }
      L_cmd_sequence->m_retrans_delay_index = 0 ;
    }

    if ((P_nb_pre_action != 0) && (P_pre_act_table != NULL)) {
      L_cmd_sequence->m_pre_action = P_nb_pre_action ;
      L_cmd_sequence->m_pre_action_table = P_pre_act_table ;
    } else {
      L_cmd_sequence->m_pre_action = 0 ;
      L_cmd_sequence->m_pre_action_table = NULL ;
    }
    L_cmd_sequence->m_post_action = 0 ;
    L_cmd_sequence->m_post_action_table = NULL ;
    L_cmd_sequence->m_id = m_sequence_max ;

    m_sequence_max++ ;
    GEN_DEBUG(1, "C_Scenario::add_cmd() end");
    return (m_sequence_max);
  } else {
    GEN_DEBUG(1, "C_Scenario::add_cmd() end");
    return (0) ;
  }
}

size_t C_Scenario::add_cmd  (T_cmd_type    P_type,
	                     unsigned long P_duration) {
  GEN_DEBUG(1, "C_Scenario::add_cmd() start");
  if (m_sequence_max < m_sequence_size) {
    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max]) ;
    L_cmd_sequence->m_type = P_type ;
    L_cmd_sequence->m_duration = P_duration ;

    L_cmd_sequence->m_pre_action = 0 ;
    L_cmd_sequence->m_pre_action_table = NULL ;
    L_cmd_sequence->m_post_action = 0 ;
    L_cmd_sequence->m_post_action_table = NULL ;

    L_cmd_sequence->m_retrans_delay = 0 ;
    L_cmd_sequence->m_retrans_index = 0 ;
    L_cmd_sequence->m_retrans_delay_index = 0 ;

    L_cmd_sequence->m_id = m_sequence_max ;

    m_sequence_max++ ;
    GEN_DEBUG(1, "C_Scenario::add_cmd() end");
    return (m_sequence_max);
  } else {
    GEN_DEBUG(1, "C_Scenario::add_cmd() end");
    return (0) ;
  }
}

size_t C_Scenario::add_cmd  (T_cmd_type    P_type) {
  GEN_DEBUG(1, "C_Scenario::add_cmd() start");
  if (m_sequence_max < m_sequence_size) {
    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max]) ;
    L_cmd_sequence->m_type = P_type ;

    L_cmd_sequence->m_pre_action = 0 ;
    L_cmd_sequence->m_pre_action_table = NULL ;
    L_cmd_sequence->m_post_action = 0 ;
    L_cmd_sequence->m_post_action_table = NULL ;

    L_cmd_sequence->m_retrans_delay = 0 ;
    L_cmd_sequence->m_retrans_index = 0 ;
    L_cmd_sequence->m_retrans_delay_index = 0 ;

    L_cmd_sequence->m_id = m_sequence_max ;

    m_sequence_max++ ;
    GEN_DEBUG(1, "C_Scenario::add_cmd() end");
    return (m_sequence_max);
  } else {
    GEN_DEBUG(1, "C_Scenario::add_cmd() end");
    return (0) ;
  }
}

iostream_output& operator<<(iostream_output& P_ostream, C_Scenario& P_scen) {
  
  // to be updated
  int L_i ;

  for (L_i = 0 ;
       L_i < P_scen.m_sequence_max;
       L_i++) {
    switch (P_scen.m_cmd_sequence[L_i].m_type) {
    case E_CMD_SCEN_SEND:
      P_ostream << "send " ;
      break ;
    case E_CMD_SCEN_RECEIVE:
      P_ostream << "receive " ;
      break ;
    case E_CMD_SCEN_WAIT:
      P_ostream << "wait (" << P_scen.m_cmd_sequence[L_i].m_duration << ")" ;
      break ;
    case E_CMD_SCEN_END:
      P_ostream << "end" ;
      break ;
    case E_NB_CMD_SCEN:
      break ;
    }
    P_ostream << iostream_endl ;
  }

  return (P_ostream);
}

T_exeCode C_Scenario::execute_action(T_pCmd_scenario P_pCmd,
				     T_pCallContext  P_callCtxt,
				     C_MessageFrame *P_msg,
				     int             P_nbActions,
				     C_CommandAction** P_actions,
				     C_MessageFrame *P_ref,bool boolUnex) {

  T_exeCode           L_exeCode = E_EXE_NOERROR ;
  T_exeCode           L_exeCodeBranch = E_EXE_NOERROR ;
  int                 L_i ;

  C_CommandAction*    L_current_action;

  bool                L_suspend = false  ;
  bool                L_BRANCH = false  ;

  GEN_DEBUG(1, "C_Scenario::execute_action() start");
  GEN_DEBUG(1, "C_Scenario::execute_action() P_nbActions is " << 
		  P_nbActions);
  if(boolUnex){
     for (L_i = 0; L_i < P_nbActions ; L_i++) {
      if(P_actions[L_i]->get_type() == E_ACTION_SCEN_CHECK_VALUE)
        if(P_actions[L_i]->get_branching() != NULL)
         break;
    }
   if(L_i == P_nbActions)
       return E_EXE_ABORT_CHECK;
  }
  for (L_i = 0; L_i < P_nbActions ; L_i++) {

    L_current_action = P_actions[L_i] ;
    L_exeCode = L_current_action->execute(P_pCmd,
                                          P_callCtxt,
                                          P_msg,
                                          P_ref);
    if(L_exeCode > 19 || L_exeCode < 0){
        L_BRANCH = true;
        L_exeCodeBranch = L_exeCode;
    }
    switch (L_exeCode) {
    case E_EXE_ERROR       :
    case E_EXE_ERROR_CHECK :
    case E_EXE_ABORT_CHECK :
      L_i = P_nbActions ;
      break ;
    case E_EXE_SUSPEND:
      L_suspend = true;
      break ;
    case E_EXE_ERR_BRANCH_EXP:
      if(boolUnex)
      return E_EXE_ABORT_CHECK;
      break;
    default:
      break;
    }
  } // for L_i

  if ((L_exeCode == E_EXE_NOERROR) && (L_suspend == true)) {
    L_exeCode = E_EXE_SUSPEND ;
  } 
  if(L_suspend != true && L_BRANCH == true)
    L_exeCode = L_exeCodeBranch ;
  
  GEN_DEBUG(1, "C_Scenario::execute_action() end");
  return (L_exeCode);
}

void C_Scenario::update_retrans_delay_cmd (size_t P_nb, unsigned long *P_table) {
  int      L_i ;
  size_t   L_j ;
  for(L_i=0; L_i < m_sequence_max; L_i++) {
    if (m_cmd_sequence[L_i].m_retrans_delay > 0 ) {
      for (L_j = 0; L_j < P_nb; L_j++) {
	if (m_cmd_sequence[L_i].m_retrans_delay == P_table[L_j]) {
	  m_cmd_sequence[L_i].m_retrans_delay_index = L_j ;
	  break ;
	}
      }
    }
  }
}

void C_Scenario::update_wait_cmd (size_t P_nb, unsigned long *P_table) {
  int      L_i ;
  size_t   L_j ;
  for(L_i=0; L_i < m_sequence_max; L_i++) {
    if (m_cmd_sequence[L_i].m_type == E_CMD_SCEN_WAIT) {
      for (L_j = 0; L_j < P_nb; L_j++) {

	if (m_cmd_sequence[L_i].m_duration == P_table[L_j]) {
	  m_cmd_sequence[L_i].m_duration_index = L_j ;
	  break ;
	}
      }
    }
  }
}

void C_Scenario::delete_post_actions (int P_cmd_index) {

  int L_j ;

  if (m_cmd_sequence[P_cmd_index].m_post_action != 0) {
    for (L_j = 0 ; L_j < m_cmd_sequence[P_cmd_index].m_post_action; L_j ++) {
      DELETE_VAR(m_cmd_sequence[P_cmd_index].m_post_action_table[L_j])
    }
    FREE_TABLE(m_cmd_sequence[P_cmd_index].m_post_action_table);
    m_cmd_sequence[P_cmd_index].m_post_action = 0 ;
  }
}

void C_Scenario::update_post_actions  
(int               P_nb_post_action,
 C_CommandAction** P_post_act_table,
 int               P_cmd_index) {
  
  T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[P_cmd_index]) ;

  if ((P_nb_post_action != 0) && (P_post_act_table != NULL)) {
    L_cmd_sequence->m_post_action = P_nb_post_action ;
    L_cmd_sequence->m_post_action_table = P_post_act_table ;
  }
}



void C_Scenario::update_pre_actions  
(int               P_nb_pre_action,
 C_CommandAction** P_pre_act_table,
 int               P_cmd_index) {
  
  T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[P_cmd_index]) ;
  if ((P_nb_pre_action != 0) && (P_pre_act_table != NULL)) {
    L_cmd_sequence->m_pre_action = P_nb_pre_action ;
    L_cmd_sequence->m_pre_action_table = P_pre_act_table ;
  }
}

void C_Scenario::delete_pre_actions (int P_cmd_index) {
  int L_j ;

  if (m_cmd_sequence[P_cmd_index].m_pre_action != 0) {
    for (L_j = 0 ; L_j < m_cmd_sequence[P_cmd_index].m_pre_action; L_j ++) {
      DELETE_VAR(m_cmd_sequence[P_cmd_index].m_pre_action_table[L_j])
    }
    FREE_TABLE(m_cmd_sequence[P_cmd_index].m_pre_action_table);
    m_cmd_sequence[P_cmd_index].m_pre_action = 0 ;
  }
}


void C_Scenario::set_stats (C_ScenarioStats *P_scenStat) {
  m_stats = P_scenStat ;
}

C_ScenarioStats* C_Scenario::get_stats () {
  return (m_stats) ;
}

void C_Scenario::delete_stats () {
  DELETE_VAR(m_stats);
}


T_exeCode    C_Scenario::get_exe_end_code() {
  return (m_exe_end_code)  ;
}

int C_Scenario::get_nb_retrans ()
{
  return (m_nb_retrans);
}

T_pCmd_scenario C_Scenario::get_commands() {
  return (m_cmd_sequence);
}


int C_Scenario::get_nb_send_per_scen ()
{
  return (m_nb_send_per_scene);
}


int C_Scenario::get_nb_recv_per_scen ()
{
  return (m_nb_recv_per_scene);
}
