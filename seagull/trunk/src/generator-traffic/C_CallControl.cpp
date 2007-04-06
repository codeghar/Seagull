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

#include "C_CallControl.hpp"
#include "Utils.hpp"
#include "GeneratorTrace.hpp"
#include "C_MultiList.cpp"

#include "C_ScenarioStats.hpp"

#include <cerrno>  // for errno definition
#include <cstring> // for strerror defionition
#include <cmath>


C_CallControl::C_CallControl(C_GeneratorConfig   *P_config, 
			     T_pC_ScenarioControl P_scenControl,
			     C_ChannelControl     *P_channel_ctrl) : C_TaskControl() {
  GEN_DEBUG (1, "C_CallControl::C_CallControl() start");

  int L_i ;

  NEW_VAR(m_msg_rcv_ctxt_list, T_RcvMsgCtxtList());
  NEW_VAR(m_event_list, T_EventRecvList());
  m_call_ctxt_table = NULL ;
  m_call_ctxt_table_size = 0 ;
  m_scenario_control = NULL ;
  m_call_ctxt_mlist = NULL ;
  m_stat = C_GeneratorStats::instance() ;
  
  m_config = NULL ;
  m_max_send_loop = 0 ;
  m_max_receive_loop = 0 ;
  m_call_timeout_ms = 0 ; 

  m_accept_new_call = true ;

  m_call_created = 0 ;
  m_pause = false ;

  // m_type = E_TRAFFIC_UNKNOWN ;
  m_type = E_TRAFFIC_SERVER ;

  m_nb_wait_values = 0 ;
  m_wait_values = NULL ;

  m_call_timeout_abort = false ;
  m_open_timeout_ms = 0 ; 

  m_max_retrans             = 0     ;
  m_retrans_enabled         = false ;
  m_retrans_context_list    = NULL  ;
  m_retrans_delay_values    = NULL  ;
  m_nb_retrans_delay_values = 0     ;

  m_nb_send_per_scene = 0 ;
  m_nb_recv_per_scene = 0 ;

  NEW_VAR(m_call_suspended, T_SuspendMap());

  m_config = P_config ;
  m_scenario_control = P_scenControl ;
  m_channel_control = P_channel_ctrl ;

  m_nb_channel = m_channel_control->nb_channel();
  m_correlation_section = false ;
  m_correlation_method = &C_CallControl::getSessionFromDico ;

  ALLOC_TABLE(m_call_map_table,
	      C_CallContext::T_pCallMap*,
	      sizeof(C_CallContext::T_pCallMap),
	      m_nb_channel);
  for(L_i=0; L_i < m_nb_channel; L_i++) {
    NEW_VAR(m_call_map_table[L_i], C_CallContext::T_CallMap());
  }

  GEN_DEBUG (1, "C_CallControl::C_CallControl() end");
}

C_CallContext::T_pCallMap* C_CallControl::get_call_map () {
  return (m_call_map_table) ;
}

void C_CallControl::start_traffic() {
}


void C_CallControl::increase_incoming_call() {
}

void C_CallControlClient::increase_incoming_call() {
}

void C_CallControlServer::increase_incoming_call() {
  m_call_created++ ;
}


C_CallControl::~C_CallControl() {

  int L_i ;

  GEN_DEBUG (1, "C_CallControl::~C_CallControl() start");

  DELETE_VAR(m_msg_rcv_ctxt_list);
  DELETE_VAR(m_event_list);
  FREE_TABLE(m_call_ctxt_table);
  m_call_ctxt_table_size = 0 ;
  m_scenario_control = NULL ;
  DELETE_VAR(m_call_ctxt_mlist);
  m_stat = NULL ;

  m_config = NULL ;
  m_max_send_loop = 0 ;
  m_max_receive_loop = 0 ;
  m_pause = false ;

  FREE_TABLE(m_wait_values);
  m_nb_wait_values = 0 ;

  m_config = NULL ;
  m_scenario_control = NULL ;

  m_call_timeout_ms = 0 ; 
  m_open_timeout_ms = 0 ; 

  m_max_retrans = 0 ;
  m_retrans_enabled = false ;

  m_nb_send_per_scene = 0 ;
  m_nb_recv_per_scene = 0 ;

  m_correlation_section = false ;
  m_correlation_method = &C_CallControl::getSessionFromDico ;

  if (m_nb_retrans_delay_values > 0 ) {
    for(L_i=0; L_i < (int)m_nb_retrans_delay_values; L_i++) {
      if (!m_retrans_context_list[L_i]->empty()) {
        m_retrans_context_list[L_i]->erase(m_retrans_context_list[L_i]->begin(),
                                          m_retrans_context_list[L_i]->end());
      }
      DELETE_VAR(m_retrans_context_list[L_i]);
    }
    FREE_TABLE(m_retrans_context_list);
  }

  FREE_TABLE(m_retrans_delay_values);
  m_nb_retrans_delay_values = 0;

  m_call_timeout_abort = false ;

  for(L_i=0; L_i < m_nb_channel; L_i++) {
    if (!m_call_map_table[L_i]->empty()) {
      m_call_map_table[L_i]->erase(m_call_map_table[L_i]->begin(),
				   m_call_map_table[L_i]->end());
    }
    DELETE_VAR(m_call_map_table[L_i]);
  }
  FREE_TABLE(m_call_map_table);
  GEN_DEBUG (1, "C_CallControl::~C_CallControl() end");
}

void C_CallControl::init_done() { // SERVER traffic case
  GEN_DEBUG (1, "C_CallControl::init_done() start");
  m_scenario_control->switch_to_traffic();
  GEN_DEBUG (1, "C_CallControl::init_done() end");
}

void C_CallControlClient::init_done() { // CLIENT traffic case
  GEN_DEBUG (1, "C_CallControlClient::init_done() start");
  m_outgoing_traffic = true ;
  m_traffic_model->start();
  C_CallControl::init_done() ;
  GEN_DEBUG (1, "C_CallControlClient::init_done() end");
}


void C_CallControl::eventReceived (T_pEventRecv P_event) {
  GEN_DEBUG (1, "C_CallControl::openEvent() start");
  m_event_list->push_back(*P_event);
  GEN_DEBUG (1, "C_CallControl::openEvent() end");
}

void C_CallControl::messageReceived (T_pReceiveMsgContext P_ctxt) {
  GEN_DEBUG (1, "C_CallControl::messageReceived() start");
  m_stat -> executeStatAction (C_GeneratorStats::E_RECV_MSG);
  m_msg_rcv_ctxt_list -> push_back (*P_ctxt) ;
  GEN_DEBUG (1, "C_CallControl::messageReceived() end");
}

T_pCallContext C_CallControl::makeCallContextUnavailable (C_Scenario *P_scen) {

  T_pCallContext     L_pCallContext = NULL ;
  int                L_callContextIdx      ;
  T_CallContextState L_state               ;


  if (m_call_ctxt_mlist->getNbElements(E_CTXT_AVAILABLE)!= 0) {
    
    // get a free context
    L_callContextIdx = m_call_ctxt_mlist->getFirst(E_CTXT_AVAILABLE);
    L_pCallContext = m_call_ctxt_table[L_callContextIdx];

    L_state = L_pCallContext -> init_state (P_scen) ;
    m_call_ctxt_mlist -> moveToList(L_state, L_callContextIdx);
    
    // the call id is not setted: just mask the call as created 
    // => the scenario must set the call id 
    L_pCallContext -> m_created_call = true ;
    
    // the new call is now created
    m_stat -> executeStatAction (C_GeneratorStats::E_CREATE_OUTGOING_CALL);
  } 
  
  return (L_pCallContext) ;
}


void C_CallControlClient::start_traffic() {

  T_pC_Scenario  L_scenario ;
  T_TrafficType  L_type ;
  T_pCallContext L_call_ctxt ;

  L_scenario = m_scenario_control->init_scenario_defined(&L_type) ;
  if (L_scenario != NULL) {
    GEN_DEBUG (1, "C_CallControl::InitProcedure() E_TRAFFIC_CLIENT");
    L_call_ctxt = C_CallControl::makeCallContextUnavailable(L_scenario);
    if (L_call_ctxt == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
		"No context available to execute init scenario");
    }
  } else {
    m_outgoing_traffic = true ;
    m_traffic_model->start();
  }
}

T_pCallContext C_CallControlClient::makeCallContextUnavailable () {

  T_pCallContext     L_pCallContext = NULL ;
  int                L_callContextIdx ;
  T_CallContextState L_state ;


  if (m_call_ctxt_mlist->getNbElements(E_CTXT_AVAILABLE)!= 0) {
    
    // get a free context
    L_callContextIdx = m_call_ctxt_mlist->getFirst(E_CTXT_AVAILABLE);
    L_pCallContext = m_call_ctxt_table[L_callContextIdx] ;

    L_state = L_pCallContext -> init_state (m_traffic_scen);
    m_call_ctxt_mlist -> moveToList(L_state, L_callContextIdx);
    
    // the call id is not setted: just mask the call as created 
    // => the scenario must set the call id 
    L_pCallContext -> m_created_call = true ;
    
    // the new call is now created
    m_traffic_model->call_created() ;
    m_stat -> executeStatAction (C_GeneratorStats::E_CREATE_OUTGOING_CALL);
  } 
  
  return (L_pCallContext) ;
}

void C_CallControl::makeCallContextAvailable (T_pCallContext *P_pCallCtxt) {
  
  int                 L_id ;
  T_pCallContext      L_callCtxt ;
  C_CallContext::T_CallMap::iterator L_call_it ;
  int                 L_i ;

  C_CallContext::T_pContextMapDataList L_map_data_list;
  C_CallContext::T_contextMapDataList::iterator L_it ;

  
  GEN_DEBUG (1, "C_CallControl::makeCallContextAvailable() start");

  L_callCtxt = *P_pCallCtxt ;

  L_id = L_callCtxt -> get_internal_id();


  L_map_data_list = L_callCtxt->m_map_data_list ;
  if (!L_map_data_list->empty()){
    for(L_it = L_map_data_list->begin();
        L_it != L_map_data_list->end();
        L_it ++) {
      m_call_map_table[L_it->m_channel]
        ->erase(L_it->m_iterator) ;
    }
    L_map_data_list->erase(L_map_data_list->begin(),
                           L_map_data_list->end());
  }
  

  // remove call context from map
  for(L_i=0; L_i < m_nb_channel; L_i++) {
    L_call_it = m_call_map_table[L_i]
      ->find (C_CallContext::T_CallMap::key_type(L_callCtxt->m_id_table[L_i]));
    if (L_call_it != m_call_map_table[L_i]->end()) {
      m_call_map_table[L_i]->erase (L_call_it);
    } 
  }

  // make call context available for a new call
  L_callCtxt->init();
  m_channel_control->reset_channel(L_callCtxt->m_channel_table);
  m_call_ctxt_mlist->moveToList(E_CTXT_AVAILABLE, L_id);

  *P_pCallCtxt = NULL ;

  GEN_DEBUG (1, "C_CallControl::makeCallContextAvailable() end");

}

void C_CallControl::messageReceivedControl () {
  
  int            L_nbMsgReceived, L_nbMsg, L_i ;

  C_MessageFrame     *L_msg = NULL ; 
  T_ReceiveMsgContext L_rcvCtxt ;

  T_pCallContext      L_pCallContext = NULL ;
  T_pC_Scenario       L_scenario = NULL ;
  T_exeCode           L_exeResult ;

  int                 L_callContextIdx ;

  T_pValueData   L_value_id = NULL ;
  
  GEN_DEBUG (1, "C_CallControl::messageReceivedControl() start");

  L_nbMsgReceived = m_msg_rcv_ctxt_list -> size();

  L_nbMsg = (L_nbMsgReceived > m_max_receive_loop) 
    ? m_max_receive_loop : L_nbMsgReceived;
  
  for(L_i=0; L_i < L_nbMsg; L_i++) {

    L_rcvCtxt = *(m_msg_rcv_ctxt_list -> begin()) ;
    m_msg_rcv_ctxt_list -> erase (m_msg_rcv_ctxt_list->begin()) ;

    L_msg = L_rcvCtxt.m_msg ;
    
    L_pCallContext = ((this)->*(m_correlation_method))(L_rcvCtxt, &L_value_id);

    if (L_pCallContext == NULL) {
      // new call management
      // search for scenario
      GEN_DEBUG(1, "C_CallControl::messageReceivedControl() L_pCallContext == NULL");

      if (m_accept_new_call == true) {
	GEN_DEBUG(1, "C_CallControl::messageReceivedControl()  m_accept_new_call == true");
	L_scenario = m_scenario_control -> find_scenario (&L_rcvCtxt);
	if (L_scenario != NULL) { 
	  // a scenario is found for the
	  // message received
	  if (m_call_ctxt_mlist->getNbElements(E_CTXT_AVAILABLE)!= 0) {
	    // get a free call context
	    L_callContextIdx 
	      = m_call_ctxt_mlist->getFirst(E_CTXT_AVAILABLE);
	    L_pCallContext = m_call_ctxt_table[L_callContextIdx];
	    L_pCallContext -> init_state (L_scenario, &L_rcvCtxt);
            // context inserted in the map by a post-action	    
	    // m_call_ctxt_mlist->moveToList (E_CTXT_RECEIVE, L_callContextIdx) ; 
            // move to list 2 times
            if (L_scenario->get_exe_end_code() != E_EXE_IGNORE ) {
              m_stat -> executeStatAction (C_GeneratorStats::E_CREATE_INCOMING_CALL);
              increase_incoming_call() ;
            }
	  } else {
	    // no more call context available
	    // => just discard the message
	    // => call refused
	    GEN_ERROR(1, "No more context available");
	    L_pCallContext = NULL ;
	    GEN_LOG_EVENT_CONDITIONAL (LOG_LEVEL_TRAFFIC_ERR, 
                                       L_value_id != NULL,
                                       "Refused (no more context) call with session-id ["
                                       << *L_value_id << "]");
	    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_REFUSED) ;
	  }
	} else {
	  // no scenario found
	  // for the message received
	  // => discard the message
	  // => message unexpected
	  //	  GEN_ERROR(1,"No scenario found");

	  GEN_ERROR(1,"Unexpexted message that doesn't match the scenario.");

	  if (!(genTraceLevel & gen_mask_table[LOG_LEVEL_TRAFFIC_ERR])) {
	    GEN_ERROR(1,"Activate 'T' log level");
	  }

	  L_pCallContext = NULL ;

	  GEN_LOG_EVENT_CONDITIONAL (LOG_LEVEL_TRAFFIC_ERR,
                                     L_value_id != NULL,
                                     "Unexpected (no scenario found) call with session-id ["
                                     << *L_value_id << "]");

	  // dump message Traffic 
	  GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
			"Unexpected message received [ " << (*L_msg) <<
			GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "]" );

	  m_stat -> executeStatAction (C_GeneratorStats::E_FAILED_UNEXPECTED_MSG);
	}
      } else {
	// new call refused
	// discard message
	GEN_LOG_EVENT_CONDITIONAL (LOG_LEVEL_TRAFFIC_ERR, 
		   L_value_id != NULL,
		   "Refused (new) call with session-id ["
		   << *L_value_id << "]");
	m_stat -> executeStatAction (C_GeneratorStats::E_CALL_REFUSED) ;
      }
    } else {
      // scenario in execution for this call
      GEN_DEBUG(1, "C_CallControl::messageReceivedControl() "<<
		"scenario in execution for this call");

      if (m_retrans_enabled) {
        stopRetrans(L_pCallContext);
      }


      if (L_pCallContext -> msg_received (&L_rcvCtxt) == false) {
	// scenario in execution but not in receive state
	GEN_LOG_EVENT_CONDITIONAL (LOG_LEVEL_TRAFFIC_ERR, 
				   L_value_id != NULL,
                                   // "Incorrect state (not receive) for call with session-id ["
                                   "Received a message (while not in receive state) for call with session-id ["
                                   << *L_value_id << "]");

        // GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
        //            "The following message is [ " << (*L_msg) <<
        //            GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "]" );

	m_stat -> executeStatAction (C_GeneratorStats::E_FAILED_UNEXPECTED_MSG);

	// try to find default scenario if possible
	L_scenario = m_scenario_control->find_default_scenario (&L_rcvCtxt) ;
	if (L_scenario == NULL) {
	  // don t know how to do with this msg
	  L_scenario = m_scenario_control->get_abort_scenario();
	  if (L_scenario != NULL) {
	    GEN_LOG_EVENT (LOG_LEVEL_TRAFFIC_ERR, 
			   "Switching to abort scenario");
	    L_pCallContext -> switch_to_scenario (L_scenario);
	    m_call_ctxt_mlist
	      ->moveToList(L_pCallContext->get_state(), 
			   L_pCallContext->get_internal_id());
	    L_pCallContext = NULL ; // do not execute the command
	  } else {
	    GEN_LOG_EVENT (LOG_LEVEL_TRAFFIC_ERR, 
			   "Unable to find an abort/default scenario");
	    makeCallContextAvailable(&L_pCallContext); // setted to NULL
	    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
	  }
	} else {
	  T_CallContextState L_state ;
	  GEN_LOG_EVENT (LOG_LEVEL_TRAFFIC_ERR, 
			 "Switching to default scenario");
	  // ok let s execute the default scenario
	  L_state = L_pCallContext->get_state() ;
	  L_pCallContext -> switch_to_scenario (L_scenario);
	  if (L_state != L_pCallContext->get_state()) {
	    m_call_ctxt_mlist
	      ->moveToList(L_pCallContext->get_state(), 
			   L_pCallContext->get_internal_id());
	  }
          (void)L_pCallContext -> msg_received (&L_rcvCtxt);
	} 
      }
    }
      
    if (L_pCallContext != NULL) {
      // state (receive) OK => process the call
      L_exeResult = execute_scenario_cmd (L_pCallContext) ;
      if (L_exeResult == E_EXE_ERROR_MSG) {
	// try to find default scenario if possible
	L_scenario = m_scenario_control->find_default_scenario (&L_rcvCtxt) ;
	if (L_scenario == NULL) {
	  // don t know how to do with this msg
	  L_scenario = m_scenario_control->get_abort_scenario();
	  if (L_scenario != NULL) {
	    L_pCallContext -> switch_to_scenario (L_scenario);
	    // abort scenario is a client one
	    m_call_ctxt_mlist
	      ->moveToList(L_pCallContext->get_state(), 
			   L_pCallContext->get_internal_id());
	  } else {
	    makeCallContextAvailable(&L_pCallContext);
	    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
	  }
	} else { // a default scenario exist for this msg
	  L_pCallContext -> switch_to_scenario (L_scenario);
	  execute_scenario_cmd (L_pCallContext);
	}
      }
    }

    DELETE_VAR(L_msg);
  }
  GEN_DEBUG (1, "C_CallControl::messageReceivedControl() end");
}

void C_CallControl::endTrafficControl() {

  int L_i; 
  int L_remaining_calls = 0;

  GEN_DEBUG (1, "C_CallControl::endTrafficControl() start");

  switch (M_state) {
  case C_TaskControl::E_STATE_RUNNING:
    if (m_call_to_simulate) {
      if (m_call_created >= m_call_to_simulate) {
	stop();
      }
    }
    break ;
  case C_TaskControl::E_STATE_STOPPING:
    for(L_i = 0 ; L_i < m_nb_channel ; L_i++) {
      L_remaining_calls = m_call_map_table[L_i]->size() ;
      if (L_remaining_calls != 0) break ;
    }
    if (L_remaining_calls == 0) {
      M_state = C_TaskControl::E_STATE_STOPPED ;
    }
    break ;
  default:
    break ;
  }

  GEN_DEBUG (1, "C_CallControl::endTrafficControl() end");
}

void C_CallControl::insert_retrans_list(T_pCallContext  P_callContext) {

  int                               L_index, L_retrans_index ;

  GEN_DEBUG (1, "C_CallControl::insert_retrans_list() start");
  
  L_index = P_callContext->m_retrans_context.m_retrans_delay_index ;
  L_retrans_index = P_callContext->m_retrans_context.m_retrans_index ;

  m_retrans_context_list[L_index]->push_back(P_callContext->m_retrans_context);
  P_callContext->m_retrans_it[L_retrans_index] = m_retrans_context_list[L_index]->end() ;
  P_callContext->m_retrans_it[L_retrans_index] -- ;
  P_callContext->m_retrans_it_available[L_retrans_index] = true ; 

  GEN_DEBUG (1, "C_CallControl::insert_retrans_list() end");
}

void C_CallControl::stopRetrans (T_pCallContext P_callContext) {
  
  int L_i, L_cmdIdx ;
  int L_retrans_delay_index ;


  GEN_DEBUG (1, "C_CallControl::stopRetrans() start");

  for (L_i = 0 ; L_i < P_callContext->m_nb_retrans; L_i++) {
    if (P_callContext->m_retrans_it_available[L_i] == true) {
      L_cmdIdx = P_callContext->m_retrans_cmd_idx[L_i];
      L_retrans_delay_index = 
        (P_callContext->get_scenario()->get_commands())[L_cmdIdx].m_retrans_delay_index ;
      m_retrans_context_list[L_retrans_delay_index]->erase(P_callContext->m_retrans_it[L_i]) ;
      P_callContext->m_retrans_it_available[L_i] = false ;
    }
  }

  GEN_DEBUG (1, "C_CallControl::stopRetrans() end");
}


void C_CallControl::messageRetransControl () {

  GEN_DEBUG (1, "C_CallControl::messageRetransControl() start");
  
  int                                L_i            ;
  int                                L_retrans_idx  ;
  int                                L_nbRetrans, L_nbRetransToDo ;
  struct timeval                     L_current_time ;
  C_CallContext                     *L_pCallContext ;
  C_CallContext::T_retransContext    L_retrans_ctxt ;
  C_CallContext::T_retransContextList::iterator L_it;

  list_t<C_CallContext*>             L_stopRetransList ;
  list_t<C_CallContext*>::iterator   L_stopRetransListIt ;

  list_t<C_CallContext::T_retransContextList::iterator> L_deleteList ;
  list_t<C_CallContext::T_retransContextList::iterator>::iterator L_deleteListIt ;

  C_CallContext::T_retransContextList L_doRetransList ;
  C_CallContext::T_retransContextList::iterator L_doRetransListIt ;

  bool                               L_first_get = false ;

  L_stopRetransList.clear();
  L_doRetransList.clear();
  L_deleteList.clear() ;
  
  for (L_i=0; L_i < (int)m_nb_retrans_delay_values; L_i++) {
    if (!(m_retrans_context_list[L_i]->empty())) {
      L_nbRetrans = m_retrans_context_list[L_i]->size();
      L_nbRetransToDo = (L_nbRetrans > m_max_send_loop) 
        ? m_max_send_loop : L_nbRetrans ;
      
      GEN_DEBUG (1, "C_CallControl::messageRetransControl() L_nbRetrans " << L_nbRetrans);
      if (L_nbRetrans) { 
        if (L_first_get == false) { 
          GET_TIME(&L_current_time); 
          L_first_get = true ; 
        }
      }

      for (L_it = m_retrans_context_list[L_i]->begin() ; 
           L_it != m_retrans_context_list[L_i]->end() ;
           L_it++) {
        
        L_retrans_ctxt = *L_it;
        L_pCallContext = L_retrans_ctxt.m_context ;
        L_retrans_idx = L_retrans_ctxt.m_retrans_index ;
        
        if (  ms_difftime(&L_current_time, &L_pCallContext->m_retrans_time[L_retrans_idx])
              < (long) m_retrans_delay_values[L_i] ) { 
          break ; 
        } else { // retrans to do 
          if (L_pCallContext->m_nb_retrans_done[L_retrans_idx] <= (int)m_max_retrans) {
            execute_scenario_cmd_retrans (L_retrans_idx, L_pCallContext) ;
            L_doRetransList.push_back(L_retrans_ctxt);
            L_deleteList.push_back(L_it);
          } else {
            L_stopRetransList.push_back(L_pCallContext) ;
          }
        }

        L_nbRetransToDo -- ;
        if (L_nbRetransToDo == 0) break ;

      }

      if (!L_deleteList.empty()) {
        for (L_deleteListIt = L_deleteList.begin();
             L_deleteListIt != L_deleteList.end();
             L_deleteListIt ++) {
          m_retrans_context_list[L_i]->erase(*L_deleteListIt); 
        }
      }
    }

    if (!L_stopRetransList.empty()) {
      for (L_stopRetransListIt = L_stopRetransList.begin();
           L_stopRetransListIt != L_stopRetransList.end();
           L_stopRetransListIt++) {
        L_pCallContext = *L_stopRetransListIt ;
        stopRetrans(L_pCallContext);
        makeCallContextAvailable(&L_pCallContext) ;
        m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
      }
      L_stopRetransList.erase(L_stopRetransList.begin(),
                              L_stopRetransList.end());
    }

    if (!L_doRetransList.empty()) {
      for (L_doRetransListIt = L_doRetransList.begin();
           L_doRetransListIt != L_doRetransList.end();
           L_doRetransListIt++) {
        L_retrans_ctxt = *L_doRetransListIt;
        m_retrans_context_list[L_i]->push_back(L_retrans_ctxt);
        L_pCallContext = L_retrans_ctxt.m_context ;
        L_pCallContext->m_retrans_it[L_retrans_ctxt.m_retrans_index] = 
          m_retrans_context_list[L_i]->end();
        (L_pCallContext->m_retrans_it[L_retrans_ctxt.m_retrans_index])--;
      }
      L_doRetransList.erase(L_doRetransList.begin(),
                            L_doRetransList.end());
    }
  }

  GEN_DEBUG (1, "C_CallControl::messageRetranstControl() end");
}

T_exeCode C_CallControl::execute_scenario_cmd_retrans (int P_index, T_pCallContext  P_callContext) {
  T_pC_Scenario  L_scenario                    ;
  T_exeCode      L_exeResult                   ;
  T_pCallContext L_callContext = P_callContext ;

  GEN_DEBUG (1, "C_CallControl::execute_scenario_cmd_retrans() start");
  
  L_scenario = L_callContext->get_scenario() ;
  // ctrl to max_retrans and delay
  L_exeResult = L_scenario->execute_cmd_retrans (P_index, L_callContext);

  switch (L_exeResult) {
  case E_EXE_NOERROR:
    break ;
  case E_EXE_ERROR_SEND:
    makeCallContextAvailable(&L_callContext) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_FAILED_CANNOT_SEND_MSG);
    m_stat -> err_msg ((char*) "Send error");
    break ;
  default: 
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
    makeCallContextAvailable(&L_callContext) ;
    break ;
  }
  GEN_DEBUG (1, "C_CallControl::execute_scenario_cmd_retrans() end");
  return (L_exeResult);
}

void C_CallControl::messageSendControl() {
  int            L_nbSend, L_nbSendToDo ;
  T_pCallContext L_pCallContext ;


  GEN_DEBUG (1, "C_CallControl::messageSendControl() start");
  L_nbSend = m_call_ctxt_mlist -> getNbElements (E_CTXT_SEND) ;
  L_nbSendToDo = (L_nbSend > m_max_send_loop) 
    ? m_max_send_loop : L_nbSend ;
  while (L_nbSendToDo > 0) {
    L_pCallContext 
      = m_call_ctxt_table[m_call_ctxt_mlist->getFirst(E_CTXT_SEND)];
    execute_scenario_cmd (L_pCallContext);
    
    if (m_retrans_enabled) {
      if ((L_pCallContext->m_retrans_to_do) == true) {
        insert_retrans_list(L_pCallContext) ;
        L_pCallContext->m_retrans_to_do = false ;
      }
    } 

    L_nbSendToDo -- ;
  }
  GEN_DEBUG (1, "C_CallControl::messageSendControl() end");
}

T_exeCode C_CallControl::execute_scenario_cmd (T_pCallContext  P_callContext,
					       bool            P_resume) {

  T_pC_Scenario  L_scenario                    ;
  T_exeCode      L_exeResult                   ;
  T_pCallContext L_callContext = P_callContext ;

  GEN_DEBUG (1, "C_CallControl::execute_scenario_cmd() start");
  
  L_scenario = L_callContext->get_scenario() ;
  L_exeResult = L_scenario->execute_cmd (L_callContext, P_resume);

  switch (L_exeResult) {

  case E_EXE_NOERROR:
    m_call_ctxt_mlist
      ->moveToList(L_callContext->get_state(), 
		   L_callContext->get_internal_id());
    break ;


   case E_EXE_TRAFFIC_END:
    makeCallContextAvailable(&L_callContext) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_TRAFFIC_SUCCESSFULLY_ENDED);
    break ;

   case E_EXE_ABORT_END:
    makeCallContextAvailable(&L_callContext) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_ABORT_SUCCESSFULLY_ENDED);
    break ;

   case E_EXE_DEFAULT_END:
    makeCallContextAvailable(&L_callContext) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_DEFAULT_SUCCESSFULLY_ENDED);
    break ;

    
//    case E_EXE_END:
//      // correct end of the scenario 
//      // => make call context available 
//      makeCallContextAvailable(&L_callContext) ;
//      m_stat -> executeStatAction (C_GeneratorStats::E_CALL_SUCCESSFULLY_ENDED);
    //    break ;

  case E_EXE_ERROR_SEND:
    makeCallContextAvailable(&L_callContext) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_FAILED_CANNOT_SEND_MSG);
    m_stat -> err_msg ((char*) "Send error");
    break ;

  case E_EXE_INIT_END:
    makeCallContextAvailable(&L_callContext) ;
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_INIT_SUCCESSFULLY_ENDED);
    init_done() ;
    break ;

  case E_EXE_ERROR_MSG:
    m_stat -> executeStatAction (C_GeneratorStats::E_FAILED_UNEXPECTED_MSG);
    // try to switch to default scenarii
    break ;

  case E_EXE_ERROR_CHECK:
    m_stat -> executeStatAction (C_GeneratorStats::E_FAILED_UNEXPECTED_MSG);
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
    makeCallContextAvailable(&L_callContext) ;
    break ;

  case E_EXE_ABORT_CHECK: {

    T_pC_Scenario       L_scenario = NULL ;
    L_scenario = m_scenario_control->get_abort_scenario();
    if (L_scenario != NULL) {
      GEN_LOG_EVENT (LOG_LEVEL_TRAFFIC_ERR, 
                     "Switching to abort scenario");
      L_callContext -> switch_to_scenario (L_scenario);
      // abort scenario is a client one
      m_call_ctxt_mlist
        ->moveToList(L_callContext->get_state(), 
                     L_callContext->get_internal_id());
    } else {
      makeCallContextAvailable(&L_callContext);
      m_stat->executeStatAction(C_GeneratorStats::E_CALL_FAILED);
    }

  }
    break ;

  case E_EXE_SUSPEND:
    makeCallContextSuspended(L_callContext);
    break ;


  case E_EXE_IGNORE:
    makeCallContextAvailable(&L_callContext) ;
    break ;


  default: 
    m_stat -> executeStatAction (C_GeneratorStats::E_CALL_FAILED) ;
    makeCallContextAvailable(&L_callContext) ;
    break ;
    
  }

  GEN_DEBUG (1, "C_CallControl::execute_scenario_cmd() end");

  return (L_exeResult);
}

T_GeneratorError C_CallControl::TaskProcedure() {

  GEN_DEBUG (1, "C_CallControl::TaskProcedure() start");
  if (m_call_timeout_ms) { 
    messageTimeoutControl() ; 
  }

  if (m_open_timeout_ms) { 
    messageOpenTimeoutControl();
  }

  if (m_retrans_enabled) {
    messageRetransControl () ;
  }

  messageReceivedControl () ;
  if (m_nb_wait_values) { waitControl() ; }
  eventControl () ;
  messageSendControl () ;
  endTrafficControl() ;
  GEN_DEBUG (1, "C_CallControl::TaskProcedure() end");

  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_CallControl::InitProcedure() {

  T_GeneratorError L_error = E_GEN_NO_ERROR ;
  int            L_i ;
  int            L_memory_used, L_channel_used, L_nb_retrans ;
  C_CallContext *L_pCallContext ;
  unsigned long  L_config_value ;
  
  T_pC_Scenario  L_scenario ;
  //  T_pCallContext L_call_ctxt ;
  T_TrafficType  L_type ;

  T_pWaitValuesSet         L_wait_values          ;

  T_pRetransDelayValuesSet L_retrans_delay_values ;
  
  GEN_DEBUG (1, "C_CallControl::InitProcedure() start");

  if (!m_config->get_value(E_CFG_OPT_MAX_SIMULTANEOUS_CALLS, 
			   &L_config_value)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Internal max simultaneous call not specified");
  }
  m_call_ctxt_table_size = (size_t)L_config_value ;

  if (!m_config->get_value(E_CFG_OPT_MAX_SEND,
			   &L_config_value)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Internal max send not specified");
  }
  m_max_send_loop = (int) L_config_value ;

  if (!m_config->get_value(E_CFG_OPT_MAX_RECEIVE,
			   &L_config_value)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Internal max receive not specified");
  }
  m_max_receive_loop = (int) L_config_value ;

  if (!m_config->get_value(E_CFG_OPT_CALL_TIMEOUT,
			   &m_call_timeout_ms)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Internal call timeout (ms) not specified");
  }

  if (!m_config->get_value(E_CFG_OPT_NUMBER_CALLS,
			   &m_call_to_simulate)) {
    GEN_FATAL(E_GEN_FATAL_ERROR,
	  "Internal number calls not specified");
  }

  if (!m_config->get_value(E_CFG_OPT_OPEN_TIMEOUT,
			   &m_open_timeout_ms)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Internal open timeout (ms) not specified");
  }

  m_call_timeout_abort = m_config -> get_call_timeout_beh_abr ();

  if (!m_config->get_value(E_CFG_OPT_MAX_RETRANS,
			   &m_max_retrans)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Internal max retrans is not specified");
  }

  m_retrans_enabled = m_config -> get_retrans_enabled();

  // wait values management
  L_wait_values = m_scenario_control->get_wait_values() ;
  m_nb_wait_values = L_wait_values -> size() ;
  if (m_nb_wait_values != 0) {

    T_waitValuesSet::iterator L_waitIt ;
    int                       L_i      ;

    ALLOC_TABLE(m_wait_values, 
		unsigned long*, 
		sizeof(unsigned long), 
	        m_nb_wait_values);
    L_i = 0 ;
    for (L_waitIt = L_wait_values->begin();
	 L_waitIt != L_wait_values->end() ;
	 L_waitIt++) {
      m_wait_values[L_i] = *L_waitIt ;
      L_i ++ ;
    }
    NEW_VAR(m_call_ctxt_mlist,
	    T_CallContextList(E_CTXT_NBSTATE+(m_nb_wait_values-1), 
			      (long)m_call_ctxt_table_size));
    m_scenario_control -> update_wait_cmd (m_nb_wait_values, m_wait_values);
  } else {
    NEW_VAR(m_call_ctxt_mlist,
	    T_CallContextList(E_CTXT_NBSTATE, 
			      (long)m_call_ctxt_table_size));
  }
  m_call_ctxt_mlist->initList (E_CTXT_AVAILABLE);

  ALLOC_TABLE(m_call_ctxt_table,
	      T_pCallContext*,
	      sizeof(T_pCallContext),
	      m_call_ctxt_table_size);

  L_memory_used = m_scenario_control->memory_used() ;
  L_channel_used = m_channel_control->nb_channel() ;
  L_nb_retrans = m_scenario_control->get_max_nb_retrans();

  if (m_retrans_enabled == false) {
    L_nb_retrans = 0 ; 
  }

  m_correlation_section = m_scenario_control->get_correlation_section();
  if (m_correlation_section == true) {
    m_correlation_method = &C_CallControl::getSessionFromScen ;
  } 

  m_nb_send_per_scene = m_scenario_control->get_max_nb_send () ; 
  m_nb_recv_per_scene = m_scenario_control->get_max_nb_recv () ; 

  for(L_i = 0; L_i < (int)m_call_ctxt_table_size; L_i++) {
    NEW_VAR(L_pCallContext, C_CallContext(this,
                                          L_i,
					  L_channel_used,
					  L_memory_used,
                                          L_nb_retrans));
    m_call_ctxt_table[L_i] = L_pCallContext ;
    m_call_ctxt_mlist->setElementPayload((long)L_i, L_pCallContext);
  }

  // test if an init scenario is defined
  L_scenario = m_scenario_control->init_scenario_defined(&L_type) ;

  if (m_retrans_enabled) {
    L_retrans_delay_values = m_scenario_control->get_retrans_delay_values() ;
    m_nb_retrans_delay_values = L_retrans_delay_values -> size() ;

    if (m_nb_retrans_delay_values != 0) {
      
      T_retransDelayValuesSet::iterator L_retransDelayIt ;
      int                               L_i              ;
      
      ALLOC_TABLE(m_retrans_delay_values, 
                  unsigned long*, 
                  sizeof(unsigned long), 
                  m_nb_retrans_delay_values);
      L_i = 0 ;
      for (L_retransDelayIt = L_retrans_delay_values->begin();
           L_retransDelayIt != L_retrans_delay_values->end() ;
           L_retransDelayIt++) {
        m_retrans_delay_values[L_i] = *L_retransDelayIt ;
        L_i ++ ;
      }

      ALLOC_TABLE(m_retrans_context_list, 
                  C_CallContext::T_pRetransContextList*, 
                  sizeof(C_CallContext::T_pRetransContextList), 
                  m_nb_retrans_delay_values);
      for (L_i = 0 ; L_i < (int)m_nb_retrans_delay_values; L_i++) {
        NEW_VAR(m_retrans_context_list[L_i], 
                C_CallContext::T_retransContextList());
      }


      m_scenario_control -> update_retrans_delay_cmd (m_nb_retrans_delay_values, 
                                                      m_retrans_delay_values);
    }

  }

  m_stat->init();
//    if (L_scenario == NULL) {
//      GEN_WARNING("no init scenario defined");
//    } else {
//      // m_type = L_type ;
//      switch(L_type) {
    
//      case E_TRAFFIC_CLIENT:
//        GEN_DEBUG (1, "C_CallControl::InitProcedure() E_TRAFFIC_CLIENT");
//        L_call_ctxt = makeCallContextUnavailable(L_scenario);
//        if (L_call_ctxt == NULL) {
//  	GEN_ERROR(E_GEN_FATAL_ERROR,
//  	      "No context available to execute init scenario");
//  	L_error = E_GEN_FATAL_ERROR ;
//        }
//        break ;

//      case E_TRAFFIC_SERVER:
//        GEN_DEBUG (1, "C_CallControl::InitProcedure() E_TRAFFIC_SERVER:");
//        m_scenario_control->switch_to_init();
//        break ;

//      default:
//        GEN_FATAL(E_GEN_FATAL_ERROR, "Unknown init scenario type");
//        break ;
//      }
    
//    }

  GEN_DEBUG (1, "C_CallControl::InitProcedure() end");
  return (L_error);
}

T_GeneratorError C_CallControl::EndProcedure() {
  
  int                 L_i ;
  // T_ReceiveMsgContext L_rcvCtxt ;

  int                       L_nbMessageSuspend ;
  int                       L_event_id         ;
  T_pCallContext            L_pCallContext     ;


  GEN_DEBUG (1, "C_CallControl::EndProcedure() start");

  if (!m_msg_rcv_ctxt_list->empty()) {
    m_msg_rcv_ctxt_list->erase(m_msg_rcv_ctxt_list->begin(),
			       m_msg_rcv_ctxt_list->end());
  }


  L_nbMessageSuspend = m_call_ctxt_mlist -> getNbElements (E_CTXT_SUSPEND) ;
  while (L_nbMessageSuspend > 0) {
     L_pCallContext 
       = m_call_ctxt_table[m_call_ctxt_mlist->getFirst(E_CTXT_SUSPEND)];
     // treat the socket
     L_event_id = L_pCallContext->m_suspend_id ;
     L_pCallContext->clean_suspended() ;
     m_channel_control->close_local_channel(L_pCallContext->m_channel_id,
                                            L_pCallContext->m_channel_table);
     
     m_call_ctxt_mlist->moveToList(E_CTXT_AVAILABLE, 
                                   L_pCallContext -> get_internal_id());
     
     L_nbMessageSuspend -- ;
  }

  // purge map
  if (!m_call_suspended->empty()) {
    m_call_suspended->erase(m_call_suspended->begin(), 
			    m_call_suspended->end());
  }
  DELETE_VAR (m_call_suspended);
  
  // delete contexts
  for(L_i = 0; L_i < (int)m_call_ctxt_table_size; L_i++) {
    DELETE_VAR(m_call_ctxt_table[L_i]);
  }

  GEN_DEBUG (1, "C_CallControl::EndProcedure() end");
  return (E_GEN_NO_ERROR);
}

void C_CallControl::stopServer() {
  m_accept_new_call = false ;
  m_stat->info_msg ((char*) "Stopping traffic");
}

T_GeneratorError C_CallControl::StoppingProcedure() {
  GEN_DEBUG (1, "C_CallControl::StoppingProcedure() start");
  stopServer();
  GEN_DEBUG (1, "C_CallControl::StoppingProcedure() end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_CallControl::ForcedStoppingProcedure() {

  if (M_state == C_TaskControl::E_STATE_STOPPING) {
    M_state = C_TaskControl::E_STATE_STOPPED ;
  }
  return (E_GEN_NO_ERROR);
}

void C_CallControl::waitControl () {
  int                 L_i, L_nbWaiting ;
  T_CallContextState  L_waitQueue ;
  C_CallContext      *L_pCallContext ;
  struct timeval      L_current_time ;
  bool                L_first_get = false ;

  GEN_DEBUG (1, "C_CallControl::waitControl() start with " << m_nb_wait_values);
  for (L_i=0; L_i < (int)m_nb_wait_values; L_i++) {
    L_waitQueue = E_CTXT_WAIT + L_i ;
    L_nbWaiting = m_call_ctxt_mlist->getNbElements(L_waitQueue);
    GEN_DEBUG (1, "C_CallControl::waitControl() L_nbWaining " << L_nbWaiting);
    if (L_nbWaiting) { 
      if (L_first_get == false) { 
	GET_TIME(&L_current_time); 
	L_first_get = true ; 
      }
    }
    while (L_nbWaiting != 0) {
      L_pCallContext 
	= m_call_ctxt_table[m_call_ctxt_mlist->getFirst(L_waitQueue)];
      if (  ms_difftime(&L_current_time, &L_pCallContext->m_current_time)
	    < (long) m_wait_values[L_i] ) { 
	break ; 
      } else { // wait time end => change state
	(void) execute_scenario_cmd (L_pCallContext) ;
	L_pCallContext->m_current_time = L_current_time ;
      }
      L_nbWaiting -- ;
    }
  }

  GEN_DEBUG (1, "C_CallControl::waitControl() end");
}

C_CallControlClient::C_CallControlClient(C_GeneratorConfig    *P_config, 
					 T_pC_ScenarioControl  P_scenControl,
					 C_ChannelControl     *P_channel_ctrl) 
  : C_CallControl(P_config, P_scenControl, P_channel_ctrl) {

  GEN_DEBUG (1, "C_CallControlClient::C_CallControlClient() start");
  m_traffic_scen = NULL ;
  m_update_param_traffic = NULL ;
  m_type = E_TRAFFIC_CLIENT ;

  GEN_DEBUG (1, "C_CallControlClient::C_CallControlClient() end");

}

C_CallControlClient::~C_CallControlClient() {
  GEN_DEBUG (1, "C_CallControlClient::~C_CallControlClient() start");
  m_outgoing_traffic = false ;
  m_traffic_scen = NULL ;
  DELETE_VAR(m_traffic_model);
  GEN_DEBUG (1, "C_CallControlClient::~C_CallControlClient() end");
}

T_GeneratorError C_CallControlClient::InitProcedure() {
  T_GeneratorError L_ret ;
  T_TrafficType    L_type ;

  GEN_DEBUG (1, "C_CallControlClient::InitProcedure() start");
  // get call rate and burst from configuration
  if (!m_config->get_value(E_CFG_OPT_CALL_RATE,
			   (unsigned long*)&m_call_rate)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Internal call rate not specified");
  }
  if (!m_config->get_value(E_CFG_OPT_BURST_LIMIT,
			   (unsigned long*)&m_burst_limit)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Internal burst limit not specified");
  }
  if (!m_config->get_value(E_CFG_OPT_CALL_RATE_SCALE,
			   (unsigned long*)&m_call_rate_scale)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Internal call rate scale not specified");
  }

   if (!m_config->get_value (E_CFG_OPT_MODEL_TRAFFIC_SELECT, 
                             &m_model_traffic_select)) {
      GEN_FATAL(E_GEN_FATAL_ERROR, "Model traffic not specified");
   }

   switch (m_model_traffic_select) {
   case 0:
     NEW_VAR(m_traffic_model, C_TrafficDistribUniform());
     m_update_param_traffic = &C_CallControlClient::calculUpdateParamTraffic ;
     break ;
   case 1:
     NEW_VAR(m_traffic_model, C_TrafficDistribBestEffort());
     m_update_param_traffic = &C_CallControlClient::calculNilParamTraffic  ;
     break ;
   case 2:
     NEW_VAR(m_traffic_model, C_TrafficDistribPoisson());
     m_update_param_traffic = &C_CallControlClient::calculNilParamTraffic   ;
     break ;
     
   default:
     GEN_FATAL(E_GEN_FATAL_ERROR, "Internal: Selection option not recognized for traffic model");
     break ;
   }
   
   
  m_traffic_model->init(m_call_rate, m_burst_limit,(long)0, m_call_rate_scale) ;

  L_ret = C_CallControl::InitProcedure();

  m_traffic_scen = m_scenario_control->outgoing_scenario() ;
  if (m_traffic_scen == NULL) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Client scenario needed");
  }

  if (m_scenario_control->init_scenario_defined(&L_type) != NULL) {
    m_outgoing_traffic = false; // stop outgoing traffic before
                                // executon of init scenario
  } else {
    init_done() ;
  }
  GEN_DEBUG (1, "C_CallControlClient::InitProcedure() end");
  return (L_ret);
}

T_GeneratorError C_CallControlClient::TaskProcedure() {
  GEN_DEBUG (1, "C_CallControlClient::TaskProcedure() start");
  if (m_outgoing_traffic == true) { 
    newCallControl() ;
  } 
  GEN_DEBUG (1, "C_CallControlClient::TaskProcedure() end");
  return (C_CallControl::TaskProcedure());
}

void C_CallControlClient::calculNilParamTraffic() {
 // nothing to do
}

void C_CallControlClient::calculUpdateParamTraffic() {
  // to be define
  long               L_currentPeriodDuration ;
  
  L_currentPeriodDuration = m_traffic_model -> get_current_period_duration() ; 
  
  L_currentPeriodDuration = (L_currentPeriodDuration == 0) ? 1 : L_currentPeriodDuration ;
  
  m_max_send_loop    =   (int)floor (0.5 + 
                                     ((m_call_rate * m_nb_send_per_scene)/ 
                                      (float)(L_currentPeriodDuration/1000.0))) ; 
  
  m_max_receive_loop =   (int) floor (0.5 + 
                                      ((m_call_rate * m_nb_recv_per_scene)/
                                       (float)(L_currentPeriodDuration/1000.0))) ;
  
}


void C_CallControlClient::newCallControl() {
  
  int                L_nbNewCalls ; // number of new calls to be created
  T_pCallContext     L_pCallContext ;


  GEN_DEBUG (1, "C_CallControlClient::newCallControl() start");


  L_nbNewCalls = m_traffic_model -> authorize_new_call() ;
  ((this)->*(m_update_param_traffic))();

  while (L_nbNewCalls) {
    
    if (m_call_to_simulate) {
      if (m_call_created == m_call_to_simulate) break ;
    }

    L_pCallContext 
      = makeCallContextUnavailable();

    if (L_pCallContext == NULL) {
      GEN_WARNING("No more call context available");
      break ;
    }
    
    m_call_created ++ ;

    L_nbNewCalls -- ;
  }

  GEN_DEBUG (1, "C_CallControlClient::newCallControl() end");
}

T_GeneratorError C_CallControl::close () {
  //  return (EndProcedure());
  return (E_GEN_NO_ERROR);
}

void C_CallControl::eventControl() {

  int                       L_nbEventReceived, L_nbEvent ;
  T_EventRecvList::iterator L_it ;
  int                       L_id ;
  T_SuspendMap::iterator    L_itSuspend ;
  T_pCallContext            L_callContext ;
  T_EventRecv               L_event ;

  L_nbEventReceived = m_event_list->size () ;

  if (L_nbEventReceived) {

    L_nbEvent = (L_nbEventReceived > m_max_send_loop) 
      ? m_max_send_loop : L_nbEventReceived ;

    while (L_nbEvent) {
      L_it = m_event_list->begin() ;
      L_event = *L_it ;
      L_id = L_event.m_id ;
      m_event_list->erase(L_it);

      L_itSuspend = m_call_suspended->find(T_SuspendMap::key_type(L_id));
      if (L_itSuspend != m_call_suspended->end()) {
	L_callContext = L_itSuspend->second ;
	m_call_suspended->erase(L_itSuspend);
	  
	// restore execution previous state
	switch (L_event.m_type) {
	case C_TransportEvent::E_TRANS_OPEN:
	  m_call_ctxt_mlist
	    ->moveToList(L_callContext->get_state(), 
			 L_callContext->get_internal_id());
	  execute_scenario_cmd(L_callContext, true /* resume command */);
	  break ;
        case C_TransportEvent::E_TRANS_OPEN_FAILED:
	  makeCallContextAvailable(&L_callContext);
	  m_stat->executeStatAction(C_GeneratorStats::E_CALL_FAILED);
	  break ;
	default:
	  GEN_FATAL(0, "Internal error: Unexpected event");
	  break ;
	}
      }

      L_nbEvent -- ;
    }
  }

}

void C_CallControl::messageOpenTimeoutControl() {

  int                       L_nbMessageSuspend ;
  T_pCallContext            L_pCallContext     ;
  struct timeval            L_current_time     ;
  int                       L_event_id         ;
  T_SuspendMap::iterator    L_itSuspend        ;

  GEN_DEBUG(1, "C_CallControl::messageOpenTimeoutControl() start");

  L_nbMessageSuspend = m_call_ctxt_mlist -> getNbElements (E_CTXT_SUSPEND) ;

  if (L_nbMessageSuspend > 0) { GET_TIME(&L_current_time); }
  
  while (L_nbMessageSuspend > 0) {
     L_pCallContext 
       = m_call_ctxt_table[m_call_ctxt_mlist->getFirst(E_CTXT_SUSPEND)];
    if (  ms_difftime(&L_current_time, &L_pCallContext->m_current_time)
	  < (long)m_open_timeout_ms ) { 
      break ; 
    } else { 
      // treat the socket
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		    "Call open timeout detected");
      m_stat->executeStatAction(C_GeneratorStats::E_FAILED_TIMEOUT);

      L_event_id = L_pCallContext->m_suspend_id ;
      L_itSuspend = m_call_suspended->find(T_SuspendMap::key_type(L_event_id));
      if (L_itSuspend != m_call_suspended->end()) {
	m_call_suspended->erase(L_itSuspend);
      }

      L_pCallContext->clean_suspended() ;
      m_channel_control->close_local_channel(L_pCallContext->m_channel_id,
                                             L_pCallContext->m_channel_table);
      makeCallContextAvailable (&L_pCallContext) ;
    }
    L_nbMessageSuspend -- ;
  }

  GEN_DEBUG(1, "C_CallControl::messageOpenTimeoutControl() end");
  
}


void C_CallControl::messageTimeoutControl() {

  int              L_nbRecv ;
  T_pCallContext   L_pCallContext ;
  struct timeval   L_current_time ;
  C_ScenarioStats *L_stats ;

  GEN_DEBUG(1, "C_CallControl::messageTimeoutControl() start");

  L_nbRecv = m_call_ctxt_mlist -> getNbElements (E_CTXT_RECEIVE) ;

  if (L_nbRecv > 0) { GET_TIME(&L_current_time); }
  
  while (L_nbRecv > 0) {
     L_pCallContext 
       = m_call_ctxt_table[m_call_ctxt_mlist->getFirst(E_CTXT_RECEIVE)];
    if (  ms_difftime(&L_current_time, &L_pCallContext->m_current_time)
	  < (long)m_call_timeout_ms ) { 
      break ; 
    } else { // timeout call => to be aborted or closed ???
      m_stat->executeStatAction(C_GeneratorStats::E_FAILED_TIMEOUT);
      // m_stat->executeStatAction(C_GeneratorStats::E_CALL_FAILED);

      // log timeout
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		    "Call timeout detected");

      if (m_retrans_enabled) {
        stopRetrans(L_pCallContext);
      }

      // Timeout for scenario stats
      if ((L_stats = (L_pCallContext->get_scenario())->get_stats()) != NULL) {
	L_stats -> updateStats(L_pCallContext->get_current_cmd_idx(),
			       C_ScenarioStats::E_TIMEOUT,
			       0);
      }

      if (m_call_timeout_abort == true) {
        T_pC_Scenario       L_scenario = NULL ;
        L_scenario = m_scenario_control->get_abort_scenario();
        if (L_scenario != NULL) {
          L_pCallContext -> switch_to_scenario (L_scenario);
          // abort scenario
          m_call_ctxt_mlist
            ->moveToList(L_pCallContext->get_state(), 
                         L_pCallContext->get_internal_id());
        } else {

          m_stat->executeStatAction(C_GeneratorStats::E_CALL_FAILED);
          makeCallContextAvailable(&L_pCallContext);
        }
       } else {

         m_stat->executeStatAction(C_GeneratorStats::E_CALL_FAILED);
         makeCallContextAvailable (&L_pCallContext) ;
       }
      // ATTENTION STAT
      
      // makeCallContextAvailable (&L_pCallContext) ;
	     
    }
    L_nbRecv -- ;
  }

  GEN_DEBUG(1, "C_CallControl::messageTimeoutControl() end");
}

T_GeneratorError C_CallControlClient::StoppingProcedure() {
  GEN_DEBUG (1, "C_CallControl::StoppingProcedure() start");
  m_outgoing_traffic = false ;
  stopServer();
  GEN_DEBUG (1, "C_CallControl::StoppingProcedure() end");
  return (E_GEN_NO_ERROR);
}

void C_CallControl::pause_traffic() {
  GEN_DEBUG (1, "C_CallControl::pause_traffic() start");
  switch(m_pause) {
  case true:
    m_accept_new_call = true ;
    m_pause = false ;
    m_stat->info_msg((char*)"Incomming traffic (pause end)");
    break ;
  case false:
    m_accept_new_call = false ;
    m_pause = true ;
    m_stat->info_msg((char*)"Incomming traffic (paused)");
    break ;
  }
  GEN_DEBUG (1, "C_CallControl::pause_traffic() end");
}

void C_CallControl::restart_traffic() {
  pause_traffic() ;
}

void C_CallControlClient::pause_traffic() {
  GEN_DEBUG (1, "C_CallControl::pause_traffic() start");
  switch(m_pause) {
  case true :
    m_pause = false ;
    m_outgoing_traffic = true ;
    m_stat->info_msg((char*)"Outgoing traffic (pause end)");
    break ;
  case false :
    m_pause = true ;
    m_outgoing_traffic = false ;
    m_stat->info_msg((char*)"Outgoing traffic (paused)");
    break ;
  }
  GEN_DEBUG (1, "C_CallControl::pause_traffic() end");
}

void C_CallControlClient::restart_traffic() {
  GEN_DEBUG (1, "C_CallControl::restart_traffic() start");
  switch(m_pause) {
  case true :
    m_pause = false ;
    m_outgoing_traffic = true ;
    m_traffic_model->init(m_call_rate, m_burst_limit,(long)0, m_call_rate_scale) ;
    m_traffic_model->start() ;
    m_stat->info_msg((char*)"Outgoing traffic (restart)");
    break ;
  default:
    break ;
  }
  GEN_DEBUG (1, "C_CallControl::restart_traffic() end");
}

void C_CallControl::force_init() {
  init_done() ;
}

unsigned long C_CallControl::get_call_rate () {
  return(0UL);
}
void C_CallControl::change_call_rate(T_GenChangeOperation P_op,
				     unsigned long P_rate) {
}
void C_CallControl::change_rate_scale(unsigned long P_scale) {
}


void C_CallControl::change_burst(unsigned long P_burst) {
  
}

unsigned long C_CallControlClient::get_call_rate () {
  return(m_traffic_model->get_desired_rate());
}

void C_CallControlClient::change_call_rate(T_GenChangeOperation P_op,
					   unsigned long P_rate) {

  switch (P_op) {
  case E_GEN_OP_SET_VALUE:
    m_call_rate = P_rate ;
    m_traffic_model->change_desired_rate(m_call_rate);
    break ;
  case E_GEN_OP_INCREASE:
    m_call_rate += m_call_rate_scale ;
    m_traffic_model->increase_desired_rate();
    break ;
  case E_GEN_OP_DECREASE:
    if (m_call_rate > m_call_rate_scale) {
      m_call_rate -= m_call_rate_scale ;
    }
    m_traffic_model->decrease_desired_rate();
    break ;
  }
}

void C_CallControlClient::change_rate_scale(unsigned long P_scale) {
  m_traffic_model->change_rate_scale(P_scale);
}


void C_CallControlClient::change_burst(unsigned long P_burst) {
  m_burst_limit = P_burst ;
  m_traffic_model->change_max_created_calls(m_burst_limit);
}

void C_CallControl::makeCallContextSuspended(T_pCallContext P_callContext) {

  int   L_id, L_event_id ;
  
  GEN_DEBUG (1, "C_CallControl::makeCallContextSuspended() start");

  L_id = P_callContext -> get_internal_id();
  m_call_ctxt_mlist->moveToList(E_CTXT_SUSPEND, L_id);

  // add elements for event list
  L_event_id = P_callContext->m_suspend_id ;
  m_call_suspended->insert(T_SuspendMap::value_type(L_event_id, P_callContext));
  
}

T_pCallContext   C_CallControl::retrieve_call_context (int P_channel_id, T_pValueData P_id) {

  T_pCallContext      L_pCallContext = NULL ;
  C_CallContext::T_CallMap::iterator L_call_it ;

  L_call_it 
    = m_call_map_table[P_channel_id]->find(C_CallContext::T_CallMap::key_type(*P_id));

  if (L_call_it != m_call_map_table[P_channel_id]->end()) {
    L_pCallContext = L_call_it -> second ;
  } else {
    L_pCallContext = NULL ;
  }
  
  return (L_pCallContext);
}


C_CallControlServer::C_CallControlServer(C_GeneratorConfig    *P_config, 
					 T_pC_ScenarioControl  P_scenControl,
					 C_ChannelControl     *P_channel_ctrl) 
  : C_CallControl(P_config, P_scenControl, P_channel_ctrl) {

  GEN_DEBUG (1, "C_CallControlServer::C_CallControlServer() start");
  GEN_DEBUG (1, "C_CallControlServer::C_CallControlServer() end");

}

C_CallControlServer::~C_CallControlServer() {
  GEN_DEBUG (1, "C_CallControlServer::~C_CallControlServer() start");
  GEN_DEBUG (1, "C_CallControlServer::~C_CallControlServer() end");
}

T_GeneratorError C_CallControlServer::InitProcedure() {

  T_GeneratorError L_ret ;
  T_pC_Scenario  L_scenario ;
  T_TrafficType  L_type ;

  L_ret = C_CallControl::InitProcedure() ;

  L_scenario = m_scenario_control->init_scenario_defined(&L_type) ;
  if (L_scenario == NULL) {
    GEN_WARNING("no init scenario defined");
  } else {
    GEN_DEBUG (1, "C_CallControl::InitProcedure() E_TRAFFIC_SERVER:");
    m_scenario_control->switch_to_init();
  }

  return (L_ret) ;
}


void C_CallControl::clean_mlist (long P_id) {
  T_pCallContext L_pCallContext ;
  int L_nbElem = m_call_ctxt_mlist -> getNbElements (P_id) ;
  while (L_nbElem) {
    L_pCallContext 
      = m_call_ctxt_table[m_call_ctxt_mlist->getFirst(P_id)];
    makeCallContextAvailable(&L_pCallContext);
    m_stat->executeStatAction(C_GeneratorStats::E_CALL_FAILED);
    L_nbElem-- ;
  }
}


void C_CallControl::clean_traffic() {

  long L_i ;

  m_accept_new_call = false ;

  if (!m_msg_rcv_ctxt_list->empty()) {
    m_msg_rcv_ctxt_list->erase(m_msg_rcv_ctxt_list->begin(),
			       m_msg_rcv_ctxt_list->end());
  }
  if (!m_event_list->empty()) {
    m_event_list->erase(m_event_list->begin(),
			m_event_list->end());
  }
  if (!m_call_suspended->empty()) {
    m_call_suspended->erase(m_call_suspended->begin(), 
			    m_call_suspended->end());
  }

  clean_mlist(E_CTXT_SEND);
  {
    int L_nbElem = m_call_ctxt_mlist -> getNbElements (E_CTXT_SUSPEND) ;
    T_pCallContext L_pCallContext ;
    while (L_nbElem) {
      L_pCallContext 
	= m_call_ctxt_table[m_call_ctxt_mlist->getFirst(E_CTXT_SUSPEND)];
      L_pCallContext->clean_suspended() ;
    }
  }
  clean_mlist(E_CTXT_SUSPEND);
  clean_mlist(E_CTXT_RECEIVE);
  for (L_i=0; L_i < (int)m_nb_wait_values; L_i++) {
    clean_mlist(E_CTXT_WAIT + L_i);
  }
  m_stat->info_msg((char*)"Traffic cleaned (channel lost)");

}

void C_CallControlClient::clean_traffic() {
  m_outgoing_traffic = false ;
  C_CallControl::clean_traffic() ;
}

void C_CallControlServer::clean_traffic() {
  C_CallControl::clean_traffic() ;
}

T_pCallContext  C_CallControl::getSessionFromDico(T_ReceiveMsgContext P_rcvCtxt,
                                                  T_pValueData        *P_value_id) {

  T_pValueData              L_value_id      = NULL ;
  T_pCallContext            L_pCallContext  = NULL ;

#ifdef INIT_CALL_FILTER
  T_ValueData               L_filtered_value ;
  bool                      L_filter = false ;
#endif //INIT_CALL_FILTER
  
  L_value_id = (P_rcvCtxt.m_msg)->get_session_id (&P_rcvCtxt) ;
  if (L_value_id == NULL) {
    GEN_DEBUG(1, "C_CallControl::messageReceivedControl() L_value_id = NULL");

#ifdef INIT_CALL_FILTER
    L_filter = false ;
#endif // INIT_CALL_FILTER    
  }
#ifdef INIT_CALL_FILTER
  else {
    if (L_value_id->m_type == E_TYPE_STRUCT) {
      L_filtered_value = *L_value_id ;
      L_filtered_value.m_value.m_val_struct.m_id_2 = 0 ;
      L_filter = true ;
    } else L_filter = false ;
  }
#endif // INIT_CALL_FILTER    
  
  if (L_value_id != NULL) {
    L_pCallContext = retrieve_call_context (P_rcvCtxt.m_channel, L_value_id);
    
#ifdef INIT_CALL_FILTER
    if ((L_pCallContext == NULL) && (L_filter == true)) {
      L_pCallContext = retrieve_call_context (P_rcvCtxt.m_channel, &L_filtered_value);
    }
#endif // INIT_CALL_FILTER
    
  } else {
    L_pCallContext = NULL ;
  }
  
  *P_value_id = L_value_id ;
  return (L_pCallContext) ;
}

T_pCallContext  C_CallControl::getSessionFromScen(T_ReceiveMsgContext P_rcvCtxt,
                                                T_pValueData        *P_value_id) {

  T_pRetrieveIdsDef   L_retrieveIds   = NULL ;
  int                 L_i                    ;
  T_pValueData        L_value_id      = NULL ;
  T_pCallContext      L_pCallContext  = NULL ;


#ifdef INIT_CALL_FILTER
  T_ValueData               L_filtered_value ;
  bool                      L_filter = false ;
#endif //INIT_CALL_FILTER

  L_retrieveIds = m_scenario_control->get_id_table_by_channel(P_rcvCtxt.m_channel);
  if (L_retrieveIds != NULL) {
    for (L_i = 0 ; L_i < L_retrieveIds->m_nb_ids ; L_i++) {
      L_value_id = (P_rcvCtxt.m_msg)->get_field_value(L_retrieveIds->m_id_table[L_i],
                                                      -1,
                                                      -1) ;
      if (L_value_id == NULL) {
        GEN_DEBUG(1, "C_CallControl::messageReceivedControl() L_value_id = NULL");
        
#ifdef INIT_CALL_FILTER
        L_filter = false ;
#endif // INIT_CALL_FILTER    
      }
#ifdef INIT_CALL_FILTER
      else {
        if (L_value_id->m_type == E_TYPE_STRUCT) {
          L_filtered_value = *L_value_id ;
          L_filtered_value.m_value.m_val_struct.m_id_2 = 0 ;
          L_filter = true ;
        } else L_filter = false ;
      }
#endif // INIT_CALL_FILTER 
      if (L_value_id != NULL) {
        L_pCallContext = retrieve_call_context (P_rcvCtxt.m_channel, L_value_id);
#ifdef INIT_CALL_FILTER
        if ((L_pCallContext == NULL) && (L_filter == true)) {
          L_pCallContext = retrieve_call_context (P_rcvCtxt.m_channel, &L_filtered_value);
        }
#endif // INIT_CALL_FILTER
      }
      
      if (L_pCallContext != NULL) {
        break;
      }
    } // for (L_i = 0...)
    if (L_pCallContext == NULL ){
      GEN_LOG_EVENT (LOG_LEVEL_TRAFFIC_ERR, 
                     "Value of session-id defiened in scenario is incorrect or not found for channel["
                     << P_rcvCtxt.m_channel
                     << "]");
    }
  } // if (L_retrieveIds != ...)
  
  *P_value_id = L_value_id ;
  
  return (L_pCallContext) ;
}

