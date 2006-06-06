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
  "E_NB_ACTION_SCEN",  // internal actions after this value
  "E_ACTION_SCEN_INTERNAL_INIT_DONE",
  "E_ACTION_SCEN_CHECK_ALL_MSG",
  "E_ACTION_SCEN_ADD_IN_CALL_MAP",
  "E_ACTION_SCEN_SELECT_EXTERNAL_DATA_LINE"
} ;


C_Scenario::C_Scenario (C_ScenarioControl     *P_scenario_control,
			C_ChannelControl      *P_channel_control,
			C_ExternalDataControl *P_external_data_control,
			T_exeCode              P_exe_code,
			char                  *P_behaviour,
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

void C_Scenario::set_call_map (T_pCallMap *P_call_map) {
  m_call_map_table = P_call_map ;
}

C_Scenario::~C_Scenario () {

  int          L_i;
  int          L_j, L_k ;
  GEN_DEBUG(1, "C_Scenario::~C_Scenario() start seq nb: " << m_sequence_max);

  if (m_sequence_max) {

    for (L_i = 0; L_i < m_sequence_max; L_i++) {

      // Do not delete sequence message here 
      // this is made during specific protocol clean

      if (m_cmd_sequence[L_i].m_pre_action != 0) {
	for (L_j = 0 ; L_j < m_cmd_sequence[L_i].m_pre_action; L_j ++) {
	  if (m_cmd_sequence[L_i].m_pre_act_table[L_j].m_type
	      == E_ACTION_SCEN_SET_VALUE) {
	    for(L_k = 0; 
		L_k < m_cmd_sequence[L_i].m_pre_act_table[L_j].
		  m_string_expr->m_nb_portion; L_k++) {
	      if (m_cmd_sequence[L_i].m_pre_act_table[L_j].
		  m_string_expr->m_portions[L_k].m_type
		  == E_STR_STATIC) {
		FREE_TABLE(m_cmd_sequence[L_i].m_pre_act_table[L_j].
			   m_string_expr->m_portions[L_k].m_data.m_value);
	      }
	    }
	    FREE_TABLE(m_cmd_sequence[L_i].m_pre_act_table[L_j].
		       m_string_expr->m_portions);
	    FREE_VAR(m_cmd_sequence[L_i].m_pre_act_table[L_j].
		       m_string_expr);

       // free regexp
	  }
	}
	FREE_TABLE(m_cmd_sequence[L_i].m_pre_act_table);
      }

      if (m_cmd_sequence[L_i].m_post_action != 0) {
	for (L_j = 0 ; L_j < m_cmd_sequence[L_i].m_post_action; L_j ++) {
	  if (m_cmd_sequence[L_i].m_post_act_table[L_j].m_type
	      == E_ACTION_SCEN_SET_VALUE) {
	    for(L_k = 0; 
		L_k < m_cmd_sequence[L_i].m_post_act_table[L_j].
		  m_string_expr->m_nb_portion; L_k++) {
	      if (m_cmd_sequence[L_i].m_post_act_table[L_j].
		  m_string_expr->m_portions[L_k].m_type
		  == E_STR_STATIC) {
		FREE_TABLE(m_cmd_sequence[L_i].m_post_act_table[L_j].
			   m_string_expr->m_portions[L_k].m_data.m_value);
	      }
	    }

	    FREE_TABLE(m_cmd_sequence[L_i].m_post_act_table[L_j].
		       m_string_expr->m_portions);
	     FREE_VAR(m_cmd_sequence[L_i].m_post_act_table[L_j].
		       m_string_expr);

        // free regexp
	  }
	}
	FREE_TABLE(m_cmd_sequence[L_i].m_post_act_table);
      }

    }
  }
  FREE_TABLE(m_cmd_sequence);
  m_sequence_max = 0 ;
  m_sequence_size = 0 ;
  m_stat = NULL ;
  m_stats = NULL ;

  GEN_DEBUG(1, "C_Scenario::~C_Scenario() end");
}

void C_Scenario::set_data_log_controller (C_DataLogControl *P_log) {
  GEN_DEBUG(1, "C_Scenario::set_data_log_controller start");
  m_log = P_log ;
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

T_exeCode C_Scenario::execute_cmd (T_pCallContext P_callCtxt, bool P_resume) {

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
				   L_pCmd->m_pre_act_table,
				   L_pCmd->m_message) ;
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
      }

    }

    if ((L_exeCode == E_EXE_NOERROR) && (L_exeCode != E_EXE_SUSPEND)) {  
      // now execute post actions
      if (L_pCmd->m_post_action != 0) {
	L_exeCode = execute_action(L_pCmd, P_callCtxt, 
				   L_sendMsg, 
				   L_pCmd->m_post_action,
				   L_pCmd->m_post_act_table,
				   L_pCmd->m_message) ;
      }
    }

    if (L_exeCode != E_EXE_SUSPEND) {
      DELETE_VAR(L_sendMsg);
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

      if (L_msgReceived -> compare_types (L_pCmd->m_message)) {
	
	if (P_resume == false) {
	  if (L_pCmd->m_pre_action != 0) {
	    L_exeCode = execute_action(L_pCmd, P_callCtxt, 
				       L_msgReceived,
				       L_pCmd->m_pre_action,
				       L_pCmd->m_pre_act_table,
				       L_pCmd->m_message) ;
	  }
	}
	
	if (L_exeCode == E_EXE_NOERROR) {
	  
	  if (L_pCmd->m_post_action != 0) {
	    L_exeCode = execute_action(L_pCmd, P_callCtxt, 
				       L_msgReceived, 
				       L_pCmd->m_post_action,
				       L_pCmd->m_post_act_table,
				       L_pCmd->m_message) ;
	  }
	  
	}
	
	if (m_stats) {
	  m_stats->updateStats(L_cmdIdx,C_ScenarioStats::E_MESSAGE,0);
	}

      } else {
	// unexpected message during call
	// TO-DO
	GEN_ERROR(1,"Unexpexted message that doesn't match the scenario.");

	if (!(genTraceLevel & gen_mask_table[LOG_LEVEL_TRAFFIC_ERR])) {
	  GEN_ERROR(1,"Activate 'T' log level");
	}

	GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		      "Unexpected message on call with session-id ["
		      << P_callCtxt->m_id_table[L_channel_id] << "]");

  	GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
  		      "Received ["
  		      << (L_msgReceived)->name()
  		      << "] when expecting ["
  		      << (L_pCmd->m_message)->name()
  		      << "]");
	
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
				   L_pCmd->m_pre_act_table,
				   L_pCmd->m_message) ;
      }
      if (L_exeCode == E_EXE_NOERROR) {
	if (L_pCmd->m_post_action != 0) {
	  L_exeCode = execute_action(L_pCmd,P_callCtxt, 
				     NULL, 
				     L_pCmd->m_post_action,
				     L_pCmd->m_post_act_table,
				     L_pCmd->m_message) ;
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
    break ;
  default:
    if (L_channel_id > 0) {
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
      L_msgOk = (P_rcvMsg->m_msg) -> compare_types(m_cmd_sequence[0].m_message) &&
	        (P_rcvMsg->m_channel == m_cmd_sequence[0].m_channel_id) ;
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
 T_pCmdAction P_post_act_table) {

  GEN_DEBUG(1, "C_Scenario::define_post_actions() start");

  if (m_sequence_max-1 >= 0) {
    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max-1]) ;
    if ((P_nb_post_action != 0) && (P_post_act_table != NULL)) {
      delete_post_actions(m_sequence_max-1);
      L_cmd_sequence->m_post_action = P_nb_post_action ;
      L_cmd_sequence->m_post_act_table = P_post_act_table ;
    }
  }

  GEN_DEBUG(1, "C_Scenario::define_post_actions() end");

  return (m_sequence_max);
}


size_t C_Scenario::define_pre_actions  
(int          P_nb_pre_action,
 T_pCmdAction P_pre_act_table) {

  GEN_DEBUG(1, "C_Scenario::define_post_actions() start");

  if (m_sequence_max-1 >= 0) {
    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max-1]) ;
    if ((P_nb_pre_action != 0) && (P_pre_act_table != NULL)) {
      //      delete_post_actions(m_sequence_max-1);
      L_cmd_sequence->m_pre_action = P_nb_pre_action ;
      L_cmd_sequence->m_pre_act_table = P_pre_act_table ;
    }
  }

  GEN_DEBUG(1, "C_Scenario::define_post_actions() end");

  return (m_sequence_max);
}




size_t C_Scenario::add_cmd  (T_cmd_type          P_type,
			     int                 P_channel_id,
	                     T_pC_MessageFrame   P_msg,
			     int                 P_nb_pre_action,
			     T_pCmdAction        P_pre_act_table) {
  GEN_DEBUG(1, "C_Scenario::add_cmd() start");

  if (m_sequence_max < m_sequence_size) {

    T_pCmd_scenario L_cmd_sequence = &(m_cmd_sequence[m_sequence_max]) ;

    L_cmd_sequence->m_type = P_type ;
    L_cmd_sequence->m_message = P_msg ;
    L_cmd_sequence->m_channel_id = P_channel_id ;

    if ((P_nb_pre_action != 0) && (P_pre_act_table != NULL)) {
      L_cmd_sequence->m_pre_action = P_nb_pre_action ;
      L_cmd_sequence->m_pre_act_table = P_pre_act_table ;
    } else {
      L_cmd_sequence->m_pre_action = 0 ;
      L_cmd_sequence->m_pre_act_table = NULL ;
    }
    L_cmd_sequence->m_post_action = 0 ;
    L_cmd_sequence->m_post_act_table = NULL ;

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
    L_cmd_sequence->m_pre_act_table = NULL ;
    L_cmd_sequence->m_post_action = 0 ;
    L_cmd_sequence->m_post_act_table = NULL ;

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
    L_cmd_sequence->m_pre_act_table = NULL ;
    L_cmd_sequence->m_post_action = 0 ;
    L_cmd_sequence->m_post_act_table = NULL ;

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
				     T_pCmdAction    P_actions,
				     C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode = E_EXE_NOERROR ;
  int                 L_i, L_j ;
  T_pCmdAction        L_current_action;
  static char         L_tmp_string [250] ;
  static char         L_tmp_string_filled [250] ;
  char               *L_ptr ;
  size_t              L_size, L_current_size ;
  bool                L_reset_value = false ;
  long                L_time_ms ;
  bool                L_check_result ;

  int                 L_id ;

  bool                L_suspend = false ;
  T_OpenStatus        L_openStatus ;
  struct timezone     L_timeZone ;

  char *L_values, *L_chr, *L_end_ptr ;
  unsigned long    L_value ;

  GEN_DEBUG(1, "C_Scenario::execute_action() start");
  GEN_DEBUG(1, "C_Scenario::execute_action() P_nbActions is " << 
		  P_nbActions);


  for (L_i = 0; L_i < P_nbActions ; L_i++) {

    L_current_action = &P_actions[L_i] ;


    GEN_DEBUG(1, "C_Scenario::execute_action() action type["
	      << L_i << "] is \"" 
	      << action_name_table[L_current_action->m_type] 
	      << "\" (" << L_current_action->m_type << ")"
	      );


    switch (L_current_action->m_type) {

    case E_ACTION_SCEN_OPEN :

      L_id = m_channel_ctrl->open_local_channel(P_pCmd->m_channel_id,
                                                L_current_action->m_args,
 						P_callCtxt->m_channel_table,
 						&L_openStatus);

						
      if (L_id != -1) {

	if (L_openStatus == E_OPEN_DELAYED) {
	  L_suspend = true ;
	  P_callCtxt->m_suspend_id = L_id ;
	  P_callCtxt->m_suspend_msg = P_msg ;
          P_callCtxt->m_channel_id   = P_pCmd->m_channel_id ;
	  gettimeofday(&P_callCtxt->m_current_time, &L_timeZone) ;
	  // OPEN DELAYED => do not execute the command
	  // just wait for open
	}

      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Open failed");
  	L_exeCode = E_EXE_ERROR ;
  	break ;
      }
      break ; 

    case E_ACTION_SCEN_CLOSE :

      // m_channel_ctrl->close_local_channel(P_pCmd->m_channel_id, NULL);
      m_channel_ctrl->close_local_channel(P_pCmd->m_channel_id, P_callCtxt->m_channel_table);
      break ;

    case E_ACTION_SCEN_MEMORY_STORE : {
      T_pValueData L_mem = NULL ;
      int            L_size ;

      P_callCtxt->reset_memory(L_current_action->m_mem_id) ;

      GEN_DEBUG(2, "store memory id = " << L_current_action->m_mem_id);
      GEN_DEBUG(2, "store field id = " << L_current_action->m_id);

      L_mem = P_callCtxt->get_memory(L_current_action->m_mem_id);

      L_size = L_current_action->m_size ;

      if (L_size != -1) {
	// string or binary type
	T_ValueData L_val ;
	L_val.m_type = E_TYPE_NUMBER ;


	if (P_msg -> get_field_value(L_current_action->m_id, 
                                     L_current_action->m_instance_id,
                                     L_current_action->m_sub_id,
                                     &L_val) == true) {

          if (L_size > ((int)L_val.m_value.m_val_binary.m_size-L_current_action->m_begin)) {
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          action_name_table[L_current_action->m_type] 
                          << ": the size desired  ["
                          << L_size << "] exceed the length of buffer ["
                          << (L_val.m_value.m_val_binary.m_size - L_current_action->m_begin) << "]");
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          "error on call with session-id ["
                          << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
            
            resetMemory(L_val);
            L_exeCode = E_EXE_ERROR;
          } else {
            L_mem->m_value.m_val_binary.m_size = L_size ;
            L_mem->m_type = L_val.m_type ;
            
            ALLOC_TABLE(L_mem->m_value.m_val_binary.m_value,
                        unsigned char*,
                        sizeof(unsigned char),
                        L_size);
            
            extractBinaryVal(*L_mem, L_current_action->m_begin, L_size,
                             L_val);
            
            resetMemory(L_val);
          }
        } else {
	  GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                        action_name_table[L_current_action->m_type] 
                        << ": the value of the field asked is incorrect or not found");
	  
	  GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
			"error on call with session-id ["
			<< P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
          
	  L_exeCode = E_EXE_ERROR;
        }
      } else {

        if (L_current_action->m_regexp_data != NULL) {
          if (P_msg -> get_field_value(L_current_action->m_id, 
                                       L_current_action->m_regexp_data,
                                       L_mem) == false) {
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          action_name_table[L_current_action->m_type] 
                          << ": the value of the field asked is incorrect or not found");
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          "error on call with session-id ["
                          << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
            
            L_exeCode = E_EXE_ERROR;
          }
        } else {
          if (P_msg -> get_field_value(L_current_action->m_id, 
                                       L_current_action->m_instance_id,
                                       L_current_action->m_sub_id,
                                       L_mem) == false) {
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          action_name_table[L_current_action->m_type] 
                          << ": the value of the field asked is incorrect or not found");
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          "error on call with session-id ["
                          << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
            
            L_exeCode = E_EXE_ERROR;
          }
        }

      }
    }

      break ;

    case E_ACTION_SCEN_MEMORY_RESTORE : {
      
      T_pValueData L_mem ;
      int          L_size  ;

      GEN_DEBUG(2, "restore memory id = " << L_current_action->m_mem_id);
      GEN_DEBUG(2, "restore field id = " << L_current_action->m_id);

      L_mem = P_callCtxt->get_memory(L_current_action->m_mem_id);

      L_size = L_current_action->m_size ;

      if (L_size != -1) {
	T_ValueData           L_val ;
	L_val.m_type = E_TYPE_NUMBER ;

        if (L_mem->m_value.m_val_binary.m_size == 0) {
          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                        action_name_table[L_current_action->m_type] 
                        << " action: you use a memory zone that was not previously stored");
        
          break ;
        }

	// RETRIEVE the data 
	if (P_msg -> get_field_value(L_current_action->m_id, 
				 L_current_action->m_instance_id,
				 L_current_action->m_sub_id,
                                     &L_val) == true) {

          if (L_size > (int)L_val.m_value.m_val_binary.m_size) {
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                          action_name_table[L_current_action->m_type] 
                          << ": the size desired  ["
                          << L_size << "] exceed the length of buffer ["
                          << L_val.m_value.m_val_binary.m_size << "]");
            L_size = L_val.m_value.m_val_binary.m_size ;
          }

          copyBinaryVal(L_val, L_current_action->m_begin, L_size,
                        *L_mem);

          if (P_msg -> set_field_value(&L_val, 
                                       L_current_action->m_id,
                                       L_current_action->m_instance_id,
                                       L_current_action->m_sub_id) == false ) {

            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          action_name_table[L_current_action->m_type] 
                          << ": problem when set field value");
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          "error on call with session-id ["
                          << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
            
            L_exeCode = E_EXE_ERROR;
          }
          resetMemory(L_val);
        } else {
	  GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                        action_name_table[L_current_action->m_type] 
                        << ": the value of the field asked is incorrect or not found");
	  
	  GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
			"error on call with session-id ["
			<< P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
          
	  L_exeCode = E_EXE_ERROR;
        }
      } else {
        if (P_msg -> set_field_value(L_mem, 
			         L_current_action->m_id,
			         L_current_action->m_instance_id,
                                     L_current_action->m_sub_id) == false ) {

          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                        action_name_table[L_current_action->m_type] 
                        << ": problem when set field value");
          
          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                        "error on call with session-id ["
                        << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
          
          L_exeCode = E_EXE_ERROR;
        }
      }
    }

      break ;

    case E_ACTION_SCEN_START_TIMER :
      
      P_callCtxt->m_start_time = P_callCtxt->m_current_time ;
      break ;

    case E_ACTION_SCEN_STOP_TIMER : 

      GEN_DEBUG(1, "C_Scenario::execute_action() E_ACTION_SCEN_STOP_TIMER " << 
		    m_log);
      L_time_ms = 
	ms_difftime(&P_callCtxt->m_current_time, 
		    &P_callCtxt->m_start_time);
      m_stat->executeStatAction (C_GeneratorStats::E_ADD_RESPONSE_TIME_DURATION,
				 L_time_ms) ;
      if (m_log) {
	// return exec => PB no memory available
	m_log->time_data(&P_callCtxt->m_start_time, 
			 &P_callCtxt->m_current_time);
      }
      break ;

    case E_ACTION_SCEN_SET_VALUE: {
      
      T_ValueData L_mem ;
      L_mem.m_type = E_TYPE_NUMBER ;

      L_mem.m_type = P_msg->get_field_type(L_current_action->m_id,0);

      switch (L_mem.m_type) {

      case E_TYPE_NUMBER:
	L_mem.m_value.m_val_number = m_controller->get_counter_value
	  (L_current_action->m_string_expr->m_portions[0].m_data.m_id) ;
	break ;

      case E_TYPE_SIGNED:
	L_mem.m_value.m_val_signed = (T_Integer32) m_controller->get_counter_value
	  (L_current_action->m_string_expr->m_portions[0].m_data.m_id) ;
	break ;

      case E_TYPE_STRING:
	L_tmp_string[0] = '\0' ;
	L_ptr = L_tmp_string ;
	L_size = 0 ;
	for (L_j = 0 ;
	     L_j < L_current_action->m_string_expr->m_nb_portion;
	     L_j ++) {
	  switch (L_current_action->m_string_expr->m_portions[L_j].m_type) {
	  case E_STR_STATIC:
	    strcpy(L_ptr, 
		   L_current_action
		   ->m_string_expr->m_portions[L_j].m_data.m_value);
	    L_current_size = strlen(L_ptr) ;
	    L_size += L_current_size ;
	    L_ptr += L_current_size ;
	    break ;
	  case E_STR_COUNTER:
	    sprintf(L_ptr, "%ld", 
		    m_controller->get_counter_value
		    (L_current_action->m_string_expr
		     ->m_portions[L_j].m_data.m_id)) ;
	    L_current_size = strlen(L_ptr);
	    L_size += L_current_size;
	    L_ptr += L_current_size;
	    break ;
	  }
	}
	L_mem.m_value.m_val_binary.m_size = L_size ;
	L_mem.m_value.m_val_binary.m_value 
	  = (unsigned char *) L_tmp_string;

        if (L_current_action->m_size != -1) {
          if (L_current_action->m_size > (int)L_size) {
            int L_filled_size = L_current_action->m_size - L_size ;
            int L_fill_idx = 0 ;
            memcpy(L_tmp_string_filled+L_filled_size,
                   L_tmp_string,
                   L_size);
            
            while ((L_fill_idx+(L_current_action->m_pattern_size)) > L_filled_size) {
              memcpy(L_tmp_string_filled+L_fill_idx,
                     L_current_action->m_pattern,
                     L_current_action->m_pattern_size);
              L_fill_idx += L_current_action->m_pattern_size ;
            }
            L_mem.m_value.m_val_binary.m_size = L_current_action->m_size ;
            L_mem.m_value.m_val_binary.m_value 
              = (unsigned char *) L_tmp_string_filled;
            
          }
        }

	L_reset_value = true ;
	
	break;
	
	case E_TYPE_STRUCT:{ 

	  // P_instance,
	  // P_sub_id

	  P_msg -> get_field_value(L_current_action->m_id, 
				   0,0,
				   // L_current_action->m_instance_id,
				   // L_current_action->m_sub_id,
				   &L_mem);
	  switch (L_current_action->m_string_expr->m_nb_portion) {
	  case 1:
	    L_values = L_current_action->m_string_expr->m_portions[0].m_data.m_value ;
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
	    if (L_current_action->m_string_expr->m_portions[0].m_type == E_STR_COUNTER){
	      L_mem.m_value.m_val_struct.m_id_1 = m_controller->get_counter_value
		(L_current_action->m_string_expr->m_portions[0].m_data.m_id) ;
	      L_values = L_current_action->m_string_expr->m_portions[1].m_data.m_value ;
	      L_chr = strrchr(L_values, ';');
	      if (*(L_chr+1) != '\0') {
		L_value = strtoul_f (L_chr+1, &L_end_ptr, 10);
		if (L_end_ptr[0] == '\0') { // good format
		  L_mem.m_value.m_val_struct.m_id_2
		    = L_value ;
		}
	      }
	      
	    } else {
	      
	      L_values = L_current_action->m_string_expr->m_portions[0].m_data.m_value ;
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
	      L_mem.m_value.m_val_struct.m_id_2 = m_controller->get_counter_value
		(L_current_action->m_string_expr->m_portions[1].m_data.m_id) ;
	      
	    } 
	    
	    break ;
	  case 3:
	    L_mem.m_value.m_val_struct.m_id_1 = m_controller->get_counter_value
	      (L_current_action->m_string_expr->m_portions[0].m_data.m_id) ;
	    L_mem.m_value.m_val_struct.m_id_2 = m_controller->get_counter_value
	      (L_current_action->m_string_expr->m_portions[2].m_data.m_id) ;
	    break ;
	  default:
	    break ;
	  }
	}
	  break ;

      case E_TYPE_NUMBER_64:
	L_mem.m_value.m_val_number_64 = (T_UnsignedInteger64) m_controller->get_counter_value
	  (L_current_action->m_string_expr->m_portions[0].m_data.m_id) ;
	break ;

      case E_TYPE_SIGNED_64:
	L_mem.m_value.m_val_signed_64 = (T_Integer64) m_controller->get_counter_value
	  (L_current_action->m_string_expr->m_portions[0].m_data.m_id) ;
	break ;

	default:
	  GEN_FATAL(E_GEN_FATAL_ERROR, "Unsupported type for action execution");
	  break ;
	}


        if (P_msg->set_field_value(&L_mem, 
                                   L_current_action->m_id,
                                   L_current_action->m_instance_id,
                                   L_current_action->m_sub_id) == false ) {

          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                        action_name_table[L_current_action->m_type] 
                        << ": problem when set field value");
          
          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                        "error on call with session-id ["
                        << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
          
          L_exeCode = E_EXE_ERROR;

        }

  	if (L_reset_value == true) {
  	  L_mem.m_type = E_TYPE_NUMBER ;
  	}

    }
      break ;

    case E_ACTION_SCEN_INC_COUNTER :
      m_controller->increase_counter(L_current_action->m_id) ;
      break ;

    case E_ACTION_SCEN_CHECK_PRESENCE:
      L_check_result 
	= P_msg->check_field_presence 
	(L_current_action->m_id,
	 L_current_action->m_check_behaviour,
	 L_current_action->m_instance_id,
	 L_current_action->m_sub_id);
      if (L_check_result == false) {
	GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		      "check error on call with session-id ["
		      << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
        switch (L_current_action->m_check_behaviour) {
        case E_CHECK_BEHAVIOUR_ERROR :
          L_exeCode = E_EXE_ERROR_CHECK ;
          break;
        case E_CHECK_BEHAVIOUR_ABORT:
          L_exeCode = E_EXE_ABORT_CHECK ;
          break;
        default:
          break;
        }
      }
      break ;

    case E_ACTION_SCEN_GET_EXTERNAL_DATA : {
      T_pValueData L_value ;
      int          L_size  ;
      
      L_value = m_external_data->get_value(P_callCtxt->m_selected_line, 
					   L_current_action->m_field_data_num);

      if (L_value != NULL) {
        L_size = L_current_action->m_size ;

        if (L_size != -1) {
          // binary or string type
          T_ValueData           L_val ;
          L_val.m_type = E_TYPE_NUMBER ;
          
          if (L_size > (int)L_value->m_value.m_val_binary.m_size) {
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                          action_name_table[L_current_action->m_type] 
                          << ": the size desired  ["
                          << L_size << "] exceed the length of buffer ["
                          << L_value->m_value.m_val_binary.m_size << "]");
            L_size = L_value->m_value.m_val_binary.m_size ;
          }
          
          if (P_msg -> get_field_value(L_current_action->m_id, 
				 L_current_action->m_instance_id,
                                       L_current_action->m_sub_id,
                                       &L_val) == true) {
            
            copyBinaryVal(L_val, L_current_action->m_begin, L_size,
                          *L_value);
            
            if (P_msg -> set_field_value(&L_val, 
                                     L_current_action->m_id,
                                     L_current_action->m_instance_id,
                                         L_current_action->m_sub_id) == false ) {

              GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                            action_name_table[L_current_action->m_type] 
                            << ": problem when set field value");
              
              GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                            "error on call with session-id ["
                            << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
              
              L_exeCode = E_EXE_ERROR;
            }
            resetMemory(L_val);

          } else {
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          action_name_table[L_current_action->m_type] 
                          << ": the value of the field asked is incorrect or not found");
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          "error on call with session-id ["
                          << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
            L_exeCode = E_EXE_ERROR;
          }
        } else {
          if (P_msg -> set_field_value(L_value, 
                                       L_current_action->m_id,
                                       L_current_action->m_instance_id,
                                       L_current_action->m_sub_id) == false) {
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          action_name_table[L_current_action->m_type] 
                          << ": problem when set field value");
            
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                          "error on call with session-id ["
                          << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
            
            L_exeCode = E_EXE_ERROR;
          }
        }
      } else {
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                      action_name_table[L_current_action->m_type] 
                      << ": the value of the field asked is incorrect or not found");
        
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
                      "error on call with session-id ["
                      << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
        L_exeCode = E_EXE_ERROR;
      }
    }
    break ;
    
    case E_ACTION_SCEN_INC_VAR: {
      
      T_pValueData L_mem ;
      L_mem = P_callCtxt->get_memory(L_current_action->m_mem_id);

      switch (L_mem->m_type) {
	
      case E_TYPE_NUMBER:
	L_mem->m_value.m_val_number = L_mem->m_value.m_val_number ++ ;
	break ;
	
      default:
	GEN_FATAL(E_GEN_FATAL_ERROR, "Unsupported type [" << L_mem->m_type << "] for increment function");
	break ;
	
      }
    }
    break ;

    case E_ACTION_SCEN_CHECK_VALUE:
      L_check_result 
	= P_msg->check_field_value 
	(P_ref,
	 L_current_action->m_id,
	 L_current_action->m_check_behaviour,
	 L_current_action->m_instance_id,
	 L_current_action->m_sub_id);
      if (L_check_result == false) {
	GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		      "check error on call with session-id ["
		      << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");

        switch (L_current_action->m_check_behaviour) {
        case E_CHECK_BEHAVIOUR_ERROR :
          L_exeCode = E_EXE_ERROR_CHECK ;
          break;
        case E_CHECK_BEHAVIOUR_ABORT:
          L_exeCode = E_EXE_ABORT_CHECK ;
          break;
        default:
          break;
        }

      }
      break ;

    case E_ACTION_SCEN_CHECK_ORDER:
      L_check_result 
	= P_msg->check_field_order 
	(L_current_action->m_id,
	 L_current_action->m_check_behaviour,
	 L_current_action->m_position);
      if (L_check_result == false) {
	GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		      "check error on call with session-id ["
		      << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");


        switch (L_current_action->m_check_behaviour) {
        case E_CHECK_BEHAVIOUR_ERROR :
          L_exeCode = E_EXE_ERROR_CHECK ;
          break;
        case E_CHECK_BEHAVIOUR_ABORT:
          L_exeCode = E_EXE_ABORT_CHECK ;
          break;
        default:
          break;
        }

      }
      break ;


    case E_ACTION_SCEN_INTERNAL_INIT_DONE:
      L_exeCode = E_EXE_INIT_END ;
      break ;

    case E_ACTION_SCEN_CHECK_ALL_MSG:
      L_check_result 
	= P_msg->check(P_ref, 
		       m_check_mask,
		       m_check_behaviour);
      if (L_check_result == false) {
	GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		      "check error on call with session-id ["
		      << P_callCtxt->m_id_table[P_pCmd->m_channel_id] << "]");
	if (m_check_behaviour == E_CHECK_BEHAVIOUR_ERROR) {
	  L_exeCode = E_EXE_ERROR_CHECK ;
	}
      }
      break ;

    case E_ACTION_SCEN_ADD_IN_CALL_MAP: {
      T_pValueData L_value_id ;
      
      L_value_id = P_msg -> get_session_id(P_callCtxt);
      
      if (L_value_id == NULL) {
        // TO DO
	GEN_ERROR(E_GEN_FATAL_ERROR, "session id is failed");
  	L_exeCode = E_EXE_ERROR ;
      } else {
        L_value_id = P_callCtxt->set_id (L_current_action->m_id,L_value_id);
        m_call_map_table[L_current_action->m_id]
          ->insert(T_CallMap::value_type(*L_value_id, P_callCtxt));
      }
    }
    break ;
    
    case E_ACTION_SCEN_SELECT_EXTERNAL_DATA_LINE: {
      P_callCtxt->m_selected_line = m_external_data->select_line();
    }
    break ;

    default:
      GEN_FATAL(E_GEN_FATAL_ERROR, "Action execution error");
      break ;
    } // switch

    if (L_exeCode != E_EXE_NOERROR) break ;

  } // for L_i

  
  if ((L_exeCode == E_EXE_NOERROR) && (L_suspend == true)) {
    L_exeCode = E_EXE_SUSPEND ;
  }

  GEN_DEBUG(1, "C_Scenario::execute_action() end");
  return (L_exeCode);
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

  int L_j, L_k ;

  if (m_cmd_sequence[P_cmd_index].m_post_action != 0) {
    for (L_j = 0 ; L_j < m_cmd_sequence[P_cmd_index].m_post_action; L_j ++) {
      if (m_cmd_sequence[P_cmd_index].m_post_act_table[L_j].m_type
	  == E_ACTION_SCEN_SET_VALUE) {
	for(L_k = 0; 
	    L_k < m_cmd_sequence[P_cmd_index].m_post_act_table[L_j].
	      m_string_expr->m_nb_portion; L_k++) {
	  if (m_cmd_sequence[P_cmd_index].m_post_act_table[L_j].
	      m_string_expr->m_portions[L_k].m_type
	      == E_STR_STATIC) {
	    FREE_TABLE(m_cmd_sequence[P_cmd_index].m_post_act_table[L_j].
		       m_string_expr->m_portions[L_k].m_data.m_value);
	  }
	}
	FREE_TABLE(m_cmd_sequence[P_cmd_index].m_post_act_table[L_j].
		   m_string_expr);

        // free regexp????
      }

    }
    FREE_TABLE(m_cmd_sequence[P_cmd_index].m_post_act_table);
    m_cmd_sequence[P_cmd_index].m_post_action = 0 ;
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
