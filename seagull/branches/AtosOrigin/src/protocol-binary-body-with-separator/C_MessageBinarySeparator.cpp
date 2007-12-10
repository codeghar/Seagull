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

#include "C_MessageBinarySeparator.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "ProtocolData.hpp"

C_MessageBinarySeparator::C_MessageBinarySeparator(C_ProtocolBinarySeparator *P_protocol)
  : C_MessageBinary(P_protocol) {

  GEN_DEBUG(1, "C_MessageBinarySeparator::C_MessageBinarySeparator() start");
  GEN_DEBUG(1, "C_MessageBinarySeparator::C_MessageBinarySeparator() end");
}

bool C_MessageBinarySeparator::update_fields (C_MessageFrame* P_msg) {

  bool                      L_result ;
  C_MessageBinarySeparator *L_msg    ;
  int                       L_i      ;

  L_msg = dynamic_cast<C_MessageBinarySeparator*>(P_msg) ;
  
  GEN_DEBUG(1, "C_MessageBinarySeparator::update_fields() start");

  if (L_msg == NULL) {
    L_result = false ;
  } else {
    L_result = (L_msg ->m_nb_body_values  == m_nb_body_values) ? true : false ;
    if (L_result) {
      m_header_id = L_msg->get_id_message();
      for (L_i =0 ; L_i < m_nb_body_values ; L_i++) {
        m_body_val[L_i].m_id = L_msg->m_body_val[L_i].m_id ;
      }
    }
  }

  GEN_DEBUG(1, "C_MessageBinarySeparator::update_fields() end");

  return (L_result);
}


C_MessageBinarySeparator::~C_MessageBinarySeparator() {

  GEN_DEBUG(1, 
	    "C_MessageBinarySeparator::~C_MessageBinarySeparator() start: ");
  GEN_DEBUG(1, "C_MessageBinarySeparator::~C_MessageBinarySeparator() end");
}

bool C_MessageBinarySeparator::compare_types (C_MessageFrame* P_msg) {

  bool                      L_result ;
  C_MessageBinarySeparator *L_msg    ;

  L_msg = dynamic_cast<C_MessageBinarySeparator*>(P_msg) ;

  GEN_DEBUG(1, "C_MessageBinarySeparator::compare_types() start");
  if (L_msg == NULL) {
    L_result = false ;
  } else {
    T_Value  L_orig, L_dest ;
    L_orig = L_msg->get_type() ;
    L_dest = get_type() ;
    L_result = compare_value(L_orig, L_dest) ? true : false ;
    FREE_TABLE(L_orig.m_val_binary.m_value);
    FREE_TABLE(L_dest.m_val_binary.m_value);
    GEN_DEBUG(1, "C_MessageBinarySeparator::compare_types() end");
  }
  GEN_DEBUG(1, "C_MessageBinarySeparator::compare_types() end");

  return (L_result) ;
}

T_Value  C_MessageBinarySeparator::get_type() {


  int            L_idx              ;
  
  T_Value        L_value            ;
  bool           L_found = false    ;
  int            L_i                ;

  GEN_DEBUG(1, "C_MessageBinarySeparator::get_type() start");
  
  L_idx = m_protocol -> get_header_type_id_body() ;
  
  for (L_i=0; L_i < m_nb_body_values ; L_i++) { // remenber: to be optimized
    if (m_body_val[L_i].m_id == L_idx) { 
      L_found=true; 
      break; 
    }
  }
  
  if (L_found == true) {
    L_value.m_val_binary.m_size = m_body_val[L_i].m_value.m_val_binary.m_size;
    
    if (L_value.m_val_binary.m_size != 0) {
      ALLOC_TABLE(L_value.m_val_binary.m_value,
                  unsigned char*,
                  sizeof(unsigned char),
                  L_value.m_val_binary.m_size);
      
      memcpy(L_value.m_val_binary.m_value,
             m_body_val[L_i].m_value.m_val_binary.m_value,
             L_value.m_val_binary.m_size);
    }
  }
  GEN_DEBUG(1, "C_MessageBinarySeparator::get_type() end");
  return (L_value) ;
}


void   C_MessageBinarySeparator::update_message_stats  () {
  int                       L_i      ;

  GEN_DEBUG(1, "C_MessageBinarySeparator::update_message_stats() start");

  if (m_protocol->get_stats()) {
    (m_protocol->get_stats())->updateStats (E_MESSAGE,
                                            E_RECEIVE,
                                            m_header_id);

    for (L_i =0 ; L_i < m_nb_body_values ; L_i++) {
      if (m_body_val[L_i].m_value.m_val_binary.m_size != 0 ) {
        (m_protocol->get_stats())->updateStats (E_MESSAGE_COMPONENT,
                                                E_RECEIVE,
                                                m_body_val[L_i].m_id);
      }
    }
  }
  GEN_DEBUG(1, "C_MessageBinarySeparator::update_message_stats() end");
}
