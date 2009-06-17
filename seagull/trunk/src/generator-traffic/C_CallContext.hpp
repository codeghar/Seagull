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
#include "exe_code_t.h"


class C_Scenario ; // prevent from circular include 
                   // => C_Scenario.hpp only in .cpp

class C_CallControl ; 


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

  typedef map_t<T_ValueData, C_CallContext*> T_CallMap, *T_pCallMap ;

  typedef struct _retrans_context {
    C_CallContext * m_context             ;
    int             m_retrans_index       ; // id table internal 
    int             m_retrans_delay_index ; // delay value index
  } T_retransContext, *T_pRetransContext ;

  typedef list_t<T_retransContext> 
  T_retransContextList, *T_pRetransContextList;


  typedef struct _context_map_data {
    T_CallMap::iterator                m_iterator   ;
    int                                m_channel    ;
  } T_contextMapData, *T_pContextMapData ;

  typedef list_t <T_contextMapData> T_contextMapDataList, *T_pContextMapDataList ;
  

  // TEMPORARY
  bool                      m_created_call  ;
  // TEMPORARY
  struct timeval            m_current_time  ;
  struct timeval            m_start_time    ;

  int                       m_suspend_id    ;
  C_MessageFrame           *m_suspend_msg   ;
  int                       m_channel_id    ;
  
  C_MessageFrame           **m_retrans_msg     ;
  struct timeval            *m_retrans_time    ;
  int                       *m_nb_retrans_done ;
  int                       *m_retrans_cmd_idx ;
  int                        m_nb_retrans      ;

  T_retransContextList::iterator *m_retrans_it;
  bool                           *m_retrans_it_available ;

  T_retransContext          m_retrans_context ;
  bool                      m_retrans_to_do ;

  T_pValueData              m_id_table ;

  int                      *m_channel_table ; // response channel table
  int                       m_selected_line;

  int                       m_channel_received ;
  
  int                       m_channel_id_verdict ;
  bool                      m_channel_id_verdict_to_do ;

  T_exeCode                 m_exec_result   ;

  C_CallControl*            m_call_control  ;

  T_pContextMapDataList     m_map_data_list ;


  C_CallContext(C_CallControl *P_call_control, 
                int P_id, int P_nbChannel, int P_mem=0, int P_nbRetrans=0);
  ~C_CallContext() ;

  T_pValueData set_id(int P_channel_id, T_pValueData P_val) ;
  T_pValueData get_id(int P_channel_id);


  static int m_nb_channel ;

  void init()  ;
  void reset() ;
  void reset_id(int P_channel_id) ;


  T_CallContextState init_state (C_Scenario *P_scen);
  T_CallContextState init_state (C_Scenario *P_scen, struct timeval *P_time);
  void               init_state (C_Scenario *P_scen, T_pReceiveMsgContext P_rcvCtxt);
  bool               msg_received (T_pReceiveMsgContext P_rcvCtxt);

  bool state_receive() ;
  void next_cmd () ;
  void set_cmd(int p_iIndx);
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
  void               clean_retrans () ;

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
