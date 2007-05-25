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

#include "C_ScenarioControl.hpp"
#include "Utils.hpp"
#include "ProtocolData.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_ReadControl.hpp"
#include "BufferUtils.hpp"

#include "C_ScenarioStats.hpp"
#include "C_CommandActionFactory.hpp"
#include "C_GeneratorStats.hpp"
#include "C_CommandAction.hpp"

#include "list_t.hpp"
#include "integer_t.hpp" // For strtoul_f
#include <regex.h>



#define XML_SCENARIO_SECTION               (char*)"scenario"
#define XML_SCENARIO_COUNTER_SECTION       (char*)"counter"
#define XML_SCENARIO_TRAFFIC_SECTION       (char*)"traffic"
#define XML_SCENARIO_DEFAULT_SECTION       (char*)"default"
#define XML_SCENARIO_INIT_SECTION          (char*)"init"
#define XML_SCENARIO_ABORT_SECTION         (char*)"abort"
#define XML_SCENARIO_CORRELATION_SECTION   (char*)"correlation"

static const T_CmdAction no_cmd_action = {
  E_ACTION_SCEN_OPEN,
  NULL,
  -1, -1, -1, -1, -1, -1,
  NULL,
  -1, -1,
  NULL,
  E_CHECK_BEHAVIOUR_WARNING,
  -1,
  NULL,
  NULL,
  E_NOTHING_TYPE
} ;
  
C_ScenarioControl::C_ScenarioControl(C_ProtocolControl  *P_protocol_control,
				     C_TransportControl *P_transport_control,
				     C_ChannelControl   *P_channel_control) {
  m_traffic_scen = NULL ;
  m_init_scen = NULL ;
  m_abort_scen = NULL ;
  m_default_scen = NULL ;
  m_traffic_type = E_TRAFFIC_UNKNOWN ;
  m_init_type = E_TRAFFIC_UNKNOWN ;
  m_max_default = 0 ;
  m_nb_default  = 0 ;
  m_nb_scenario = 0 ;
  NEW_VAR(m_memory_map, T_MemoryIdMap());
  m_memory_map->clear();
  m_memory_max = 0 ;
  m_save_traffic_scen = NULL ;
  m_config = NULL ;
  m_log = NULL ;
  NEW_VAR(m_wait_values, T_waitValuesSet());

  m_retrans_enabled = false;
  NEW_VAR(m_retrans_delay_values, T_retransDelayValuesSet());

  m_external_data = NULL ;
  m_external_data_used = false ;

  m_check_level = 0 ;
  m_check_behaviour = E_CHECK_BEHAVIOUR_WARNING ;

  // generator model
  m_protocol_ctrl = P_protocol_control ;
  m_transport_ctrl = P_transport_control ;
  m_channel_ctrl = P_channel_control ;
  m_nb_channel = 0 ;
  
  m_retrieve_id_table = NULL ;
  NEW_VAR(m_label_map, T_LabelMap());
  m_label_map->clear();


  NEW_VAR(m_pre_action_map, T_CommandActionMap());
  NEW_VAR(m_post_action_map, T_CommandActionMap());
  m_pre_action_map ->clear()     ;
  m_post_action_map ->clear()     ;

  m_label_table = NULL ;
  m_label_table_size = 0 ;
  NEW_VAR(m_id_label_gen, C_IdGenerator()) ;
  m_correlation_section = false ;

  NEW_VAR (m_correl_name_list, T_CharList);
  m_correl_name_list->clear();
 
  NEW_VAR(m_counter_map, T_CounterMap()) ;
  m_nb_counter = 0 ;
  m_counter_table = NULL ;
  m_action_check_abort = false ;
}

C_ScenarioControl::~C_ScenarioControl() {

  int L_i ;

  if (m_init_scen == m_traffic_scen) {
    m_traffic_scen = m_save_traffic_scen ;
  }

  if (m_traffic_scen) {
    m_traffic_scen->delete_stats();
  }
  DELETE_VAR(m_traffic_scen);
  
  if (m_init_scen) {
    m_init_scen->delete_stats();
  }
  DELETE_VAR(m_init_scen);

  if (m_abort_scen) {
    m_abort_scen->delete_stats();
  }
  DELETE_VAR(m_abort_scen);

  if (m_nb_default) {
    for(L_i=0; L_i < m_nb_default; L_i++) {
      if (m_default_scen[L_i]) {
	m_default_scen[L_i]->delete_stats() ;
      }
    }
  }
  FREE_TABLE(m_default_scen);

  m_traffic_type = E_TRAFFIC_UNKNOWN ;
  m_init_type = E_TRAFFIC_UNKNOWN ;
  m_max_default = 0 ;
  m_nb_default = 0 ;
  m_memory_max = 0 ;
  if (!m_memory_map->empty()) {
    m_memory_map->erase(m_memory_map->begin(), m_memory_map->end());
  }
  DELETE_VAR(m_memory_map);

  m_save_traffic_scen = NULL ;
  m_config = NULL ;
  if (!m_wait_values->empty()) {
    m_wait_values->erase(m_wait_values->begin(), m_wait_values->end());
  }
  DELETE_VAR(m_wait_values);

  if (!m_retrans_delay_values->empty()) {
    m_retrans_delay_values->erase(m_retrans_delay_values->begin(), m_retrans_delay_values->end());
  }
  DELETE_VAR(m_retrans_delay_values);

  if (m_nb_counter != 0){
    for (L_i = 0; L_i < m_nb_counter; L_i++) {
      DELETE_VAR(m_counter_table[L_i]);
    }
    DELETE_TABLE(m_counter_table);
    m_nb_counter = 0 ;
  }
  if (!m_counter_map->empty()) {
    m_counter_map->erase(m_counter_map->begin(), m_counter_map->end());
  }
  DELETE_VAR(m_counter_map);

  m_action_check_abort = false ;

  if (m_retrieve_id_table != NULL) {
    for (L_i = 0; L_i < m_nb_channel; L_i++) {
      FREE_TABLE(m_retrieve_id_table[L_i].m_id_table);
      m_retrieve_id_table[L_i].m_id_table = NULL ;
      m_retrieve_id_table[L_i].m_nb_ids = 0 ;
    }
    FREE_TABLE(m_retrieve_id_table);
  }
  m_nb_channel = 0 ;


  if (!m_label_map->empty()) {
    m_label_map->erase(m_label_map->begin(), m_label_map->end());
  }
  DELETE_VAR(m_label_map);

  if (m_label_table_size != 0) {
    for (L_i = 0 ; L_i < m_label_table_size; L_i++) {
      FREE_VAR(m_label_table[L_i]);
    }
    FREE_TABLE(m_label_table);
    m_label_table_size = 0 ;
  }
  DELETE_VAR(m_id_label_gen) ;
  m_correlation_section = false ;

  if (!m_correl_name_list->empty()) {
    m_correl_name_list->erase(m_correl_name_list->begin(),
                              m_correl_name_list->end());
    
  }
  DELETE_VAR(m_correl_name_list);

  if (!m_pre_action_map->empty()) {
    m_pre_action_map->erase(m_pre_action_map->begin(), m_pre_action_map->end());
  }
  DELETE_VAR(m_pre_action_map);


  if (!m_post_action_map->empty()) {
    m_post_action_map->erase(m_post_action_map->begin(), m_post_action_map->end());
  }
  DELETE_VAR(m_post_action_map);

}

void C_ScenarioControl::set_data_log_controller (C_DataLogControl *P_log) {

  int L_i ;

  GEN_DEBUG(1, "C_ScenarioControl::set_data_log_controller() start");
  m_log = P_log ;
  if (m_traffic_scen) { 
	  m_traffic_scen->set_data_log_controller(m_log); 
  }
  if (m_save_traffic_scen) { 
	  m_save_traffic_scen->set_data_log_controller(m_log); 
  }
  if (m_init_scen) { 
	  m_init_scen->set_data_log_controller(m_log);
  }
  if (m_abort_scen) { 
	  m_abort_scen->set_data_log_controller(m_log); 
  }
  if (m_nb_default) {
    for (L_i = 0; L_i < m_nb_default; L_i++) {
      m_default_scen[L_i]->set_data_log_controller(m_log);
    }
  }
  GEN_DEBUG(1, "C_ScenarioControl::set_data_log_controller() end");
}

void C_ScenarioControl::set_external_data_controller (C_ExternalDataControl *P_external_data) {
  GEN_DEBUG(1, "C_ScenarioControl::set_external_data_controller() start");
  m_external_data = P_external_data ;  
  GEN_DEBUG(1, "C_ScenarioControl::set_external_data_controller() end");
}


void C_ScenarioControl::set_default_scenarii (int P_nb) {
  if (m_nb_default == 0) {
    m_max_default = P_nb ;
    ALLOC_TABLE(m_default_scen,
		T_pC_Scenario*,
		sizeof(T_pC_Scenario),
		m_max_default);
  } else {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Default number already setted");
  }
}

T_pC_Scenario C_ScenarioControl::outgoing_scenario () {
  T_pC_Scenario L_scen = NULL ;
  if (m_traffic_type == E_TRAFFIC_CLIENT) {
    L_scen = m_traffic_scen ;
  }
  return (L_scen);
}

T_pC_Scenario C_ScenarioControl::find_scenario (T_pReceiveMsgContext P_rcvMsg) {

  T_pC_Scenario L_ret ;
  
  GEN_DEBUG(1, "C_ScenarioControl::find_scenario() start");

  //GEN_DEBUG(1, "P_msg [" << (*P_msg) << "]");

  L_ret = (m_traffic_scen->check_msg_received(P_rcvMsg)) 
    ? m_traffic_scen : find_default_scenario(P_rcvMsg) ;
  GEN_DEBUG(1, "C_ScenarioControl::find_scenario() end");
  return (L_ret);

}

T_pC_Scenario C_ScenarioControl::get_default_scenario (int P_index) {

  T_pC_Scenario L_ret = NULL ;

  if (P_index < m_nb_default) {
    L_ret = m_default_scen[P_index];
  }
  
  return (L_ret);

}


T_pC_Scenario C_ScenarioControl::find_default_scenario (T_pReceiveMsgContext P_rcvMsg) {

  T_pC_Scenario L_ret = NULL ;
  int           L_i ;

  GEN_DEBUG(1, "C_ScenarioControl::find_default_scenario() start");
  GEN_DEBUG(1, "m_nb_default " << m_nb_default);
  
  for (L_i = 0 ;
       L_i < m_nb_default;
       L_i++) {
    if (m_default_scen[L_i]->check_msg_received(P_rcvMsg)) {
      L_ret = m_default_scen[L_i];
      break ;
    }
  }
  
  GEN_DEBUG(1, "C_ScenarioControl::find_default_scenario() end");
  return (L_ret);

}

T_pC_Scenario C_ScenarioControl::get_abort_scenario () {
  return (m_abort_scen);
}


T_pC_Scenario C_ScenarioControl::get_init_scenario () {
  return (m_init_scen);
}

T_pC_Scenario C_ScenarioControl::get_traffic_scenario () {
  if (m_save_traffic_scen != NULL) {
    return (m_save_traffic_scen);
  } else {
    return (m_traffic_scen);
  }
}



char* C_ScenarioControl::get_behaviour_scenario(C_XmlData     *P_scen) {

  char                                *L_value = NULL    ;
  C_XmlData::T_pXmlField_List          L_fields_list     ;
  C_XmlData::T_XmlField_List::iterator L_fieldIt         ;

  L_fields_list = P_scen->get_fields();
  if (!L_fields_list->empty()) {
    for(L_fieldIt  = L_fields_list->begin() ;
	L_fieldIt != L_fields_list->end() ;
	L_fieldIt++) {
      if(strcmp((*L_fieldIt)->get_name() , (char *)"behaviour") == 0) {
	L_value = (*L_fieldIt)->get_value() ;
	break ;
      }
    }
  }

  return (L_value);

}

int C_ScenarioControl::add_scenario

(T_scenario_type           P_type,
 C_XmlData                *P_scen,
 T_pTrafficType            P_trafficType,
 int                      *P_nbOpen,
 T_LabelDataList&          P_LabelDataList, 
 bool                     *P_data_mesure,
 bool                      P_checkMsgRecv) {

  C_XmlData                        *L_data ;
  T_pXmlData_List                   L_subList ;  
  T_XmlData_List::iterator          L_subListIt ;
  char                             *L_value ;

  T_cmd_type                        L_cmd_type = E_NB_CMD_SCEN ;
  bool                              L_cmd_ok = false ;
  size_t                            L_nb_cmd ;
  int                               L_i ; 
  int                               L_ret = 0 ;

  T_CmdAction                       L_action_end_init ;
  C_CommandAction**                 L_action_end_init_table = NULL ;
  T_pC_Scenario                     L_current_scen = NULL ;
  C_ScenarioStats                  *L_current_stat = NULL ;

  unsigned int                      L_check_level = 0 ;
  T_CheckBehaviour                  L_check_behave = E_CHECK_BEHAVIOUR_WARNING ;
  bool                              L_selectline_added = false ;

  bool                              *L_add_map_inserted = NULL ;
  T_pTrafficType                     L_channel_usage = NULL ;
  int                                L_primary_channel_id = -1 ;
  int                                L_nb_channel ;


  char                              *L_behaviour_scen = NULL ;


  m_controllers.m_log = m_log ;
  m_controllers.m_scenario_control = this ;
  m_controllers.m_stat = C_GeneratorStats::instance() ;
  m_controllers.m_external_data = m_external_data ;
  m_controllers.m_check_mask      = m_check_level ;
  m_controllers.m_check_behaviour      = m_check_behaviour ;
  m_controllers.m_channel_ctrl = m_channel_ctrl ;

  C_CommandActionFactory    L_CommandActionFactory(&m_controllers) ;


  GEN_DEBUG(1, "C_ScenarioControl::add_scenario() start");
  (*P_nbOpen) = 0 ;

  if (m_config != NULL) {
    m_retrans_enabled = m_config -> get_retrans_enabled();
  }

  L_behaviour_scen = get_behaviour_scenario(P_scen) ;
  if (L_behaviour_scen != NULL) {
    if ((strcmp(L_behaviour_scen, (char*)"failed")  != 0) && 
	(strcmp(L_behaviour_scen, (char*)"success") != 0) && 
        (strcmp(L_behaviour_scen, (char*)"ignore") != 0) ) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown behaviour scenario " << L_behaviour_scen);
      L_ret = -1 ;
      return (L_ret);
    }
  } 


  L_nb_channel = m_channel_ctrl->nb_channel();
  ALLOC_TABLE(L_add_map_inserted,
	      bool*,
	      sizeof(bool),
	      L_nb_channel);
  ALLOC_TABLE(L_channel_usage,
	      T_pTrafficType,
	      sizeof(T_TrafficType),
	      L_nb_channel);
  for(L_i = 0 ; L_i < L_nb_channel; L_i++) {
    L_add_map_inserted[L_i] = false ;
    L_channel_usage[L_i] = E_TRAFFIC_UNKNOWN ;
  }

  switch (P_type) {
  case E_SCENARIO_TRAFFIC:
    if (m_config != NULL) {
      m_config->get_value(E_CFG_OPT_CHECK_LEVEL, &L_check_level);
      m_config->get_value(E_CFG_OPT_CHECK_BEHAVIOUR, 
			  (unsigned int*)&L_check_behave);
    }
    m_check_level = L_check_level ;
    m_check_behaviour = L_check_behave ;
    NEW_VAR(m_traffic_scen, 
	    C_Scenario(this, m_channel_ctrl, 
		       m_external_data,
		       E_EXE_TRAFFIC_END,
		       L_behaviour_scen,
                       m_retrans_enabled,
		       L_check_level,
		       L_check_behave
		       ));
    L_current_scen = m_traffic_scen ;
    m_nb_scenario++;
    GEN_DEBUG(1, "C_ScenarioControl::add_scenario() E_SCENARIO_TRAFFIC ");
    break ;

  case E_SCENARIO_INIT:

    if (L_behaviour_scen != NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Behavior feature is not allowed on scenario init");
      L_ret = -1 ;
      return (L_ret);
    }

    NEW_VAR(m_init_scen, C_Scenario(this, m_channel_ctrl, 
				    m_external_data,
                                    E_EXE_INIT_END,
                                    L_behaviour_scen,
                                    m_retrans_enabled)) ;

    L_current_scen = m_init_scen ;
    m_nb_scenario++;
    GEN_DEBUG(1, "C_ScenarioControl::add_scenario() E_SCENARIO_INIT ");
    break ;

  case E_SCENARIO_DEFAULT:
    if (m_nb_default == m_max_default) {
      GEN_FATAL(E_GEN_FATAL_ERROR, "Maximum number of default scenario reached");
    }
    NEW_VAR(m_default_scen[m_nb_default], 
	    C_Scenario(this, m_channel_ctrl, 
                       m_external_data,
                       E_EXE_DEFAULT_END,
                       L_behaviour_scen,
                       m_retrans_enabled));
    L_current_scen = m_default_scen[m_nb_default] ;
    GEN_DEBUG(1, "C_ScenarioControl::add_scenario() E_SCENARIO_DEFAULT ");
    m_nb_default ++ ;
    m_nb_scenario++;
    break ;

  case E_SCENARIO_ABORT:
    NEW_VAR(m_abort_scen, 
	    C_Scenario(this, m_channel_ctrl, 
                       m_external_data,
                       E_EXE_ABORT_END,
                       L_behaviour_scen,
                       m_retrans_enabled));
    L_current_scen = m_abort_scen ;
    m_nb_scenario++;
    GEN_DEBUG(1, "C_ScenarioControl::add_scenario() E_SCENARIO_ABORT ");
    break ;
  }

  GEN_DEBUG(1, "C_ScenarioControl::add_scenario() nb scenario: " << m_nb_scenario);

  *P_trafficType = E_TRAFFIC_UNKNOWN;

  L_subList = P_scen->get_sub_data () ;
  if (!L_subList->empty()) {
    
    L_current_scen->set_size(L_subList->size()+1) ;
    
    for(L_subListIt  = L_subList->begin() ;
	L_subListIt != L_subList->end() ;
	L_subListIt++) {

      L_cmd_ok = false ;
      L_data = *L_subListIt ;
      L_value = L_data->get_name() ;

      for(L_i = 0;
	  L_i < (int)E_NB_CMD_SCEN ;
	  L_i++) {
	
        GEN_DEBUG(1, "C_ScenarioControl::add_scenario() command_name_table[" << 
	  	        L_i << "] is " << command_name_table[L_i]);
        GEN_DEBUG(1, "C_ScenarioControl::add_scenario() L_value is " << L_value);
	if (strcmp(L_value, command_name_table[L_i])==0) {
	  L_cmd_ok = true ;
	  L_cmd_type = (T_cmd_type) L_i ;
	  break ;
	}
      }

      if (L_cmd_ok == true) {
	L_ret = add_command(L_cmd_type, 
			    L_data,
			    P_trafficType,
			    L_current_scen,
			    P_nbOpen,
			    P_data_mesure,
			    L_add_map_inserted,
			    &L_selectline_added,
			    L_channel_usage,
			    &L_primary_channel_id,
                            P_LabelDataList, 
			    P_checkMsgRecv);
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown command " << L_value);
	L_ret = -1 ;
	break ;
      }
      if (L_ret == -1) break ;
    }
  }

  if (L_ret != -1) {
    // add final scenario end
    L_nb_cmd = L_current_scen->add_cmd(E_CMD_SCEN_END) ;

    switch (P_type) {
    case E_SCENARIO_TRAFFIC:
      m_traffic_type = *P_trafficType ;
      break ;
    case E_SCENARIO_INIT:
      m_init_type = *P_trafficType;

      L_action_end_init = no_cmd_action ;
      // add init done action at the end of the command
      L_action_end_init.m_type        = E_ACTION_SCEN_INTERNAL_INIT_DONE ;
      
      ALLOC_TABLE(L_action_end_init_table, 
                  C_CommandAction**, sizeof(C_CommandAction*),1);
      L_action_end_init_table[0] 
        = L_CommandActionFactory.create(L_action_end_init);
      L_nb_cmd = L_current_scen
        ->define_post_actions(1, (C_CommandAction**)L_action_end_init_table);
      break ;

    case E_SCENARIO_DEFAULT:
      if (*P_trafficType != E_TRAFFIC_SERVER) {
	L_ret = -1 ;
	GEN_ERROR(E_GEN_FATAL_ERROR, "Default scenario must be server");
      }

      if (L_nb_cmd > 3) { 
	L_ret = -1 ;
	GEN_ERROR(E_GEN_FATAL_ERROR, "Default scenario must have two commands");
      }
      break ;

    case E_SCENARIO_ABORT:
      if (*P_trafficType != E_TRAFFIC_CLIENT) {
	L_ret = -1 ;
	GEN_ERROR(E_GEN_FATAL_ERROR, "Abort scenario must be client");
      }
      break ;
    }
  }

  FREE_TABLE(L_add_map_inserted);
  
  if ((L_ret != -1) && 
      (m_correlation_section == false )) {

    L_ret = check_channel_usage (L_channel_usage, L_primary_channel_id, L_nb_channel);
  }
  FREE_TABLE(L_channel_usage);


  if (m_config->get_display_scenario_stat() == true) {
    NEW_VAR(L_current_stat, C_ScenarioStats(L_current_scen));
  }

  GEN_DEBUG(1, "C_ScenarioControl::add_scenario() end");

  return (L_ret);
  
}


bool C_ScenarioControl::find_action (C_XmlData          *P_data,
                                    char               *P_name) {
  T_pXmlData_List                L_subList           = NULL     ;
  T_XmlData_List::iterator       L_listIt                       ;
  C_XmlData                     *L_msgData                      ;

  T_pXmlData_List                L_actionList        = NULL     ;  
  T_XmlData_List::iterator       L_actionListIt                 ;
  C_XmlData                     *L_action                       ;
  bool                           L_actionFound       = false    ;
  
  L_subList = P_data -> get_sub_data() ;
  for(L_listIt  = L_subList->begin() ;
      L_listIt != L_subList->end() ;
      L_listIt++) {
    L_msgData = *L_listIt ;
    if (strcmp(L_msgData -> get_name (), (char*)"action") == 0) {
      L_actionList = L_msgData -> get_sub_data() ;
      if (L_actionList != NULL) {
        for (L_actionListIt  = L_actionList->begin();
             L_actionListIt != L_actionList->end();
             L_actionListIt++) {
          L_action = *L_actionListIt ;
          if (strcmp(L_action -> get_name(), P_name)==0) {
            L_actionFound = true ;
            break ;
          }
        }
      }
    }
    if (L_actionFound == true ) break;
  } // for 
  return (L_actionFound) ;
}


void C_ScenarioControl::find_list_correl_from_map(T_pCommandActionMap P_action_map,
                                                  char               *P_name,
                                                  T_CommandActionLst& P_list_actions) {
  T_CommandActionMap::iterator      L_it                       ; 
  L_it = P_action_map->find(T_CommandActionMap::key_type(P_name));
  if (L_it != P_action_map->end()) {
    P_list_actions = L_it->second ;
  }
}


void C_ScenarioControl::delete_list_correl_from_map(T_pCommandActionMap P_action_map,
                                                    char               *P_name,
                                                    bool                P_reset) {
  T_CommandActionLst                L_commandActionLst  ;
  T_CommandActionMap::iterator      L_it                       ; 
  
  T_CommandActionLst::iterator      L_list_it                  ;

  
  L_it = P_action_map->find(T_CommandActionMap::key_type(P_name));
  if (L_it != P_action_map->end()) {
    L_commandActionLst = L_it->second ;
    if (P_reset) {
      for (L_list_it = L_commandActionLst.begin();
           L_list_it != L_commandActionLst.end();
           L_list_it++ ) {
        DELETE_VAR(*L_list_it) ;
      }
    }

    if (!L_commandActionLst.empty()) {
      L_commandActionLst.erase(L_commandActionLst.begin(),
                                L_commandActionLst.end());
    }
    P_action_map->erase (L_it);
  }
}


int C_ScenarioControl::add_command (T_cmd_type                P_cmd_type, 
				    C_XmlData                *P_data,
				    T_pTrafficType            P_trafficType,
				    T_pC_Scenario             P_scen,
				    int                      *P_nbOpen,
				    bool                     *P_data_mesure,
				    bool                     *P_map_inserted,
				    bool                     *P_selectLine_added,
				    T_pTrafficType            P_channelTrafficTable,
				    int                      *P_channel_primary_id,
                                    T_LabelDataList&          P_LabelDataList,
				    bool                      P_checkMsgRecv) {

  
  int                            L_channel_id        = -1       ;

  char                          *L_value                        ;
  int                            L_ret               = 0        ;
  C_CommandAction**             L_CommandActionTable    = NULL     ;
  int                            L_nb_action         = 0        ;

  T_CommandActionLst             L_commandActionList            ;

  T_CommandActionLst             L_commandActionLabelList       ;

  bool                           L_map_inserted      = false    ;

  bool                           L_pre_action        = false    ;
  bool                           L_cmd_inserted      = false    ;
  bool                           L_pre_action_done   = false    ;


  T_pXmlData_List                L_subList           = NULL     ;
  T_XmlData_List::iterator       L_listIt                       ;
  C_XmlData                     *L_msgData                      ;
  char                          *L_currentName                  ;
  C_ProtocolFrame               *L_protocol          = NULL     ;
  T_InstanceDataList             L_instance_list                ;
  int                            L_nb_body_value     = -1       ;

  int                            L_msg_id            = -1       ;

  char                          *L_value_retrans     = NULL     ;
  unsigned long                  L_retrans_delay     = 0        ;
  char                          *L_end_str                      ;


  size_t                         L_nb_cmd                       ;
  char                          *L_value_label       = NULL     ;
  int                            L_label_id_table               ;
  T_pLabelData                   L_label_data        = NULL     ;
  bool                           L_insert_pre        = false    ;


  m_controllers.m_log = m_log ;
  m_controllers.m_scenario_control = this ;
  m_controllers.m_stat = C_GeneratorStats::instance() ;
  m_controllers.m_external_data = m_external_data ;
  m_controllers.m_check_mask      = m_check_level ;
  m_controllers.m_check_behaviour      = m_check_behaviour ;
  m_controllers.m_channel_ctrl = m_channel_ctrl ;

  C_CommandActionFactory    L_CommandActionFactory(&m_controllers) ;
  T_CmdAction L_action_check ;


  GEN_DEBUG(1, "C_ScenarioControl::add_command() start");

  // L_cmdActionList.clear() ;
  L_commandActionList.clear() ;
  L_commandActionLabelList.clear();

  switch (P_cmd_type) {

  case E_CMD_SCEN_RECEIVE:
    if (P_checkMsgRecv == true) {
      // T_CmdAction L_action_check ;
      L_action_check = no_cmd_action ;
      L_action_check.m_type = E_ACTION_SCEN_CHECK_ALL_MSG ;
      L_commandActionList.push_back
        (L_CommandActionFactory.create(L_action_check));
      L_nb_action++ ;
      GEN_DEBUG(1, "C_ScenarioControl::add_command() E_CMD_SCEN_RECEIVE");
    }
  case E_CMD_SCEN_SEND: {
	  
    C_MessageFrame           *L_msg = NULL ;
    char                     *L_msg_name = NULL ;
    char                      L_no_msg_name [1] ;

    
    //    L_transid = -1 ;
    L_no_msg_name [0] = '\0' ;
    L_value = P_data->find_value((char*)"channel");
    if (L_value != NULL) {
      L_channel_id = m_channel_ctrl->get_channel_id(L_value);
      if (L_channel_id  == ERROR_CHANNEL_UNKNOWN) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "Chanel [" << L_value << "] unknowm for the send command");
	L_ret = -1 ;
	break ;
      }
      L_map_inserted = (P_map_inserted[L_channel_id] == true);
      if ((*P_channel_primary_id) == -1) {
	*P_channel_primary_id = L_channel_id ;
      }
      if (P_channelTrafficTable[L_channel_id] == E_TRAFFIC_UNKNOWN) {
	switch(P_cmd_type) {
	case E_CMD_SCEN_SEND:
	  P_channelTrafficTable[L_channel_id]=E_TRAFFIC_CLIENT;
	  break ;
	case E_CMD_SCEN_RECEIVE:
	  P_channelTrafficTable[L_channel_id]=E_TRAFFIC_SERVER;
	  break;
	default:
	  break;
	}
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Channel not specified for command");
      L_ret = -1 ;
      break ;
    }

    L_value_label = P_data->find_value((char*)"label");
    if (L_value_label != NULL) {
      if (m_label_map->find(T_LabelMap::key_type(L_value_label)) 
          != m_label_map->end()) {
        GEN_ERROR(E_GEN_FATAL_ERROR, 
                  "label with name ["
                  << L_value_label << "] already defined");
        L_ret = -1 ;
        break ;
      }
    }
    
    L_protocol = m_channel_ctrl->get_channel_protocol(L_channel_id) ;

    L_value_retrans = P_data->find_value((char*)"retrans");
    if (L_value_retrans != NULL) {
      L_retrans_delay = strtoul_f (L_value_retrans, &L_end_str, 10);
      if (L_end_str[0] != '\0') {
        GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                  << L_value_retrans << "] not a number");
        L_ret = -1 ;
        break;
      }
    }

    L_subList = P_data -> get_sub_data() ;
    for(L_listIt  = L_subList->begin() ;
	L_listIt != L_subList->end() ;
	L_listIt++) {
      
      L_msgData = *L_listIt ;
      L_currentName = L_msgData -> get_name () ;
      L_msg_name = (L_protocol->message_name() == NULL) 
        ? L_no_msg_name : L_protocol->message_name() ;

      GEN_DEBUG(1, "C_ScenarioControl::add_command() msgData Name is "
                    << L_currentName );

      if (    (strcmp(L_currentName, (char*)"CDATA") == 0) 
	   || (strcmp(L_currentName, L_msg_name) == 0) ) {

	L_msg = L_protocol -> create_new_message ((void*)L_msgData,
						  &L_instance_list,
						  &L_nb_body_value) ;
	if (L_msg == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "Error while creating message");
	  L_ret = -1 ;
	  break ;
	}

        L_msg_id = L_msg->get_id_message() ;

	if ( L_msg_id == -1) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "Error while creating message[" 
                    << L_msg_id 
                    << "]");
	  L_ret = -1 ;
	  break ;
	}

	// finally add message on the scenario sequence
	if ((L_msg != NULL) && (L_channel_id != -1)) {
	  L_nb_cmd = P_scen
	    ->add_cmd (P_cmd_type, 
		       L_channel_id,
		       L_msg,
		       0,
		       NULL,
                       L_retrans_delay);
	  
	  L_cmd_inserted = true ;

          if (L_retrans_delay > 0 ) {
            m_retrans_delay_values->insert(T_retransDelayValuesSet::value_type(L_retrans_delay));
          }

          if (L_value_label != NULL) {
            ALLOC_VAR(L_label_data,
                      T_pLabelData,
                      sizeof(T_LabelData));
            
            L_label_id_table = m_id_label_gen->new_id() ;
            
            L_label_data->m_idx             = L_label_id_table    ;
            L_label_data->m_id_cmd          = L_nb_cmd - 1        ;
            L_label_data->m_id_msg          = L_msg_id            ;
            L_label_data->m_nb_body_value   = L_nb_body_value     ;
            L_label_data->m_select_line_added = 
              find_action(P_data, (char*)"restore-from-external");
            L_label_data->m_scenario        = P_scen              ;
            
            P_LabelDataList.push_back(L_label_data);

            m_label_map
              ->insert(T_LabelMap::value_type(L_value_label,
                                              L_label_id_table)) ;
          }

	  if (*P_trafficType == E_TRAFFIC_UNKNOWN) {
	    switch (P_cmd_type) {
	    case E_CMD_SCEN_RECEIVE:
	      *P_trafficType=E_TRAFFIC_SERVER;
	      break;
	    case E_CMD_SCEN_SEND:
	      *P_trafficType=E_TRAFFIC_CLIENT;
	      break;
	    default:
	      break;
	    }
	  }
	}

      } else if (strcmp(L_currentName, (char*) "action") == 0) {

	if (L_cmd_inserted == false) { 
	  L_pre_action = true ; 
	// } else {
	//   L_post_action = true ;
	}
	
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Unsupported message [" << L_currentName << "]");
	L_ret = -1 ;
      }
      if (L_ret == -1) break;
    } // for 
  }

    break ;
    
  case E_CMD_SCEN_WAIT:

    L_value = P_data->find_value((char*)"value");
    if (L_value == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "value is mandatory for wait-ms command");
      L_ret = -1 ;
    }
    if (L_ret != -1) {
      unsigned long  L_value_ms ;
      char          *L_end_str = NULL ;
      L_value_ms = strtoul_f (L_value, &L_end_str, 10);
      if (L_end_str[0] != '\0') {
	GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
	      << L_value << "] not a number");
	L_ret = -1 ;
      } else {
	(void) P_scen->add_cmd(P_cmd_type, L_value_ms);
	// update the number of wait values
	m_wait_values->insert(T_waitValuesSet::value_type(L_value_ms));
      }
    }
    break ;
    
  default:
    break ;
    
  }

  if (L_ret != -1) {
    switch (P_cmd_type) {

    case E_CMD_SCEN_RECEIVE:
    case E_CMD_SCEN_SEND: {
      L_value_label = NULL ;
      L_value_label = P_data->find_value((char*)"label");
      if ((m_correlation_section == true) &&
          (L_value_label != NULL) &&
          (L_nb_action > 0 )) {
        L_commandActionLabelList.push_back
          (L_CommandActionFactory.create(L_action_check));
      } 


      for(L_listIt  = L_subList->begin() ;
	  L_listIt != L_subList->end() ;
	  L_listIt++) {
	
        L_insert_pre  = false  ;

	L_msgData = *L_listIt ;
	L_currentName = L_msgData -> get_name () ;
	
	if (strcmp(L_currentName, (char*) "action") == 0) {
          if ((L_pre_action == true) &&
              (L_pre_action_done == false)) {
            L_insert_pre = true ;
          }

	  // action decoding 
	  L_ret = add_actions(L_msgData,
			      L_instance_list,
			      L_protocol,
			      P_scen,
			      P_nbOpen,
			      P_data_mesure,
			      P_map_inserted,
			      P_selectLine_added,
			      L_channel_id,
			      L_pre_action,
			      L_pre_action_done,
			      L_nb_action,
			      L_map_inserted,
			      L_nb_body_value,
                              L_msg_id,
                              L_CommandActionFactory,
                              L_commandActionList,
                              L_CommandActionTable,
                              L_value_label,
                              L_commandActionLabelList,
                              false
			      );

	  if (L_ret == -1) break ;

          if ((m_correlation_section == true) && 
              (L_value_label != NULL )) {
            
            if (L_insert_pre == true) {
              m_pre_action_map
                ->insert(T_CommandActionMap::value_type(L_value_label,
                                                        L_commandActionLabelList)) ;

            } else {
              m_post_action_map
                ->insert(T_CommandActionMap::value_type(L_value_label,
                                                        L_commandActionLabelList)) ;
            }

            if (!L_commandActionLabelList.empty()) {
              L_commandActionLabelList.erase(L_commandActionLabelList.begin(),
                                             L_commandActionLabelList.end());
            }
          }
	} // if
      } // for

      if (L_map_inserted == false) {
        if (m_correlation_section == false) {
          T_CmdAction L_action_map ;
          C_CommandAction** L_action_map_table ;
          ALLOC_TABLE(L_action_map_table, C_CommandAction**, sizeof(C_CommandAction*),1);
          L_action_map = no_cmd_action ;
          L_action_map.m_type = E_ACTION_SCEN_ADD_IN_CALL_MAP ;
          L_action_map.m_id = L_channel_id ;
          L_action_map_table[0] = L_CommandActionFactory.create(L_action_map) ;
          
          P_scen -> define_post_actions(1, (C_CommandAction**)L_action_map_table);
          L_map_inserted = true ;
        }
      }

    }
    break ;

    default:
      break ;

    }
  }

  GEN_DEBUG(1, "C_ScenarioControl::add_command() end");
  if (m_correlation_section == false) {
    if (L_map_inserted == true) { P_map_inserted[L_channel_id] = true ; }
  }

  return (L_ret);
  
} // add_command

int C_ScenarioControl::add_actions (C_XmlData                *P_msgData,
				    T_InstanceDataList&       P_instance_list,
				    C_ProtocolFrame          *P_protocol,
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
				    ) {

  int                       L_ret                   = 0      ;
  int                       L_i                              ;
  T_pXmlData_List           L_actionList                     ;  
  T_XmlData_List::iterator  L_actionListIt                   ;
  char                     *L_actionName                     ;
  char                     *L_actionArg, *L_actionArg2= NULL ;
  C_XmlData                *L_action                         ;
  bool                      L_actionFound                    ;
  T_action_type             L_actionType  = E_NB_ACTION_SCEN ;
  T_pCmdAction              L_actionData             = NULL  ;
  bool                      L_memoryRefFound         = false ;

  bool                      L_entityFieldFound       = false ;
  bool                      L_nameFieldFound         = false ;
  T_CmdAction               L_select_line_action             ;
  int                       L_field_id                       ;
  char                     *L_end_str                        ;
  bool                      L_begin_present          = false ;
  int                       L_position                       ;

  T_pXmlData_List           L_reg_action_list        = NULL  ;  
  T_XmlData_List::iterator  L_reg_action_it                  ;
  C_XmlData                *L_regexp                 = NULL  ;
  T_pActionRegExpStr        L_regexp_data            = NULL  ;
  C_RegExp                 *L_cRegExp                = NULL  ;
  int                       L_error_comp             = 0     ;
  int                       L_value_set_bit                  ;

  int                       L_len_format             = 0     ;
  bool                      L_external_method_exist  = false ;


  GEN_DEBUG(1, "C_ScenarioControl::add_actions() start");

  L_actionList = P_msgData -> get_sub_data() ;
  
  if (L_actionList != NULL) {
    for (L_actionListIt  = L_actionList->begin();
	 L_actionListIt != L_actionList->end();
	 L_actionListIt++) {
      L_action = *L_actionListIt ;

      L_actionName = L_action -> get_name() ;
      L_actionFound = false ;
      L_begin_present=false ;
      for(L_i=0;
	  L_i< (T_action_type)E_NB_ACTION_SCEN ;
	  L_i++) {
	
	if (strcmp(L_actionName, action_name_table[L_i])==0) {
	  L_actionType=(T_action_type)L_i ;
	  L_actionFound = true ;
	  break ;
	}
      }
      if (L_actionFound == true) {

 	ALLOC_VAR(L_actionData, T_pCmdAction, sizeof(T_CmdAction));
        *L_actionData = no_cmd_action ;
	L_actionData -> m_type            = L_actionType ;
	L_actionData -> m_instance_id     = 0            ;
                
	switch (L_actionType) {
	  
	case E_ACTION_SCEN_OPEN :
	  L_actionArg = L_action -> find_value((char*) "args");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "args value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }

          if (strlen(L_actionArg) > 0 ) {
            L_actionData -> m_args = L_actionArg ;
          }

	  (*P_nbOpen) ++ ;
	  break ;

        case E_ACTION_SCEN_SET_NEW_SESSION_ID :
	case E_ACTION_SCEN_MEMORY_STORE :
	case E_ACTION_SCEN_MEMORY_RESTORE : 
	  L_actionArg = L_action -> find_value((char*) "name");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }


	  L_actionArg2 = L_action -> find_value((char*) "entity");
	  if (L_actionArg2 == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "entity value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }
	  L_memoryRefFound = false ;
	  
	  L_actionData->m_id 
	    = P_protocol->find_field (L_actionArg2) ;
	  if (L_actionData->m_id != -1) {
	    L_memoryRefFound = true ;
	  }

	  if (L_memoryRefFound == true) {
	    L_actionData->m_mem_id = 
	      add_memory (L_actionArg);
	  } else {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find definition for ["
		      << L_actionArg2 << "]");
	    L_ret = -1 ;
	    break;
	  }
	  
          if (P_protocol->find_present_session (P_msg_id,L_actionData->m_id) == false) {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to present definition for ["
  		      << L_actionArg2 << "] in body of message");
  	    L_ret = -1 ;
  	    break;
          }


	  L_actionArg = L_action -> find_value((char*) "instance");
	  if (L_actionArg != NULL) {
  	    T_InstanceDataList::iterator L_instance_it ;
  	    bool L_instance_found = false ;
  	    for (L_instance_it = P_instance_list.begin();
  		 L_instance_it != P_instance_list.end();
  		 L_instance_it++) {
  	      if (strcmp(L_actionArg, L_instance_it->m_instance_name)==0) {
  		L_actionData->m_instance_id = L_instance_it->m_instance_id;
  		L_instance_found = true ;
		break ;
  	      }
  	    }
  	    if (L_instance_found == false) {
  	      GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find instance name ["
  			<< L_actionArg << "]");
  	      L_ret = -1 ;
	      break ;
  	    } else {
	      if (L_instance_it->m_id != L_actionData->m_id) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "Bad instance name ["
			  << L_actionArg << "]");
		L_ret = -1 ;
		break ;
	      }
	    }
	  }
	  
	  L_actionArg = L_action -> find_value((char*) "sub-entity");
	  if (L_actionArg != NULL) {
	    
	    L_actionData->m_sub_id 
	      = P_protocol->find_field (L_actionArg) ;
	    if (L_actionData->m_sub_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"No definition found for ["
			<< L_actionArg << "]");
	      L_ret = -1 ;
	      break ;
	    }
	    // std::cerr << "***** " <<L_actionData->m_sub_id << std::endl;  
	    // id correct => > header+body_value
	    //		  L_actionData->m_sub_id = ;
	  } // else id >header+body_value => error

	  if (P_protocol->check_sub_entity_needed(L_actionData->m_id) == true) {

	    if (L_actionData->m_sub_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"sub entity needed for  ["
			<< L_actionArg2 
			<< "]");
	      L_ret = -1 ;
	      break ;
	    }
	  }

	  
	  // TO DO 
	  L_actionArg = L_action -> find_value((char*) "begin");
	  if (L_actionArg != NULL) {

	    // check type field == STRING || BINARY
	    if (P_protocol->get_field_type(L_actionData->m_id, 
					   L_actionData->m_sub_id)
		!= E_TYPE_STRING ) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"Type field of entity ["
			<< L_actionArg2 
			<< "] is not STRING");
	      L_ret = -1 ;
	      break ;
	    }

	    unsigned long  L_begin ;
	    char          *L_end_str = NULL ;
	    L_begin = strtoul_f (L_actionArg, &L_end_str, 10);
	    if (L_end_str[0] != '\0') {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
			<< L_actionArg << "] not a number");
	      L_ret = -1 ;
	      break ;
	    } else {
	      L_actionData->m_begin = (int)L_begin ;
	      L_begin_present = true ;
	    }
	  }
	  
	  L_actionArg = L_action -> find_value((char*) "end");
	  if (L_actionArg != NULL) {

	    if (L_begin_present == true) {
	      unsigned long  L_end ;
	      char          *L_end_str = NULL ;
	      L_end = strtoul_f (L_actionArg, &L_end_str, 10);
	      if (L_end_str[0] != '\0') {
		GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
			  << L_actionArg << "] not a number");
		L_ret = -1 ;
	      } else {
		if ((int)L_end > L_actionData->m_begin) {
		  L_actionData->m_size = L_end - L_actionData->m_begin ;
		} else {
		  GEN_ERROR(E_GEN_FATAL_ERROR, "End before begin index");
		  L_ret = -1 ;
		}
	      }
	      
	    } else {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "End index defined without begin index");
	      L_ret = -1 ;
	    }
          }
          
          // analyze of regexp
          L_reg_action_list = L_action->get_sub_data() ;
          for(L_reg_action_it  = L_reg_action_list->begin() ;
              L_reg_action_it != L_reg_action_list->end() ;
              L_reg_action_it++) {

            L_regexp = *L_reg_action_it ;
            if (strcmp(L_regexp->get_name(), (char*)"regexp") == 0) {
  
              NEW_VAR(L_regexp_data, T_ActionRegExpStr());  

              L_regexp_data->m_expr = L_regexp->find_value((char*)"expr") ;

              if (L_regexp_data->m_expr == NULL ) {
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "no expression defined for this regexp");
                L_ret = -1;
                DELETE_VAR(L_regexp_data);
              } 
              
              if (L_ret != -1) {
                // default values ?
                L_regexp_data->m_nb_match = (L_regexp->find_value((char*)"nbexpr") == NULL) ?
                  1 : atoi(L_regexp->find_value((char*)"nbexpr")) ;
                L_regexp_data->m_sub_match = (L_regexp->find_value((char*)"subexpr") == NULL) ? 
                  0 : atoi(L_regexp->find_value((char*)"subexpr")) ;
                L_regexp_data->m_line = (L_regexp->find_value((char*)"line") == NULL) ?
                  -1 : atoi(L_regexp->find_value((char*)"line")) ;
              

                // std::cerr << " L_regexp_data->m_expr " << L_regexp_data->m_expr
                //          << " L_regexp_data->m_nb_match " << L_regexp_data->m_nb_match
                //          << " L_regexp_data->m_sub_match " << L_regexp_data->m_sub_match
                //          << " L_regexp_data->m_line " << L_regexp_data->m_line 
                //          << std::endl;
              
                NEW_VAR(L_cRegExp, C_RegExp(L_regexp_data->m_expr, 
                                            &L_error_comp,
                                            L_regexp_data->m_nb_match,
                                            L_regexp_data->m_sub_match,
                                            L_regexp_data->m_line));
              
                if (L_error_comp == 0) {
                  L_actionData->m_regexp_data = L_cRegExp ;
                } else {
                  GEN_ERROR(E_GEN_FATAL_ERROR, 
                            "Bad regular expression ["
                            << L_regexp_data->m_expr 
                            << "] for action ["
                            << L_actionName 
                            << "]");
                  L_ret = -1 ;
                }
              } // if (L_ret != -1)
              break;
            } // if (strcmp(L_regexp->get_name(), (char*)"regexp") == 0)
          }
	
	  break ;
	
	case E_ACTION_SCEN_START_TIMER :
	  break ;

	case E_ACTION_SCEN_STOP_TIMER :
	  (*P_data_mesure) = true ;
	  break ;

	case E_ACTION_SCEN_CLOSE :
	  break ;

	case E_ACTION_SCEN_SET_VALUE :
	  
	  L_actionArg = L_action -> find_value((char*) "name");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
	  
	  L_actionArg2 = L_action -> find_value((char*) "format");
	  if (L_actionArg2 == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "format value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } else {
            L_len_format = strlen(L_actionArg2) ;
          }
	  
	  L_actionData->m_id 
	    = P_protocol->find_field (L_actionArg) ;
	  if (L_actionData->m_id == -1) {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		      "No definition found for ["
		      << L_actionArg << "]");
	    L_ret = -1 ;
	    break ;
	  }

	  if (L_ret != -1) {
            L_actionArg = L_action -> find_value((char*)"method");
            if (L_actionArg != NULL) {
              L_external_method_exist = true ;
              L_actionData->m_external_method = 
                P_protocol->find_method_extern(L_actionArg);
              if (L_actionData->m_external_method == NULL) {
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "No method extern found for ["
                          << L_actionArg << "]");
                L_ret = -1 ;
                break ;
              } else {
                char   *L_actionArg3 = NULL ;
                L_actionData->m_type =  E_ACTION_SCEN_SET_VALUE_METHOD_EXTERN ;
  		L_actionArg3 = L_action -> find_value((char*)"message_part");
  		if (L_actionArg3 != NULL) {

  		  if (strcmp (L_actionArg3, (char*)"nothing") == 0) {
  		    L_actionData->m_msg_part_type =  E_NOTHING_TYPE ;
  		  } else if (strcmp (L_actionArg3, (char*)"header") == 0) {
  		    L_actionData->m_msg_part_type =  E_HEADER_TYPE ;
  		  } else if (strcmp (L_actionArg3, (char*)"body") == 0) {
  		    L_actionData->m_msg_part_type =  E_BODY_TYPE ;
  		  } else if (strcmp (L_actionArg3, (char*)"all") == 0) {
  		    L_actionData->m_msg_part_type =  E_ALL_TYPE ;
  		  } else {
  		    GEN_ERROR(E_GEN_FATAL_ERROR,
  			      "bad value for ["
  			      << L_actionArg3 << "] (nothing, header, body or all)");
  		    L_ret = -1 ;
  		    break ;
  		  }
  		}
              }
            } // if (L_actionArg != NULL)
          } // if (L_ret != -1)

          if (L_len_format > 0) {
            L_ret = add_expression(L_actionArg2, P_protocol,
                                   &L_actionData->m_string_expr) ;
            
            
            if (L_ret == -1) {
              GEN_ERROR(E_GEN_FATAL_ERROR,
                        "Format error for ["
                        << L_actionArg2 << "]");
              break ;
            }
          } else {
            if (L_external_method_exist == false) {
              GEN_ERROR(E_GEN_FATAL_ERROR,
                        "Format empty for ["
                        << L_actionName << "] action");
              L_ret = -1 ;
              break ;
            }
          }

	  if (L_ret != -1) {
            L_actionArg2 = L_action -> find_value((char*) "fill-pattern");
            if (L_actionArg2 != NULL) {
              if ((strlen(L_actionArg2)>2) 
                  && (L_actionArg2[0] == '0') 
                  && (L_actionArg2[1] == 'x')) { // hexa buffer value
                
                char  *L_ptr = L_actionArg2 + 2 ;
                size_t L_res_size ;
                
                L_actionData -> m_pattern 
                  = convert_hexa_char_to_bin(L_ptr, &L_res_size);
                
                if (L_actionData -> m_pattern  == NULL ) {
                  GEN_ERROR(E_GEN_FATAL_ERROR, 
                            "Bad buffer size for hexadecimal buffer ["
                            << L_actionArg2 << "]" ); 
                  L_ret = -1 ;
                  break;
                } else {
                  L_actionData -> m_pattern_size = L_res_size ;	  
                }
                
              } else { // direct string value
                L_actionData -> m_pattern_size = strlen(L_actionArg2) ;
                L_actionData -> m_pattern = (unsigned char*)L_actionArg2 ;                       
              }
            } 
          }

	  if (L_ret != -1) {
	    unsigned long  L_size ;
	    char          *L_end_str = NULL ;
            L_actionArg2 = L_action -> find_value((char*) "fill-size");
            if (L_actionArg2 != NULL) {
              L_size = strtoul_f (L_actionArg2, &L_end_str, 10);
              if (L_end_str[0] != '\0') {
                GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                          << L_actionArg2 << "] not a number");
                L_ret = -1 ;
                break ;
              } else {
                L_actionData->m_size = (int)L_size ;
              }
            } 
          }
          
          if (L_ret != -1) {
            if (L_len_format > 0) {
              L_ret = check_expression (L_actionData, P_protocol) ;
              if (L_ret == -1) {
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "Expression error for ["
                          << L_actionArg2 << "]");
                break ;
              }
            }
          }
	  break ;
	  
	case E_ACTION_SCEN_INC_COUNTER:
	  L_actionArg = L_action -> find_value((char*) "name");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } else {
	    L_actionData -> m_id 
	      = get_counter_id (L_actionArg);
	    if (L_actionData->m_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"Counter ["
			<< L_actionArg << "] not found");
	      L_ret = -1 ;
	      break ;
	    }
	  }
	  break ;
	  
	case E_ACTION_SCEN_CHECK_PRESENCE:
	  L_actionArg = L_action -> find_value((char*) "name");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
	  L_actionArg2 = L_action -> find_value((char*) "behaviour");
	  if (L_actionArg2 == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "behaviour value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
	  
	  L_actionData->m_id 
	    = P_protocol->find_field (L_actionArg) ;
	  if (L_actionData->m_id == -1) {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		      "No definition found for ["
		      << L_actionArg << "]");
	    L_ret = -1 ;
	    break ;
	  }

	  L_actionArg = L_action -> find_value((char*) "instance");
	  if (L_actionArg != NULL) {
  	    T_InstanceDataList::iterator L_instance_it ;
  	    bool L_instance_found = false ;
  	    for (L_instance_it = P_instance_list.begin();
  		 L_instance_it != P_instance_list.end();
  		 L_instance_it++) {
  	      if (strcmp(L_actionArg, L_instance_it->m_instance_name)==0) {
  		L_actionData->m_instance_id = L_instance_it->m_instance_id;
  		L_instance_found = true ;
		break ;
  	      }
  	    }
  	    if (L_instance_found == false) {
  	      GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find instance name ["
  			<< L_actionArg << "]");
  	      L_ret = -1 ;
	      break ;
  	    } else {
	      if (L_instance_it->m_id != L_actionData->m_id) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "Bad instance name ["
			  << L_actionArg << "]");
		L_ret = -1 ;
		break ;
	      }
	    }
	  }


	  // decode behaviour
	  if (strcmp (L_actionArg2, (char*)"warning") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_WARNING ;
	  } else if (strcmp (L_actionArg2, (char*)"error") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_ERROR ;
	  } else if (strcmp (L_actionArg2, (char*)"abort") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_ABORT ;
             m_action_check_abort = true ;
	  } else {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		      "bad value for ["
		      << L_actionArg << "] (warning or error)");
	    L_ret = -1 ;
	    break ;
	  }
	  break ;
	  
	case E_ACTION_SCEN_GET_EXTERNAL_DATA:
	  
	  if (m_external_data != NULL) {
	    L_actionArg = L_action -> find_value((char*) "field");
	    if (L_actionArg == NULL) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "field value mandatory for action ["
			<< L_actionName << "]");
	      L_ret = -1 ;
	      break ; 
	    }
	    L_field_id = (int)strtoul_f (L_actionArg, &L_end_str, 10);
	    if (L_end_str[0] != '\0') {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
			<< L_actionArg << "] not a number");
	      L_ret = -1 ;
	      break;
	    }
	    
	    // ctrl number field
	    if (L_field_id >= (int)(m_external_data -> get_nb_field())) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "bad value ["
			<< L_actionArg << "] greater or equal than ["
			<< m_external_data -> get_nb_field() 
			<< "] for max field number");
	      L_ret = -1 ;
	      break;
	    }

	    L_actionArg = L_action -> find_value((char*)"name");
	    if (L_actionArg != NULL) {
              L_actionData -> m_type = E_ACTION_SCEN_GET_EXTERNAL_DATA_TO_MEM  ;
              L_actionData->m_mem_id = 
                add_memory (L_actionArg);
              L_nameFieldFound = true ;
	    } else {    
              L_actionArg2 = L_action -> find_value((char*) "entity");
              if (L_actionArg2 == NULL) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "entity value mandatory for action ["
                          << L_actionName << "]");
                L_ret = -1 ;
                break ; 
              }
              
              L_entityFieldFound = false ;
              L_actionData->m_id 
                = P_protocol->find_field (L_actionArg2) ;
              if (L_actionData->m_id == -1) {
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "Unable to find definition for ["
                          << L_actionArg2 << "]");
                L_ret = -1 ;
                break ;
              } else {
                L_entityFieldFound = true ;
              }
              
              L_actionArg = L_action -> find_value((char*) "instance");
              if (L_actionArg != NULL) {
                T_InstanceDataList::iterator L_instance_it ;
                bool L_instance_found = false ;
                for (L_instance_it = P_instance_list.begin();
                     L_instance_it != P_instance_list.end();
                     L_instance_it++) {
                  if (strcmp(L_actionArg, L_instance_it->m_instance_name)==0) {
                    L_actionData->m_instance_id = L_instance_it->m_instance_id;
                    L_instance_found = true ;
                    break ;
                  }
                }
                if (L_instance_found == false) {
                  GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find instance name ["
                            << L_actionArg << "]");
                  L_ret = -1 ;
                  break ;
                } else {
                  if (L_instance_it->m_id != L_actionData->m_id) {
                    GEN_ERROR(E_GEN_FATAL_ERROR, "Bad instance name ["
                              << L_actionArg << "]");
                    L_ret = -1 ;
                    break ;
                  }
                }
              } // ctrl instance
              
              L_actionArg = L_action -> find_value((char*) "sub-entity");
              if (L_actionArg != NULL) {	
                
                
                L_actionData->m_sub_id 
                  = P_protocol->find_field (L_actionArg) ;
                if (L_actionData->m_sub_id == -1) {
                  GEN_ERROR(E_GEN_FATAL_ERROR,
                            "No definition found for ["
                            << L_actionArg << "]");
                  L_ret = -1 ;
                  break ;
                }
              } 
              
              if (P_protocol->check_sub_entity_needed(L_actionData->m_id) == true) {
                if (L_actionData->m_sub_id == -1) {
                  GEN_ERROR(E_GEN_FATAL_ERROR,
                            "sub entity needed for  ["
                            << L_actionArg2 
                            << "]");
                  
                  L_ret = -1 ;
                  break ;
                }
              }
              
              L_actionArg = L_action -> find_value((char*) "begin");
              if (L_actionArg != NULL) {
                
                // check type field == STRING || BINARY	      
                
                if (P_protocol->get_field_type(L_actionData->m_id, 
                                               L_actionData->m_sub_id)
                    != E_TYPE_STRING ) {
                  GEN_ERROR(E_GEN_FATAL_ERROR,
                            "Type field of entity ["
                            << L_actionArg2 
                            << "] is not STRING");
                  L_ret = -1 ;
                  break ;
                }
                
                unsigned long  L_begin ;
                char          *L_end_str = NULL ;
                L_begin = strtoul_f (L_actionArg, &L_end_str, 10);
                if (L_end_str[0] != '\0') {
                  GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                            << L_actionArg << "] not a number");
                  L_ret = -1 ;
                  break ;
                } else {
                  L_actionData->m_begin = (int)L_begin ;
                  L_begin_present = true ;
                }
              }
              
              L_actionArg = L_action -> find_value((char*) "end");
              if (L_actionArg != NULL) {
                
                if (L_begin_present == true) {
                  
                  unsigned long  L_end ;
                  char          *L_end_str = NULL ;
                  L_end = strtoul_f (L_actionArg, &L_end_str, 10);
                  if (L_end_str[0] != '\0') {
                    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                              << L_actionArg << "] not a number");
                    L_ret = -1 ;
                    break ;
                  } else {
                    if ((int)L_end > L_actionData->m_begin) {
                      
                      L_actionData->m_size = L_end - L_actionData->m_begin ;
                      
                    } else {
                      GEN_ERROR(E_GEN_FATAL_ERROR, "End before begin index");
                      L_ret = -1 ;
                      break ;
                    }
                  }
                } else {
                  GEN_ERROR(E_GEN_FATAL_ERROR, "End index defined without begin index");
                  L_ret = -1 ;
                  break ;
                }
              }
              
              // ctrl type of field
              if (P_protocol->get_field_type(L_actionData->m_id, 
                                             L_actionData->m_sub_id)
                  != m_external_data->get_field_type(L_field_id)) {
                // P_sub_id
                T_TypeType L_type=P_protocol->get_field_type(L_actionData->m_id, 
                                                             L_actionData->m_sub_id);
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "Incompatible type between external data field ["
                          << m_external_data->get_field_type(L_field_id)
                          << "] and entity ["
                          << L_type
                          << "]");
                L_ret = -1 ;
                break ;
              }
            } // else
 
            if ((L_entityFieldFound == true) ||  
                (L_nameFieldFound == true)) {
              if (*P_selectLine_added == false) {
                L_select_line_action = no_cmd_action ;
                // Add a new action for select data line
                L_select_line_action.m_type = E_ACTION_SCEN_SELECT_EXTERNAL_DATA_LINE ;
                P_CommandActionLst.push_back
                  (P_CmdActionFactory.create(L_select_line_action));
                P_nb_action++ ;
                if ((m_correlation_section == true) &&
                    (P_label_command != NULL)) {
                  P_CommandActionLabelLst.push_back
                    (P_CmdActionFactory.create(L_select_line_action));
                }
                // once time
                *(P_selectLine_added) = true ;
              }
              m_external_data_used = true ;
              L_actionData->m_field_data_num = L_field_id;
            }
          } else {
            GEN_ERROR(E_GEN_FATAL_ERROR, 
                      "Using restore-from-external action with no external data defined");
            L_ret = -1 ;
          }
	  
	  break ;

	case E_ACTION_SCEN_INC_VAR:
	    
	    L_actionArg = L_action -> find_value((char*) "name");
	    if (L_actionArg == NULL) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
			<< L_actionName << "]");
	      L_ret = -1 ;
	      break ; 
	    }

	    L_actionData->m_mem_id = 
	      add_memory (L_actionArg);
	    
	    break ;
	    
	case E_ACTION_SCEN_CHECK_VALUE:
	  L_actionArg = L_action -> find_value((char*) "name");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 

	  L_actionArg2 = L_action -> find_value((char*) "behaviour");
	  if (L_actionArg2 == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "behaviour value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
	  
	  L_actionData->m_id 
	    = P_protocol->find_field (L_actionArg) ;
	  if (L_actionData->m_id == -1) {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		      "No definition found for ["
		      << L_actionArg << "]");
	    L_ret = -1 ;
	    break ;
	  }
	  
	  L_actionArg = L_action -> find_value((char*) "sub-entity");
	  if (L_actionArg != NULL) {
	    
	    L_actionData->m_sub_id 
	      = P_protocol->find_field (L_actionArg) ;
	    if (L_actionData->m_sub_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"No definition found for ["
			<< L_actionArg << "]");
	      L_ret = -1 ;
	      break ;
	    }
	    // std::cerr << "***** " <<L_actionData->m_sub_id << std::endl;  
	    // id correct => > header+body_value
	    //		  L_actionData->m_sub_id = ;
	  } // else id >header+body_value => error
	  
	  

	  L_actionArg = L_action -> find_value((char*) "instance");
	  if (L_actionArg != NULL) {
  	    T_InstanceDataList::iterator L_instance_it ;
  	    bool L_instance_found = false ;
  	    for (L_instance_it = P_instance_list.begin();
  		 L_instance_it != P_instance_list.end();
  		 L_instance_it++) {
  	      if (strcmp(L_actionArg, L_instance_it->m_instance_name)==0) {
  		L_actionData->m_instance_id = L_instance_it->m_instance_id;
  		L_instance_found = true ;
		break ;
  	      }
  	    }
  	    if (L_instance_found == false) {
  	      GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find instance name ["
  			<< L_actionArg << "]");
  	      L_ret = -1 ;
	      break ;
  	    } else {
	      if (L_instance_it->m_id != L_actionData->m_id) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "Bad instance name ["
			  << L_actionArg << "]");
		L_ret = -1 ;
		break ;
	      }
	    }
	  }

	  
	  // decode behaviour
	  if (strcmp (L_actionArg2, (char*)"warning") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_WARNING ;
	  } else if (strcmp (L_actionArg2, (char*)"error") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_ERROR ;
	  } else if (strcmp (L_actionArg2, (char*)"abort") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_ABORT ;
             m_action_check_abort = true ;
	  } else {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		      "bad value for ["
		      << L_actionArg << "] (warning or error)");
	    L_ret = -1 ;
	    break ;
	  }
	  break ;

        case E_ACTION_SCEN_TRANSPORT_OPTION : 
          L_actionArg = L_action -> find_value((char*) "channel");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }

          L_actionData->m_id = m_channel_ctrl->get_channel_id(L_actionArg);
          if (L_actionData->m_id  == ERROR_CHANNEL_UNKNOWN) {
            GEN_ERROR(E_GEN_FATAL_ERROR, 
                      "Channel [" << L_actionArg << "] unknowm for action ["
		      << L_actionName << "]");
            L_ret = -1 ;
            break ;
          }


	  L_actionArg2 = L_action -> find_value((char*) "value");
	  if (L_actionArg2 == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } else {
            if (strlen(L_actionArg2) > 0 ) {
              L_actionData -> m_args = L_actionArg2 ;
            } else {
              GEN_ERROR(E_GEN_FATAL_ERROR, "no value for action ["
                        << L_actionName << "]");
              L_ret = -1 ;
            }
          }

        break;

	case E_ACTION_SCEN_CHECK_ORDER:
	  L_actionArg = L_action -> find_value((char*) "name");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
	  L_actionArg2 = L_action -> find_value((char*) "behaviour");
	  if (L_actionArg2 == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "behaviour value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
	  
	  L_actionData->m_id 
	    = P_protocol->find_field (L_actionArg) ;
	  if (L_actionData->m_id == -1) {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		      "No definition found for ["
		      << L_actionArg << "]");
	    L_ret = -1 ;
	    break ;
	  }

	  L_actionArg = L_action -> find_value((char*) "position");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "position value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 

  	  L_position = (int)strtoul_f (L_actionArg, &L_end_str, 10);
  	  if (L_end_str[0] != '\0') {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
  		      << L_actionArg << "] not a number");
  	    L_ret = -1 ;
  	    break;
  	  }
	    
  	  // ctrl number field
  	  if (L_position > P_nb_value) {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad value ["
  		      << L_actionArg << "] greater or equal than ["
  		      << P_nb_value 
  		      << "] for max number of body value");
  	    L_ret = -1 ;
  	    break;
  	  }

	  L_actionData->m_position = L_position ;
	  

	   
	  // decode behaviour
	  if (strcmp (L_actionArg2, (char*)"warning") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_WARNING ;
	  } else if (strcmp (L_actionArg2, (char*)"error") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_ERROR ;
	  } else if (strcmp (L_actionArg2, (char*)"abort") == 0) {
	    L_actionData->m_check_behaviour = E_CHECK_BEHAVIOUR_ABORT ;
             m_action_check_abort = true ;
	  } else {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		      "bad value for ["
		      << L_actionArg << "] (warning or error)");
	    L_ret = -1 ;
	    break ;
	  }
	  break ;
          
	case E_ACTION_SCEN_SET_BIT : 
          
	  L_actionArg = L_action -> find_value((char*) "name");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "name value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }
          
	  L_actionArg2 = L_action -> find_value((char*) "entity");
	  if (L_actionArg2 == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "entity value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }
	  L_memoryRefFound = false ;
	  
	  L_actionData->m_id 
	    = P_protocol->find_field (L_actionArg2) ;
	  if (L_actionData->m_id != -1) {
	    L_memoryRefFound = true ;
	  }
          
	  if (L_memoryRefFound == true) {
	    L_actionData->m_mem_id = 
	      add_memory (L_actionArg);
	  } else {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find definition for ["
		      << L_actionArg2 << "]");
	    L_ret = -1 ;
	    break;
	  }
          
	  L_actionArg = L_action -> find_value((char*) "instance");
	  if (L_actionArg != NULL) {
  	    T_InstanceDataList::iterator L_instance_it ;
  	    bool L_instance_found = false ;
  	    for (L_instance_it = P_instance_list.begin();
  		 L_instance_it != P_instance_list.end();
  		 L_instance_it++) {
  	      if (strcmp(L_actionArg, L_instance_it->m_instance_name)==0) {
  		L_actionData->m_instance_id = L_instance_it->m_instance_id;
  		L_instance_found = true ;
		break ;
  	      }
  	    }
  	    if (L_instance_found == false) {
  	      GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find instance name ["
  			<< L_actionArg << "]");
  	      L_ret = -1 ;
	      break ;
  	    } else {
	      if (L_instance_it->m_id != L_actionData->m_id) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "Bad instance name ["
			  << L_actionArg << "]");
		L_ret = -1 ;
		break ;
	      }
	    }
	  }
          
	  L_actionArg = L_action -> find_value((char*) "sub-entity");
	  if (L_actionArg != NULL) {	    
	    L_actionData->m_sub_id 
	      = P_protocol->find_field (L_actionArg) ;
	    if (L_actionData->m_sub_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"No definition found for ["
			<< L_actionArg << "]");
	      L_ret = -1 ;
	      break ;
	    }
	  }
          
          
	  if (P_protocol->check_sub_entity_needed(L_actionData->m_id) == true) {
            
	    if (L_actionData->m_sub_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"sub entity needed for  ["
			<< L_actionArg2 
			<< "]");
	      L_ret = -1 ;
	      break ;
	    }
	  }
          
          // check type field == STRING || BINARY
//  	  if (P_protocol->get_field_type(L_actionData->m_id, 
//                                           L_actionData->m_sub_id)
//                != E_TYPE_STRING ) {
//              GEN_ERROR(E_GEN_FATAL_ERROR,
//                        "Type field of entity ["
//                        << L_actionArg2 
//                        << "] is not STRING");
//              L_ret = -1 ;
//              break ;
//  	  }
          
	  L_actionArg = L_action -> find_value((char*) "position");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "position value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
          
  	  L_position = (int)strtoul_f (L_actionArg, &L_end_str, 10);
  	  if (L_end_str[0] != '\0') {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
  		      << L_actionArg << "] not a number");
  	    L_ret = -1 ;
  	    break;
  	  }
          
	  L_actionData->m_position = L_position ;
          
	  L_actionArg = L_action -> find_value((char*) "value");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
          
  	  L_value_set_bit = (int)strtoul_f (L_actionArg, &L_end_str, 10);
  	  if (L_end_str[0] != '\0') {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
  		      << L_actionArg << "] not a number");
  	    L_ret = -1 ;
  	    break;
  	  }
          
          if ((L_value_set_bit != 0) && (L_value_set_bit !=1)) {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
  		      << L_actionArg << "] value must be 0 or 1");
  	    L_ret = -1 ;
  	    break;
  	  }
          
          L_actionData->m_field_data_num = L_value_set_bit;
          break;

	case E_ACTION_SCEN_SET_VALUE_BIT : 
          
	  L_actionArg = L_action -> find_value((char*) "entity");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "entity value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }
	  
	  L_actionData->m_id 
	    = P_protocol->find_field (L_actionArg) ;
          
	  L_actionArg = L_action -> find_value((char*) "instance");
	  if (L_actionArg != NULL) {
  	    T_InstanceDataList::iterator L_instance_it ;
  	    bool L_instance_found = false ;
  	    for (L_instance_it = P_instance_list.begin();
  		 L_instance_it != P_instance_list.end();
  		 L_instance_it++) {
  	      if (strcmp(L_actionArg, L_instance_it->m_instance_name)==0) {
  		L_actionData->m_instance_id = L_instance_it->m_instance_id;
  		L_instance_found = true ;
		break ;
  	      }
  	    }
  	    if (L_instance_found == false) {
  	      GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find instance name ["
  			<< L_actionArg << "]");
  	      L_ret = -1 ;
	      break ;
  	    } else {
	      if (L_instance_it->m_id != L_actionData->m_id) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "Bad instance name ["
			  << L_actionArg << "]");
		L_ret = -1 ;
		break ;
	      }
	    }
	  }
          
	  L_actionArg = L_action -> find_value((char*) "sub-entity");
	  if (L_actionArg != NULL) {	    
	    L_actionData->m_sub_id 
	      = P_protocol->find_field (L_actionArg) ;
	    if (L_actionData->m_sub_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"No definition found for ["
			<< L_actionArg << "]");
	      L_ret = -1 ;
	      break ;
	    }
	  }
          
          
	  if (P_protocol->check_sub_entity_needed(L_actionData->m_id) == true) {
            
	    if (L_actionData->m_sub_id == -1) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"sub entity needed for  ["
			<< L_actionArg2 
			<< "]");
	      L_ret = -1 ;
	      break ;
	    }
	  }
          
          // check type field == STRING || BINARY
//  	  if (P_protocol->get_field_type(L_actionData->m_id, 
//                                           L_actionData->m_sub_id)
//                != E_TYPE_STRING ) {
//              GEN_ERROR(E_GEN_FATAL_ERROR,
//                        "Type field of entity ["
//                        << L_actionArg2 
//                        << "] is not STRING");
//              L_ret = -1 ;
//              break ;
//  	  }
          
          
	  L_actionArg = L_action -> find_value((char*) "position");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "position value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
          
  	  L_position = (int)strtoul_f (L_actionArg, &L_end_str, 10);
  	  if (L_end_str[0] != '\0') {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
  		      << L_actionArg << "] not a number");
  	    L_ret = -1 ;
  	    break;
  	  }
          
	  L_actionData->m_position = L_position ;
          
	  L_actionArg = L_action -> find_value((char*) "value");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "value mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  } 
          
  	  L_value_set_bit = (int)strtoul_f (L_actionArg, &L_end_str, 10);
  	  if (L_end_str[0] != '\0') {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
  		      << L_actionArg << "] not a number");
  	    L_ret = -1 ;
  	    break;
  	  }
          
          if ((L_value_set_bit != 0) && (L_value_set_bit !=1)) {
  	    GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
  		      << L_actionArg << "] value must be 0 or 1");
  	    L_ret = -1 ;
  	    break;
  	  }
          
          L_actionData->m_field_data_num = L_value_set_bit;
          break;

	case E_ACTION_SCEN_INSERT_IN_MAP : 

          L_actionArg = L_action -> find_value((char*) "channel");
	  if (L_actionArg == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "Channel section is mandatory for action ["
		      << L_actionName << "]");
	    L_ret = -1 ;
	    break ; 
	  }
          
          L_actionData->m_position = m_channel_ctrl->get_channel_id(L_actionArg);
          if (L_actionData->m_position  == ERROR_CHANNEL_UNKNOWN) {
            GEN_ERROR(E_GEN_FATAL_ERROR, 
                      "Channel [" << L_actionArg << "] unknowm for action ["
		      << L_actionName << "]");
            L_ret = -1 ;
            break ;
          }
          
          L_actionArg = L_action -> find_value((char*)"name");
          if (L_actionArg != NULL) {
            L_actionData->m_mem_id = check_memory (L_actionArg) ;
            if (L_actionData->m_mem_id == -1) {
              GEN_ERROR(E_GEN_FATAL_ERROR,
                        "Unable to find definition for ["
                        << L_actionArg << "]");
              L_ret = -1 ;
              break ;
            }
            L_actionData -> m_type = E_ACTION_SCEN_INSERT_IN_MAP_FROM_MEM  ;
          } else {    
            L_actionArg2 = L_action -> find_value((char*) "entity");
            if (L_actionArg2 == NULL) {
              GEN_ERROR(E_GEN_FATAL_ERROR, "entity value mandatory for action ["
                        << L_actionName << "]");
              L_ret = -1 ;
              break ; 
            }
            
            L_actionData->m_id 
              = P_protocol->find_field (L_actionArg2) ;
            if (L_actionData->m_id == -1) {
              GEN_ERROR(E_GEN_FATAL_ERROR,
                        "Unable to find definition for ["
                        << L_actionArg2 << "]");
              L_ret = -1 ;
              break ;
            } 
            
            L_actionArg = L_action -> find_value((char*) "instance");
            if (L_actionArg != NULL) {
              T_InstanceDataList::iterator L_instance_it ;
              bool L_instance_found = false ;
              for (L_instance_it = P_instance_list.begin();
                   L_instance_it != P_instance_list.end();
                   L_instance_it++) {
                if (strcmp(L_actionArg, L_instance_it->m_instance_name)==0) {
                  L_actionData->m_instance_id = L_instance_it->m_instance_id;
                  L_instance_found = true ;
                  break ;
                }
              }
              if (L_instance_found == false) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find instance name ["
                          << L_actionArg << "]");
                L_ret = -1 ;
                break ;
              } else {
                if (L_instance_it->m_id != L_actionData->m_id) {
                  GEN_ERROR(E_GEN_FATAL_ERROR, "Bad instance name ["
                            << L_actionArg << "]");
                  L_ret = -1 ;
                  break ;
                }
              }
            } // ctrl instance
            
            L_actionArg = L_action -> find_value((char*) "sub-entity");
            if (L_actionArg != NULL) {	
              
              
              L_actionData->m_sub_id 
                = P_protocol->find_field (L_actionArg) ;
              if (L_actionData->m_sub_id == -1) {
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "No definition found for ["
                          << L_actionArg << "]");
                L_ret = -1 ;
                break ;
              }
            } 
            
            if (P_protocol->check_sub_entity_needed(L_actionData->m_id) == true) {
              if (L_actionData->m_sub_id == -1) {
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "sub entity needed for  ["
                          << L_actionArg2 
                          << "]");
                
                L_ret = -1 ;
                break ;
              }
            }
            
            L_actionArg = L_action -> find_value((char*) "begin");
            if (L_actionArg != NULL) {
              
              // check type field == STRING || BINARY	      
              
              if (P_protocol->get_field_type(L_actionData->m_id, 
                                             L_actionData->m_sub_id)
                  != E_TYPE_STRING ) {
                GEN_ERROR(E_GEN_FATAL_ERROR,
                          "Type field of entity ["
                          << L_actionArg2 
                          << "] is not STRING");
                L_ret = -1 ;
                break ;
              }
              
              unsigned long  L_begin ;
              char          *L_end_str = NULL ;
              L_begin = strtoul_f (L_actionArg, &L_end_str, 10);
              if (L_end_str[0] != '\0') {
                GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                          << L_actionArg << "] not a number");
                L_ret = -1 ;
                break ;
              } else {
                L_actionData->m_begin = (int)L_begin ;
                L_begin_present = true ;
              }
            }
            
            L_actionArg = L_action -> find_value((char*) "end");
            if (L_actionArg != NULL) {
              
              if (L_begin_present == true) {
                
                unsigned long  L_end ;
                char          *L_end_str = NULL ;
                L_end = strtoul_f (L_actionArg, &L_end_str, 10);
                if (L_end_str[0] != '\0') {
                  GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                            << L_actionArg << "] not a number");
                  L_ret = -1 ;
                  break ;
                } else {
                  if ((int)L_end > L_actionData->m_begin) {
                    
                    L_actionData->m_size = L_end - L_actionData->m_begin ;
                    
                  } else {
                    GEN_ERROR(E_GEN_FATAL_ERROR, "End before begin index");
                    L_ret = -1 ;
                    break ;
                  }
                }
              } else {
                GEN_ERROR(E_GEN_FATAL_ERROR, "End index defined without begin index");
                L_ret = -1 ;
                break ;
              }
            }
            
          } // else
          break ;
          
	default:
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Action not implemented");
	  L_ret = -1 ;
	  break ;
	} // switch L_actionType
	
	if (L_ret != -1) {
          if ( m_config->get_execute_check_action() == false) {
            switch (L_actionType) {
            case E_ACTION_SCEN_CHECK_PRESENCE:
            case E_ACTION_SCEN_CHECK_VALUE:
            case E_ACTION_SCEN_CHECK_ORDER:
              FREE_VAR(L_actionData);
              break ;
            default:
              P_CommandActionLst.push_back(P_CmdActionFactory.create(*L_actionData));
              if ((m_correlation_section == true) &&
                  (P_label_command != NULL)) {
                P_CommandActionLabelLst.push_back
                  (P_CmdActionFactory.create(*L_actionData));
              }
              FREE_VAR(L_actionData);
              P_nb_action++ ;
              break ;
            }
          } else {
            P_CommandActionLst.push_back(P_CmdActionFactory.create(*L_actionData));
            if ((m_correlation_section == true) &&
                (P_label_command != NULL)) {
              P_CommandActionLabelLst.push_back
                (P_CmdActionFactory.create(*L_actionData));
            }
            FREE_VAR(L_actionData);
            P_nb_action++ ;
          }
	} 
	
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown action name [" << L_actionName << "]");
	L_ret = -1 ;
	break ;
      }
      if (L_ret == -1) break ;

    } // for (L_actionListIt
    
    if (L_ret != -1) {
      if ((P_pre_action == true) && (P_pre_action_done == false)) {
  	// pre actions settings
  	if ((P_nb_action != 0) &&
            (P_insert_correlation == false)) {
	  T_CommandActionLst::iterator L_cmdActionIt ;
	  int                       L_actionIdx = 0 ;
	  
	  ALLOC_TABLE(P_CommandActionTable, 
                      C_CommandAction**,
                      sizeof(C_CommandAction*),
                      P_nb_action);
	  for (L_cmdActionIt  = P_CommandActionLst.begin();
	       L_cmdActionIt != P_CommandActionLst.end();
	       L_cmdActionIt++) {
            P_CommandActionTable[L_actionIdx] = *L_cmdActionIt ;
	    L_actionIdx ++ ;
	  }
	  if (!P_CommandActionLst.empty()) {
	    P_CommandActionLst.erase(P_CommandActionLst.begin(),P_CommandActionLst.end());
	  }

          P_scen -> define_pre_actions(P_nb_action, P_CommandActionTable);
	  P_nb_action = 0 ;
	  P_CommandActionTable = NULL ;
          
  	}
  	P_pre_action_done = true ;	
	
      } else {	
  	// post actions settings
  	// post actions in the command => add in call map added if needed 
  	// (== if not already inserted for this channel)
  	P_inserted = P_map_inserted[P_channel_id];
  	if (P_inserted == false) {
          if  (m_correlation_section == false) {
            T_CmdAction L_action_map ;
            L_action_map = no_cmd_action ;
            L_action_map . m_type = E_ACTION_SCEN_ADD_IN_CALL_MAP ;
            L_action_map . m_id = P_channel_id ;
            P_CommandActionLst.push_back(P_CmdActionFactory.create(L_action_map));
            if ((m_correlation_section == true) &&
                (P_label_command != NULL)) {
              P_CommandActionLabelLst.push_back
                (P_CmdActionFactory.create(L_action_map));
            }

            P_nb_action++ ;
            P_inserted = true ;
          }


          for (L_actionListIt  = L_actionList->begin();
               L_actionListIt != L_actionList->end();
               L_actionListIt++) {
            L_action = *L_actionListIt ;
            L_actionName = L_action -> get_name() ;
            L_actionFound = false ;
            for(L_i=0;
                L_i< (T_action_type)E_NB_ACTION_SCEN ;
                L_i++) {
              
              if (strcmp(L_actionName, action_name_table[L_i])==0) {
                L_actionType=(T_action_type)L_i ;
                L_actionFound = true ;
                break ;
              }
            }
            if (L_actionFound == true) {
              if (L_actionType == E_ACTION_SCEN_SET_NEW_SESSION_ID) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "Action ["
                          << L_actionName << "] is not allowed on the first command");
                L_ret = -1 ;
                break ; 
              }
            }
          }
          if (L_ret == -1) {
            return (L_ret);
          }
        }

	
  	if ((P_nb_action != 0) && 
            (P_insert_correlation == false)) {
  	  T_CommandActionLst::iterator L_cmdActionIt ;
  	  int                          L_actionIdx = 0 ;
	  
	  ALLOC_TABLE(P_CommandActionTable, 
                      C_CommandAction**,
                      sizeof(C_CommandAction*),
                      P_nb_action);

  	  for (L_cmdActionIt  = P_CommandActionLst.begin();
  	       L_cmdActionIt != P_CommandActionLst.end();
  	       L_cmdActionIt++) {
  	    P_CommandActionTable[L_actionIdx] = *L_cmdActionIt ;
  	    L_actionIdx ++ ;
  	  }
  	  if (!P_CommandActionLst.empty()) {
  	    P_CommandActionLst.erase(P_CommandActionLst.begin(),P_CommandActionLst.end());
  	  }
	  
          P_scen -> define_post_actions(P_nb_action, P_CommandActionTable);
  	}
      }
    }
  }


  GEN_DEBUG(1, "C_ScenarioControl::add_actions() end with " << L_ret);
  return (L_ret);
	
} // add_actions



int C_ScenarioControl::memory_used() {
  return(m_memory_max);
}

iostream_output& operator<<(iostream_output& P_ostream , 
				C_ScenarioControl& P_scenControl) {

  P_ostream << (*P_scenControl.m_traffic_scen) ;
  return (P_ostream) ;
  
}

int C_ScenarioControl::add_expression (char       *P_arg,
				       C_ProtocolFrame *P_protocol,
				       T_pStringExpression *P_strExpr) {

  int L_ret = 0 ;

  char *L_ptr; 
  size_t L_size, L_i, L_j ;
  int    L_type;

  char   L_value[100] ;
  size_t L_value_size = 0 ;

  list_t<T_pStringValue> L_list ;
  list_t<T_pStringValue>::iterator L_it ;

  T_pStringValue L_string_val ;
  T_pStringExpression L_string_expr ;

  L_ptr = P_arg ;
  L_size = strlen(L_ptr) ;

  //  std::cerr << "size   *****" << L_size << std::endl;
  // std::cerr << "buffer *****" << L_ptr << std::endl;

  L_type = 0 ; // 0: normal string
               // 1: $ found => need ( for dynamic def
               // 2: $( found => dynamic def

  *P_strExpr = NULL ;

  L_i = 0 ;
  while (L_i < L_size) {
    switch (L_ptr[L_i]) {
    case '$':
      if (L_value_size != 0) {
	L_value[L_value_size] = 0 ;
	L_value_size++ ;
	ALLOC_VAR(L_string_val, T_pStringValue, sizeof(T_StringValue));
	L_string_val->m_type = E_STR_STATIC ;
	ALLOC_TABLE(L_string_val->m_data.m_value,
		    char*,
		    sizeof(char),
		    strlen(L_value)+1);
	strcpy(L_string_val->m_data.m_value, L_value);
	L_list.push_back(L_string_val);
	L_value_size = 0 ;
      }
      if (L_type == 0) {
	L_type = 1 ;
      }
      break ;
    case '(':
      if (L_type == 1) {
	L_type = 2 ;
      } else {
	L_value[L_value_size] = L_ptr[L_i] ;
	L_value_size++ ;
      }	
      break ;
    case ')':
      if (L_type == 2) {
	if (L_value_size != 0) {
	  L_value[L_value_size] = 0 ;
	  L_value_size++ ;
	  ALLOC_VAR(L_string_val, T_pStringValue, sizeof(T_StringValue));
	  L_string_val->m_type = E_STR_COUNTER ;
	  L_string_val->m_data.m_id 
	    = get_counter_id (L_value) ;
	  if (L_string_val->m_data.m_id == -1) {
            L_string_val->m_data.m_id 
              = check_memory (L_value) ;
            if (L_string_val->m_data.m_id == -1) {
              GEN_ERROR(E_GEN_FATAL_ERROR,
                        "Unable to find definition for ["
                        << L_value << "]");
              L_ret = -1 ;
              break ;
            } else {
              L_string_val->m_type = E_STR_MEMORY ;
              L_list.push_back(L_string_val);
              L_value_size = 0 ;

            }
	  } else {
	    L_list.push_back(L_string_val);
	    L_value_size = 0 ;
	  }
	}
	L_type = 0 ;
      } else {
	L_value[L_value_size] = L_ptr[L_i] ;
	L_value_size++ ;
      }
      break ;

    default :
      switch (L_type) {
      case 0:
	// add static value ;
	L_value[L_value_size] = L_ptr[L_i] ;
	L_value_size++ ;
        // std::cerr << "L_i " << L_i << "and L_value_size *****" << L_value_size << std::endl;
	break ;
      case 1:
	L_type = 0 ;
	break ;
      case 2:
	// add value counter
	L_value[L_value_size] = L_ptr[L_i] ;
	L_value_size++ ;
	break ;
      }
      break ;
    }
    if (L_ret == -1) break ;
    L_i ++ ;
  }

  if (L_ret == 0) {
    if (L_value_size != 0) {
      L_value[L_value_size] = 0 ;
      L_value_size++ ;
      ALLOC_VAR(L_string_val, T_pStringValue, sizeof(T_StringValue));
      L_string_val->m_type = E_STR_STATIC ;
      ALLOC_TABLE(L_string_val->m_data.m_value,
		  char*,
		  sizeof(char),
		  strlen(L_value)+1);
      strcpy(L_string_val->m_data.m_value, L_value);
      L_list.push_back(L_string_val);
      L_value_size = 0 ;
    } 

    ALLOC_VAR(L_string_expr,
	      T_pStringExpression,
	      sizeof(T_StringExpression));

    L_string_expr->m_nb_portion = L_list.size() ;


    //    std::cerr << "m_nb_portion *****" << L_string_expr->m_nb_portion << std::endl;

    if (L_string_expr->m_nb_portion != 0) {
      ALLOC_TABLE(L_string_expr->m_portions,
		  T_pStringValue,
		  sizeof(T_StringValue),
		  L_string_expr->m_nb_portion);
      L_j = 0 ;
      for (L_it  = L_list.begin();
	   L_it != L_list.end();
	   L_it++) {
	L_string_expr->m_portions[L_j] = *(*L_it);
	FREE_VAR (*L_it);
	L_j ++ ;
      }
      *P_strExpr = L_string_expr ;
      L_list.erase(L_list.begin(), L_list.end());
      
    } else {
      L_string_expr->m_portions = NULL ;
    }
    
  }

  // analysis of arg2 => format expression

  return (L_ret) ;
}

int C_ScenarioControl::check_expression (T_pCmdAction  P_action, 
					 C_ProtocolFrame   *P_protocol) {

  int L_ret = 0 ;

  regex_t    L_regExpr ;
  regmatch_t L_pmatch ;
  int        L_status ;
  char       L_buffer[1024] ;
  char      *L_search       ;

  L_ret = ((P_action->m_type != E_ACTION_SCEN_SET_VALUE) &&
           (P_action->m_type != E_ACTION_SCEN_SET_VALUE_METHOD_EXTERN))
           ? -1 : 0 ;

  if (L_ret == 0) {
    switch (P_protocol->get_field_type(P_action->m_id,0)) {

    case E_TYPE_NUMBER:
    case E_TYPE_SIGNED:
    case E_TYPE_NUMBER_64:
    case E_TYPE_SIGNED_64:
      if (P_action->m_string_expr->m_nb_portion != 1) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "Just one value allowed for this kind of field");
	L_ret = -1 ;
	break ;
      }
      if (P_action->m_string_expr->m_portions[0].m_type != E_STR_COUNTER) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "Just one counter value allowed for this kind of field");
	L_ret = -1 ;
	break ;
      }
      break ;

    case E_TYPE_STRING:
      // nothing to check: everything is allowed
      break ;

    case E_TYPE_STRUCT: {
      
      if (P_action->m_string_expr->m_nb_portion > 3 ) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "One or two values allowed for this kind of field");
	L_ret = -1 ;
	break ;
      } 
      
      if (P_action->m_string_expr->m_nb_portion ==1) {
	if ((P_action->m_string_expr->m_portions[0].m_type != E_STR_STATIC) ) { 
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "Just one string value allowed for this kind of field :" << 
		    " [0;0], [;0] or [0;]");
	  L_ret = -1 ;
	  break ;
	} else {
	  
	  L_search = P_action->m_string_expr->m_portions[0].m_data.m_value;
	  
	  L_status = regcomp (&L_regExpr, "[[:blank:]]*[[:digit:]]*[[:blank:]]*[;][[:blank:]]*[[:digit:]]*[[:blank:]]*", REG_EXTENDED) ;
	  if (L_status != 0 ) {
	    regerror(L_status, &L_regExpr, L_buffer, 1024);
	    regfree (&L_regExpr) ;
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "Just one string value allowed for this kind of field :" << 
		      " [0;0], [;0] or [0;]");
	    L_ret = -1 ;
	    break ;
	  }  
	  L_pmatch.rm_eo = 0 ;
	  L_pmatch.rm_so = 0 ;
	   
	  L_status = regexec (&L_regExpr, L_search, 1, &L_pmatch, 0) ;
	  regfree (&L_regExpr) ;
	  if ((L_status != 0 ) || (L_pmatch.rm_eo != (int )strlen(L_search)) || (L_pmatch.rm_so != 0)) { 
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "Just one string value allowed for this kind of field :" << 
		      " [0;0], [;0] or [0;]");
	    L_ret = -1 ;
	    break ;
	  }
	}
      }
      
      if (P_action->m_string_expr->m_nb_portion ==2) {
	switch (P_action->m_string_expr->m_portions[0].m_type) {
	case E_STR_STATIC:
	  if( P_action->m_string_expr->m_portions[1].m_type != E_STR_COUNTER)  {
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "One string value and one counter are allowed for this kind of field :" << 
		      "[0;counter] or [;counter]");
	    L_ret = -1 ;
	    break ;
	  } else {
	    
	    L_search = P_action->m_string_expr->m_portions[0].m_data.m_value;
	    L_status = regcomp (&L_regExpr, "[[:blank:]]*[[:digit:]]*[[:blank:]]*[;][[:blank:]]*", REG_EXTENDED) ;
	    if (L_status != 0 ) {
	      regerror(L_status, &L_regExpr, L_buffer, 1024);
	      regfree (&L_regExpr) ;
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
			"One string value and one counter are allowed for this kind of field :" << 
			"[0;counter] or [;counter]");
	      L_ret = -1 ;
	      break ;
	    }   
	    L_status = regexec (&L_regExpr, L_search, 1, &L_pmatch, 0) ;
	    regfree (&L_regExpr) ;
	    if ((L_status != 0 ) || (L_pmatch.rm_eo != (int )strlen(L_search)) || (L_pmatch.rm_so != 0)) { 
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
			"One string value and one counter are allowed for this kind of field :" << 
			"[0;counter] or [;counter]");
	      L_ret = -1 ;
	      break ;
	    }
	  }
	  
	  break ;
	case E_STR_COUNTER:
	  if( P_action->m_string_expr->m_portions[1].m_type != E_STR_STATIC ) { 
	    
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "One counter and one string value are allowed for this kind of field :" << 
		      "[counter;0] or [counter;]");
	    L_ret = -1 ;
	    break ;
	  } else {
	    
	    L_search = P_action->m_string_expr->m_portions[1].m_data.m_value;
	    L_status = regcomp (&L_regExpr, "[[:blank:]]*[;][[:blank:]]*[[:digit:]]*[[:blank:]]*", REG_EXTENDED) ;
	    if (L_status != 0 ) {
	      regerror(L_status, &L_regExpr, L_buffer, 1024);
	      regfree (&L_regExpr) ;
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
			"One counter and one string value are allowed for this kind of field :" << 
			"[counter;0] or [counter;]");
	      L_ret = -1 ;
	      break ;
	    }   
	    L_status = regexec (&L_regExpr, L_search, 1, &L_pmatch, 0) ;
	    regfree (&L_regExpr) ;
	    if ((L_status != 0 ) || (L_pmatch.rm_eo != (int )strlen(L_search)) || (L_pmatch.rm_so != 0)) { 
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
			"One counter and one string value are allowed for this kind of field :" << 
			"[counter;0] or [counter;]");
	      L_ret = -1 ;
	      break ;
	    }
	  }
	  break ;
	  
	default :
	  // nothing to do
	  break ;
	}
	
      }

      if (P_action->m_string_expr->m_nb_portion == 3) {
	if( (P_action->m_string_expr->m_portions[0].m_type != E_STR_COUNTER)  ||
	    (P_action->m_string_expr->m_portions[1].m_type != E_STR_STATIC)  ||
	    (P_action->m_string_expr->m_portions[2].m_type != E_STR_COUNTER)  ) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "One counter, one string value and one counter are allowed for this kind of field :" << 
		    "[counter;counter] ");
	  L_ret = -1 ;
	  break ;
	} else {
	  L_search = P_action->m_string_expr->m_portions[1].m_data.m_value;
	  L_status = regcomp (&L_regExpr, "[[:blank:]]*[;][[:blank:]]*", REG_EXTENDED) ;
	  if (L_status != 0 ) {
	    regerror(L_status, &L_regExpr, L_buffer, 1024);
	    regfree (&L_regExpr) ;
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "One counter, one string value and one counter are allowed for this kind of field :" << 
		      "[counter;counter] ");
	    L_ret = -1 ;
	    break ;
	  }   
	  L_status = regexec (&L_regExpr, L_search, 1, &L_pmatch, 0) ;
	  regfree (&L_regExpr) ;
	  if ((L_status != 0 ) || (L_pmatch.rm_eo != (int )strlen(L_search)) || (L_pmatch.rm_so != 0)) { 
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "One counter, one string value and one counter are allowed for this kind of field :" << 
		      "[counter;counter] ");
	    L_ret = -1 ;
	    break ;
	  }
	}
      }
    }
      
      break ;
      
      
    default:
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported type for expression");
      L_ret = -1 ;
      break ;
    }
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Action type incorrect");
  }
  

  return (L_ret) ;
  
}

T_pC_Scenario C_ScenarioControl::init_scenario_defined (T_pTrafficType P_type) {
  *P_type = m_init_type ;
  return (m_init_scen);
}


int C_ScenarioControl::check_memory (char *P_mem_name) {

  int L_mem_id = -1 ;
  T_MemoryIdMap::iterator
    L_it = m_memory_map->find (T_MemoryIdMap::key_type(P_mem_name)) ;


  if (L_it != m_memory_map->end()) {
    L_mem_id = L_it->second ;
  }

  return (L_mem_id) ;

}


int C_ScenarioControl::add_memory (char *P_mem_name) {

  int L_mem_id = check_memory (P_mem_name) ;

  if (L_mem_id == -1) {
    L_mem_id = m_memory_max ;
    m_memory_map->insert(T_MemoryIdMap::value_type(P_mem_name, L_mem_id));
    m_memory_max++;
  }

  return (L_mem_id) ;

}


void C_ScenarioControl::switch_to_init () {
  GEN_DEBUG(1, "C_ScenarioControl::switch_to_init() start");
  if (m_init_scen != NULL) {
    m_save_traffic_scen = m_traffic_scen ;
    m_traffic_scen = m_init_scen ;
  }
  GEN_DEBUG(1, "C_ScenarioControl::switch_to_init() m_trafiic_scen is " << m_traffic_scen);
  GEN_DEBUG(1, "C_ScenarioControl::switch_to_init() end");
}

void C_ScenarioControl::switch_to_traffic() {
  if (m_save_traffic_scen != NULL) {
    m_traffic_scen = m_save_traffic_scen ;
    m_save_traffic_scen = NULL ;
  }
}


void C_ScenarioControl::set_config (C_GeneratorConfig *P_config) {
  m_config = P_config ;
}

T_pWaitValuesSet C_ScenarioControl::get_wait_values() {
  return (m_wait_values);
}


T_pRetransDelayValuesSet C_ScenarioControl::get_retrans_delay_values () {
  return (m_retrans_delay_values);
}

void  C_ScenarioControl::update_retrans_delay_cmd (size_t P_nb, 
                                                   unsigned long *P_table) {

  int L_i ;
  if (m_abort_scen != NULL) {
    m_abort_scen -> update_retrans_delay_cmd (P_nb, P_table);
  }
  if (m_traffic_scen != NULL) {
    m_traffic_scen -> update_retrans_delay_cmd (P_nb, P_table);
  }
  if (m_save_traffic_scen != NULL) {
    m_save_traffic_scen -> update_retrans_delay_cmd(P_nb, P_table);
  }
  if (m_init_scen != NULL) {
    m_init_scen -> update_retrans_delay_cmd (P_nb, P_table);
  }
  if (m_nb_default != 0) {
    for(L_i=0; L_i < m_nb_default; L_i++) {
      m_default_scen[L_i] -> update_retrans_delay_cmd (P_nb, P_table);
    }
  }
}


void  C_ScenarioControl::update_wait_cmd (size_t P_nb, 
					  unsigned long *P_table) {

  int L_i ;
  if (m_abort_scen != NULL) {
    m_abort_scen -> update_wait_cmd (P_nb, P_table);
  }
  if (m_traffic_scen != NULL) {
    m_traffic_scen -> update_wait_cmd (P_nb, P_table);
  }
  if (m_save_traffic_scen != NULL) {
    m_save_traffic_scen -> update_wait_cmd (P_nb, P_table);
  }
  if (m_init_scen != NULL) {
    m_init_scen -> update_wait_cmd (P_nb, P_table);
  }
  if (m_nb_default != 0) {
    for(L_i=0; L_i < m_nb_default; L_i++) {
      m_default_scen[L_i] -> update_wait_cmd (P_nb, P_table);
    }
  }
  
}

bool C_ScenarioControl::fromXml (C_XmlData     *P_data, 
				 bool           P_check_msg,
				 bool          *P_data_mesure,
				 T_pTrafficType P_traffic_type,
                                 bool          *P_action_check_abort) {
  
  bool L_XmlOk = true ;

  C_XmlData               *L_data  = P_data ;
  T_pXmlData_List          L_subList, L_subscen ;
  T_XmlData_List::iterator L_subListIt, L_subscen_it ;
  C_XmlData               *L_scen_data ;

  C_XmlData               *L_xml_traffic_scen = NULL ;
  C_XmlData               *L_xml_init_scen    = NULL ;
  C_XmlData               *L_xml_abort_scen   = NULL ;
  list_t<C_XmlData*>       L_xml_default_scen ;
  list_t<C_XmlData*>::iterator     L_default_it ;
  C_XmlData                       *L_current_default_scen = NULL ;


  int                      L_nb_open ;
  T_TrafficType            L_trafficType, L_initTrafficType ;
  T_TrafficType            L_scenarioTrafType ;

  C_XmlData                  *L_xml_correlation   = NULL ;
  T_pLabelData                L_label_data        = NULL ;
  T_LabelDataList             L_label_data_list          ;
  T_LabelDataList::iterator   L_it                       ;
  T_pXmlData_List             L_correlationList   = NULL ;

  L_label_data_list.clear();

  m_nb_channel = m_channel_ctrl->nb_channel();

  if (L_data != NULL) {

    L_subList = L_data->get_sub_data() ;

    for(L_subListIt  = L_subList->begin() ;
	L_subListIt != L_subList->end() ;
	L_subListIt++) {

      L_data = *L_subListIt ;

      if (strcmp(L_data->get_name(), XML_SCENARIO_SECTION)==0) {
	
	// multi-scenarii management 
	L_subscen = L_data->get_sub_data() ;
	if (L_subscen != NULL) {
	  for (L_subscen_it = L_subscen->begin();
	       L_subscen_it != L_subscen->end();
	       L_subscen_it++) {
	    L_scen_data = *L_subscen_it ;
            
	    if (strcmp (L_scen_data->get_name(), 
			XML_SCENARIO_TRAFFIC_SECTION) == 0) {
	      if (L_xml_traffic_scen == NULL) {
		L_xml_traffic_scen = L_scen_data;
	      } else {
		GEN_ERROR(E_GEN_FATAL_ERROR,
                          "One traffic scenario allowed");
		L_XmlOk = false ;
	      }
	    } else if (strcmp (L_scen_data->get_name(), 
			       XML_SCENARIO_DEFAULT_SECTION) == 0) {
	      L_xml_default_scen.push_back(L_scen_data);
	    } else if (strcmp (L_scen_data->get_name(), 
			       XML_SCENARIO_INIT_SECTION) == 0) {
	      if (L_xml_init_scen == NULL) {
		L_xml_init_scen = L_scen_data;
	      } else {
		GEN_ERROR(E_GEN_FATAL_ERROR,
		      "One init scenario allowed");
		L_XmlOk = false ;
	      }
	    } else if (strcmp (L_scen_data->get_name(), 
			       XML_SCENARIO_ABORT_SECTION) == 0) {
	      if (L_xml_abort_scen == NULL) {
		L_xml_abort_scen = L_scen_data;
	      } else {
		GEN_ERROR(E_GEN_FATAL_ERROR,
		      "One abort scenario allowed");
		L_XmlOk = false ;
	      }
	    } else if (strcmp (L_scen_data->get_name(), 
			       XML_SCENARIO_COUNTER_SECTION) == 0) {
	      L_XmlOk = set_counters (L_scen_data);

	    } else if (strcmp (L_scen_data->get_name(), 
			       XML_SCENARIO_CORRELATION_SECTION) == 0) {
              L_xml_correlation = L_scen_data ;
              L_correlationList = L_xml_correlation->get_sub_data() ;
              if (L_correlationList != NULL) {
                m_correlation_section = true ;
              }
	    } else {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "Unkown section ["
		    << L_scen_data->get_name() 
		    << "] while traffic, init or default is expected");
		L_XmlOk = false ;
	    }
	    if (L_XmlOk == false) break ;
	  }
	}
      } // XML_SCENARIO_SECTION
    }   // for(L_subListIt ...

    if (L_XmlOk == true) {
      if (L_xml_traffic_scen == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "Section [scenario] with [traffic] is mandatory");
	L_XmlOk = false ;
      }
    }


    // add scenario init if defined
    if(L_XmlOk == true) {
      if (L_xml_init_scen != NULL) {
	if (add_scenario (E_SCENARIO_INIT, 
			  L_xml_init_scen,
			  &L_initTrafficType,
			  &L_nb_open, 
                          L_label_data_list,
                          P_data_mesure) != 0) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Init scenario error");
	  L_XmlOk = false ;
	} 
      }
    }

    // add scenario traffic (mandatory)
    if(L_XmlOk == true) {
      if (add_scenario (E_SCENARIO_TRAFFIC, 
                        L_xml_traffic_scen,
                        &L_trafficType,
                        &L_nb_open,
                        L_label_data_list,
                        P_data_mesure,
                        P_check_msg) == 0) {
        if ((L_xml_init_scen != NULL) && 
            (L_initTrafficType != L_trafficType)) {
          GEN_ERROR(E_GEN_FATAL_ERROR, 
                    "Traffic scenario and Init scenario must be"
                    << " of the same type (client or server)");
          L_XmlOk = false ;
        } else {
          L_scenarioTrafType = L_trafficType ;
          *P_traffic_type = L_scenarioTrafType ;
          GEN_DEBUG(1, "L_trafficType [" << L_trafficType << "]");
          switch (L_trafficType) {
          case E_TRAFFIC_CLIENT:
            C_GeneratorStats::instance()->info_msg((char*)"Outgoing traffic");
            break ;
          case E_TRAFFIC_SERVER:
            C_GeneratorStats::instance()->info_msg((char*)"Incomming traffic");
            break ;
          default:
            break ;
          }
        }
      } else {
        GEN_ERROR(E_GEN_FATAL_ERROR, "Traffic scenario error");
        L_XmlOk = false ;
      }
    }

    // add abort scenario if defined
    if (L_XmlOk == true) {
      if (L_xml_abort_scen != NULL) {
	if (add_scenario (E_SCENARIO_ABORT, 
			  L_xml_abort_scen,
			  &L_trafficType,
			  &L_nb_open, 
                          L_label_data_list,
                          P_data_mesure) != 0) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Abort scenario error");
	  L_XmlOk = false ;
	}
      }
    }

    if (L_XmlOk == true) {
      if ((m_abort_scen == NULL) && 
          (m_action_check_abort == true) && 
          (m_config->get_execute_check_action() == true)
          ) {
        *P_action_check_abort = true ;
      }
    }

    // add default scenarii if defined
    if (L_XmlOk == true) {
      set_default_scenarii (L_xml_default_scen.size());
      if (m_max_default != 0) {
	for (L_default_it  = L_xml_default_scen.begin();
	     L_default_it != L_xml_default_scen.end () ;
	     L_default_it++) {
	  L_current_default_scen = *L_default_it ;
	  if (add_scenario (E_SCENARIO_DEFAULT, 
			    L_current_default_scen,
			    &L_trafficType,
			    &L_nb_open, 
                            L_label_data_list,
                            P_data_mesure) != 0) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "Default scenario error");
	    L_XmlOk = false ;
	    break ;
	  } 
	}
      }
    }

    if (L_XmlOk == true) {
      if ((m_label_table_size = L_label_data_list.size ()) > 0) {
        ALLOC_TABLE(m_label_table,
                    T_pLabelData*,
                    sizeof(T_pLabelData),
                    m_label_table_size);
        
        for (L_it  = L_label_data_list.begin();
             L_it != L_label_data_list.end()  ;
             L_it++) {
          L_label_data = *L_it ;

          ALLOC_VAR(m_label_table[L_label_data->m_idx], 
                    T_pLabelData, 
                    sizeof(T_LabelData));

          m_label_table[L_label_data->m_idx]->m_idx      
            = L_label_data -> m_idx      ;
          m_label_table[L_label_data->m_idx]->m_id_cmd   
            = L_label_data -> m_id_cmd   ;
          m_label_table[L_label_data->m_idx]->m_id_msg   
            = L_label_data -> m_id_msg   ;
          m_label_table[L_label_data->m_idx]->m_nb_body_value   
            = L_label_data -> m_nb_body_value   ;
          m_label_table[L_label_data->m_idx]->m_select_line_added   
            = L_label_data->m_select_line_added  ;
          m_label_table[L_label_data->m_idx]->m_scenario = L_label_data -> m_scenario ;
        }
      }

      if (!L_label_data_list.empty()) {
  	for (L_it  = L_label_data_list.begin();
  	     L_it != L_label_data_list.end()  ;
  	     L_it++) {
  	  if (L_XmlOk == false) { // error
  	    if (m_label_table_size > 0) {
  	      L_label_data = *L_it ;
  	      FREE_VAR(m_label_table[L_label_data->m_idx]);
  	    }
  	  }
  	  FREE_VAR(*L_it);
  	}
  	L_label_data_list.erase(L_label_data_list.begin(),
                                L_label_data_list.end());
      }
      
      if (L_xml_correlation != NULL) {
        L_XmlOk = analyze_correlation (L_xml_correlation,
                                       P_data_mesure);
        if (L_XmlOk == true) {
          delete_correlation_map(&m_pre_action_map);
          delete_correlation_map(&m_post_action_map);
        }
      }
    } 
  } else {
    L_XmlOk = false ;
  }
 
  return (L_XmlOk);
}

bool C_ScenarioControl::analyze_correlation_retrieve (C_XmlData *P_data,
                                                      int        P_channel_id) {
  C_XmlData                      *L_data                   ;
  T_XmlData_List::iterator        L_listIt                 ;
  T_pXmlData_List                 L_subList     = NULL     ;
  bool                            L_ret         = true     ;
  char                           *L_value       = NULL     ;

  C_ProtocolFrame                *L_protocol    = NULL     ;
  int                             L_id                     ;

  T_IdRetrieveList                L_id_retrieve_list       ;
  T_IdRetrieveList::iterator      L_it                     ;
  int                             L_nb_id_retrieve         ;


  L_protocol = m_channel_ctrl->get_channel_protocol(P_channel_id) ;
  L_subList = P_data->get_sub_data() ;
  if (L_subList != NULL) {
    for(L_listIt  = L_subList->begin() ;
        L_listIt != L_subList->end() ;
        L_listIt++) {
      L_data = *L_listIt ;
      L_value = L_data->get_name() ;
      if (strcmp(L_value, (char*)"search-in-map") == 0) {
        L_value = L_data->find_value((char*)"name") ;
        if (L_value != NULL ) {
          L_id  = L_protocol->find_field (L_value) ;
          if (L_id == -1) {
            if (strcmp(L_value, (char*)"session-method-open-id") == 0) {
              L_id_retrieve_list.push_back(L_id);
            } else {
              GEN_ERROR(E_GEN_FATAL_ERROR,
                        "No definition found for ["
                        << L_value << "]");
              L_ret = false;
              break ;
            }
          } else {
            L_id_retrieve_list.push_back(L_id);
          }
        } else {
          GEN_ERROR(E_GEN_FATAL_ERROR, "Name is mandatory for section [search-in-map]");
          L_ret = false;
          break ;
        }
      } // if (strcmp(L_value, (char*)"search-in-map") ...)
    } // for(L_listIt = ...)

    if (L_ret) {
      if (!L_id_retrieve_list.empty()) {
        L_nb_id_retrieve = L_id_retrieve_list.size() ;
	m_retrieve_id_table[P_channel_id].m_nb_ids = L_nb_id_retrieve ;
        ALLOC_TABLE(m_retrieve_id_table[P_channel_id].m_id_table,
                    int*,
                    sizeof(int),
                    L_nb_id_retrieve);
        
        L_id = 0 ;
        for (L_it  = L_id_retrieve_list.begin();
             L_it != L_id_retrieve_list.end()  ;
             L_it++) {
          m_retrieve_id_table[P_channel_id].m_id_table[L_id] = *L_it;
          L_id++;
        }
        L_id_retrieve_list.erase(L_id_retrieve_list.begin(),
                                 L_id_retrieve_list.end());
      }
    }
  } // if (L_subList ...) 
  return (L_ret);
}

bool C_ScenarioControl::analyze_correlation_command  (C_XmlData *P_data,
                                                      int        P_channel_id,
                                                      bool      *P_data_mesure) {
  
  C_XmlData                      *L_data                         ;
  T_XmlData_List::iterator        L_listIt                       ;
  T_pXmlData_List                 L_subList           = NULL     ;

  C_XmlData                      *L_dataAction                   ;
  T_XmlData_List::iterator        L_listActionIt                 ;
  T_pXmlData_List                 L_subActionList     = NULL     ;

  bool                            L_ret               = true     ;
  char                           *L_value             = NULL     ;
  char                           *L_value_label       = NULL     ;
  int                             L_result                       ;
  int                             L_label_id                     ;
  T_pLabelData                    L_label_data        = NULL     ;

  T_InstanceDataList              L_instance_list                ;
  C_ProtocolFrame                *L_protocol = 
        m_channel_ctrl->get_channel_protocol(P_channel_id)       ;
  
  T_pC_Scenario                   L_scen              = NULL     ;
  bool                            L_map_inserted      = false    ;
  bool                            L_inserted          = false    ;
  int                             L_nb_action         = 0        ;
  int                             L_nbOpen            = 0        ;
  bool                            L_pre_action        = true     ;
  bool                            L_pre_action_done   = false    ;
  bool                            L_selectLine_added  = false    ;
  bool                            L_post_action       = false    ;
  bool                            L_add_def_action    = false    ;   
  T_pCmd_scenario                 L_cmd_sequence =  NULL               ; 

  C_CommandAction**               L_commandActionTable = NULL          ;


  T_CommandActionLst              L_preActionLabelList                 ;
  T_CommandActionLst              L_preActionMapLst                    ;
  T_CommandActionLst              L_postActionLabelList                ;
  T_CommandActionLst              L_postActionMapLst                   ;

  T_CharList::iterator            L_iterator                           ;

  L_preActionLabelList.clear();
  L_preActionMapLst.clear();
  L_postActionLabelList.clear();
  L_postActionMapLst.clear();

  C_CommandActionFactory          L_CmdActionFactory(&m_controllers)   ;


  L_instance_list.clear();
  
  L_value_label = P_data->find_value((char*)"name") ;  

  if (L_value_label != NULL ) {
    
    for (L_iterator = m_correl_name_list->begin();
         L_iterator != m_correl_name_list->end();
         L_iterator++) {
      if (strcmp(*L_iterator,L_value_label) == 0) {
        GEN_ERROR(E_GEN_FATAL_ERROR, 
                  "Command with name ["
                  << L_value_label << "] already defined");
        return (false);
      }
    }

    L_label_id = get_label_id (L_value_label) ;
    if (L_label_id != -1 ) {
      L_label_data = m_label_table[L_label_id] ;
      m_correl_name_list->push_back(L_value_label)   ;
      L_scen = L_label_data->m_scenario        ;
      L_selectLine_added = L_label_data->m_select_line_added ;
      L_cmd_sequence = &(L_scen->get_commands())[L_label_data->m_id_cmd] ;
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "Command name ["
                << L_value_label << "] unknown");
      return (false);
    }
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Name is mandatory for section [command]");
    return (false);
  }
  
  L_subList = P_data->get_sub_data() ;
  if (L_subList != NULL) {
    if (L_label_data->m_id_cmd == 0 ) {
      for (L_listIt  = L_subList->begin() ;
           L_listIt != L_subList->end() ;
           L_listIt++) {
        L_data = *L_listIt ;
        if (strcmp(L_data->get_name(), (char*)"post-action") == 0) {
          L_post_action  = true ;
          L_subActionList = L_data->get_sub_data();
          if (L_subActionList != NULL) {
            for (L_listActionIt  = L_subActionList->begin() ;
                 L_listActionIt != L_subActionList->end() ;
                 L_listActionIt++) {
              L_dataAction = *L_listActionIt ;
              if (strcmp(L_dataAction->get_name(), (char*)"insert-in-map") == 0) {
                L_map_inserted = true ; 
                break;
              }
            } // for (L_listActionIt...)
          }
        }
        if (L_map_inserted == true) break;
      } // for (L_listIt...)

      
      if (L_post_action == false) {
        L_add_def_action = true ;
      } else {
        if (L_map_inserted == false) {
          L_add_def_action      = true  ;
        } 
      }
    } // if (L_label_data->m_id_cmd ...)

    for(L_listIt  = L_subList->begin() ;
        L_listIt != L_subList->end() ;
        L_listIt++) {
      L_data = *L_listIt ;
      L_value = L_data->get_name() ;

      if (strcmp(L_value, (char*)"pre-action") == 0) {
        L_pre_action = true ;
        L_pre_action_done   = false    ;
        L_result = add_actions (L_data,
                                L_instance_list,
                                L_protocol,
                                L_scen,
                                &L_nbOpen,
                                P_data_mesure,
                                &L_map_inserted,
                                &L_selectLine_added,
                                P_channel_id,
                                L_pre_action,
                                L_pre_action_done,
                                L_nb_action,
                                L_inserted,
                                L_label_data->m_nb_body_value,
                                L_label_data->m_id_msg,
                                L_CmdActionFactory,
                                L_preActionLabelList,
                                L_commandActionTable, 
                                NULL, 
                                L_preActionMapLst,
                                true) ;
        if (L_result == -1) {
          return (false);
        }
      }

      if (strcmp(L_value, (char*)"post-action") == 0) {
        L_pre_action = false ;
        L_pre_action_done   = true    ;
        L_result = add_actions (L_data,
                                L_instance_list,
                                L_protocol,
                                L_scen,
                                &L_nbOpen,
                                P_data_mesure,
                                &L_map_inserted,
                                &L_selectLine_added,
                                P_channel_id,
                                L_pre_action,
                                L_pre_action_done,
                                L_nb_action,
                                L_inserted,
                                L_label_data->m_nb_body_value,
                                L_label_data->m_id_msg,
                                L_CmdActionFactory,
                                L_postActionLabelList,
                                L_commandActionTable, 
                                NULL, 
                                L_postActionMapLst,
                                true) ;
        if (L_result == -1) {
          return (false);
        }
      }
    } //  for(L_listIt...)


    if ((L_label_data->m_id_cmd == 0)  &&
        (L_add_def_action == true)) {
      T_CmdAction               L_action_map  ;
      L_action_map = no_cmd_action ;
      L_action_map.m_type = E_ACTION_SCEN_ADD_DEFAULT_IN_CALL_MAP ;
      L_action_map.m_id = P_channel_id ;
      L_postActionLabelList.push_back(L_CmdActionFactory.create(L_action_map)) ;
    }

    update_actions (L_preActionLabelList,
                    L_preActionMapLst,
                    m_pre_action_map,
                    L_cmd_sequence,
                    L_value_label,
                    L_label_data,
                    true) ;

    update_actions (L_postActionLabelList,
                    L_postActionMapLst,
                    m_post_action_map,
                    L_cmd_sequence,
                    L_value_label,
                    L_label_data,
                    false) ;
  } // if (L_subList != NULL)

  return (L_ret);

}

void C_ScenarioControl::update_actions (T_CommandActionLst&  P_ActionLabelList,
                                        T_CommandActionLst&  P_ActionMapLst,
                                        T_pCommandActionMap  P_action_map,
                                        T_pCmd_scenario      P_cmd_sequence,
                                        char                *P_value_label,
                                        T_pLabelData         P_label_data,
                                        bool                 P_pre_action) {
  
  int                          L_nb_action_global   = 0                           ; 
  int                          L_nb_action_label    = P_ActionLabelList.size()    ;
  int                          L_nb_action_from_map = 0                           ;
  T_CommandActionLst::iterator L_cmdActionIt                                      ;
  int                          L_actionIdx          = 0                           ;
  T_pC_Scenario                L_scen               = NULL                        ;
  
  L_scen = P_label_data->m_scenario        ;

  C_CommandAction**   L_CommandActionTable = NULL ;
  
  if (!P_ActionLabelList.empty()) {
    find_list_correl_from_map(P_action_map,
                              P_value_label,
                              P_ActionMapLst) ;

    if (!P_ActionMapLst.empty()) {
      L_nb_action_from_map = P_ActionMapLst.size() ;      
    }
      
    L_nb_action_global = L_nb_action_label + L_nb_action_from_map ;

    ALLOC_TABLE(L_CommandActionTable,
                C_CommandAction**,
                sizeof(C_CommandAction*),
                L_nb_action_global);
    
    for (L_cmdActionIt  = P_ActionLabelList.begin();
         L_cmdActionIt != P_ActionLabelList.end();
         L_cmdActionIt++) {
      L_CommandActionTable[L_actionIdx] = *L_cmdActionIt ;
      
      L_actionIdx ++ ;
    }
    
    if (!P_ActionLabelList.empty()) {
      P_ActionLabelList.erase(P_ActionLabelList.begin(),
                              P_ActionLabelList.end());
    }
    
    if (!P_ActionMapLst.empty()) {
      for (L_cmdActionIt  = P_ActionMapLst.begin();
           L_cmdActionIt != P_ActionMapLst.end();
           L_cmdActionIt++) {
        L_CommandActionTable[L_actionIdx] = *L_cmdActionIt ;
        L_actionIdx ++ ;
      }
      P_ActionMapLst.erase(P_ActionMapLst.begin(),
                           P_ActionMapLst.end());
    }
    
    if (P_pre_action == true) {
      L_scen -> update_pre_actions(L_nb_action_global, 
                                   L_CommandActionTable,
                                   P_label_data->m_id_cmd);
    } else {
      L_scen -> update_post_actions(L_nb_action_global, 
                                    L_CommandActionTable, 
                                    P_label_data->m_id_cmd);
    }
    delete_list_correl_from_map(P_action_map,
                                P_value_label,
                                false);
  } else {
    delete_list_correl_from_map(P_action_map,
                                P_value_label,
                                true);
  }
}
  
bool C_ScenarioControl::analyze_correlation (C_XmlData *P_xml_correlation,
                                             bool      *P_data_mesure) {

  C_XmlData                      *L_data                   ;
  T_XmlData_List::iterator        L_listIt                 ;
  T_pXmlData_List                 L_subList     = NULL     ;
  bool                            L_ret         = true     ;
  char                           *L_value                  ;
  int                             L_channel_id  = -1       ;
  int                             L_i                      ;
  C_XmlData                      *L_data_2                 ;
  T_pXmlData_List                 L_subListRetCommand      ;
  T_XmlData_List::iterator        L_listRetCommandIt       ;
  bool                            L_first_retrieve = false ;


  GEN_DEBUG(1, "C_ScenarioControl::analyze_correlation() start");


  L_subList = P_xml_correlation->get_sub_data() ;
  if (L_subList != NULL) {
    ALLOC_TABLE(m_retrieve_id_table, 
                T_pRetrieveIdsDef,
                sizeof(T_RetrieveIdsDef), 
                m_nb_channel);
    for (L_i = 0 ; L_i < m_nb_channel ; L_i ++) {
      m_retrieve_id_table[L_i].m_id_table = NULL ;
      m_retrieve_id_table[L_i].m_nb_ids = 0 ;
    }
    
    for(L_listIt  = L_subList->begin() ;
        L_listIt != L_subList->end() ;
        L_listIt++) {
      L_data = *L_listIt ;
      L_value = L_data->get_name() ;
      L_first_retrieve = false ;
      if (strcmp(L_value, (char*)"channel") == 0) {
        L_value = L_data->find_value((char*)"name") ;
        if (L_value == NULL ) {
          GEN_FATAL(E_GEN_FATAL_ERROR, "name value on [channel] section is mandatory");
        }
        L_channel_id = m_channel_ctrl->get_channel_id(L_value);
        if (L_channel_id  == ERROR_CHANNEL_UNKNOWN) {
          GEN_ERROR(E_GEN_FATAL_ERROR, 
                    "Channel [" << L_value << "] unknowm");
          L_ret = false ;
          break ;
        } else {
          L_subListRetCommand = L_data->get_sub_data() ;
          for(L_listRetCommandIt = L_subListRetCommand->begin() ;
              L_listRetCommandIt != L_subListRetCommand->end() ;
              L_listRetCommandIt++) {

            L_data_2 = *L_listRetCommandIt ;
            if ((L_first_retrieve == false) &&
                (strcmp(L_data_2->get_name(), (char*)"retrieve") == 0)) {
              L_first_retrieve = true;
              L_ret = analyze_correlation_retrieve(L_data_2,
                                                   L_channel_id);
              if (L_ret == false) {
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "Error on [retrieve] section on Channel [" 
                          << L_value 
                          << "]");
                break;
              }
            }

            if (strcmp(L_data_2->get_name(), (char*)"command") == 0) {
              L_ret = analyze_correlation_command(L_data_2,
                                                  L_channel_id,
                                                  P_data_mesure);
              if (L_ret == false) {
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "Error on [command] section on Channel [" 
                          << L_value 
                          << "]");
                break;
              }
            }
          } // for(L_listRetCommandIt ...)
          if (L_ret == false ) break;
        } // else ...
      } // if (strcmp)
    } // for(L_listIt...)
  } // if (L_subList...)


  return (L_ret);
}

bool C_ScenarioControl::set_counters (C_XmlData *P_xml_counter_def) {

  C_XmlData                      *L_data ;
  T_XmlData_List::iterator        L_listIt ;
  T_pXmlData_List                 L_subList ;
  char                           *L_value, *L_value_init ;
  bool                            L_ret = true ;
  unsigned long                   L_init    ;

  char                           *L_value_min, *L_value_max ;
  char                           *L_value_behaviour ;
  bool                            L_bLimitedCounter = false;
  unsigned long                   L_min    ;
  unsigned long                   L_max    ;
  T_CounterBehaviour              L_behaviour = E_CNT_RESET_MIN_TYPE ;


  list_t<C_CounterDef*>           L_counter_list ;
  list_t<C_CounterDef*>::iterator L_counter_it ;
  C_CounterDef *                  L_counter = NULL;
  C_LimitedCounterDef *           L_limitedCounter = NULL;

  int                             L_id = 0 ;
  T_CounterMap::iterator          L_counter_m_it ;
  
  GEN_DEBUG(1, "C_ScenarioControl::set_counters() start");

  L_subList = P_xml_counter_def->get_sub_data() ;

  for(L_listIt  = L_subList->begin() ;
      L_listIt != L_subList->end() ;
      L_listIt++) {

    L_data = *L_listIt ;
    L_value = L_data->get_name() ;
    
    if (strcmp(L_value, (char*)"counterdef") == 0) {
      char *L_endstr ;
      
      L_value = L_data->find_value((char*)"name") ;
      if (L_value == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "counterdef name value is mandatory");
	L_ret = false ;
	break ;
      }
      L_counter_m_it = m_counter_map->find(T_CounterMap::key_type(L_value));
      if (L_counter_m_it != m_counter_map->end()) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "counter name ["
		  << L_value << "] already defined");
	L_ret = false ;
	break ;
      }
      L_value_init = L_data->find_value((char*)"init") ;
      if (L_value_init == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
                  "counterdef init value is mandatory ["
                  << L_value << "]");
	L_ret = false ;
	break ;
      } else {
	L_init = strtoul_f (L_value_init, &L_endstr,10) ;
	if (L_endstr[0] != '\0') {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Bad format for init value ["
                    << L_value_init << "]");
	  L_ret = false ;
	  break ;
	}
      }
      
      L_value_min = L_data->find_value((char*)"min") ;
      if (L_value_min == NULL) {
        // No min value use init
        L_min = L_init;
      } else {
        L_min = strtoul_f (L_value_min, &L_endstr,10) ;
        if (L_endstr[0] != '\0') {
          GEN_ERROR(E_GEN_FATAL_ERROR, "Bad format for min value ["
                    << L_value_min << "]");
          L_ret = false ;
          break ;
        }
        L_bLimitedCounter = true;
      }

      L_value_max = L_data->find_value((char*)"max") ;
      if (L_value_max == NULL) {
        // No max value
        L_max = ULONG_MAX;
      } else {
        L_max = strtoul_f (L_value_max, &L_endstr,10) ;
        if (L_endstr[0] != '\0') {
          GEN_ERROR(E_GEN_FATAL_ERROR, "Bad format for max value ["
                    << L_value_max << "]");
          L_ret = false ;
          break ;
        }
        L_bLimitedCounter = true;
      }
      
      L_value_behaviour = L_data->find_value((char*)"behaviour") ;
      if (L_value_behaviour == NULL) {
        // Set the behaviour to default
        L_behaviour = E_CNT_RESET_INIT_TYPE ;
      } else {
        if (strcmp(L_value_behaviour,"min") == 0) {
          L_behaviour = E_CNT_RESET_MIN_TYPE ;
        } else
          if (strcmp(L_value_behaviour,"init") == 0) {
            L_behaviour = E_CNT_RESET_INIT_TYPE ;
          } else
            if (strcmp(L_value_behaviour,"no_reset") == 0) {
              L_behaviour = E_CNT_NO_RESET_TYPE ;
            } else {
              GEN_WARNING("Unknown counter behaviour ["
                          << L_value_behaviour << "]");
              // Set the behaviour to default
              L_behaviour = E_CNT_RESET_INIT_TYPE ;
            }
        L_bLimitedCounter = true;
      }

      if (L_ret == true) {
        if (L_bLimitedCounter) {
	  NEW_VAR(L_limitedCounter, 
                  C_LimitedCounterDef(L_id, L_value, L_init, 
                                      L_min, L_max, L_behaviour));
          L_counter = (C_CounterDef*)L_limitedCounter;
        } else {
	  NEW_VAR(L_counter, C_CounterDef(L_id, L_value, L_init));
        }
	L_id ++ ;
	L_counter_list.push_back(L_counter);
	m_counter_map->insert(T_CounterMap::value_type(L_value, L_counter));
      }
    }
  }

  if (!L_counter_list.empty()) {
    if (L_ret == true) {
      m_nb_counter = L_counter_list.size();

      NEW_TABLE(m_counter_table,
	        C_CounterDef*,
		m_nb_counter);

      for (L_counter_it = L_counter_list.begin();
	   L_counter_it != L_counter_list.end();
	   L_counter_it ++) {
	L_counter = *L_counter_it ;
	m_counter_table[L_counter->get_counter_id()] = L_counter ;
      }
    } else {
      for (L_counter_it = L_counter_list.begin();
	   L_counter_it != L_counter_list.end();
	   L_counter_it ++) {
	L_counter = *L_counter_it ;
	DELETE_VAR(L_counter);
      }
    }
    L_counter_list.erase(L_counter_list.begin(), 
			 L_counter_list.end());
  }

  GEN_DEBUG(1, "C_ScenarioControl::set_counters() return " << L_ret);
  
  return (L_ret);
}

T_UnsignedInteger32 C_ScenarioControl::get_counter_value (int P_id) {
  return (m_counter_table[P_id]->get_counter_value());
}

void          C_ScenarioControl::increase_counter  (int P_id) {
  m_counter_table[P_id]->increase_counter();
}

int           C_ScenarioControl::get_counter_id (char *P_name) {
  int                    L_id = -1 ;
  T_CounterMap::iterator L_counter_m_it ;

  L_counter_m_it = m_counter_map->find(T_CounterMap::key_type(P_name));
  if (L_counter_m_it != m_counter_map->end()) {
    L_id = (L_counter_m_it->second)->get_counter_id() ;
  }

  return (L_id);
}

bool C_ScenarioControl::external_data_used() {
  return (m_external_data_used);
}

int C_ScenarioControl::check_channel_usage (T_pTrafficType P_channelUsageTable,
					    int            P_primary_channel_id,
					    int            P_nb_channel) {
  int L_ret = 0 ;
  int L_i ;
  T_TrafficType L_traffic_type = E_TRAFFIC_UNKNOWN;

  if (P_primary_channel_id != -1) {
    L_traffic_type = P_channelUsageTable[P_primary_channel_id];
  } else {
    L_ret = -1 ;
  }
    
  if (L_ret != -1) {

    for (L_i = 0; L_i < P_nb_channel; L_i++) {
      if (L_i != P_primary_channel_id) {
	switch (L_traffic_type) {
	case E_TRAFFIC_CLIENT:
	  L_ret = (P_channelUsageTable[L_i] == E_TRAFFIC_SERVER) ? -1 : 0 ;
	  if (L_ret == -1) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "The primary channel is in send mode");
	    GEN_ERROR(E_GEN_FATAL_ERROR, "No receive mode possible for secondary channel(s)");
	  }
	  break ;
	case E_TRAFFIC_SERVER:
	  L_ret = (P_channelUsageTable[L_i] == E_TRAFFIC_SERVER) ? -1 : 0 ;
	  if (L_ret == -1) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "The primary channel is in receive mode");
	    GEN_ERROR(E_GEN_FATAL_ERROR, "No receive mode possible for secondary channel(s)");
	  }
	  break ;
	default:
	  L_ret = -1 ;
	  break ;
	}
      }
      if (L_ret == -1) break ;
    }
    
  }

  return (L_ret) ;
}

int C_ScenarioControl::get_nb_scenario () {
  return (m_nb_scenario);
}


int C_ScenarioControl::get_nb_default_scenario () {
  return (m_nb_default);
}

int C_ScenarioControl::get_max_nb_retrans () {

  int L_i ;
  int L_res = 0 ;
  int L_max = 0 ;
  if (m_abort_scen != NULL) {
    L_res = m_abort_scen->get_nb_retrans ();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_traffic_scen != NULL) {
    L_res = m_traffic_scen->get_nb_retrans();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_save_traffic_scen != NULL) {
    L_res = m_save_traffic_scen->get_nb_retrans();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_init_scen != NULL) {
    L_res = m_init_scen->get_nb_retrans () ;
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_nb_default != 0) {
    for(L_i=0; L_i < m_nb_default ; L_i++) {
      L_res = m_default_scen[L_i]->get_nb_retrans();
      if (L_res > L_max) { L_max = L_res ; }
    }
  }

  return (L_max);
}



int C_ScenarioControl::get_max_nb_send () {

  int L_i ;
  int L_res = 0 ;
  int L_max = 0 ;
  if (m_abort_scen != NULL) {
    L_res = m_abort_scen->get_nb_send_per_scen ();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_traffic_scen != NULL) {
    L_res = m_traffic_scen->get_nb_send_per_scen();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_save_traffic_scen != NULL) {
    L_res = m_save_traffic_scen->get_nb_send_per_scen();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_init_scen != NULL) {
    L_res = m_init_scen->get_nb_send_per_scen () ;
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_nb_default != 0) {
    for(L_i=0; L_i < m_nb_default ; L_i++) {
      L_res = m_default_scen[L_i]->get_nb_send_per_scen();
      if (L_res > L_max) { L_max = L_res ; }
    }
  }

  return (L_max);
}


int C_ScenarioControl::get_max_nb_recv () {

  int L_i ;
  int L_res = 0 ;
  int L_max = 0 ;
  if (m_abort_scen != NULL) {
    L_res = m_abort_scen->get_nb_recv_per_scen ();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_traffic_scen != NULL) {
    L_res = m_traffic_scen->get_nb_recv_per_scen();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_save_traffic_scen != NULL) {
    L_res = m_save_traffic_scen->get_nb_recv_per_scen();
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_init_scen != NULL) {
    L_res = m_init_scen->get_nb_recv_per_scen () ;
  }
  if (L_res > L_max) { L_max = L_res ; }
  if (m_nb_default != 0) {
    for(L_i=0; L_i < m_nb_default ; L_i++) {
      L_res = m_default_scen[L_i]->get_nb_recv_per_scen();
      if (L_res > L_max) { L_max = L_res ; }
    }
  }

  return (L_max);
}

int C_ScenarioControl::get_label_id (char *P_name) {
  int                         L_ret = -1 ;
  T_LabelMap::iterator        L_it       ;

  L_it = m_label_map->find(T_LabelMap::key_type(P_name)) ;
  if (L_it != m_label_map->end()) {
    L_ret = L_it->second ;
  }
  return (L_ret) ;
}

T_pLabelData C_ScenarioControl::get_label_data (int P_id) {
  T_pLabelData L_ret = NULL ;

  if (P_id < m_label_table_size) {
    L_ret = m_label_table[P_id] ;
  }

  return (L_ret) ;
}


bool C_ScenarioControl::get_correlation_section () {
  return (m_correlation_section);
}

T_pRetrieveIdsDef C_ScenarioControl::get_id_table_by_channel (int P_id) {

  T_pRetrieveIdsDef   L_ret   = NULL ;
  if (P_id < m_nb_channel) {
    L_ret = &m_retrieve_id_table[P_id] ;
  }
  return (L_ret) ;
}

void C_ScenarioControl::delete_correlation_map(T_pCommandActionMap *P_action_map) {

  T_CommandActionMap::iterator   L_it_map                ;
  T_CharList::iterator           L_iterator              ;


  T_CommandActionLst::iterator   L_list_it               ;
  bool                           L_found         = false ;


  for (L_it_map =  (*P_action_map)->begin();
       L_it_map != (*P_action_map)->end();
       L_it_map++) {
    L_found  = false ;
    for (L_iterator = m_correl_name_list->begin();
         L_iterator != m_correl_name_list->end();
         L_iterator++) {
      if (strcmp((L_it_map->first).c_str(),*L_iterator) == 0) {
        L_found = true ;
        break;
      }
    } // for (L_iterator ...)
    
    if (L_found == false) {    
      T_CommandActionLst   L_commandActionLst  ;
      L_commandActionLst = L_it_map->second ;
      for (L_list_it = L_commandActionLst.begin();
           L_list_it != L_commandActionLst.end();
           L_list_it++ ) {
        DELETE_VAR(*L_list_it) ;
      }
      if (!L_commandActionLst.empty()) {
        L_commandActionLst.erase(L_commandActionLst.begin(),
                                 L_commandActionLst.end());
      }
    }
    if (!(*P_action_map)->empty()) {
      (*P_action_map)->erase((*P_action_map)->begin(), (*P_action_map)->end());
    }
  } // for (L_it_map...)
}


