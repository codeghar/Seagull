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

#include "C_ProtocolStats.hpp"

#define max(a,b) ((a)>(b)) ? (a) : (b)

#define DISPLAY_CROSS_LINE()\
        printf("|--------------------------------+---------------------+-----------------------|\r\n")

#define DISPLAY_NEXT()\
        printf("|----------------------------------------- Next screen : Press the same key ---|\r\n")



#define DISPLAY_3TXT(T1, T2, T3)\
	printf("| %-30s | %19s | %21s |\r\n", (T1), (T2), (T3))

#define DISPLAY_COUNTERS(T,V1,V2,V3,V4) \
	printf("| %-30s | %8d | %8d | %9d | %9d |\r\n", (T), (V1), (V2), (V3), (V4))

#define DISPLAY_5TXT(T1, T2, T3, T4, T5)\
	printf("| %-30s | %8s | %8s | %9s | %9s |\r\n", (T1), (T2), (T3), (T4), (T5))


C_ProtocolStats::C_ProtocolStats(C_ProtocolFrame *P_protocol)

 : C_ProtocolStatsFrame(P_protocol, (C_DisplayObject*)this) {

  T_pNameAndIdList L_list_message  = NULL ;
  T_pNameAndIdList L_list_comp_message  = NULL ;
  int              L_i, L_id ;
  T_NameAndIdList::iterator  L_elt_it ;


  m_id_msg_table            = NULL ;
  m_counter_msg_table       = NULL ;
  m_message_names           = NULL ;
  m_id_msg_comp_table       = NULL ;
  m_counter_msg_comp_table  = NULL ;
  m_message_component_names = NULL ;

  m_counter_message = 0 ;
  m_counter_message_comp = 0 ;


  m_nb_counter_msg  = 0 ;
  m_nb_counter_msg_comp  = 0 ;


  m_other_msg_screen     = false  ;
  m_other_msg_screen_comp = false ;

  
  m_message_name = m_protocol->message_name () ; // => primitive

  m_message_component_name = m_protocol->message_component_name () ; // => component

  // m_counter_msg_table construction
  L_list_message = m_protocol->message_name_list () ; // => TC_BEGIN, .... +ids
  if((L_list_message != NULL) && (!L_list_message -> empty())) {
    m_nb_counter_msg = L_list_message->size() ; 
    if (m_nb_counter_msg > 0 ) {
      // => max id msg
      m_max_id_msg = 0 ;
      for(L_elt_it=L_list_message->begin();
	  L_elt_it != L_list_message->end();
	  L_elt_it++) {
	m_max_id_msg = max(m_max_id_msg,(L_elt_it->m_id));
      }
      
      // => alloc m_id_msg_table[Id_prot] = Id m_counter_msg_table
      ALLOC_TABLE(m_id_msg_table,
		  int*,
		  sizeof(int),
		  m_max_id_msg+1);
      
      ALLOC_TABLE(m_counter_msg_table,
		  C_pCounterData*,
		  sizeof(C_pCounterData),
		  m_nb_counter_msg);
      
      ALLOC_TABLE(m_message_names,
		  char**,
		  sizeof(char*),
		  m_nb_counter_msg);
      
      for (L_i = 0 ; L_i < m_nb_counter_msg; L_i++) {
	NEW_TABLE (m_counter_msg_table[L_i], C_CounterData, (size_t)E_NB_ACTIONS);
      }
      
      L_id = 0 ;
      for(L_elt_it =  L_list_message->begin();
	  L_elt_it != L_list_message->end();
	  L_elt_it++) {
	
	m_id_msg_table[(L_elt_it->m_id)] = L_id ;
	ALLOC_TABLE(m_message_names[L_id],
		    char*, sizeof(char),
		    strlen(L_elt_it->m_name)+1);
	
	strcpy(m_message_names[L_id],(L_elt_it->m_name));
	m_counter_msg_table[L_id][(int)E_SEND].m_counter_value = 0;
	m_counter_msg_table[L_id][(int)E_RECEIVE].m_counter_value = 0;
	
	m_counter_msg_table[L_id][(int)E_SEND].m_counter_periodic_value = 0;
	m_counter_msg_table[L_id][(int)E_RECEIVE].m_counter_periodic_value = 0;
	
	m_counter_msg_table[L_id][(int)E_SEND].m_counter_log_value = 0;
	m_counter_msg_table[L_id][(int)E_RECEIVE].m_counter_log_value = 0;
	
	m_counter_msg_table[L_id][(int)E_SEND].m_counter_periodic_log_value = 0;
	m_counter_msg_table[L_id][(int)E_RECEIVE].m_counter_periodic_log_value = 0;
	
	L_id ++ ;
      }
      
      
      m_current_proto_stat_id = -1 ;
      m_nb_stat_proto_id = 2 ;
      ALLOC_TABLE(m_current_proto_stat_table, 
		  T_DisplayProtocolFct*, 
		  sizeof(T_DisplayProtocolFct), 2);
      m_current_proto_stat_table[0] = &C_ProtocolStats::display_stats_message ;
      m_current_proto_stat_table[1] = &C_ProtocolStats::display_stats_sub_message ;
      m_current_proto_stat_display = m_current_proto_stat_table[0];
      
    }
  }

  // m_counter_msg_comp_table construction
  L_list_comp_message = m_protocol->message_component_name_list () ; // => TC_INVOKE... + ids
  if((L_list_comp_message != NULL) && (!L_list_comp_message -> empty())) {
    m_nb_counter_msg_comp = L_list_comp_message->size() ;
    if (m_nb_counter_msg_comp > 0 ) {
      // => max id comp msg
      m_max_id_msg_comp = 0 ;
      for(L_elt_it=L_list_comp_message->begin();
	  L_elt_it != L_list_comp_message->end();
	  L_elt_it++) {
	m_max_id_msg_comp = max(m_max_id_msg_comp,(L_elt_it->m_id));
      }
      // => alloc m_id_msg_comp_table[Id_prot]=Id m_counter_msg_comp_table
      
      ALLOC_TABLE(m_id_msg_comp_table,
		  int*,
		  sizeof(int),
		  m_max_id_msg_comp+1);
      
      ALLOC_TABLE(m_counter_msg_comp_table,
		  C_pCounterData*,
		  sizeof(C_pCounterData),
		  m_nb_counter_msg_comp);
      

      ALLOC_TABLE(m_message_component_names,
		  char**,
		  sizeof(char*),
		  m_nb_counter_msg_comp);
      
      for (L_i = 0 ; L_i < m_nb_counter_msg_comp; L_i++) {
	NEW_TABLE (m_counter_msg_comp_table[L_i], C_CounterData, 
		   (size_t) E_NB_ACTIONS);
      }
      
      L_id = 0 ;
      for(L_elt_it =  L_list_comp_message->begin();
	  L_elt_it != L_list_comp_message->end();
	  L_elt_it++) {
	
	m_id_msg_comp_table[(L_elt_it->m_id)] = L_id ;
	ALLOC_TABLE(m_message_component_names[L_id],
		    char*, sizeof(char),
		    strlen(L_elt_it->m_name)+1);
	
	strcpy(m_message_component_names[L_id],(L_elt_it->m_name));
	m_counter_msg_comp_table[L_id][(int)E_SEND].m_counter_value = 0;
	m_counter_msg_comp_table[L_id][(int)E_RECEIVE].m_counter_value = 0;
	m_counter_msg_comp_table[L_id][(int)E_SEND].m_counter_periodic_value = 0;
	m_counter_msg_comp_table[L_id][(int)E_RECEIVE].m_counter_periodic_value = 0;
	
	m_counter_msg_comp_table[L_id][(int)E_SEND].m_counter_log_value = 0;
	m_counter_msg_comp_table[L_id][(int)E_RECEIVE].m_counter_log_value = 0;
	
	m_counter_msg_comp_table[L_id][(int)E_SEND].m_counter_periodic_log_value = 0;
	m_counter_msg_comp_table[L_id][(int)E_RECEIVE].m_counter_periodic_log_value = 0;
	
	L_id ++ ;
      }
    }
  }

  m_fileName = NULL ;
  m_initDone = false ;
  m_do_log = false ;
  

}


C_ProtocolStats::~C_ProtocolStats(){
  int L_i ;

  if (m_message_names != NULL){
    for(L_i = 0; L_i < m_nb_counter_msg; L_i++) {
      FREE_TABLE(m_message_names[L_i]);
    }
    FREE_TABLE(m_message_names);
  }

  if (m_message_component_names!= NULL){
    for(L_i = 0; L_i < m_nb_counter_msg_comp; L_i++) {
      FREE_TABLE(m_message_component_names[L_i]);
    }
    FREE_TABLE(m_message_component_names);
  }

  for (L_i = 0 ; L_i < m_nb_counter_msg; L_i++) {
    DELETE_TABLE(m_counter_msg_table[L_i]);
  }
  FREE_TABLE(m_counter_msg_table);
  m_nb_counter_msg = 0 ;

  for (L_i = 0 ; L_i < m_nb_counter_msg_comp; L_i++) {
    DELETE_TABLE(m_counter_msg_comp_table[L_i]);
  }
  FREE_TABLE(m_counter_msg_comp_table);
  m_nb_counter_msg_comp = 0 ;

  FREE_TABLE(m_id_msg_table);
  FREE_TABLE(m_id_msg_comp_table);
  m_max_id_msg = 0 ;
  m_max_id_msg_comp = 0 ;

  FREE_TABLE(m_current_proto_stat_table);


  FREE_TABLE(m_fileName);
  m_initDone = false ;

  DELETE_VAR(m_outputStream);

}

void C_ProtocolStats::updateStats (T_ProtocolStatDataType P_type,
				   T_ProtocolStatDataAction P_action,
				   int P_id) {
  switch (P_type) {
  case E_MESSAGE : {
    if ((P_id >= 0) && (P_id <= m_max_id_msg)) {
      m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_sem.P();
      (m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_counter_value)++ ;
      (m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_counter_periodic_value)++ ;
      m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_sem.V();
      
      
      if (m_do_log) {
        m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_sem_log.P();
        (m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_counter_log_value)++ ;
        (m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_counter_periodic_log_value)++ ;
        m_counter_msg_table[m_id_msg_table[P_id]][P_action].m_sem_log.V();
      }
    }
  }
  break ;
  case E_MESSAGE_COMPONENT : {
    if ((P_id >= 0) && (P_id <= m_max_id_msg_comp)) {
      
      m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_sem.P();
      (m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_counter_value)++ ;
      (m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_counter_periodic_value)++ ;
      m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_sem.V();
      
      if (m_do_log) {
        m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_sem_log.P();
        (m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_counter_log_value)++ ;
        (m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_counter_periodic_log_value)++ ;
        m_counter_msg_comp_table[m_id_msg_comp_table[P_id]][P_action].m_sem_log.V();
      }
    }
  }
  break ;
  default:
    break ;
  }
  
}

iostream_output& C_ProtocolStats::display (iostream_output& P_stream) {

  int L_i ;

  P_stream << m_message_name << "   sent   received" << iostream_endl ;
  for(L_i = 0 ; L_i < m_nb_counter_msg; L_i ++) {
    P_stream << m_message_names[L_i] << " " ;

    m_counter_msg_table[L_i][(int)E_SEND].m_sem.P();
    P_stream << m_counter_msg_table[L_i][(int)E_SEND].m_counter_value << " " ;
    m_counter_msg_table[L_i][(int)E_SEND].m_sem.V();

    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem.P();
    P_stream << m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_value ;
    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem.V();

    P_stream << iostream_endl ;
  }

  P_stream << m_message_component_name << "   sent   received" << iostream_endl ;
  for(L_i = 0 ; L_i < m_nb_counter_msg_comp; L_i ++) {
    P_stream << m_message_component_names[L_i] << " " ;

    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem.P();
    P_stream << m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_value << " " ;
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem.V();

    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem.P();
    P_stream << m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_value ;
    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem.V();

    P_stream << iostream_endl ;
  }
  
  return (P_stream);
}



void C_ProtocolStats::setCurrentScreen(bool P_first) {

  if (P_first == true) {

    m_counter_message = 0 ;
    m_counter_message_comp = 0 ;
    
    m_other_msg_screen     = false  ;
    m_other_msg_screen_comp = false ;
    
    m_current_proto_stat_id = 0 ;
    
  } else {

  
    m_current_proto_stat_id ++ ;
    
    if (m_other_msg_screen == true) {
      m_counter_message += 18;
      if (m_counter_message >= m_nb_counter_msg) {
	m_counter_message = 0 ;
	m_other_msg_screen = false ;  
      } else {
	m_current_proto_stat_id --;
      }
    }
    
    
    if (m_other_msg_screen_comp == true) {
      m_counter_message_comp += 18 ;
      if (m_counter_message_comp >= m_nb_counter_msg_comp) {
	m_counter_message_comp = 0 ;
	m_other_msg_screen_comp = false ;  
      } else {
	m_current_proto_stat_id --;
      }
    }
    
    if (m_current_proto_stat_id == m_nb_stat_proto_id) {
      m_current_proto_stat_id = 0 ;
    }
  }
  m_current_proto_stat_display 
    = m_current_proto_stat_table[m_current_proto_stat_id];

}


void C_ProtocolStats::displayScreen() {

  ((this)->*(m_current_proto_stat_display))();

}

void C_ProtocolStats::display_stats_message() {

  int L_i ;

  int L_count ;

  DISPLAY_CROSS_LINE ();
  printf("|                                |  Periodic value     |   Cumulative value    |\r\n");  
  DISPLAY_5TXT(m_message_name, "sent", "received", "sent", "received");
  DISPLAY_CROSS_LINE ();

  L_count = 0 ;

  for(L_i = m_counter_message ; L_i < m_nb_counter_msg; L_i ++) {
    m_counter_msg_table[L_i][(int)E_SEND].m_sem.P();
    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem.P();

    DISPLAY_COUNTERS(m_message_names[L_i],
		     m_counter_msg_table[L_i][(int)E_SEND].m_counter_periodic_value,
		     m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_periodic_value,
		     m_counter_msg_table[L_i][(int)E_SEND].m_counter_value,
		     m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_value);


    m_counter_msg_table[L_i][(int)E_SEND].m_counter_periodic_value = 0;
    m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_periodic_value = 0;

    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem.V();
    m_counter_msg_table[L_i][(int)E_SEND].m_sem.V();

    L_count++ ;
    if (L_count > 18) {
      DISPLAY_NEXT() ;
      if (m_other_msg_screen == false) {
	m_other_msg_screen = true ;
      }
      break ;
    }
  }

  if (L_count < 19) {
    for (L_i = L_count ; L_i < 19 ; L_i ++) {
      DISPLAY_3TXT("","","");
    }
    DISPLAY_NEXT() ;
  }
  
}


void C_ProtocolStats::display_stats_sub_message() {
  
  int L_i ;

  int L_count ;

  DISPLAY_CROSS_LINE ();
  printf("|                                |    Periodic value   |     Cumulative value  |\r\n");  

  DISPLAY_5TXT(m_message_component_name, "sent", "received", "sent", "received");

  DISPLAY_CROSS_LINE ();

  L_count = 0 ;
  for(L_i = m_counter_message_comp ; L_i < m_nb_counter_msg_comp; L_i ++) {
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem.P();
    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem.P();

    DISPLAY_COUNTERS(m_message_component_names[L_i],
		     m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_periodic_value,
		     m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_periodic_value,
		     m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_value,
		     m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_value);


    m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_periodic_value = 0;
    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_periodic_value = 0;

    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem.V();
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem.V();
    L_count ++ ;

    if (L_count > 18) {
      DISPLAY_NEXT() ;
      if (m_other_msg_screen_comp == false) {
	m_other_msg_screen_comp = true ;
      }
      break ;
    }
  }

  if (L_count < 19) {
    for (L_i = L_count ; L_i < 19 ; L_i ++) {
      DISPLAY_3TXT("","","");
    }
    DISPLAY_NEXT() ;
  }
}


void C_ProtocolStats::set_file_name(char * P_name)
{
  int sizeOf; 

  if(P_name != NULL) { 
    sizeOf = strlen(P_name);
    if(sizeOf > 0) {
      ALLOC_TABLE(m_fileName, char*,
                  sizeof(char), sizeOf+1);
      strcpy(m_fileName, P_name);
    } else {
      iostream_error << "new file name length is null " << iostream_endl;
    }
  } else {
    iostream_error << 
      "new file name is NULL ! " << iostream_endl;
  }
}

int C_ProtocolStats::init () {
  if (m_initDone == false) {
    GET_TIME (&m_startTime);
    memcpy   (&m_plStartTime, &m_startTime, sizeof (struct timeval));
    m_outputStream = NULL;
    m_headerAlreadyDisplayed = false;
    m_initDone = true ;
  }
  return(1);
}


void C_ProtocolStats::make_first_log () {

  int  L_i ;
  if(m_outputStream == NULL) {
    // if the file is still not opened, we opened it now
    NEW_VAR(m_outputStream,
	    fstream_output(m_fileName));
    m_headerAlreadyDisplayed = false;
    
    if(m_outputStream == NULL) {
      iostream_error << "Unable to open log file !" << iostream_endl;
      exit(-1);
    }
    
    if(!m_outputStream->good()) {
      iostream_error << "Unable to open log file !" << iostream_endl;
      exit(-1);
    }
  }
  
  // header - it's dump in file only one time at the beginning of the file
  (*m_outputStream) << "StartTime;"
		    << "LastResetTime;"
		    << "CurrentTime;" 
		    << "ElapsedTime(P);"
		    << "ElapsedTime(C);" ;


  for(L_i = 0 ; L_i < m_nb_counter_msg; L_i ++) {
    (*m_outputStream) << m_message_names[L_i] << " received(P);" ;
    (*m_outputStream) << m_message_names[L_i] << " received(C);" ;

    (*m_outputStream) << m_message_names[L_i] << " sent(P);" ;
    (*m_outputStream) << m_message_names[L_i] << " sent(C);" ;
  }


  for(L_i = 0 ; L_i < m_nb_counter_msg_comp; L_i ++) {
  
    (*m_outputStream) << m_message_component_names[L_i] << " received(P);" ;
    (*m_outputStream) << m_message_component_names[L_i] << " received(C);" ;

    (*m_outputStream) << m_message_component_names[L_i] << " sent(P);" ;
    (*m_outputStream) << m_message_component_names[L_i] << " sent(C);" ;

  }


  (*m_outputStream) << iostream_endl;
  // flushing the output file to let the tail -f working !
  (*m_outputStream).flush();

}

void C_ProtocolStats::make_log () {
  int            L_i               ;
  long           localElapsedTime  ;
  long           globalElapsedTime ;
  struct timeval currentTime       ;


  GET_TIME (&currentTime);
  // computing the real call rate
  globalElapsedTime   = ms_difftime (&currentTime, &m_startTime);
  localElapsedTime    = ms_difftime (&currentTime, &m_plStartTime);

  (*m_outputStream) << formatTime(&m_startTime)                                << ";";
  (*m_outputStream) << formatTime(&m_plStartTime)                              << ";";
  (*m_outputStream) << formatTime(&currentTime)                                << ";"
                    << msToHHMMSS(localElapsedTime)                            << ";";
  (*m_outputStream) << msToHHMMSS(globalElapsedTime)                           << ";";


  for(L_i = 0 ; L_i < m_nb_counter_msg; L_i ++) {
    m_counter_msg_table[L_i][(int)E_SEND].m_sem_log.P();
    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem_log.P();

    (*m_outputStream) 
      << m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_periodic_log_value << ";" 
      << m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_log_value << ";" 
      << m_counter_msg_table[L_i][(int)E_SEND].m_counter_periodic_log_value << ";" 
      << m_counter_msg_table[L_i][(int)E_SEND].m_counter_log_value << ";"  ;
      

    m_counter_msg_table[L_i][(int)E_SEND].m_counter_periodic_log_value = 0;
    m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_periodic_log_value = 0;

    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem_log.V();
    m_counter_msg_table[L_i][(int)E_SEND].m_sem_log.V();
  }

  for(L_i = 0 ; L_i < m_nb_counter_msg_comp; L_i ++) {
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem_log.P();
    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem_log.P();

    (*m_outputStream) 
      << m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_periodic_log_value << ";" 
      << m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_log_value << ";" 
      << m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_periodic_log_value << ";" 
      << m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_log_value << ";"  ;
      

    m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_periodic_log_value = 0;
    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_periodic_log_value = 0;

    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem_log.V();
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem_log.V();
  }

  GET_TIME (&m_plStartTime);

  (*m_outputStream) << iostream_endl;
  // flushing the output file to let the tail -f working !
  (*m_outputStream).flush();



}




char* C_ProtocolStats::formatTime (struct timeval* P_tv) {

  static char L_time [TIME_LENGTH];
  struct tm * L_currentDate;
  
  // Get the current date and time
  L_currentDate = localtime ((const time_t *)&P_tv->tv_sec);
  
  // Format the time
  if (L_currentDate == NULL) {
    memset (L_time, 0, TIME_LENGTH);
  } else {
    sprintf(L_time, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", 
	    L_currentDate->tm_year + 1900,
	    L_currentDate->tm_mon + 1,
	    L_currentDate->tm_mday,
	    L_currentDate->tm_hour,
	    L_currentDate->tm_min,
	    L_currentDate->tm_sec);
  }
  return (L_time);
} /* end of formatTime */


char* C_ProtocolStats::msToHHMMSS (unsigned long P_ms)
{
	static char L_time [TIME_LENGTH];
	unsigned long hh, mm, ss;
	
	P_ms = P_ms / 1000;
	hh = P_ms / 3600;
	mm = (P_ms - hh * 3600) / 60;
	ss = P_ms - (hh * 3600) - (mm * 60);
	sprintf (L_time, "%2.2ld:%2.2ld:%2.2ld", hh, mm, ss);
	return (L_time);
} /* end of msToHHMMSS */

void C_ProtocolStats::do_log() {
  m_do_log = true ;
}


void C_ProtocolStats::reset_cumul_counters() {

  int L_i ;

  for(L_i = 0 ; L_i < m_nb_counter_msg; L_i ++) {
    m_counter_msg_table[L_i][(int)E_SEND].m_sem.P();
    m_counter_msg_table[L_i][(int)E_SEND].m_counter_value = 0 ;
    m_counter_msg_table[L_i][(int)E_SEND].m_sem.V();

    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem.P();
    m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_value  = 0 ;
    m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem.V();


    if (m_do_log) {
      m_counter_msg_table[L_i][(int)E_SEND].m_sem_log.P();
      m_counter_msg_table[L_i][(int)E_SEND].m_counter_log_value = 0 ;
      m_counter_msg_table[L_i][(int)E_SEND].m_sem_log.V();

      m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem_log.P();
      m_counter_msg_table[L_i][(int)E_RECEIVE].m_counter_log_value = 0 ; 
      m_counter_msg_table[L_i][(int)E_RECEIVE].m_sem_log.V();
    }
  }

  for(L_i = 0 ; L_i < m_nb_counter_msg_comp; L_i ++) {
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem.P();
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_value = 0  ;
    m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem.V();

    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem.P();
    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_value = 0 ;
    m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem.V();

    if (m_do_log) {
      m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem_log.P();
      m_counter_msg_comp_table[L_i][(int)E_SEND].m_counter_log_value = 0 ;
      m_counter_msg_comp_table[L_i][(int)E_SEND].m_sem_log.V();
      
      m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem_log.P();
      m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_counter_log_value = 0 ; 
      m_counter_msg_comp_table[L_i][(int)E_RECEIVE].m_sem_log.V();

    }
  }
}
