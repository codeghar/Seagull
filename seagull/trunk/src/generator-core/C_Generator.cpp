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

#include "C_Generator.hpp"
#include "C_XmlParser.hpp"
#include "Utils.hpp"
#include "TimeUtils.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "TrafficType.h"
#include "C_GeneratorStats.hpp"

#include "C_SequentialExtDataCtrl.hpp"
#include "C_RandomExtDataCtrl.hpp"

#include "C_ProtocolFrame.hpp"

#include "C_ProtocolStatsFrame.hpp"

#include "C_ScenarioStats.hpp"

#include "string_t.hpp"
#include "iostream_t.hpp"

#include <cstdlib>


typedef struct _end_trace_arg {
  C_TaskControl *m_instance ;
  pthread_t     *m_thread   ;
} T_EndTraceArg, *T_pEndTraceArg ;

void _gen_stop_controller (void* P_arg);

void _gen_log_error(char *P_data) {
  GEN_ERROR(E_GEN_FATAL_ERROR, P_data);
}

void _gen_log_info(char *P_data) {
  GEN_LOG_EVENT_FORCE(P_data);
}

C_Generator::C_Generator(cmd_line_pt P_cmd_line) : C_TaskControl() {
  int L_i;  

  // generator model
  NEW_VAR(m_transport_ctrl, C_TransportControl(_gen_log_error, _gen_log_info));
  NEW_VAR(m_protocol_ctrl, C_ProtocolControl(m_transport_ctrl));
  NEW_VAR(m_channel_ctrl, C_ChannelControl());

  NEW_VAR(m_scen_control, 
	  C_ScenarioControl(m_protocol_ctrl,m_transport_ctrl, m_channel_ctrl));
  NEW_VAR(m_read_control, C_ReadControl(m_channel_ctrl, m_transport_ctrl));

  NEW_VAR(m_keyboard_control, C_KeyboardControl());
  m_do_keyboard_control = false ;


  m_nb_forced = 0 ;


  for (L_i = 0 ; L_i < P_cmd_line->m_nb_args ; L_i ++) {
    if ((P_cmd_line->m_args[L_i]) && (strcmp(P_cmd_line->m_args[L_i],(char*)"-ctrl")== 0)) {
      P_cmd_line->m_nb_args += 2 ;
      
      ALLOC_TABLE(P_cmd_line->m_args[P_cmd_line->m_nb_args-2],
                  char*,
                  sizeof(char),
                  strlen((char*)"-dico")+1);
      strcpy(P_cmd_line->m_args[P_cmd_line->m_nb_args-2],(char*)"-dico");

      ALLOC_TABLE(P_cmd_line->m_args[P_cmd_line->m_nb_args-1],
                  char*,
                  sizeof(char),
                  strlen((char*)"remote-ctrl.xml")+1);
      strcpy(P_cmd_line->m_args[P_cmd_line->m_nb_args-1],
             (char*)"remote-ctrl.xml");
      break;
    }
  }

//    for (L_i = 0 ; L_i < P_cmd_line->m_nb_args ; L_i ++) {
//      std::cerr << "P_cmd_line->m_args[" << L_i << "] " << 
//        P_cmd_line->m_args[L_i] << std::endl;
//    }

  m_cmd_line_size =  P_cmd_line->m_nb_args ;
  m_cmd_line_values = P_cmd_line->m_args;

  m_xml_data = NULL ;

  m_do_display_control = false ;
  m_display_control = NULL ;

  m_do_log_stat_control = false ;
  m_log_stat_control = NULL ;


  m_nb_log_protocol_stat_control = 0 ;
  m_log_protocol_stat_control = NULL ;


  m_do_data_log_control = false ;
  m_data_log_control = NULL ;

  m_external_data_control = NULL ;

  m_display_traffic = NULL ;
  m_display_rtime = NULL ;
  m_display_help = NULL ;

  m_display_period = 0 ;
}

C_Generator::~C_Generator() {

  int  L_i ;
  GEN_DEBUG(1, "C_Generator::~C_Generator() start");

  DELETE_VAR(m_read_control);
  DELETE_VAR(m_scen_control);

  m_do_log_stat_control = false ;
  DELETE_VAR(m_log_stat_control);

  for (L_i = 0 ; L_i < m_nb_log_protocol_stat_control ; L_i++ ) {
    DELETE_VAR(m_log_protocol_stat_control[L_i]);
  }

  DELETE_TABLE(m_log_protocol_stat_control);


  DELETE_VAR(m_remote_control);
  m_do_remote_control = false ;

  DELETE_VAR(m_keyboard_control);
  m_do_keyboard_control = false ;

  DELETE_VAR(m_data_log_control);
  m_do_data_log_control = false ;

  m_do_display_control = false ;
  DELETE_VAR(m_display_control);

  DELETE_VAR(m_external_data_control);

  // generator model
  DELETE_VAR(m_channel_ctrl);
  DELETE_VAR(m_transport_ctrl);

  DELETE_VAR(m_protocol_ctrl);
  GEN_DEBUG(1, "C_Generator::~C_Generator() after protocol");

  // Close the Stat object
  C_GeneratorStats::instance() ->close();

  // DELETE_VAR (m_xml_data);
  m_xml_data = NULL ;
  m_cmd_line_size = 0 ;
  m_cmd_line_values = NULL ;

  DELETE_VAR(m_display_traffic);
  DELETE_VAR(m_display_rtime);
  DELETE_VAR(m_display_help);

  m_display_period = 0 ;

  GEN_DEBUG(1, "C_Generator::~C_Generator() end");
}

T_GeneratorError C_Generator::TaskProcedure() {

  T_GeneratorError        L_error_code = E_GEN_NO_ERROR ;

  pthread_t              *L_displayThread = NULL;
  pthread_t              *L_logStatThread = NULL;
  pthread_t              *L_logDataThread = NULL;
  pthread_t             **L_logProtocolStatThread = NULL ;

  pthread_t              *L_remoteThread = NULL;
  
  T_pEndTraceArg          L_arg           = NULL;


  int                     L_nb_protocol         ;
  int                     L_i                   ;

  char                    L_comment[80] ;

  GEN_DEBUG(1, "C_Generator::TaskProcedure() start");

  if (m_do_keyboard_control == true) {
    m_keyboard_control->init(this);

    m_keyboard_thread = start_thread_control(m_keyboard_control);

    // keyboard thread launched => modify end procedure for fatal errors
    ALLOC_VAR(L_arg, T_pEndTraceArg, sizeof(T_EndTraceArg));
    L_arg -> m_instance = m_keyboard_control ;
    L_arg -> m_thread = m_keyboard_thread ;
    
    use_trace_end(_gen_stop_controller, (void*)L_arg);
  }

  if (m_do_remote_control == true) {
    m_remote_control->init();
    L_remoteThread = start_thread_control(m_remote_control);
  }

  // start display control if needed
  if (m_do_display_control == true) {

    // retrieve nb protocol from protocol control
    C_ProtocolFrame      *L_ref            = NULL ;
    C_ProtocolStatsFrame *L_protocol_stats = NULL ;
    char                  L_char                  ;

    if (m_config->get_display_protocol_stat()) {

    L_nb_protocol = m_protocol_ctrl->get_nb_protocol();
    for (L_i = 0; L_i < L_nb_protocol ; L_i++) {
      L_ref = m_protocol_ctrl->get_protocol(L_i);
      snprintf(L_comment, 80, "Protocol %s screen(s)", 
	      m_protocol_ctrl->get_protocol_name(L_i));
      L_protocol_stats = L_ref->get_stats() ;
      // retrieve m_protocol_stat for each protocol
      // add_display for each
      if (L_protocol_stats != NULL) {
	L_char = '3' + L_i ;
	m_display_control->add_screen(L_protocol_stats->get_display(),
				      L_char,
				      L_comment);
      }
    }
    }

    if (m_config->get_display_scenario_stat()) {
      // retrieve nb scenario from m_scen_control
      // int    L_nb_scen ;
      T_pC_Scenario L_init_scen     = NULL ;
      T_pC_Scenario L_traffic_scen  = NULL ;
      T_pC_Scenario L_abort_scen    = NULL ;
      C_ScenarioStats *L_scenario_stats = NULL ;
      
      L_char =   'A' ;
      
      // L_nb_scen = m_scen_control->get_nb_scenario();
      L_init_scen = m_scen_control->get_init_scenario() ;
      if (L_init_scen != NULL) {
	// there is a scenario init 
	L_scenario_stats = L_init_scen->get_stats();
	if (L_scenario_stats) {
	  snprintf(L_comment, 80, "%s", "Scenario init stats");
	  m_display_control->add_screen(L_scenario_stats,
					L_char,
					L_comment);
	  L_char++;
	}
      }
      
      L_traffic_scen = m_scen_control->get_traffic_scenario() ;
      if (L_traffic_scen != NULL) {
	// there is a scenario traffic 
	L_scenario_stats = L_traffic_scen->get_stats();
	if (L_scenario_stats) {
	  snprintf(L_comment, 80, "%s", "Scenario traffic stats");
	  m_display_control->add_screen(L_scenario_stats,
					L_char,
					L_comment);
	  L_char++;
	}
      }
      
      L_abort_scen = m_scen_control->get_abort_scenario() ;
      if (L_abort_scen != NULL) {
	// there is a scenario abort 
	L_scenario_stats = L_abort_scen->get_stats();
	if (L_scenario_stats) {
	  snprintf(L_comment, 80, "%s", "Scenario abort stats");
	  m_display_control->add_screen(L_scenario_stats,
					L_char,
					L_comment);
	  L_char++;
	}
      }
      
      if (m_scen_control->get_nb_default_scenario() != 0) {
	// there is once or more scenario default 
	C_Scenario *L_default_scen ;
	int         L_default ;
	for (L_default = 0; L_default < m_scen_control->get_nb_default_scenario();
	     L_default++) {
	  L_default_scen = m_scen_control->get_default_scenario(L_default);
	  L_scenario_stats = L_default_scen->get_stats();
	  if (L_scenario_stats) {
	    snprintf(L_comment, 80, "Scenario default %d stats", L_default);
	    m_display_control->add_screen(L_scenario_stats,
					  L_char,
					  L_comment);
	    L_char++;
	  }
	}
      }
    }
    
    m_display_control->init_screens();
    L_displayThread = start_thread_control(m_display_control);
  }
  // start log stat control if needed
  if (m_do_log_stat_control == true) {
    L_logStatThread = start_thread_control(m_log_stat_control);
  }

  if (m_nb_log_protocol_stat_control) {
    ALLOC_TABLE(L_logProtocolStatThread,
		pthread_t**,
		sizeof(pthread_t*),
		m_nb_log_protocol_stat_control);
    for (L_i = 0 ; L_i < m_nb_log_protocol_stat_control ; L_i++ ) {
      L_logProtocolStatThread[L_i] 
	= start_thread_control(m_log_protocol_stat_control[L_i]);
    }
  }


  // start log data control if needed
  if (m_do_data_log_control == true) {
    L_logDataThread = start_thread_control(m_data_log_control);
  }

  // start read controller
  if (L_error_code == E_GEN_NO_ERROR) {
    L_error_code = m_read_control -> run() ;
  }

  if (m_do_keyboard_control == true) {
    m_keyboard_control -> stop () ;
  }

  if (m_do_remote_control == true) {
    m_remote_control -> stop () ;
  }

  if (m_do_display_control == true) {
    m_display_control -> stop() ;
  }
  if (m_do_log_stat_control == true) {
    m_log_stat_control -> stop() ;
  }
  if (m_do_data_log_control == true) {
    m_data_log_control -> stop() ;
  }
  for (L_i = 0 ; L_i < m_nb_log_protocol_stat_control ; L_i++ ) {
    m_log_protocol_stat_control[L_i] -> stop() ;
  }

  GEN_DEBUG(1, "C_Generator::TaskProcedure() wait log data end");
  if (m_do_data_log_control == true) {
    wait_thread_control_end (L_logDataThread);
  }

  GEN_DEBUG(1, "C_Generator::TaskProcedure() wait log stat end");
  if (m_do_log_stat_control == true) {
    wait_thread_control_end (L_logStatThread);
  }

  if (m_do_remote_control == true) {
    wait_thread_control_end (L_remoteThread);
  }


  GEN_DEBUG(1, "C_Generator::TaskProcedure() wait display end");
  if (m_do_display_control == true) {
    wait_thread_control_end (L_displayThread);
  }

  GEN_DEBUG(1, "C_Generator::TaskProcedure() wait keyboard end");
  if (m_do_keyboard_control == true) {
    wait_thread_control_end (m_keyboard_thread);
    FREE_VAR(L_arg);
  }

  for (L_i = 0 ; L_i < m_nb_log_protocol_stat_control ; L_i++ ) {
    wait_thread_control_end(L_logProtocolStatThread[L_i]);
  }
  FREE_TABLE(L_logProtocolStatThread);

  // TO DO: delete thread_t struct

  GEN_DEBUG(1, "C_Generator::TaskProcedure() end");
  return (L_error_code);
}

T_GeneratorError C_Generator::InitProcedure() {

  T_GeneratorError                 L_genError = E_GEN_NO_ERROR ;
  C_XmlParser                      L_parser    ;
  int                              L_error     = 0 ;
  char                            *L_conf_file ;
  char                            *L_scen_file ;
  
  string_t                        *L_file_all        =NULL    ;
  T_pcharPlist                     L_dico_file_list           ;
  int                              L_i                        ;
  int                              L_number_file              ; 
  int                              L_size_dico_file_list      ;
  list_t<char*>::iterator          L_it                       ;

  char                            *L_remote_cmd               ;
  char                            *L_remote_dico_path         ;


  string_t                         L_log_file("") ;

  T_TrafficType                    L_trafficType ;

  bool                             L_action_check_abort = false ;

  long                             L_displayPeriod = 0 ;
  unsigned int                     L_logLevel ;

  long                             L_logStatPeriod = 0 ;
  string_t                         L_logStatFileName ;

  char*                            L_repart ;

  struct timeval                   L_time_file ;
  char                             L_time_char [TIME_STRING_LENGTH] ;
  size_t                           L_pos ;

  bool                             L_check_msg ;

  bool                             L_data_mesure = false ;
  char                            *L_external_data_file = NULL ;

  bool                             L_files_no_timestamp = false ;

  long                             L_logProtocolStatPeriod = 0    ;
  string_t                         L_logProtocolStatFileName      ;
  char                            *L_logProtocolStatName = NULL   ;
  list_t<char*>::iterator          L_it_elt                       ;


  GEN_DEBUG(1, "C_Generator::InitProcedure() start");

  GET_TIME(&L_time_file);
  time_tochar_minus(L_time_char, &L_time_file);

  // comand line analysis
  NEW_VAR(m_config, C_GeneratorConfig(m_cmd_line_size, m_cmd_line_values));

  // retrieve values from command line
  L_conf_file = m_config -> get_conf_file () ;
  L_logLevel  = m_config -> get_log_level () ;
  L_scen_file = m_config -> get_scen_file () ;

  L_dico_file_list = m_config -> get_dico_file_list () ;
  L_size_dico_file_list  = L_dico_file_list->size() ;

  L_check_msg = m_config -> get_check_msg () ;

  L_remote_cmd = m_config -> get_remote_cmd () ;


  L_remote_dico_path = m_config -> get_remote_dico_path () ;

  // configuration xml file analysis
  if (L_conf_file != NULL) {

    GEN_DEBUG(1," the conf file [" << L_conf_file << "]" );
    GEN_DEBUG(1," the number of dico [" << L_size_dico_file_list  << "]");

    L_number_file = L_size_dico_file_list + 2 ;

    NEW_TABLE(L_file_all, string_t, L_number_file); 
    L_file_all[0] = L_conf_file ;

    L_i=1;
    for (L_it = L_dico_file_list->begin();
	 L_it != L_dico_file_list->end();
	 L_it++) {
      
      if ((L_remote_cmd) && (strcmp(*L_it,(char*)"remote-ctrl.xml")== 0)) {
        char      L_name_dico[80];
        snprintf(L_name_dico, 80, 
                 "%s/%s",L_remote_dico_path,*L_it); 
        L_file_all[L_i] = L_name_dico ;
      } else {
        L_file_all[L_i] = *L_it;
      }
      L_i ++ ;
    }
    L_file_all[L_i] = L_scen_file;

    m_xml_data = L_parser.parse(L_number_file, L_file_all, &L_error);
    
    DELETE_TABLE(L_file_all);

    if (L_error != 0) {
      L_genError = E_GEN_FATAL_ERROR ;
    }
  }

  // config management
  if (L_genError == E_GEN_NO_ERROR) {
    GEN_DEBUG(1, "C_Generator::InitProcedure() m_config->fromXml called");

    L_genError = (m_config->fromXml(m_xml_data) == false) 
      ? E_GEN_FATAL_ERROR : E_GEN_NO_ERROR ;
  }

  if (L_genError == E_GEN_NO_ERROR) {
    L_files_no_timestamp = m_config -> get_files_no_timestamp () ;
    if (m_config->get_log_file() != NULL) {
      L_log_file.append(m_config->get_log_file()) ;
    }

    if (L_log_file != "") {
      
      bool L_found = false ;
      L_pos = L_log_file.find('.');
      
      while (L_pos < L_log_file.size()) {
        if (L_pos+1 < L_log_file.size()) {
          if (L_log_file[L_pos+1] != '.') {
            L_found = true ;
            if (L_files_no_timestamp == false) {
              L_log_file.insert(L_pos,L_time_char);
              L_log_file.insert(L_pos,".");
            }
	  break ;
          } else {
	  if (L_pos+2 < L_log_file.size()) {
	    L_pos = L_log_file.find('.', L_pos+2);
	  } else {
	    break ;
	  }
          }
        } else {
          break ;
        }
      }
      
      if (L_found == false) {
        if (L_files_no_timestamp == false) {
          L_log_file += "." ;
          L_log_file.append(L_time_char);
        }
      }
      
    }
    
  }


  // transport management
  if (L_genError == E_GEN_NO_ERROR) {
    GEN_DEBUG(1, "C_Generator::InitProcedure() m_transport_ctrl->fromXml called");
    L_genError = (m_transport_ctrl->fromXml(m_xml_data) == false) 
      ? E_GEN_FATAL_ERROR : E_GEN_NO_ERROR ;
  }

  // protocol management
  if (L_genError == E_GEN_NO_ERROR) {
    GEN_DEBUG(1, "C_Generator::InitProcedure() m_protocol_ctrl->fromXml called");
    L_genError = (m_protocol_ctrl->fromXml(m_xml_data, 
					   m_config->get_config_param_list(),
					   m_config->get_display_protocol_stat()) == false) 
      ? E_GEN_FATAL_ERROR : E_GEN_NO_ERROR ;
  }

  // display control update with protocol stats
  if (L_genError == E_GEN_NO_ERROR) {
    GEN_DEBUG(1, "C_Generator::InitProcedure() m_transport_ctrl->set_config_value called");
    L_genError = (m_transport_ctrl->set_config_value(m_config->get_config_param_list()) == false) 
   ? E_GEN_FATAL_ERROR : E_GEN_NO_ERROR ;
  }

  // channel management
  if (L_genError == E_GEN_NO_ERROR) {
    GEN_DEBUG(1, "C_Generator::InitProcedure() m_channel_ctrl->fromXml called");
    L_genError = (m_channel_ctrl->fromXml(m_xml_data, 
					  m_protocol_ctrl, 
					  m_transport_ctrl) == false) 
      ? E_GEN_FATAL_ERROR : E_GEN_NO_ERROR ;
  }

  if (L_genError == E_GEN_NO_ERROR) {

    GEN_DEBUG(1, "C_Generator::InitProcedure() fromXml called success");
    m_read_control -> set_config (m_config) ;
    m_scen_control -> set_config (m_config) ;
    
    if (m_config->get_value(E_CFG_OPT_RESP_TIME_REPART, &L_repart)) {
      C_GeneratorStats::instance() 
        ->setRepartitionResponseTime(L_repart);
    }
  }


  if (L_genError == E_GEN_NO_ERROR) {

    unsigned int           L_external_select    ;
    bool                   L_result             ;

    if (m_config->get_value (E_CFG_OPT_EXTERNAL_DATA_FILE,
			     &L_external_data_file)) {
      if (L_external_data_file != NULL) {
	(void)m_config->get_value 
	  (E_CFG_OPT_EXTERNAL_DATA_SELECT, &L_external_select);

	switch (L_external_select) {
	case 0:
	  NEW_VAR(m_external_data_control, C_SequentialExtDataCtrl());
	  break ;
	case 1:
	  NEW_VAR(m_external_data_control, C_RandomExtDataCtrl());
	  break ;
	default:
	  GEN_FATAL(E_GEN_FATAL_ERROR, "Internal: Selection option not recognized for external data");
	  break ;
	}
	
	L_result = m_external_data_control->init(L_external_data_file);
        if (L_result == false) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "External data definition error");
	  L_genError = E_GEN_FATAL_ERROR ;
	}
      }
    }
  }

  if (L_genError == E_GEN_NO_ERROR) {
    init_trace (L_logLevel, 
		L_log_file, 
		m_config->get_timestamp_log()) ;
  }

  // scenario management 
  if (L_genError == E_GEN_NO_ERROR) {
    m_scen_control->set_external_data_controller(m_external_data_control);
    L_genError = (m_scen_control->fromXml(m_xml_data, 
                                          L_check_msg, 
                                          &L_data_mesure, 
                                          &L_trafficType,
                                          &L_action_check_abort) == false)
      ? E_GEN_FATAL_ERROR : E_GEN_NO_ERROR ;
  } 

  if (L_genError == E_GEN_NO_ERROR) {
    if (L_external_data_file == NULL) {
	if (m_scen_control->external_data_used() == true) {
	  GEN_FATAL(E_GEN_FATAL_ERROR, "Using external data action with no external file definition");
	  L_genError = E_GEN_FATAL_ERROR ;
	}
    }
  } 

  if (L_genError == E_GEN_NO_ERROR) {
    m_read_control -> set_scenario_control (m_scen_control, L_trafficType) ;
  }


  if (L_genError == E_GEN_NO_ERROR) {

#if BYTE_ORDER == BIG_ENDIAN 
    GEN_LOG_EVENT_FORCE("Platform use BIG_ENDIAN coding");
#else
    GEN_LOG_EVENT_FORCE("Platform use LITTLE_ENDIAN coding");
#endif

    GEN_LOG_EVENT_FORCE("Integer32 Type size is: " << sizeof(T_Integer32));

    if (sizeof(T_Integer32) < 4) {
     GEN_FATAL(E_GEN_FATAL_ERROR, "Integer32 Type is not 4 bytes size minimum");
     L_genError = E_GEN_FATAL_ERROR ;
    }
  }

  if (L_genError == E_GEN_NO_ERROR) {
    GEN_LOG_EVENT_FORCE("Integer64 Type size is: " << sizeof(T_Integer64));

    if (sizeof(T_Integer64) < 8) {
     GEN_FATAL(E_GEN_FATAL_ERROR, "Integer64 Type is not 8 bytes size minimum");
     L_genError = E_GEN_FATAL_ERROR ;
    }
  }

  if (L_genError == E_GEN_NO_ERROR) {
    GEN_LOG_EVENT_FORCE(*m_config);
  }

  if (L_genError == E_GEN_NO_ERROR) {

    if ((m_config -> get_call_timeout_beh_abr () == true) &&
        (m_scen_control->get_abort_scenario() == NULL)) {
      // there isn't a scenario abort 
      GEN_LOG_EVENT_FORCE("Abort scenario not found and you use call-timeout-behaviour-abort traffic option ");
    }

    if (L_action_check_abort == true) {
      GEN_LOG_EVENT_FORCE("Abort scenario not found and you use an action check with behavior abort");

    }
    
  }



  if (L_genError == E_GEN_NO_ERROR) {
    L_genError = m_read_control -> init() ;
  }

  if (L_genError == E_GEN_NO_ERROR) {

    // display control init
    if (m_config->get_value (E_CFG_OPT_DISPLAY_PERIOD, 
			     (unsigned long *)&L_displayPeriod)) {
      if (L_displayPeriod != 0) {
        m_display_period = L_displayPeriod ;
	

	m_do_display_control = true ;
	NEW_VAR(m_display_control, C_DisplayControl());
	m_display_control->init(L_displayPeriod);

	// set generator screens
	NEW_VAR(m_display_traffic, C_DisplayTraffic());
	NEW_VAR(m_display_rtime, C_DisplayRTime());
	NEW_VAR(m_display_help, C_DisplayHelp());

	m_display_help->set_display_control(m_display_control);


        m_display_control
          ->add_help('+',
                     (char*)"Increase call rate by call-rate-scale (default 1)");
        m_display_control
          ->add_help('-',
                     (char*)"Decrease call rate by call-rate-scale (default 1)");
        
        m_display_control
          ->add_help('c',
                     (char*)"Command mode (format : set var value)");

	m_display_control
	  ->add_help('q',
		     (char*)"Tool exit (forced when pressed two times)");
	m_display_control
	  ->add_help('p',
		     (char*)"Pause/Restart traffic");

	m_display_control
	  ->add_help('b',
		     (char*)"Burst traffic (after pause)");
	m_display_control
	  ->add_help('f',
		     (char*)"Force init scenario (switch to traffic)");


	m_display_control
	  ->add_help('d',
		     (char*)"Reset cumulative counters for each stat set in config file");

	m_display_control
	  ->add_help('a',
		     (char*)"activate/deactivate: percentage in Response time screen");
	

  	m_display_control->add_screen(m_display_traffic,
  				      '1',
				      (char *)"Traffic screen");
    	m_display_control->add_screen(m_display_rtime,
    				      '2',
				      (char *)"Response time screen");

      	m_display_control->add_screen(m_display_help,
      				      'h',
  				      (char *)"Help screen");

      }
    }


    // stat logging init
    if (m_config->get_value (E_CFG_OPT_LOG_STAT_PERIOD, 
			     (unsigned long *)&L_logStatPeriod)) {
      if (L_logStatPeriod != 0) {
	if (m_config->get_value (E_CFG_OPT_LOG_STAT_FILE,
				 L_logStatFileName)) {
	  if (L_logStatFileName != "") {

	    bool L_found = false ;
	    L_pos = L_logStatFileName.find('.');
	    while (L_pos < L_logStatFileName.size()) {
	      if (L_pos+1 < L_logStatFileName.size()) {
		if (L_logStatFileName[L_pos+1] != '.') {
		  L_found = true ;
                  if (L_files_no_timestamp == false) {
                    L_logStatFileName.insert(L_pos,L_time_char);
                    L_logStatFileName.insert(L_pos,".");
                  }
		  break ;
		} else {
		  if (L_pos+2 < L_logStatFileName.size()) {
		    L_pos = L_logStatFileName.find('.', L_pos+2);
		  } else {
		    break ;
		  }
		}
	      } else {
		break ;
	      }
	    }
	    
	    if (L_found == false) {
	      L_logStatFileName += "." ;
	      L_logStatFileName.append(L_time_char);
	    }
	    m_do_log_stat_control = true ;
	    NEW_VAR(m_log_stat_control, C_LogStatControl());
	    m_log_stat_control->init(L_logStatPeriod, L_logStatFileName);
	  } else {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "No file defined for statistic log");
	  }
	}
      }
    }

    if (L_remote_cmd) {
      char                    L_remote_address[80] ;

      snprintf(L_remote_address, 80, 
               "mode=server;source=%s",L_remote_cmd); 
      NEW_VAR(m_remote_control, C_RemoteControl(this,m_protocol_ctrl,L_remote_address)); // add a port
      m_do_remote_control = true ; 
    }

    if (m_config->get_display_protocol_stat()) {
      if (m_config->get_value (E_CFG_OPT_LOG_PROTOCOL_STAT_PERIOD, 
			       (unsigned long *)&L_logProtocolStatPeriod)) {
	if (L_logProtocolStatPeriod != 0) {
	  // retrieve the name of protocol stat
	  if (m_config->get_value (E_CFG_OPT_LOG_PROTOCOL_STAT_FILE,
				   L_logProtocolStatFileName)) {
	    if (L_logProtocolStatFileName == "") {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "No file defined for statistic log");
	    } else {
	      // retrieve protocol 
	      if (m_config->get_value (E_CFG_OPT_LOG_PROTOCOL_STAT_NAME,
				       &L_logProtocolStatName)) {
		if (L_logProtocolStatName == NULL) {
		  GEN_ERROR(E_GEN_FATAL_ERROR, "No name defined for protocol statistic log");
		} else {
		  // Regular expression
		  T_charPlist                *L_protocol_list                ;
		  bool                        L_all_found      = false       ;
		  C_ProtocolFrame            *L_ref = NULL                   ;
		  // char                       *L_protocol_name  = NULL        ;
		  int                         L_nb_protocol    = 0           ;
		  C_ProtocolStatsFrame       *L_protocol_stats = NULL        ;
		  C_LogProtocolStatControl   *L_proto_stat_inter = NULL      ;
		  int                         L_id                           ;
		  
		  
		  NEW_VAR (L_protocol_list, T_charPlist);
		  
		  if (m_config->analyze_protocol_stat(L_logProtocolStatName,
						      L_protocol_list,
						      &L_all_found) == -1 ) {
		    GEN_ERROR(E_GEN_FATAL_ERROR, "Regular expression building is failed");
		    L_genError = E_GEN_FATAL_ERROR ;
		  } else {
		    // retrieve nb protocol from m_protocol_control
		    if (L_all_found == true) {
		      L_nb_protocol = m_protocol_ctrl->get_nb_protocol();
		      for (L_i = 0; L_i < L_nb_protocol ; L_i++) {
			// retrieve the name of each protocol
			// L_protocol_name = m_protocol_ctrl->get_protocol_name (L_i);
			L_ref = m_protocol_ctrl->get_protocol(L_i);		      
			L_protocol_stats = L_ref->get_stats() ;		      
			if (L_protocol_stats != NULL) {
			  L_protocol_stats->do_log();
			  m_nb_log_protocol_stat_control++ ;
			}
		      }
		      
		      if ( m_nb_log_protocol_stat_control != 0 ) {
			
			NEW_TABLE(m_log_protocol_stat_control,
				  C_LogProtocolStatControl*,
				  m_nb_log_protocol_stat_control);
			
			L_id = 0;
			for (L_i = 0; L_i < L_nb_protocol ; L_i++) {
			  
			  L_proto_stat_inter 
			    = create_log_protocol_stats(m_protocol_ctrl->get_protocol(L_i),
							L_time_char,
							L_logProtocolStatFileName,
							L_logProtocolStatPeriod,
                                                        L_files_no_timestamp);
			  if (L_proto_stat_inter != NULL) {
			    m_log_protocol_stat_control[L_id] = L_proto_stat_inter ;
			    L_id++ ;
			  }
			  
			  
			} //  for (L_i = 0; L_i < L_nb_protocol ; L_i++)
		    } // if ( m_nb_log_protocol_stat_control != 0 )
		      
		      
		    } else {
		      if (!L_protocol_list->empty()) {
			for (L_it_elt  = L_protocol_list->begin() ;
			     L_it_elt != L_protocol_list->end()  ;
			     L_it_elt++) {
			  L_ref = m_protocol_ctrl->get_protocol (*L_it_elt);
			  if (L_ref == NULL) {
			    GEN_ERROR(E_GEN_FATAL_ERROR, "No protocol found for this name ["
				      << *L_it_elt << "]");
			  } else {
			    
			    L_protocol_stats = L_ref->get_stats() ;		      
			    if (L_protocol_stats != NULL) {
			      L_protocol_stats->do_log();
			      m_nb_log_protocol_stat_control++ ; 
			    }
			  } 
			} // for (L_it_elt...
			
			if ( m_nb_log_protocol_stat_control != 0 ) {
			  
			  NEW_TABLE(m_log_protocol_stat_control,
				    C_LogProtocolStatControl*,
				    m_nb_log_protocol_stat_control);
			  
			  L_id = 0;
			  
			  for (L_it_elt  = L_protocol_list->begin() ;
			       L_it_elt != L_protocol_list->end()  ;
			       L_it_elt++) {
			    L_ref = m_protocol_ctrl->get_protocol (*L_it_elt);
			    
			    if (L_ref != NULL) {
			      L_protocol_stats = L_ref->get_stats() ;      
			      if (L_protocol_stats != NULL) {
				L_proto_stat_inter 
				  = create_log_protocol_stats(L_ref,
							      L_time_char,
							      L_logProtocolStatFileName,
							      L_logProtocolStatPeriod,
                                                              L_files_no_timestamp);
				if (L_proto_stat_inter != NULL) {
				  m_log_protocol_stat_control[L_id] = L_proto_stat_inter ;
				  L_id++ ;
				}
			      } // if (L_protocol_stats != NULL)
			    } // if (L_ref != NULL)
			  } // for (L_it_elt  = L_protocol_list->begin() ..
			} // if ( m_nb_log_protocol_stat_control != 0 )
		      } // if (!L_protocol_list->empty())
		    } // if (L_all_found == true)
		    
		    
		    // remove list 
		    if (!L_protocol_list->empty()) {
		      // for each element free table
		      for (L_it_elt  = L_protocol_list->begin() ;
			   L_it_elt != L_protocol_list->end()  ;
			   L_it_elt++) {
			FREE_TABLE(*L_it_elt);
		      }
		      L_protocol_list->erase(L_protocol_list->begin(),
					     L_protocol_list->end());
		      
		    }
		    DELETE_VAR(L_protocol_list);
		    
		    
		  } // if (m_config->analyze_protocol_stat...
		  
		} //  if (L_logProtocolStat != NULL)
	      }
	    } // if (L_logProtocolStatFileName == "")
	  }
	} // if (L_logProtocolStatPeriod != 0)
      } // if (m_config->get_value (...
    }


    // data logging init
    if (L_data_mesure == true) {

      string_t              L_data_log_file ;
      size_t                L_data_nb = 0;
      unsigned long         L_data_val, L_data_period ;

      unsigned long         L_data_rtdistrib = 0 ;

      // take values from configuration
      if (m_config->get_value (E_CFG_OPT_DATA_LOG_FILE,
			       L_data_log_file)) {

	if (L_data_log_file != "") {
	  bool L_found = false ;
	  (void)m_config->get_value 
	    (E_CFG_OPT_DATA_LOG_PERIOD, &L_data_period);
	  (void)m_config->get_value
	    (E_CFG_OPT_DATA_LOG_NUMBER, &L_data_val);
	  L_data_nb = (size_t) L_data_val ;

	  // add date
	  L_pos = L_data_log_file.find('.');
	  
	  while (L_pos < L_data_log_file.size()) {
	    if (L_pos+1 < L_data_log_file.size()) {
	      if (L_data_log_file[L_pos+1] != '.') {
		L_found = true ;
                if (L_files_no_timestamp == false) {
                  L_data_log_file.insert(L_pos,L_time_char);
                  L_data_log_file.insert(L_pos,".");
                }
		break ;
	      } else {
		if (L_pos+2 < L_data_log_file.size()) {
		  L_pos = L_data_log_file.find('.', L_pos+2);
		} else {
		  break ;
		}
	      }
	    } else {
	      break ;
	    }
	  }
	  
	  if (L_found == false) {
	    L_data_log_file += "." ;
	    L_data_log_file.append(L_time_char);
	  }
	  
	  if (L_data_nb == 0) {
	    m_do_data_log_control = false ;
	  } else {
	    if (L_data_period == 0) {
	      m_do_data_log_control = false ;
            } else {
	      m_do_data_log_control = true ;
	    }
	  }

	} else {
	  m_do_data_log_control = false ;
	}

	if (m_do_data_log_control == true) {
	  
          GEN_DEBUG(1, "C_Generator::InitProcedure() L_data_nb is " << L_data_nb);
          GEN_DEBUG(1, "C_Generator::InitProcedure() L_data_period is " << L_data_period);

	  (void)m_config->get_value 
	    (E_CFG_OPT_DATA_LOG_RTDISTRIB, &L_data_rtdistrib);
	  
	  if (L_data_rtdistrib == 0) {
	    NEW_VAR(m_data_log_control, 
		    C_DataLogControl(2*L_data_nb,
				     L_data_nb,
				     (char*)L_data_log_file.c_str(),
				     L_data_period));
	  } else {
	    NEW_VAR(m_data_log_control, 
		    C_DataLogRTDistrib(2*L_data_nb,
				       L_data_nb,
				       (char*)L_data_log_file.c_str(),
				       L_data_period, L_data_rtdistrib));
	    
	  }
	  
	  m_scen_control->set_data_log_controller(m_data_log_control);
	  m_data_log_control->init();
	  if (L_data_rtdistrib == 0) {
	    if (m_data_log_control->data((char*)"time-ms;response-time-ms;") == 0) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, "Internal error");
	      L_genError = E_GEN_FATAL_ERROR ;
	    }
	  }
	}
	
      }
      
    } // if (L_data_mesure ..)
  }


  if (L_genError == E_GEN_NO_ERROR) {

    // background mode
    if (m_config->get_bg_mode() == true) {

      pid_t L_pid;

      m_do_display_control = false ; // no display
      m_do_keyboard_control = false ; // no keyboard control
      switch(L_pid = fork())
        {
        case -1:
          // error when forking !
	  GEN_FATAL(E_GEN_FATAL_ERROR, "Forking error");
	  break ;
        case 0:
          // child process - poursuing the execution
	  // background mode ok
          break;
        default:
          // parent process - killing the parent - the child get the parent pid
          iostream_error << "Background mode - PID [" 
			 << L_pid << "]" << iostream_endl ;
          exit(0);
        }
      
    } else {
      m_do_keyboard_control = true ;
    }
    GEN_LOG_EVENT_FORCE("PID [" << getpid() << "]");
  }

  GEN_DEBUG(1, "C_Generator::InitProcedure() end");
  return (L_genError);
}

T_GeneratorError C_Generator::EndProcedure() {

  GEN_DEBUG(1, "C_Generator::EndProcedure() start");

  DELETE_VAR(m_config);

  GEN_DEBUG(1, "C_Generator::EndProcedure() end");
  
  return (ReturnCode());
}

T_GeneratorError C_Generator::ForcedStoppingProcedure() {

  // just other thread concerned
  m_read_control->stop();

  m_nb_forced++ ;

  if (m_nb_forced >= 2) {
    iostream_error << "Forced exit" << iostream_endl ;
    GEN_LOG_EVENT_FORCE("Forced exit");
    ExitProcedure() ;
  }

  return (E_GEN_NO_ERROR);
}


T_GeneratorError C_Generator::StoppingProcedure() {

  GEN_DEBUG(1, "C_Generator::StoppingProcedure () start");

  // just other thread concerned
  m_read_control -> stop() ;

  GEN_DEBUG(1, "C_Generator::StoppingProcedure () end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_Generator::ReturnCode() {
 
  unsigned long      L_counter_value_failed   = 0              ;
  unsigned long      L_counter_value_rejected = 0              ;
  unsigned long      L_counter_value_aborted  = 0              ;
  unsigned long      L_counter_value_timeout  = 0              ;

  T_GeneratorError   L_genError               = E_GEN_NO_ERROR ;

  L_counter_value_failed = 
    C_GeneratorStats::instance()->getStatAction(C_GeneratorStats::E_CALL_FAILED,
                                                C_GeneratorStats::CPT_C_FailedCall);

  L_counter_value_rejected = 
    C_GeneratorStats::instance()->getStatAction(C_GeneratorStats::E_CALL_REFUSED,
                                                C_GeneratorStats::CPT_C_RefusedCall);

  L_counter_value_aborted = 
    C_GeneratorStats::instance()->getStatAction(C_GeneratorStats::E_FAILED_ABORTED,
                                                C_GeneratorStats::CPT_C_FailedCallAborted);

  L_counter_value_timeout = 
    C_GeneratorStats::instance()->getStatAction(C_GeneratorStats::E_FAILED_TIMEOUT,
                                                C_GeneratorStats::CPT_C_FailedCallTimeout);

  if (L_counter_value_timeout  == 0  &&
      L_counter_value_aborted  == 0  &&
      L_counter_value_rejected == 0  &&
      L_counter_value_failed   == 0) {
    L_genError = E_GEN_NO_ERROR ;
  } else {
    L_genError = E_GEN_ERROR_CALL_FAILED ;
  }

  return (L_genError);
}

void C_Generator::set_screen (char P_key) {
  GEN_DEBUG(1, "C_Generator::set_screen(" << P_key << ") start");
  if (m_display_control != NULL) {
    m_display_control->set_screen(P_key) ;
  }
  GEN_DEBUG(1, "C_Generator::set_screen() end");
}

void C_Generator::pause_traffic () {
  GEN_DEBUG(1, "C_Generator::pause_traffic()");
  if (m_read_control != NULL) {
    m_read_control -> pause_traffic () ;
  }
  GEN_DEBUG(2, "C_Generator::pause_traffic()");
}

void C_Generator::pause_display () {
  GEN_DEBUG(1, "C_Generator::pause_traffic()");
  if (m_display_control != NULL) {
    m_display_control -> pause () ;
  }
  GEN_DEBUG(2, "C_Generator::pause_traffic()");
}

void C_Generator::restart_traffic () {
  GEN_DEBUG(1, "C_Generator::restart_traffic()");
  if (m_read_control != NULL) {
    m_read_control -> restart_traffic () ;
  }
  GEN_DEBUG(2, "C_Generator::restart_traffic()");
}

void C_Generator::burst_traffic () {
  GEN_DEBUG(1, "C_Generator::burst_traffic()");
  if (m_read_control != NULL) {
    m_read_control -> burst_traffic () ;
  }
  GEN_DEBUG(2, "C_Generator::burst_traffic()");
}

void C_Generator::force_init() {
  if (m_read_control != NULL) {
    m_read_control -> force_init() ;
  }
}

unsigned long C_Generator::get_call_rate() {
  unsigned long L_ret = 0 ;
  if (m_read_control != NULL) {
    L_ret = m_read_control -> get_call_rate ();
  }
  return (L_ret);
}


void C_Generator::change_call_rate(T_GenChangeOperation P_op,
				   unsigned long        P_rate) {
  if (m_read_control != NULL) {
    m_read_control -> change_call_rate (P_op, P_rate);
  }
}

void C_Generator::change_rate_scale(unsigned long P_scale) {
  if (m_read_control != NULL) {
    m_read_control -> change_rate_scale (P_scale);
  }
}


void C_Generator::change_burst (unsigned long P_burst) {
  if (m_read_control != NULL) {
    m_read_control -> change_burst (P_burst);
  }
}


void C_Generator::change_display_period(long P_period) {
  if ((m_display_period) && (P_period) && (m_display_control != NULL) ) {
    m_display_control -> change_display_period (P_period);
  }
}

void C_Generator::ExitProcedure () {
  int L_i = 0 ;
  if (m_do_keyboard_control == true) {
    m_keyboard_control -> stop () ;
  }
  if (m_do_display_control == true) {
    m_display_control -> stop() ;
  }
  if (m_do_log_stat_control == true) {
    m_log_stat_control -> stop() ;
  }

  if (m_do_remote_control == true) {
    m_remote_control -> stop () ;
  }

  if (m_do_keyboard_control == true) {
    m_keyboard_control->force_end_procedure();
  }
  for (L_i = 0 ; L_i < m_nb_log_protocol_stat_control ; L_i++ ) {
    m_log_protocol_stat_control[L_i] -> stop() ;
  }
  exit (-1) ;
}

void _gen_stop_controller (void* P_arg) {

  T_pEndTraceArg L_arg = (T_pEndTraceArg) P_arg ;

  if (L_arg != NULL) {
    if (L_arg->m_instance != NULL) {
      L_arg->m_instance->stop() ;
      if (L_arg->m_thread != NULL) {
	wait_thread_control_end(L_arg->m_thread);
      }
    }
  }

}


C_LogProtocolStatControl* C_Generator::create_log_protocol_stats (C_ProtocolFrame *P_protocol,
								  char            *P_timechar,
								  string_t&        P_logProtocolStatFileName,
								  unsigned long    P_log_period_period,
                                                                  bool             P_files_no_timestamp) {
  
  C_ProtocolStatsFrame       *L_protocol_stats   = NULL  ;
  C_LogProtocolStatControl   *L_proto_stat_inter = NULL  ;
  bool                        L_found            = false ;
  size_t                      L_pos                      ;
  char                       *L_protocol_name            ;
  string_t                    L_file_name("")            ;
  
  L_protocol_stats = P_protocol->get_stats() ;	
  L_protocol_name = P_protocol->name();

  L_file_name += P_logProtocolStatFileName ;

  if (L_protocol_stats != NULL) {
    
    L_pos = L_file_name.find('.');
    
    while (L_pos < L_file_name.size()) {
      if (L_pos+1 < L_file_name.size()) {
	if (L_file_name[L_pos+1] != '.') {
	  L_found = true ;
          if (P_files_no_timestamp == false) {
            L_file_name.insert(L_pos,P_timechar);
            L_file_name.insert(L_pos,".");
          }

	  L_file_name.insert(L_pos,L_protocol_name);
	  L_file_name.insert(L_pos,".");
	  break ;
	} else {
	  if (L_pos+2 < L_file_name.size()) {
	    L_pos = L_file_name.find('.', L_pos+2);
	  } else {
	    break ;
	  }
	}
      } else {
	break ;
      }
    }
    
    if (L_found == false) {
      L_file_name += "." ;
      L_file_name.insert(L_pos,L_protocol_name);
      if (P_files_no_timestamp == false) {
        L_file_name += "." ;
        L_file_name.append(P_timechar);
      }
    }
	
    NEW_VAR(L_proto_stat_inter, 
	    C_LogProtocolStatControl(L_protocol_stats));
    
    L_proto_stat_inter->init(P_log_period_period, L_file_name);
  }

  return (L_proto_stat_inter);
			  
}		  


void C_Generator::reset_cumul_counters () {
  int  L_i ;
  GEN_DEBUG(1, "C_Generator::reset_cumul_counters() start");

  GEN_LOG_EVENT_FORCE("Reset cumulative counters for each stat set in config file");

  if (m_do_log_stat_control == true) {
    m_log_stat_control -> reset_cumul_counters() ;
  }

  for (L_i = 0 ; L_i < m_nb_log_protocol_stat_control ; L_i++ ) {
    m_log_protocol_stat_control[L_i] -> reset_cumul_counters() ;
  }

  if (m_config->get_display_scenario_stat()) {
    // retrieve nb scenario from m_scen_control
    T_pC_Scenario L_init_scen     = NULL ;
    T_pC_Scenario L_traffic_scen  = NULL ;
    T_pC_Scenario L_abort_scen    = NULL ;
    C_ScenarioStats *L_scenario_stats = NULL ;
    
    L_init_scen = m_scen_control->get_init_scenario() ;
    if (L_init_scen != NULL) {
      // there is a scenario init 
      L_scenario_stats = L_init_scen->get_stats();
      if (L_scenario_stats) {
	L_scenario_stats->reset_cumul_counters() ;
      }
    }
    
    L_traffic_scen = m_scen_control->get_traffic_scenario() ;
    if (L_traffic_scen != NULL) {
      // there is a scenario traffic 
      L_scenario_stats = L_traffic_scen->get_stats();
      if (L_scenario_stats) {
	L_scenario_stats->reset_cumul_counters() ;
      }
    }
      
    L_abort_scen = m_scen_control->get_abort_scenario() ;
    if (L_abort_scen != NULL) {
      // there is a scenario abort 
      L_scenario_stats = L_abort_scen->get_stats();
      if (L_scenario_stats) {
	L_scenario_stats->reset_cumul_counters() ;
      }
    }
      
    if (m_scen_control->get_nb_default_scenario() != 0) {
      // there is once or more scenario default 
      C_Scenario *L_default_scen ;
      int         L_default ;
      for (L_default = 0; L_default < m_scen_control->get_nb_default_scenario();
	   L_default++) {
	L_default_scen = m_scen_control->get_default_scenario(L_default);
	L_scenario_stats = L_default_scen->get_stats();
	if (L_scenario_stats) {
	  L_scenario_stats->reset_cumul_counters() ;
	}
      }
    }
  } //  if (m_config->get_display_scenario_stat())


  GEN_DEBUG(1, "C_Generator::reset_cumul_counters() end");
}




void C_Generator::activate_percent_traffic () {
  if (m_do_log_stat_control == true) {
    m_log_stat_control -> activate_percent_traffic() ;
  }
}




