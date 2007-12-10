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

#include "C_ProtocolContext.hpp"

#include "GeneratorError.h"
#include "BufferUtils.hpp"

#include <cstdlib> // for strtoul
#include "GeneratorTrace.hpp"



C_ProtocolContext::C_ProtocolContext(int P_nb_values) {

  m_nb_values = P_nb_values ;

  ALLOC_TABLE(m_ctxt_val_table, 
              T_pCtxtValue,
              sizeof(T_CtxtValue),
              m_nb_values);

  reset_ctxt_values() ;

  m_end_ctxt_counter = false;
}


void C_ProtocolContext::reset_ctxt_values(int P_index) {

  int L_i ;

  for(L_i = P_index ; L_i < m_nb_values ; L_i++) {
    m_ctxt_val_table[L_i].m_ctxt_value.m_type = E_TYPE_NUMBER ;
    m_ctxt_val_table[L_i].m_ctxt_value.m_id   = L_i ;
    m_ctxt_val_table[L_i].m_ctxt_value.m_value.m_val_number = 0 ;
    m_ctxt_val_table[L_i].m_ctxt_position.m_id_field = -1 ;
    m_ctxt_val_table[L_i].m_ctxt_position.m_position_ptr = NULL ;
  }
}

C_ProtocolContext::~C_ProtocolContext() {
  FREE_TABLE(m_ctxt_val_table);
  m_end_ctxt_counter = false;
}


void C_ProtocolContext::start_ctxt_value(int P_ctxt_id,
                                         int P_size) {
  m_ctxt_val_table[P_ctxt_id].m_ctxt_value.m_value.m_val_number = P_size ;
}


void C_ProtocolContext::update_ctxt_values(int P_nbCtx,
                                           int *P_idTable, 
                                           int P_size) {

  int L_i ;

  for (L_i = 0 ; L_i < P_nbCtx; L_i++) {
    m_ctxt_val_table[P_idTable[L_i]].m_ctxt_value.m_value.m_val_number += P_size ;
  }
}


int C_ProtocolContext::update_ctxt_values_decode(int P_nbCtx,
                                                 int *P_idTable, 
                                                 int P_size) {
  int L_ret = 0 ;
  int L_i       ;

  for (L_i = 0 ; L_i < P_nbCtx; L_i++) {
    m_ctxt_val_table[P_idTable[L_i]].m_ctxt_value.m_value.m_val_number 
      -= P_size ;
    
    if (m_ctxt_val_table[P_idTable[L_i]].m_ctxt_value.m_value.m_val_number 
	< 0) {
      L_ret = -1 ;
      break;
    }
  }

  return (L_ret);
}


void C_ProtocolContext::update_ctxt_values(int P_id,
                                           int P_size) {
  m_ctxt_val_table[P_id].m_ctxt_value.m_value.m_val_number += P_size ;
}


void C_ProtocolContext::update_ctxt_position(int P_id, T_pPositionValue P_position_value) {
  m_ctxt_val_table[P_id].m_ctxt_position.m_id_field     =  P_position_value->m_id_field;
  m_ctxt_val_table[P_id].m_ctxt_position.m_position_ptr = P_position_value->m_position_ptr;
}


unsigned long C_ProtocolContext::get_counter_ctxt(int P_ctxt_id) {
  return(m_ctxt_val_table[P_ctxt_id].m_ctxt_value.m_value.m_val_number);
}


bool C_ProtocolContext::end_counter_ctxt() {

  int L_i ;

  for (L_i=1; L_i < m_nb_values; L_i++) {
    m_end_ctxt_counter = (m_ctxt_val_table[L_i].m_ctxt_value.m_value.m_val_number
                          == 0) ;
    if (!m_end_ctxt_counter) break ;
  }
  return (m_end_ctxt_counter);
}
