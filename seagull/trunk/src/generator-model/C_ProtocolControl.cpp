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

#include "C_ProtocolControl.hpp"
#include "Utils.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "C_ProtocolBinary.hpp"
#include "C_ProtocolExternal.hpp"
#include "C_ProtocolBinaryBodyNotInterpreted.hpp"
#include "C_ProtocolBinarySeparator.hpp"
#include "C_ProtocolText.hpp"

#define XML_PROTOCOL_SECTION (char*)"protocol"
#define XML_PROTOCOL_NAME    (char*)"name"
#define XML_PROTOCOL_TYPE    (char*)"type"


C_ProtocolControl::C_ProtocolControl(C_TransportControl *P_transport_control) {
  GEN_DEBUG(1, "C_ProtocolControl::C_ProtocolControl() start");
  NEW_VAR(m_name_map, T_ProtocolNameMap());
  m_name_map->clear() ;
  m_protocol_table = NULL ;
  m_protocol_name_table = NULL ;
  m_protocol_table_size = 0 ;
  NEW_VAR(m_id_gen, C_IdGenerator());
  m_transport_control = P_transport_control ;
  GEN_DEBUG(1, "C_ProtocolControl::C_ProtocolControl() end");
}

C_ProtocolControl::~C_ProtocolControl() {

  int L_i ;


  GEN_DEBUG(1, "C_ProtocolControl::~C_ProtocolControl() start");
  if (!m_name_map->empty()) {
    m_name_map->erase(m_name_map->begin(), m_name_map->end());
  }
  DELETE_VAR(m_name_map);

  if (m_protocol_table_size != 0) {
    for (L_i = 0 ; L_i < m_protocol_table_size; L_i++) {
      DELETE_VAR(m_protocol_table[L_i]);
    }

    FREE_TABLE(m_protocol_table);
    FREE_TABLE(m_protocol_name_table);
    m_protocol_table_size = 0 ;
  }
  DELETE_VAR(m_id_gen) ;
  m_transport_control = NULL ;
  GEN_DEBUG(1, "C_ProtocolControl::~C_ProtocolControl() end");
}

char* C_ProtocolControl::get_protocol_name(C_XmlData *P_data) {
  return (P_data->find_value(XML_PROTOCOL_NAME)) ;
}

char* C_ProtocolControl::get_protocol_type(C_XmlData *P_data) {
  return (P_data->find_value(XML_PROTOCOL_TYPE)) ;
}


bool C_ProtocolControl::fromXml (C_XmlData *P_data,
				 T_pConfigValueList P_config_value_list,
				 bool               P_display_protocol_stats) {
  
  bool                          L_ret       = true   ;
  T_pXmlData_List               L_subList            ;
  T_XmlData_List::iterator      L_subListIt          ;
  C_XmlData                    *L_data               ;
  char                         *L_protocol_name, 
                               *L_protocol_type      ;
  int                           L_protocol_id        ;
  T_ProtocolInstList            L_protocol_inst_list ;
  T_pProtocolInstanceInfo       L_protocol_info      ;
  T_ProtocolInstList::iterator  L_it                 ;

  

  GEN_DEBUG(1, "C_ProtocolControl::fromXml() start");
  if (P_data != NULL) {
    if ((L_subList = P_data->get_sub_data()) != NULL) {
      for (L_subListIt = L_subList->begin() ;
	   L_subListIt != L_subList->end() ;
	   L_subListIt++) {

	L_data = *L_subListIt ;

	if (L_data != NULL) {
	  if (strcmp(L_data->get_name(), XML_PROTOCOL_SECTION) == 0) {
	    // protocol section definition found
	    L_protocol_name = get_protocol_name (L_data) ;
	    // check protocol type for creation
	    L_protocol_type = get_protocol_type (L_data) ;
	    
	    // check name/type presence
	    if (L_protocol_name == NULL) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
			"name mandatory for section " << XML_PROTOCOL_SECTION);
	      L_ret = false ;
	      break ;
	    }
	    if (L_protocol_type == NULL) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
			"type mandatory for section " << XML_PROTOCOL_SECTION);
	      L_ret = false ;
	      break ;
	    }

	    // check protocol name unicity
	    if (m_name_map->find(T_ProtocolNameMap::key_type(L_protocol_name)) 
		!= m_name_map->end()) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
		        XML_PROTOCOL_SECTION << " with name ["
			<< L_protocol_name << "] already defined");
	      L_ret = false ;
	      break ;
	    }

	    // check protocol type or sub type
	    if (strcmp(L_protocol_type, "binary") == 0) {
	      // create protocol instance
	      
	      C_ProtocolBinary    *L_protocol_instance = NULL ;
	      T_ConstructorResult  L_res ;

	      NEW_VAR(L_protocol_instance,
		      C_ProtocolBinary());
              
              L_protocol_instance->construction_data(L_data, &L_protocol_name, &L_res) ;

	      if (L_res != E_CONSTRUCTOR_OK) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "Error found in protocol definition");
		DELETE_VAR(L_protocol_instance);
		L_ret = false ;
		break ;
	      } else {
		// store new instance
		if (P_display_protocol_stats == true) {
		  C_ProtocolStats  *L_protocol_stats ;
		  NEW_VAR(L_protocol_stats,C_ProtocolStats(L_protocol_instance));
		  L_protocol_instance->set_stats(L_protocol_stats);
		}

		L_protocol_id = m_id_gen->new_id() ;
		ALLOC_VAR(L_protocol_info,
			  T_pProtocolInstanceInfo,
			  sizeof(T_ProtocolInstanceInfo));

		L_protocol_info->m_instance = L_protocol_instance ;
		L_protocol_info->m_id = L_protocol_id ;
		L_protocol_info->m_name = L_protocol_name ;
		L_protocol_inst_list.push_back(L_protocol_info);
		m_name_map
		  ->insert(T_ProtocolNameMap::value_type(L_protocol_name,
							 L_protocol_id)) ;
	      }
              
	    } else if (strcmp(L_protocol_type, "binary-body-not-interpreted") == 0) {
	      // create protocol instance
	      C_ProtocolBinaryBodyNotInterpreted    *L_protocol_instance = NULL ;

	      T_ConstructorResult  L_res ;


	      NEW_VAR(L_protocol_instance,
		      C_ProtocolBinaryBodyNotInterpreted());
              
              L_protocol_instance->construction_data(L_data, &L_protocol_name, &L_res) ;
              
	      if (L_res != E_CONSTRUCTOR_OK) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "Error found in protocol definition");
		DELETE_VAR(L_protocol_instance);
		L_ret = false ;
		break ;
	      } else {
		// store new instance
		if (P_display_protocol_stats == true) {
		  C_ProtocolStats  *L_protocol_stats ;
		  NEW_VAR(L_protocol_stats,C_ProtocolStats(L_protocol_instance));
		  L_protocol_instance->set_stats(L_protocol_stats);
		}
                
		L_protocol_id = m_id_gen->new_id() ;
		ALLOC_VAR(L_protocol_info,
			  T_pProtocolInstanceInfo,
			  sizeof(T_ProtocolInstanceInfo));
                
		L_protocol_info->m_instance = L_protocol_instance ;
		L_protocol_info->m_id = L_protocol_id ;
		L_protocol_info->m_name = L_protocol_name ;
		L_protocol_inst_list.push_back(L_protocol_info);
		m_name_map
		  ->insert(T_ProtocolNameMap::value_type(L_protocol_name,
							 L_protocol_id)) ;
	      }

	    } else if (strcmp(L_protocol_type, "binary-separator") == 0) {

	      // create protocol instance
	      C_ProtocolBinarySeparator    *L_protocol_instance = NULL ;

	      T_ConstructorResult  L_res ;


	      NEW_VAR(L_protocol_instance,
		      C_ProtocolBinarySeparator());
              
              L_protocol_instance
		->construction_data(L_data, &L_protocol_name, &L_res) ;
              
	      if (L_res != E_CONSTRUCTOR_OK) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "Error found in protocol definition");
		DELETE_VAR(L_protocol_instance);
		L_ret = false ;
		break ;
	      } else {
		// store new instance
		if (P_display_protocol_stats == true) {
		  C_ProtocolStats  *L_protocol_stats ;
		  NEW_VAR(L_protocol_stats,C_ProtocolStats(L_protocol_instance));
		  L_protocol_instance->set_stats(L_protocol_stats);
		}
                
		L_protocol_id = m_id_gen->new_id() ;
		ALLOC_VAR(L_protocol_info,
			  T_pProtocolInstanceInfo,
			  sizeof(T_ProtocolInstanceInfo));
                
		L_protocol_info->m_instance = L_protocol_instance ;
		L_protocol_info->m_id = L_protocol_id ;
		L_protocol_info->m_name = L_protocol_name ;
		L_protocol_inst_list.push_back(L_protocol_info);
		m_name_map
		  ->insert(T_ProtocolNameMap::value_type(L_protocol_name,
							 L_protocol_id)) ;
	      }
	    } else if (strcmp(L_protocol_type, "external-library") == 0) {
              
	      C_ProtocolExternal    *L_protocol_instance = NULL ;
	      T_ConstructorResult    L_res = E_CONSTRUCTOR_OK ;
	      NEW_VAR(L_protocol_instance,
		      C_ProtocolExternal(m_transport_control, 
					 L_data, &L_protocol_name, 
					 P_config_value_list,
					 &L_res));

	      if (L_res != E_CONSTRUCTOR_OK) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "Error found in protocol definition");
		DELETE_VAR(L_protocol_instance);
		L_ret = false ;
		break ;
	      } else {

		if (P_display_protocol_stats == true) {
		  C_ProtocolStats  *L_protocol_stats ;
		  NEW_VAR(L_protocol_stats,C_ProtocolStats(L_protocol_instance));
		  L_protocol_instance->set_stats(L_protocol_stats);
		}

		// store new instance
		L_protocol_id = m_id_gen->new_id() ;
		ALLOC_VAR(L_protocol_info,
			  T_pProtocolInstanceInfo,
			  sizeof(T_ProtocolInstanceInfo));

		L_protocol_info->m_instance = L_protocol_instance ;
		L_protocol_info->m_id = L_protocol_id ;
		L_protocol_info->m_name = L_protocol_name ;
		L_protocol_inst_list.push_back(L_protocol_info);
		m_name_map
		  ->insert(T_ProtocolNameMap::value_type(L_protocol_name,
							 L_protocol_id)) ;
	      }
	    }  else if (strcmp(L_protocol_type, "text") == 0) {
              
	      C_ProtocolText    *L_protocol_instance = NULL ;
	      T_ConstructorResult    L_res = E_CONSTRUCTOR_OK ;

	      NEW_VAR(L_protocol_instance,
		      C_ProtocolText());
              
              L_protocol_instance->analyze_data(L_data, 
                                                &L_protocol_name, 
                                                P_config_value_list,
                                                &L_res);

            if (L_res != E_CONSTRUCTOR_OK) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "Error found in protocol definition");
		DELETE_VAR(L_protocol_instance);
		L_ret = false ;
		break ;
	      } else {

		if (P_display_protocol_stats == true) {
		  C_ProtocolStats  *L_protocol_stats ;
		  NEW_VAR(L_protocol_stats,C_ProtocolStats(L_protocol_instance));
		  L_protocol_instance->set_stats(L_protocol_stats);
		}

		// store new instance
		L_protocol_id = m_id_gen->new_id() ;
		ALLOC_VAR(L_protocol_info,
			  T_pProtocolInstanceInfo,
			  sizeof(T_ProtocolInstanceInfo));

		L_protocol_info->m_instance = L_protocol_instance ;
		L_protocol_info->m_id = L_protocol_id ;
		L_protocol_info->m_name = L_protocol_name ;
		L_protocol_inst_list.push_back(L_protocol_info);
		m_name_map
		  ->insert(T_ProtocolNameMap::value_type(L_protocol_name,
							 L_protocol_id)) ;
	      }
	    } else {
	      GEN_ERROR(E_GEN_FATAL_ERROR,
		        XML_PROTOCOL_SECTION 
			<< " [" 
			<< L_protocol_name
			<< "] with type ["
			<< L_protocol_type << "] unsupported");
	      L_ret = false ;
	    }
	  }
	}
      }
      if (L_ret != false) {
	if (!L_protocol_inst_list.empty()) {
	  m_protocol_table_size = L_protocol_inst_list.size() ;
	  ALLOC_TABLE(m_protocol_table,
		      T_pC_ProtocolFrame*,
		      sizeof(T_pC_ProtocolFrame),
		      m_protocol_table_size) ;
	  ALLOC_TABLE(m_protocol_name_table,
		      char**,
		      sizeof(char*),
		      m_protocol_table_size) ;
	  for (L_it  = L_protocol_inst_list.begin();
	       L_it != L_protocol_inst_list.end()  ;
	       L_it++) {
	    L_protocol_info = *L_it ;
	    m_protocol_table[L_protocol_info->m_id] 
	      = L_protocol_info->m_instance ;
	    m_protocol_name_table[L_protocol_info->m_id]
	      = L_protocol_info->m_name ;
	  }
	} else {
	  GEN_ERROR(E_GEN_FATAL_ERROR,
		    "No protocol definition found"); 
	  L_ret = false ;
	}
      } // if L_ret != false 
      
      if (!L_protocol_inst_list.empty()) {
	  for (L_it  = L_protocol_inst_list.begin();
	       L_it != L_protocol_inst_list.end()  ;
	       L_it++) {
	    FREE_VAR(*L_it);
	  }
	  L_protocol_inst_list.erase(L_protocol_inst_list.begin(),
				     L_protocol_inst_list.end());
      }
    } 
  }

  GEN_DEBUG(1, "C_ProtocolControl::fromXml() end ret=" << L_ret);
  return (L_ret);
}

C_ProtocolFrame* C_ProtocolControl::get_protocol (char *P_name) {
  C_ProtocolFrame            *L_ret = NULL ;
  int                         L_id         ;

  L_id = get_protocol_id (P_name) ;
  if (L_id != ERROR_PROTOCOL_UNKNOWN) {
    L_ret = m_protocol_table[L_id] ;
  }

  return (L_ret) ;
}

C_ProtocolFrame* C_ProtocolControl::get_protocol (int P_id) {
  C_ProtocolFrame *L_ret = NULL ;
  if ((P_id < m_protocol_table_size) && (P_id >= 0)) {
    L_ret = m_protocol_table[P_id] ;
  }
  return (L_ret);
}

int C_ProtocolControl::get_protocol_id (char *P_name) {
  int                         L_ret = ERROR_PROTOCOL_UNKNOWN ;
  T_ProtocolNameMap::iterator L_it ;

  L_it = m_name_map->find(T_ProtocolNameMap::key_type(P_name)) ;
  if (L_it != m_name_map->end()) {
    L_ret = L_it->second ;
  }
  return (L_ret) ;
}


int C_ProtocolControl::get_nb_protocol () {
  return (m_protocol_table_size);
}

char* C_ProtocolControl::get_protocol_name (int P_id) {
  char *L_ret = NULL ;
  if ((P_id < m_protocol_table_size) && (P_id >= 0)) {
    L_ret = m_protocol_name_table[P_id] ;
  }
  return (L_ret);
}



