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

#include "C_CallContext.hpp"
#include "C_Scenario.hpp"
#include "GeneratorTrace.hpp"

#include "Utils.hpp"

int C_CallContext::m_nb_channel = 0 ;

T_pValueData 
C_CallContext::set_id(int P_channel_id, T_pValueData P_val) {
  m_id_table[P_channel_id] = *P_val ;
  if (P_val->m_type == E_TYPE_STRING) {
    ALLOC_TABLE(m_id_table[P_channel_id].m_value.m_val_binary.m_value,
		unsigned char*,
		sizeof(unsigned char),
		P_val->m_value.m_val_binary.m_size);
    memcpy (m_id_table[P_channel_id].m_value.m_val_binary.m_value,
	    P_val->m_value.m_val_binary.m_value,
	    P_val->m_value.m_val_binary.m_size);
    m_id_table[P_channel_id].m_value.m_val_binary.m_size 
      = P_val->m_value.m_val_binary.m_size ;
  }
  return (&m_id_table[P_channel_id]);
}
   

void C_CallContext::next_cmd() {
  m_current_cmd_idx++;
}

void C_CallContext::reset_id () {
  int L_i ;
  for (L_i = 0 ; L_i < m_nb_channel; L_i++) {
    if (m_id_table[L_i].m_type == E_TYPE_STRING) {
      if (m_id_table[L_i].m_value.m_val_binary.m_size) {
	FREE_TABLE(m_id_table[L_i].m_value.m_val_binary.m_value);
      }
    }
    m_id_table[L_i].m_type = E_TYPE_NUMBER ;
    m_id_table[L_i].m_value.m_val_number = 0 ;
  }
}


void C_CallContext::reset_memory(int P_id) {
  GEN_DEBUG(2, "C_CallContext::reset_memory start P_id: " << P_id
		  << " type: " << m_memory_table[P_id].m_type);

  if (m_memory_table[P_id].m_type == E_TYPE_STRING) {
    if (m_memory_table[P_id].m_value.m_val_binary.m_size) {
      FREE_TABLE(m_memory_table[P_id].m_value.m_val_binary.m_value);
    }
  }
  m_memory_table[P_id].m_type = E_TYPE_NUMBER ;
  m_memory_table[P_id].m_value.m_val_number = 0 ;

  GEN_DEBUG(2, "C_CallContext::reset_memory end");
}
  

C_CallContext::C_CallContext(int P_id, int P_nbChannel, int P_mem, int P_nbRetrans) : C_ContextFrame() {
  int L_i ;
  m_internal_call_id = P_id ;
  m_state = E_CTXT_ERROR ;
  m_scenario = NULL ;
  m_msg_received = NULL ;
  m_current_cmd_idx = -1 ;
  m_selected_line = -1 ;


  if (P_mem) {
    ALLOC_TABLE(m_memory_table,
		T_pValueData,
		sizeof(T_ValueData),
		P_mem);
    for (L_i = 0; L_i < P_mem; L_i++) {
      m_memory_table[L_i].m_type = E_TYPE_NUMBER ;
      m_memory_table[L_i].m_value.m_val_number = 0 ;
    }
    m_nb_mem = P_mem ;
  } else {
    m_memory_table = NULL ;
    m_nb_mem = 0 ;
  }
  m_created_call = false ;

  if (P_nbChannel) {
    m_nb_channel = P_nbChannel ;
    ALLOC_TABLE(m_channel_table,
		int *,
		sizeof(int),
		P_nbChannel);
    ALLOC_TABLE(m_id_table,
		T_pValueData,
		sizeof(T_ValueData),
		P_nbChannel);
    for(L_i = 0 ; L_i < P_nbChannel; L_i++) {
      m_channel_table[L_i] = -1 ;
      m_id_table[L_i].m_type = E_TYPE_NUMBER ;
      m_id_table[L_i].m_value.m_val_number = 0 ;
    }
  }

  m_nb_retrans = P_nbRetrans ;
  if (P_nbRetrans) {
    ALLOC_TABLE(m_retrans_msg, C_MessageFrame**, sizeof(C_MessageFrame*), P_nbRetrans);
    ALLOC_TABLE(m_retrans_time, struct timeval*, sizeof(struct timeval), P_nbRetrans);
    ALLOC_TABLE(m_nb_retrans_done, int*, sizeof(int), P_nbRetrans);
    ALLOC_TABLE(m_retrans_cmd_idx, int*, sizeof(int), P_nbRetrans);

    ALLOC_TABLE(m_retrans_it, T_retransContextList::iterator*, sizeof(T_retransContextList::iterator), P_nbRetrans);
    ALLOC_TABLE(m_retrans_it_available, bool*, sizeof(bool), P_nbRetrans);


    for(L_i = 0 ; L_i < P_nbRetrans; L_i++) {
      m_nb_retrans_done[L_i] = 0 ;
      m_retrans_cmd_idx[L_i] = 0 ;
      m_retrans_msg[L_i] = NULL ;
      m_retrans_it_available[L_i] = false ;
    }
  } else {
    m_retrans_msg = NULL ;
    m_retrans_time = NULL ;
    m_nb_retrans_done = NULL ;
    m_retrans_cmd_idx = NULL ;
    m_retrans_it = NULL ;
    m_retrans_it_available = NULL ; 
  }

  m_retrans_to_do = false ;

}

C_CallContext::~C_CallContext() {
  int L_i ;

  m_internal_call_id = -1 ;
  m_state = E_CTXT_ERROR ;
  m_scenario = NULL ;
  m_msg_received = NULL ;
  m_current_cmd_idx = -1 ;
  m_selected_line = -1 ;
  reset_id() ;
  m_created_call = false ;


  for (L_i = 0 ; L_i < m_nb_mem; L_i++) {
    reset_memory(L_i) ;
  }
  FREE_TABLE(m_memory_table);
  m_nb_mem = 0 ;

  FREE_TABLE(m_channel_table);

  FREE_TABLE(m_id_table);

  clean_retrans();
  
  FREE_TABLE(m_retrans_time);
  FREE_TABLE(m_nb_retrans_done);
  FREE_TABLE(m_retrans_cmd_idx);

  FREE_TABLE(m_retrans_it);
  FREE_TABLE(m_retrans_it_available);

}

void C_CallContext::init() {
  int L_i ;
  m_state = E_CTXT_AVAILABLE ;
  m_current_cmd_idx = 0 ;
  m_created_call = false ;
  reset_id();
  for (L_i = 0 ; L_i < m_nb_mem; L_i++) {
    reset_memory(L_i) ;
  }
  
  clean_retrans () ;
}

void C_CallContext::reset() {
  m_state = E_CTXT_AVAILABLE ;
  m_current_cmd_idx = 0 ;
  m_created_call = false ;
  clean_retrans () ;
}

T_CallContextState C_CallContext::get_state() {
  return (m_state);
}

void  C_CallContext::set_state(T_CallContextState P_state) {
  m_state = P_state ;
}

T_pC_Scenario C_CallContext::get_scenario() {
  return (m_scenario);
}

void  C_CallContext::set_scenario(T_pC_Scenario P_scen) {
  m_scenario = P_scen ;
}

C_MessageFrame* C_CallContext::get_msg_received() {
  return (m_msg_received);
}

void C_CallContext::set_msg_received(C_MessageFrame *P_msg) {
  m_msg_received = P_msg ;
}

int C_CallContext::get_current_cmd_idx() {
  return (m_current_cmd_idx);
}

T_CallContextState C_CallContext::init_state (T_pC_Scenario P_scen,
					      struct timeval *P_time) {

  m_scenario = P_scen ;
  m_state = m_scenario->first_state(); 
  m_msg_received = NULL ;
  m_current_cmd_idx = 0 ;
  reset_id();
  m_created_call = false ;
  m_current_time = *P_time ;
  clean_retrans () ;
  return (m_state);
}

T_CallContextState C_CallContext::init_state (T_pC_Scenario P_scen) {

  m_scenario = P_scen ;
  m_state = m_scenario->first_state(); 
  m_msg_received = NULL ;
  m_current_cmd_idx = 0 ;
  reset_id();
  m_created_call = false ;
  clean_retrans () ;
  return (m_state);
}

void C_CallContext::init_state(C_Scenario *P_scen, T_pReceiveMsgContext P_rcvCtxt) {
  m_state = E_CTXT_RECEIVE ;
  m_scenario = P_scen ;
  m_msg_received = P_rcvCtxt->m_msg ;
  m_channel_received = P_rcvCtxt->m_channel ;
  m_current_cmd_idx = 0 ;
  // m_id to be tested
  clean_retrans () ;
  m_created_call = false ;
  m_current_time = P_rcvCtxt->m_time ;

  m_channel_table[P_rcvCtxt->m_channel] = P_rcvCtxt->m_response ;
  
}

bool C_CallContext::msg_received(T_pReceiveMsgContext P_rcvCtxt) {

  bool L_ret ;

  if (m_state == E_CTXT_RECEIVE) {
    m_msg_received = P_rcvCtxt -> m_msg  ;
    m_channel_received = P_rcvCtxt->m_channel ;
    m_current_time = P_rcvCtxt -> m_time ;
    m_channel_table[P_rcvCtxt->m_channel] = P_rcvCtxt->m_response ;
    L_ret = true ;
  } else {
    L_ret = false ;
  }
  return (L_ret);
    
}


bool C_CallContext::state_receive() {
  return (m_state == E_CTXT_RECEIVE);
}

int C_CallContext::get_internal_id() {
  return (m_internal_call_id);
}

T_pValueData C_CallContext::get_memory (int P_id) {
  return (&(m_memory_table[P_id])) ;
}

void C_CallContext::switch_to_scenario (C_Scenario *P_scen) {
  m_scenario = P_scen ;
  m_state = P_scen->first_state();
  m_current_cmd_idx = 0 ;
  m_retrans_to_do = false ;
}

void C_CallContext::clean_suspended () {
  DELETE_VAR(m_suspend_msg)   ;
}

void C_CallContext::clean_retrans () {
  int L_i = 0 ;
  for (L_i = 0 ; L_i < m_nb_retrans; L_i++) {
    m_nb_retrans_done[L_i] = 0 ;
    m_retrans_cmd_idx[L_i] = 0 ;
    DELETE_VAR(m_retrans_msg[L_i]);
    m_retrans_it_available[L_i] = false ;
  }
  m_retrans_to_do = false ;
}





