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

#include "C_ReadControl.hpp"
#include "Utils.hpp"

#include "GeneratorTrace.hpp"
#include "BufferUtils.hpp"

#include <cerrno>  // for errno definition
#include <cstring> // for strerror definition

#include "dlfcn_t.hpp"

#define MAX_CTXT_TRANS 1

C_ReadControl::C_ReadControl(C_ChannelControl   *P_channel_ctrl,
			     C_TransportControl *P_transport_ctrl) : C_TaskControl() {


  GEN_DEBUG(1, "C_ReadControl::C_ReadControl() start");

  m_scen_controller = NULL ;

  m_stat = C_GeneratorStats::instance () ;

  m_max_event_nb = 0 ;
  m_events = NULL ;

  m_call_select = &select ;

  m_last_traffic_type = E_TRAFFIC_SERVER ;

  m_channel_ctrl = P_channel_ctrl ;
  m_transport_ctrl = P_transport_ctrl ;
  m_transport_table = NULL ;
  m_transport_table_size = 0 ;

  m_nb_global_channel = 0 ;

  GEN_DEBUG(1, "C_ReadControl::C_ReadControl() end");
}

C_ReadControl::~C_ReadControl() {

  GEN_DEBUG(1, "C_ReadControl::~C_ReadControl() start");

  DELETE_VAR(m_call_controller);
  m_scen_controller = NULL ;
  m_stat = NULL ;

  DELETE_TABLE(m_events);
  m_max_event_nb = 0 ;

  m_channel_ctrl = NULL ;
  m_transport_table = NULL ;
  m_transport_table_size = 0 ;

  m_nb_global_channel = 0 ;

  GEN_DEBUG(1, "C_ReadControl::~C_ReadControl() end");
}

T_GeneratorError C_ReadControl::TaskProcedure() {

  T_GeneratorError L_error_code ;

  GEN_DEBUG(1, "C_ReadControl::TaskProcedure() start");
  L_error_code = receiveControl() ;
  if ((L_error_code == E_GEN_NO_ERROR) && (m_call_controller != NULL)) {
    L_error_code = m_call_controller -> run_task_once() ;
  }
  endTrafficControl();
  GEN_DEBUG(1, "C_ReadControl::TaskProcedure() end");
  
  return (L_error_code);

}

T_GeneratorError C_ReadControl::InitProcedure() {

  unsigned long L_config_value ;
  GEN_DEBUG(1, "C_ReadControl::InitProcedure() start");
  // init of the read controller
  if (!m_config->get_value(E_CFG_OPT_SELECT_TIMEOUT_MS,
			   &L_config_value)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Internal select timeout not specified");
  }
  m_select_timeout.tv_sec = L_config_value / 1000 ;
  m_select_timeout.tv_usec = (L_config_value % 1000) * 1000 ;

  if (!m_config->get_value(E_CFG_OPT_MAX_SIMULTANEOUS_CALLS,
			   &L_config_value)) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Internal max simultaneous call not specified");
  } 
  m_max_event_nb = 2 * L_config_value ;
  //  m_max_event_nb = 2048 ;
  GEN_DEBUG(1, "C_ReadControl::InitProcedure() m_max_event_nb: " 
		  << m_max_event_nb << " sizeof(C_TransportEvent) = " 
		  <<  sizeof(C_TransportEvent));
  NEW_TABLE(m_events, C_TransportEvent, m_max_event_nb);
  GEN_DEBUG(1, "C_ReadControl::InitProcedure() m_events: " << m_events);

  m_nb_global_channel = m_channel_ctrl->nb_global_channel() ;
  transport_table () ;

  m_call_select = m_transport_ctrl -> get_call_select () ;

  GEN_DEBUG(1, "C_ReadControl::InitProcedure() end");
  return (E_GEN_NO_ERROR);

}

T_GeneratorError C_ReadControl::EndProcedure() {

  int L_i ;

  GEN_DEBUG(1, "C_ReadControl::EndProcedure() start");

  if (m_call_controller != NULL) {
    // In order to stop execution
    m_call_controller -> run_task_once() ;
    m_call_controller->close() ;
  }

  // close the instances of transport 

  for(L_i=0; L_i < m_transport_table_size ; L_i++) {
    m_transport_table[L_i]->close() ;
  }

  GEN_DEBUG(1, "C_ReadControl::EndProcedure() end");

  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_ReadControl::StoppingProcedure() {

  GEN_DEBUG(1, "C_ReadControl::StoppingProcedure() start");
  if (m_call_controller != NULL) m_call_controller->stop() ;
  GEN_DEBUG(1, "C_ReadControl::StoppingProcedure() end");

  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_ReadControl::ForcedStoppingProcedure() {
  if (m_call_controller != NULL) m_call_controller->stop ();
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_ReadControl::receiveControl () {

  fd_set                    L_ReadMask             ;
  fd_set                    L_WriteMask            ;
  fd_set                    L_ExceptionMask        ;
  int                       L_n = 0                ;

  /* Number of Fd popped in the select */
  int                       L_MaxFd = 0            ; 
  int                       L_error = 0            ;

  struct timeval            L_TimeOut              ;
  struct timeval           *L_pTimeOut=&L_TimeOut  ;

  T_ReceiveMsgContext       L_currentRcvCtxt       ;

  size_t                    L_nb_event             ;
  size_t                    L_i                    ;

  int                       L_j                    ;

  T_pC_TransportEvent       L_event_occured        ;
  int                       L_event_id             ;
  int                       L_channel_id           ;

  GEN_DEBUG(1, "C_ReadControl::receiveControl() start");

  L_TimeOut = m_select_timeout ;

  /* Init masks for the select */
  FD_ZERO(&L_ReadMask);
  FD_ZERO(&L_WriteMask);
  FD_ZERO(&L_ExceptionMask);

  L_MaxFd = 0 ;

  for (L_j = 0 ; L_j < m_transport_table_size; L_j ++) {

    L_MaxFd = m_transport_table[L_j]->pre_select 
      ( L_MaxFd, 
       &L_ReadMask, 
       &L_WriteMask, 
       &L_ExceptionMask, 
        L_pTimeOut,
	NULL,
	0) ;
  } // for L_j
       
  if (L_MaxFd >= 1) {

    // select system call
    L_n = (*m_call_select) (L_MaxFd+1,
			    &L_ReadMask,
			    &L_WriteMask,
			    &L_ExceptionMask,
			    L_pTimeOut);

  }
    
    /* select failed ? */

  // select error cases
  if (L_n < 0) {
    switch (errno) {
       case EINTR :
         /*
          * NOTHING TO DO, AND MUST DO
          * SELECT HAS BEEN INTERRUPTED (EINTR) BY A SIGNAL
          */
         break;
       default :

	 GEN_ERROR(E_GEN_FATAL_ERROR, 
		   "select failed " << strerror(errno));
         break;
    }
  }

  // if L_n == 0 => nothing to do => no event => just timeout 
  GEN_DEBUG(1, "select return = " << L_n);

  // select popped with events
  if (L_n > 0) {

    
    for (L_j = 0 ; L_j < m_transport_table_size; L_j ++) {

      L_nb_event = m_max_event_nb ;
      L_error = m_transport_table[L_j]->post_select
	(L_n, &L_ReadMask,&L_WriteMask, &L_ExceptionMask,
	 m_events,
	 (size_t*)&L_nb_event);

      if (L_error <0) {

	GEN_ERROR(E_GEN_FATAL_ERROR, "Post select (" << L_error << ")");

      } else { // L_error >= 0

	GEN_DEBUG(1, "C_ReadControl::receiveControl() nb events = " << L_nb_event);
	
	for (L_i = 0 ; L_i < L_nb_event ; L_i++) {

	  L_event_occured = &m_events [L_i];
	  L_event_id      = L_event_occured->m_id ;
	  L_channel_id    = L_event_occured->m_channel_id ;

	  switch (L_event_occured->m_type) {

	  case C_TransportEvent::E_TRANS_RECEIVED: {

	    GEN_DEBUG(1, 
		      "C_ReadControl::receiveControl() E_TRANS_RECEIVED id ["
		      << L_event_id << "]");
	    
	    while ((m_transport_table[L_j]
		    ->get_message(L_event_occured->m_id, &L_currentRcvCtxt)) == true) {
	      m_call_controller
		->messageReceived (&L_currentRcvCtxt) ;
	    }
	    break ;
	  }

	  case C_TransportEvent::E_TRANS_CLOSED: {
	    GEN_DEBUG(1, "C_ReadControl::receiveControl() E_TRANS_CLOSED id["
		      << L_event_id << "]");
	    
	    m_channel_ctrl->closed(L_channel_id, L_event_id);
	    if (m_call_controller != NULL) {
	      if (m_nb_global_channel != m_channel_ctrl->opened()) {
		m_call_controller->stop() ;
	      }
	    } else {
	      if (m_nb_global_channel != m_channel_ctrl->opened()) {
		stop() ;
	      }
	    }
	    
	    if (m_last_traffic_type == E_TRAFFIC_CLIENT) {
	      m_call_controller->stop();
	      GEN_FATAL(E_GEN_FATAL_ERROR, "Connection closed (Client traffic stopped)");
	    }
	    break ;
	  }

	  case C_TransportEvent::E_TRANS_CONNECTION: {
	    GEN_DEBUG(1, 
		      "C_ReadControl::receiveControl() E_TRANS_CONNECTION id["
		      << L_event_id << "]");
	    break ;
	  }

	  case C_TransportEvent::E_TRANS_OPEN: {
	    T_EventRecv L_event_recv ;

	    GEN_DEBUG(1, 
		      "C_ReadControl::receiveControl() E_TRANS_OPEN id ["
		      << L_event_id << "]");

	    m_channel_ctrl->opened(L_channel_id, L_event_id);

	    if (m_call_controller != NULL) {
	      L_event_recv.m_id = L_event_id ;
	      L_event_recv.m_type = L_event_occured->m_type ;
	      m_call_controller->eventReceived (&L_event_recv);
	    } else {
	      if (m_nb_global_channel == m_channel_ctrl->opened()) {
		create_call_controller() ;
	      }
	    }

	    break ;
	  }


	  case C_TransportEvent::E_TRANS_OPEN_FAILED: {

	    
	    GEN_DEBUG(1, 
		      "C_ReadControl::receiveControl() E_TRANS_OPEN_FAILED id ["
		      << L_event_id << "]");
	    
	    m_channel_ctrl->open_failed(L_channel_id, L_event_id);
	    if (m_call_controller != NULL) {

	      T_EventRecv L_event_recv ;
	      L_event_recv.m_id = L_event_id ;
	      L_event_recv.m_type = L_event_occured->m_type ;
	      m_call_controller->eventReceived (&L_event_recv);

	    } else {
	      if (m_nb_global_channel != m_channel_ctrl->opened()) {
		stop() ;
	      }
	    }
	    break ;
	  }

	  case C_TransportEvent::E_TRANS_NO_EVENT: {
	    GEN_DEBUG(1, 
		      "C_ReadControl::receiveControl() E_TRANS_NO_EVENT id ["
		      << L_event_id << "]");
	    break ;
	  }
	    
	  } // switch L_event_occured

	} // for L_i
	
      } // if L_error
    } // for L_j
    
  } // if L_n > 0

  GEN_DEBUG(1, "C_ReadControl::receiveControl() end");
  return (E_GEN_NO_ERROR);
  
}


void C_ReadControl::set_scenario_control 
(T_pC_ScenarioControl P_scenControl,
 T_TrafficType        P_trafficType) {
  
  GEN_DEBUG(1, "C_ReadControl::set_scenario_control() start");
  m_scen_controller = P_scenControl ;
  m_traffic_type = P_trafficType ;
  GEN_DEBUG(1, "C_ReadControl::set_scenario_control() end");

}

void C_ReadControl::set_config(C_GeneratorConfig *P_config) {
  GEN_DEBUG(1, "C_ReadControl::set_config() start");
  m_config = P_config ;
  GEN_DEBUG(1, "C_ReadControl::set_config() end");
}  



void C_ReadControl::endTrafficControl() {

  GEN_DEBUG(1, "C_ReadControl::endTrafficControl() start");
  GEN_DEBUG(1, "C_ReadControl::endTrafficControl() state [" << M_state << "]");
  
  
  switch(M_state) {
    
  case C_CallControl::E_STATE_RUNNING:
    if (m_call_controller != NULL) {
      if (   m_call_controller->get_state() 
	     == C_TaskControl::E_STATE_STOPPED) {
	M_state = C_TaskControl::E_STATE_STOPPING ;
      }
    } // else {
      //      M_state = C_TaskControl::E_STATE_STOPPED ;
      // }
    break ;
    
  case C_CallControl::E_STATE_STOPPING:
    if (m_call_controller != NULL) {
      if (   m_call_controller->get_state() 
	     == C_TaskControl::E_STATE_STOPPED) {
	M_state = C_TaskControl::E_STATE_STOPPED ;
	}
    } else {
      M_state = C_TaskControl::E_STATE_STOPPED ;
    }
    break ;
    
  default:
    break ;
  }

  GEN_DEBUG(1, "C_ReadControl::endTrafficControl() end");
}

void C_ReadControl::pause_traffic() {
  GEN_DEBUG(1, "C_ReadControl::pause_traffic() start");
  if (m_call_controller != NULL) {
    m_call_controller -> pause_traffic() ;
  }
  GEN_DEBUG(1, "C_ReadControl::pause_traffic() end");
}

void C_ReadControl::restart_traffic() {
  GEN_DEBUG(1, "C_ReadControl::restart_traffic() start");
  if (m_call_controller != NULL) {
    m_call_controller -> restart_traffic() ;
  }
  GEN_DEBUG(1, "C_ReadControl::restart_traffic() end");
}

void C_ReadControl::force_init() {
  if (m_call_controller != NULL) {
    m_call_controller->force_init();
  }
}

unsigned long C_ReadControl::get_call_rate() {
  unsigned long L_ret = 0 ;
  if (m_call_controller != NULL) {
    L_ret = m_call_controller->get_call_rate();
  }  
  return (L_ret);
}

void C_ReadControl::change_call_rate(T_GenChangeOperation P_op, 
				     unsigned long        P_rate) {
  if (m_call_controller != NULL) {
    m_call_controller->change_call_rate(P_op, P_rate);
  }  
}

void C_ReadControl::change_rate_scale(unsigned long P_scale) {
  if (m_call_controller != NULL) {
    m_call_controller->change_rate_scale(P_scale);
  }  
}

void C_ReadControl::change_burst (unsigned long P_burst) {
  if (m_call_controller != NULL) {
    m_call_controller->change_burst(P_burst);
  }  
}


void C_ReadControl::transport_table () {
  GEN_DEBUG(1, "C_ReadControl::transport_table() start");
  m_transport_table = 
    m_channel_ctrl->get_transport_table(&m_transport_table_size);
  if (m_channel_ctrl->open_global_channel()
      == m_channel_ctrl->nb_global_channel()) {
    create_call_controller () ;
  } 
  // TO DO by protocol
  //  else {
  //  GEN_FATAL(E_GEN_FATAL_ERROR, "Global channel not opened");
  // }
  GEN_DEBUG(1, "C_ReadControl::transport_table() end");
}

void C_ReadControl::create_call_controller () {
  // initialization of the call controller

  GEN_DEBUG(1, "C_ReadControl::create_call_controller() start");
  if (m_traffic_type == E_TRAFFIC_CLIENT) {
    NEW_VAR(m_call_controller, C_CallControlClient(m_config, 
						   m_scen_controller,
						   m_channel_ctrl)) ;
  } else {
    NEW_VAR(m_call_controller, C_CallControl(m_config, 
					     m_scen_controller,
					     m_channel_ctrl)) ;
  }
  m_call_controller -> init () ;

  GEN_DEBUG(1, "C_ReadControl::create_call_controller() end");
}

