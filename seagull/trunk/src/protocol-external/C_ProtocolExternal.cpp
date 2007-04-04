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

#include "C_ProtocolExternal.hpp"
#include "C_MessageExternal.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "dlfcn_t.hpp"
#include "Utils.hpp"

#include "ProtocolData.hpp"

#include "set_t.hpp"


C_ProtocolExternal::C_ProtocolExternal
(C_TransportControl   *P_transport_control,
 C_XmlData            *P_def,
 char                **P_name,
 T_pConfigValueList    P_config_value_list,
 T_ConstructorResult  *P_res) : C_ProtocolExternalFrame() {

  T_ConstructorResult  L_res = E_CONSTRUCTOR_KO ;
  char                *L_library_name = NULL ;
  char                *L_use_transport_name = NULL ;
  int                  L_transport_id ;
  C_TransportControl::T_pTransportContext L_transport_ctxt ;


  m_factory_info.m_create = NULL ;
  m_factory_info.m_delete = NULL ;
  m_factory = NULL ;

  m_library_handle = NULL ;

  m_field_body_name_map = NULL;
  m_field_name_map = NULL ;
  m_body_value_name_map = NULL ;
  m_body_decode_map = NULL ;


  m_id_counter = -1 ;
  m_start_body_index = -1 ;
  m_end_header_index = -1 ;
  
  m_header_defaults = NULL ;
  m_body_defaults = NULL ;

  m_nb_body_values = 0 ;

  m_session_id       = -1 ;
  m_outof_session_id = -1 ;

  m_message_map = NULL ;
  

  m_header_field_desc_table = NULL    ;
  m_body_field_desc_table   = NULL    ;
  m_body_value_table        = NULL    ;
  m_names_table             = NULL    ;
  m_message_names_table     = NULL    ;
  m_nb_names                = 0       ;
  m_nb_message_names        = 0       ;
  m_message_decode_map       = NULL   ;


  m_body_not_present_table = NULL ;
  m_header_not_present_table = NULL ;

  m_from_string_table = NULL ;
  m_to_string_table = NULL ;
  m_nb_from_string = 0 ;


  m_from_string_field_body_table = NULL ;
  m_to_string_field_body_table   = NULL ;
  m_nb_from_string_field_body    = 0    ;


  m_config_value_list = P_config_value_list ;


  NEW_VAR (m_message_name_list, T_NameAndIdList()) ;
  NEW_VAR (m_message_comp_name_list, T_NameAndIdList()) ;


  if (P_def != NULL) {

    *P_name = P_def->find_value((char *)"name");
    if (*P_name == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No name for external protocol definition");
    } else {
      m_name = *P_name ;
      L_library_name = P_def->find_value((char *)"library");
      L_use_transport_name = P_def->find_value((char*)"use-transport-library");

      if ((L_library_name == NULL) && (L_use_transport_name == NULL)) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Definition of protocol ["
		  << *P_name << "] with no library or use-transport-library field" );
      } else {

	if (L_use_transport_name != NULL) {
	  // library already loaded from transport lib
	  L_transport_id = P_transport_control->get_transport_id (L_use_transport_name);
	  if (L_transport_id != ERROR_TRANSPORT_UNKNOWN) {
	    L_transport_ctxt = P_transport_control->get_transport_context(L_transport_id);
	    m_library_handle = L_transport_ctxt->m_lib_handle ;
	  } else {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "External protocol ["
		      << *P_name << "] defined with library transport ["
		      << L_use_transport_name << "] unknown");
	  }

	} else {
	  // direct library loading
	  m_library_handle = dlopen(L_library_name, RTLD_LAZY);
	  if (m_library_handle == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "Unable to open library file [" 
		      << L_library_name
		      << "] error [" << dlerror() << "]");
	  }
	}

	if (m_library_handle != NULL) {
	  if (xml_analysis(P_def, P_name,P_config_value_list) == 0) {
	    L_res = E_CONSTRUCTOR_OK ;
	  }
	}
      }
    }
      
  } else {

    GEN_ERROR(E_GEN_FATAL_ERROR, "C_ProtocolExternal() called with NULL Xml definition");
    
  }

  *P_res = L_res ;

}

C_ProtocolExternal::~C_ProtocolExternal() {

  int  L_i ;


  if (m_factory != NULL) {
    if (m_factory_context != NULL) {
      m_factory->delete_context(&m_factory_context);
    }
    if (m_factory_info.m_delete != NULL) {
      (*m_factory_info.m_delete)(&m_factory);
    }
  }


  // reset m_config_field ??
  if (m_header_field_desc_table != NULL) {
    for (L_i = 0; L_i < m_nb_header_fields ; L_i++) {
      if ((m_header_field_desc_table[L_i])->m_config_field_name != NULL) {
	(m_header_field_desc_table[L_i])->m_config_field_name = NULL ;
      }
    }
    FREE_TABLE(m_header_field_desc_table) ;
  }

  FREE_TABLE(m_body_field_desc_table)   ;
  FREE_TABLE(m_body_value_table)   ;


  if ((m_names_table != NULL) && (m_nb_names > 0)) {
    for(L_i = 0; L_i < m_nb_names; L_i++) {
      FREE_TABLE(m_names_table[L_i]);
    }
    FREE_TABLE(m_names_table);
  }


  if (m_message_names_table != NULL) {
    for(L_i = 0; L_i < m_nb_message_names; L_i++) {
      FREE_TABLE(m_message_names_table[L_i]);
    }
    FREE_TABLE(m_message_names_table);
  }


  // delete m_message_decode_map 
  if (m_message_decode_map != NULL) {
    if (!m_message_decode_map->empty()) {
      m_message_decode_map->erase(m_message_decode_map->begin(),
			      m_message_decode_map->end());
    }
  }

  DELETE_VAR(m_message_decode_map);

  // delete m_field_name_map 
  if (m_field_name_map != NULL) {
    if (!m_field_name_map->empty()) {
      m_field_name_map->erase(m_field_name_map->begin(),
			      m_field_name_map->end());
    }
    DELETE_VAR(m_field_name_map);
  }

  // delete m_body_value_name_map
  if (m_body_value_name_map != NULL) {
    if (!m_body_value_name_map->empty()) {
      m_body_value_name_map->erase(m_body_value_name_map->begin(),
				   m_body_value_name_map->end());
    }
    DELETE_VAR(m_body_value_name_map);
  }
    
  // delete m_field_body_name_map
  if (m_field_body_name_map != NULL) {
    if (!m_field_body_name_map->empty()) {
      m_field_body_name_map->erase(m_field_body_name_map->begin(),
				   m_field_body_name_map->end());
    }
    DELETE_VAR(m_field_body_name_map);
  }


  // delete m_field_body_name_map
  if (m_body_decode_map != NULL) {
    if (!m_body_decode_map->empty()) {
      m_body_decode_map->erase(m_body_decode_map->begin(),
				   m_body_decode_map->end());
    }
  }

  DELETE_VAR(m_body_decode_map);


  // delete m_message_map
  if (m_message_map != NULL) {
    if (!m_message_map->empty()) {
      m_message_map->erase(m_message_map->begin(),
			   m_message_map->end());
    }
  }

  DELETE_VAR(m_message_map);

  if (m_body_not_present_table != NULL ) {
    for (L_i = 0; L_i <m_nb_body_values ; L_i++) {
      FREE_TABLE (m_body_not_present_table[L_i]);
    }
    FREE_TABLE (m_body_not_present_table) ;
  }

  if (m_header_not_present_table != NULL ) {
    for (L_i = 0; L_i <m_nb_header_fields ; L_i++) {
      FREE_TABLE (m_header_not_present_table[L_i]);
    }
    FREE_TABLE (m_header_not_present_table) ;
  }

  FREE_TABLE(m_from_string_table);
  FREE_TABLE(m_to_string_table);
  m_nb_from_string = 0 ;


  DELETE_VAR(m_stats);


  FREE_TABLE(m_from_string_field_body_table);
  FREE_TABLE(m_to_string_field_body_table);
  m_nb_from_string_field_body = 0 ;


  if(! m_message_name_list -> empty()) {
    T_NameAndIdList::iterator  L_elt_it ;
    T_NameAndId                L_elt    ;

    for(L_elt_it=m_message_name_list->begin();
	L_elt_it != m_message_name_list->end();
	L_elt_it++) {
      L_elt = *L_elt_it ;
      FREE_TABLE(L_elt.m_name);
    }
    m_message_name_list -> erase (m_message_name_list->begin(), m_message_name_list->end());
  }
  DELETE_VAR (m_message_name_list) ;

  if(! m_message_comp_name_list -> empty()) {
    T_NameAndIdList::iterator  L_elt_it ;
    T_NameAndId                L_elt    ;
    
    for(L_elt_it=m_message_comp_name_list->begin();
	L_elt_it != m_message_comp_name_list->end();
	L_elt_it++) {
      L_elt = *L_elt_it ;
      FREE_TABLE(L_elt.m_name);
    }
    m_message_comp_name_list -> erase (m_message_comp_name_list->begin(), 
				       m_message_comp_name_list->end());
  }
  DELETE_VAR (m_message_comp_name_list) ;


  m_config_value_list = NULL ;

  
}


C_MessageFrame* C_ProtocolExternal::create_new_message (C_MessageFrame *P_msg) {
  C_MessageExternal *L_dest = NULL ;
  C_MessageExternal *L_source = dynamic_cast<C_MessageExternal *>(P_msg) ;
  
  if (L_source != NULL) {
    // L_source->dump(std::cerr);
    //    NEW_VAR(L_dest, C_MessageExternal());
    //    *L_dest = *L_source ;
    NEW_VAR(L_dest, C_MessageExternal(*L_source));
  }
  return (L_dest);
}

C_MessageFrame* C_ProtocolExternal::create_new_message (void                *P_xml, 
							T_pInstanceDataList  P_list,
							int                 *P_nb_value) {
  // xml message analysis 
  C_XmlData         *P_data = (C_XmlData *)P_xml ;
  T_FieldHeaderList *L_field_list = NULL ;
  int                L_ret = 0 ;
  C_MessageExternal *L_msg = NULL ;
  T_MessageNameMap::iterator L_it ;

  if (P_data != NULL) {
    L_field_list = analyze_header_value(P_data, L_field_list, false, &L_ret);
  }
  
  if (L_ret != -1) {
    if (L_field_list != NULL) {
      if (!L_field_list->empty()) {
	L_it = m_message_map->find(T_MessageNameMap::key_type((*(L_field_list->begin())).m_name));
	if (L_it == m_message_map->end()) {
	  GEN_ERROR(E_GEN_FATAL_ERROR,
		    "Unknown message [" << (*(L_field_list->begin())).m_name << "] in dictionnary");
	} else {
	  L_msg = build_message(L_it->second, *(L_field_list->begin()), P_list, P_nb_value);
	  if (L_msg != NULL) {
	    L_msg -> m_id = L_it->second->m_id ;


	  } 
	}
      }
    }
  }
  
  return (L_msg);
}

char*           C_ProtocolExternal::message_name       () {
  return (m_message_name);
}


char*           C_ProtocolExternal::message_component_name () {
  return (m_body_name);
}

T_pNameAndIdList C_ProtocolExternal::message_name_list     () {
  T_MessageNameMap::iterator   L_it_message              ;
  T_NameAndId                  L_elt                     ;
  int                          L_id                      ;

  L_id = 0 ;
  for (L_it_message = m_message_map->begin();
       L_it_message != m_message_map->end();
       L_it_message++) {

    
    ALLOC_TABLE(L_elt.m_name,
  		char*, sizeof(char),
  		((L_it_message->first).length())+1);
    
    strcpy(L_elt.m_name,(L_it_message->first).c_str()) ;
    L_elt.m_id = L_id ;
    m_message_name_list->push_back(L_elt);
    L_id ++ ;      
  }

  return (m_message_name_list);
}

T_pNameAndIdList C_ProtocolExternal::message_component_name_list    () {
  T_FieldBodyNameMap::iterator L_it_body                 ;
  T_NameAndId                  L_elt                     ;

  for (L_it_body = m_body_value_name_map->begin();
       L_it_body != m_body_value_name_map->end();
       L_it_body++) {

    ALLOC_TABLE(L_elt.m_name,
	       char*, sizeof(char),
		strlen((L_it_body->second)->m_name)+1);
    
    strcpy(L_elt.m_name,(L_it_body->second)->m_name) ;
    
    L_elt.m_id = (L_it_body->second)->m_id - m_end_header_index -1 ;
    
    m_message_comp_name_list->push_back(L_elt);

  }


  return (m_message_comp_name_list);

}


int             C_ProtocolExternal::find_field         (char *P_name) {
  int L_id ;

  for(L_id = 0 ; L_id < m_nb_names; L_id++) {

    if (strcmp(m_names_table[L_id], P_name)==0) {
      return (L_id);
    }
  }
  return (-1);
}

T_TypeType      C_ProtocolExternal::get_field_type     (int P_id,
							int P_sub_id) {

  //  std::cerr << "P_id = " << P_id << " P_sub_id = " << P_sub_id << " val= " << P_id - m_start_body_index << std::endl ;
  if (P_id < m_nb_header_fields) {
    return(m_header_field_desc_table[P_id]->m_type) ;
  } else {
    return (m_body_field_desc_table[P_sub_id - m_start_body_index]->m_type);
  }

}

C_ProtocolFrame::T_MsgError C_ProtocolExternal::from_external 
(C_MsgBuildContext* P_build,T_pReceiveMsgContext P_recvMsgCtx) {

  C_ProtocolFrame::T_MsgError L_error = C_ProtocolFrame::E_MSG_OK;
  
  int                              L_i               ;
  T_pValueData                     L_header          ;

  int                             *L_body_instance   ;
  T_pValueData                     L_a_body          ;
  T_ValueDataList                  L_body            ;
  list_t<int>                      L_body_id         ;

  T_BodyDecodeMap::iterator        L_it              ;

  int                              L_id              ;
  int                              L_msg_id          ;

  T_ValueDataList::iterator       L_it_body          ;

  T_MessageDecodeMap::iterator     L_msg_it          ;
  C_MessageExternal               *L_msg = NULL      ;


  P_recvMsgCtx->m_msg = NULL ;

  P_build->init_from_external();

  ALLOC_TABLE(L_header, T_pValueData, sizeof(T_ValueData), m_nb_header_fields);
  ALLOC_TABLE(L_body_instance, int*, sizeof(int), 
	      m_nb_body_values);
  for (L_i = 0 ; L_i < m_nb_body_values; L_i ++) {
    L_body_instance[L_i] = 0 ;
  }

  L_header[m_type_id].m_id = m_type_id ;
  L_header[m_type_id].m_type 
    = m_header_field_desc_table[m_type_id]->m_type ;
  
  ((P_build)->*(m_header_field_desc_table[m_type_id]->m_get))
    (&L_header[m_type_id]);
  
  // To be improved: the type of the message can be in the header or the body
  L_msg_it = m_message_decode_map
    ->find(T_MessageDecodeMap::key_type(L_header[m_type_id]));

  if (L_msg_it != m_message_decode_map->end()) {

    L_msg_id = (L_msg_it->second)->m_id ;

    if (m_stats) {
      m_stats->updateStats (E_MESSAGE,
			    E_RECEIVE,
			    L_msg_id);
    }

    for(L_i = 0; L_i < m_nb_header_fields; L_i++) {
      
      if (m_header_not_present_table[L_msg_id][L_i] == true) {
	
	L_header[L_i].m_id = L_i ;
	L_header[L_i].m_type = m_header_field_desc_table[L_i]->m_type ;
	
	
	if (m_header_field_desc_table[L_i]->m_check_get 
	    != (C_MsgBuildContext::T_ContextFunction)NULL) {
	  if ( ((P_build)->*(m_header_field_desc_table[L_i]->m_check_get))
	       (&(L_header[L_i])) == true ) {
	    ((P_build)->*(m_header_field_desc_table[L_i]->m_get))(&(L_header[L_i]));
	  }
	} else {
	  ((P_build)->*(m_header_field_desc_table[L_i]->m_get))(&(L_header[L_i]));
	}
      }
    }
    
    while ((((P_build)->*(m_get_body))(NULL)) == true) {
      
      ALLOC_TABLE(L_a_body, T_pValueData, 
		  sizeof(T_ValueData), m_nb_body_fields);
      L_body.push_back(L_a_body);
      
      // decode first body type
      L_a_body[m_body_type_id-m_start_body_index].m_id = m_body_type_id ;
      L_a_body[m_body_type_id-m_start_body_index].m_type 
  	= m_body_field_desc_table[m_body_type_id-m_start_body_index]->m_type ;
      
      ((P_build)->*(m_body_field_desc_table[m_body_type_id-m_start_body_index]->m_get))
  	(&L_a_body[m_body_type_id-m_start_body_index]);
      L_it = m_body_decode_map
  	->find(T_BodyDecodeMap::key_type(L_a_body[m_body_type_id-m_start_body_index]));

      if (L_it != m_body_decode_map->end()) {
  	L_id = L_it->second - m_end_header_index - 1 ;

	if (m_stats) {
	  m_stats->updateStats (E_MESSAGE_COMPONENT,
				E_RECEIVE,
				L_id);
	}

	L_body_instance[L_id] ++ ;
	L_body_id.push_back(L_id);
      } else {
  	// error on decoding body
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                      "Error on decoding body for message [" <<
                      m_message_names_table[L_msg_id] <<
                      "] " << 
                      GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "]" );
        L_error = C_ProtocolFrame::E_MSG_EXTERNAL_ERROR;
  	break ;
      }
      
      
      // now decode all the other fields
      for (L_i = 0; L_i < m_nb_body_fields; L_i++) {
	if (L_i != (m_body_type_id-m_start_body_index)) {
	  if (m_body_not_present_table[L_id][L_i] == true) {
	    L_a_body[L_i].m_id = L_i + m_start_body_index ;
	    L_a_body[L_i].m_type = m_body_field_desc_table[L_i]->m_type ;

	    ((P_build)->*(m_body_field_desc_table[L_i]->m_get))(&L_a_body[L_i]);
	  } else {
	    L_a_body[L_i].m_type = E_TYPE_NUMBER ;
	    // temporary TO DO m_id = -1
	    L_a_body[L_i].m_id = 0 ;
	  }
	}
      }
    
    } // while 

    if (L_error == C_ProtocolFrame::E_MSG_OK) {
      NEW_VAR(L_msg, 
              C_MessageExternal(this, L_header, &L_body, &L_body_id, L_body_instance));
      
      L_msg->m_id =  L_msg_id ;
        // (L_msg_it->second)->m_id 
      // TODO: delete primitive and component ?????
      ((P_build)->*(m_delete_body))(NULL) ;
      ((P_build)->*(m_delete_header))(NULL) ;
      
      P_recvMsgCtx->m_msg = L_msg ;
    }
  } else {
    ((P_build)->*(m_delete_body))(NULL) ;
    ((P_build)->*(m_delete_header))(NULL) ;
    FREE_TABLE(L_header);
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                  "message unknown" <<
                  GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "]" );
    
    
    L_error = C_ProtocolFrame::E_MSG_EXTERNAL_ERROR;
    iostream_error << "message unknown" << iostream_endl ;
  }
  

  FREE_TABLE(L_body_instance);
  if (!L_body.empty()) {
    if (L_error != C_ProtocolFrame::E_MSG_OK ) {
      for (L_it_body = L_body.begin() ;
           L_it_body != L_body.end()  ;
           L_it_body ++) {
        FREE_TABLE(*L_it_body);
      }
    }
    L_body.erase(L_body.begin(), L_body.end());
    L_body_id.erase(L_body_id.begin(), L_body_id.end());
  }

  return (L_error);
}

C_ProtocolFrame::T_MsgError C_ProtocolExternal::to_external   (C_MsgBuildContext* P_build, C_MessageFrame* P_msg) {

  int               L_i,  L_j ;
  T_pValueData      L_header ;
  C_MessageExternal *L_msg =  dynamic_cast<C_MessageExternal*>(P_msg);

  //  T_BodyDecodeMap::iterator L_it ;
  int                       L_id ;
  int                       L_msg_id ;


  ((P_build)->*(m_create_header))(NULL) ;

  P_build->init_to_external();

  L_header = L_msg->m_header ;
  L_msg_id = L_msg->m_id     ;
  
  if (m_stats) {
    m_stats->updateStats(E_MESSAGE,
			 E_SEND,
			 L_msg_id);
  }

  for(L_i = 0; L_i < m_nb_header_fields; L_i++) {
    if (m_header_not_present_table[L_msg_id][L_i] == true) {
      L_header[L_i].m_id = L_i ;
      L_header[L_i].m_type = m_header_field_desc_table[L_i]->m_type ;

      if (m_header_field_desc_table[L_i]->m_check_set 
  	  != (C_MsgBuildContext::T_ContextFunction)NULL) {
  	if ( ((P_build)->*(m_header_field_desc_table[L_i]->m_check_set))
  	     (&(L_header[L_i])) == true ) {
  	  ((P_build)->*(m_header_field_desc_table[L_i]->m_set))(&(L_header[L_i]));
  	}
      } else {
  	((P_build)->*(m_header_field_desc_table[L_i]->m_set))(&(L_header[L_i]));
      }

      //   ((P_build)->*(m_header_field_desc_table[L_i]->m_set))(&(L_header[L_i]));
    }
  }
  
  for (L_i = m_nb_header_fields ; L_i < L_msg->m_nb_values ; L_i++ ) {
    ((P_build)->*(m_create_body))(NULL) ;

    L_id = L_msg->m_ids[L_i] ;

    if (m_stats) {
      m_stats->updateStats (E_MESSAGE_COMPONENT,
			    E_SEND,
			    L_id);
    }
    
    for (L_j = 0; L_j < m_nb_body_fields ; L_j++) {
      if (m_body_not_present_table[L_id][L_j] == true) {
	L_msg->m_all_values[L_i][L_j].m_type 
	  = m_body_field_desc_table[L_j]->m_type ;
	((P_build)->*(m_body_field_desc_table[L_j]->m_set))
	  (&(L_msg->m_all_values[L_i][L_j]));
	}
      }
    ((P_build)->*(m_add_body))(NULL) ;
  }

  return (C_ProtocolFrame::E_MSG_OK);
}

// Private methods

int C_ProtocolExternal::xml_analysis (C_XmlData *P_data, char **P_name,
				      T_pConfigValueList P_config_value_list) {

  int                      L_ret = 0 ;
  char                    *L_factory_create_symbol = NULL ;
  char                    *L_factory_delete_symbol = NULL ;
  T_pXmlData_List          L_data = NULL ;
  T_pXmlData_List          L_data_dico = NULL ;
  T_XmlData_List::iterator L_it ;
  bool                     L_found = false ;

  T_pFieldDefList          L_header_fields = NULL ;
  T_pFieldDefList          L_body_fields = NULL ;

  char                    *L_header_type = NULL ;
  char                    *L_body_type = NULL ;
  char                    *L_header_create_function = NULL ;
  char                    *L_body_create_function = NULL ;

  char                    *L_header_delete_function = NULL ;
  char                    *L_body_delete_function = NULL ;

  char                    *L_body_add_function = NULL ;
  char                    *L_body_get_function = NULL ;
  char                    *L_value_field = NULL ;

  C_XmlData                    *L_dataDico = NULL ;
  char                         *L_session_id_name = NULL ;
  char                         *L_outof_session_id_name = NULL ;

  T_pFieldHeaderList            L_header_fields_dico = NULL ;
  T_pFieldBodyList              L_body_fields_dico = NULL ;
  
  T_pParamDefList               L_config_params_dico = NULL ;
  T_ParamDefList::iterator      L_config_params_dico_it     ;
  

  L_factory_create_symbol = P_data->find_value((char *)"context-factory-constructor");
  L_factory_delete_symbol = P_data->find_value((char *)"context-factory-destructor");

  if (L_factory_create_symbol == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Field [context-factory-constructor] not found");
    L_ret = -1 ;
  }

  if (L_factory_delete_symbol == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Field [context-factory-destructor] not found");
    L_ret = -1 ;
  }

  if (L_ret != -1) {
    m_factory_info.m_create =
      (T_CreateContextFactory) dlsym(m_library_handle, L_factory_create_symbol);
    if (m_factory_info.m_create == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Symbol [" << dlerror() << "]");
      L_ret = -1 ;
    }
  }

  if (L_ret != -1) {
    m_factory_info.m_delete =
      (T_DeleteContextFactory) dlsym(m_library_handle, L_factory_delete_symbol);
    if (m_factory_info.m_delete == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Symbol [" << dlerror() << "]");
      L_ret = -1 ;
    }
  }

  if (L_ret != -1) {
    m_factory = (*m_factory_info.m_create)() ;
    if (m_factory == NULL) {
      L_ret = -1;
    } else {
      m_factory_context = m_factory->create_context() ;
    }
  }

  // xml dictionnary analysis
  if (L_ret != -1) {
    L_data = P_data->get_sub_data();
    if (L_data == NULL){
      L_ret = -1 ;
    } 
  }
   
  // types analysis => to be done later / check compatibility
  // with other protocols

  // analyze presence of configuration-parameters in dico
  if (L_ret != -1) {
    for (L_it = L_data->begin();
	 L_it != L_data->end();
	 L_it++) {
      if (strcmp((*L_it)->get_name(), "configuration-parameters") == 0) {
	L_found = true ;
	NEW_VAR(L_config_params_dico, T_ParamDefList());
	L_ret = xml_configuration_parameters (*L_it, L_config_params_dico);
	break ;
      }
    }


    if (L_ret != -1) {
      if (L_found == true) {
	L_found = false ;
	if ((L_config_params_dico != NULL ) && (!L_config_params_dico->empty())) {
	  m_nb_config_params = L_config_params_dico->size();
	  if (m_nb_config_params != 0) {
	    // ctrl between parameters from config and dico
	    for (L_config_params_dico_it = L_config_params_dico->begin();
		 L_config_params_dico_it != L_config_params_dico->end();
		 L_config_params_dico_it++) {
	      L_ret = update_config_params (*L_config_params_dico_it, P_config_value_list) ;
	      if (L_ret == -1) { break;}
	    }

	  }
	
	} else {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Error in  configuration parameters definition for protocol ["
		    << *P_name << "]");
	  L_ret = -1 ;
	}
      
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "No configuration parameters definition found for protocol ["
		  << *P_name << "]");
	L_ret = -1 ;
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Error in configuration parameters definition found for protocol ["
		  << *P_name << "]");
	L_ret = -1 ;
    }
  }

  // header analysis
  if (L_ret != -1) {
    for (L_it = L_data->begin();
	 L_it != L_data->end();
	 L_it++) {
      if (strcmp((*L_it)->get_name(), "header") == 0) {
	L_found = true ;
	NEW_VAR(L_header_fields, T_FieldDefList());
	L_ret = xml_fields (*L_it, L_header_fields, 
			    &m_message_name,
			    &L_header_type,
			    &L_header_create_function,
			    &L_header_delete_function,
			    NULL,
			    NULL,
			    NULL,
			    false) ;
	break ;
      }
    }
  }

  if (L_found == true) {
    L_found = false ;
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, "No header definition found for protocol ["
	      << *P_name << "]");
    L_ret = -1 ;
  }

  // body analysis
  if (L_ret != -1) {
    for (L_it = L_data->begin();
	 L_it != L_data->end();
	 L_it++) {
      if (strcmp((*L_it)->get_name(), "body") == 0) {
	L_found = true ;
	NEW_VAR(L_body_fields, T_FieldDefList());
	L_ret = xml_fields (*L_it, L_body_fields, 
			    &m_body_name,
			    &L_body_type,
			    &L_body_create_function,
			    &L_body_delete_function,
			    &L_body_add_function,
			    &L_body_get_function,
			    &L_value_field,
			    true) ;
	break ;
      }
    }
  }

  if (L_found == true) {
    L_found = false ;
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, "No body definition found for protocol ["
	      << *P_name << "]");
    L_ret = -1 ;
  }
  
  // dico analysis
  if (L_ret != -1) {
    for (L_it = L_data->begin();
	 L_it != L_data->end();
	 L_it++) {
      
      if (strcmp((*L_it)->get_name(), "dictionary") == 0) {
	L_found = true ;
	
	L_dataDico = *L_it ;
	if (L_dataDico != NULL) {
	  L_data_dico = L_dataDico->get_sub_data();
	}
	GEN_DEBUG(1, "dictionnary [" << L_dataDico->get_name() << "]");
	
	L_session_id_name = L_dataDico->find_value((char *)"session-id");
	if (L_session_id_name == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR,
		    "session-id value is mandatory for ["
		    << m_message_name  << "] section");
	  L_ret = -1 ;
	  break ;
	}

	GEN_DEBUG(1, "session-id [" << L_session_id_name << "]");
	
	L_outof_session_id_name 
	  = L_dataDico->find_value ((char *)"out-of-session-id") ;
	if (L_outof_session_id_name == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR,
		    "out-of-session-id value is mandatory for ["
		    <<  m_message_name << "] section");
	  L_ret = -1 ;
	  break ;
	}

	GEN_DEBUG(1, "out-of-session-id [" << L_outof_session_id_name << "]");
	GEN_DEBUG(1, "m_message_name [" << m_message_name << "]");
	GEN_DEBUG(1, "m_body_name [" << m_body_name << "]");
	break ;
	
      } // if (strcmp((*L_it)->get_name(), "dictionary") == 0)
    } // for (L_it = ...)
  } // if L_ret != -1

  if (L_dataDico == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "No dictionnary found (or no value) in protocol ["
	      << *P_name << "]");
    L_ret = -1 ;
  }

  // header values analysis => don t forget optionnal
  if (L_ret != -1) {
    for (L_it = L_data_dico->begin();
	 L_it != L_data_dico->end();
	 L_it++) {
      if (strcmp((*L_it)->get_name(), m_body_name) == 0) {
	L_body_fields_dico = analyze_body_value(*L_it, L_body_fields_dico, true, &L_ret);
	if (L_ret == -1) break ;
      }
    }
  }


  // body values analysis => probably none
  // header values analysis => don t forget optionnal
  if (L_ret != -1) {
    for (L_it = L_data_dico->begin();
	 L_it != L_data_dico->end();
	 L_it++) {
      if (strcmp((*L_it)->get_name(), m_message_name) == 0) {
	L_header_fields_dico = analyze_header_value(*L_it, L_header_fields_dico, true, &L_ret);
	if (L_ret == -1) break ;
      }
    }
  }

  if (L_value_field == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "No field data defined");
    L_ret = -1 ;
  } else {
    m_data_field_name = L_value_field ;
  }



  if (L_ret != -1) {
    if (L_header_create_function == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No create-function defined in header");
      L_ret = -1 ;
    } else {
      m_create_header = m_factory->get_function(L_header_create_function) ;
    }
  }

  if (L_ret != -1) {
    if (L_header_delete_function == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No delete-function defined in header");
      L_ret = -1 ;
    } else {
      m_delete_header = m_factory->get_function(L_header_delete_function) ;
    }
  }

  if (L_ret != -1) {
    if (L_body_create_function == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No create-function defined in body");
      L_ret = -1 ;
    } else {
      m_create_body = m_factory->get_function(L_body_create_function) ;
    }
  }

  if (L_ret != -1) {
    if (L_body_delete_function == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No delete-function defined in body");
      L_ret = -1 ;
    } else {
      m_delete_body = m_factory->get_function(L_body_delete_function) ;
    }
  }


  // ctrl add_body and get_body
  if (L_ret != -1) {
    if (L_body_add_function == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No add-function defined in body");
      L_ret = -1 ;
    } else {
      m_add_body = m_factory->get_function(L_body_add_function) ;
    }
  }

  if (L_ret != -1) {
    if (L_body_get_function == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No get-function defined in body");
      L_ret = -1 ;
    } else {
      m_get_body = m_factory->get_function(L_body_get_function) ;
    }
  }

  if (L_ret != -1) {


    L_ret = analyze_dictionnary(L_header_fields,
				L_body_fields,
				L_value_field,
				L_session_id_name,
				L_outof_session_id_name,
				L_body_fields_dico,
				L_header_fields_dico,
				L_header_type,
				P_config_value_list,
				L_body_type);

  }

  // delete list
  if (L_header_fields != NULL) {
    if (!L_header_fields->empty()) {
      L_header_fields->erase(L_header_fields->begin(),
			     L_header_fields->end());
    }
    DELETE_VAR(L_header_fields);
  }

  if (L_body_fields != NULL ) {
    if (!L_body_fields->empty()) {
      L_body_fields->erase(L_body_fields->begin(),
			   L_body_fields->end());
    }
    DELETE_VAR(L_body_fields);
  }

  if (L_body_fields_dico != NULL) {
    if (!L_body_fields_dico->empty()) {
      L_body_fields_dico->erase(L_body_fields_dico->begin(),
				L_body_fields_dico->end());
    }
    DELETE_VAR(L_body_fields_dico);
  }

  if (L_header_fields_dico != NULL ) {
    if (!L_header_fields_dico->empty()) {
      L_header_fields_dico->erase(L_header_fields_dico->begin(),
				  L_header_fields_dico->end());
    }
    DELETE_VAR(L_header_fields_dico);
  }

  if (L_config_params_dico != NULL) {
    if (!L_config_params_dico->empty()) {
      L_config_params_dico->erase(L_config_params_dico->begin(),
			     L_config_params_dico->end());
    }
    DELETE_VAR(L_config_params_dico);
  }


  return (L_ret);
}

int C_ProtocolExternal::xml_fields(C_XmlData *P_data, 
				   T_FieldDefList *P_fielddef_list,
				   char **P_section_name,
				   char **P_type,
				   char **P_create,
				   char **P_delete,
				   char **P_add,
				   char **P_get,
				   char **P_value_field,
				   bool   P_ctrl_body) {

  int                       L_ret = 0 ;

  T_pXmlData_List           L_fielddef_xml_list = NULL ;
  C_XmlData                *L_fielddef = NULL          ;
  T_XmlData_List::iterator  L_fielddef_it              ;

  T_FieldDef                L_fielddef_data            ;

  GEN_DEBUG(1, "C_ProtocolExternal::xml_fields() start");

  *P_section_name = P_data->find_value((char *)"name");
  if (*P_section_name == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "name definition value is mandatory for header section");
    L_ret = -1 ;
  }

  *P_type   = P_data->find_value((char *)"type");
  if (*P_type == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "type definition value is mandatory for header section");
    L_ret = -1 ;
  }

  *P_create = P_data->find_value((char *)"create-function");
  if (*P_create == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "create-function definition value is mandatory for header section");
    L_ret = -1 ;
  }

  *P_delete = P_data->find_value((char *)"delete-function");
  if (*P_delete == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "delete-function definition value is mandatory for header section");
    L_ret = -1 ;
  }



  if (P_ctrl_body == true) {

    *P_value_field = P_data->find_value((char *)"value-field");
    if (*P_value_field == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
		"value-field definition value is mandatory for header section");
      L_ret = -1 ;
    }

    *P_add = P_data->find_value((char *)"add-function");
    if (*P_add == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
		"add-function definition value is mandatory for header section");
      L_ret = -1 ;
    }
    
    *P_get = P_data->find_value((char *)"get-function");
    if (*P_get == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
		"get-function definition value is mandatory for header section");
      L_ret = -1 ;
    }
  }

  L_fielddef_xml_list = P_data->get_sub_data() ;

  for(L_fielddef_it  = L_fielddef_xml_list->begin() ;
      L_fielddef_it != L_fielddef_xml_list->end() ;
      L_fielddef_it++) {

    L_fielddef = *L_fielddef_it ;

    if (strcmp(L_fielddef->get_name(), (char*)"fielddef") == 0) {

      L_fielddef_data.m_name = L_fielddef->find_value((char*)"name") ;
      if (L_fielddef_data.m_name == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "fielddef name value is mandatory");
	L_ret = -1 ;
      }
      L_fielddef_data.m_type = L_fielddef->find_value((char*)"type") ;
      if (L_fielddef_data.m_type == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "fielddef type value is mandatory");
	L_ret = -1 ;
      }
      L_fielddef_data.m_set = L_fielddef->find_value((char*)"set-function") ;
      if (L_fielddef_data.m_set == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "fielddef set-function value is mandatory");
	L_ret = -1 ;
      }
      L_fielddef_data.m_get = L_fielddef->find_value((char*)"get-function") ;
      if (L_fielddef_data.m_get == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "fielddef get-function value is mandatory");
	L_ret = -1 ;
      }
      L_fielddef_data.m_check_set = L_fielddef->find_value((char*)"check-set") ;
      L_fielddef_data.m_check_get = L_fielddef->find_value((char*)"check-get") ;

      L_fielddef_data.m_default = L_fielddef->find_value((char*)"default") ;

      L_fielddef_data.m_from_string = L_fielddef->find_value((char*)"from-string") ;
      L_fielddef_data.m_to_string = L_fielddef->find_value((char*)"to-string") ;


      L_fielddef_data.m_config_field_name = L_fielddef->find_value((char*)"config-field") ;


      if (L_ret == -1) { 
	break ; 
      } else {
	P_fielddef_list->push_back(L_fielddef_data);
      }

    }
  }

  GEN_DEBUG(1, "C_ProtocolExternal::xml_fields() end");
  return (L_ret);
}


int C_ProtocolExternal::xml_configuration_parameters(C_XmlData *P_data, 
						     T_ParamDefList *P_paramdef_list) { 



  int                       L_ret = 0 ;

  T_pXmlData_List           L_paramdef_xml_list = NULL ;
  C_XmlData                *L_paramdef = NULL          ;
  T_XmlData_List::iterator  L_paramdef_it              ;

  T_ParamDef                L_paramdef_data            ;
  bool                      L_found_default = false    ;

  GEN_DEBUG(1, "C_ProtocolExternal::xml_configuration_parameters() start");

  L_paramdef_xml_list = P_data->get_sub_data() ;

  for(L_paramdef_it  = L_paramdef_xml_list->begin() ;
      L_paramdef_it != L_paramdef_xml_list->end() ;
      L_paramdef_it++) {

    L_paramdef = *L_paramdef_it ;

    if (strcmp(L_paramdef->get_name(), (char*)"paramdef") == 0) {

      L_found_default = false ;
      L_paramdef_data.m_name = L_paramdef->find_value((char*)"name") ;
      if (L_paramdef_data.m_name == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "paramdef name value is mandatory");
	L_ret = -1 ;
	break ;
      }

      L_paramdef_data.m_default = L_paramdef->find_value((char*)"default") ;
      if (L_paramdef_data.m_default != NULL) {
	L_found_default = true ;
      } 

      L_paramdef_data.m_mandatory = L_paramdef->find_value((char*)"mandatory") ;

      if ((L_paramdef_data.m_mandatory == NULL) ||
	  (strcmp(L_paramdef_data.m_mandatory,"false") == 0 )) {
	
	if (L_found_default == false ) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "paramdef mandatory or default value is mandatory");
	  L_ret = -1 ;
	}
      }

      if (L_ret == -1) { 
	break ; 
      } else {
	P_paramdef_list->push_back(L_paramdef_data);
      }

    } // strcmp paradef
  }

  GEN_DEBUG(1, "C_ProtocolExternal::xml_configuration_parameters() end");
  return (L_ret);
}


C_ProtocolExternal::T_FieldBodyList* 
C_ProtocolExternal::analyze_body_value (C_XmlData          *P_data, 
					T_FieldBodyList    *P_fielddef_body_list,
					bool                P_body_only,
					int                *P_ret) { 

  C_XmlData::T_pXmlField_List  L_fields_list      ;
  C_XmlData::T_XmlField_List::iterator L_fieldIt  ;

  char                     *L_value               ;
  T_FieldBody               L_fielddef_body       ;

  T_pXmlData_List           L_subListSetField     ;
  C_XmlData                *L_data                ;  
  T_XmlData_List::iterator L_listFieldIt          ;

  GEN_DEBUG(1, "C_ProtocolExternal::analyze_body_value() start");

  (*P_ret) = 0  ;
  
  // ctrl fields 
  L_value = P_data->find_value((char *)"name");
  GEN_DEBUG(1, "C_ProtocolExternal::analyze_body_value() "
	    << "[" << m_body_name << "] [" << L_value << "]");

  if (L_value == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "name definition value is mandatory for this section");
    *P_ret = -1 ;
  } else {
    L_fielddef_body.m_name = L_value ;
    L_fielddef_body.m_instance = NULL ;
    // Add not-present option 
    L_fielddef_body.m_list_not_present = NULL ;
  }
  
 
  // get fields
  L_fields_list = P_data->get_fields();
  if (!L_fields_list->empty()) {
    
    NEW_VAR(L_fielddef_body.m_list_value, T_FieldValueList());
    for(L_fieldIt  = L_fields_list->begin() ;
	L_fieldIt != L_fields_list->end() ;
	L_fieldIt++) {
      
      T_FieldValue L_field_val ;
      if(strcmp((*L_fieldIt)->get_name() , (char *)"instance") == 0) {
	L_fielddef_body.m_instance = (*L_fieldIt)->get_value() ;

	GEN_DEBUG(1, "C_ProtocolExternal::analyze_body_value() "
		  << "instance [" << (*L_fieldIt)->get_value()  << "]");
  
      } else if (strcmp((*L_fieldIt)->get_name() , (char *)"name") != 0) {
	L_field_val.m_name = (*L_fieldIt)->get_name();
	L_field_val.m_value = (*L_fieldIt)->get_value();
	GEN_DEBUG(1, "C_ProtocolExternal::analyze_body_value() "
		  << "[" << L_field_val.m_name << "] = [" 
		  << L_field_val.m_value << "]");
	
	
	L_fielddef_body.m_list_value->push_back(L_field_val);
      }
      // name case
    }
  } else {
    L_fielddef_body.m_list_value = NULL;
  }

  if (*P_ret != -1){
    // get setfield
    L_subListSetField = P_data->get_sub_data() ;
    if (L_subListSetField != NULL) {
      for (L_listFieldIt = L_subListSetField->begin();
	   L_listFieldIt != L_subListSetField->end();
	   L_listFieldIt++) {
	
	L_data = *L_listFieldIt ;
	if (strcmp(L_data->get_name(), "setfield") == 0) {
	  L_fielddef_body.m_list_value 
	    = analyze_setfield(L_data, L_fielddef_body.m_list_value, P_ret);
	  if (*P_ret == -1) break ;
	} // if (strcmp(L_data->get_name(), (char*)"setfield") == 0)

	// Add not-present option
	// attention only component !!!!
	if (P_body_only == true) {
	  if (strcmp(L_data->get_name(), "not-present") == 0) {
	    L_fielddef_body.m_list_not_present 
	      = analyze_not_present(L_data, L_fielddef_body.m_list_not_present, P_ret);
	    if (*P_ret == -1) break ;
	  } // if (strcmp(L_data->get_name(), (char*)"not-present") == 0)
	}
	

      } // for (L_listFieldIt ... )
      
    }
  }
  
  if (*P_ret != -1){
    if (P_fielddef_body_list == NULL) {
      NEW_VAR(P_fielddef_body_list, T_FieldBodyList());
    }
    P_fielddef_body_list->push_back(L_fielddef_body);
  } 
  GEN_DEBUG(1, "C_ProtocolExternal::xml_fields_body() end");

  return (P_fielddef_body_list);
}

C_ProtocolExternal::T_FieldHeaderList* 
C_ProtocolExternal::analyze_header_value (C_XmlData          *P_data, 
					  T_FieldHeaderList  *P_fielddef_header_list,
					  bool                P_header_only,
					  int                *P_ret) {

  C_XmlData::T_pXmlField_List  L_fields_list     ;
  C_XmlData::T_XmlField_List::iterator L_fieldIt ;
 
  char                     *L_value              ;
  T_FieldHeader             L_fielddef_header    ;

  T_pXmlData_List           L_subListSetField    ;
  C_XmlData                *L_data               ;  
  T_XmlData_List::iterator  L_listFieldIt        ;

  *P_ret = 0 ;
  
  GEN_DEBUG(1, "C_ProtocolExternal::analyze_header_value() start");

  // ctrl fields 
  L_value = P_data->find_value((char *)"name");

  GEN_DEBUG(1, "C_ProtocolExternal::analyze_header_value() "
	    << "[" << m_message_name << "] = [" << L_value << "]");

  if (L_value == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "name definition value is mandatory for this section");
    *P_ret = -1 ;
  } else {
    L_fielddef_header.m_name = L_value ;
    L_fielddef_header.m_list_header_not_present = NULL;
  }

  // get fields
  L_fields_list = P_data->get_fields();
  if (!L_fields_list->empty()) {
	
    NEW_VAR(L_fielddef_header.m_list_value, T_FieldValueList());
    for(L_fieldIt  = L_fields_list->begin() ;
	L_fieldIt != L_fields_list->end() ;
	L_fieldIt++) {
      T_FieldValue L_field_val ;
      if(strcmp((*L_fieldIt)->get_name() , (char *)"name") != 0) {
	L_field_val.m_name = (*L_fieldIt)->get_name();
	L_field_val.m_value = (*L_fieldIt)->get_value();

	GEN_DEBUG(1, "C_ProtocolExternal::analyze_header_value() "
		  << "[" << L_field_val.m_name << "] = [" 
		  << L_field_val.m_value << "]");


	L_fielddef_header.m_list_value->push_back(L_field_val);
      }
    }
  } else {
    L_fielddef_header.m_list_value = NULL;
  }



  // get setfield

  if (*P_ret != -1) {

    L_subListSetField = P_data->get_sub_data() ;
    if (L_subListSetField != NULL) {
      L_fielddef_header.m_list_body = NULL ;
      for (L_listFieldIt = L_subListSetField->begin();
	   L_listFieldIt != L_subListSetField->end();
	   L_listFieldIt++) {
	
	L_data = *L_listFieldIt ;
	if (strcmp(L_data->get_name(), "setfield") == 0) {
	  L_fielddef_header.m_list_value 
	    = analyze_setfield(L_data, L_fielddef_header.m_list_value, P_ret);
	  if (*P_ret == -1) break ;
	} // if (strcmp(L_data->get_name(), (char*)"setfield") == 0)

	if (P_header_only == true) {
	  if (strcmp(L_data->get_name(), "not-present") == 0) {
	    L_fielddef_header.m_list_header_not_present 
	      = analyze_not_present(L_data, L_fielddef_header.m_list_header_not_present, P_ret);
	    if (*P_ret == -1) break ;
	  } // if (strcmp(L_data->get_name(), (char*)"not-present") == 0)
	}
	

	if (strcmp(L_data->get_name(), m_body_name) == 0) {

	  L_fielddef_header.m_list_body
	    = analyze_body_value(L_data, L_fielddef_header.m_list_body, false, P_ret); 
	  if (*P_ret == -1) break ;
	}
      } // for (L_listFieldIt ... )
    }
  }
  
  if (*P_ret != -1) {
    if (P_fielddef_header_list == NULL) {
      NEW_VAR(P_fielddef_header_list, T_FieldHeaderList());
    }
    P_fielddef_header_list->push_back(L_fielddef_header);
  }
  

  return (P_fielddef_header_list);
}


C_ProtocolExternal::T_FieldValueList* 
C_ProtocolExternal::analyze_setfield (C_XmlData          *P_data,
				      T_FieldValueList   *P_field_value_list,
				      int                *P_ret) {

  C_XmlData                *L_data                ;  
  char                     *L_fieldName, *L_fieldValue ;
  
  GEN_DEBUG(1, "C_ProtocolExternal::analyze_setfield() start");
  
  (*P_ret) = 0  ;
  
  L_data = P_data ;

  L_fieldName = L_data->find_value((char*)"name") ;
  if (L_fieldName == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "setfield name is mandatory");
    (*P_ret) = -1 ;
  }
	
  L_fieldValue = L_data->find_value((char*)"value") ;
  if (L_fieldValue == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "setfield value is mandatory for ["
	      << L_fieldName << "]");
    (*P_ret) = -1 ;
  }
	
  if ((*P_ret) != -1) {
    T_FieldValue L_field_val ;
    if (P_field_value_list == NULL) {
      NEW_VAR(P_field_value_list, T_FieldValueList());
    }
    L_field_val.m_name = L_fieldName;
    L_field_val.m_value = L_fieldValue;
    GEN_DEBUG(1, "C_ProtocolExternal::analyze_setfield() "
	      << "[" << L_field_val.m_name << "] = [" 
	      << L_field_val.m_value << "]");

    P_field_value_list->push_back(L_field_val);
  } // if (*P_ret != -1)
	
  GEN_DEBUG(1, "C_ProtocolExternal::analyze_setfield() end");
  return (P_field_value_list);
}

C_ProtocolExternal::T_CharList* 
C_ProtocolExternal::analyze_not_present (C_XmlData          *P_data,
					 T_CharList         *P_field_not_present_list,
					 int                *P_ret) {

  C_XmlData                *L_data            ;  
  char                     *L_fieldName       ;
  
  GEN_DEBUG(1, "C_ProtocolExternal::analyze_not_present() start");
  
  (*P_ret) = 0  ;
  
  L_data = P_data ;

  L_fieldName = L_data->find_value((char*)"name") ;
  if (L_fieldName == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "not-present name is mandatory");
    (*P_ret) = -1 ;
  }

  if ((*P_ret) != -1) {
    if (P_field_not_present_list == NULL) {
      NEW_VAR(P_field_not_present_list, T_CharList());
    }
    
    GEN_DEBUG(1, "C_ProtocolExternal::analyze_not_present() "
	      << "[" << L_fieldName << "]"); 
    
    P_field_not_present_list->push_back(L_fieldName);
  } // if (*P_ret != -1)
  
  GEN_DEBUG(1, "C_ProtocolExternal::analyze_not_present() end");
  return (P_field_not_present_list);
}


int C_ProtocolExternal::analyze_dictionnary (T_FieldDefList *P_header_fields, // fields
					     T_FieldDefList *P_body_fields,   // sub fields
					     char *P_body_value_name,
					     char *P_session_id_name,
					     char *P_outof_session_id_name,
					     T_FieldBodyList *P_body_values,  // fields
					     T_FieldHeaderList *P_header_values, // message
					     char *P_header_type,
					     T_pConfigValueList P_config_value_list,
					     char *P_body_type) {

  int                          L_ret              = 0    ;
  T_FieldDefList::iterator     L_fielddef_it             ;
  T_pFieldDesc                 L_field_desc       = NULL ;
  char                        *L_data_type_name   = NULL ;
  char                        *L_body_type        = NULL ;
  char                        *L_header_type      = NULL ;

  T_FieldBodyList::iterator    L_body_fielddef_it        ;
  T_pFieldBodyDesc             L_body_desc = NULL        ;

  T_FieldNameMap::iterator     L_it                      ;

  T_FieldHeaderList::iterator  L_header_fielddef_it      ;
  int                          L_result           = -1   ;
  int                          L_i, L_j                  ;
  T_FieldBodyNameMap::iterator L_it_body                 ;
  T_MessageNameMap::iterator   L_it_message              ;
  int                          L_id ;

  GEN_DEBUG(1, "C_ProtocolExternal::analyze_dictionnary() start");


  NEW_VAR(m_field_body_name_map, T_FieldNameMap());
  NEW_VAR(m_field_name_map, T_FieldNameMap());
  NEW_VAR(m_body_value_name_map, T_FieldBodyNameMap());


  m_field_name_map->clear();
  m_field_body_name_map->clear();
  m_body_value_name_map->clear();


  if (!P_body_fields->empty()) {
    m_nb_body_fields = P_body_fields->size()  ;
    if (m_nb_body_fields > 0) {
      m_nb_names += m_nb_body_fields ;
      for (L_fielddef_it = P_body_fields->begin();
	   L_fielddef_it != P_body_fields->end();
	   L_fielddef_it++) {
	L_field_desc = check_field ((*L_fielddef_it), P_body_value_name, 
				    &L_data_type_name, 
				    P_body_type, &L_body_type, NULL, false);
	if (L_field_desc == NULL) { L_ret = -1 ; break ; }
      }
    } 
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, "ERROR TO DEFINE");
    L_ret = -1 ;
  }

  if (L_ret != -1) {
    if (L_body_type == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find field [" << P_body_type << "]");
      L_ret = -1 ;
    }
  }

  if (L_ret != -1) {
    if (!P_header_fields->empty()) {
      m_nb_header_fields = P_header_fields->size();
      if (m_nb_header_fields != 0) {
	m_nb_names += m_nb_header_fields ;
	for (L_fielddef_it = P_header_fields->begin();
	     L_fielddef_it != P_header_fields->end();
	     L_fielddef_it++) {
	  L_field_desc = check_field ((*L_fielddef_it), NULL, NULL, 
				      P_header_type, &L_header_type, 
				      P_config_value_list,
				      true);
	  if (L_field_desc == NULL) { L_ret = -1 ; break ; }
	}
      } 
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "ERROR TO DEFINE");
      L_ret = -1 ;
    }
  }

  if (L_ret != -1) {
    if (L_data_type_name == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find field [" << P_body_value_name << "]");
      L_ret = -1 ;
    }
  }

  if (L_ret != -1) {
    if (!P_body_values->empty()) {
      m_nb_body_values = P_body_values->size();
      m_nb_names += m_nb_body_values ;
      for (L_body_fielddef_it = P_body_values->begin();
	   L_body_fielddef_it != P_body_values->end();
	   L_body_fielddef_it++) {
	L_body_desc = check_body ((*L_body_fielddef_it));
	if (L_body_desc == NULL) { L_ret = -1 ; break ; }
      }
    }
  }

  if (L_ret != -1) {
    if (L_header_type == NULL) {
      // check the field header 
      // ctrl the body contains in the map of body
      L_it_body = m_body_value_name_map->find(T_FieldBodyNameMap::key_type(P_header_type));
      if (L_it_body == m_body_value_name_map->end()) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find field [" << P_header_type << "]");
	L_ret = -1 ;
      }
    }
  }


  // remove the data body value from map
  if (L_ret != -1) {
      set_body_field_id() ;
  }

  if (L_ret != -1) {
    // check not-present field for each body
    // phase init 
    ALLOC_TABLE(m_body_not_present_table,
		bool**,
		sizeof(bool*),
		m_nb_body_values);
    
    for (L_i = 0; L_i <m_nb_body_values ; L_i++) {
      ALLOC_TABLE (m_body_not_present_table[L_i], bool*, sizeof(bool), m_nb_body_fields);
      for (L_j = 0; L_j < m_nb_body_fields ; L_j++) {
	m_body_not_present_table[L_i][L_j] = true ;
      }
    }

    // phase anlyze for not-present     
    for (L_body_fielddef_it = P_body_values->begin();
	 L_body_fielddef_it != P_body_values->end();
	 L_body_fielddef_it++) {
      
      L_ret = check_not_present ((*L_body_fielddef_it));
      if (L_ret == -1 ) { break ; }
    }

    // delete m_list_not_present TO DO

  }

  
  if (L_ret != -1) { 
    T_FieldNameMap::iterator L_it_f_b ;
    T_pFieldDesc             L_desc_f_b ;
    int                      L_i_f_b  ;

    if (!m_field_body_name_map->empty()) {
      m_nb_from_string_field_body = m_field_body_name_map->size();
      ALLOC_TABLE(m_from_string_field_body_table,
		  C_MsgBuildContext::T_ContextStringFunction*,
		  sizeof(C_MsgBuildContext::T_ContextStringFunction),
		  m_nb_from_string_field_body);
      ALLOC_TABLE(m_to_string_field_body_table,
		  C_MsgBuildContext::T_ContextStringFunction*,
		  sizeof(C_MsgBuildContext::T_ContextStringFunction),
		  m_nb_from_string_field_body);
      for (L_i_f_b = 0 ; L_i_f_b < m_nb_from_string_field_body; L_i_f_b++) {
	m_from_string_field_body_table[L_i_f_b] = (C_MsgBuildContext::T_ContextStringFunction)NULL;
	m_to_string_field_body_table[L_i_f_b] = (C_MsgBuildContext::T_ContextStringFunction)NULL;
      }
      for (L_it_f_b = m_field_body_name_map->begin() ;
	   L_it_f_b != m_field_body_name_map->end() ;
	   L_it_f_b ++) {
	L_desc_f_b = L_it_f_b->second ;
	m_from_string_field_body_table[L_desc_f_b->m_id-m_start_body_index] = L_desc_f_b->m_from_string ;
	m_to_string_field_body_table[L_desc_f_b->m_id-m_start_body_index] = L_desc_f_b->m_to_string ;
      }
    } 
  }

  if (L_ret != -1) {
    bool L_convert ;
    if (m_nb_body_fields != 0) {
      ALLOC_TABLE(m_body_defaults, T_pValueData, sizeof(T_ValueData), m_nb_body_fields);
      for (L_fielddef_it = P_body_fields->begin();
	   L_fielddef_it != P_body_fields->end();
	   L_fielddef_it++) {

	L_convert = false ;
	L_result = -1 ;
	L_it = m_field_body_name_map->find(T_FieldNameMap::key_type(L_fielddef_it->m_name)) ;
	
	if (L_fielddef_it->m_default != NULL) {
	  if (m_from_string_field_body_table[((L_it->second)->m_id)-m_start_body_index] 
	      != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
	    L_convert = 
	      ((m_factory_context)->*(m_from_string_field_body_table[((L_it->second)->m_id)-m_start_body_index]))
	      (&(L_fielddef_it->m_default), &m_body_defaults[((L_it->second)->m_id)-m_start_body_index]) ;
	    if (L_convert == true) { L_result = 0 ; }
	  }
	}
	if (L_convert == false) {
	  m_body_defaults[((L_it->second)->m_id)-m_start_body_index] 
	    = valueFromString ((L_fielddef_it->m_default == NULL) 
			       ? (char*)defaultStringValue((L_it->second)->m_type) 
			       : L_fielddef_it->m_default,
			       (L_it->second)->m_type,
			       L_result);
	}
	if (L_result == -1) { L_ret = -1 ; break ; }
	m_body_defaults[((L_it->second)->m_id)-m_start_body_index].m_id
	  = (L_it->second)->m_id ;
      }
    } 
  }
  

  
  if (L_ret != -1) {
    for (L_body_fielddef_it = P_body_values->begin();
	   L_body_fielddef_it != P_body_values->end();
	   L_body_fielddef_it++) {
	L_body_desc = check_body_value ((*L_body_fielddef_it));
	if (L_body_desc == NULL) { L_ret = -1 ; break ; }
    }
  }
  
  if (L_ret != -1) {
    // retrieve id and type of session id
    m_type_id = find_field_id(P_header_type) ;
    if (m_type_id == -1) {
      char *L_name = (P_header_type == NULL) ?
	(char*)"" : P_header_type ; 
      GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << L_name 
		<< "] not found for message type");
      L_ret = -1 ;
    } 
  }

  if (L_ret != -1) {
    // retrieve m_body_type_id
    m_body_type_id = -1 ;
    L_it = m_field_body_name_map->
      find(T_FieldNameMap::key_type(P_body_type));
    if (L_it != m_field_body_name_map->end()) {
      m_body_type_id = (L_it->second)->m_id ;
    } 

    if (m_body_type_id == -1) {
      char *L_name = (P_body_type == NULL) ?
	(char*)"" : P_body_type ;
      GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << L_name
		<< "] not found for body type");
      L_ret = -1 ;
    }
    GEN_DEBUG(1, "C_ProtocolExternal::analyze_dictionnary() "
	      << "m_body_type_id [" << m_body_type_id << "]"); 
  }

  if (L_ret != -1) {
    // retrieve id of session 
    m_session_id = find_field_id(P_session_id_name) ;
    if (m_session_id == -1) {
      char *L_name = (P_session_id_name == NULL) ?
	(char*)"" : P_session_id_name ; 
      GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << L_name 
		<< "] not found for message session");
      L_ret = -1 ;
    } 
  }
  
  if (L_ret != -1) {
    // retrieve id of outof-session 
    m_outof_session_id = find_field_id(P_outof_session_id_name) ;
    if (m_outof_session_id == -1) {
      char *L_name = (P_outof_session_id_name == NULL) ?
	(char*)"" : P_outof_session_id_name ; 
      GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << L_name 
		<< "] not found for message out-of session");
      L_ret = -1 ;
    } 
  }
  

  if (L_ret != -1) {
    // build a table for all names  
    if (m_nb_names > 0) {

      ALLOC_TABLE(m_names_table, char**, sizeof(char*),m_nb_names);
      for (L_i = 0; L_i < m_nb_names ; L_i++) {
	m_names_table[L_i] = NULL ;
      } 
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
		"header field, body and header body not found"); 
      L_ret = -1 ;
    }
  }

  if (L_ret != -1) {
    if (!P_header_values->empty()) {
      m_nb_message_names =  P_header_values->size();
      
      // build a table for all message names  
      if (m_nb_message_names > 0) {
	ALLOC_TABLE(m_message_names_table, char**, sizeof(char*),m_nb_message_names);
	for (L_i = 0; L_i < m_nb_message_names ; L_i++) {
	  m_message_names_table[L_i] = NULL ;
	} 
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "messages not found in dictionnary"); 
	L_ret = -1 ;
      }
    } else {
      L_ret = -1 ;
    }
  }
  

  if (L_ret != -1) {
    // check not-present field for each header
    // phase init 
    ALLOC_TABLE(m_header_not_present_table,
		bool**,
		sizeof(bool*),
		m_nb_message_names);
    for (L_i = 0; L_i <m_nb_message_names ; L_i++) {
      ALLOC_TABLE (m_header_not_present_table[L_i], bool*, sizeof(bool), m_nb_header_fields);
      for (L_j = 0; L_j < m_nb_header_fields ; L_j++) {
	m_header_not_present_table[L_i][L_j] = true ;
      }
    }

  }


  if (L_ret != -1) { 
    T_FieldNameMap::iterator L_it_h ;
    T_pFieldDesc             L_desc_h ;
    int                      L_i_h  ;

    if (!m_field_name_map->empty()) {
      m_nb_from_string = m_field_name_map->size();
      ALLOC_TABLE(m_from_string_table,
		  C_MsgBuildContext::T_ContextStringFunction*,
		  sizeof(C_MsgBuildContext::T_ContextStringFunction),
		  m_nb_from_string);
      ALLOC_TABLE(m_to_string_table,
		  C_MsgBuildContext::T_ContextStringFunction*,
		  sizeof(C_MsgBuildContext::T_ContextStringFunction),
		  m_nb_from_string);
      for (L_i_h = 0 ; L_i_h < m_nb_from_string; L_i_h++) {
	m_from_string_table[L_i_h] = (C_MsgBuildContext::T_ContextStringFunction)NULL;
	m_to_string_table[L_i_h] = (C_MsgBuildContext::T_ContextStringFunction)NULL;
      }
      for (L_it_h = m_field_name_map->begin() ;
	   L_it_h != m_field_name_map->end() ;
	   L_it_h ++) {
	L_desc_h = L_it_h->second ;
	m_from_string_table[L_desc_h->m_id] = L_desc_h->m_from_string ;
	m_to_string_table[L_desc_h->m_id] = L_desc_h->m_to_string ;
      }
      
    } 
  }

  // default value management
  if (L_ret != -1) {
    bool L_convert ;
    if (m_nb_header_fields != 0) {
      ALLOC_TABLE(m_header_defaults, T_pValueData, sizeof(T_ValueData), m_nb_header_fields);
      for (L_fielddef_it = P_header_fields->begin();
	   L_fielddef_it != P_header_fields->end();
	   L_fielddef_it++) {

	L_convert = false ;
	L_result = -1 ;
	L_it = m_field_name_map->find(T_FieldNameMap::key_type(L_fielddef_it->m_name)) ;

	if (L_fielddef_it->m_default != NULL) {
	  if (m_from_string_table[(L_it->second)->m_id] 
	      != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
	    L_convert = ((m_factory_context)->*(m_from_string_table[(L_it->second)->m_id]))
	      (&(L_fielddef_it->m_default), &m_header_defaults[(L_it->second)->m_id]) ;
	    if (L_convert == true) { L_result = 0 ; }
	  }
	}
	if (L_convert == false) {
	  m_header_defaults[(L_it->second)->m_id] 
	    = valueFromString ((L_fielddef_it->m_default == NULL) 
			       ? (char*)defaultStringValue((L_it->second)->m_type) 
			       : L_fielddef_it->m_default,
			       (L_it->second)->m_type,
			       L_result);
	}
	if (L_result == -1) { L_ret = -1 ; break ; }
	m_header_defaults[(L_it->second)->m_id].m_id = (L_it->second)->m_id ;
      }
    } 
  }

  if (L_ret != -1) {
    C_MessageExternal::set_protocol_data(m_nb_header_fields,
					 m_nb_body_values,
					 m_nb_body_fields,
					 m_session_id,
					 m_outof_session_id,
					 m_type_id,
					 m_names_table,
					 m_message_names_table,
					 m_body_not_present_table,
					 m_header_not_present_table);
  }


  if (L_ret != -1) {
    NEW_VAR(m_message_map, T_MessageNameMap());
    m_message_map->clear();
    C_MessageExternal *L_msg = NULL ;
    for (L_header_fielddef_it = P_header_values->begin();
	 L_header_fielddef_it != P_header_values->end();
	 L_header_fielddef_it++) {
      L_msg = build_message (NULL, *L_header_fielddef_it, NULL,NULL);
      if (L_msg == NULL) { L_ret = -1 ; break ; }

      m_message_map->insert(T_MessageNameMap::value_type(L_header_fielddef_it->m_name,
							 L_msg));
      
    }
  }

  if (L_ret != -1) {
    // build a table for header field using m_field_name_map 

    ALLOC_TABLE(m_header_field_desc_table,
		T_pFieldDesc*,
		sizeof(T_pFieldDesc),
		m_nb_header_fields);
    
    for (L_it = m_field_name_map->begin();
	 L_it != m_field_name_map->end();
	 L_it++) {
      
      ALLOC_TABLE(m_names_table[(L_it->second)->m_id],
		  char*, sizeof(char),
		  strlen((L_it->second)->m_name)+1);
      
      strcpy(m_names_table[(L_it->second)->m_id],(L_it->second)->m_name) ;

      m_header_field_desc_table[(L_it->second)->m_id] = L_it->second  ;

    }
    
  }

  if (L_ret != -1) {
    NEW_VAR(m_body_decode_map, T_BodyDecodeMap());
    m_body_decode_map->clear();

    // build a table for body value using m_body_value_name_map
    ALLOC_TABLE(m_body_value_table,
		T_pFieldBodyDesc*,
		sizeof(T_pFieldBodyDesc),
		m_nb_body_values);

    // copy names of body in m_names_table using  m_body_value_name_map 
    for (L_it_body = m_body_value_name_map->begin();
	 L_it_body != m_body_value_name_map->end();
	 L_it_body++) {
      
      ALLOC_TABLE(m_names_table[(L_it_body->second)->m_id],
		  char*, sizeof(char),
		  strlen((L_it_body->second)->m_name)+1);

      strcpy(m_names_table[(L_it_body->second)->m_id],(L_it_body->second)->m_name) ;
      
      m_body_value_table[(L_it_body->second)->m_id - m_end_header_index -1] 
	= (L_it_body->second)  ;

      // build a m_body_decode_map  using m_body_value_name_map       
      T_ValueData  L_value_body;

      L_value_body.m_type = E_TYPE_NUMBER ;

      copyValue(L_value_body,
		((L_it_body->second)->m_header[m_body_type_id-m_start_body_index]),
		false);

      m_body_decode_map->insert(T_BodyDecodeMap::value_type(L_value_body,
							    (L_it_body->second)->m_id));
      // do not reset the value (if string => the string allocated is used by the map)
    }
  }

  if (L_ret != -1) {
    // build a table for body header field using m_field_body_name_map

    ALLOC_TABLE(m_body_field_desc_table,
		T_pFieldDesc*,
		sizeof(T_pFieldDesc),
		m_nb_body_fields);
    for (L_it = m_field_body_name_map->begin();
	 L_it != m_field_body_name_map->end();
	 L_it++) {
      ALLOC_TABLE(m_names_table[(L_it->second)->m_id],
		  char*, sizeof(char),
		  strlen((L_it->second)->m_name)+1);
      strcpy(m_names_table[(L_it->second)->m_id],(L_it->second)->m_name) ;

      GEN_DEBUG(1, "C_ProtocolExternal::analyze_dictionnary() "
		<< "(L_it->second)->m_id [" << (L_it->second)->m_id << "]"); 

      m_body_field_desc_table[(L_it->second)->m_id - m_start_body_index] = L_it->second  ;
      
    }
  } 
  
  
//    if (L_ret != -1) {
//      for (L_i = 0 ; L_i <  m_nb_names ; L_i ++) {
//        std::cerr << "m_names_table[" << L_i << "] = [" 
//          	<< m_names_table[L_i] << "]"<< std::endl;
//      }
//    }
  
  if (L_ret != -1) {
    NEW_VAR(m_message_decode_map, T_MessageDecodeMap());
    m_message_decode_map->clear();
    
    C_MessageExternal *L_msg ;

    L_id = 0 ;
    for (L_it_message = m_message_map->begin();
	 L_it_message != m_message_map->end();
	 L_it_message++) {
      L_msg = L_it_message->second ;
      L_msg->m_id = L_id ;


      ALLOC_TABLE(m_message_names_table[L_id],
		  char*, sizeof(char),
		  ((L_it_message->first).length())+1);
		  
      strcpy(m_message_names_table[L_id],(L_it_message->first).c_str()) ;

      L_id ++ ;      

      GEN_DEBUG(1, "C_ProtocolExternal::analyze_dictionnary() "
		<< "L_msg from m_message_map [" << (*L_msg) << "]"); 

      T_ValueData   L_value ;
      L_value.m_type = E_TYPE_NUMBER ;
      
      copyValue(L_value,
		*(L_msg->get_type()),
		false);
      
      m_message_decode_map->insert(T_MessageDecodeMap::value_type(L_value,
								  L_msg));
    }
    
  }
  
//    if (L_ret != -1) {
//      for (L_i = 0 ; L_i <  m_nb_message_names ; L_i ++) {
//        std::cerr << "m_message_names_table[" << L_i << "] = [" 
//        		<< m_message_names_table[L_i] << "]"<< std::endl;
//      }
//    }
  
  
  if (L_ret != -1) {
    // phase anlyze for not-present     
    for (L_header_fielddef_it = P_header_values->begin();
	 L_header_fielddef_it != P_header_values->end();
	 L_header_fielddef_it++) {

      L_ret = check_header_not_present ((*L_header_fielddef_it));
      if (L_ret == -1 ) { break ; }
    }

  }

  
  
  GEN_DEBUG(1, "C_ProtocolExternal::analyze_dictionnary() end");
  return (L_ret);

}

C_ProtocolExternal::T_pFieldDesc 
C_ProtocolExternal::check_field(T_FieldDef &P_field_def,
				char *P_field_name,
				char **P_data_type_name,
				char *P_field_type,
				char **P_field_type_found,
				T_pConfigValueList P_config_value_list,
				bool P_header) {

  T_pFieldDesc                  L_ret = NULL ;
  T_FieldNameMap::iterator      L_it ;
  size_t                        L_size = 0 ;


  GEN_DEBUG(1, "C_ProtocolExternal::check_field() start");


  
  L_it = m_field_body_name_map->find(T_FieldNameMap::key_type(P_field_def.m_name));
  if (L_it != m_field_body_name_map->end()) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << P_field_def.m_name << "] already defined");
    return (L_ret);
  }
  L_it = m_field_name_map->find(T_FieldNameMap::key_type(P_field_def.m_name));
  if (L_it != m_field_name_map->end()) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << P_field_def.m_name << "] already defined");
    return (L_ret);
  }
  
  ALLOC_VAR(L_ret, T_pFieldDesc, sizeof(T_FieldDesc));

  L_ret->m_config_field_name = NULL ;
  

  L_ret->m_type = typeFromString (P_field_def.m_type);
  if (L_ret->m_type == E_UNSUPPORTED_TYPE) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown type definition for [" << P_field_def.m_type << "]");
    FREE_VAR(L_ret);
    return (L_ret);
  }

  L_size = strlen(P_field_def.m_name) ;
  ALLOC_TABLE(L_ret->m_name, char*, sizeof(char), L_size+1);
  strcpy(L_ret->m_name, P_field_def.m_name);
  
  L_ret->m_get = m_factory->get_function(P_field_def.m_get);
  if ((L_ret->m_get) == (C_MsgBuildContext::T_ContextFunction)NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown function definition for [" << P_field_def.m_get << "]");
    FREE_VAR(L_ret);
    return (L_ret);
  }

  L_ret->m_set = m_factory->get_function(P_field_def.m_set);
  if ((L_ret->m_set) == (C_MsgBuildContext::T_ContextFunction)NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown function definition for [" << P_field_def.m_set << "]");
    FREE_VAR(L_ret);
    return (L_ret);
  }

  if (P_field_def.m_check_get != NULL) {
    L_ret->m_check_get = m_factory->get_function(P_field_def.m_check_get);
    if ((L_ret->m_check_get) == (C_MsgBuildContext::T_ContextFunction)NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown function definition for [" 
		<< P_field_def.m_check_get << "]");
      FREE_VAR(L_ret);
      return (L_ret);
    }
  } else {
    L_ret->m_check_get = (C_MsgBuildContext::T_ContextFunction)NULL;
  }

  if (P_field_def.m_check_set != NULL) {
    L_ret->m_check_set = m_factory->get_function(P_field_def.m_check_set);
    if ((L_ret->m_check_set) == (C_MsgBuildContext::T_ContextFunction)NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown function definition for [" 
		<< P_field_def.m_check_set << "]");
      FREE_VAR(L_ret);
      return (L_ret);
    }
  } else {
    L_ret->m_check_set = (C_MsgBuildContext::T_ContextFunction)NULL;
  }

  if (P_field_def.m_from_string != NULL) {
    L_ret->m_from_string = m_factory->get_string_function(P_field_def.m_from_string);
    if ((L_ret->m_from_string) == (C_MsgBuildContext::T_ContextStringFunction)NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown function definition for [" 
		<< P_field_def.m_from_string << "]");
      FREE_VAR(L_ret);
      return (L_ret);
    }
  } else {
    L_ret->m_from_string = (C_MsgBuildContext::T_ContextStringFunction)NULL;
  }


  if (P_field_def.m_to_string != NULL) {
    L_ret->m_to_string = m_factory->get_string_function(P_field_def.m_to_string);
    if ((L_ret->m_to_string) == (C_MsgBuildContext::T_ContextStringFunction)NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown function definition for [" 
		<< P_field_def.m_to_string << "]");
      FREE_VAR(L_ret);
      return (L_ret);
    }
  } else {
    L_ret->m_to_string = (C_MsgBuildContext::T_ContextStringFunction)NULL;
  }

  if (P_field_def.m_config_field_name != NULL) {
    L_ret->m_config_field_name = P_field_def.m_config_field_name ;
  } 
  
  if (P_header == true) {
    L_ret->m_id = new_id();
    if (get_id() > m_end_header_index) { m_end_header_index = get_id(); }
    m_field_name_map->insert(T_FieldNameMap::value_type(P_field_def.m_name, L_ret));
  } else {
    m_field_body_name_map->insert(T_FieldNameMap::value_type(P_field_def.m_name, L_ret));
    if (strcmp(P_field_name, P_field_def.m_name) == 0) {
      *P_data_type_name = P_field_def.m_name ;
    }
  }

  if (strcmp(P_field_def.m_name, P_field_type) == 0) {
    (*P_field_type_found) = P_field_def.m_name ;
  } 
  
  GEN_DEBUG(1, "C_ProtocolExternal::check_field() end ");

  return (L_ret);
}

int C_ProtocolExternal::update_config_params(T_ParamDef& P_config_param_dico,
					     T_pConfigValueList P_config_value_list) {


  int                          L_ret            = 0     ;
  T_ConfigValueList::iterator  L_configValue_it         ;
  bool                         L_found          = false ;
  T_ConfigValue               L_configValue             ;

  GEN_DEBUG(1, "C_ProtocolExternal::update_config_params() start");

  if (!P_config_value_list->empty()) {
    for (L_configValue_it = P_config_value_list->begin();
	 L_configValue_it != P_config_value_list->end();
	 L_configValue_it++) {
      if (strcmp(L_configValue_it->m_name, P_config_param_dico.m_name) == 0) {
	L_found = true ;
	// the config file value is used
	break;
      }
    }
  }
  
  if (L_found == false) {
    // ctrl mandatory => error , must be in the config
    if ((P_config_param_dico.m_mandatory != NULL) && 
	(strcmp(P_config_param_dico.m_mandatory,"true") == 0 )) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
		"paramdef mandatory value is mandatory for ["
		<< P_config_param_dico.m_name << "]");
      L_ret = -1 ;
    } else {
      // not mandatory, but a default value is known
      // element not found in config list
      // Add this element in the P_config_value_list
      L_configValue.m_name = P_config_param_dico.m_name ;
      L_configValue.m_value = P_config_param_dico.m_default ;
      P_config_value_list->push_back(L_configValue);
    }
  } 
 
  GEN_DEBUG(1, "C_ProtocolExternal::update_config_params() end");
  return (L_ret);
}



C_ProtocolExternal::T_pFieldBodyDesc 
C_ProtocolExternal::check_body(T_FieldBody &P_field_body) {
  T_pFieldBodyDesc              L_desc = NULL ;
  T_FieldBodyNameMap::iterator  L_it       ;
  size_t                        L_size = 0 ;

  GEN_DEBUG(1, "C_ProtocolExternal::check_body() start ");

  L_it = m_body_value_name_map->find(T_FieldBodyNameMap::key_type(P_field_body.m_name));
  if (L_it != m_body_value_name_map->end()) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << P_field_body.m_name << "] already defined");
    return (L_desc);
  }

  ALLOC_VAR(L_desc, T_pFieldBodyDesc, sizeof(T_FieldBodyDesc));
  L_desc->m_id = new_id();
  L_desc->m_header = NULL ;
  
  L_size = strlen(P_field_body.m_name) ;
  ALLOC_TABLE(L_desc->m_name, char*, sizeof(char), L_size+1);
  strcpy(L_desc->m_name,P_field_body.m_name);
  
  m_body_value_name_map->insert(T_FieldBodyNameMap::value_type(P_field_body.m_name, L_desc));
  
  GEN_DEBUG(1, "C_ProtocolExternal::check_body() end");

  return (L_desc);
}

C_ProtocolExternal::T_pFieldBodyDesc 
C_ProtocolExternal::check_body_value(T_FieldBody &P_field_body) {
  
  T_pFieldBodyDesc L_desc = NULL ;
  T_FieldBodyNameMap::iterator L_it ;
  T_FieldValueList::iterator L_field_it ;
  
  T_FieldNameMap::iterator L_field_found ;
  int                      L_result = 0 ;
  int                      L_i ;
  int L_id ;

  T_ValueData              L_value ;
  bool                     L_convert ;

  GEN_DEBUG(1, "C_ProtocolExternal::check_body_value() start");  

  L_it = m_body_value_name_map->find(T_FieldBodyNameMap::key_type(P_field_body.m_name));
  if (L_it == m_body_value_name_map->end()) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << P_field_body.m_name << "] already defined");
    return (L_desc);
  }

  L_desc = L_it->second ;

  ALLOC_TABLE(L_desc->m_header, T_pValueData, 
	      sizeof(T_ValueData), m_nb_body_fields);
  for(L_i = 0 ; L_i < m_nb_body_fields; L_i++) {
    L_desc->m_header[L_i].m_type = E_TYPE_NUMBER ;
    copyValue(L_desc->m_header[L_i], m_body_defaults[L_i], false) ;
  }
  
  for (L_field_it = (P_field_body.m_list_value)->begin();
       L_field_it != (P_field_body.m_list_value)->end();
       L_field_it++) {
    L_field_found = m_field_body_name_map->find(T_FieldNameMap::key_type(L_field_it->m_name));
    if (L_field_found == m_field_body_name_map->end()) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << L_field_it->m_name << "] not found");
      return (NULL);
    } else {



      L_convert = false ;
      L_result = -1 ;
      if (m_from_string_field_body_table[((L_field_found->second)->m_id) - m_start_body_index] 
	  != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
	L_convert = 
	  ((m_factory_context)->*(m_from_string_field_body_table[((L_field_found->second)->m_id)- m_start_body_index]))
	  (&(L_field_it->m_value), &L_value) ;
	if (L_convert == true) { L_result = 0 ; }
      }

      if (L_convert == false) {
	L_value = valueFromString(L_field_it->m_value, (L_field_found->second)->m_type, L_result);
      }

      L_value.m_id = (L_field_found->second)->m_id;
      if (L_result == -1) { return(NULL); }
      L_id = ((L_field_found->second)->m_id) - m_start_body_index ;
      copyValue((L_desc->m_header)[L_id],
		L_value, true);
      resetMemory(L_value);
      (L_desc->m_header)[L_id].m_id 
	= (L_field_found->second)->m_id ;
    }
  }

  GEN_DEBUG(1, "C_ProtocolExternal::check_body_value() end");    
  return (L_desc);
}


int C_ProtocolExternal::check_not_present(T_FieldBody &P_field_body) {
  
  int                          L_ret  = 0    ;
  T_CharList::iterator         L_field_it    ;
  T_FieldNameMap::iterator     L_field_found ;
  T_FieldBodyNameMap::iterator L_it          ;
  int                          L_i, L_j      ;

  GEN_DEBUG(1, "C_ProtocolExternal::check_not_present() start");  
  

  // find the id of body
  L_it = m_body_value_name_map->find(T_FieldBodyNameMap::key_type(P_field_body.m_name));
  if (L_it == m_body_value_name_map->end()) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "body [" << P_field_body.m_name << "] not found");
    L_ret = -1 ;
    return (L_ret);
  }

  L_i = (L_it->second)->m_id - m_end_header_index - 1 ;


  // find the field not present for this body corresponding to L_id
  if ((P_field_body.m_list_not_present != NULL ) && (!(P_field_body.m_list_not_present)->empty())) {	  
    for (L_field_it = (P_field_body.m_list_not_present)->begin();
	 L_field_it != (P_field_body.m_list_not_present)->end();
	 L_field_it++) {

      L_field_found = m_field_body_name_map->find(T_FieldNameMap::key_type(*L_field_it));
      if (L_field_found == m_field_body_name_map->end()) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << *L_field_it << "] not found");
      	L_ret = -1 ;
      } else {
	L_j = ((L_field_found->second)->m_id) - m_start_body_index ;
	m_body_not_present_table[L_i][L_j] = false ;
      }
    }
  } 

  GEN_DEBUG(1, "C_ProtocolExternal::check_not_present() end");    
  return (L_ret);
}

int C_ProtocolExternal::check_header_not_present(T_FieldHeader &P_field_header) {
  
  int                          L_ret  = 0    ;
  T_CharList::iterator         L_field_it    ;
  T_FieldNameMap::iterator     L_field_found ;

  T_MessageNameMap::iterator   L_it          ;
  int                          L_j           ;
  int                          L_i           ;
 
  GEN_DEBUG(1, "C_ProtocolExternal::check_header_not_present() start");  
  

  // find the id of body
  L_it = m_message_map->find(T_MessageNameMap::key_type(P_field_header.m_name));
  if (L_it == m_message_map->end()) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "body [" << P_field_header.m_name << "] not found");
    L_ret = -1 ;
    return (L_ret);
  }

  L_i = (L_it->second)->m_id ;


  // find the field not present for this body corresponding to L_id
  if ((P_field_header.m_list_header_not_present != NULL ) && 
      (!(P_field_header.m_list_header_not_present)->empty())) {	  
    for (L_field_it = (P_field_header.m_list_header_not_present)->begin();
	 L_field_it != (P_field_header.m_list_header_not_present)->end();
	 L_field_it++) {

      L_field_found = m_field_name_map->find(T_FieldNameMap::key_type(*L_field_it));
      if (L_field_found == m_field_name_map->end()) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << *L_field_it << "] not found");
      	L_ret = -1 ;
      } else {
	L_j = ((L_field_found->second)->m_id) ;
	m_header_not_present_table[L_i][L_j] = false ;
      }
    }
  } 

  GEN_DEBUG(1, "C_ProtocolExternal::check_header_not_present() end");    
  return (L_ret);
}

int C_ProtocolExternal::new_id() {
  m_id_counter ++ ;
  return(m_id_counter);
}

int C_ProtocolExternal::get_id() {
  return(m_id_counter);
}


void C_ProtocolExternal::set_body_field_id() {
  T_FieldNameMap::iterator L_it ;
  if (!m_field_body_name_map->empty()) {
    for(L_it = m_field_body_name_map->begin();
	L_it != m_field_body_name_map->end();
	L_it++) {
      ((L_it->second)->m_id) = new_id();
      if (m_start_body_index == -1) {
	m_start_body_index = get_id();
      }
    }
  }
}

C_MessageExternal* C_ProtocolExternal::build_message (C_MessageExternal *P_msg,
						      T_FieldHeader &P_header,
						      T_pInstanceDataList P_list,
						      int                *P_nb_value) {

  C_MessageExternal           *L_msg = NULL      ;
  int                          L_i               ;
  int                          L_id              ;
  int                          L_field_id        ;

  T_pValueData                 L_header          ;
  T_pValueData                 L_header_init     ;
  int                         *L_body_instance   ;

  T_pValueData                 L_a_body ;
  T_ValueDataList              L_body   ;
  list_t<int>                  L_body_id;
  int                          L_instance;

  T_FieldValueList::iterator   L_value_it        ;

  T_FieldBodyList::iterator    L_body_it         ;
  T_FieldNameMap::iterator     L_field_it        ;

  T_FieldBodyNameMap::iterator L_it_body         ;



  T_ValueData                  L_value           ;
  int                          L_result          ;
  //  bool                         L_data          = false ;

  set_t<int>                   L_confict_set     ;
  bool                         L_convert ;
  //  set_t<int>::iterator         L_conf_it         ;
  //  int                          L_j, L_k          ;
  //  bool                         L_do_copy = false ;


  ALLOC_TABLE(L_body_instance, int*, sizeof(int), 
	      m_nb_body_values);
  for (L_i = 0 ; L_i < m_nb_body_values; L_i ++) {
    L_body_instance[L_i] = 0 ;
  }



  L_confict_set.clear();

  GEN_DEBUG(1, "C_ProtocolExternal::build_message() start");    

  GEN_DEBUG(1, "C_ProtocolExternal::build_message() "
	    << "P_header.m_name [" << P_header.m_name << "]");

  L_header_init = (P_msg == NULL) ? m_header_defaults : P_msg->m_header ;

  ALLOC_TABLE(L_header, T_pValueData, sizeof(T_ValueData), m_nb_header_fields);

  for (L_i = 0 ; L_i < m_nb_header_fields; L_i ++) {

    if ((P_msg != NULL) && (m_header_field_desc_table[L_i]->m_config_field_name != NULL)) {
      char * L_valueString = NULL ;
      L_convert = false ;
      L_result = -1 ;
      L_valueString = find_config_value(m_header_field_desc_table[L_i]->m_config_field_name);
      if (L_valueString != NULL ) {
	if (m_from_string_table[m_header_field_desc_table[L_i]->m_id] 
	    != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
	  L_convert = 
	    ((m_factory_context)->*(m_from_string_table[m_header_field_desc_table[L_i]->m_id]))
	    (&(L_valueString), &L_value) ;
	  if (L_convert == true) { L_result = 0 ; }
	}
	if (L_convert == false) {
	  L_value = valueFromString(L_valueString, m_header_field_desc_table[L_i]->m_type, L_result);
	}
	if (L_result == -1) {
	  return (NULL);
	}

	L_value.m_id = m_header_field_desc_table[L_i]->m_id ;
        L_header[L_i].m_type = m_header_field_desc_table[L_i]->m_type ;

	copyValue(L_header[L_i], L_value, true);
	resetMemory(L_value);
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find [" 
		  << m_header_field_desc_table[L_i]->m_config_field_name 
		  << "] in configuration parameters");
	return (NULL);
      }
    } else {
      copyValue(L_header[L_i], L_header_init[L_i], false);
    }
  }

  if ((P_header.m_list_value != NULL ) 
      && (!(P_header.m_list_value)->empty())) {  
    for(L_value_it = (P_header.m_list_value)->begin();
	L_value_it != (P_header.m_list_value)->end();
	L_value_it++) {
      
      GEN_DEBUG(1, "C_ProtocolExternal::build_message() "
		<< "L_value_it->m_name [" << L_value_it->m_name << "]");

      L_field_it = m_field_name_map->find(T_FieldNameMap::key_type(L_value_it->m_name));
      if (L_field_it == m_field_name_map->end()) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "C_ProtocolExternal::build_message field header [" 
		  << L_value_it->m_name << "] not found");
	return (NULL);
      }

      L_convert = false ;
      L_result = -1 ;


      if (P_msg != NULL) {
	char *L_valueConfig = NULL ;
	char *L_variable = NULL ;
	char *L_valueString = L_value_it->m_value ;

	if ((L_variable = is_variable(L_valueString)) != NULL) {
	  L_valueConfig = find_config_value(L_variable);
	  if (L_valueConfig == NULL) {
	    // => error variable existe pas
	    GEN_ERROR(E_GEN_FATAL_ERROR, "Unable to find [" 
		      << L_variable 
		      << "] in configuration parameters");
      
	    FREE_TABLE(L_variable);
	    return(NULL);
	  } else {
	    L_valueString = L_valueConfig;
	  }
	  FREE_TABLE(L_variable);
	} 

	if (m_from_string_table[(L_field_it->second)->m_id] 
	    != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
	  L_convert = ((m_factory_context)->*(m_from_string_table[(L_field_it->second)->m_id]))
	    (&(L_valueString), &L_value) ;
	  if (L_convert == true) { L_result = 0 ; }
	}
	if (L_convert == false) {
	  L_value = valueFromString(L_valueString, (L_field_it->second)->m_type, L_result);
	}
	if (L_result == -1) {
	  return (NULL);
	}

      } else {
	
	if (m_from_string_table[(L_field_it->second)->m_id] 
	    != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
	  L_convert = ((m_factory_context)->*(m_from_string_table[(L_field_it->second)->m_id]))
	    (&(L_value_it->m_value), &L_value) ;
	  if (L_convert == true) { L_result = 0 ; }
	}
	if (L_convert == false) {
	  
	  L_value = valueFromString(L_value_it->m_value, (L_field_it->second)->m_type, L_result);
	}
	if (L_result == -1) {
	  return (NULL);
	}
	
      }

      L_value.m_id = (L_field_it->second)->m_id ;

      copyValue(L_header[(L_field_it->second)->m_id], L_value, true);

      resetMemory(L_value);
    }
  }

  if ((P_header.m_list_body != NULL ) 
      && (!((P_header.m_list_body)->empty()))) {  

    // count the body and body field
    for(L_body_it = (P_header.m_list_body)->begin();
	L_body_it != (P_header.m_list_body)->end();
	L_body_it++) {

      // ctrl the body contains in the map of body
      L_it_body = m_body_value_name_map->find(T_FieldBodyNameMap::key_type(L_body_it->m_name));
      if (L_it_body == m_body_value_name_map->end()) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "field body [" << L_body_it->m_name << "] not found");
	return (NULL);
      }
      L_id = ((L_it_body->second)->m_id) - m_end_header_index -1 ;

    } // for(L_body_it ..)
    

    for(L_body_it = (P_header.m_list_body)->begin();
	L_body_it != (P_header.m_list_body)->end();
	L_body_it++) {
      
      // ctrl the body contains in the map of body
      L_it_body = m_body_value_name_map->find(T_FieldBodyNameMap::key_type(L_body_it->m_name));

      L_id = ((L_it_body->second)->m_id) - m_end_header_index - 1 ;

      ALLOC_TABLE(L_a_body, T_pValueData, sizeof(T_ValueData), 
		  m_nb_body_fields);
      for (L_i = 0 ; L_i < m_nb_body_fields; L_i ++) {
	copyValue(L_a_body[L_i], 
		  ((L_it_body->second)->m_header)[L_i], false);
      }
      L_body.push_back(L_a_body);

      L_body_id.push_back(L_id);
      L_instance = L_body_instance[L_id] ;
      L_body_instance[L_id]++;

      if (L_body_it->m_instance != NULL) {

	T_InstanceData L_data ;
	T_InstanceDataList::iterator L_it_data ;
	L_data.m_instance_name = L_body_it->m_instance ;

	if (!P_list->empty()) {
	  for (L_it_data = P_list->begin();
	       L_it_data != P_list->end();
	       L_it_data++) {
	    if (strcmp(L_it_data->m_instance_name, L_body_it->m_instance) == 0) {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
			"Duplicate instance name [" << L_body_it->m_instance << "]");
	      return (NULL);
	    }
	  }
	}
	
	L_data.m_id = (L_it_body->second)->m_id ;
	L_data.m_instance_id = L_instance ;

	P_list->push_back(L_data);
      }
      
      
      // build header field for this body
      if (L_body_it->m_list_value != NULL)  {  

	GEN_DEBUG(1, "C_ProtocolExternal::build_message() "
		  << "L_body_it->m_list_value size [" << (L_body_it->m_list_value)->size() << "]");
	      
	
	for(L_value_it = (L_body_it->m_list_value)->begin();
	    L_value_it != (L_body_it->m_list_value)->end();
	    L_value_it++) {
	  
	  GEN_DEBUG(1, "C_ProtocolExternal::build_message() "
		    << "L_value_it->m_name [" << L_value_it->m_name << "]");
	  
     L_field_it = m_field_body_name_map->find(T_FieldNameMap::key_type(L_value_it->m_name));
     if (L_field_it == m_field_body_name_map->end()) {
       GEN_ERROR(E_GEN_FATAL_ERROR,
          "C_ProtocolExternal::build_message field body ["
          << L_value_it->m_name << "] not found");
       return (NULL);
     }

	  L_convert = false ;
	  L_result = -1 ;
     if (m_from_string_field_body_table[((L_field_it->second)->m_id) - m_start_body_index] 
	      != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
	    L_convert = 
	      ((m_factory_context)->*(m_from_string_field_body_table[((L_field_it->second)->m_id)- m_start_body_index]))
	      (&(L_value_it->m_value), &L_value) ;
	    if (L_convert == true) { L_result = 0 ; }
	  }
	  if (L_convert == false) {
	    L_value = valueFromString(L_value_it->m_value, (L_field_it->second)->m_type, L_result);
	  }



	  if (L_result == -1) {
	    return (NULL);
	  }

	  L_field_id = ((L_field_it->second)->m_id) - m_start_body_index ;
	  L_value.m_id = (L_field_it->second)->m_id ;
	  copyValue(L_a_body[L_field_id], 
		    L_value, 
		    true);
	  resetMemory(L_value);

	} // for(L_value_it =...)
      } // if (L_body_it->m_list_value != NULL)
    } // for(L_body_it = ...)
  } // (P_header.m_list_body != NULL ...)

  // TO DO: add body from dictionnary

  if ((P_nb_value != NULL) && (!L_body.empty())) {
    (*P_nb_value) = L_body.size() ;
  }
  

  NEW_VAR(L_msg, C_MessageExternal(this, L_header, &L_body, &L_body_id, L_body_instance));

  

  FREE_TABLE(L_body_instance);
  if (!L_body.empty()) {
    L_body.erase(L_body.begin(), L_body.end());
    L_body_id.erase(L_body_id.begin(), L_body_id.end());
  }

  GEN_DEBUG(1, "C_ProtocolExternal::build_message() end");    
  
  if (!L_confict_set.empty()) {
    L_confict_set.erase(L_confict_set.begin(), L_confict_set.end());
  }


  return (L_msg);
}

int C_ProtocolExternal::find_field_id (char*P_name) {

  int L_id = -1 ;
  T_FieldNameMap::iterator     L_it                      ;
  T_FieldBodyNameMap::iterator L_body_it                 ; 

  // retrieve id and type of session id
  if (P_name != NULL ) {
    // retrieve session id in header field map (m_field_name_map)
    L_it = m_field_name_map->find(T_FieldNameMap::key_type(P_name));
    if (L_it != m_field_name_map->end()) {
      L_id = (L_it->second)->m_id ;
    } else {
      // retrieve session id in body map (m_body_value_name_map)
      L_body_it = m_body_value_name_map->
	find(T_FieldBodyNameMap::key_type(P_name));
      if (L_body_it != m_body_value_name_map->end()) {
	L_id = (L_body_it->second)->m_id ;
      } 
      
    }
  } 
  return (L_id);
}


C_MsgBuildContextFactory* C_ProtocolExternal::get_factory() {
  return (m_factory);
}
 
void C_ProtocolExternal::log_message (char *P_header, C_MessageFrame *P_msg) {
  //  C_MessageExternal *L_msg = dynamic_cast<C_MessageExternal*>(P_msg);

  if (genTraceLevel & gen_mask_table[LOG_LEVEL_MSG]) {
    GEN_LOG_EVENT(LOG_LEVEL_MSG, 
		  "Message " << P_header << " [ " << *P_msg <<
		  GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL << "]" );

  }

}


bool C_ProtocolExternal::check_sub_entity_needed (int P_id) {
  bool L_ret = false;
  
  if ((P_id > m_end_header_index) && 
      (P_id <= (m_end_header_index + m_nb_body_values))) {
    L_ret = true ;
  }
  return (L_ret);
}

char *C_ProtocolExternal::get_string_value (int P_id, T_pValueData P_data) {
 
  char *L_string = NULL ;

  if (m_to_string_table[P_id] != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
    (void)((m_factory_context)->*(m_to_string_table[P_id]))(&L_string, P_data) ;
  }
  
  return (L_string);
 
}


char *C_ProtocolExternal::get_string_value_field_body (int P_id, T_pValueData P_data) {
 
  char *L_string = NULL ;

  if (m_to_string_field_body_table[P_id] != (C_MsgBuildContext::T_ContextStringFunction)NULL) {
    (void)((m_factory_context)->*(m_to_string_field_body_table[P_id]))(&L_string, P_data) ;
  }
  
  return (L_string);
 
}

char* C_ProtocolExternal::find_config_value(char* P_varible) {
  
  char* L_value = NULL ;

  T_ConfigValueList::iterator  L_configValue_it         ;
  GEN_DEBUG(1, "C_ProtocolExternal::find_config_value() start");    

  if (!m_config_value_list->empty()) {
    for (L_configValue_it = m_config_value_list->begin();
	 L_configValue_it != m_config_value_list->end();
	 L_configValue_it++) {
      if (strcmp(L_configValue_it->m_name, P_varible) == 0) {
	L_value = L_configValue_it->m_value ;
	break ;
      }
    }
  }
    
  GEN_DEBUG(1, "C_ProtocolExternal::find_config_value() end");    
  return (L_value);

}


char* C_ProtocolExternal::is_variable(char* P_varibleString) {
  
  char *L_ptr     = P_varibleString ;
  char *L_value   = NULL            ;
  char *L_search  = NULL            ;
  char *L_ptr_end = NULL            ;

  int   L_value_size                ;

  if ((L_ptr == NULL) ||
      ((strlen(L_ptr)) <= 2)) {
    return (L_value) ;
  } 

  if (!((L_ptr[0] == '$') && (L_ptr[1] == '('))) {
    return (L_value) ;
  } else {
    L_search = L_ptr + 2 ;
    L_ptr_end = strchr(L_search, ')') ;
    if (L_ptr_end == NULL) {
      return (L_value) ;
    } else {
      L_value_size =  L_ptr_end - L_search ; 
      ALLOC_TABLE(L_value,
  		  char*, 
  		  sizeof(char),
  		  L_value_size+1);
      memcpy(L_value, L_search, L_value_size);
      L_value[L_value_size] = 0 ;
    }    
  }

  GEN_DEBUG(1, "C_ProtocolExternal::retrieve_config_value() end");    
  return (L_value);

}

bool C_ProtocolExternal::find_present_session (int P_msg_id,int P_id) {
  return (true);
}

