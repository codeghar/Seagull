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

#ifndef _C_PROTOCOLCONTEXT_
#define _C_PROTOCOLCONTEXT_

#include "Utils.hpp"
#include "ProtocolDataType.hpp"
#include "list_t.hpp"



class C_ProtocolContext {

public:

  typedef struct _struct_value {
    int            m_id_field     ;
    unsigned char *m_position_ptr ;
  } T_PositionValue, *T_pPositionValue ;

  typedef struct _struct_ctxt_value {
    T_ValueData       m_ctxt_value    ;
    T_PositionValue   m_ctxt_position ;
  } T_CtxtValue, *T_pCtxtValue ;


  C_ProtocolContext(int P_nb_values) ;
 
  ~C_ProtocolContext() ;

  void reset_ctxt_values(int P_index = 0) ;
  void update_ctxt_values(int P_id, int P_size) ;
  void update_ctxt_values(int P_nbCtx, int *P_idTable, int P_size) ;

  int  update_ctxt_values_decode(int P_nbCtx,
                                 int *P_idTable, 
                                 int P_size) ;

  void start_ctxt_value(int P_ctxt_id, int P_size) ;

  void update_ctxt_position(int P_id, T_pPositionValue P_position_value) ;

  unsigned long  get_counter_ctxt(int P_ctxt_id) ;
  
  bool end_counter_ctxt() ;
  

  bool          m_end_ctxt_counter ;
  
  int           m_nb_values        ;
  T_pCtxtValue  m_ctxt_val_table   ;


private:


} ; 

#endif // _C_PROTOCOLCONTEXT_









