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

#ifndef _C_SCENARIO_STATS_H
#define _C_SCENARIO_STATS_H

#include "iostream_t.hpp"
#include "Utils.hpp"

#include "C_Semaphore.hpp"
#include "C_DisplayObject.hpp"

class C_Scenario;

class C_ScenarioStats : public C_DisplayObject {

public:

  C_ScenarioStats(C_Scenario *P_scen) ;
  virtual ~C_ScenarioStats();

  typedef enum _scenario_stat_data_action {
    E_MESSAGE,
    E_RETRANS,
    E_TIMEOUT,
    E_UNEXPECTED,
    E_NB_ACTIONS
  } T_ScenarioStatDataAction, *T_pScenarioStatDataAction ;


  void updateStats (int P_cmd_index, 
  		    T_ScenarioStatDataAction P_action, 
		    int P_sub_msg_index);

  //  iostream_output& display (iostream_output& P_stream);

  void display_stats_command ();
  // void display_stats_command_sub_message ();

  void displayScreen() ;
  void setCurrentScreen(bool P_first) ;

  void reset_cumul_counters () ;
  char * get_scenario_structure();
  char * get_scenario_data();



protected:
private:

  typedef int T_CounterType, *T_pCounterType ;
  
  class C_CounterData {
  public:
    T_CounterType m_counter_value ;
    C_Semaphore   m_sem           ;
  } ;
  
  typedef struct _command_stat {
    char          *m_name ;
    C_CounterData *m_counters;
  } T_CommandStat, *T_pCommandStat ; 

  typedef C_CounterData *C_pCounterData ;


  C_Scenario            *m_scenario     ;

  T_pCommandStat  m_counter_cmd_table ;

  typedef void (C_ScenarioStats::* T_DisplayScenFct)() ;

  int                   m_current_scen_stat_id      ;
  int                   m_nb_stat_scen_id           ;
  T_DisplayScenFct     *m_current_scen_stat_table    ;
  T_DisplayScenFct      m_current_scen_stat_display  ;
  
  int                   m_nb_command                 ;

} ;

#endif // _C_SCENARIO_STATS_H



















