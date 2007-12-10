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

#ifndef _C_GENERATOR_H
#define _C_GENERATOR_H

#include "C_TaskControl.hpp"

#include "cmd_line_t.hpp"

#include "C_GeneratorConfig.hpp"
#include "GeneratorXmlData.hpp"
#include "C_ReadControl.hpp"
#include "C_XmlData.hpp"
#include "C_ScenarioControl.hpp"
#include "C_RemoteControl.hpp"

#include "C_DisplayControl.hpp"
#include "C_LogStatControl.hpp"
#include "C_KeyboardControl.hpp"

#include "C_DataLogControl.hpp"
#include "C_DataLogRTDistrib.hpp"

#include "C_ProtocolControl.hpp"
#include "C_TransportControl.hpp"
#include "C_ChannelControl.hpp"

#include "C_ExternalDataControl.hpp"

#include "C_DisplayTraffic.hpp"
#include "C_DisplayRTime.hpp"
#include "C_DisplayHelp.hpp"

#include "gen_operation_t.hpp"
#include "C_LogProtocolStatControl.hpp"

#include "list_t.hpp"


class C_Generator : public C_TaskControl {

public:
   C_Generator(cmd_line_pt P_cmd_line);

  ~C_Generator();

  void set_screen (char P_key) ;

  void pause_traffic () ;
  void restart_traffic () ;
  void burst_traffic() ;

  void force_init () ;
  void pause_display () ;
  unsigned long get_call_rate() ;
  void change_call_rate(T_GenChangeOperation P_op, unsigned long P_rate);
  void change_rate_scale(unsigned long P_scale);
  void change_burst (unsigned long P_burst);

  void change_display_period (long P_period);

  C_LogProtocolStatControl* create_log_protocol_stats(C_ProtocolFrame *P_protocol,
						      char            *P_timechar,
						      string_t&        P_logProtocolStatFileName,
						      unsigned long    P_log_period_period,
                                                      bool             P_files_no_timestamp);

  void reset_cumul_counters () ;
  void activate_percent_traffic () ;

  char* get_view();
  C_Scenario * get_right_scenario(unsigned long P_value);
  
protected:

private:

  int                    m_cmd_line_size ;
  char                 **m_cmd_line_values ;
  C_GeneratorConfig     *m_config ;
  C_XmlData             *m_xml_data ;

  C_ReadControl         *m_read_control ;
  C_ScenarioControl     *m_scen_control ;

  C_DataLogControl      *m_data_log_control ;
  bool                   m_do_data_log_control ;

  C_ExternalDataControl *m_external_data_control ;


  // other controllers
  C_DisplayControl        *m_display_control ;
  bool                     m_do_display_control ;

  long                   m_display_period  ;

  // display objectss
  C_DisplayTraffic        *m_display_traffic ;
  C_DisplayRTime          *m_display_rtime   ;
  C_DisplayHelp           *m_display_help ;

  C_LogStatControl        *m_log_stat_control ;
  bool                     m_do_log_stat_control ;

  C_LogProtocolStatControl   **m_log_protocol_stat_control     ;
  int                          m_nb_log_protocol_stat_control  ;

  C_KeyboardControl       *m_keyboard_control ;
  bool                     m_do_keyboard_control ;


  C_RemoteControl        *m_remote_control ;
  bool                    m_do_remote_control ;
  

  int                      m_nb_forced ;
  pthread_t               *m_keyboard_thread ;

  // generator model
  C_ProtocolControl        *m_protocol_ctrl  ;
  C_TransportControl       *m_transport_ctrl ;
  C_ChannelControl         *m_channel_ctrl ;

  // TaskController related methods
  T_GeneratorError TaskProcedure();
  T_GeneratorError InitProcedure();
  T_GeneratorError EndProcedure();
  T_GeneratorError StoppingProcedure();
  T_GeneratorError ForcedStoppingProcedure();
  
  void             ExitProcedure () ;
  T_GeneratorError ReturnCode ()    ; 


} ;

typedef C_Generator * T_pC_Generator ;

#endif  // _C_GENERATOR_H









