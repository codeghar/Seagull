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

#ifndef _C_READ_CONTROL_H
#define _C_READ_CONTROL_H

#include "select_t.hpp"

#include "C_TaskControl.hpp"
#include "C_Transport.hpp"
#include "C_CallControl.hpp"
#include "GeneratorXmlData.hpp"
#include "C_ScenarioControl.hpp"
#include "TrafficType.h"
#include "C_GeneratorConfig.hpp"

#include "C_ProtocolFrame.hpp"
#include "C_GeneratorStats.hpp"

#include "C_TransportControl.hpp"
#include "C_ChannelControl.hpp"

#include "gen_operation_t.hpp"

class C_ReadControl : public C_TaskControl {

public:
  // transport context definition

   C_ReadControl(C_ChannelControl *P_channel_ctrl, C_TransportControl *P_transport_ctrl);
  ~C_ReadControl();

  void set_config (C_GeneratorConfig *P_config);

  void set_scenario_control (T_pC_ScenarioControl P_scenControl,
			     T_TrafficType        P_trafficType);

  void pause_traffic() ;
  void restart_traffic () ;
  void burst_traffic() ;

  void force_init () ;
  unsigned long get_call_rate();
  void change_call_rate(T_GenChangeOperation P_op, unsigned long P_rate);
  void change_rate_scale(unsigned long P_scale);
  void change_burst (unsigned long P_burst);

  void process_event (C_Transport *P_transport, T_pC_TransportEvent P_event);

protected:

private:

  // Specific field
  
  T_pC_ScenarioControl m_scen_controller ;
  T_pC_CallControl     m_call_controller ;
  T_TrafficType        m_traffic_type ;

  C_ChannelControl    *m_channel_ctrl ;
  C_TransportControl  *m_transport_ctrl ;
  C_Transport        **m_transport_table ;
  int                  m_transport_table_size ;

  struct timeval       m_select_timeout ;

  C_GeneratorStats    *m_stat   ;
  C_GeneratorConfig   *m_config ;

  size_t               m_max_event_nb ;
  T_pC_TransportEvent  m_events ;

  int                  m_nb_global_channel ;

  T_SelectDef          m_call_select ;
  T_TrafficType        m_last_traffic_type ;

  // TaskController related methods
  T_GeneratorError TaskProcedure();
  T_GeneratorError InitProcedure();
  T_GeneratorError EndProcedure();
  T_GeneratorError StoppingProcedure();
  T_GeneratorError ForcedStoppingProcedure();

  void endTrafficControl () ;

  // Internal methods
  T_GeneratorError receiveControl () ;


  void transport_table () ;
  void create_call_controller () ;
  void start_call_controller () ;


} ;

typedef C_ReadControl * T_pC_ReadControl ;

#endif  // _C_READ_CONTROL_H
