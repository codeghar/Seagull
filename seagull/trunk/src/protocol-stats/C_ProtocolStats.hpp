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

#ifndef _C_PROTOCOL_STATS_H
#define _C_PROTOCOL_STATS_H

#include "iostream_t.hpp"
#include "C_ProtocolFrame.hpp"
#include "Utils.hpp"
#include "C_DisplayObject.hpp"
#include "C_ProtocolStatsFrame.hpp"

#include "C_Semaphore.hpp"
#include "fstream_t.hpp"
#include "TimeUtils.hpp"

#define TIME_LENGTH 32

class C_ProtocolStats : public C_DisplayObject, 
			public C_ProtocolStatsFrame {

public:

  C_ProtocolStats(C_ProtocolFrame *P_protocol) ;
  virtual ~C_ProtocolStats();

  void updateStats (T_ProtocolStatDataType P_type,
		    T_ProtocolStatDataAction P_action,
		    int P_id);

  iostream_output& display (iostream_output& P_stream);

  void display_stats_message ();
  void display_stats_sub_message ();

  void displayScreen() ;
  void setCurrentScreen(bool P_first) ;


  void set_file_name     (char * name)   ;
  void make_first_log    ()              ;
  void make_log          ()              ;
  int  init () ;
  void do_log       () ;

  void reset_cumul_counters () ;  


protected:
private:

  class C_CounterData {
  public:
    T_CounterType m_counter_value                ;
    T_CounterType m_counter_periodic_value       ;

    // log stat
    T_CounterType m_counter_log_value            ;
    T_CounterType m_counter_periodic_log_value   ;
    
    C_Semaphore   m_sem                          ;
    C_Semaphore   m_sem_log                      ;

  } ;

  typedef C_CounterData *C_pCounterData ;

  int              m_nb_counter_msg ;
  int              m_nb_counter_msg_comp ;

  C_pCounterData  *m_counter_msg_comp_table ;
  C_pCounterData  *m_counter_msg_table ;

  int             *m_id_msg_table ;
  int             *m_id_msg_comp_table ;
  int              m_max_id_msg ;
  int              m_max_id_msg_comp ;

  char            *m_message_name ;
  char            *m_message_component_name ;
  char           **m_message_names ;
  char           **m_message_component_names ;


  typedef void (C_ProtocolStats::* T_DisplayProtocolFct)() ;

  int                   m_current_proto_stat_id      ;
  int                   m_nb_stat_proto_id           ;
  T_DisplayProtocolFct *m_current_proto_stat_table   ;
  T_DisplayProtocolFct  m_current_proto_stat_display ;



  bool                     m_initDone ;
  struct timeval           m_startTime;
  struct timeval           m_plStartTime;

  bool                     m_headerAlreadyDisplayed;
  char                    *m_fileName;
  fstream_output          *m_outputStream;
  char* formatTime (struct timeval* P_tv);
  char* msToHHMMSS (unsigned long P_ms);

  bool                     m_do_log  ;

  int                      m_counter_message        ;
  int                      m_counter_message_comp   ;

  bool                     m_other_msg_screen       ;
  bool                     m_other_msg_screen_comp  ;


} ;

#endif // _C_PROTOCOL_STATS_H



















