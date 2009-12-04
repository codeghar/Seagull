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

#include "C_ChannelControl.hpp"
#include "Utils.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "set_t.hpp"

#define XML_CHANNEL_SECTION      (char*)"configuration"
#define XML_CHANNEL_SUBSECTION   (char*)"define"
#define XML_CHANNEL_ENTITY       (char*)"entity"
#define XML_CHANNEL_ENTITY_VALUE (char*)"channel"
#define XML_CHANNEL_NAME         (char*)"name"
#define XML_CHANNEL_GLOBAL       (char*)"global"
#define XML_CHANNEL_GLOBAL_VALUE (char*)"yes"
#define XML_CHANNEL_PROTOCOL     (char*)"protocol"
#define XML_CHANNEL_TRANSPORT    (char*)"transport"
#define XML_CHANNEL_OPEN         (char*)"open-args"
#define XML_CHANNEL_RECONNECT    (char*)"reconnect"


typedef struct _channel_info {
  int                                m_id             ;
  C_ChannelControl::T_ChannelType    m_type           ;
  char                              *m_protocol_name  ;
  char                              *m_transport_name ;
  char                              *m_open_args      ;
  char                              *m_name           ;
  char                              *m_reconnect      ;
} T_ChannelInfo, *T_pChannelInfo ;
typedef list_t<T_pChannelInfo> T_ChannelInfoList ;

typedef set_t<int> T_IntSet, *T_pIntSet ;

C_ChannelControl::C_ChannelControl() {
  GEN_DEBUG(1, "C_ChannelControl::C_ChannelControl() start");
  NEW_VAR(m_name_map, T_ChannelNameMap());
  //  m_name_map->clear() ;
  m_channel_table = NULL ;
  m_channel_table_size = 0 ;
  m_nb_global_channel = 0 ;
  NEW_VAR(m_id_gen, C_IdGenerator()) ;
  m_channel_to_ctxt_table = NULL ;
  m_ctxt_to_channel_table = NULL ;
  m_transport_table = NULL ;
  m_nb_transport = 0 ;
  m_reconnect = false ;
  m_sem_reconnect = NULL ;
  GEN_DEBUG(1, "C_ChannelControl::C_ChannelControl() end");
}

bool C_ChannelControl::reconnect() {
  return (m_reconnect);
}

C_ChannelControl::~C_ChannelControl() {

  int L_i ;

  GEN_DEBUG(1, "C_ChannelControl::~C_ChannelControl() start");
  if (!m_name_map->empty()) {
    m_name_map->erase(m_name_map->begin(), m_name_map->end());
  }
  DELETE_VAR(m_name_map);
  if (m_channel_table_size != 0) {
    for (L_i = 0 ; L_i < m_channel_table_size; L_i++) {
      FREE_VAR(m_channel_table[L_i]);
    }
    FREE_TABLE(m_channel_table);
    m_channel_table_size = 0 ;
  }
  DELETE_VAR(m_id_gen) ;
  m_nb_global_channel = 0 ;
  FREE_TABLE(m_channel_to_ctxt_table);
  FREE_TABLE(m_ctxt_to_channel_table);
  m_nb_transport = 0 ;
  FREE_TABLE(m_transport_table);
  DELETE_VAR(m_sem_reconnect);
  GEN_DEBUG(1, "C_ChannelControl::~C_ChannelControl() end");
}

bool C_ChannelControl::fromXml (C_XmlData *P_data, 
				C_ProtocolControl  *P_protocol_ctrl,
				C_TransportControl *P_transport_ctrl) {
  
  bool                          L_ret       = true   ;
  T_pXmlData_List               L_sectionList, L_subSectionList            ;
  T_XmlData_List::iterator      L_sectionListIt, L_subSectionListIt ;
  C_XmlData                    *L_section, *L_subSection ;
  char                         *L_entity_value ;
  char                         *L_name, 
                               *L_open_args,
                               *L_transport_name,
                               *L_protocol_name ,
                               *L_reconnect;
  int                           L_channel_id ;
  T_ChannelType                 L_channel_type ;
  T_pChannelInfo                L_channel_info = NULL ;
  T_ChannelInfoList             L_channel_info_list ;
  T_ChannelInfoList::iterator   L_it ;
  T_pChannelData                L_data ;

  GEN_DEBUG(1, "C_ChannelControl::fromXml() start");

  if (P_data != NULL) {
    if ((L_sectionList = P_data->get_sub_data()) != NULL) {
      
      for(L_sectionListIt  = L_sectionList->begin() ;
	  L_sectionListIt != L_sectionList->end() ;
	  L_sectionListIt++) {

	L_section = *L_sectionListIt ;
	if (L_section != NULL) {
	  if (strcmp(L_section->get_name(), XML_CHANNEL_SECTION) == 0) {

	    L_subSectionList = L_section->get_sub_data() ;
	    for(L_subSectionListIt  = L_subSectionList->begin() ;
		L_subSectionListIt != L_subSectionList->end() ;
		L_subSectionListIt++) {
	      L_subSection = *L_subSectionListIt ;
	      
	      if (strcmp(L_subSection->get_name(), XML_CHANNEL_SUBSECTION) == 0) {

		L_entity_value = L_subSection->find_value(XML_CHANNEL_ENTITY) ;
		if (L_entity_value != NULL) {
		  if (strcmp(L_entity_value, XML_CHANNEL_ENTITY_VALUE) == 0) {

		    L_name = L_subSection->find_value(XML_CHANNEL_NAME); 
		    if (L_name == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_name << "] value is mandatory for section ["
				<< XML_CHANNEL_SECTION << "], sub section ["
				<< XML_CHANNEL_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }

		    L_open_args = L_subSection->find_value(XML_CHANNEL_OPEN);
		    if (L_open_args == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_open_args << "] value is mandatory for section ["
				<< XML_CHANNEL_SECTION << "], sub section ["
				<< XML_CHANNEL_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }
		    L_protocol_name = L_subSection->find_value(XML_CHANNEL_PROTOCOL);
		    if (L_protocol_name == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_protocol_name << "] value is mandatory for section ["
				<< XML_CHANNEL_SECTION << "], sub section ["
				<< XML_CHANNEL_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }

		    L_reconnect = L_subSection->find_value(XML_CHANNEL_RECONNECT);

		    L_transport_name = L_subSection->find_value(XML_CHANNEL_TRANSPORT);
		    if (L_transport_name == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_transport_name << "] value is mandatory for section ["
				<< XML_CHANNEL_SECTION << "], sub section ["
				<< XML_CHANNEL_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }
		    if (L_subSection->find_value(XML_CHANNEL_GLOBAL) != NULL) {
		      if (strcmp(L_subSection->find_value(XML_CHANNEL_GLOBAL), XML_CHANNEL_GLOBAL_VALUE) == 0) {
			L_channel_type = E_CHANNEL_GLOBAL ;
		      } else {
			L_channel_type = E_CHANNEL_LOCAL ;
		      }
		    } else {
		      L_channel_type = E_CHANNEL_GLOBAL ;
		    }

		    // check channel name unicity
		    if (m_name_map->find(T_ChannelNameMap::key_type(L_name)) 
			!= m_name_map->end()) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, 
				XML_CHANNEL_SECTION << " with name ["
				<< L_name << "] already defined");
		      L_ret = false ;
		      break ;
		    }

		    // check transport and protocol definition
		    if (P_protocol_ctrl->get_protocol_id(L_protocol_name) 
			== ERROR_PROTOCOL_UNKNOWN) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, 
				"Protocol ["
				<< L_protocol_name << "] unknown for channel ["
				<< L_name << "]");
		      L_ret = false ;
		      break ;
		    }
		    if (P_transport_ctrl->get_transport_id(L_transport_name) 
			== ERROR_TRANSPORT_UNKNOWN) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, 
				"Transport ["
				<< L_transport_name << "] unknown for channel ["
				<< L_name << "]");
		      L_ret = false ;
		      break ;
		    }
		    
		    L_channel_id = m_id_gen->new_id() ;
		    ALLOC_VAR(L_channel_info,
			      T_pChannelInfo,
			      sizeof(T_ChannelInfo));

		    L_channel_info->m_id = L_channel_id ;
		    L_channel_info->m_open_args = L_open_args ;
		    L_channel_info->m_protocol_name = L_protocol_name ; 
		    L_channel_info->m_transport_name = L_transport_name ; 
		    L_channel_info->m_type = L_channel_type ;
		    L_channel_info->m_name = L_name ;
		    L_channel_info->m_reconnect = L_reconnect ;
		    L_channel_info_list.push_back(L_channel_info);
		    m_name_map
		      ->insert(T_TransportNameMap::value_type(L_name,
							      L_channel_id)) ;
		    
		  }

		}
		
	      }

	    }

	  }
	}
      }

      if (L_ret != false) {
	if (!L_channel_info_list.empty()) {
	  m_channel_table_size = L_channel_info_list.size () ;
	  ALLOC_TABLE(m_channel_table,
		      T_pChannelData*,
		      sizeof(T_pChannelData),
		      m_channel_table_size);
	  for (L_it  = L_channel_info_list.begin();
	       L_it != L_channel_info_list.end()  ;
	       L_it++) {
	    L_channel_info = *L_it ;
	    ALLOC_VAR(m_channel_table[L_channel_info->m_id], 
		      T_pChannelData, 
		      sizeof(T_ChannelData));
	    L_data = m_channel_table[L_channel_info->m_id] ;

	    L_data->m_id = L_channel_info->m_id ;
	    L_data->m_type = L_channel_info->m_type ;
	    L_data->m_protocol_id = P_protocol_ctrl->get_protocol_id(L_channel_info->m_protocol_name) ;
	    L_data->m_transport_id = P_transport_ctrl->get_transport_id(L_channel_info->m_transport_name) ;
	    L_data->m_protocol = P_protocol_ctrl->get_protocol(L_channel_info->m_protocol_name) ;
	    L_data->m_transport = P_transport_ctrl->get_transport(L_channel_info->m_transport_name) ;
	    L_data->m_open_args = L_channel_info->m_open_args ;
	    L_data->m_open_status = E_CHANNEL_CLOSED ;

	    L_data->m_reconnect = (L_channel_info->m_reconnect == NULL) ?
	      false : (strcmp(L_channel_info->m_reconnect, "yes") == 0) ;

	    if (L_data->m_reconnect == true) { m_reconnect = true ; }

	    L_data->m_name = L_channel_info->m_name ;

	    if (L_data->m_type == E_CHANNEL_GLOBAL) {
	      m_nb_global_channel++ ;
	    }

	  }
	} else {
	  GEN_ERROR(E_GEN_FATAL_ERROR,
		    "No channel definition found");
	  L_ret = false ;
	}
      }

      if (!L_channel_info_list.empty()) {
	for (L_it  = L_channel_info_list.begin();
	     L_it != L_channel_info_list.end()  ;
	     L_it++) {
	  if (L_ret == false) { // error
	    if (m_channel_table_size > 0) {
	      L_channel_info = *L_it ;
	      FREE_VAR(m_channel_table[L_channel_info->m_id]);
	    }
	  }
	  FREE_VAR(*L_it);
	}
	L_channel_info_list.erase(L_channel_info_list.begin(),
				    L_channel_info_list.end());
      }

      if (L_ret == false) {
	// delete structures
	if (m_channel_table_size > 0) {
	  FREE_TABLE(m_channel_table);
	  m_channel_table_size = 0 ;
	}
      }
    }
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "No channel definition found");
    L_ret = false ;
  }

  if (L_ret == true) {
    create_context_channel () ;
    create_transport_table (P_transport_ctrl) ;
  }

  NEW_VAR(m_sem_reconnect, C_SemaphoreTimed(1));
  
  GEN_DEBUG(1, "C_ChannelControl::fromXml() end ret=" << L_ret);
  return (L_ret);
}

C_ChannelControl::T_pChannelData C_ChannelControl::get_channel_data (int P_id) {
  T_pChannelData L_ret = NULL ;

  if (P_id < m_channel_table_size) {
    L_ret = m_channel_table[P_id] ;
  }

  return (L_ret) ;
}

C_ChannelControl::T_pChannelData C_ChannelControl::get_channel_data (char *P_name) {
  T_pChannelData L_ret = NULL ;
  int            L_id         ;

  L_id = get_channel_id (P_name) ;
  if (L_id != ERROR_CHANNEL_UNKNOWN) {
    L_ret = m_channel_table[L_id] ;
  }

  return (L_ret) ;
}

int C_ChannelControl::get_channel_id (char *P_name) {
  int                         L_ret = ERROR_CHANNEL_UNKNOWN ;
  T_ChannelNameMap::iterator L_it ;

  L_it = m_name_map->find(T_ProtocolNameMap::key_type(P_name)) ;
  if (L_it != m_name_map->end()) {
    L_ret = L_it->second ;
  }
  return (L_ret) ;
}


char* C_ChannelControl::get_channel_name (int P_id) {
  return (m_channel_table[P_id]->m_name);
}

int C_ChannelControl::check_global_channel (int P_timeLag) {
  int            L_ret = 0 ;
  T_OpenStatus   L_status  ;
  int            L_i       ;
  T_pChannelData L_data    ;

  for(L_i=0; L_i < m_channel_table_size; L_i++) {
    L_data = m_channel_table[L_i] ;

    if (L_data->m_type == E_CHANNEL_GLOBAL) {
      if (L_data->m_open_status == E_CHANNEL_CLOSED) {
	if (L_data->m_reconnect == true) {
           sleep(P_timeLag);
	  // wait
	  m_sem_reconnect->P();
	  m_sem_reconnect->P();

	  L_data->m_open_id
	    = (L_data->m_transport)
	    ->open(L_data->m_id,
		   L_data->m_open_args, 
		   &L_status,
		   L_data->m_protocol);
	  GEN_DEBUG(0, "open channel [" << L_i << "] with [" 
		    << L_data->m_open_id << "]");
	  
	  switch (L_status) {
	  case E_OPEN_OK:
	    L_ret ++ ; 
	    L_data->m_open_status = E_CHANNEL_OPENED ;
	    break ;
	  case E_OPEN_DELAYED:
	    L_data->m_open_status = E_CHANNEL_OPEN_IN_PROGESS ;
	    break ;
	  default:
	    break;
	  }
	}
      } else {
	L_ret ++ ;
      }
    }
  }
  return (L_ret);
}


int C_ChannelControl::open_global_channel () {

  int            L_ret = 0 ;
  T_OpenStatus   L_status  ;
  int            L_i       ;
  T_pChannelData L_data    ;

  for(L_i=0; L_i < m_channel_table_size; L_i++) {
    L_data = m_channel_table[L_i] ;
    if ((L_data->m_open_status == E_CHANNEL_CLOSED)
	&& (L_data->m_type == E_CHANNEL_GLOBAL)) {
      L_data->m_open_id
	= (L_data->m_transport)
	->open(L_data->m_id,
	       L_data->m_open_args, 
	       &L_status,
	       L_data->m_protocol);
      GEN_DEBUG(0, "open channel [" << L_i << "] with [" 
		<< L_data->m_open_id << "]");

      switch (L_status) {
      case E_OPEN_OK:
	L_ret ++ ; 
	L_data->m_open_status = E_CHANNEL_OPENED ;
	break ;
      case E_OPEN_DELAYED:
	//	L_ret ++ ; 
	L_data->m_open_status = E_CHANNEL_OPEN_IN_PROGESS ;
	break ;
      default:
	break;
      }
    }
  }
  return (L_ret);
}


int C_ChannelControl::open_local_channel (int P_id,
                                          char *P_args,
                                          int *P_table, 
                                          T_pOpenStatus P_status) {
  int            L_ret  = 0       ;
  T_pChannelData L_data           ;

  char          *L_args = NULL    ;

  if (P_id < m_channel_table_size) {
    L_data = m_channel_table [P_id] ;

    if ((L_data->m_open_status == E_CHANNEL_CLOSED)
	&& (L_data->m_type == E_CHANNEL_LOCAL)) {
      if (P_args != NULL ) {
        L_args = P_args ;
      } else {
        L_args =  L_data->m_open_args ;
      }
      L_ret
	= (L_data->m_transport)
	->open(L_data->m_id,
	       L_args, 
	       P_status,
	       L_data->m_protocol);
      P_table[P_id] = L_ret ;
    }
  }
    
  return (L_ret);
}

void C_ChannelControl::create_context_channel() {

  int L_nb_local_channel, L_i, L_j ;

  L_nb_local_channel = m_channel_table_size - m_nb_global_channel ;
  if (L_nb_local_channel > 0) {
    ALLOC_TABLE(m_channel_to_ctxt_table,
		int*,
		sizeof(int),
		m_channel_table_size);
    ALLOC_TABLE(m_ctxt_to_channel_table,
		int*,
		sizeof(int),
		L_nb_local_channel);
    L_j = 0 ;
    for (L_i = 0; L_i < m_channel_table_size; L_i++) {
      if (m_channel_table[L_i]->m_type == E_CHANNEL_LOCAL) {
	m_ctxt_to_channel_table[L_j] = L_i ;
	m_channel_to_ctxt_table[L_i] = L_j ;
      } else {
	m_channel_to_ctxt_table[L_i] = -1 ;
      }
    }
  }
}

int C_ChannelControl::nb_channel () {
  return (m_channel_table_size) ;
}

int C_ChannelControl::nb_global_channel () {
  return (m_nb_global_channel) ;
}

int C_ChannelControl::open_id_from_channel_id (int P_id, int *P_table) {
  int L_ret = -1 ;

  if (P_id < m_channel_table_size) {
    if (m_channel_table[P_id]->m_type == E_CHANNEL_LOCAL) {
      L_ret = P_table[m_channel_to_ctxt_table[P_id]];
    } else {
      L_ret = m_channel_table[P_id]->m_open_id ;
    }
  }
  return (L_ret);
}

int C_ChannelControl::send_to_channel(int P_id, int *P_table, 
				      C_MessageFrame *P_msg) {
  
  int          L_ret       ;

  if (P_table[P_id] == -1) {
    P_table[P_id] = m_channel_table[P_id]->m_open_id ;
  }

  L_ret = (m_channel_table[P_id]->m_transport)->send_message(P_table[P_id], 
							     P_msg);
  
  return (L_ret) ;
}

void C_ChannelControl::create_transport_table(C_TransportControl *P_transport_ctrl) {
  
  T_pIntSet          L_set ;
  T_IntSet::iterator L_it ;
  int                L_i, L_id ;
  
  NEW_VAR(L_set, T_IntSet());

  for(L_i = 0; L_i < m_channel_table_size; L_i++) {
    L_id = m_channel_table[L_i]->m_transport_id ;
    L_it = L_set->find(L_id) ;
    if (L_it == L_set->end()) { L_set->insert(L_id); }
  }

  if (!L_set->empty()) {
    m_nb_transport = L_set->size() ;
    ALLOC_TABLE(m_transport_table,
		C_Transport**,
		sizeof(C_Transport*),
		m_nb_transport);
    L_i = 0 ;
    for(L_it=L_set->begin(); L_it != L_set->end() ; L_it++) {
      m_transport_table[L_i] 
	= P_transport_ctrl->get_transport(*L_it);
      L_i++;
    }
    L_set->erase(L_set->begin(), L_set->end());
  }
  DELETE_VAR(L_set);
}

C_Transport** C_ChannelControl::get_transport_table (int *P_size) {
  *P_size = m_nb_transport ;
  return (m_transport_table);
}

C_ProtocolFrame* C_ChannelControl::get_channel_protocol (int P_id) {
  C_ProtocolFrame *L_ret = NULL ;
  if (P_id < m_channel_table_size) {
    L_ret = m_channel_table[P_id]->m_protocol ;
  }
  return (L_ret) ;
}

void C_ChannelControl::close_local_channel (int P_id, int *P_table) {

  int            L_id ;

  if (m_channel_table[P_id]->m_type == E_CHANNEL_LOCAL) {

    L_id = P_table[P_id] ;
    if (L_id != -1) {
      (m_channel_table[P_id]->m_transport)->close(L_id);
    }
  }

}


void C_ChannelControl::reset_channel(int *P_table) {

  int L_i ;

  for (L_i=0; L_i < m_channel_table_size; L_i++) {
    P_table[L_i] = -1 ;
  }
  
}

void C_ChannelControl::opened(int P_id, int P_open_id) {
  if (m_channel_table[P_id]->m_open_id == P_open_id) {
    m_channel_table[P_id]->m_open_status 
      = E_CHANNEL_OPENED ;
  }
}

void C_ChannelControl::open_failed(int P_id, int P_open_id) {
  T_pChannelData L_data = m_channel_table[P_id] ;

  if (L_data) {
    if (L_data->m_open_id == P_open_id) {
      L_data->m_open_status 
	= E_CHANNEL_CLOSED ;
      (L_data->m_transport)
	->close (L_data->m_open_id) ;
      GEN_ERROR(E_GEN_FATAL_ERROR, "Open channel failed");
    }
  }
}

void C_ChannelControl::closed(int P_id, int P_open_id) {

  T_pChannelData L_data = m_channel_table[P_id] ;

  if (L_data) {
    if (L_data->m_open_id == P_open_id) {
      L_data->m_open_status = E_CHANNEL_CLOSED ;
      (L_data->m_transport)
	->close (L_data->m_open_id) ;
    }
  }
}

int C_ChannelControl::opened () {

  int            L_nb = 0 ;
  int            L_i       ;
  T_pChannelData L_data    ;

  for(L_i=0; L_i < m_channel_table_size; L_i++) {
    L_data = m_channel_table[L_i] ;
    if ((L_data->m_open_status == E_CHANNEL_OPENED)
	&& (L_data->m_type == E_CHANNEL_GLOBAL)) {
      L_nb ++ ;
    }
  }
  return (L_nb);
}


int C_ChannelControl::set_option_global_channel (int P_id,
                                                 char *P_args,
                                                 int *P_table) {
  int            L_ret  = 0       ;
  T_pChannelData L_data           ;


  if (P_id < m_channel_table_size) {
    L_data = m_channel_table [P_id] ;

    // global channel
    L_ret = (L_data->m_transport)
      ->set_option(P_table[P_id],
                   P_args);

  }
    
  return (L_ret);
}

//  int C_ChannelControl::set_option_global_channel (int P_Channel_Id, char *P_buf) {
//    int L_ret = 0 ;
//    // global channel
//    L_ret = (m_channel_table[P_Channel_Id]->m_transport)
//      ->set_option(m_channel_table[P_Channel_Id]->m_open_id, 
//                   P_buf);
  
//    return (L_ret);
//  }
