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

#include "C_GeneratorConfig.hpp"
#include "C_CommandLine.cpp"
#include "GeneratorTrace.hpp"
#include "GeneratorDefaults.h"
#include "integer_t.hpp" // For strtoul_f

#include <unistd.h>
#include <cstring>

#define XML_CONFIG_SECTION      (char*)"configuration"
#define XML_CONFIG_SUBSECTION   (char*)"define"
#define XML_CONFIG_ENTITY       (char*)"entity"
#define XML_CONFIG_ENTITY_VALUE (char*)"traffic-param"
#define XML_CONFIG_NAME         (char*)"name"
#define XML_CONFIG_VALUE        (char*)"value"
#define XML_CONFIG_PARAM_ENTITY_VALUE (char*)"config-param"


const char* config_opt_table[] = {
  "call-rate",
  "burst-limit",
  "max-send",
  "max-receive",
  "select-timeout-ms",
  "max-simultaneous-calls",
  "call-timeout-ms",
  "msg-buffer-size",
  "display-period",
  "log-stat-period",
  "log-file",
  "log-stat-file",
  "response-time-repartition",
  "number-calls",
  "msg-check-level",
  "msg-check-behaviour",
  "data-log-file",
  "data-log-period",
  "data-log-number",
  "call-rate-scale",
  "external-data-file",
  "external-data-select",
  "data-log-rtdistrib",
  "log-protocol-stat-period",
  "log-protocol-stat-name",
  "log-protocol-stat-file",
  "display-protocol-stat",
  "display-scenario-stat",
  "files-no-timestamp",
  "call-timeout-behaviour-abort",
  "call-open-timeout-ms",
  "execute-check-action",
  "max-retrans",
  "retrans-enabled",
  "model-traffic-select"

};

static const char _check_level_char [] = {
  'P',
  'A'
} ;

static const char _check_behaviour_char [] = {
  'W',
  'E'
} ;

const unsigned int _check_level_mask [] = {
  0x00000002,
  0x00000004
} ;

const unsigned int _check_behaviour_mask [] = {
  LOG_LEVEL_WAR,
  LOG_LEVEL_ERR
} ;

const char* _check_external_select[] = {
  "sequential",
  "random"
} ;


const char* _model_traffic_select[] = {
  "uniform",
  "best-effort",
  "poisson"
} ;

bool C_GeneratorConfig::set_data 
(T_GeneratorCmdLineOption   P_opt, 
 char                     **P_argtab) {

  char * L_currentValue = P_argtab [0] ;

    switch (P_opt) {

    case E_CMDLINE_conf_file:
      m_option_conf_file = L_currentValue ;
      return (true) ;

    case E_CMDLINE_scen_file:
      m_option_scen_file = L_currentValue ;
      return (true) ;

    case E_CMDLINE_dico_file:
      m_option_dico_file_list->push_back(L_currentValue) ;
      return (true) ;

    case E_CMDLINE_log_file:
      m_option_log_file = L_currentValue ;
      return (true) ;

    case E_CMDLINE_log_level: {
      for (unsigned int L_i=0; L_i < strlen(L_currentValue) ; L_i++) {
	switch (L_currentValue[L_i]) {
	case 'E':
	case 'e':
	  m_option_log_level |= gen_mask_table[LOG_LEVEL_ERR] ;
	  break ;
	case 'W':
	case 'w':
	  m_option_log_level |= gen_mask_table[LOG_LEVEL_WAR] ;
	  break ;
	case 'M':
	case 'm':
	  m_option_log_level |= gen_mask_table[LOG_LEVEL_MSG] ;
	  break ;
	case 'T':
	case 't':
	  m_option_log_level |= gen_mask_table[LOG_LEVEL_TRAFFIC_ERR] ;
	  break ;
	case 'B':
	case 'b':
	  m_option_log_level |= gen_mask_table[LOG_LEVEL_BUF] ;
	  break ;
	case 'A':
	case 'a':
	  m_option_log_level |= gen_mask_table[LOG_LEVEL_ALL] ;
	  break ;
	case 'N':
        case 'n':
	  if (m_option_log_level & gen_mask_table[LOG_LEVEL_ERR]) {
	    m_option_log_level -= gen_mask_table[LOG_LEVEL_ERR] ;
	  }
	  break ;
	default:
	  return (false);
	}
      }
    }
      return (true) ;

    case E_CMDLINE_help:
      return (true) ;

    case E_CMDLINE_bg_mode:
      m_option_bg_mode = true ;
      return (true) ;

    case E_CMDLINE_timestamp_log:
      m_option_timestamp_log = false ;
      return (true) ;

    case E_CMDLINE_check_msg:
      m_option_check_msg = true ;
      return (true) ;

    case E_CMDLINE_remote_cmd:
      m_option_remote_cmd = L_currentValue ;
      return (true) ;

    case E_CMDLINE_remote_dico_path:
      m_option_remote_dico_path = L_currentValue ;
      return (true) ;


    case E_CMDLINE_nbOptions:
      return (false) ;		// ???
      
    } // switch (P_opt)
    
    return false;		// ???

} // C_GeneratorConfig::set_data


C_GeneratorConfig::C_GeneratorConfig (int P_argc, char** P_argv) {

  int L_result ;
  int L_i ;

  NEW_VAR (m_option_dico_file_list, T_charPlist);
 
  NEW_VAR (m_configValueList, T_ConfigValueList);

  C_GeneratorConfig::T_value_type one_value_string [] = {
    C_GeneratorConfig::E_VT_STRING
  } ;


  C_GeneratorConfig::T_command_option L_command_opt [] = {

     { E_CMDLINE_conf_file, (char*)"conf", 
       C_GeneratorConfig::E_OT_MANDATORY, 1 , one_value_string, 
       (char*)"<configuration file name>", (char*)""},

     { E_CMDLINE_scen_file, (char*)"scen", 
       C_GeneratorConfig::E_OT_MANDATORY, 1 , one_value_string, 
       (char*)"<scenario file name>", (char*)""},

     { E_CMDLINE_dico_file, (char*)"dico", 
       C_GeneratorConfig::E_OT_MANDATORY, 1 , one_value_string, 
       (char*)"<protocol dictionary file name>", (char*)"can be used more than once"},

     { E_CMDLINE_log_file, (char*)"log", 
       C_GeneratorConfig::E_OT_OPTIONAL, 1 , one_value_string, 
       (char*)"<logging file name>", (char*)""},

     { E_CMDLINE_log_level, (char*)"llevel", 
       C_GeneratorConfig::E_OT_OPTIONAL, 1 , one_value_string, 
       (char*)"<logging level mask>", 
       (char*)"levels: \n          M: msg,     B: buffer,   E: error,\n          W: warning, N: no error, T: traffic error,\n          A: all.     Default E"},

     { E_CMDLINE_help, (char*)"help",
       C_GeneratorConfig::E_OT_OPTIONAL, 0, NULL,
       (char*)"", (char*)"display syntax command line"},

     { E_CMDLINE_bg_mode, (char*)"bg",
       C_GeneratorConfig::E_OT_OPTIONAL, 0, NULL,
       (char*)"", (char*)"background mode"},

     { E_CMDLINE_timestamp_log, (char*)"notimelog",
       C_GeneratorConfig::E_OT_OPTIONAL, 0, NULL,
       (char*)"", (char*)"no time stamp on the log (default time stamp)"},

     { E_CMDLINE_check_msg, (char*)"msgcheck",
       C_GeneratorConfig::E_OT_OPTIONAL, 0, NULL,
       (char*)"", 
       (char*)"check the field of the messages received (default no check)"},

     { E_CMDLINE_remote_cmd, (char*)"remotecontrol",
       C_GeneratorConfig::E_OT_OPTIONAL, 1,  one_value_string,
       (char*)"", 
       (char*)"remote control active @IP:port (default no remote)"},

     { E_CMDLINE_remote_dico_path, (char*)"remote_dico_path",
       C_GeneratorConfig::E_OT_OPTIONAL, 1,  one_value_string,
       (char*)"", 
       (char*)"remote dictionnary path (default /usr/local/share/seagull/config)"}

  } ;


  // general command line description
  C_GeneratorConfig::T_command_line L_command_descr = {
    E_CMDLINE_nbOptions, // options number
    E_CMDLINE_help, // option index for help: -1 => no help option
    L_command_opt // command options table
  } ;


  // Init to default values
  m_option_log_file  = DEF_OPTION_LOG_FILE  ;
  m_option_conf_file = DEF_OPTION_CONF_FILE ;

  m_option_scen_file = DEF_OPTION_SCEN_FILE ;
  m_option_log_level = gen_mask_table[DEF_OPTION_LOG_LEVEL] ;
  m_option_bg_mode   = DEF_OPTION_BG_MODE   ;
  m_call_rate = DEF_CALL_RATE ;
  m_burst_limit = DEF_BURST_LIMIT ;
  m_max_send = DEF_MAX_SEND ;
  m_max_receive = DEF_MAX_RECEIVE ;
  m_select_timeout = DEF_SELECT_TIMEOUT_MS ;
  m_max_simultaneous_calls = DEF_MAX_SIMULTANEOUS_CALLS ;
  m_call_timeout = DEF_CALL_TIMEOUT ;
  m_msg_buffer_size = DEF_MSG_BUFFER_SIZE ;
  m_display_period = DEF_DISPLAY_PERIOD ;
  m_log_stat_period = DEF_LOG_STAT_PERIOD ;
  m_log_stat_file = DEF_LOG_STAT_FILE ;

  m_external_data_file = DEF_EXTERNAL_DATA_FILE ; 
  m_external_data_select = DEF_EXTERNAL_DATA_SELECT ; // by default sequential

  m_resp_time_repart = DEF_RESP_TIME_REPART ;
  m_number_calls = DEF_NUMBER_CALLS ;
  m_option_timestamp_log = DEF_TIMESTAMP_LOG ;
  m_option_check_msg = DEF_CHECK_MSG ;
  m_check_level_mask = _check_level_mask[DEF_CHECK_LEVEL] ;
  m_check_behaviour = DEF_CHECK_BEHAVIOUR ;

  m_data_log_file = DEF_DATA_LOG_FILE ;
  m_data_log_period = DEF_DATA_LOG_PERIOD ;
  m_data_log_number = DEF_DATA_LOG_NUMBER ;

  m_data_log_rtdistrib = DEF_DATA_LOG_RTDISTRIB ;
  m_log_protocol_stat_period = DEF_LOG_PROTOCOL_STAT_PERIOD ;
  m_log_protocol_stat_name  = DEF_LOG_PROTOCOL_STAT  ;
  m_log_protocol_stat_file = DEF_LOG_PROTOCOL_STAT_FILE ;
  m_display_protocol_stat  = DEF_DISPLAY_PROTOCOL_STAT   ;
  m_display_scenario_stat  = DEF_DISPLAY_SCENARIO_STAT   ;
  m_files_no_timestamp     = DEF_FILES_NO_TIMESTAMP ;
  m_call_timeout_beh_abr   = DEF_CALL_TIMEOUT_BEH_ABRT ;

  m_execute_check_action   = DEF_EXECUTE_CHECK_ACTION ;
  m_open_timeout           = DEF_OPEN_TIMEOUT     ;

  m_max_retrans        = DEF_MAX_RETRANS       ;
  m_retrans_enabled    = DEF_RETRANS_ENABLED   ;

  m_model_traffic_select = DEF_MODEL_TRAFFIC_SELECT ; // by default to be define

  m_call_rate_scale = DEF_CALL_RATE_SCALE ;

  m_option_remote_cmd = DEF_OPTION_REMOTE_CMD ;
  m_option_remote_dico_path = DEF_REMOTE_DICO_PATH ;

  ALLOC_TABLE(m_conf_opt_set, bool*, sizeof(bool), E_CFG_OPT_Number);
  for(L_i=0; L_i < E_CFG_OPT_Number; L_i++) {
    m_conf_opt_set[L_i] = false ;
  }
  
  L_result = 
    startAnalyzeCommandLine (P_argc, (char**) P_argv, &L_command_descr) ;

  if (L_result == -1) {
     GEN_ERROR(E_GEN_FATAL_ERROR, "Analyze command line error");
  }

}

C_GeneratorConfig::~C_GeneratorConfig() {

  m_option_log_file = NULL ;
  m_option_conf_file = NULL ;
  m_option_bg_mode = false ;
  m_option_remote_cmd  = NULL ;

  m_option_remote_dico_path = NULL ;

  if (!m_option_dico_file_list->empty()) {
    m_option_dico_file_list->erase(m_option_dico_file_list->begin(),
				  m_option_dico_file_list->end());

  }
  DELETE_VAR(m_option_dico_file_list);


  if (!m_configValueList->empty()) {
    m_configValueList->erase(m_configValueList->begin(),
			     m_configValueList->end());

  }
  DELETE_VAR(m_configValueList);


  FREE_TABLE(m_conf_opt_set);

} // destructor




bool          C_GeneratorConfig::checkConfig(){

  bool L_return = true ;

  return (L_return) ;
}

char* C_GeneratorConfig::get_conf_file() {
  return(m_option_conf_file) ;
}


char* C_GeneratorConfig::get_remote_cmd() {
  return(m_option_remote_cmd) ;
}

char* C_GeneratorConfig::get_remote_dico_path() {
  return(m_option_remote_dico_path) ;
}

char* C_GeneratorConfig::get_log_file() {
  return(m_option_log_file) ;
}

char* C_GeneratorConfig::get_scen_file() {
  return(m_option_scen_file) ;
}

T_pcharPlist  C_GeneratorConfig::get_dico_file_list() {
return(m_option_dico_file_list) ;
}


T_pConfigValueList C_GeneratorConfig::get_config_param_list() {
return(m_configValueList) ;
}


unsigned int C_GeneratorConfig::get_log_level() {
  return(m_option_log_level) ;
}

bool  C_GeneratorConfig::get_bg_mode () {
  return(m_option_bg_mode) ;
}

bool  C_GeneratorConfig::get_check_msg () {
  return(m_option_check_msg) ;
}

bool  C_GeneratorConfig::get_timestamp_log () {
  return(m_option_timestamp_log) ;
}


bool  C_GeneratorConfig::get_display_protocol_stat () {
  return(m_display_protocol_stat) ;
}

bool  C_GeneratorConfig::get_display_scenario_stat () {
  return(m_display_scenario_stat) ;
}

bool  C_GeneratorConfig::get_files_no_timestamp () {
  return(m_files_no_timestamp) ;
}

bool  C_GeneratorConfig::get_call_timeout_beh_abr () {
  return(m_call_timeout_beh_abr) ;
}

bool  C_GeneratorConfig::get_execute_check_action () {
  return (m_execute_check_action) ;
}

bool  C_GeneratorConfig::get_retrans_enabled () {
  return (m_retrans_enabled) ;
}


bool C_GeneratorConfig::set_value (T_GeneratorConfigOption P_opt, 
				   char *P_value) {
  bool         L_ret     = true ;
  char        *L_end_str = NULL ;
  unsigned int L_i              ;
  int          L_j              ;

  switch (P_opt) {
  case E_CFG_OPT_CALL_RATE :
    m_call_rate = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_BURST_LIMIT :
    m_burst_limit = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_MAX_SEND :
    m_max_send = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_MAX_RECEIVE :
    m_max_receive = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_SELECT_TIMEOUT_MS :
    m_select_timeout = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_MAX_SIMULTANEOUS_CALLS :
    m_max_simultaneous_calls = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_CALL_TIMEOUT :
    m_call_timeout = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_MSG_BUFFER_SIZE :
    m_msg_buffer_size = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_DISPLAY_PERIOD :
    m_display_period = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_LOG_STAT_PERIOD :
    m_log_stat_period = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_LOG_FILE :
    m_option_log_file = P_value ; 
    break ;

  case E_CFG_OPT_LOG_STAT_FILE :
    m_log_stat_file = P_value ;
    break ;

  case E_CFG_OPT_RESP_TIME_REPART:
    m_resp_time_repart = P_value ;
    break ;

  case E_CFG_OPT_NUMBER_CALLS:
    m_number_calls = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_CHECK_LEVEL:
    m_check_level_mask = 0 ;
    for (L_i=0; L_i < strlen(P_value); L_i++) {
      for (L_j = 0 ; L_j < E_NB_CHECK_LEVEL; L_j++) {
	if (P_value[L_i] == _check_level_char[L_j]) {
	  m_check_level_mask |= _check_level_mask[L_j] ;
	  break ;
	}
      }
    }
    break ;

  case E_CFG_OPT_CHECK_BEHAVIOUR:
    L_ret = false ;
    if (strlen(P_value) == 1) {
      for (L_j = 0 ; L_j < E_NB_CHECK_BEHAVIOUR; L_j++) {
	if (P_value[0] == _check_behaviour_char[L_j]) {
	  m_check_behaviour = L_j ;
	  L_ret = true ;
	  break ;
	}
      }
    }
    break ;

  case E_CFG_OPT_DATA_LOG_FILE:
    m_data_log_file = P_value ;
    break ;

  case E_CFG_OPT_DATA_LOG_PERIOD:
    m_data_log_period = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_DATA_LOG_NUMBER:
    m_data_log_number = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_CALL_RATE_SCALE :
    m_call_rate_scale = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_EXTERNAL_DATA_FILE :
    m_external_data_file = P_value ;
    break ;

  case E_CFG_OPT_EXTERNAL_DATA_SELECT :
    L_ret = false ;
    for (L_j = 0; L_j < E_NB_CHECK_DATA_SELECT ; L_j ++) {
      if(strcmp(P_value, _check_external_select[L_j]) == 0) {
	m_external_data_select = L_j ;
	L_ret = true ;
	break ;
      }
    }

      break ;


  case E_CFG_OPT_DATA_LOG_RTDISTRIB:

    m_data_log_rtdistrib = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_LOG_PROTOCOL_STAT_PERIOD:

    m_log_protocol_stat_period = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_LOG_PROTOCOL_STAT_NAME:
    m_log_protocol_stat_name = P_value ;
    break ;

  case E_CFG_OPT_LOG_PROTOCOL_STAT_FILE:
    m_log_protocol_stat_file = P_value ;
    break ;

  case E_CFG_OPT_DISPLAY_PROTOCOL_STAT:
    if(strcmp(P_value, (char *)"true") == 0) {
      m_display_protocol_stat = true ;
    } else {
      if(strcmp(P_value, (char *)"false") != 0) {
	L_ret = false ;
      }
    }

    break ;

  case E_CFG_OPT_DISPLAY_SCENARIO_STAT:

    if(strcmp(P_value, (char *)"true") == 0) {
      m_display_scenario_stat = true ;
    } else {
      if(strcmp(P_value, (char *)"false") != 0) {
	L_ret = false ;
      }
    }
    break ;

  case E_CFG_OPT_FILES_NO_TIMESTAMP:

    if(strcmp(P_value, (char *)"true") == 0) {
      m_files_no_timestamp = true ;
    } else {
      if(strcmp(P_value, (char *)"false") != 0) {
	L_ret = false ;
      }
    }
    break ;


  case E_CFG_OPT_CALL_TIMEOUT_BEH_ABR:

    if(strcmp(P_value, (char *)"true") == 0) {
      m_call_timeout_beh_abr = true ;
    } else {
      if(strcmp(P_value, (char *)"false") != 0) {
	L_ret = false ;
      }
    }
    break ;

  case E_CFG_OPT_OPEN_TIMEOUT :
    m_open_timeout = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_EXECUTE_CHECK_ACTION:

    if(strcmp(P_value, (char *)"false") == 0) {
      m_execute_check_action = false ;
    } else {
      if(strcmp(P_value, (char *)"true") != 0) {
	L_ret = false ;
      }
    }
    break ;


  case E_CFG_OPT_MAX_RETRANS :
    m_max_retrans = strtoul_f(P_value, &L_end_str, 10) ;
    if (L_end_str[0] != '\0') { // not a number
      L_ret = false ;
    }
    break ;

  case E_CFG_OPT_RETRANS_ENABLED:

    if(strcmp(P_value, (char *)"true") == 0) {
      m_retrans_enabled = true ;
    } else {
      if(strcmp(P_value, (char *)"false") != 0) {
	L_ret = false ;
      }
    }
    break ;

  case E_CFG_OPT_MODEL_TRAFFIC_SELECT :
    L_ret = false ;
    for (L_j = 0; L_j < E_NB_MODEL_TRAFFIC_SELECT ; L_j ++) {
      if(strcmp(P_value, _model_traffic_select[L_j]) == 0) {
	m_model_traffic_select = L_j ;
	L_ret = true ;
	break ;
      }
    }

      break ;


  default:
    L_ret = false ;
    break ;
  } // switch

  if (L_ret == true) {
    m_conf_opt_set[P_opt]=true ;
  }

  return (L_ret);

}

bool C_GeneratorConfig::is_value_set (T_GeneratorConfigOption  P_opt) {
  bool L_ret = false ;
  if (P_opt < E_CFG_OPT_Number) {
    L_ret = m_conf_opt_set[P_opt] ;
  }
  return (L_ret);
}

bool C_GeneratorConfig::get_value (T_GeneratorConfigOption  P_opt,
				   unsigned long           *P_val) {
  bool L_ret = true ;

  switch (P_opt) {
  case E_CFG_OPT_CALL_RATE :
    *P_val = m_call_rate ;
    break ;

  case E_CFG_OPT_BURST_LIMIT :
    *P_val = m_burst_limit ;
    break ;

  case E_CFG_OPT_MAX_SEND :
    *P_val = m_max_send ;
    break ;

  case E_CFG_OPT_MAX_RECEIVE :
    *P_val = m_max_receive ;
    break ;

  case E_CFG_OPT_SELECT_TIMEOUT_MS :
    *P_val = m_select_timeout ;
    break ;

  case E_CFG_OPT_MAX_SIMULTANEOUS_CALLS :
    *P_val = m_max_simultaneous_calls ;
    break ;

  case E_CFG_OPT_CALL_TIMEOUT :
    *P_val = m_call_timeout ;
    break ;

  case E_CFG_OPT_MSG_BUFFER_SIZE :
    *P_val = m_msg_buffer_size ;
    break ;

  case E_CFG_OPT_DISPLAY_PERIOD :
    *P_val = m_display_period ;
    break ;

  case E_CFG_OPT_LOG_STAT_PERIOD :
    *P_val = m_log_stat_period ;
    break ;

  case E_CFG_OPT_NUMBER_CALLS:
    *P_val = m_number_calls ;
    break ;

  case E_CFG_OPT_DATA_LOG_PERIOD:
    *P_val = m_data_log_period ;
    break ;

  case E_CFG_OPT_DATA_LOG_NUMBER:
    *P_val = m_data_log_number ;
    break ;

  case E_CFG_OPT_CALL_RATE_SCALE :
    *P_val = m_call_rate_scale ;
    break ;

  case E_CFG_OPT_DATA_LOG_RTDISTRIB:

    *P_val = m_data_log_rtdistrib ;
    break ;

  case E_CFG_OPT_LOG_PROTOCOL_STAT_PERIOD :
    *P_val = m_log_protocol_stat_period ;
    break ;

  case E_CFG_OPT_OPEN_TIMEOUT :
    *P_val = m_open_timeout ;
    break ;

  case E_CFG_OPT_MAX_RETRANS :
    *P_val = m_max_retrans ;
    break ;


  default:
    L_ret = false ;
    break ;
  } // switch

  return (L_ret) ;

}

bool C_GeneratorConfig::get_value (T_GeneratorConfigOption  P_opt,
				   unsigned int            *P_val) {
  bool L_ret = true ;

  switch (P_opt) {
  case E_CFG_OPT_CHECK_LEVEL:
    *P_val = m_check_level_mask ;
    break ;

  case E_CFG_OPT_CHECK_BEHAVIOUR:
    *P_val = m_check_behaviour ;
    break ;

  case E_CFG_OPT_EXTERNAL_DATA_SELECT :
    *P_val = m_external_data_select ;
    break ;

  case E_CFG_OPT_MODEL_TRAFFIC_SELECT :
    *P_val = m_model_traffic_select ;
    break ;

  default:
    L_ret = false ;
    break ;
  } // switch

  return (L_ret) ;

}

bool C_GeneratorConfig::get_value (T_GeneratorConfigOption  P_opt,
				    char                    **P_val) {

  bool L_ret = true ;

  switch (P_opt) {

  case E_CFG_OPT_LOG_FILE :
    *P_val = m_option_log_file ;
    break ;

  case E_CFG_OPT_LOG_STAT_FILE :
    *P_val = m_log_stat_file ;
    break ;

  case E_CFG_OPT_RESP_TIME_REPART :
    *P_val = m_resp_time_repart ;
    break ;

  case E_CFG_OPT_EXTERNAL_DATA_FILE :
    *P_val = m_external_data_file ;
    break ;

  case E_CFG_OPT_LOG_PROTOCOL_STAT_NAME :
    *P_val = m_log_protocol_stat_name ;
    break ;

  default:
    L_ret = false ;
    break ;
  } // switch

  return (L_ret) ;

}

bool C_GeneratorConfig::get_value (T_GeneratorConfigOption  P_opt,
				   string_t&             P_val) {

  bool L_ret = true ;

  switch (P_opt) {

  case E_CFG_OPT_LOG_FILE :
    P_val = "" ;
    if (m_option_log_file != NULL) {
      P_val.append(m_option_log_file);
    }
    break ;

  case E_CFG_OPT_LOG_STAT_FILE :
    P_val = "" ;
    if (m_log_stat_file != NULL) {
      P_val.append(m_log_stat_file);
    }
    break ;

  case E_CFG_OPT_RESP_TIME_REPART :
    P_val = "" ;
    if (m_resp_time_repart != NULL) {
      P_val.append(m_resp_time_repart);
    }
    break ;

  case E_CFG_OPT_DATA_LOG_FILE :
    P_val = "" ;
    if (m_data_log_file != NULL) {
      P_val.append(m_data_log_file);
    }
    break ;

  case E_CFG_OPT_LOG_PROTOCOL_STAT_FILE :

    P_val = "" ;
    if (m_log_protocol_stat_file != NULL) {
      P_val.append(m_log_protocol_stat_file);
    }
    break ;

  default:
    L_ret = false ;
    break ;
  } // switch

  return (L_ret) ;

}

T_GeneratorConfigOption C_GeneratorConfig::get_code (char* P_name) {
  T_GeneratorConfigOption L_ret = E_CFG_OPT_Number ;
  int                     L_i ;

  for (L_i = 0 ; L_i < E_CFG_OPT_Number; L_i++) {
    if (strcmp(P_name, config_opt_table[L_i])==0) {
      L_ret = (T_GeneratorConfigOption) L_i ; 
      break ;
    }
  }
  return (L_ret) ;
}

bool C_GeneratorConfig::fromXml (C_XmlData          *P_data) {

  bool                      L_ret = true ;
  T_pXmlData_List           L_sectionList, L_subSectionList            ;
  T_XmlData_List::iterator  L_sectionListIt, L_subSectionListIt ;
  C_XmlData                *L_section, *L_subSection ;
  char                     *L_entity_value ;
  char                     *L_param_name, *L_param_value ;
  T_GeneratorConfigOption   L_param_code ;
  bool                      L_decode_param ;

  T_ConfigValue             L_configValue     ;
  
  if (P_data != NULL) {

    if ((L_sectionList = P_data->get_sub_data()) != NULL) {
      
      for(L_sectionListIt  = L_sectionList->begin() ;
	  L_sectionListIt != L_sectionList->end() ;
	  L_sectionListIt++) {
	
	L_section = *L_sectionListIt ;
	if (L_section != NULL) {
	  if (strcmp(L_section->get_name(), 
		     XML_CONFIG_SECTION) == 0) {

	    L_subSectionList = L_section->get_sub_data() ;
	    for(L_subSectionListIt  = L_subSectionList->begin() ;
		L_subSectionListIt != L_subSectionList->end() ;
		L_subSectionListIt++) {
	      L_subSection = *L_subSectionListIt ;
	      
	      if (strcmp(L_subSection->get_name(), 
			 XML_CONFIG_SUBSECTION) == 0) {

		L_entity_value = L_subSection->find_value(XML_CONFIG_ENTITY) ;
		if (L_entity_value != NULL) {

		  if (strcmp(L_entity_value, XML_CONFIG_ENTITY_VALUE) == 0) {
		    L_param_name = L_subSection->find_value(XML_CONFIG_NAME);
		    L_param_value = L_subSection->find_value(XML_CONFIG_VALUE);

		    L_param_code = get_code(L_param_name);

		    if (L_param_code != E_CFG_OPT_Number) {
		      L_decode_param = set_value (L_param_code, L_param_value) ;

		      if (L_decode_param == false) {
			GEN_ERROR(E_GEN_FATAL_ERROR, "Format incorrect for value ["
				  << L_param_value << "] for parameter ["
				  << L_param_name << "]");
			L_ret = false ;
			break ;
		      }
		    } else {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown configuration parameter ["
				<< L_param_name << "]");
		      L_ret = false ;
		      break ;
		    }
		  }
		   if (strcmp(L_entity_value, XML_CONFIG_PARAM_ENTITY_VALUE) == 0) {
		    L_configValue.m_name = L_subSection->find_value(XML_CONFIG_NAME);
		    L_configValue.m_value = L_subSection->find_value(XML_CONFIG_VALUE);
		    m_configValueList->push_back(L_configValue);
		   }

		} 
	      }
	    }
	  }
	} 
      }

    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No Xml data");
      L_ret = false ;
    }
  } else { // P_data == NULL
    GEN_ERROR(E_GEN_FATAL_ERROR, "No Xml data");
    L_ret = false ;
  }


  return (L_ret);
}

iostream_output& operator<< (iostream_output& P_ostream, 
			     C_GeneratorConfig & P_conf) {

  const char  novalue[]    = "none"    ;
  const char  truevalue[]  = "true"    ;
  const char  falsevalue[] = "false"   ;
  const char  warnvalue[]  = "warning" ;
  const char  errvalue[]   = "error"   ;

  const char  seqselect[]   = "sequential"   ;
  const char  ranselect[]   = "random"   ;

  char       *L_value                  ;
  string_t    logstring    = ""        ;
  string_t    checkstring  = ""        ;
  int         L_j                      ;
  T_charPlist::iterator     L_iterator ;

  P_ostream << "Configuration data:" << iostream_endl ;


  L_value = (P_conf.m_option_log_file == NULL) 
    ? (char*) novalue : P_conf.m_option_log_file ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "option_log_file  ["
	    <<  L_value
	    << "]" << iostream_endl ;

  L_value = (P_conf.m_option_conf_file == NULL) 
    ? (char*) novalue : P_conf.m_option_conf_file ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "option_conf_file ["
	    << L_value 
	    << "]" << iostream_endl ;

  if (P_conf.m_option_dico_file_list->empty()) {
    L_value = (char*) novalue ; 
    P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "option_dico_file ["
	      << L_value 
	      << "]" << iostream_endl ;
  } else {
    for (L_iterator = P_conf.m_option_dico_file_list->begin();
	 L_iterator != P_conf.m_option_dico_file_list->end();
	 L_iterator++) {
      P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "option_dico_file ["
		<< *L_iterator
		<< "]" << iostream_endl ;
    }
  }
  
  L_value = (P_conf.m_option_scen_file == NULL) 
    ? (char*)novalue : P_conf.m_option_scen_file ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "option_scen_file ["
	    << L_value 
	    << "]" << iostream_endl ;

  L_value = (P_conf.m_log_stat_file == NULL) 
    ? (char*)novalue : P_conf.m_log_stat_file ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "log_stat_file    ["
	    << L_value 
	    << "]" << iostream_endl ;

  if (P_conf.m_option_log_level == gen_mask_table[LOG_LEVEL_ALL]) {
    logstring += "A" ;
  } else {
    if (P_conf.m_option_log_level & gen_mask_table[LOG_LEVEL_ERR]) {
      logstring += "E" ;
    }
    if (P_conf.m_option_log_level & gen_mask_table[LOG_LEVEL_WAR]) {
      logstring += "W" ;
    }
    if (P_conf.m_option_log_level & gen_mask_table[LOG_LEVEL_MSG]) {
      logstring += "M" ;
    }
    if (P_conf.m_option_log_level & gen_mask_table[LOG_LEVEL_TRAFFIC_ERR]) {
      logstring += "T" ;
    }
    if (P_conf.m_option_log_level & gen_mask_table[LOG_LEVEL_BUF]) {
      logstring += "B" ;
    }
  }
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "option_log_level ["
	    << logstring << "]" << iostream_endl ;

  L_value = (P_conf.m_option_bg_mode == true) 
    ? (char*)truevalue : (char*)falsevalue ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "option_bg_mode   ["
	    << L_value << "]" << iostream_endl ;

  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "call_rate        ["
	    << P_conf.m_call_rate << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "burst_limit      ["
	    << P_conf.m_burst_limit << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "max_send         ["
	    << P_conf.m_max_send << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "max_receive      ["
	    << P_conf.m_max_receive << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "select_timeout   ["
	    << P_conf.m_select_timeout << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "max_simult_calls ["
	    << P_conf.m_max_simultaneous_calls << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "call_timeout_ms  ["
	    << P_conf.m_call_timeout << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "msg_buffer_size  ["
	    << P_conf.m_msg_buffer_size << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "display_period   ["
	    << P_conf.m_display_period << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "log_stat_period  ["
	    << P_conf.m_log_stat_period << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "resp_time_repart ["
	    << P_conf.m_resp_time_repart << "]" << iostream_endl ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "number_calls     ["
	    << P_conf.m_number_calls << "]" << iostream_endl ;

  L_value = (P_conf.m_option_timestamp_log == true) ? 
    (char*)falsevalue : (char*)truevalue ;

  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "notimelog        ["
	    << L_value << "]" << iostream_endl ;

  L_value = (P_conf.m_option_check_msg == false) ? 
    (char*)falsevalue : (char*)truevalue ;

  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "msg check        ["
	    << L_value << "]" << iostream_endl ;

  for (L_j = 0; L_j < E_NB_CHECK_LEVEL; L_j++) {
    if (P_conf.m_check_level_mask & _check_level_mask[L_j]) {
      checkstring += _check_level_char[L_j] ;
    }
  }
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "msg check mask   ["
	    << checkstring << "]" << iostream_endl ;

  L_value = (char*)novalue ;
  if (P_conf.m_check_behaviour == E_CHECK_BEHAVIOUR_WARNING) {
    L_value = (char*)warnvalue ;
  }
  if (P_conf.m_check_behaviour == E_CHECK_BEHAVIOUR_ERROR) {
    L_value = (char*)errvalue ;
  }

  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "check behaviour  ["
	    << L_value << "]" << iostream_endl ;

  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "call rate scale  ["
	    << P_conf.m_call_rate_scale << "]" << iostream_endl ;

  L_value = (P_conf.m_external_data_file == NULL) 
    ? (char*)novalue : P_conf.m_external_data_file ;
  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "external data file    ["
	    << L_value 
	    << "]" << iostream_endl ;

  L_value = (char*)novalue ;
  if (P_conf.m_external_data_select == 0) {
    L_value = (char*)seqselect ;
  }
  if (P_conf.m_external_data_select == 1) {
    L_value = (char*)ranselect ;
  }

  P_ostream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "external data select  ["
	    << L_value << "]" ;


  return (P_ostream) ;
}

int         C_GeneratorConfig::analyze_protocol_stat(char          *P_logProtocolStat,
						     T_charPlist   *P_protocol_list,
						     bool           *P_all_found) {
  regex_t                 *L_regExpr = NULL   ;
  regmatch_t               L_pmatch           ;

  char                    *L_search  = NULL   ;
  char                     L_buffer[100]      ;
  char                    *L_search_begin     ;
  char                    *L_search_end       ;
  char                    *L_string  = NULL   ;

  int                      L_status           ;
  int                      L_size             ;
  int                      L_ret      = 0     ;

  int                      L_result   = 0     ;

  ALLOC_VAR(L_regExpr, regex_t*, sizeof(regex_t));

  L_status = regcomp (L_regExpr, "[^;][^;]*[;]*", REG_EXTENDED) ;

  if (L_status != 0) {
    regerror(L_status, L_regExpr, L_buffer, 100);
    regfree (L_regExpr) ;
    FREE_VAR(L_regExpr);
    GEN_ERROR(E_GEN_FATAL_ERROR, "regcomp error: [" << L_buffer << "]");
    L_result = -1 ;
  } else {
    L_search = P_logProtocolStat ;
    do {
      L_status = regexec (L_regExpr, L_search, 1, &L_pmatch, 0) ;
      
      // matching is OK
      if (L_status == 0) {
	L_ret  = L_pmatch.rm_eo ;
	L_size = (L_pmatch.rm_eo - L_pmatch.rm_so) ;
	
	L_search_begin = L_search ;
	L_search_end = L_search + L_size - 1 ;
	
	while (L_search_end[0] == ';') L_search_end--;
	while (L_search_end[0] == ' ') L_search_end--;

	L_size = L_search_end - L_search + 1 ;

	while(isspace(*L_search_begin)) {
	  L_search_begin++;
	  L_size--;
	}

	if (L_size > 0) {
	  ALLOC_TABLE(L_string, char*, sizeof(char), L_size+1);
	  snprintf(L_string, L_size+1, "%s", L_search_begin);
	  L_string[L_size+1] = 0 ;
	  if (strcmp(L_string, (char*)"all") == 0) {
	    (*P_all_found) = true ;	    
	    break;
	  }
	  P_protocol_list ->push_back(L_string) ;
	}
 
	L_search += L_ret ;	

	if (L_search[0] == '\0') break ;
	
      } else {
	L_result = -1 ;
      }
    } while (L_status == 0) ;

    if (L_regExpr) {
      regfree (L_regExpr) ;
      FREE_VAR(L_regExpr);
    }
  }
  
  return (L_result) ;
}


// End of file
