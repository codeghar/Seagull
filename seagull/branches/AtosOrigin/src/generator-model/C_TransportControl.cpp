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

#include "C_TransportControl.hpp"
#include "Utils.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "dlfcn_t.hpp"

#define XML_TRANSPORT_SECTION      (char*)"configuration"
#define XML_TRANSPORT_SUBSECTION   (char*)"define"
#define XML_TRANSPORT_ENTITY       (char*)"entity"
#define XML_TRANSPORT_ENTITY_VALUE (char*)"transport"
#define XML_TRANSPORT_NAME         (char*)"name"
#define XML_TRANSPORT_FILE         (char*)"file"
#define XML_TRANSPORT_CREATE       (char*)"create_function"
#define XML_TRANSPORT_DELETE       (char*)"delete_function"
#define XML_TRANSPORT_INIT         (char*)"init-args"

typedef struct _transport_instance_info {
  C_Transport *m_instance ;
  int          m_id       ;
  char        *m_init_args ;
  char        *m_library_file_name ;
  char        *m_instance_create_fct_name ;
  char        *m_instance_delete_fct_name ;
} T_TransportInstanceInfo, *T_pTransportInstanceInfo ;
typedef list_t<T_pTransportInstanceInfo> T_TransportInstList ;

C_TransportControl::C_TransportControl(T_logFunction P_logError, 
				       T_logFunction P_logInfo) {
  GEN_DEBUG(1, "C_TransportControl::C_TransportControl() start");
  NEW_VAR(m_name_map, T_TransportNameMap());
  m_name_map->clear() ;
  m_transport_table_size = 0 ;
  m_transport_context_table = NULL ;
  m_call_select = &select ;
  NEW_VAR(m_id_gen, C_IdGenerator());
  m_log_error = P_logError ;
  m_log_info = P_logInfo ;
  GEN_DEBUG(1, "C_TransportControl::C_TransportControl() end");
}

C_TransportControl::~C_TransportControl() {
  int L_i ;

  GEN_DEBUG(1, "C_TransportControl::~C_TransportControl() start");
  if (!m_name_map->empty()) {
    m_name_map->erase(m_name_map->begin(), m_name_map->end());
  }
  DELETE_VAR(m_name_map);
  if (m_transport_table_size != 0) {
    for (L_i = 0 ; L_i < m_transport_table_size; L_i++) {
      m_transport_context_table[L_i]
	->m_ext_delete(&(m_transport_context_table[L_i]->m_instance));
      FREE_VAR(m_transport_context_table[L_i]);
    }
    FREE_TABLE(m_transport_context_table);
    m_transport_table_size = 0 ;
  }
  DELETE_VAR(m_id_gen) ;
  GEN_DEBUG(1, "C_TransportControl::~C_TransportControl() end");
}

C_Transport* C_TransportControl::get_transport (char *P_name) {
  C_Transport *L_ret = NULL ;
  int          L_id         ;

  L_id = get_transport_id (P_name) ;
  if (L_id != ERROR_TRANSPORT_UNKNOWN) {
    L_ret = m_transport_context_table[L_id]->m_instance ;
  }

  return (L_ret) ;
}

C_Transport* C_TransportControl::get_transport (int   P_id) {
  C_Transport *L_ret = NULL ;
  if ((P_id < m_transport_table_size) && (P_id >= 0)) {
    L_ret = m_transport_context_table[P_id]->m_instance ;
  }
  return (L_ret);
}

C_TransportControl::T_pTransportContext C_TransportControl::get_transport_context (int   P_id) {
  T_pTransportContext L_ret = NULL ;
  if ((P_id < m_transport_table_size) && (P_id >= 0)) {
    L_ret = m_transport_context_table[P_id] ;
  }
  return (L_ret);
}

int C_TransportControl::get_transport_id (char *P_name) {
  int                          L_ret = ERROR_TRANSPORT_UNKNOWN ;
  T_TransportNameMap::iterator L_it ;

  L_it = m_name_map->find(T_TransportNameMap::key_type(P_name)) ;
  if (L_it != m_name_map->end()) {
    L_ret = L_it->second ;
  }
  return (L_ret) ;
}

bool C_TransportControl::fromXml (C_XmlData *P_data) {

  bool                          L_ret = true ;
  T_pXmlData_List               L_sectionList, L_subSectionList            ;
  T_XmlData_List::iterator      L_sectionListIt, L_subSectionListIt ;
  C_XmlData                    *L_section, *L_subSection ;
  char                         *L_entity_value ;
  char                         *L_name, 
                               *L_init_args,
                               *L_library_file_name,
                               *L_instance_create_fct_name,
                               *L_instance_delete_fct_name ;
  int                           L_transport_id ;
  T_pTransportInstanceInfo      L_transport_info = NULL ;
  T_TransportInstList           L_transport_info_list ;
  T_TransportInstList::iterator L_it ;


  GEN_DEBUG(1, "C_TransportControl::fromXml() start");
  if (P_data != NULL) {
    if ((L_sectionList = P_data->get_sub_data()) != NULL) {
      // std::cerr << "L_sectionList->size() " << L_sectionList->size() << std::endl;
      for(L_sectionListIt  = L_sectionList->begin() ;
	  L_sectionListIt != L_sectionList->end() ;
	  L_sectionListIt++) {

	L_section = *L_sectionListIt ;
	if (L_section != NULL) {
	  if (strcmp(L_section->get_name(), XML_TRANSPORT_SECTION) == 0) {

	    L_subSectionList = L_section->get_sub_data() ;
	    for(L_subSectionListIt  = L_subSectionList->begin() ;
		L_subSectionListIt != L_subSectionList->end() ;
		L_subSectionListIt++) {
	      L_subSection = *L_subSectionListIt ;
	      
	      if (strcmp(L_subSection->get_name(), XML_TRANSPORT_SUBSECTION) == 0) {

		L_entity_value = L_subSection->find_value(XML_TRANSPORT_ENTITY) ;
		if (L_entity_value != NULL) {
		  if (strcmp(L_entity_value, XML_TRANSPORT_ENTITY_VALUE) == 0) {

		    L_name = L_subSection->find_value(XML_TRANSPORT_NAME); 
		    if (L_name == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_name << "] value is mandatory for section ["
				<< XML_TRANSPORT_SECTION << "], sub section ["
				<< XML_TRANSPORT_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }
		    L_init_args = L_subSection->find_value(XML_TRANSPORT_INIT);
		    if (L_init_args == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_init_args << "] value is mandatory for section ["
				<< XML_TRANSPORT_SECTION << "], sub section ["
				<< XML_TRANSPORT_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }
		    L_library_file_name = L_subSection->find_value(XML_TRANSPORT_FILE);
		    if (L_library_file_name == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_library_file_name << "] value is mandatory for section ["
				<< XML_TRANSPORT_SECTION << "], sub section ["
				<< XML_TRANSPORT_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }
		    L_instance_create_fct_name = L_subSection->find_value(XML_TRANSPORT_CREATE);
		    if (L_instance_create_fct_name == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_instance_create_fct_name << "] value is mandatory for section ["
				<< XML_TRANSPORT_SECTION << "], sub section ["
				<< XML_TRANSPORT_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }
		    L_instance_delete_fct_name = L_subSection->find_value(XML_TRANSPORT_DELETE);
		    if (L_instance_delete_fct_name == NULL) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, "[" 
				<< L_instance_delete_fct_name << "] value is mandatory for section ["
				<< XML_TRANSPORT_SECTION << "], sub section ["
				<< XML_TRANSPORT_SUBSECTION << "]");
		      L_ret = false ;
		      break ;
		    }

		    // check transport name unicity
		    if (m_name_map->find(T_TransportNameMap::key_type(L_name)) 
			!= m_name_map->end()) {
		      GEN_ERROR(E_GEN_FATAL_ERROR, 
				XML_TRANSPORT_SECTION << " with name ["
				<< L_name << "] already defined");
		      L_ret = false ;
		      break ;
		    }

		    L_transport_id = m_id_gen->new_id() ;
		    ALLOC_VAR(L_transport_info,
			      T_pTransportInstanceInfo,
			      sizeof(T_TransportInstanceInfo));

		    L_transport_info->m_id = L_transport_id ;
		    L_transport_info->m_instance = NULL ;
		    L_transport_info->m_init_args = L_init_args ;
		    L_transport_info->m_library_file_name = L_library_file_name;
		    L_transport_info->m_instance_create_fct_name = L_instance_create_fct_name ;
		    L_transport_info->m_instance_delete_fct_name = L_instance_delete_fct_name;
		    L_transport_info_list.push_back(L_transport_info);
		    m_name_map
		      ->insert(T_TransportNameMap::value_type(L_name,
							      L_transport_id)) ;
		    
		  }

		}
		
	      }

	    }

	  }
	} // if (L_section != NULL)
      } // for(L_sectionListIt

      if (L_ret != false) {
	if (!L_transport_info_list.empty()) {
	  m_transport_table_size = L_transport_info_list.size () ;
	  ALLOC_TABLE(m_transport_context_table,
		      T_pTransportContext*,
		      sizeof(T_pTransportContext),
		      m_transport_table_size);
	  for (L_it  = L_transport_info_list.begin();
	       L_it != L_transport_info_list.end()  ;
	       L_it++) {
	    L_transport_info = *L_it ;
	    ALLOC_VAR(m_transport_context_table[L_transport_info->m_id], 
		      T_pTransportContext, 
		      sizeof(T_TransportContext));
	    if (update_context (m_transport_context_table[L_transport_info->m_id],
				L_transport_info) == false) {
	      L_ret = false ;
	    }
	  }
	} else {
	  GEN_ERROR(E_GEN_FATAL_ERROR,
		    "No transport definition found");
	  L_ret = false ;
	}
      }

      if (!L_transport_info_list.empty()) {
	for (L_it  = L_transport_info_list.begin();
	     L_it != L_transport_info_list.end()  ;
	     L_it++) {
	  if (L_ret == false) { // error
	    if (m_transport_table_size > 0) {
	      L_transport_info = *L_it ;
	      FREE_VAR(m_transport_context_table[L_transport_info->m_id]);
	    }
	  }
	  FREE_VAR(*L_it);
	}
	L_transport_info_list.erase(L_transport_info_list.begin(),
				    L_transport_info_list.end());
      }

      if (L_ret == false) {
	// delete structures
	if (m_transport_table_size > 0) {
	  FREE_TABLE(m_transport_context_table);
	  m_transport_table_size = 0 ;
	}
      }
    }
  }
  GEN_DEBUG(1, "C_TransportControl::fromXml() end ret=" << L_ret);

  return (L_ret);
}

bool C_TransportControl::update_context (T_pTransportContext P_context,
					void                *P_value) {

  bool                     L_ret  = true ;

  T_pTransportInstanceInfo L_info = (T_pTransportInstanceInfo) P_value ;
  T_SelectDef              L_select ;

  GEN_DEBUG(1, "C_TransportControl::update_context() start");
  
  P_context->m_lib_handle 
    = dlopen(L_info->m_library_file_name, RTLD_LAZY);
  if (P_context->m_lib_handle == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Unable to open library file [" 
	      << L_info->m_library_file_name
	      << "] error [" << dlerror() << "]");
    L_ret = false ;
  }
  
  if (L_ret) {

    P_context->m_ext_create
      = (T_TransportCreateInstance) dlsym
      (P_context->m_lib_handle, 
       L_info->m_instance_create_fct_name);
    
    if (P_context->m_ext_create == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Symbol [" << dlerror() << "]");
      L_ret = false ;
    }

    P_context->m_ext_delete
      = (T_TransportDeleteInstance) dlsym
      (P_context->m_lib_handle, 
       L_info->m_instance_delete_fct_name);
    
    if (P_context->m_ext_delete == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Symbol [" << dlerror() << "]");
      L_ret = false ;
    }
    
  }

  if (L_ret) {
    P_context->m_instance = (*(P_context->m_ext_create))() ;
    if (P_context->m_instance == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Transport instance not created");
      L_ret = false ;
    } else {
      L_select = (P_context->m_instance)->select_definition() ;
      if (L_select != NULL) {
	m_call_select = L_select;
      }

      return ( ((P_context->m_instance)->init(L_info->m_init_args, m_log_error, m_log_info) != 0) ? false : true) ; 
    }
  }
  
  GEN_DEBUG(1, "C_TransportControl::update_context() end ret=" << L_ret);
  
  return (L_ret);

}

T_SelectDef  C_TransportControl::get_call_select () {
  return (m_call_select);
}


bool C_TransportControl::set_config_value(T_pConfigValueList P_config_value_list) {
  int                           L_i                   ;
  int                           L_config    = 0       ;

  GEN_DEBUG(1, "C_TransportControl::set_config_value() start");

  if (!P_config_value_list->empty()) {
    if (m_transport_table_size > 0) {
      for (L_i = 0 ; L_i < m_transport_table_size; L_i++) {
	L_config = ((m_transport_context_table[L_i])->m_instance)
	  ->config(P_config_value_list) ; // check of init fct to be done ?
	if (L_config != 0) break ;
      }
      
    }
  }

  GEN_DEBUG(1, "C_TransportControl::set_config_value() end ret=" << L_config);
	    
  return ((L_config ==0) ? true : false);

}

