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
#include "C_CommandActionFactory.hpp"

#include "C_ResponseTimeLog.hpp"

class C_CounterDef {
public:
  C_CounterDef (int P_id, char * P_name, T_UnsignedInteger32 P_init) {
    m_id = P_id;
    m_name = P_name;
    m_init = m_value = P_init;
  }
  virtual ~C_CounterDef () {
  }
  
  virtual void increase_counter() {
    m_value++;
  }
  
  inline T_UnsignedInteger32 get_counter_value() {
    return m_value;
  }
  
  inline int get_counter_id() {
    return m_id;
  }
  
protected:
  // Init an dcurrent value of counter
  T_UnsignedInteger32 m_value ;
  T_UnsignedInteger32 m_init  ;
  
private:
  int                 m_id    ;
  char               *m_name  ;
} ;

typedef enum _enum_counter_behaviour {
  E_CNT_RESET_MIN_TYPE = 0,
  E_CNT_RESET_INIT_TYPE,
  E_CNT_NO_RESET_TYPE
} T_CounterBehaviour, *T_pCounterBehaviour ;


class C_LimitedCounterDef : C_CounterDef {
public:
  C_LimitedCounterDef (int P_id, char * P_name, 
                       T_UnsignedInteger32 P_init,
                       T_UnsignedInteger32 P_min,
                       T_UnsignedInteger32 P_max,
                       T_CounterBehaviour  P_behaviour) : 
    C_CounterDef(P_id, P_name, P_init) {
    m_min = P_min;
    m_max = P_max;
    m_behaviour = P_behaviour;
  }
  virtual ~C_LimitedCounterDef () {
  }
  
  virtual void increase_counter() {
    if (m_value == m_max) {
      switch(m_behaviour) {
      case E_CNT_RESET_MIN_TYPE:
        m_value = m_min;
        break;         
      case E_CNT_RESET_INIT_TYPE: 
        m_value = m_init;
        break;
      case E_CNT_NO_RESET_TYPE:   
        // nothing to do counter value stay to max
        break;
      }
    } else {
      m_value++;
    }
  }
  
private:
  T_UnsignedInteger32 m_min  ;
  T_UnsignedInteger32 m_max ;
  T_CounterBehaviour m_behaviour;
} ;

typedef struct _label_data {
  int              m_idx               ;
  int              m_id_cmd            ;
  int              m_id_msg            ;
  int              m_nb_body_value     ;
  bool             m_select_line_added ;
  C_Scenario      *m_scenario          ;
} T_LabelData,  *T_pLabelData ;
typedef list_t<T_pLabelData> T_LabelDataList ;

typedef struct _struct_retrieve_id_def {
  int          m_nb_ids     ;
  int         *m_id_table  ;
} T_RetrieveIdsDef, *T_pRetrieveIdsDef ;


typedef list_t<char*> T_CharList, *T_pCharList ;

typedef set_t<unsigned long> T_waitValuesSet, *T_pWaitValuesSet ;
typedef set_t<unsigned long> T_retransDelayValuesSet, *T_pRetransDelayValuesSet ;

class C_ReadControl ; // prevent from circular include

class C_CommandAction ; 

typedef list_t<C_CommandAction *> T_CommandActionLst, 
  *T_pCommandActionLst ;

typedef list_t<int> T_IdRetrieveList, *T_pIdRetrieveList ;

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



  bool                set_counters        (C_XmlData *P_xml_counter_def);
  int                 get_label_id             (char * P_name) ;
  T_pLabelData        get_label_data           (int P_id)      ;
  bool                get_correlation_section  ()              ;
  T_pRetrieveIdsDef   get_id_table_by_channel  (int P_id) ;
  T_UnsignedInteger32 get_counter_value (int P_id);
  void                increase_counter  (int P_id);
  int                 get_counter_id    (char *P_name);

  int                 get_nb_scenario   ();
  int                 get_nb_default_scenario () ;
  int                 get_max_nb_retrans () ;
  int                 get_max_nb_send () ;
  int                 get_max_nb_recv () ;
  void                set_rsp_time_logger(C_ResponseTimeLog *P_rsptimelog);


private:


  T_Controllers  m_controllers;

  // memory store/restore name management
  typedef map_t<string_t, int> T_MemoryIdMap, *T_pMemoryIdMap ; 
  typedef map_t<string_t, int> T_LabelMap, *T_pLabelMap ;
  typedef map_t<string_t, T_CommandActionLst> T_CommandActionMap, *T_pCommandActionMap ;

  T_pMemoryIdMap  m_memory_map ;
  int             m_memory_max ;

  T_pLabelMap        m_label_map           ;
  T_pLabelData      *m_label_table         ;
  int                m_label_table_size    ;
  T_pC_IdGenerator   m_id_label_gen        ;

  T_pCommandActionMap m_pre_action_map     ;
  T_pCommandActionMap m_post_action_map    ;

  bool               m_correlation_section ;

  int                m_nb_channel          ;
  T_pCharList        m_correl_name_list    ;
  
  
  T_pRetrieveIdsDef  m_retrieve_id_table ;

  // counter name management
  typedef map_t<string_t, C_CounterDef*> T_CounterMap, *T_pCounterMap ;
  T_pCounterMap   m_counter_map   ;
  int             m_nb_counter    ;
  C_CounterDef ** m_counter_table ;

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

  // Response Time Log
  C_ResponseTimeLog  *m_rsp_time_log;

  // external data management
  C_ExternalDataControl  *m_external_data ;
  bool m_external_data_used ;

  unsigned int m_check_level ;
  T_CheckBehaviour m_check_behaviour ;

  // wait values
  T_pWaitValuesSet           m_wait_values ;

  bool                       m_retrans_enabled      ;
  T_pRetransDelayValuesSet   m_retrans_delay_values ;

  // generator model
  C_ProtocolControl  *m_protocol_ctrl  ;
  C_TransportControl *m_transport_ctrl ;
  C_ChannelControl   *m_channel_ctrl   ;

  bool                analyze_correlation (C_XmlData *P_xml_correlation,
                                           bool      *P_data_mesure);
  bool                analyze_correlation_retrieve (C_XmlData *P_data,
                                                    int        P_channel_id);
  bool                analyze_correlation_command  (C_XmlData *P_data,
                                                    int        P_channel_id,
                                                    bool      *P_data_mesure);
  bool                find_action                  (C_XmlData          *P_data,
                                                    char               *P_name);

  void                find_list_correl_from_map    (T_pCommandActionMap P_action_map,
                                                    char               *P_name,
                                                    T_CommandActionLst& P_list_actions) ;

  void                delete_list_correl_from_map  (T_pCommandActionMap P_action_map,
                                                    char               *P_name,
                                                    bool                P_reset);

  void                delete_correlation_map  (T_pCommandActionMap *P_action_map) ;

  void                update_actions (T_CommandActionLst&  P_ActionLabelList,
                                      T_CommandActionLst&  P_ActionMapLst,
                                      T_pCommandActionMap  P_action_map,
                                      T_pCmd_scenario      P_cmd_sequence,
                                      char                *P_value_label,
                                      T_pLabelData         P_label_data,
                                      bool                 P_pre_action) ;

  int add_scenario (T_scenario_type           P_type,
		    C_XmlData                *P_scen,
		    T_pTrafficType            P_trafficType,
		    int                      *P_nbOpen,
                    T_LabelDataList&          P_LabelDataList,
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
                   T_LabelDataList&          P_LabelDataList,
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
		   int&                      P_nb_action,
		   bool&                     P_inserted,
		   int                       P_nb_value,
		   int                       P_msg_id,
                   C_CommandActionFactory&   P_CmdActionFactory,
                   T_CommandActionLst&       P_CommandActionLst,
                   C_CommandAction**&        P_CommandActionTable,
                   char                     *P_label_command,
                   T_CommandActionLst&       P_CommandActionLabelLst,
                   bool                      P_insert_correlation
                   ) ;

  int add_expression (char                *P_arg,
		      C_ProtocolFrame          *P_protocol,
		      T_pStringExpression *P_strExpr);	

  int check_expression (T_pCmdAction  P_action,
			C_ProtocolFrame   *P_protocol) ;


  int check_channel_usage(T_pTrafficType P_channelUsageTable,
			  int            P_primary_channel_id,
			  int            P_nb_channel);
  
  int add_memory    (char *P_mem_name) ;
  int check_memory  (char *P_mem_name) ;



} ;

typedef C_ScenarioControl *T_pC_ScenarioControl ;

#endif // _C_SCENARIO_CONTROL_H









