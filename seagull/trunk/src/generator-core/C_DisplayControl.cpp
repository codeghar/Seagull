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

#include "C_DisplayControl.hpp"

#include "Utils.hpp"
#include "GeneratorTrace.hpp"

#include <cstdlib>
#include <pthread.h> // for sched_yield()

#define MAX_KEY 255

#define DISPLAY_LINE()\
        printf("|-----------------------------------------------------------------------------|\r\n")


#define DISPLAY_LINE_EMPTY()\
        printf("|                                                                             |\r\n")

#define DISPLAY_3TXT(T1, T2, T3)\
	printf("| %-22s | %25s | %23s |\r\n", (T1), (T2), (T3))


#define DISPLAY_INFO(T1)\
	printf("| %-76.76s|\r\n", T1)

#define DISPLAY_NEXT()\
        printf("|--- Select a key ----------------------- Next screen : Press the same key ---|\r\n")




C_DisplayControl::C_DisplayControl():C_TaskControl(){
  int L_i;

  m_sem = NULL ;
  m_stat = NULL ;
  m_sem_scr_id = NULL ;
  m_screen_id = (int)'1' ;
  m_previous_screen_id = m_screen_id ;
  m_pause = false ;
  m_sem_pause = NULL ;
  NEW_VAR(m_screen_list, T_ScreenDataList());

  ALLOC_TABLE(m_screen_table, T_pScreenByKey, sizeof(T_ScreenByKey), MAX_KEY);
  for(L_i=0; L_i< MAX_KEY; L_i++) {
    m_screen_table[L_i].m_nb_screen = 0 ;
    m_screen_table[L_i].m_current_screen = 0 ;
    m_screen_table[L_i].m_screen_object = NULL ;
    m_screen_table[L_i].m_comment = NULL ;
  }
  m_current_screen = -1 ;
  m_current_display = NULL ;

  m_help_table_size = 0 ;
  m_help_table = NULL ;
  NEW_VAR(m_help_list, T_HelpInfoList());
}

C_DisplayControl::~C_DisplayControl(){

  int L_i ;

  DELETE_VAR (m_sem) ;
  m_stat = NULL ;
  DELETE_VAR (m_sem_scr_id);
  DELETE_VAR (m_sem_pause);

  if (!m_screen_list->empty()) {
    m_screen_list->erase(m_screen_list->begin(), 
			 m_screen_list->end());
  }
  DELETE_VAR(m_screen_list);


  for(L_i=0; L_i < MAX_KEY; L_i++) {
    FREE_TABLE(m_screen_table[L_i].m_screen_object);
    FREE_TABLE(m_screen_table[L_i].m_comment);
  }
  FREE_TABLE (m_screen_table);

  m_current_display = NULL ;

  if (!m_help_list->empty()) {
    m_help_list->erase(m_help_list->begin(),
		       m_help_list->end());
  }
  DELETE_VAR(m_help_list);

  FREE_TABLE(m_help_table);
		  
}

void  C_DisplayControl::init(long P_period) {
  GEN_DEBUG(0, "C_DisplayControl::init() start");
  m_stat = C_GeneratorStats::instance() ;
  NEW_VAR(m_sem_scr_id, C_Semaphore());
  NEW_VAR(m_sem_pause, C_Semaphore());
  NEW_VAR(m_sem, C_SemaphoreTimed(P_period));
  m_sem -> P();
  C_TaskControl::init() ;
  GEN_DEBUG(0, "C_DisplayControl::init() end");
}


T_GeneratorError C_DisplayControl::InitProcedure() {
  GEN_DEBUG(0, "C_DisplayControl::doInit() start");
  m_stat->init();
  GEN_DEBUG(0, "C_DisplayControl::doInit() end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_DisplayControl::TaskProcedure() {

  bool L_pause ; 

  GEN_DEBUG(0, "C_DisplayControl::doTask() start");

  m_sem_pause->P();
  L_pause = m_pause ;
  m_sem_pause->V() ;

  if (L_pause == false) {
    m_current_display->displayScreen();
  }

  m_stat->executeStatAction(C_GeneratorStats::E_RESET_PD_COUNTERS);
  sched_yield () ;
  m_sem -> P() ;
  GEN_DEBUG(0, "C_DisplayControl::doTask() end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_DisplayControl::EndProcedure() {
  // nothing to do at the end
  // just display the last screen

  m_current_display->displayScreen();
  
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_DisplayControl::StoppingProcedure() {
  M_state = C_TaskControl::E_STATE_STOPPED ;
  m_sem->V();

  return (E_GEN_NO_ERROR);
}

void C_DisplayControl::set_screen (char P_key) {
  int L_current ;
  bool L_first = false ;
  m_sem_scr_id->P() ;
  m_screen_id = (int)(P_key);

  // std::cerr << "m_screen_id " << m_screen_id << std::endl;
  // std::cerr << "m_screen_table[m_screen_id].m_nb_screen "  
  //	    << m_screen_table[m_screen_id].m_nb_screen << std::endl ;

  if ((m_screen_id <= MAX_KEY) && (m_screen_table[m_screen_id].m_nb_screen != 0)) {
    m_current_screen = m_screen_id ;

    L_current = m_screen_table[m_screen_id].m_current_screen ;
    if (m_previous_screen_id != m_screen_id) {
      m_previous_screen_id = m_screen_id ;
      L_first = true ;
      L_current = 0 ;
    } else {
      L_current++ ;
      if (L_current == m_screen_table[m_screen_id].m_nb_screen) { L_current = 0; }
    }

    m_screen_table[m_screen_id].m_screen_object[L_current]->setCurrentScreen(L_first);
    m_screen_table[m_screen_id].m_current_screen = L_current ;
    m_current_display = m_screen_table[m_screen_id].m_screen_object[L_current] ;
  }
  
  m_sem_scr_id->V() ;
}

T_GeneratorError C_DisplayControl::ForcedStoppingProcedure() {
  return (E_GEN_NO_ERROR);
}

void C_DisplayControl::pause () {
  m_sem_pause->P() ;
  m_pause = (m_pause == true) ? false : true ;
  m_sem_pause->V() ;
}

void C_DisplayControl::init_screens() {

  T_ScreenDataList::iterator  L_it      ;
  int                         L_current ;

  T_HelpInfoList::iterator L_list_help_it ;
  T_HelpInfo               L_list_info ;

  if (!m_screen_list->empty()) {

    for(L_it = m_screen_list->begin();
	L_it != m_screen_list->end();
	L_it++) {
      m_screen_table[(int)(L_it->m_key)].m_nb_screen++ ;
    }
    
    for(L_it = m_screen_list->begin();
	L_it != m_screen_list->end();
	L_it++) {
      if ((m_screen_table[(int)(L_it->m_key)].m_screen_object)
	  == NULL) {
	ALLOC_TABLE(m_screen_table[(int)(L_it->m_key)].m_screen_object,
		    C_DisplayObject**,
		    sizeof(C_DisplayObject*),
		    m_screen_table[(int)(L_it->m_key)].m_nb_screen);

	L_list_info.m_key = L_it->m_key ;
	L_list_info.m_comment = L_it->m_comment ;
	m_help_list->push_back (L_list_info);
      }
      L_current = m_screen_table[(int)(L_it->m_key)].m_current_screen ;
      if (m_current_display == NULL) {
	m_current_display = L_it->m_object ;
      }
      m_screen_table[(int)(L_it->m_key)].m_screen_object[L_current]
	= L_it->m_object ;
      m_screen_table[(int)(L_it->m_key)].m_current_screen++ ;
    }

    for(L_it = m_screen_list->begin();
	L_it != m_screen_list->end();
	L_it++) {
      m_screen_table[(int)(L_it->m_key)].m_current_screen = 0 ;
      m_screen_table[(int)(L_it->m_key)].m_comment = L_it->m_comment ;
    }

    if (!m_help_list->empty()) {
      L_current = 0 ;
      m_help_table_size = m_help_list->size();
      ALLOC_TABLE(m_help_table,
		  T_pHelpInfo,
		  sizeof(T_HelpInfo),
		  m_help_table_size);
      for (L_list_help_it = m_help_list->begin();
	   L_list_help_it != m_help_list->end();
	   L_list_help_it++) {
	m_help_table[L_current].m_key = L_list_help_it->m_key ;
	m_help_table[L_current].m_comment = L_list_help_it->m_comment ;
	L_current++;
      }
    }
  }
}

void C_DisplayControl::add_screen(C_DisplayObject *P_display, 
				  char P_key,
				  char *P_comment) {
  T_ScreenData L_screen_data ;

  L_screen_data.m_object = P_display ;
  L_screen_data.m_key = P_key ;

  ALLOC_TABLE(L_screen_data.m_comment,
	      char*,
	      sizeof(char),
	      strlen(P_comment)+1);
  strcpy(L_screen_data.m_comment,P_comment);

  m_screen_list->push_back(L_screen_data);
}

void C_DisplayControl::add_help(char P_key,
			       char *P_comment) {

  T_HelpInfo L_info ;

  L_info.m_key = P_key ;
  L_info.m_comment = P_comment ;
  m_help_list->push_back (L_info);

}

int  C_DisplayControl::help_all_screens(int P_counter) {

  int    L_i ;
  //  bool   L_found = true ;
  int    L_counter_help = m_help_table_size ;
  int    L_count ;

  DISPLAY_LINE ();

  // Add version tool : VERSION

  DISPLAY_INFO("Key: Description");
  DISPLAY_LINE ();

  L_count = 0 ;
  for (L_i = P_counter ; L_i < L_counter_help ; L_i++) {
    printf("|%c : %-73.73s|\n\r", 
	   m_help_table[L_i].m_key, m_help_table[L_i].m_comment);
    if (m_help_table[L_i].m_key == 'c' ) {
      printf("|    set call-rate      50 : call-rate become 50 c/s                          |\r\n");
      printf("|    set call-rate-scale 5 : use +/- key to increase/decrease call-rate by 5  |\r\n");
      L_count += 2 ;
    }
    
    L_count++;
    if (L_count > 19) {
      DISPLAY_NEXT();
      return (L_count) ; 
    }
  }

  if (L_count < 20) {  
    for (L_i = L_count ; L_i < 20 ; L_i ++) {
      DISPLAY_LINE_EMPTY();
    }
  }
  if (P_counter > 0) {
    DISPLAY_NEXT();
  } else {
    DISPLAY_LINE ();
  }

  return (L_count) ; 
}


void C_DisplayControl::change_display_period (long P_period) {
  m_sem -> change_display_period(P_period);
}


// end of file




