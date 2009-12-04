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

#ifndef _C_GENERATOR_CMDLINE_H
#define _C_GENERATOR_CMDLINE_H

#include "Utils.hpp"
#include "C_CommandLine.hpp"
#include "C_XmlData.hpp"

#include "T_ConfigValue.h"

#include "iostream_t.hpp"
#include "string_t.hpp"

#include "message_check_t.h"

#include "list_t.hpp"
#include <regex.h>

typedef list_t<char*> T_charPlist, *T_pcharPlist ;


typedef enum {
  E_CHECK_LEVEL_FIELD_PRESENCE,
  E_CHECK_LEVEL_FIELD_ADDED,
  E_NB_CHECK_LEVEL
} T_CheckLevel ;

typedef enum {
  E_CHECK_DATA_SEQUENTIAL = 0 ,
  E_CHECK_DATA_RANDOM,
  E_NB_CHECK_DATA_SELECT
} T_CheckDataSelect ;

typedef enum {
  E_MODEL_TRAFFIC_UNIFORM = 0 ,
  E_MODEL_TRAFFIC_BESTEFFORT,
  E_MODEL_TRAFFIC_POISSON,
  E_NB_MODEL_TRAFFIC_SELECT
} T_ModelTrafficSelect ;

extern const unsigned int _check_level_mask [] ;
extern const unsigned int _check_behaviour_mask [] ;
extern const char*        _check_external_select [] ;
extern const char*        _model_traffic_select [] ;



typedef enum {
  E_CMDLINE_conf_file = 0,
  E_CMDLINE_scen_file,
  E_CMDLINE_dico_file,
  E_CMDLINE_log_file,
  E_CMDLINE_log_level,
  E_CMDLINE_help,
  E_CMDLINE_bg_mode,
  E_CMDLINE_timestamp_log,
  E_CMDLINE_check_msg,
  E_CMDLINE_remote_cmd,
  E_CMDLINE_remote_dico_path,

  E_CMDLINE_nbOptions
} T_GeneratorCmdLineOption ;

typedef enum {
  E_CFG_OPT_CALL_RATE = 0 ,
  E_CFG_OPT_BURST_LIMIT,
  E_CFG_OPT_MAX_SEND,
  E_CFG_OPT_MAX_RECEIVE,
  E_CFG_OPT_SELECT_TIMEOUT_MS,
  E_CFG_OPT_MAX_SIMULTANEOUS_CALLS,
  E_CFG_OPT_CALL_TIMEOUT,
  E_CFG_OPT_MSG_BUFFER_SIZE,
  E_CFG_OPT_DISPLAY_PERIOD,
  E_CFG_OPT_LOG_STAT_PERIOD,
  E_CFG_OPT_LOG_FILE,
  E_CFG_OPT_LOG_STAT_FILE,
  E_CFG_OPT_RESP_TIME_REPART,
  E_CFG_OPT_NUMBER_CALLS,
  E_CFG_OPT_CHECK_LEVEL,
  E_CFG_OPT_CHECK_BEHAVIOUR,
  E_CFG_OPT_DATA_LOG_FILE,
  E_CFG_OPT_DATA_LOG_PERIOD,
  E_CFG_OPT_DATA_LOG_NUMBER,
  E_CFG_OPT_CALL_RATE_SCALE,
  E_CFG_OPT_EXTERNAL_DATA_FILE,
  E_CFG_OPT_EXTERNAL_DATA_SELECT,
  E_CFG_OPT_DATA_LOG_RTDISTRIB,
  E_CFG_OPT_LOG_PROTOCOL_STAT_PERIOD,
  E_CFG_OPT_LOG_PROTOCOL_STAT_NAME,
  E_CFG_OPT_LOG_PROTOCOL_STAT_FILE,
  E_CFG_OPT_DISPLAY_PROTOCOL_STAT,
  E_CFG_OPT_DISPLAY_SCENARIO_STAT,
  E_CFG_OPT_FILES_NO_TIMESTAMP,
  E_CFG_OPT_CALL_TIMEOUT_BEH_ABR,
  E_CFG_OPT_OPEN_TIMEOUT,
  E_CFG_OPT_EXECUTE_CHECK_ACTION,
  E_CFG_OPT_MAX_RETRANS,
  E_CFG_OPT_RETRANS_ENABLED,
  E_CFG_OPT_MODEL_TRAFFIC_SELECT,
  E_CFG_OPT_RECONNECT_LAG,
  
  E_CFG_OPT_Number
} T_GeneratorConfigOption ;

class C_GeneratorConfig:public C_CommandLine<T_GeneratorCmdLineOption> {

 public:

  C_GeneratorConfig(int P_argc, char** P_argv);
  virtual ~C_GeneratorConfig();

  bool set_data (T_GeneratorCmdLineOption P_opt, char **P_argtab) ;

  bool set_value (T_GeneratorConfigOption P_opt, char *P_value);

  bool get_value (T_GeneratorConfigOption P_opt, unsigned long *P_val);
  bool get_value (T_GeneratorConfigOption P_opt, unsigned int *P_val);
  bool get_value (T_GeneratorConfigOption P_opt, char **P_val);
  bool get_value (T_GeneratorConfigOption P_opt, string_t &P_val);
  bool is_value_set (T_GeneratorConfigOption P_opt) ;

  T_GeneratorConfigOption get_code (char* P_name) ;

  friend iostream_output&  operator<< (iostream_output&, C_GeneratorConfig&);

  bool         checkConfig();

  bool fromXml (C_XmlData *P_data);



  char*        get_conf_file();
  char*        get_log_file();
  char*        get_scen_file();


  T_pcharPlist get_dico_file_list() ;

  unsigned int get_log_level();
  bool         get_bg_mode ();
  bool         get_timestamp_log ();
  bool         get_check_msg () ;

  char*        get_remote_cmd() ;

  char*        get_remote_dico_path() ;


  bool         get_display_protocol_stat () ;
  bool         get_display_scenario_stat () ;


  bool         get_files_no_timestamp () ;
  bool         get_call_timeout_beh_abr () ;

  bool         get_execute_check_action () ;

  bool         get_retrans_enabled () ;


  T_pConfigValueList get_config_param_list() ;
  int         analyze_protocol_stat(char          *P_logProtocolStat,
				    T_charPlist   *P_protocol_list,
				    bool          *P_all_found);

private:

  // command line option
  char        *m_option_log_file      ;
  char        *m_option_conf_file     ;

  T_pcharPlist    m_option_dico_file_list  ;

  char        *m_option_scen_file     ;
  unsigned int m_option_log_level     ;
  bool         m_option_bg_mode       ;
  bool         m_option_timestamp_log ;
  bool         m_option_check_msg     ;

  char        *m_option_remote_cmd    ;
  char        *m_option_remote_dico_path    ;
  

  bool            *m_conf_opt_set ; // config option setted
  // config options
  unsigned long  m_call_rate ;
  unsigned long  m_burst_limit ;
  unsigned long  m_max_send ;
  unsigned long  m_max_receive ;
  unsigned long  m_select_timeout ;
  unsigned long  m_max_simultaneous_calls ;
  unsigned long  m_call_timeout ;
  unsigned long  m_msg_buffer_size ;
  unsigned long  m_display_period ;
  unsigned long  m_log_stat_period ;
  char          *m_log_stat_file ;
  char          *m_resp_time_repart ;
  unsigned long  m_number_calls ;


  char          *m_external_data_file ;
  unsigned int   m_external_data_select ;

  unsigned int   m_model_traffic_select ;


  unsigned int   m_check_level_mask ;
  unsigned int   m_check_behaviour  ;
  
  char          *m_data_log_file   ;
  unsigned long  m_data_log_number ;
  unsigned long  m_data_log_period ;


  unsigned long  m_data_log_rtdistrib        ;
  unsigned long  m_log_protocol_stat_period  ;
  char          *m_log_protocol_stat_name    ; 
  char          *m_log_protocol_stat_file    ;
  bool           m_display_protocol_stat     ;
  bool           m_display_scenario_stat     ;

  bool           m_files_no_timestamp        ;
  bool           m_call_timeout_beh_abr      ;

  bool           m_execute_check_action      ;

  unsigned long  m_open_timeout              ;

  unsigned long  m_max_retrans           ;
  bool           m_retrans_enabled       ;

  unsigned long  m_call_rate_scale           ;
  unsigned long  m_reconnect_lag	;

  T_pConfigValueList         m_configValueList ;

}; // class C_GeneratorConfig

#endif // _C_GENERATOR_CMDLINE_H







