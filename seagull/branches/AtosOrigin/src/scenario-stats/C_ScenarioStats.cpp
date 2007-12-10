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

#include "C_ScenarioStats.hpp"
#include "C_Scenario.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"


#define max(a,b) ((a)>(b)) ? (a) : (b)

#define DISPLAY_CROSS_LINE()\
        printf("|--------------------------+-------------------------+-------------------------|\r\n")

#define DISPLAY_COMMAND(T) \
	printf("| %-24s |                                                   |\r\n", (T))
        
#define DISPLAY_FIRST_LINE(T1,T2,T3,T4,T5) \
	printf("| %-24s | %-10s | %-10s | %-10s | %-10s |\r\n", (T1), (T2), (T3), (T4), (T5))

#define DISPLAY_COUNTERS(T,V1,V2,V3,V4) \
	printf("| %-24s | %10d | %10d | %10d | %10d |\r\n", (T), (V1), (V2), (V3), (V4))


#define DISPLAY_3TXT(T1, T2, T3)\
	printf("| %-24s | %23s | %23s |\r\n", (T1), (T2), (T3))

#define MAX_CHAR_BUFFER_SIZE          256
#define MAX_DATA_BUFFER_SIZE          4096
static const char* COMMAND_SEPARATOR = "#" ;


C_ScenarioStats::C_ScenarioStats(C_Scenario *P_scen) {

  int              L_i ;
  char            *L_name ;

  // extract ethe end of scenario
  m_scenario = P_scen ;

  if (m_scenario->m_sequence_max > 1) {
    m_nb_command =  m_scenario->m_sequence_max - 1;
    
    
    ALLOC_TABLE(m_counter_cmd_table,
                T_pCommandStat,
                sizeof(T_CommandStat),
                m_nb_command);
    
    for (L_i = 0 ; L_i < m_nb_command ; L_i++) {
      m_counter_cmd_table[L_i].m_name = NULL ;
      ALLOC_TABLE(L_name, char*, sizeof(char), 25);
      L_name[0] = '\0' ;
      
      
      switch (m_scenario->m_cmd_sequence[L_i].m_type) {
      case E_CMD_SCEN_SEND:
        snprintf(L_name, 25, "%20s -->", 
                 m_scenario->m_cmd_sequence[L_i].m_message->name());
      case E_CMD_SCEN_RECEIVE:
        if (L_name[0] == '\0') {
          snprintf(L_name, 25, "%20s <--",
                   m_scenario->m_cmd_sequence[L_i].m_message->name());
        }
        NEW_TABLE (m_counter_cmd_table[L_i].m_counters, 
                   C_CounterData, (size_t)E_NB_ACTIONS);
        m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE]
          .m_counter_value = 0;
        m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS]
          .m_counter_value = 0;
        m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT]
          .m_counter_value = 0;
        m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED]
          .m_counter_value = 0;
        break ;
      case E_CMD_SCEN_WAIT:
        snprintf(L_name, 25, "           [%8ld ms]", 
                 m_scenario->m_cmd_sequence[L_i].m_duration);
        m_counter_cmd_table[L_i].m_counters = NULL ;
        break ;
      default:
        break ;
      }
      m_counter_cmd_table[L_i].m_name = L_name ;
      
    }
    
    m_current_scen_stat_id = -1 ;
    m_nb_stat_scen_id = 1 ;
    ALLOC_TABLE(m_current_scen_stat_table, 
                T_DisplayScenFct*, 
                sizeof(T_DisplayScenFct), 2);
    m_current_scen_stat_table[0] = &C_ScenarioStats::display_stats_command ;
    m_current_scen_stat_display = m_current_scen_stat_table[0];
    
    m_scenario -> set_stats (this);
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, 
              "Bad scenario definition");

  }
    
}

C_ScenarioStats::~C_ScenarioStats(){
  int L_i ;

  m_scenario = NULL ;
  for (L_i = 0 ; L_i < m_nb_command; L_i++) {
    FREE_TABLE(m_counter_cmd_table[L_i].m_name);
    DELETE_TABLE(m_counter_cmd_table[L_i].m_counters);
  }
  FREE_TABLE(m_counter_cmd_table);
  m_nb_command = 0 ;
  FREE_TABLE(m_current_scen_stat_table);
}


void C_ScenarioStats::updateStats (int P_cmd_index,
  				   T_ScenarioStatDataAction P_action,
  				   int P_sub_msg_index) {
  
  switch (P_action) {
  case E_MESSAGE :
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.P();
    (m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_counter_value)++ ;
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.V();
    break ;
  case E_RETRANS :
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.P();
    (m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_counter_value)++ ;
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.V();
    break ;
  case E_TIMEOUT :
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.P();
    (m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_counter_value)++ ;
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.V();
    break ;
    
  case E_UNEXPECTED :
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.P();
    (m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_counter_value)++ ;
    m_counter_cmd_table[P_cmd_index].m_counters[P_action].m_sem.V();
    break ;
    
  default:
    break ;
    
  }
}

void C_ScenarioStats::setCurrentScreen(bool P_first) {


  if (P_first == true) {
    m_current_scen_stat_id = 0 ;
  } else {

    m_current_scen_stat_id ++ ;
    if (m_current_scen_stat_id == m_nb_stat_scen_id) {
      m_current_scen_stat_id = 0 ;
    }
  }
  m_current_scen_stat_display 
    = m_current_scen_stat_table[m_current_scen_stat_id];
}

void C_ScenarioStats::displayScreen() {

  ((this)->*(m_current_scen_stat_display))();

}


void C_ScenarioStats::display_stats_command() {

  int L_i ;
  T_CounterType L_message, L_timeout, L_unexpected, L_retrans ;

  DISPLAY_FIRST_LINE("","Messages","Retrans","Timeout","Unexp.");

  for (L_i = 0 ; L_i < m_nb_command ; L_i++) {

    if (m_counter_cmd_table[L_i].m_counters) {
      m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_sem.P();
      L_message = m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_sem.P();
      L_timeout = m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_sem.P();
      L_unexpected = m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_sem.P();
      L_retrans = m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_sem.V();
      
      DISPLAY_COUNTERS(m_counter_cmd_table[L_i].m_name,
		      L_message,
		      L_retrans,
		      L_timeout,
		      L_unexpected);
    } else {
      DISPLAY_COMMAND(m_counter_cmd_table[L_i].m_name);
    }

  }

  if (m_nb_command < 21) {
    for (L_i = m_nb_command ; L_i < 21 ; L_i ++) {
      DISPLAY_3TXT("","","");
    }
  }
  // DISPLAY_3TXT("","","");
  // DISPLAY_3TXT("","","");


  DISPLAY_CROSS_LINE ();
  
}



void C_ScenarioStats::reset_cumul_counters() {
  int L_i ;

  for (L_i = 0 ; L_i < m_nb_command ; L_i++) {

    if (m_counter_cmd_table[L_i].m_counters) {
      m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_sem.P();
      m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_counter_value = 0 ;
      m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_sem.P();
      m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_counter_value = 0 ;
      m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_sem.P();
      m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_counter_value = 0 ;
      m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_sem.P();
      m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_counter_value = 0;
      m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_sem.V();
    }
  } 
}

char* C_ScenarioStats::get_scenario_structure() {
  char* L_result_data = NULL; 
  char  L_buffer[MAX_CHAR_BUFFER_SIZE];  

  ALLOC_TABLE(L_result_data, char*, sizeof(char), MAX_DATA_BUFFER_SIZE);
  sprintf(L_result_data, "%s", (char*)"Start");
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  sprintf(L_buffer, "%s", (char*)"5"); 
  strcat(L_result_data, L_buffer);
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  sprintf(L_buffer, "%s", (char*)" "); 
  strcat(L_result_data, L_buffer);
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  sprintf(L_buffer, "%s", (char*)"Messages"); 
  strcat(L_result_data, L_buffer);
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  sprintf(L_buffer, "%s", (char*)"Retrans"); 
  strcat(L_result_data, L_buffer);
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  sprintf(L_buffer, "%s", (char*)"Timeout"); 
  strcat(L_result_data, L_buffer);
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  sprintf(L_buffer, "%s", (char*)"Unexp."); 
  strcat(L_result_data, L_buffer);
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  sprintf(L_buffer, "%d", m_nb_command); 
  strcat(L_result_data, L_buffer);
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  for (int L_i = 0 ; L_i < m_nb_command ; L_i++) {
    sprintf(L_buffer, "%s", m_counter_cmd_table[L_i].m_name); 
    strcat(L_result_data, L_buffer);
    strcat(L_result_data, COMMAND_SEPARATOR);
  }   
  return (L_result_data);
} 

char* C_ScenarioStats::get_scenario_data() {
  char* L_result_data = NULL; 
  char  L_buffer[MAX_CHAR_BUFFER_SIZE];  

  ALLOC_TABLE(L_result_data, char*, sizeof(char), MAX_DATA_BUFFER_SIZE);
  sprintf(L_result_data, "%s", (char*)"Start");
  strcat(L_result_data, COMMAND_SEPARATOR);
  
  T_CounterType L_message, L_timeout, L_unexpected, L_retrans ;
  
  for (int L_i = 0 ; L_i < m_nb_command ; L_i++) {
    if (m_counter_cmd_table[L_i].m_counters) {
      m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_sem.P();
      L_message = m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_MESSAGE].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_sem.P();
      L_timeout = m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_TIMEOUT].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_sem.P();
      L_unexpected = m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_UNEXPECTED].m_sem.V();
      
      m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_sem.P();
      L_retrans = m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_counter_value;
      m_counter_cmd_table[L_i].m_counters[(int)E_RETRANS].m_sem.V();	
      
      sprintf(L_buffer, "%d", L_message); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
      
      sprintf(L_buffer, "%d", L_retrans); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
      
      sprintf(L_buffer, "%d", L_timeout); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
      
      sprintf(L_buffer, "%d", L_unexpected); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
      
    } else {
      sprintf(L_buffer, "%s", (char*)" "); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
      sprintf(L_buffer, "%s", (char*)" "); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
      sprintf(L_buffer, "%s", (char*)" "); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
      sprintf(L_buffer, "%s", (char*)" "); 
      strcat(L_result_data, L_buffer);
      strcat(L_result_data, COMMAND_SEPARATOR);
    }
    
  }
  return (L_result_data); 
}

