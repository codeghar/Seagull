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

#include "C_MessageBinaryBodyNotInterpreted.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "ProtocolData.hpp"

C_MessageBinaryBodyNotInterpreted::C_MessageBinaryBodyNotInterpreted(C_ProtocolBinaryBodyNotInterpreted *P_protocol): C_MessageBinary(P_protocol) {

  int L_i ;
  // body non interpreted
  for(L_i = 0; L_i < MAX_BODY_VALUES; L_i++) {
    m_body_val[L_i].m_id = L_i ;
    m_body_val[L_i].m_value.m_val_binary.m_size = 0 ;
    m_body_val[L_i].m_value.m_val_binary.m_value = NULL ;
  }

  m_local_protocol = P_protocol ;

}

C_MessageBinaryBodyNotInterpreted::~C_MessageBinaryBodyNotInterpreted() {

  unsigned long L_i ;
  GEN_DEBUG(2, "m_nb_body_values = " << m_nb_body_values);
  
  for(L_i = 0; L_i < (unsigned long)m_nb_body_values; L_i++) {
    m_protocol->delete_body_value(&m_body_val[L_i]);
    GEN_DEBUG(2, "delete body value [" << L_i << "] OK");
  }
  FREE_TABLE(m_body_val);
  GEN_DEBUG(2, "FREE_TABLE(m_body_val) OK");

  m_nb_body_values = 0 ;
  
  m_protocol->reset_value_data (&m_id);
  
  m_local_protocol = NULL ;

}

T_pValueData C_MessageBinaryBodyNotInterpreted::get_session_id (C_ContextFrame *P_ctxt) {
  return (((this)->*(m_protocol->get_m_session_method()))(P_ctxt)) ;
}

bool C_MessageBinaryBodyNotInterpreted::get_body_value (T_pValueData P_res, 
                                                        int P_id, int P_occurence) {

  C_ProtocolBinaryBodyNotInterpreted::T_HeaderBodyPositionSize L_pos ;

  bool            L_found = true ;

  // ctrl a ajouter
  if (P_id != 0) {
    // find position of field
    m_local_protocol->get_field_position(&L_pos, P_id);
    
    // copy value to m_body_val[P_id]
    if (L_pos.m_size != 0) {
      // init the m_body_val[P_id] to string with size 0
      if (m_body_val[P_id].m_value.m_val_binary.m_size != L_pos.m_size) {
        FREE_TABLE(m_body_val[P_id].m_value.m_val_binary.m_value);
        ALLOC_TABLE(m_body_val[P_id].m_value.m_val_binary.m_value,
                    unsigned char*,
                    sizeof(unsigned char),
                    L_pos.m_size);
        m_body_val[P_id].m_value.m_val_binary.m_size = L_pos.m_size;
      }
      extractBinaryVal(m_body_val[P_id].m_value, 
                       L_pos.m_position, 
                       L_pos.m_size, 
                       m_body_val[0].m_value);
    }
    m_protocol->get_body_value(P_res, &m_body_val[P_id]);
    resetValue(m_body_val[P_id].m_value);
  } else {
    m_protocol->get_body_value(P_res, &m_body_val[P_id]);
  }

  return (L_found);
}


bool C_MessageBinaryBodyNotInterpreted::set_body_value (int P_id, int P_occurence, T_pValueData P_val) {
  C_ProtocolBinaryBodyNotInterpreted::T_HeaderBodyPositionSize L_pos ;
  // ctrl a ajouter
  if (P_id != 0) {
    // find position of field
    m_local_protocol->get_field_position(&L_pos, P_id);

    // copy value to m_body_val[P_id]
    if ((L_pos.m_size != 0) && (P_val->m_value.m_val_binary.m_size != 0)) {
      // init the m_body_val[P_id] to string with size 0
      copyBinaryVal(m_body_val[0].m_value, 
                    L_pos.m_position, 
                    L_pos.m_size, 
                    P_val->m_value);
    }
  } else {
    m_protocol->set_body_value(&m_body_val[0], P_val) ;
  }
  
  return (true) ;

}


T_pValueData C_MessageBinaryBodyNotInterpreted::getSessionFromBody(int P_id) {
  T_pValueData                        L_ret   = NULL  ;
  bool                                L_found = false ;
  
  if(m_protocol->check_present_session (m_header_id, P_id)) {
    m_protocol->reset_value_data(&m_id);
    L_found = get_body_value(&m_id,P_id, 1);
    if (L_found == true) {
      L_ret = &m_id ;
    }
  }
  return (L_ret);
}
