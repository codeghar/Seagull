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

#ifndef _C_SCENARIO_CONTROL_H
#define _C_SCENARIO_CONTROL_H

#include "iostream_t.hpp"
#include "map_t.hpp"
#include "set_t.hpp"
#include "string_t.hpp"

#include "C_GeneratorConfig.hpp"
#include "GeneratorXmlData.hpp"
#include "C_XmlData.hpp"
#include "C_Scenario.hpp"
#include "TrafficType.h"

#include "C_ProtocolControl.hpp"
#include "C_TransportControl.hpp"
#include "C_ChannelControl.hpp"

#include "C_DataLogControl.hpp"

#include "C_ExternalDataControl.hpp"

typedef struct _counter_def_t {
  int                 m_id    ;
  char               *m_name  ;
  T_UnsignedInteger32 m_init  ;
  T_UnsignedInteger32 m_value ;
} T_CounterDef, *T_pCounterDef ;

typedef set_t<unsigned long> T_waitValuesSet, *T_pWaitValuesSet ;
typedef set_t<unsigned long> T_retransDelayValuesSet, *T_pRetransDelayValuesSet ;

class C_ReadControl ; // prevent from circular include

class C_ScenarioControl {
public:
   C_ScenarioControl(C_ProtocolControl  *P_protocol_control,
		     C_TransportControl *P_transport_control,
		     C_ChannelControl   *P_channel_control);
  ~C_ScenarioControl();

  typedef enum _enum_scen_type {
    E_SCENARIO_TRAFFIC = 0 ,
    E_SCENARIO_DEFAULT,
    E_SCENARIO_INIT,
    E_SCENARIO_ABORT
  } T_scenario_type, T_pScenario_type ;

  void set_data_log_controller (C_DataLogControl *P_log);

  void set_external_data_controller (C_ExternalDataControl *P_external_data);
  bool external_data_used() ;

  bool fromXml (C_XmlData *P_data, 
		bool  P_check_msg,
		bool *P_data_mesure,
		T_pTrafficType P_traffic_type,
                bool  *P_action_check_abort);
  
  T_pC_Scenario find_scenario (T_pReceiveMsgContext P_rcvMsg);
  T_pC_Scenario find_default_scenario (T_pReceiveMsgContext P_rcvMsg);

  T_pC_Scenario get_init_scenario () ;
  T_pC_Scenario get_traffic_scenario () ;

  T_pC_Scenario get_abort_scenario () ;
  T_pC_Scenario get_default_scenario (int L_index) ;

  T_pC_Scenario outgoing_scenario () ;
  T_pC_Scenario init_scenario_defined (T_pTrafficType P_type) ;

  void set_default_scenarii(int P_nb) ;
  void switch_to_init() ;
  void switch_to_traffic();

  void set_config (C_GeneratorConfig *P_config);

  int  memory_used () ; 

  T_pWaitValuesSet get_wait_values () ;
  T_pRetransDelayValuesSet get_retrans_delay_values () ;
  void             update_wait_cmd (size_t P_nb, unsigned long *P_table);

  void  update_retrans_delay_cmd (size_t P_nb, unsigned long *P_table);

  friend iostream_output& operator<<(iostream_output&, C_ScenarioControl&);

  bool                set_counters      (C_XmlData *P_xml_counter_def);
  T_UnsignedInteger32 get_counter_value (int P_id);
  void                increase_counter  (int P_id);
  int                 get_counter_id    (char *P_name);

  void set_call_map (T_pCallMap *P_call_map);

  int           get_nb_scenario   ();
  int           get_nb_default_scenario () ;
  int           get_max_nb_retrans () ;

private:


  // memory store/restore name management
  typedef map_t<string_t, int> T_MemoryIdMap, *T_pMemoryIdMap ; 
  T_pMemoryIdMap  m_memory_map ;
  int             m_memory_max ;

  // counter name management
  typedef map_t<string_t, T_pCounterDef> T_CounterMap, *T_pCounterMap ;
  T_pCounterMap   m_counter_map   ;
  int             m_nb_counter    ;
  T_pCounterDef  *m_counter_table ;
  

  // traffic scenario management
  T_pC_Scenario   m_traffic_scen, m_save_traffic_scen ;
  T_TrafficType   m_traffic_type ;

  // nb scenario
  int             m_nb_scenario  ;

  // init scenario management
  T_pC_Scenario   m_init_scen    ;
  T_TrafficType   m_init_type    ;

  // abort scenario management
  T_pC_Scenario   m_abort_scen   ;

  bool            m_action_check_abort ;

  // default scenario management
  T_pC_Scenario  *m_default_scen ;
  int            m_max_default  ;
  int            m_nb_default   ;

  // configuration data
  C_GeneratorConfig *m_config ;

  // data log control
  C_DataLogControl  *m_log ;

  // external data management
  C_ExternalDataControl  *m_external_data ;
  bool m_external_data_used ;

  // wait values
  T_pWaitValuesSet           m_wait_values ;

  bool                       m_retrans_enabled      ;
  T_pRetransDelayValuesSet   m_retrans_delay_values ;

  // generator model
  C_ProtocolControl  *m_protocol_ctrl  ;
  C_TransportControl *m_transport_ctrl ;
  C_ChannelControl   *m_channel_ctrl   ;


  int add_scenario (T_scenario_type           P_type,
		    C_XmlData                *P_scen,
		    T_pTrafficType            P_trafficType,
		    int                      *P_nbOpen,
		    bool                     *P_data_mesure,
		    bool                      P_checkMsgRecv = false);


  char* get_behaviour_scenario(C_XmlData     *P_scen);

  int add_command (T_cmd_type P_cmd_type, 
		   C_XmlData *P_data,
		   T_pTrafficType            P_trafficType,
		   T_pC_Scenario             P_scen,
		   int                      *P_nbOpen,
		   bool                     *P_data_mesure,
		   bool                     *P_map_inserted,
		   bool                     *P_selectLine_added,
		   T_pTrafficType            P_channelTrafficTable,
		   int                      *P_channel_primary_id,
		   bool                      P_checkMsgRecv = false
		   ) ;


  int add_actions (C_XmlData *P_msgData,
		   T_InstanceDataList& P_instance_list,
		   C_ProtocolFrame *P_protocol,
		   T_pC_Scenario             P_scen,
		   int                      *P_nbOpen,
		   bool                     *P_data_mesure,
		   bool                     *P_map_inserted,
		   bool                     *P_selectLine_added,
		   int&                      P_channel_id,
		   bool&                     P_pre_action,
		   bool&                     P_pre_action_done,
		   T_pCmdAction&             P_CmdActionTable,
		   int&                      P_nb_action,
		   T_CmdActionList&          P_CmdActionList,
		   bool&                     P_inserted,
		   int                       P_nb_value,
		   int                       P_msg_id
                   ) ;

  int add_expression (char                *P_arg,
		      C_ProtocolFrame          *P_protocol,
		      T_pStringExpression *P_strExpr);	

  int check_expression (T_pCmdAction  P_action,
			C_ProtocolFrame   *P_protocol) ;


  int check_channel_usage(T_pTrafficType P_channelUsageTable,
			  int            P_primary_channel_id,
			  int            P_nb_channel);
  
  int add_memory  (char *P_mem_name) ;


} ;

typedef C_ScenarioControl *T_pC_ScenarioControl ;

#endif // _C_SCENARIO_CONTROL_H









