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

#ifndef _C_CALL_CONTEXT
#define _C_CALL_CONTEXT

#include "map_t.hpp"

#include "ProtocolData.hpp"
#include "C_MessageFrame.hpp"
#include "ReceiveMsgContext.h"
#include "C_ContextFrame.hpp"

class C_Scenario ; // prevent from circular include 
                   // => C_Scenario.hpp only in .cpp

typedef long T_CallContextState, *T_pCallContextState ;
#define E_CTXT_AVAILABLE    0L
#define E_CTXT_SUSPEND      1L
#define E_CTXT_RECEIVE      2L
#define E_CTXT_SEND         3L
#define E_CTXT_WAIT         4L
#define E_CTXT_NBSTATE      5L
#define E_CTXT_ERROR       -1L

class C_CallContext : public C_ContextFrame {
public:

  // TEMPORARY
  bool                      m_created_call  ;
  // TEMPORARY
  struct timeval            m_current_time  ;
  struct timeval            m_start_time    ;

  int                       m_suspend_id    ;
  C_MessageFrame           *m_suspend_msg   ;
  int                       m_channel_id    ;

  T_pValueData              m_id_table ;

  int                      *m_channel_table ; // response channel table
  int                       m_selected_line;

  int                       m_channel_received ;


  C_CallContext(int P_id, int P_nbChannel, int P_mem=0);
  ~C_CallContext() ;

  T_pValueData 
  set_id(int P_channel_id, T_pValueData P_val) ;

  static int m_nb_channel ;

  void init()  ;
  void reset() ;

  T_CallContextState init_state (C_Scenario *P_scen);
  T_CallContextState init_state (C_Scenario *P_scen, struct timeval *P_time);
  void               init_state (C_Scenario *P_scen, T_pReceiveMsgContext P_rcvCtxt);
  bool               msg_received (T_pReceiveMsgContext P_rcvCtxt);

  bool state_receive() ;
  void next_cmd () ;
  void switch_to_scenario (C_Scenario *P_scen);


  T_pValueData get_memory(int P_id) ;
  void                     reset_memory(int P_id);

  T_CallContextState get_state() ;
  void               set_state(T_CallContextState P_state);

  C_Scenario*        get_scenario();
  void               set_scenario(C_Scenario *P_scen);

  C_MessageFrame*    get_msg_received();
  void               set_msg_received(C_MessageFrame *P_msg);

  int                get_current_cmd_idx();

  int                get_internal_id ();

  void               clean_suspended () ;

private:

  int                       m_internal_call_id ;
  T_CallContextState        m_state ;
  C_Scenario               *m_scenario ;
  C_MessageFrame           *m_msg_received ;
  int                       m_current_cmd_idx ;
  T_pValueData              m_memory_table ;
  int                       m_nb_mem ;

  void reset_id() ;

} ;

typedef C_CallContext* T_pCallContext ;

typedef map_t<T_ValueData,
	      T_pCallContext> T_CallMap, *T_pCallMap ;

#endif // _C_CALL_CONTEXT
