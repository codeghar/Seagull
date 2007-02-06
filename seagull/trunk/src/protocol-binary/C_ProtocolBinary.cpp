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

#include "C_ProtocolBinary.hpp"

#include "GeneratorError.h"
#include "C_MessageBinary.hpp"
#include "BufferUtils.hpp"

#include <cstdlib> // for strtoul
#include "GeneratorTrace.hpp"

#include "ProtocolData.hpp"

#include "integer_t.hpp"

unsigned long C_ProtocolBinary::get_header_size() {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_size() m_header_size: " << m_header_size);
  return (m_header_size);
}

unsigned long C_ProtocolBinary::get_nb_field_header () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_nb_field_header() m_nb_field_header: " << m_nb_field_header);
  return (m_nb_field_header) ;
}

C_ProtocolBinary::T_pHeaderField 
C_ProtocolBinary::get_header_field_description (int P_id) {

  T_pHeaderField L_result ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_field_description() start");
  L_result = (P_id < (int)m_nb_field_header) ?
    &(m_header_field_table[P_id]) : NULL ;
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_field_description() end");

  return (L_result);
}

C_ProtocolBinary::T_pHeaderField 
C_ProtocolBinary::get_header_field_description (char *P_name) {

  T_pHeaderField         L_result ;
  int                    L_id     ;
  T_IdMap::iterator      L_it     ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_field_description() start");
  L_it = m_header_id_map 
    -> find (T_IdMap::key_type(P_name)) ;
  
  if (L_it != m_header_id_map->end()) {
    L_id = L_it->second ;
    L_result = get_header_field_description(L_id);
  } else {
    L_result = NULL ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_field_description() end");

  return (L_result) ;
}

T_pTypeDef
C_ProtocolBinary::get_type_description (int P_id) {

  T_pTypeDef L_result ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_type_description() start");
  L_result = (P_id < (int)m_nb_types) ?
    &(m_type_def_table[P_id]) : NULL ;
  GEN_DEBUG(1, "C_ProtocolBinary::get_type_description() end");

  return (L_result);
}

T_pTypeDef 
C_ProtocolBinary::get_type_description (char *P_name) {

  T_pTypeDef         L_result ;
  int                    L_id     ;
  T_IdMap::iterator L_it     ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_type_description() start");
  L_it = m_type_id_map 
    -> find (T_IdMap::key_type(P_name)) ;
  
  if (L_it != m_type_id_map->end()) {
    L_id = L_it->second ;
    L_result = get_type_description(L_id);
  } else {
    L_result = NULL ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_type_description() end");

  return (L_result) ;
}

unsigned long C_ProtocolBinary::decode_msg_size (unsigned char* P_buffer, 
					         unsigned long P_size   ) {
  unsigned long  L_ret = 0 ;
  unsigned char *L_buf = P_buffer ;

  GEN_DEBUG(1, "C_ProtocolBinary::decode_msg_size() start");

  // check the size of buffer according to the
  // protocol definition => already done

  if (m_transport_type == 1) {
    L_ret = P_size;
  } else {

  // extract the length value
  L_buf += m_header_length_index ;
  L_ret = convert_bin_network_to_ul
    (L_buf, m_header_field_table[m_header_length_id].m_size);

  }
  GEN_DEBUG(1, "C_ProtocolBinary::decode_msg_size() end return: " << L_ret);

  return (L_ret) ;

}

int C_ProtocolBinary::get_header_type_id () {
  return (m_header_type_id) ;
}


int C_ProtocolBinary::get_header_type_id_body () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_type_id_body() = "
                << m_header_type_id_body);
  return (m_header_type_id_body) ;
}


int C_ProtocolBinary::get_header_length_id () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_length_id() = "
               << m_header_length_id);
  return (m_header_length_id) ;
}

int C_ProtocolBinary::get_header_length_index () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_length_index() = "
               << m_header_length_index);
  return (m_header_length_index) ;
}

void C_ProtocolBinary::set_header_type_id (int P_id) {
  GEN_DEBUG(1, "C_ProtocolBinary::set_header_type_id() start: new P_id = "
               << P_id);
  GEN_DEBUG(1, "C_ProtocolBinary::set_header_type_id() end");
  m_header_type_id = P_id ;
}
void C_ProtocolBinary::set_header_length_id (int P_id) {
  int L_id ;

  GEN_DEBUG(1, "C_ProtocolBinary::set_header_length_id() start");
  m_header_length_id = P_id ;
  m_header_length_index = 0 ;
  for(L_id = 0; L_id < m_header_length_id; L_id++) {
    m_header_length_index += (int) m_header_field_table[L_id].m_size ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::set_header_length_id() end");

}

int C_ProtocolBinary::add_type (char         *P_name, 
			        T_TypeType    P_type, 
			        unsigned long P_size) {

  int L_id = m_nb_types ;

  GEN_DEBUG(1, "C_ProtocolBinary::add_type() start");

  GEN_DEBUG(1, "C_ProtocolBinary::add_type() type["<<L_id<<"] name:" << P_name);
  GEN_DEBUG(1, "C_ProtocolBinary::add_type() type["<<L_id<<"] type:" << P_type);
  GEN_DEBUG(1, "C_ProtocolBinary::add_type() type["<<L_id<<"] size:" << P_size);

  m_type_def_table[L_id].m_id = L_id ;
  m_type_def_table[L_id].m_name = P_name ;
  m_type_def_table[L_id].m_type = P_type ;
  m_type_def_table[L_id].m_size = P_size;

  m_nb_types++ ;
  GEN_DEBUG(1, "C_ProtocolBinary::add_type() end with id: " << L_id);

  return (L_id) ;
}

int C_ProtocolBinary::add_header_field (char           *P_name, 
				        unsigned long   P_size,
				        unsigned long  *P_nb_field,
				        unsigned long  *P_header_size,
				        T_pHeaderField  P_field_table,
				        T_pIdMap        P_map,
				        T_pCondPresence P_condPresence,
                                        long            P_type_id) {

  int L_id = (*P_nb_field) ;

  GEN_DEBUG(1, "C_ProtocolBinary::add_header_field() start (nb field: "
		  << (*P_nb_field) << ")");
  P_field_table[L_id].m_id = L_id ;
  P_field_table[L_id].m_name = P_name ;
  P_field_table[L_id].m_size = P_size ;
  P_field_table[L_id].m_cond_presence = P_condPresence ;
  GEN_DEBUG(1, "C_ProtocolBinary::add_header_field() [" 
		  << L_id << "] m_cond_presence: " <<
		  ((P_condPresence == NULL) ? (unsigned long)0 : P_condPresence ));
  P_field_table[L_id].m_type_id = P_type_id ;
  
  // Set flag to true only for complex type i.e. type_id != -1
  if (P_type_id > -1) {
    m_header_complex_type_presence = true;
  }

  if (P_condPresence == NULL) { // mandatory
    (*P_header_size) += P_size ;
    (*P_nb_field)++ ;
  } else { // optional
    if (m_header_body_start_optional_id == -1) {
      m_header_body_start_optional_id = L_id ;
    }
  }

  P_map->insert(T_IdMap::value_type(P_name, L_id));

  GEN_DEBUG(1, "C_ProtocolBinary::add_header_field() end (nb field: " 
		  << (*P_nb_field) << ")");
  return (L_id) ;
}

int C_ProtocolBinary::get_header_from_xml (C_XmlData *P_def) {

  C_XmlData                *L_data ;
  T_XmlData_List::iterator  L_listIt ;
  T_pXmlData_List           L_subList ;
  char                     *L_value, *L_value_size, *L_value_unit,  *L_value_type;
  char                     *L_value_field_length, *L_value_field_type ;
  int                       L_ret = 0 ;
  unsigned long             L_size    ;
  int                       L_id      ;
  
  int                       L_fieldDefCpt      ;
  T_IdMap::iterator         L_IdMapIt   ;
  int                       L_typeId ;

  char                     *L_transport_field       ;
  char                     *L_value_mask            ;
  unsigned long             L_mask                  ;
  T_pCondPresence           L_presence_mask = NULL  ;
  bool                      L_mask_present  = false ; 



  GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() start");

  // Search the type field definition
  L_value_field_type   = P_def->find_value((char*)"type");
  if (L_value_field_type ==  NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,"type definition value is mandatory");
    L_ret = -1 ;
  } else {
    m_header_type_name = L_value_field_type ;
    GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() m_header_type_name is " << m_header_type_name );
  }

  // Search the length field definition
  L_value_field_length = P_def->find_value((char*)"length");
  if (L_value_field_length == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,"length definition value is mandatory");
    L_ret = -1 ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() m_header_length_name is " 
            << L_value_field_length );

  // Search the Name of protocol exchange element
  m_header_name = P_def->find_value((char*)"name");
  if (m_header_name == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,"header name value is mandatory");
    L_ret = -1 ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() m_header_name is " << m_header_name );


  L_transport_field = P_def->find_value((char*)"transport-driven");
  if (L_transport_field != NULL) {
    if (strcmp(L_transport_field,(char*)"yes") == 0) {
      m_transport_type = 1 ;
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown transport type [" 
                << L_transport_field << "]");
      L_ret = -1 ;
    }
  }

  L_subList = P_def->get_sub_data() ;
  L_fieldDefCpt = 0;

  for(L_listIt  = L_subList->begin() ;
      L_listIt != L_subList->end() ;
      L_listIt++, L_fieldDefCpt++) {

    L_data = *L_listIt ;
    L_value = L_data->get_name() ;

    L_mask_present  = false ; 

    if (strcmp(L_value, (char*)"fielddef") == 0) {
      char *L_endstr ;
      
      GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() fielddef definition:" );
      
      L_value = L_data->find_value((char*)"name") ;
      if (L_value == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef [" << L_fieldDefCpt << "] name value is mandatory");
	L_ret = -1 ;
	break ;
      }
      GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() field name ["
                << L_fieldDefCpt <<"] is " << L_value );

      L_value_unit = L_data->find_value((char*)"unit") ;
      if (L_value_unit != NULL) {
        GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() field unit ["
                  << L_fieldDefCpt <<"] is " << L_value_unit );
        
        // temporary
        if (strcmp(L_value_unit, (char*)"octet") != 0) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported unit ["
                    << L_value_unit << "] value on fielddef [" << L_fieldDefCpt << "] defintion");
	  L_ret = -1 ;
	  break;
        }

        L_value_size = L_data->find_value((char*)"size") ;
        if (L_value_size == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
	        "fielddef [" << L_fieldDefCpt << "] size value is mandatory ["
                    << L_value << "]");
	  L_ret = -1 ;
	  break ;
        }
        L_size = strtoul_f (L_value_size, &L_endstr,10) ;
        GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() field length ["
                  << L_fieldDefCpt <<"] is " << L_size );

        if (L_endstr[0] != '\0') {
          L_size = strtoul_f (L_value_size, &L_endstr,16) ;
          if (L_endstr[0] != '\0') {
            GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
                      << L_value_size << "] bad format  on fielddef [" << L_fieldDefCpt << "] defintion");
            L_ret = -1 ;
            break ;
          }
        }
        if ( L_size > sizeof(unsigned long)) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef [" << L_fieldDefCpt 
                    << "] max size value ["  << sizeof(unsigned long) << "]");
	  L_ret = -1 ;
	  break ;
        }

        if (m_msg_length_start_detected == false) {
          m_msg_length_start += L_size ;
        }
        
        // Set default unset type
        L_typeId = -1;

        L_value_mask = L_data->find_value((char*)"mask") ;
        if (L_value_mask != NULL) {
          L_mask =  strtoul_f (L_value_mask, &L_endstr,10) ;
          if (L_endstr[0] != '\0') {
            L_mask = strtoul_f (L_value_mask, &L_endstr,16) ;
            if (L_endstr[0] != '\0') {
              GEN_ERROR(E_GEN_FATAL_ERROR, "typedef mask value ["
                        << L_value_mask 
                        << "] bad format  on fielddef [" 
                        << L_fieldDefCpt << "] defintion");
              L_ret = -1 ;
              break ;
            }
          }
          if (L_ret != -1) {
            L_mask_present  = true ; 
          }
        }

      } else {
        L_value_type = L_data->find_value((char*)"type") ;
        if (L_value_type == NULL) {
          GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef [" << L_fieldDefCpt 
                    << "] unit or type definition is mandatory");
          L_ret = -1 ;
          break ;
        }
        GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() field type ["
                  << L_fieldDefCpt <<"] is " << L_value_type );

        // retrieve size of type
        L_IdMapIt =
          m_type_id_map->find(T_IdMap::key_type(L_value_type));
        if (L_IdMapIt != m_type_id_map->end()) {
          L_typeId = L_IdMapIt->second ;
        } else {
          GEN_ERROR(E_GEN_FATAL_ERROR,
                "Type ["
                << L_value_type << "] not defined on fielddef [" << L_fieldDefCpt << "] definition");
          L_ret = -1 ;
          break;
        }
        L_size = m_type_def_table[L_typeId].m_size;
        GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() field length ["
                  << L_fieldDefCpt <<"] is " << L_size );
      }

      if (L_ret != -1) {
 	L_id = add_header_field (L_value, 
				 L_size,
				 &m_nb_field_header,
				 &m_header_size,
				 m_header_field_table,
				 m_header_id_map,
                                 NULL,
                                 L_typeId) ;
        
  	if (strcmp(L_value, L_value_field_type) == 0) {
  	  set_header_type_id(L_id) ;
  	}
        
   	if (strcmp(L_value, L_value_field_length) == 0) {
   	  set_header_length_id(L_id) ;
   	}
        

        if (L_mask_present) {
          ALLOC_VAR(L_presence_mask, T_pCondPresence, sizeof(T_CondPresence)) ;
          L_presence_mask->m_mask = L_mask ;
          L_presence_mask->m_f_id = L_id ;
          m_header_field_table[L_id].m_cond_presence = L_presence_mask ;
        }

      }
    } //     if (strcmp(L_value, (char*)"fielddef") == 0) 

    if (strcmp(L_value, (char*)"start-length") == 0) {
      m_msg_length_start_detected = true ;
    }

  } // for

  if (m_msg_length_start_detected == false) {
    m_msg_length_start = 0 ;
  }

  if (m_header_length_id == -1) {
    GEN_WARNING("length field name not defined in protocol");
  }

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_from_xml() end");

  return (L_ret);
}

int C_ProtocolBinary::analyze_types_from_xml (C_XmlData *P_def) {

  int                       L_ret = 0 ;
  C_XmlData                *L_data    ;

  GEN_DEBUG(1, "C_ProtocolBinary::analyze_types_from_xml() start");

  L_data = P_def ;

  m_max_nb_types = (L_data->get_sub_data())->size() ;
  ALLOC_TABLE(m_type_def_table,
              T_pTypeDef, sizeof(T_TypeDef),
              m_max_nb_types);
  L_ret = get_types_from_xml(L_data);

  GEN_DEBUG(1, "C_ProtocolBinary::analyze_types_from_xml() end");
  return (L_ret) ;
}

int C_ProtocolBinary::analyze_header_from_xml (C_XmlData *P_def) {

  int                       L_ret = 0 ;
  C_XmlData                *L_data    ;
  
  GEN_DEBUG(1, "C_ProtocolBinary::analyze_header_from_xml() start");

  L_data = P_def ;

  m_max_nb_field_header = (L_data->get_sub_data())->size() ;
  m_nb_field_header = 0 ;
  ALLOC_TABLE(m_header_field_table, 
              T_pHeaderField, sizeof(T_HeaderField), 
              m_max_nb_field_header) ;
  L_ret = get_header_from_xml(L_data);

  GEN_DEBUG(1, "C_ProtocolBinary::analyze_header_from_xml() end");
  return (L_ret) ;
}


int C_ProtocolBinary::analyze_body_from_xml (C_XmlData *P_def) {
  int                       L_ret = 0                       ;
  C_XmlData                *L_data                          ;
  
  T_pXmlData_List           L_subListHeader, L_subListValue ;
  T_XmlData_List::iterator  L_listItHeader, L_listItValue   ; 
  
  int                       L_nb_optional_field = 0         ; 
  bool                      L_headerBodyFound = false       ;

  
  GEN_DEBUG(1, "C_ProtocolBinary::analyze_body_from_xml() start");
  
  L_subListHeader = P_def->get_sub_data();
  
  if (L_subListHeader != NULL) {
    
    for (L_listItHeader = L_subListHeader->begin() ;
         L_listItHeader != L_subListHeader->end() ;
         L_listItHeader++) {
      L_data = *L_listItHeader ;

      if ((L_data->get_name() != NULL ) && (strcmp(L_data->get_name(), (char*)"header") == 0)) {
        
        if (L_data->get_sub_data() != NULL) {
          
          // header definition for body values
          m_max_nb_field_header_body = (L_data->get_sub_data())->size() ;
          L_subListValue = L_data->get_sub_data () ;
          for (L_listItValue = L_subListValue->begin();
               L_listItValue != L_subListValue->end();
               L_listItValue ++) {
            if ( strcmp((*L_listItValue)->get_name(), 
                        (char*) "optional" ) == 0) { 
              if (L_nb_optional_field == 0) {
                m_max_nb_field_header_body -- ;
                if ((*L_listItValue)->get_sub_data() != NULL) {
                  L_nb_optional_field
                    = ((*L_listItValue)->get_sub_data())->size() ;
                }
              } else {
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "Only one section optional is allowed");
                L_ret = -1 ;
                break ;
              }
            } else {
              if (L_nb_optional_field != 0) {
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "The section optional must be"
                          << " at the end of header");
                L_ret = -1 ;
                break ;
              }
            }
          }

          if (L_ret == -1) break ;
          m_max_nb_field_header_body += L_nb_optional_field ;
          m_nb_field_header_body = 0 ;
          L_headerBodyFound = true ;
          ALLOC_TABLE(m_header_body_field_table, 
                      T_pHeaderField, sizeof(T_HeaderField), 
                      m_max_nb_field_header_body) ;
          L_ret = get_header_body_from_xml (L_data);
          if (L_ret == -1) break ;
        } // get_sub_data != NULL
      } // if ((L_data->get_name() != NULL )
    }   // for (L_listItHeader = L_subListHeader->begin()
    
    if (L_headerBodyFound == false)  {
      L_ret = -1 ;
      GEN_ERROR(E_GEN_FATAL_ERROR, "No body definition found for protocol");
    }
    
  } // if (L_subListHeader != NULL) 
  
  GEN_DEBUG(1, "C_ProtocolBinary::analyze_body_from_xml() end");
  return (L_ret);
}

int C_ProtocolBinary::analyze_dictionnary_from_xml (C_XmlData *P_def) {

  int                       L_ret = 0                      ;
  C_XmlData                *L_data                         ;
  T_pXmlData_List           L_subListDico                  ;  
  T_XmlData_List::iterator  L_listItDico                   ; 
  bool                      L_headerValueFound = false     ;
  bool                      L_headerBodyValueFound = false ;  
  int                       L_i                            ;

  GEN_DEBUG(1, "C_ProtocolBinary::analyze_dictionnary_from_xml() start");

  L_subListDico = P_def->get_sub_data();
  if (L_subListDico != NULL) {

    for (L_listItDico = L_subListDico->begin() ;
         L_listItDico != L_subListDico->end() ;
         L_listItDico++) {
      L_data = *L_listItDico ;
      
      GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() L_data->get_name is \"" 
                << L_data->get_name() << "\"");
      GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() m_header_name is \"" 
                << m_header_name << "\" and m_header_body_name is \"" << 
                m_header_body_name << "\"" ); 
      
      if ((L_data->get_name() != NULL ) 
	  && (strcmp(L_data->get_name(), m_header_name) == 0)) {
        // "Message" defintion
 
        L_ret = analyze_sessions_id_from_xml(L_data);
        if (L_ret == -1) break;
        // header values dictionnary definitions

        if (L_data->get_sub_data() != NULL) {
          L_headerValueFound = true ;
          
          m_nb_header_values = (L_data->get_sub_data())->size() ;
          
          ALLOC_TABLE(m_header_value_table,
                      T_pHeaderValue,
                      sizeof(T_HeaderValue),
                      m_nb_header_values);
          
          ALLOC_TABLE(m_body_value_table,
                      T_pBodyValueDef,
                      sizeof(T_BodyValueDef),
                      m_nb_header_values);
          for (L_i = 0; L_i < (int)m_nb_header_values; L_i++) {
            m_body_value_table[L_i].m_nb_values = 0 ;
            m_body_value_table[L_i].m_value_table = NULL ;
          }
          L_ret = get_header_values_from_xml (L_data);
          if (L_ret == -1) break ;
        }
      }
      
      if ((L_data->get_name() != NULL ) 
	  && (strcmp(L_data->get_name(), m_header_body_name) == 0)) {
        // body values dictionnary definitions
        if (L_data->get_sub_data() != NULL) {
          L_headerBodyValueFound = true ;
          m_nb_header_body_values = (L_data->get_sub_data())->size() ;
          
          GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() m_nb_header_body_values is " << 
                    m_nb_header_body_values);
          L_ret = get_header_body_values_from_xml (L_data);
          if (L_ret == -1) break ;
        }
      }

    } // for (L_listItDico = L_subListDico->begin()
  } // L_subListDico

  if (L_ret != -1) {
    if (L_headerValueFound == false) {
      char *L_name = (m_header_name == NULL) ? 
        (char*)"no" : m_header_name ;
      GEN_WARNING("No section [" << L_name << "] found");
    } 
  }
  
#ifdef DEBUG_MODE
  {
    char *L_name = (m_header_name == NULL) ? (char*)"no" : m_header_name ;
    GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() "
              << "header_name = ["
              << L_name << "]");
  }
#endif

  if (L_ret != -1) {
    if (L_headerBodyValueFound == false) {
      char* L_name = (m_header_body_name == NULL) ? 
        (char*)"no" : m_header_body_name ;
      GEN_WARNING("No section [" << L_name << "] found");
    }
  }
  
#ifdef DEBUG_MODE
  {
    char* L_name = (m_header_body_name == NULL) ? 
      (char*)"no" : m_header_body_name ;
    GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() "
              << "header_body_name = ["
              <<  L_name
              << "]");
  }
#endif
 
  
  GEN_DEBUG(1, "C_ProtocolBinary::analyze_dictionnary_from_xml() end");
  return (L_ret);
}


int C_ProtocolBinary::xml_interpretor(C_XmlData *P_def) {

  C_XmlData                *L_data                           ;
  T_pXmlData_List           L_subList                        ; 
  T_XmlData_List::iterator  L_listIt                         ;
  char                     *L_value                          ;
  int                       L_ret = 0                        ;

  bool                      L_headerFound = false            ;

  GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() start");


  m_nb_header_values = 0 ;
  m_header_value_table = NULL ;
  m_body_value_table = NULL ;
  NEW_VAR(m_header_value_id_map, T_IdMap());
  NEW_VAR(m_header_decode_map, T_DecodeMap());
  m_header_value_id_map->clear();
  m_header_decode_map->clear();

  m_nb_header_body_values = 0 ;
  m_header_body_value_table = NULL ;
  NEW_VAR(m_header_body_value_id_map, T_IdMap());
  NEW_VAR(m_header_body_decode_map, T_DecodeMap());
  m_header_body_value_id_map->clear();
  m_header_body_decode_map->clear();

  L_subList = P_def->get_sub_data() ;
  if (L_subList == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "No protocol definition");
    L_ret = -1 ;
  } else {

    for(L_listIt  = L_subList->begin() ;
	L_listIt != L_subList->end() ;
	L_listIt++) {

      L_data = *L_listIt ;
      L_value = L_data->get_name() ;

      // Type definition
      if (strcmp(L_value, "types") == 0) {
        L_ret = analyze_types_from_xml (L_data) ;
	if (L_ret == -1) break ;
      }
      
      // Message Header definition
      if (strcmp(L_value, "header") ==0) {
	L_headerFound = true ;
        L_ret = analyze_header_from_xml (L_data) ;
	if (L_ret == -1) break ;
      }

      // Message Body definition
      if (strcmp(L_value, "body") ==0) {
        L_ret = analyze_body_from_xml (L_data);
        if (L_ret == -1) break ;
      } // body

      // Message dictionary for "fields" and "Message" definition
      if (strcmp(L_value, (char*)"dictionary") ==0) {
        //        L_ret = analyze_dictionnary_from_xml (L_data, &L_session_id_name, &L_outof_session_id_name);
        L_ret = analyze_dictionnary_from_xml (L_data);
        if (L_ret == -1) break ;
      } // L_value == dico

    } // L_listIt

    if (L_headerFound == false) {
      L_ret = -1 ;
      GEN_ERROR(E_GEN_FATAL_ERROR, "No header definition found for protocol");
    }
  } // else 

  GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() end");
  return (L_ret) ;
}

C_ProtocolBinary::C_ProtocolBinary() {


  GEN_DEBUG(1, "C_ProtocolBinary::C_ProtocolBinary() start");

  m_header_size = (unsigned long) 0 ;
  m_header_body_size = (unsigned long) 0;
  m_nb_types = 0 ;
  m_header_body_start_optional_id = -1 ; // no optional
  m_header_type_id = -1 ;
  m_header_type_id_body = -1 ; // no optional
  m_header_complex_type_presence = false;
  m_header_length_excluded = false ;
  m_padding_value = 0 ;

  m_header_name = NULL ;
  m_header_body_name = NULL ;
  m_nb_header_body_values = 0;
  m_max_nb_field_header_body = 0;
  m_header_body_field_table = NULL;

  m_header_length_id = -1 ;
  m_transport_type = 0 ;

  m_header_body_type_id = -1 ;
  m_header_body_length_id = -1;
  
  m_header_body_field_separator = NULL ;

  m_msg_length_start = (unsigned long) 0 ;
  m_msg_length_start_detected = false    ;

  NEW_VAR (m_message_name_list, T_NameAndIdList()) ;
  NEW_VAR (m_message_comp_name_list, T_NameAndIdList()) ;

  m_session_id_position = -1 ;

  GEN_DEBUG(1, "C_ProtocolBinary::C_ProtocolBinary() end");
  
}


void C_ProtocolBinary::construction_data(C_XmlData *P_def,
                                      char      **P_name,
                                      T_pContructorResult P_res) {

  char         *L_string_val ;
  char         *L_endstr = NULL ;
  unsigned long L_val ;

  T_ConstructorResult L_res = E_CONSTRUCTOR_OK ;

  if (P_def) {
    NEW_VAR(m_header_id_map, T_IdMap());
    m_header_id_map->clear();

    NEW_VAR(m_messageList, list_t<C_MessageFrame*>);
    m_messageList->clear();

    NEW_VAR(m_header_body_id_map, T_IdMap());
    m_header_body_id_map->clear();

    NEW_VAR(m_type_id_map, T_IdMap());
    m_type_id_map->clear();

    *P_name = P_def->find_value((char*)"name") ;
    if (*P_name == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
		"No name for protocol definition");
      L_res = E_CONSTRUCTOR_KO ;
    }

    if (L_res == E_CONSTRUCTOR_OK) {
      m_name = *P_name ;
      
      L_string_val = P_def->find_value((char*)"padding");
      if (L_string_val != NULL) {
	
	L_val = strtoul_f (L_string_val, &L_endstr,10) ;
	if (L_endstr[0] != '\0') {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "bad format ["
		    << L_string_val << "] number expected for padding");
	} else {
	  m_padding_value = L_val ;
	}
      }
      
      L_string_val = P_def->find_value((char*)"length");
      if (L_string_val != NULL) {
        if (strcmp(L_string_val, (char*)"header-excluded") == 0) {
	  m_header_length_excluded = true ;
        }
      }
      

      *P_res = (xml_interpretor(P_def) ==0) 
	? E_CONSTRUCTOR_OK : E_CONSTRUCTOR_KO ;
    } else {
      *P_res = L_res ;
    }

  } else {
    *P_res = E_CONSTRUCTOR_KO ;
    m_header_id_map = NULL ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::C_ProtocolBinary() start");
}

void C_ProtocolBinary::set_padding(unsigned long P_value) {
  m_padding_value = P_value ;
}

C_ProtocolBinary::~C_ProtocolBinary() {

  int L_i ;

  GEN_DEBUG(1, "C_ProtocolBinary::~C_ProtocolBinary() start");


  // Add clear the map message 
  if (m_messageList != NULL)
  {
    if (!m_messageList->empty()) {
      list_t<C_MessageFrame*>::iterator L_messageIt;
      C_MessageBinary                  *L_msg;
      for (L_messageIt = m_messageList->begin();
           L_messageIt !=  m_messageList->end();
	   L_messageIt++)
      {
        L_msg = dynamic_cast<C_MessageBinary*>(*L_messageIt);
        DELETE_VAR(L_msg);
      }
      m_messageList->erase(m_messageList->begin(), m_messageList->end());
    }
    DELETE_VAR(m_messageList);
  }

  m_header_size = (unsigned long) 0 ;


  for (L_i = 0 ; L_i < (int)m_max_nb_field_header; L_i++) {
    FREE_VAR(m_header_field_table[L_i].m_cond_presence);
  }
  FREE_TABLE(m_header_field_table);

  m_nb_field_header = 0 ;
  m_max_nb_field_header = 0 ;
  m_header_type_id = -1 ;
  m_header_length_id = -1 ;

  m_header_body_length_id = -1;


  if (m_header_id_map != NULL) {
    if (!m_header_id_map->empty()) {
      m_header_id_map->erase(m_header_id_map->begin(), m_header_id_map->end());
    }
  }
  DELETE_VAR(m_header_id_map);

  m_header_body_size = (unsigned long) 0 ;
  
  if (m_header_body_field_table != NULL) {
    for (L_i = 0 ; L_i < (int)m_max_nb_field_header_body; L_i++) {
      FREE_VAR(m_header_body_field_table[L_i].m_cond_presence);
    }
    FREE_TABLE(m_header_body_field_table);
  }

  FREE_TABLE(m_type_def_table);

  if (m_header_body_id_map != NULL) {
    if (!m_header_body_id_map->empty()) {
      m_header_body_id_map->erase(m_header_body_id_map->begin(), m_header_body_id_map->end());
    }
  }
  DELETE_VAR(m_header_body_id_map);

  if (m_type_id_map != NULL) {
    if (!m_type_id_map->empty()) {
      m_type_id_map->erase(m_type_id_map->begin(), m_type_id_map->end());
    }
  }
  DELETE_VAR(m_type_id_map);


  for (L_i = 0; L_i < (int)m_nb_header_body_values; L_i++) {
    FREE_TABLE(m_header_body_value_table[L_i].m_id_value_setted);
    FREE_TABLE(m_header_body_value_table[L_i].m_value_setted);
    FREE_TABLE(m_header_body_value_table[L_i].m_values);
    FREE_TABLE(m_header_body_value_table[L_i].m_size);
  }
  FREE_TABLE(m_header_body_value_table);


  for (L_i = 0; L_i < (int)m_nb_header_values; L_i++) {
    FREE_TABLE(m_header_value_table[L_i].m_id_value_setted);
    FREE_TABLE(m_header_value_table[L_i].m_value_setted);
    FREE_TABLE(m_header_value_table[L_i].m_values);
  }
  FREE_TABLE(m_header_value_table);

  for (L_i = 0; L_i < (int)m_nb_header_values; L_i++) {
    FREE_TABLE(m_body_value_table[L_i].m_value_table);
  }
  FREE_TABLE(m_body_value_table);

  if (m_header_value_id_map != NULL) {
    if (!m_header_value_id_map->empty()) {
      m_header_value_id_map
	->erase(m_header_value_id_map->begin(), 
		m_header_value_id_map->end());
    }
  }
  DELETE_VAR(m_header_value_id_map);
  if (m_header_decode_map != NULL) {
    if (!m_header_decode_map->empty()) {
      m_header_decode_map->erase(m_header_decode_map->begin(), 
				 m_header_decode_map->end());
    }
  }
  DELETE_VAR(m_header_decode_map);

  if (m_header_body_value_id_map != NULL) {
    if (!m_header_body_value_id_map->empty()) {
      m_header_body_value_id_map
	->erase(m_header_body_value_id_map->begin(), 
		m_header_body_value_id_map->end());
    }
  }
  DELETE_VAR(m_header_body_value_id_map);
  if (m_header_body_decode_map != NULL) {
    if (!m_header_body_decode_map->empty()) {
      m_header_body_decode_map->erase(m_header_body_decode_map->begin(), 
				 m_header_body_decode_map->end());
    }
  }
  DELETE_VAR(m_header_body_decode_map);


  DELETE_VAR(m_stats);
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

  m_header_body_field_separator = NULL ;
  m_session_id_position = -1 ;
  m_transport_type = 0 ;

  GEN_DEBUG(1, "C_ProtocolBinary::~C_ProtocolBinary() end");

}

bool C_ProtocolBinary::get_header_length_excluded () {
  return(m_header_length_excluded);
}

unsigned long C_ProtocolBinary::get_msg_length_start () {
  return(m_msg_length_start);
}

bool C_ProtocolBinary::get_msg_length_start_detected () {
  return(m_msg_length_start_detected);
}

int C_ProtocolBinary::get_types_from_xml (C_XmlData *P_def) {

  C_XmlData                *L_data ;
  T_XmlData_List::iterator  L_listIt ;
  T_pXmlData_List           L_subList ;
  char                     *L_value, 
    *L_type_name,
    *L_type_type,
    *L_type_size, 
    *L_type_unit ;
  int                       L_ret = 0 ;
  unsigned long             L_size    = 0;
  T_TypeType                L_type    = E_UNSUPPORTED_TYPE ;
  int                       L_i ;

  int                       L_id      ;
  
  GEN_DEBUG(1, "C_ProtocolBinary::get_types_from_xml() start");
  L_subList = P_def->get_sub_data() ;

  for(L_listIt  = L_subList->begin() ;
      L_listIt != L_subList->end() ;
      L_listIt++) {

    L_data = *L_listIt ;
    L_value = L_data->get_name() ;

    if (strcmp(L_value, (char*)"typedef") == 0) {
      char *L_endstr = NULL ;

      L_type_name = L_data->find_value((char*)"name") ;
      if (L_type_name == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "typedef name value is mandatory");
	L_ret = -1 ;
	break ;
      }
      GEN_DEBUG(1, "C_ProtocolBinary::get_types_from_xml() typedef name is " << L_type_name);

      L_type_type = L_data->find_value((char*)"type") ;
      if (L_type_type == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "typedef type value is mandatory for " << L_type_name);
	L_ret = -1 ;
	break ;
      }
      GEN_DEBUG(1, "C_ProtocolBinary::get_types_from_xml() typedef type is " << L_type_type);

      L_type = E_UNSUPPORTED_TYPE ;
      for(L_i=0; L_i < (int) E_UNSUPPORTED_TYPE; L_i++) {
	if (strcmp(L_type_type, type_type_table[L_i])==0) {
	  L_type = (T_TypeType) L_i ;
	}
      }
      if (L_type == E_UNSUPPORTED_TYPE) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "typedef type value ["
	      << L_type_type << "] unsupported for " << L_type_name);
	L_ret = -1 ;
	break ;
      }
      

      switch (L_type) {
      case E_TYPE_NUMBER:
      case E_TYPE_SIGNED:

	L_type_size = L_data->find_value((char*)"size") ;
	if (L_type_size == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value is mandatory for "
                      << L_type_name);
	  L_ret = -1 ;
	  break ;
	}
	L_size = strtoul_f (L_type_size, &L_endstr,10) ;
	if (L_endstr[0] != '\0') {
	  L_size = strtoul_f (L_type_size, &L_endstr,16) ;
	  if (L_endstr[0] != '\0') {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
		      << L_type_size << "] bad format for "
                      << L_type_name);
	    L_ret = -1 ;
	    break ;
	  }
	}


        if ( L_size > sizeof(T_UnsignedInteger32)) {
	   GEN_ERROR(E_GEN_FATAL_ERROR, "typedef max size value ["  
                     << L_size << "] for " << L_type_name << " instead of ["
                     << sizeof(T_UnsignedInteger32) << "]" );
	   L_ret = -1 ;
	   break ;
        }


	L_type_unit = L_data->find_value((char*)"unit") ;
	if (L_type_unit == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "typedef unit value is mandatory for "
                    << L_type_name);
	  L_ret = -1 ;
	  break ;
	}

	// temporary
	if (strcmp(L_type_unit, (char*)"octet") != 0) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported unit ["
		<< L_type_unit << "] value for "
                << L_type_name);
	  L_ret = -1 ;
	  break;
	}
	break ;

      case E_TYPE_STRUCT:
	L_size = 8;
	break ;

      case E_TYPE_STRING:

        L_type_size = L_data->find_value((char*)"size") ;
        if (L_type_size == NULL) {
          // Has before
          L_size = 0;
        } else {
          L_size = strtoul_f (L_type_size, &L_endstr,10) ;
          if (L_endstr[0] != '\0') {
            L_size = strtoul_f (L_type_size, &L_endstr,16) ;
            if (L_endstr[0] != '\0') {
              GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
                        << L_type_size << "] bad format for "
                        << L_type_name);
              L_ret = -1 ;
              break ;
            }
          }
        }

        L_type_unit = L_data->find_value((char*)"unit") ;
        if (L_type_unit == NULL) {
          GEN_ERROR(E_GEN_FATAL_ERROR, "typedef unit value is mandatory for "
                    << L_type_name);
          L_ret = -1 ;
          break ;
        }
        // temporary
        if (strcmp(L_type_unit, (char*)"octet") != 0) {
          GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported unit ["
                << L_type_unit << "] value for " << L_type_name);
          L_ret = -1 ;
          break;
        }
        break ;

      case E_TYPE_NUMBER_64:
      case E_TYPE_SIGNED_64:

	L_type_size = L_data->find_value((char*)"size") ;
	if (L_type_size == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value is mandatory for "
                    << L_type_name);
	  L_ret = -1 ;
	  break ;
	}
	L_size = strtoull_f (L_type_size, &L_endstr,10) ;
	if (L_endstr[0] != '\0') {
	  L_size = strtoull_f (L_type_size, &L_endstr,16) ;
	  if (L_endstr[0] != '\0') {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
		      << L_type_size << "] bad format for "
                      << L_type_name);
	    L_ret = -1 ;
	    break ;
	  }
	}

        if ( L_size > sizeof(T_UnsignedInteger64)) {
	   GEN_ERROR(E_GEN_FATAL_ERROR, "typedef max size value ["  
                     << L_size << "] for " << L_type_name << " instead of ["
                     << sizeof(T_UnsignedInteger64) << "]" );
	   L_ret = -1 ;
	   break ;
        }

	L_type_unit = L_data->find_value((char*)"unit") ;
	if (L_type_unit == NULL) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "typedef unit value is mandatory for "
                    << L_type_name);
	  L_ret = -1 ;
	  break ;
	}

	// temporary
	if (strcmp(L_type_unit, (char*)"octet") != 0) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported unit ["
		<< L_type_unit << "] value for " << L_type_name);
	  L_ret = -1 ;
	  break;
	}
	break ;

      default :
	L_size = 0 ;
	break ;
      }


      if (L_ret != -1) {

	L_id = add_type(L_type_name, L_type, L_size);
	m_type_id_map->insert(T_IdMap::value_type(L_type_name, L_id));
	
      }
    } // if (strcmp(L_value, (char*)"typedef") == 0)
  } // for

  GEN_DEBUG(1, "C_ProtocolBinary::get_types_from_xml() end");
  return (L_ret);
}

int C_ProtocolBinary::get_header_body_from_xml (C_XmlData *P_def) {

  C_XmlData                *L_data ;
  T_XmlData_List::iterator  L_listIt ;
  T_pXmlData_List           L_subList ;
  char                     *L_value, *L_value_size, *L_value_unit ;
  char                     *L_value_field_length, *L_value_field_type ;
  int                       L_ret = 0 ;
  unsigned long             L_size    ;
  int                       L_id      ;

  
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_from_xml() start");

  m_header_body_name = P_def->find_value((char*)"name");
  if (m_header_body_name == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,"header name value is mandatory");
    L_ret = -1 ;
  }

  L_value_field_type = NULL ;
  L_value_field_length = NULL ;
  m_header_body_field_separator = P_def->find_value((char*)"field-separator");
  if (m_header_body_field_separator == NULL) { // no separator defined 
                                               // => type and length mandatory
    
    L_value_field_type   = P_def->find_value((char*)"type");
    if (L_value_field_type == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR,"type definition value is mandatory");
      L_ret = -1 ;
    }
    L_value_field_length = P_def->find_value((char*)"length");
    if (L_value_field_length == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR,"length definition value is mandatory");
      L_ret = -1 ;
    }
  } 

  L_subList = P_def->get_sub_data() ;

  for(L_listIt  = L_subList->begin() ;
      L_listIt != L_subList->end() ;
      L_listIt++) {

    L_data = *L_listIt ;
    L_value = L_data->get_name() ;

    if (strcmp(L_value, (char*)"fielddef") == 0) {

      char *L_endstr ;

      L_value = L_data->find_value((char*)"name") ;
      if (L_value == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef name value is mandatory");
	L_ret = -1 ;
	break ;
      }

      L_value_size = L_data->find_value((char*)"size") ;
      if (L_value_size == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "fielddef size value is mandatory ["
	      << L_value << "]");
	L_ret = -1 ;
	break ;
      }

      L_size = strtoul_f (L_value_size, &L_endstr,10) ;
      if (L_endstr[0] != '\0') {
         L_size = strtoul_f (L_value_size, &L_endstr,16) ;
	 if (L_endstr[0] != '\0') {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef size value ["
		      << L_value_size << "] bad format");
	    L_ret = -1 ;
	    break ;
	 }
      }

      L_value_unit = L_data->find_value((char*)"unit") ;
      if (L_value_unit == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef unit value is mandatory");
	L_ret = -1 ;
	break ;
      }

      if ( L_size > sizeof(unsigned long)) {
	   GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef max size value ["  << sizeof(unsigned long) << "]");
	   L_ret = -1 ;
	   break ;
      }

      // temporary
      if (strcmp(L_value_unit, (char*)"octet") != 0) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported unit ["
	      << L_value_unit << "] value");
	L_ret = -1 ;
	break;
      }

      if (L_ret != -1) {

 	L_id = add_header_field (L_value, 
				 L_size,
				 &m_nb_field_header_body,
				 &m_header_body_size,
				 m_header_body_field_table,
				 m_header_body_id_map) ;

	if (L_value_field_type) {
	  if (strcmp(L_value, L_value_field_type) == 0) {
	    set_header_body_type_id(L_id) ;
	  }
	}
	if (L_value_field_length) {
	  if (strcmp(L_value, L_value_field_length) == 0) {
	    set_header_body_length_id(L_id) ;
	  }
	}

      }
    } else if ( strcmp(L_value, (char *)"optional") == 0 ) {

      // option field section
      if (L_data -> get_sub_data() != NULL) {

	L_ret = get_header_body_optional_from_xml(L_data);
	if (L_ret == -1) break ;

      }


    } else {

      GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported section ["
	    << L_value << "]");
      L_ret = -1 ;
      break;
      
    }
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_from_xml() end");

  return (L_ret);
}


int C_ProtocolBinary::get_header_body_optional_from_xml (C_XmlData *P_def) {

  C_XmlData                *L_data ;
  T_XmlData_List::iterator  L_listIt ;
  T_pXmlData_List           L_subList ;
  char                     *L_value, *L_value_size, *L_value_unit ;
  char                     *L_value_cond, *L_value_field, *L_value_mask ;
  int                       L_ret = 0 ;
  unsigned long             L_size    ;
  unsigned long             L_mask ;
  int                       L_field_id ;
  T_pCondPresence           L_condPresence = NULL ;
  
  
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_optional_from_xml() start");

  L_subList = P_def->get_sub_data() ;

  for(L_listIt  = L_subList->begin() ;
      L_listIt != L_subList->end() ;
      L_listIt++) {

    L_data = *L_listIt ;
    L_value = L_data->get_name() ;

    if (strcmp(L_value, (char*)"fielddef") == 0) {

      char *L_endstr ;

      L_value = L_data->find_value((char*)"name") ;
      if (L_value == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef name value is mandatory");
	L_ret = -1 ;
	break ;
      }

      L_value_size = L_data->find_value((char*)"size") ;
      if (L_value_size == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "fielddef size value is mandatory ["
	      << L_value << "]");
	L_ret = -1 ;
	break ;
      }

      L_size = strtoul_f (L_value_size, &L_endstr,10) ;
      if (L_endstr[0] != '\0') {
         L_size = strtoul_f (L_value_size, &L_endstr,16) ;
	 if (L_endstr[0] != '\0') {
	    GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef size value ["
		      << L_value_size << "] bad format");
	    L_ret = -1 ;
	    break ;
	 }
      }
      if ( L_size > sizeof(unsigned long)) {
	   GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef max size value ["  << sizeof(unsigned long) << "]");
	   L_ret = -1 ;
	   break ;
      }
      L_value_unit = L_data->find_value((char*)"unit") ;
      if (L_value_unit == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef unit value is mandatory");
	L_ret = -1 ;
	break ;
      }

      // temporary
      if (strcmp(L_value_unit, (char*)"octet") != 0) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported unit ["
	      << L_value_unit << "] value");
	L_ret = -1 ;
	break;
      }

      L_value_cond = L_data->find_value ((char*)"condition") ;
      if (L_value_cond == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef condition value is mandatory");
	L_ret = -1 ;
	break ;
      }
      if (strcmp(L_value_cond, (char*)"mask") != 0) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported fielddef condition value ["
	      << L_value_cond << "]");
	L_ret = -1 ;
	break ;
      }
      L_value_field = L_data->find_value ((char*)"field") ;
      if (L_value_field == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef condition value is mandatory");
	L_ret = -1 ;
	break ;
      } else {
	// find id of the field 
	T_IdMap::iterator L_bodyFieldIt ;
	L_bodyFieldIt = 
	  m_header_body_id_map->find(T_IdMap::key_type(L_value_field));
	if (L_bodyFieldIt != m_header_body_id_map->end()) {
	  L_field_id = L_bodyFieldIt->second ;
	} else {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "no definition found for field ["
		<< L_value_field << "]");
	  L_ret = -1 ;
	  break ;
	}
      }
      L_value_mask = L_data->find_value (L_value_cond) ;
      if (L_value_mask == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef " 
	      << L_value_cond << " value is mandatory");
	L_ret = -1 ;
	break ;
      } else {
	L_mask = strtoul_f (L_value_mask, &L_endstr,10) ;
	if (L_endstr[0] != '\0') {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "bad mask format for ["
		<< L_value_mask << "]");
	  L_ret = -1 ;
	  break ;
	}
      }

      if (L_ret != -1) {

	ALLOC_VAR(L_condPresence, T_pCondPresence, sizeof(T_CondPresence)) ;
	L_condPresence->m_mask = L_mask ;
	L_condPresence->m_f_id = L_field_id ;
 	
 	(void) add_header_field (L_value, 
				 L_size,
				 &m_nb_field_header_body,
				 &m_header_body_size,
				 m_header_body_field_table,
				 m_header_body_id_map,
				 L_condPresence) ;

      }

    } else {

      GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported section ["
	    << L_value << "]");
      L_ret = -1 ;
      break;
      
    }
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_optional_from_xml() end");

  return (L_ret);
}



void C_ProtocolBinary::set_header_body_type_id (int P_id) {
  GEN_DEBUG(1, "C_ProtocolBinary::set_header_body_type_id() start");
  GEN_DEBUG(1, "C_ProtocolBinary::set_header_body_type_id() end");
  m_header_body_type_id = P_id ;
}
void C_ProtocolBinary::set_header_body_length_id (int P_id) {

  int L_id ;

  GEN_DEBUG(1, "C_ProtocolBinary::set_header_body_length_id() start");
  m_header_body_length_id = P_id ;
  m_header_body_length_index = 0 ;
  for(L_id = 0; L_id < m_header_body_length_id; L_id++) {
    m_header_body_length_index 
      += (int) m_header_body_field_table[L_id].m_size ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::set_header_body_length_id() end");

}

unsigned long C_ProtocolBinary::get_header_body_size() {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_size() start");
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_size() end");
  return (m_header_body_size);
}

unsigned long C_ProtocolBinary::get_nb_field_header_body () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_nb_field_header_body() start");
  GEN_DEBUG(1, "C_ProtocolBinary::get_nb_field_header_body() end");
  return (m_nb_field_header_body) ;
}

C_ProtocolBinary::T_pHeaderField 
C_ProtocolBinary::get_header_body_field_description (int P_id) {

  T_pHeaderField L_result ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_field_description() start");
  L_result = (P_id < (int)m_max_nb_field_header_body) ?
    &(m_header_body_field_table[P_id]) : NULL ;
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_field_description() end");

  return (L_result);
}

C_ProtocolBinary::T_pHeaderField 
C_ProtocolBinary::get_header_body_field_description (char *P_name) {

  T_pHeaderField         L_result ;
  int                    L_id     ;
  T_IdMap::iterator L_it     ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_field_description() start");
  L_it = m_header_body_id_map 
    -> find (T_IdMap::key_type(P_name)) ;
  
  if (L_it != m_header_body_id_map->end()) {
    L_id = L_it->second ;
    L_result = get_header_body_field_description(L_id);
  } else {
    L_result = NULL ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_field_description() end");

  return (L_result) ;
}

int C_ProtocolBinary::get_header_body_type_id () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_type_id() start");
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_type_id() end");
  return (m_header_body_type_id) ;
}

int C_ProtocolBinary::get_header_body_length_id () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_length_id() start");
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_length_id() end");
  return (m_header_body_length_id) ;
}

int C_ProtocolBinary::get_header_body_length_index () {
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_length_index() start");
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_length_index() end");
  return (m_header_body_length_index) ;
}

int C_ProtocolBinary::get_header_values_from_xml (C_XmlData *P_def) {

  int                       L_ret = 0 ;
  C_XmlData                *L_data    ;
  char                     *L_value, *L_name, *L_endstr ;
  char                     *L_fieldName, *L_fieldValue ;
  T_XmlData_List::iterator  L_listIt, L_listFieldIt  ;
  T_pXmlData_List           L_subListDefine, L_subListSetField ;
  int                       L_id ;

  T_IdMap::iterator         L_IdMapIt   ;
  int                       L_fieldId, L_fieldIdx  ;
  T_UnsignedInteger32       L_fieldValueUl;
  // unsigned long             L_fieldValueUl, L_fieldCode;
  unsigned long             L_fieldCode = 0 ; 
  bool                      L_codeFound ;
  unsigned long             L_i ;

  unsigned long             L_nb_setfield, L_nb_bodyval ;
  char                     *L_body_name, *L_body_value ;
  int                       L_valueIdx, L_valueId ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() start");

  L_id = 0 ;
  L_subListDefine = P_def->get_sub_data() ;

  for(L_listIt  = L_subListDefine->begin() ;
      L_listIt != L_subListDefine->end() ;
      L_listIt++) {

    L_data = *L_listIt ;
    L_value = L_data->get_name() ;

    GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  " 
	      << "L_data->get_name() " << 
	      L_data->get_name() );

    if (strcmp(L_value, (char*)"define") == 0) {

      m_header_value_table[L_id].m_id = L_id ; 

      L_name = L_data->find_value((char*)"name") ;
      if (L_name == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "define name value is mandatory");
	L_ret = -1 ;
	break ;
      }
      m_header_value_table[L_id].m_name = L_name ;

      if (L_ret != -1) {

 	L_subListSetField = L_data->get_sub_data() ;

	if (L_subListSetField != NULL) {
          GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  " 
		    << "L_subListSetField " << 
		    L_subListSetField );


	  // setfield and header_body_name section 
	  L_nb_setfield = 0 ;
	  L_nb_bodyval = 0 ;

	  for (L_listFieldIt = L_subListSetField->begin();
	       L_listFieldIt != L_subListSetField->end();
	       L_listFieldIt++) {
	    L_data = *L_listFieldIt ;
            
	    if (strcmp(L_data->get_name(), (char*)"setfield") == 0) {
	      L_nb_setfield++ ;
	    } else if (strcmp(L_data->get_name(), (char*)m_header_body_name) == 0) {
	      L_nb_bodyval++;
            } else {
              GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown ["
                        << L_data->get_name() << "] section.[setfield] or ["
                        << m_header_body_name << "] section is mandatory");
              L_ret = -1 ;
              break ;
	    }
	  }

	  if (L_ret == -1) break ;
	  
	  m_header_value_table[L_id].m_nb_set 
	    = L_nb_setfield ;
	  

          GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  " 
		    << "m_header_value_table [" << 
		    L_id << "].m_nb_set " 
		    << m_header_value_table[L_id].m_nb_set );

	  ALLOC_TABLE(m_header_value_table[L_id].m_values,
		      T_pValueData,
		      sizeof(T_ValueData),
		      m_header_value_table[L_id].m_nb_set);

	  ALLOC_TABLE(m_header_value_table[L_id].m_value_setted,
		      bool*,
		      sizeof(bool),
		      m_nb_field_header);

	  ALLOC_TABLE(m_header_value_table[L_id].m_id_value_setted,
		      int*,
		      sizeof(int),
		      m_nb_field_header);

	  for(L_i = 0; L_i < m_nb_field_header; L_i++) {
	    m_header_value_table[L_id].m_value_setted[L_i] = false ;
	    m_header_value_table[L_id].m_id_value_setted[L_i] = -1 ;
	  }

	  L_fieldIdx = 0 ;
	  L_codeFound = false ;

	  m_body_value_table[L_id].m_nb_values 
	    = L_nb_bodyval ;

	  ALLOC_TABLE(m_body_value_table[L_id].m_value_table,
		      T_pBodyValue,
		      sizeof(T_BodyValue),
		      L_nb_bodyval);
	  L_valueIdx = 0 ;

          GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  " 
		    << "m_body_value_table [ " << 
		    L_id << "].m_nb_values " 
		    << m_body_value_table[L_id].m_nb_values);

		      
	  for (L_listFieldIt = L_subListSetField->begin();
	       L_listFieldIt != L_subListSetField->end();
	       L_listFieldIt++) {
	    L_data = *L_listFieldIt ;

            GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  " << 
		      "L_data->get_name() is " << 
		      L_data->get_name() );

	    if (strcmp(L_data->get_name(), (char*)"setfield") == 0) {
	      L_fieldName = L_data->find_value((char*)"name") ;
	      if (L_fieldName == NULL) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "setfield name value is mandatory");
		L_ret = -1 ;
		break ;
	      }

	      L_IdMapIt = 
		m_header_id_map->find(T_IdMap::key_type(L_fieldName));

	      if (L_IdMapIt != m_header_id_map->end()) {
		L_fieldId = L_IdMapIt->second ;
                GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  L_fieldName is " << 
		              L_fieldName );
	      } else {
		GEN_ERROR(E_GEN_FATAL_ERROR, 
		      "Field ["
		      << L_fieldName << "] not defined");
		L_ret = -1 ;
		break;
	      }

	      L_fieldValue = L_data->find_value((char*)"value") ;
	      if (L_fieldValue == NULL) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "setfield value is mandatory");
		L_ret = -1 ;
		break ;
	      }
              GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() L_fieldValue is " << 
		            L_fieldValue );


	      L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,10) ;


	      if (L_endstr[0] != '\0') {
		L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,16) ;
		  GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() typedef size value ["
			    << L_fieldValueUl << "] format");
		if (L_endstr[0] != '\0') {
		  GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
			    << L_fieldValue << "] bad format");
		  L_ret = -1 ;
		  break ;
		}
	      }

	      if (L_ret != -1) {
		       
                  GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  "
			    << "L_fieldValueUl is " << 
			    L_fieldValueUl );

		  GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() " 
			    << "L_fieldId is " << 
			    L_fieldId << " and m_header_type_id " 
			    << m_header_type_id) ;

		if (L_fieldId == m_header_type_id) {
		  L_fieldCode = L_fieldValueUl ;
		  L_codeFound = true ;
		}

		GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() " 
			  << "L_id = " << L_id );
		GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() " 
			  << "L_fieldId =  " << L_fieldId );
		GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() " 
			  << "L_fieldIdx = " << L_fieldIdx );

		m_header_value_table[L_id]
		  .m_value_setted[L_fieldId] = true ;
		m_header_value_table[L_id]
		  .m_id_value_setted[L_fieldId] = L_fieldIdx ;

		(m_header_value_table[L_id].m_values)[L_fieldIdx].m_id
		  = L_fieldId ;
		(m_header_value_table[L_id].m_values)[L_fieldIdx].m_type
		  = E_TYPE_NUMBER ;
		(m_header_value_table[L_id].m_values)[L_fieldIdx]
		  .m_value.m_val_number
		  = L_fieldValueUl ;
		L_fieldIdx++;
	      }

	    } else if (strcmp(L_data->get_name(), (char*)m_header_body_name) == 0) {

	      L_body_name = L_data->find_value((char*)"name") ;
	      if (L_body_name == NULL) {
		GEN_ERROR(E_GEN_FATAL_ERROR, 
			  "the name of [" << m_header_body_name <<
			  "] is mandatory");
		L_ret = -1 ;
		break ;
	      }
	      GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  "
			<< "L_body_name [" << L_body_name << "]");

	      L_valueId 
		= get_header_body_value_id(L_body_name);
	      if (L_valueId == -1) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "No definition found for ["
			  << L_body_name << "]");
		L_ret = -1 ;
		break ;
	      } 
	      if (get_body_value_type (L_valueId) 
		  == E_TYPE_GROUPED) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "Grouped type not supported for body value");
		L_ret = -1 ;
		break ;
	      }

	      L_body_value = L_data->find_value((char*)"value") ;
	      if (L_body_value == NULL) {
		GEN_ERROR(E_GEN_FATAL_ERROR, 
			  "the value of [" << m_header_body_name <<
			  "] is mandatory");
		L_ret = -1 ;
		break ;
	      }
	      GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml()  "
			<< "L_body_value [" << L_body_value << "]");

	      if (set_body_value(L_valueId,
				 L_body_value,
				 1,
				 &(m_body_value_table[L_id].m_value_table[L_valueIdx])) == 0) {

		if ((m_header_type_id == -1) && (L_valueId == m_header_type_id_body)) {
                  if (m_header_body_field_separator == NULL ) {
                    L_fieldCode = 
                      m_body_value_table[L_id].m_value_table[L_valueIdx].m_value.m_val_number ;
                  } else {
                    L_fieldCode = 
                      convert_char_to_ul((char*)m_body_value_table[L_id].m_value_table[L_valueIdx].m_value.m_val_binary.m_value);
                  }
		  L_codeFound = true ;

		}


	      } else {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "Bad format for ["
			  << L_body_value << "]");
		L_ret = -1 ;
		break ;
	      }
	      L_valueIdx ++ ;
	    }
	    if (L_ret == -1) break ;
	  } // for
	  if (L_ret == -1) break ;
	} else {
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		"setfield for ["
		    << m_header_name << "] or [" << m_header_body_name << "] code is mandatory");
	  L_ret = -1 ;
	  break ;
	}
	if (L_ret == -1) break ;
	if ((L_codeFound == false) && (m_header_type_id != -1) ) {
	  GEN_ERROR(E_GEN_FATAL_ERROR,
		"No value found for the field ["
		<< m_header_field_table[m_header_type_id].m_name
		<< "]");
	  L_ret = -1 ;
	  break ;
	}
	GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() L_name = " << 
                      L_name << " L_fieldCode = " << L_fieldCode << 
	              " and L_id = " << L_id) ;

	m_header_value_id_map
	  ->insert(T_IdMap::value_type(L_name, L_id));
	m_header_decode_map
	  ->insert(T_DecodeMap::value_type(L_fieldCode, L_id));
	L_id ++ ;
      }
    }

    if (L_ret == -1) break ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() end");

  return (L_ret);
}

int C_ProtocolBinary::get_header_body_values_from_xml (C_XmlData *P_def) {

  int                       L_ret = 0 ;
  C_XmlData                *L_data    ;
  char                     *L_value, *L_name, *L_type, *L_endstr ;
  char                     *L_fieldName, *L_fieldValue ;
  T_XmlData_List::iterator  L_listIt, L_listFieldIt  ;
  T_pXmlData_List           L_subListDefine, L_subListSetField ;
  int                       L_id ;

  T_IdMap::iterator         L_IdMapIt   ;
  int                       L_typeId ;
  int                       L_fieldId, L_fieldIdx  ;
  unsigned long             L_fieldValueUl, L_fieldCode;
  bool                      L_codeFound ;

  int                       L_i ;

  int                       L_nb_setfield = 0 ;

  int                       L_nb_setsize  = 0 ;
  int                       L_nb_notpresent  = 0 ;
  bool                      L_setsizeFound = false ;


  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() start");


  ALLOC_TABLE(m_header_body_value_table,
              T_pHeaderBodyValue,
              sizeof(T_HeaderBodyValue),
              m_nb_header_body_values);
  
  L_id = 0 ;
  L_subListDefine = P_def->get_sub_data() ;

  for(L_listIt  = L_subListDefine->begin() ;
      L_listIt != L_subListDefine->end() ;
      L_listIt++) {

    L_data = *L_listIt ;
    L_value = L_data->get_name() ;

    if (strcmp(L_value, (char*)"define") == 0) {

      L_name = L_data->find_value((char*)"name") ;
      if (L_name == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "define name value is mandatory");
	L_ret = -1 ;
	break ;
      } 

      L_type = L_data->find_value((char*)"type") ;
      if (L_type == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "define type value is mandatory");
	L_ret = -1 ;
	break ;
      }
      L_IdMapIt = 
	m_type_id_map->find(T_IdMap::key_type(L_type));
      
      if (L_IdMapIt != m_type_id_map->end()) {
	L_typeId = L_IdMapIt->second ;
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Type ["
	      << L_type << "] not defined");
	L_ret = -1 ;
	break;
      }


      m_header_body_value_table[L_id].m_id = L_id ;
      m_header_body_value_table[L_id].m_name = L_name ;
      m_header_body_value_table[L_id].m_type_id = L_typeId ;

      if ((L_ret != -1) && (get_header_type_id() == -1)) {
        if (strcmp(L_name, m_header_type_name) == 0) {
          m_header_type_id_body = L_id ;
        }
      } 

      if ((L_ret != -1) && (m_header_body_field_separator == NULL)) {

        GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() " <<
		  "L_id = " << L_id);
        GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() " << 
		  "L_name = " << L_name);
        GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() " << 
		  "m_header_type_name = " << 
		  m_header_type_name);


  	L_subListSetField = L_data->get_sub_data() ;

	if (L_subListSetField != NULL) {
	  // setfield and header_body_name section 

	  for (L_listFieldIt = L_subListSetField->begin();
	       L_listFieldIt != L_subListSetField->end();
	       L_listFieldIt++) {
	    L_data = *L_listFieldIt ;
            
	    if (strcmp(L_data->get_name(), (char*)"setfield") == 0) {
	      L_nb_setfield++ ;
	    } else if (strcmp(L_data->get_name(), (char*)"setsize") == 0) {
	      L_nb_setsize++;
            } else if (strcmp(L_data->get_name(), (char*)"not-present") == 0) {
              L_nb_notpresent++;   
            }
	  }
        }


        if (L_subListSetField != NULL) {
	  m_header_body_value_table[L_id].m_nb_set 
	    = L_nb_setfield ; 
        } else {
          m_header_body_value_table[L_id].m_nb_set=0;
        }

	if (m_header_body_value_table[L_id].m_nb_set != 0) {

          ALLOC_TABLE(m_header_body_value_table[L_id].m_values,
                      T_pValueData,
                      sizeof(T_ValueData),
                      m_header_body_value_table[L_id].m_nb_set);
          
          ALLOC_TABLE(m_header_body_value_table[L_id].m_value_setted,
                      bool*,
                      sizeof(bool),
                      m_max_nb_field_header_body);
          
          ALLOC_TABLE(m_header_body_value_table[L_id].m_id_value_setted,
                      int*,
                      sizeof(int),
                      m_max_nb_field_header_body);

          ALLOC_TABLE(m_header_body_value_table[L_id].m_size,
                      unsigned long *,
                      sizeof(unsigned long),
                      m_max_nb_field_header_body);

          ALLOC_TABLE(m_header_body_value_table[L_id].m_present,
                      bool*,
                      sizeof(bool),
                      m_max_nb_field_header_body);
          
          
          
          for(L_i = 0; L_i < (int)m_max_nb_field_header_body; L_i++) {
            m_header_body_value_table[L_id].m_value_setted[L_i] = false ;
            m_header_body_value_table[L_id].m_id_value_setted[L_i] = -1 ;
            m_header_body_value_table[L_id].m_size[L_i] =  
              m_header_body_field_table[L_i].m_size ;
            m_header_body_value_table[L_id].m_present[L_i] =  true ;
          }
          

          L_fieldIdx = 0 ;
          L_codeFound = false ;
          L_setsizeFound = false ;

          
          for (L_listFieldIt = L_subListSetField->begin();
               L_listFieldIt != L_subListSetField->end();
               L_listFieldIt++) {
            
            L_data = *L_listFieldIt ;

            if (strcmp(L_data->get_name(), (char*)"setsize") == 0) {
              
              L_fieldName = L_data->find_value((char*)"name") ;
              if (L_fieldName == NULL) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "setsize name value is mandatory");
                L_ret = -1 ;
                break ;
              }
              
              L_IdMapIt = 
                m_header_body_id_map->find(T_IdMap::key_type(L_fieldName));
              
              if (L_IdMapIt != m_header_id_map->end()) {
                L_fieldId = L_IdMapIt->second ;
              } else {
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "Field ["
                          << L_fieldName << "] not defined");
                L_ret = -1 ;
                break;
              }
              
              L_fieldValue = L_data->find_value((char*)"value") ;
              if (L_fieldValue == NULL) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "setsize value is mandatory");
                L_ret = -1 ;
                break ;
              }
              L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,10) ;
              if (L_endstr[0] != '\0') {
                L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,16) ;
                if (L_endstr[0] != '\0') {
                  GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
                            << L_fieldValue << "] bad format");
                  L_ret = -1 ;
                  break ;
                }
              }
              if (L_ret == -1) break ;
              m_header_body_value_table[L_id].m_size[L_fieldId] =  L_fieldValueUl ;
              if (L_fieldId < m_header_body_type_id) {
                
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "setsize for ["
                          << m_header_body_name 
                          << "] is not possible."
                          << "setsize defined after type"); 
                L_ret = -1 ;
                break; 
                     
              } 
              if (L_fieldId == m_header_body_length_id) {
                L_setsizeFound = true;
              }

            } // if (strcmp(L_data->get_name(), (char*)"setsize") == 0)  

            if (L_ret == -1) break ;

            if (strcmp(L_data->get_name(), (char*)"not-present") == 0) {
              L_fieldName = L_data->find_value((char*)"name") ;
              if (L_fieldName == NULL) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "not-present name value is mandatory");
                L_ret = -1 ;
                break ;
              }
              
              L_IdMapIt = 
                m_header_body_id_map->find(T_IdMap::key_type(L_fieldName));
              
              if (L_IdMapIt != m_header_id_map->end()) {
                L_fieldId = L_IdMapIt->second ;
              } else {
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "Field ["
                          << L_fieldName << "] not defined");
                L_ret = -1 ;
                break;
              }
              
              if (L_ret == -1) break ;
              m_header_body_value_table[L_id].m_present[L_fieldId] = false ;
            } // if (strcmp(L_data->get_name(), (char*)"not-present") == 0)  

            if (strcmp(L_data->get_name(), (char*)"setfield") == 0) {
              
              L_fieldName = L_data->find_value((char*)"name") ;
              if (L_fieldName == NULL) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "setfield name value is mandatory");
                L_ret = -1 ;
                break ;
              }
              GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() " 
                        << "L_fieldName " << 
                        L_fieldName );
              
              
              L_IdMapIt = 
                m_header_body_id_map->find(T_IdMap::key_type(L_fieldName));
              
              if (L_IdMapIt != m_header_id_map->end()) {
                L_fieldId = L_IdMapIt->second ;
              } else {
                GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "Field ["
                          << L_fieldName << "] not defined");
                L_ret = -1 ;
                break;
              }
              
              L_fieldValue = L_data->find_value((char*)"value") ;
              if (L_fieldValue == NULL) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "setfield value is mandatory");
                L_ret = -1 ;
                break ;
              }
              L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,10) ;
              if (L_endstr[0] != '\0') {
                L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,16) ;
                if (L_endstr[0] != '\0') {
                  GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
                            << L_fieldValue << "] bad format");
                  L_ret = -1 ;
                  break ;
                }
              }
              
              if (L_ret != -1) {
                GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() L_fieldId = " << 
                          L_fieldId << " m_header_body_type_id = " << 
                          m_header_body_type_id << " and L_fieldValueUl = " << 
                          L_fieldValueUl) ;
                
                if (L_fieldId == m_header_body_type_id) {
                  L_fieldCode = L_fieldValueUl ;
                  L_codeFound = true ;
                }

                m_header_body_value_table[L_id]
                  .m_value_setted[L_fieldId] = true ;
                m_header_body_value_table[L_id]
		  .m_id_value_setted[L_fieldId] = L_fieldIdx ;
                
                (m_header_body_value_table[L_id].m_values)[L_fieldIdx].m_id
                  = L_fieldId ;
                (m_header_body_value_table[L_id].m_values)[L_fieldIdx].m_type
                  = E_TYPE_NUMBER ;
                (m_header_body_value_table[L_id].m_values)[L_fieldIdx]
                  .m_value.m_val_number
                  = L_fieldValueUl ;

                L_fieldIdx++ ;
              }
            } // if (strcmp(L_data->get_name(), (char*)"setfield") == 0) 
            if (L_ret == -1) break ;
          } // for (L_listFieldIt = L_subListSetField->begin();

          if ((m_header_body_field_table[m_header_body_length_id].m_size == 0) && 
              (L_setsizeFound == false)) {
            GEN_ERROR(E_GEN_FATAL_ERROR, 
                      "setsize for ["
                      << m_header_body_name << "] code is mandatory");
            L_ret = -1 ;
            break ;
          }
        } else { // if (m_header_body_value_table[L_id].m_nb_set == 0) 
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "setfield for ["
		    << m_header_body_name << "] code is mandatory");
	  L_ret = -1 ;
	  break ;
        }
        if (L_ret == -1) break ;
        if (L_codeFound == false) {
          if (m_header_body_field_table) {
            GEN_ERROR(E_GEN_FATAL_ERROR,
                      "No value found for the field ["
                      << m_header_body_field_table[m_header_body_type_id].m_name
                      << "]");
          } else {
            GEN_ERROR(E_GEN_FATAL_ERROR,
                      "No value found in the header of body");
          }
          L_ret = -1 ;
          break ;
        }
        m_header_body_decode_map
          ->insert(T_DecodeMap::value_type(L_fieldCode, L_id));
      } // if (L_ret != -1) && (m_header_body_field_separator == NULL)
    }
    if (L_ret == -1) break ;
    m_header_body_value_id_map
      ->insert(T_IdMap::value_type(L_name, L_id));
    L_id ++ ;
  } // L_listIt
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() end");
  
  return (L_ret);
}

char* C_ProtocolBinary::get_header_name() {
  return (m_header_name);
}

char* C_ProtocolBinary::get_header_body_name () {
  return (m_header_body_name);
}

int   C_ProtocolBinary::get_header_value_id (char* P_name) {
  T_IdMap::iterator L_IdMapIt ;
  int               L_Id      ;

  L_IdMapIt = m_header_value_id_map
    ->find(T_IdMap::key_type(P_name));

  L_Id = (L_IdMapIt != m_header_value_id_map->end()) 
    ? L_IdMapIt->second : -1 ;

  return (L_Id);
}

int   C_ProtocolBinary::get_header_body_value_id (char* P_name) {
  T_IdMap::iterator L_IdMapIt ;
  int               L_Id      ;

  L_IdMapIt = m_header_body_value_id_map
    ->find(T_IdMap::key_type(P_name));

  L_Id = (L_IdMapIt != m_header_body_value_id_map->end()) 
    ? L_IdMapIt->second : -1 ;

  return (L_Id);
}

int  C_ProtocolBinary::set_body_value(int          P_id, 
				char*        P_value, 
				int          P_nb,
				T_pBodyValue P_res,
				bool        *P_del) {

  int   L_ret = 0 ;
  int   L_type_id ;
  char *L_endstr  ;

  GEN_DEBUG(1, "C_ProtocolBinary::set_body_value() start");
  GEN_DEBUG(1, " P_id = " << P_id );
  if (P_value != NULL)
  GEN_DEBUG(1, " P_value = " << P_value);
  else
  GEN_DEBUG(1, " P_value = " << "empty");
  GEN_DEBUG(1, " P_nb = " << P_nb );
  GEN_DEBUG(1, " P_res = " << P_res);

  if (P_del != NULL) {
    *P_del = false;
  }

  P_res->m_id = P_id ;
  P_res->m_sub_val = NULL ;


  // retrieve type definition of the field
  L_type_id = m_header_body_value_table[P_id].m_type_id ;

  GEN_DEBUG(1, " L_type_id = " << L_type_id);

  switch (m_type_def_table[L_type_id].m_type) {

  case E_TYPE_NUMBER :
    if ((strlen(P_value)>2) && (P_value[0] == '0') && (P_value[1] == 'x')) { // hexa buffer value
      P_res->m_value.m_val_number = strtoul_f(P_value, &L_endstr, 16) ;
    } else {
      P_res->m_value.m_val_number = strtoul_f(P_value, &L_endstr, 10) ;
    }
    if (L_endstr[0] != '\0') {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Incorrect value format for ["
	    << P_value << "] - Unsigned Integer expected");
      L_ret = -1 ;
    } 
    break;
  case E_TYPE_SIGNED : {
    if ((strlen(P_value)>2) && (P_value[0] == '0') && (P_value[1] == 'x')) { // hexa buffer value
      P_res->m_value.m_val_signed = strtol_f(P_value, &L_endstr, 16) ;
    } else {
      P_res->m_value.m_val_signed = strtol_f(P_value, &L_endstr, 10) ;
    }

    if (L_endstr[0] != '\0') {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Incorrect value format for ["
	    << P_value << "] - Integer expected");
      L_ret = -1 ;
    } 
  }
    break;

  case E_TYPE_STRING : {

    if ((strlen(P_value)>2) && (P_value[0] == '0') && (P_value[1] == 'x')) { // hexa buffer value

      char  *L_ptr = P_value+2 ;
      size_t L_res_size ;

      P_res->m_value.m_val_binary.m_value
	= convert_hexa_char_to_bin(L_ptr, &L_res_size);

      if (P_res->m_value.m_val_binary.m_value == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Bad buffer size for hexadecimal buffer ["
	      << P_value << "]");
	L_ret = -1 ;
      } else {
        GEN_DEBUG(1, " P_res->m_value.m_val_binary.m_value = " 
                  << (void*)(P_res->m_value.m_val_binary.m_value));

        if (P_del != NULL) {
        *P_del = true;
         }

        // *P_del = true;
	P_res->m_value.m_val_binary.m_size= L_res_size ;
      }
      
    } else { // direct string value

      P_res->m_value.m_val_binary.m_value=(unsigned char*)P_value;
      P_res->m_value.m_val_binary.m_size=strlen(P_value);

    }
  }
    break;

  case E_TYPE_GROUPED:
    GEN_DEBUG(1, " E_TYPE_GROUPED");
    if (P_nb == 0) {
      P_res->m_value.m_val_number = 0 ;
      P_res->m_sub_val = NULL ;
    } else {
      P_res->m_value.m_val_number = P_nb ;
      ALLOC_TABLE(P_res->m_sub_val, 
		  T_pBodyValue,
		  sizeof(T_BodyValue),
		  P_nb);
      
    }
    break ;

  case E_TYPE_STRUCT : {
   
    int   L_i = 0 ;
    int   L_size = (P_value == NULL ) ? 0 : strlen(P_value) ;
    char *L_value1, *L_value2, *L_value3 ;

    L_value1 = NULL ;
    L_value2 = NULL ;
    L_value3 = NULL ;


    while (L_i< L_size) {
       if ( P_value[L_i]== ';')  {
            P_value[L_i] = 0 ;
	    L_value3= &P_value[L_i];
	    L_value1= P_value ;
	    if (L_i+1 < L_size ) {
		   L_value2 = &P_value[L_i+1]; 
            }
       }
       L_i ++;
    }

    if ( L_value1 != NULL ) {
       if ((strlen(L_value1)>2) && (L_value1[0] == '0') && (L_value1[1] == 'x')) { // hexa buffer value
          P_res->m_value.m_val_struct.m_id_1 = strtoul_f(L_value1, &L_endstr, 16) ;
       } else {
          P_res->m_value.m_val_struct.m_id_1 = strtoul_f(L_value1, &L_endstr, 10) ;
       }
      if (L_endstr[0] != '\0') {
        GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Incorrect value format for ["
	    << L_value1 << "] - Unsigned Integer expected");
        L_ret = -1 ;
      } 

      if ( (L_value2 != NULL)  && (L_ret != -1 )) {
        if ((strlen(L_value2)>2) && (L_value2[0] == '0') && (L_value2[1] == 'x')) { // hexa buffer value
          P_res->m_value.m_val_struct.m_id_2 = strtoul_f(L_value2, &L_endstr, 16) ;
        } else {
          P_res->m_value.m_val_struct.m_id_2 = strtoul_f(L_value2, &L_endstr, 10) ;
        }
       if (L_endstr[0] != '\0') {
        GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Incorrect value format for ["
	    << L_value2 << "] - Unsigned Integer expected");
        L_ret = -1 ;
        } 

      }
      if (L_ret != -1) *L_value3 = ';' ;

	    
    } else {
      char * L_value = (P_value == NULL) ? (char *)"not set" : P_value ;
      GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Incorrect value format for ["
	    << L_value << "] - struct expected");
      L_ret = -1 ;
    }

    }
    break;

  case E_TYPE_NUMBER_64 :
    if ((strlen(P_value)>2) && (P_value[0] == '0') && (P_value[1] == 'x')) { // hexa buffer value
      P_res->m_value.m_val_number_64 = strtoull_f(P_value, &L_endstr, 16) ;
    } else {
      P_res->m_value.m_val_number_64 = strtoull_f(P_value, &L_endstr, 10) ;
    }
    if (L_endstr[0] != '\0') {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Incorrect value format for ["
	    << P_value << "] - Unsigned Integer64 expected");
      L_ret = -1 ;
    } 
    break;
  case E_TYPE_SIGNED_64 : {
    if ((strlen(P_value)>2) && (P_value[0] == '0') && (P_value[1] == 'x')) { // hexa buffer value
      P_res->m_value.m_val_signed_64 = strtoll_f(P_value, &L_endstr, 16) ;
    } else {
      P_res->m_value.m_val_signed_64 = strtoll_f(P_value, &L_endstr, 10) ;
    }

    if (L_endstr[0] != '\0') {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Incorrect value format for ["
	    << P_value << "] - Integer64 expected");
      L_ret = -1 ;
    } 
  }
    break;

  default:
    GEN_ERROR(E_GEN_FATAL_ERROR, "Type not implemented");
    L_ret = -1 ;
    break ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::set_body_value() end return: " <<L_ret);

  return (L_ret);

}


int  C_ProtocolBinary::set_body_sub_value(int          P_index,
				    int          P_id,
				    char*        P_value, 
				    T_pBodyValue P_res) {

  int   L_ret = 0 ;
  T_pBodyValue L_res ;

  GEN_DEBUG(1, "C_ProtocolBinary::set_body_sub_value() start");
  GEN_DEBUG(1, " P_index: " << P_index);
  GEN_DEBUG(1, " P_id:    " << P_id);


  L_res = &(P_res -> m_sub_val[P_index]) ;
  L_ret = set_body_value(P_id, P_value, 0, L_res) ;

  GEN_DEBUG(1, "C_ProtocolBinary::set_body_sub_value() end return: " <<L_ret);

  return (L_ret);

}



void C_ProtocolBinary::set_body_value(T_pBodyValue P_res, T_pBodyValue P_val) {

  int   L_type_id = 0 ;
  unsigned long L_i ;

  GEN_DEBUG(1, "C_ProtocolBinary::set_body_value() T_pBodyValue - T_pBodyValue start");
  GEN_DEBUG(1, "P_res = " << P_res << " P_val = " << P_val);

  (P_res->m_id) = (P_val-> m_id) ;

  // retrieve type definition of the field
  L_type_id = m_header_body_value_table[P_res->m_id].m_type_id ;

  switch (m_type_def_table[L_type_id].m_type) {
  case E_TYPE_NUMBER :
    P_res->m_value.m_val_number = P_val->m_value.m_val_number;
    break;
  case E_TYPE_SIGNED :
    P_res->m_value.m_val_signed = P_val->m_value.m_val_signed;
    break;
  case E_TYPE_STRING :
    ALLOC_TABLE(P_res->m_value.m_val_binary.m_value,
		unsigned char *,
		sizeof(unsigned char),
		P_val ->m_value.m_val_binary.m_size);
    memcpy(P_res->m_value.m_val_binary.m_value,
	   P_val->m_value.m_val_binary.m_value,
	   P_val ->m_value.m_val_binary.m_size);
    P_res->m_value.m_val_binary.m_size=P_val ->m_value.m_val_binary.m_size;
    break;
  case E_TYPE_STRUCT :
    P_res->m_value.m_val_struct = P_val->m_value.m_val_struct;
    break;
  case E_TYPE_GROUPED:
    P_res->m_value.m_val_number = P_val->m_value.m_val_number ;
    if (P_res->m_value.m_val_number != 0 ) {
    ALLOC_TABLE(P_res->m_sub_val,
		T_pBodyValue,
		sizeof(T_BodyValue),
		P_res->m_value.m_val_number);
    for (L_i = 0 ; L_i < P_res->m_value.m_val_number; L_i++) {
      set_body_value(&(P_res->m_sub_val[L_i]), &(P_val->m_sub_val[L_i]));
    }
    } else {
      P_res->m_sub_val = NULL ;
    }
    break ;
  case E_TYPE_NUMBER_64 :
    P_res->m_value.m_val_number_64 = P_val->m_value.m_val_number_64;
    break;
  case E_TYPE_SIGNED_64 :
    P_res->m_value.m_val_signed_64 = P_val->m_value.m_val_signed_64;
    break;
  default:
    GEN_FATAL(E_GEN_FATAL_ERROR, "Value type not implemented");
    break ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::set_body_value() end");
}

void C_ProtocolBinary::delete_body_value(T_pBodyValue P_res) {

  int   L_type_id = 0 ;
  int   L_i ;

  GEN_DEBUG(1, "C_ProtocolBinary::delete_body_value() start");
  GEN_DEBUG(1, "C_ProtocolBinary::delete_body_value() Value Id: " 
               << P_res->m_id);

  // retrieve type definition of the field
  L_type_id = m_header_body_value_table[P_res->m_id].m_type_id ;

  GEN_DEBUG(1, "C_ProtocolBinary::delete_body_value() Type: " 
               << m_type_def_table[L_type_id].m_type 
	       << "(" << m_header_body_value_table[P_res->m_id].m_type_id << ")");

  switch (m_type_def_table[L_type_id].m_type) {
  case E_TYPE_STRING :
    FREE_TABLE(P_res->m_value.m_val_binary.m_value);
    P_res->m_value.m_val_binary.m_size = 0 ;
    break;
  case E_TYPE_GROUPED :
    GEN_DEBUG(1, "C_ProtocolBinary::delete_body_value() Grouped nb: " 
                 << (int)P_res->m_value.m_val_number);
    if (P_res->m_value.m_val_number != 0 ) {

    for (L_i = 0 ; L_i < (int)P_res->m_value.m_val_number; L_i++) {
      delete_body_value(&(P_res->m_sub_val[L_i])) ;
      GEN_DEBUG(1, "C_ProtocolBinary::delete_body_value() Grouped nb id: " 
                    << L_i);
    }
    FREE_TABLE (P_res->m_sub_val);
    P_res->m_value.m_val_number=0 ;
    }
    break ;
  default:
    break ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::delete_body_value() end");
}

void C_ProtocolBinary::reset_grouped_body_value(T_pBodyValue P_res) {

  int   L_type_id = 0 ;
  int   L_i ;

  GEN_DEBUG(1, "C_ProtocolBinary::reset_grouped_body_value() start");
  GEN_DEBUG(1, "C_ProtocolBinary::reset_grouped_body_value() Value Id: " 
               << P_res->m_id);

  // retrieve type definition of the field
  L_type_id = m_header_body_value_table[P_res->m_id].m_type_id ;

  GEN_DEBUG(1, "C_ProtocolBinary::reset_grouped_body_value() Type: " 
               << m_type_def_table[L_type_id].m_type 
	       << "(" << m_header_body_value_table[P_res->m_id].m_type_id << ")");

  switch (m_type_def_table[L_type_id].m_type) {
  case E_TYPE_GROUPED :
    GEN_DEBUG(1, "C_ProtocolBinary::reset_grouped_body_value() Grouped nb: " 
                 << (int)P_res->m_value.m_val_number);

    if (P_res->m_value.m_val_number != 0 ){

    for (L_i = 0 ; L_i < (int)P_res->m_value.m_val_number; L_i++) {
      reset_grouped_body_value(&(P_res->m_sub_val[L_i])) ;
      GEN_DEBUG(1, "C_ProtocolBinary::reset_grouped_body_value() Grouped nb id: " 
                    << L_i);
    }
    FREE_TABLE (P_res->m_sub_val);
    P_res->m_value.m_val_number=0 ;
    }
    break ;
  default:
    break ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::reset_grouped_body_value() end");
}

void C_ProtocolBinary::update_length (unsigned char *P_buf,
                                      size_t         P_size) {

  unsigned long L_new_size ;

  if (!m_transport_type) {
    L_new_size = (unsigned long) P_size ;
    convert_ul_to_bin_network(P_buf+m_header_length_index,
                              m_header_field_table[m_header_length_id].m_size,
                              L_new_size);
  }  
}

int  C_ProtocolBinary::decode_header (unsigned char *P_buf, 
				      size_t         P_size,
				      T_pValueData   P_valDec) {
  
  unsigned char        *L_ptr = P_buf  ;
  unsigned long         L_fieldIdx     ;
  T_pHeaderField        L_fieldDescr   ;
  unsigned long         L_current_size = 0;
  T_UnsignedInteger32   L_current_value = 0;
  long                  L_current_type_id;
  T_DecodeMap::iterator L_decodeIt ;
  int                   L_ret = -1 ;

  GEN_DEBUG(1, "C_ProtocolBinary::decode_header() start");


  for(L_fieldIdx=0; L_fieldIdx < m_nb_field_header; L_fieldIdx++) {

    L_fieldDescr      = &m_header_field_table[L_fieldIdx] ;
    L_current_size    = L_fieldDescr -> m_size ;
    L_current_type_id = L_fieldDescr -> m_type_id ; 

    if (L_current_type_id == -1)
    {
      P_valDec[L_fieldIdx].m_type = E_TYPE_NUMBER ;

      GEN_DEBUG(1, "C_ProtocolBinary::decode_header() value ["
                << L_fieldDescr->m_name << "] with type [number] "
                << " ("<<L_current_type_id<<")]");
    } else {
      P_valDec[L_fieldIdx].m_type = m_type_def_table[L_current_type_id].m_type ;

      GEN_DEBUG(1, "C_ProtocolBinary::decode_header() value ["
                << L_fieldDescr->m_name << "] with type ["
                << m_type_def_table[L_current_type_id].m_name
                << " ("<<L_current_type_id<<")]");

    }

    switch (P_valDec[L_fieldIdx].m_type) {
        case E_TYPE_NUMBER:
          GEN_DEBUG(1, "Number value decoding (" << L_fieldIdx << ")");
          P_valDec[L_fieldIdx].m_id = L_fieldDescr->m_id ;
          P_valDec[L_fieldIdx].m_value.m_val_number
            = convert_bin_network_to_ul(L_ptr, L_current_size) ;

          // Store the current number value for decoding
          L_current_value =  P_valDec[L_fieldIdx].m_value.m_val_number;

          if (L_fieldDescr -> m_cond_presence != NULL) {
            L_current_value = (L_fieldDescr->m_cond_presence)->m_mask & L_current_value ;
            P_valDec[L_fieldIdx].m_value.m_val_number = L_current_value ;
          }

          GEN_DEBUG(1, "Number value = " << P_valDec[L_fieldIdx].m_value.m_val_number);
          break ;

        case E_TYPE_SIGNED:
          GEN_DEBUG(1, "Signed value decoding (" << L_fieldIdx << ")");
          P_valDec[L_fieldIdx].m_id = L_fieldDescr->m_id ;
          P_valDec[L_fieldIdx].m_value.m_val_signed
            = convert_bin_network_to_l(L_ptr, L_current_size) ;

          GEN_DEBUG(1, "Signed value = " << P_valDec[L_fieldIdx].m_value.m_val_signed);
          break ;

        case E_TYPE_STRING:
          GEN_DEBUG(1, "String value decoding (" << L_fieldIdx << ")");
          P_valDec[L_fieldIdx].m_id = L_fieldDescr->m_id ;
          P_valDec[L_fieldIdx].m_value.m_val_binary.m_size = L_current_size ;
          ALLOC_TABLE(P_valDec[L_fieldIdx].m_value.m_val_binary.m_value,
                      unsigned char*,
                      sizeof(unsigned char),
                      L_current_size);
          memcpy(P_valDec[L_fieldIdx].m_value.m_val_binary.m_value,
                 L_ptr,
                 L_current_size);
          GEN_DEBUG(1, "String value size = "
                << P_valDec[L_fieldIdx].m_value.m_val_binary.m_size);
          break ;

        case E_TYPE_STRUCT: {
          GEN_DEBUG(1, "Number value decoding (" << L_fieldIdx << ")");
          size_t   L_sub_size = L_current_size/2 ;
          P_valDec[L_fieldIdx].m_id = L_fieldDescr->m_id ;
          P_valDec[L_fieldIdx].m_value.m_val_struct.m_id_1
            = convert_bin_network_to_ul(L_ptr, L_sub_size) ;

          P_valDec[L_fieldIdx].m_value.m_val_struct.m_id_2
            = convert_bin_network_to_ul(L_ptr + L_sub_size, L_sub_size) ;


          GEN_DEBUG(1, "Number value 1= " 
                       << P_valDec[L_fieldIdx].m_value.m_val_struct.m_id_1);
          GEN_DEBUG(1, "Number value 2= " 
                       << P_valDec[L_fieldIdx].m_value.m_val_struct.m_id_2);
          }
          break ;

        case E_TYPE_GROUPED:
          GEN_DEBUG(1, "Grouped values decoding (" << L_fieldIdx << ")");
          P_valDec[L_fieldIdx].m_id = L_fieldDescr->m_id ;

        case E_TYPE_NUMBER_64:
          GEN_DEBUG(1, "Number64 value decoding (" << L_fieldIdx << ")");
          P_valDec[L_fieldIdx].m_id = L_fieldDescr->m_id ;
          P_valDec[L_fieldIdx].m_value.m_val_number_64
            = convert_bin_network_to_ull(L_ptr, L_current_size) ;

          GEN_DEBUG(1, "Number64 value = " << P_valDec[L_fieldIdx].m_value.m_val_number_64);
          break ;

        case E_TYPE_SIGNED_64:
          GEN_DEBUG(1, "Signed64 value decoding (" << L_fieldIdx << ")");
          P_valDec[L_fieldIdx].m_id = L_fieldDescr->m_id ;
          P_valDec[L_fieldIdx].m_value.m_val_signed_64
            = convert_bin_network_to_ll(L_ptr, L_current_size) ;

          GEN_DEBUG(1, "Signed64 value = " << P_valDec[L_fieldIdx].m_value.m_val_signed_64);
          break ;

        default:
          GEN_ERROR(E_GEN_FATAL_ERROR, "value header type not implemented");
          break ;
    }
    L_ptr += L_current_size ;


    // NOT IMPLEMENTED 
    // if values have to be tested => do it here

    GEN_DEBUG(1, "C_ProtocolBinary::decode_header() L_fieldIdx " << 
	          L_fieldIdx << " and m_header_type_id  " << m_header_type_id );

    // m_header_type_id_body
    if (m_header_type_id != -1) {
      if (L_fieldIdx == (unsigned long) m_header_type_id) {

        // retrieve type defined
        L_decodeIt = m_header_decode_map
	  ->find(T_DecodeMap::key_type(L_current_value));
        if (L_decodeIt != m_header_decode_map->end()) {
	  // header recognized
	  L_ret = L_decodeIt->second ;

	  if (m_stats) {
	    m_stats->updateStats (E_MESSAGE,
				  E_RECEIVE,
				  L_ret);
	  }

          GEN_DEBUG(1, "C_ProtocolBinary::decode_header() L_ret: " << L_ret);
        }
      }
    } 
  }

  GEN_DEBUG(1, "C_ProtocolBinary::decode_header() end l_ret: " << L_ret);
  return (L_ret) ;
}

int C_ProtocolBinary::decode_body(unsigned char *P_buf, 
			          size_t         P_size,
			          T_pBodyValue   P_valDec,
			          int           *P_nbValDec,
			          int           *P_headerId) {

  int                   L_max_values = *P_nbValDec ;
  unsigned char        *L_ptr = P_buf ;
  int                   L_ret = 0 ;
  T_pHeaderField        L_body_fieldDescr   ;
  T_pHeaderBodyValue    L_body_fieldValues  ;
  unsigned long         L_body_fieldIdx, L_current_size, L_current_value ;
  unsigned long         L_total_size, L_data_size, L_data_type, L_padding ;
  T_DecodeMap::iterator L_decodeIt ;
  int                   L_body_value_id ;
  int                   L_type_id ;
  T_TypeType            L_type ;
  int                   L_nbValDec = 0 ;

  unsigned long         L_body_found_val [100] ; // TEMPORARY
  T_BodyValue           L_body_val       [50]  ; // TEMPORARY
  unsigned long         L_i ;

  unsigned long         L_type_id_val ;

  int                   L_header_type_id = get_header_type_id();

  unsigned long         L_body_fieldIdx2  ;
  bool                  L_header_body_type_id_present ;
  unsigned long         L_header_body_size ;
 


  GEN_DEBUG(1, "\nC_ProtocolBinary::decode_body() start");
  GEN_DEBUG(1, "C_ProtocolBinary::decode_body() P_size: " << P_size);
  GEN_DEBUG(1, "C_ProtocolBinary::decode_body() P_nbValDec: " << *P_nbValDec);
  GEN_DEBUG(1, "C_ProtocolBinary::decode_body() P_headerId: " << *P_headerId);

  L_total_size = 0 ;
  *P_nbValDec  = 0 ;

  while (L_total_size < P_size) {
    
    GEN_DEBUG(1, "\nField Nb: " << L_nbValDec);
    
    L_data_size = 0  ;
    L_data_type = 0  ;
    L_header_body_type_id_present = false ;
    L_header_body_size  = 0 ;
    
    if (L_nbValDec == L_max_values) {
      GEN_FATAL(E_GEN_FATAL_ERROR, "Maximum number of values reached ["
                << L_max_values << "]");
      break ;
    }
    
    // Decode field header 

    for(L_body_fieldIdx=0; 
	L_body_fieldIdx < m_nb_field_header_body; 
	L_body_fieldIdx++) {
      
      L_body_fieldDescr = &m_header_body_field_table[L_body_fieldIdx];
      L_current_size  = L_body_fieldDescr -> m_size ;
      L_header_body_size  += L_current_size ;

      L_current_value = convert_bin_network_to_ul(L_ptr, L_current_size);
      L_body_found_val[L_body_fieldIdx] = L_current_value ;
      if (L_body_fieldIdx == (unsigned long)m_header_body_type_id) {        
        L_decodeIt = 
          m_header_body_decode_map->find (T_DecodeMap::key_type(L_current_value));
        
        if (L_decodeIt != m_header_body_decode_map->end()) {
          L_body_value_id = L_decodeIt->second ;
          L_body_fieldValues = &m_header_body_value_table[L_body_value_id] ;
          L_header_body_type_id_present = true;
        } else {
          GEN_WARNING("No definition found for ["
                      << L_current_value << "]");
        }
      }
      
      L_total_size += L_current_size ;
      if (L_total_size >= P_size) break ;
      L_ptr += L_current_size ;
      if (L_header_body_type_id_present) break;

    }

    for(L_body_fieldIdx2 = L_body_fieldIdx +1 ; 
	L_body_fieldIdx2 < m_nb_field_header_body; 
	L_body_fieldIdx2++) {
      
      if (L_body_fieldValues-> m_present[L_body_fieldIdx2]) {
      L_current_size  = L_body_fieldValues-> m_size[L_body_fieldIdx2] ;
      L_header_body_size  += L_current_size ;
      L_current_value = convert_bin_network_to_ul(L_ptr, L_current_size);
      L_body_found_val[L_body_fieldIdx2] = L_current_value ;
      
      if (L_body_fieldIdx2 == (unsigned long)m_header_body_length_id) {
        L_data_size = L_current_value ;
      }
      
      L_total_size += L_current_size ;
      if (L_total_size >= P_size) break ;
      L_ptr += L_current_size ;
      }
    } // for

    // Retrieve optional fields
    if (m_header_body_start_optional_id != -1) {
      for(L_body_fieldIdx=m_header_body_start_optional_id; 
	  L_body_fieldIdx < m_max_nb_field_header_body; 
	  L_body_fieldIdx++) {

	L_body_fieldDescr = &m_header_body_field_table[L_body_fieldIdx];
	if (check_presence_needed(L_body_fieldDescr->m_cond_presence,
				  L_body_found_val) == true) {
	  // L_current_size = L_body_fieldDescr -> m_size ;
          L_current_size = L_body_fieldValues-> m_size[L_body_fieldIdx] ;
          L_header_body_size += L_current_size ;
          
	  L_current_value = convert_bin_network_to_ul(L_ptr, L_current_size);
	  L_total_size += L_current_size ;
	  if (L_total_size >= P_size) break ;
	  L_ptr += L_current_size ;
	}
      }
    } // if (m_header_body_start_optional_id != -1)
    
    if(! get_header_length_excluded ()) {
      L_data_size -= L_header_body_size ;
    }

    // padding ?
    if (m_padding_value) {
      L_padding = L_data_size % m_padding_value ;
      if (L_padding) { L_padding = m_padding_value - L_padding ; }
    } else {
      L_padding = 0 ;
    }

    GEN_DEBUG(1, "body field data size " << L_data_size << " ");

    // now retrieve data type and value
    if ((L_total_size + L_data_size) <= P_size) {
      if (m_stats) {
        m_stats->updateStats (E_MESSAGE_COMPONENT,
                              E_RECEIVE,
                              L_body_value_id);
      }
      
      L_type_id = L_body_fieldValues->m_type_id ;
      L_type = m_type_def_table[L_type_id].m_type ;
      
      GEN_DEBUG(1, "body value ["
                << L_body_fieldValues->m_name << "] with type [" 
                << m_type_def_table[L_type_id].m_name
                << " ("<<L_type<<")]");
      switch (L_type) {
      case E_TYPE_NUMBER:
        GEN_DEBUG(1, "  Number value decoding (" << L_nbValDec << ")");
        P_valDec[L_nbValDec].m_id = L_body_value_id ;
        P_valDec[L_nbValDec].m_value.m_val_number 
          = convert_bin_network_to_ul(L_ptr, L_data_size) ;
        
        GEN_DEBUG(1, "  Number value = " 
                  << P_valDec[L_nbValDec].m_value.m_val_number);
        L_nbValDec ++ ;
        break ;
        
      case E_TYPE_SIGNED:
        GEN_DEBUG(1, "  Signed value decoding (" << L_nbValDec << ")");
        // GEN_DEBUG(1, "Val value = " << L_ptr[0] << " ");
        P_valDec[L_nbValDec].m_id = L_body_value_id ;
        
        P_valDec[L_nbValDec].m_value.m_val_signed 
          = convert_bin_network_to_l(L_ptr, L_data_size) ;
        
        GEN_DEBUG(1, "  Signed value = " 
                  << P_valDec[L_nbValDec].m_value.m_val_signed);
        L_nbValDec ++ ;
        break ;
        
      case E_TYPE_STRING:
        GEN_DEBUG(1, "  String value decoding (" << L_nbValDec << ")");
        P_valDec[L_nbValDec].m_id = L_body_value_id ;
        P_valDec[L_nbValDec].m_value.m_val_binary.m_size = L_data_size ;
        
        ALLOC_TABLE(P_valDec[L_nbValDec].m_value.m_val_binary.m_value,
                    unsigned char*,
                    sizeof(unsigned char),
                    L_data_size);
        memcpy(P_valDec[L_nbValDec].m_value.m_val_binary.m_value,
               L_ptr,
               L_data_size);
        GEN_DEBUG(1, "  String value size = " 
                  << P_valDec[L_nbValDec].m_value.m_val_binary.m_size);

        L_nbValDec ++ ;
        break ;
        
      case E_TYPE_STRUCT: {
        GEN_DEBUG(1, "  Struct value decoding (" << L_nbValDec << ")");
        size_t   L_sub_size = L_data_size/2 ;
        P_valDec[L_nbValDec].m_id = L_body_value_id ;
        
        P_valDec[L_nbValDec].m_value.m_val_struct.m_id_1 
          = convert_bin_network_to_ul(L_ptr, L_sub_size) ;
        
        P_valDec[L_nbValDec].m_value.m_val_struct.m_id_2 
          = convert_bin_network_to_ul(L_ptr + L_sub_size, L_sub_size) ;
        
        
        GEN_DEBUG(1, "  Number value 1 = " 
                  << P_valDec[L_nbValDec].m_value.m_val_struct.m_id_1);
        GEN_DEBUG(1, "  Number value 2 = " 
                  << P_valDec[L_nbValDec].m_value.m_val_struct.m_id_2);
        L_nbValDec ++ ;
      }
      break ;
      
      case E_TYPE_GROUPED:
	{
	  GEN_DEBUG(1, "  Grouped values decoding (" << L_nbValDec << ")");
	  P_valDec[L_nbValDec].m_id = L_body_value_id ;
          
	  int L_val_nb = 50;
          
	  L_ret = decode_body(L_ptr, 
			      (size_t)L_data_size, 
			      L_body_val, 
			      &L_val_nb,
			      P_headerId) ;
	  if (L_ret == 0) {
	    GEN_DEBUG(1, "  allocated nb values: " << L_val_nb 
                      << " on grouped value (" << L_nbValDec << ")" );
            
            
	    P_valDec[L_nbValDec].m_value.m_val_number = L_val_nb ;
            if (L_val_nb > 0){
              ALLOC_TABLE(P_valDec[L_nbValDec].m_sub_val,
                          T_pBodyValue,
                          sizeof(T_BodyValue),
                          L_val_nb);
              for (L_i = 0 ; L_i < (unsigned int) L_val_nb; L_i++) {
                set_body_value(&(P_valDec[L_nbValDec].m_sub_val[L_i]), 
                               &(L_body_val[L_i]));
              }
              // Now do not forget to clean L_body_val
              reset_body_values(L_val_nb, L_body_val);
            } else {
              P_valDec[L_nbValDec].m_sub_val = NULL ;
            }
            
	    L_nbValDec++;
	    
	  } else {
	    GEN_ERROR (E_GEN_FATAL_ERROR, "Erroneous grouped value");
	    L_ret = -1 ;
	  }
          
	}
        break ;
        
      case E_TYPE_NUMBER_64:
        GEN_DEBUG(1, "  Number64 value decoding (" << L_nbValDec << ")");
        P_valDec[L_nbValDec].m_id = L_body_value_id ;
        P_valDec[L_nbValDec].m_value.m_val_number_64 
          = convert_bin_network_to_ull(L_ptr, L_data_size) ;
        
        GEN_DEBUG(1, "  Number64 value = " 
                  << P_valDec[L_nbValDec].m_value.m_val_number_64);
        L_nbValDec ++ ;
        break ;
        
      case E_TYPE_SIGNED_64:
        GEN_DEBUG(1, "  Signed64 value decoding (" << L_nbValDec << ")");
        P_valDec[L_nbValDec].m_id = L_body_value_id ;
        P_valDec[L_nbValDec].m_value.m_val_signed_64 
          = convert_bin_network_to_ll(L_ptr, L_data_size) ;
        
        GEN_DEBUG(1, "  Signed64 value = " 
                  << P_valDec[L_nbValDec].m_value.m_val_signed_64);
        L_nbValDec ++ ;
        break ;
        
      default:
        GEN_ERROR(E_GEN_FATAL_ERROR, "value body type not implemented");
        break ;
      }
      
      if (L_header_type_id == -1) {
        
        // Now check if the current field is the msg type one
        if (L_body_value_id == m_header_type_id_body) {
          
          L_type_id_val = P_valDec[L_nbValDec-1].m_value.m_val_number ;
          
          GEN_DEBUG(1, "body value ["
                    << L_body_fieldValues->m_name << "] with id [" 
                    << L_body_value_id 
                    << "] specify the message type: " << L_type_id_val);
          
          L_decodeIt = m_header_decode_map
            ->find(T_DecodeMap::key_type(L_type_id_val));
          if (L_decodeIt != m_header_decode_map->end()) {
            // header recognized
            (*P_headerId) = L_decodeIt->second ;
          } else {
            GEN_LOG_EVENT_FORCE("not found");
          }
          
        }
      }
      
      L_ptr += L_data_size ;
      L_ptr += L_padding ;
      L_total_size += L_data_size + L_padding ;
      
    } else {
      GEN_ERROR (E_GEN_FATAL_ERROR, "message size error (body size)");
      break ;
    }
    
    if (L_ret == -1) break ;
    
  } // End while
  
  *P_nbValDec = L_nbValDec ;
  if (L_total_size != P_size) { L_ret = -1 ; } 
  
  GEN_DEBUG(1, "C_ProtocolBinary::decode_body() end nb Val: " << L_nbValDec
	        << " ret: " << L_ret << "\n");
  return (L_ret) ;

}

void C_ProtocolBinary::encode_header (int P_id, 
				unsigned char *P_buf, 
				size_t *P_size) {

  unsigned char *L_ptr = P_buf  ;
  unsigned long  L_fieldIdx     ;
  int            L_valueIdx     ;
  T_pHeaderField L_fieldDescr   ;
  T_pHeaderValue L_fieldValues  ;
  unsigned long  L_current_size, L_total_size ;
  unsigned long  L_current_value ;

  GEN_DEBUG(1, "C_ProtocolBinary::encode_header(" << P_id 
	<< "," << &P_buf << "," << P_size << ") 2 start");

  L_current_size = 0 ;
  L_total_size = 0 ;
  L_fieldValues = &m_header_value_table[P_id] ;

  GEN_DEBUG(1, "C_ProtocolBinary::encode_header m_nb_field_header = " 
               << m_nb_field_header);


  for(L_fieldIdx=0; L_fieldIdx < m_nb_field_header; L_fieldIdx++) {

    L_fieldDescr = &m_header_field_table[L_fieldIdx] ;
    L_current_size = L_fieldDescr -> m_size ;
    L_total_size += L_current_size ;

    GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_fieldIdx = " 
                 << L_fieldIdx); 
    GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_current_size = " 
                 << L_current_size);
    GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_total_size = " 
                 << L_total_size);

    if (L_fieldValues->m_value_setted[L_fieldIdx] == false) {
      L_current_value = (unsigned long) 0 ;
    } else {
      L_valueIdx = L_fieldValues->m_id_value_setted[L_fieldIdx];
      L_current_value = L_fieldValues->m_values[L_valueIdx].m_value.m_val_number ;
    }

    GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_current_value " 
                 << L_current_value);

    convert_ul_to_bin_network(L_ptr,
			      L_current_size,
			      L_current_value) ;

    L_ptr += L_current_size ;
  }

  *P_size = L_total_size ;



  GEN_DEBUG(1, "C_ProtocolBinary::encode_header() 2 end");

}


void C_ProtocolBinary::encode_header (int            P_id,
				      T_pValueData   P_headerVal,
				      unsigned char *P_buf, 
				      size_t        *P_size,
                                      C_ProtocolFrame::T_pMsgError P_error) {

  unsigned char      *L_ptr = P_buf  ;
  unsigned long       L_fieldIdx     ;
  T_pHeaderField      L_fieldDescr   ;

  unsigned long       L_current_size, L_total_size ;
  T_UnsignedInteger32 L_current_value ;
  T_UnsignedInteger64 L_current_value_ll ;

  int            L_type_id ;
  T_TypeType     L_type ;


  GEN_DEBUG(1, "C_ProtocolBinary::encode_header(" << P_id 
	<< "," << &P_buf << "," << P_size << ") 1 start");

  *P_error = C_ProtocolFrame::E_MSG_OK ;

  L_current_size = 0 ;
  L_total_size = 0 ;


  GEN_DEBUG(1, "C_ProtocolBinary::encode_header m_nb_field_header = " 
               << m_nb_field_header);

  if (m_stats) {
    m_stats->updateStats(E_MESSAGE,
			 E_SEND,
			 P_id);
  }

  for(L_fieldIdx=0; L_fieldIdx < m_nb_field_header; L_fieldIdx++) {

    L_fieldDescr = &m_header_field_table[L_fieldIdx] ;
    
    L_current_size = L_fieldDescr -> m_size ;
    
    L_total_size += L_current_size ;

    if (L_total_size > *P_size) {
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                    "Buffer max size reached [" << *P_size << "]");
      *P_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
      break ;
    }

    L_type_id = L_fieldDescr->m_type_id ;
    if (L_type_id == -1)
    {
      L_type = E_TYPE_NUMBER;

      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_fieldIdx = " 
                   << L_fieldDescr -> m_name << "(" <<  L_fieldIdx << ")" ); 
      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_type = number" );
      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_current_size = " 
                   << L_current_size);
      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_total_size = " 
                   << L_total_size);

    } else {
      L_type = m_type_def_table[L_type_id].m_type ;

      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_fieldIdx = " 
                   << L_fieldDescr -> m_name << "(" <<  L_fieldIdx << ")" ); 
      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_type = " 
                   << m_type_def_table[L_type_id].m_name);
      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_current_size = " 
                   << L_current_size);
      GEN_DEBUG(1, "C_ProtocolBinary::encode_header L_total_size = " 
                   << L_total_size);

    }

    // now add the value of the body
    switch (L_type) {

    case E_TYPE_NUMBER:
      L_current_value = P_headerVal[L_fieldIdx].m_value.m_val_number ;

      GEN_DEBUG(1, "Number Value set = " << L_current_value);

      convert_ul_to_bin_network(L_ptr,
                                L_current_size,
                                L_current_value);
      break ;

    case E_TYPE_SIGNED:
      L_current_value = P_headerVal[L_fieldIdx].m_value.m_val_signed ;

      GEN_DEBUG(1, "Signed Value set = " << L_current_value);

      convert_l_to_bin_network(L_ptr,
                               L_current_size,
                               L_current_value);
      break ;

    case E_TYPE_STRING: {
      size_t L_padding ;
      if (P_headerVal[L_fieldIdx].m_value.m_val_binary.m_size == 0) {
        GEN_ERROR(E_GEN_FATAL_ERROR, 
                  "The value of the field ["
                  << L_fieldDescr -> m_name 
                  << "] is not set");
        
        //        *P_error = C_ProtocolBinaryFrame::E_MSG_ERROR_ENCODING ;
        *P_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
        break;
      }

      memcpy(L_ptr, P_headerVal[L_fieldIdx].m_value.m_val_binary.m_value, L_current_size);
      if (m_padding_value) {
        L_padding = L_current_size % m_padding_value ;
        if (L_padding) { L_padding = m_padding_value - L_padding ; }
        while (L_padding) {
          *(L_ptr+L_current_size) = '\0' ;
          L_current_size++ ;
          L_padding-- ;
        }
      } else {
        L_padding = 0 ;
      }
    }
        break ;

    case E_TYPE_STRUCT: {
      size_t   L_sub_value_size = L_current_size/2 ;

      convert_ul_to_bin_network(L_ptr,
                                L_sub_value_size,
                                P_headerVal[L_fieldIdx].m_value.m_val_struct.m_id_1);

      convert_ul_to_bin_network(L_ptr + L_sub_value_size,
                                L_sub_value_size,
                                P_headerVal[L_fieldIdx].m_value.m_val_struct.m_id_2);
      }
        break ;

    case E_TYPE_NUMBER_64:
      L_current_value_ll = P_headerVal[L_fieldIdx].m_value.m_val_number_64 ;

      GEN_DEBUG(1, "Number64 Value set = " << L_current_value_ll);

      convert_ull_to_bin_network(L_ptr,
                                 L_current_size,
                                 L_current_value_ll);
      break ;

    case E_TYPE_SIGNED_64:
      L_current_value_ll = P_headerVal[L_fieldIdx].m_value.m_val_signed_64 ;

      GEN_DEBUG(1, "Signed64 Value set = " << L_current_value_ll);

      convert_ll_to_bin_network(L_ptr,
                                L_current_size,
                                L_current_value_ll);
      break ;

    case E_TYPE_GROUPED:
    default:
      GEN_FATAL(E_GEN_FATAL_ERROR,
            "Encoding method not implemented for this value");
      break ;
    }

    L_ptr += L_current_size ;
  }


  if (*P_error == C_ProtocolFrame::E_MSG_OK) {
    *P_size = L_total_size ;
  }


  GEN_DEBUG(1, "C_ProtocolBinary::encode_header() 1 end");

}


C_ProtocolFrame::T_MsgError C_ProtocolBinary::encode_body (int            P_nbVal, 
                                                           T_pBodyValue   P_val,
                                                           unsigned char *P_buf, 
                                                           size_t        *P_size) {

  unsigned char     *L_ptr = P_buf ;
  int                L_i, L_body_id ;
  int                L_valueIdx     ;
  size_t             L_total_size   = 0 ;
  size_t             L_current_size = 0 ;
  T_pHeaderField     L_body_fieldDescr   ;
  T_pHeaderBodyValue L_body_fieldValues  ;
  T_pBodyValue       L_body_val ;
  unsigned long      L_body_fieldIdx, L_valueSize  ;
  int                L_type_id ;
  T_TypeType         L_type ;
  unsigned long      L_current_value ;

  unsigned long      L_header_body_size ;

  unsigned char *L_save_length_ptr = NULL;
  unsigned long  L_save_length = 0;
  size_t         L_length_size = 0;

  size_t         L_sub_size ;
  C_ProtocolFrame::T_MsgError  L_error = C_ProtocolFrame::E_MSG_OK;

  GEN_DEBUG(1, "C_ProtocolBinary::encode_body() start");

  L_total_size = 0 ;
  L_current_size = 0 ;

  for (L_i = 0; L_i < P_nbVal ; L_i ++) {

    L_body_val = &P_val[L_i] ;
    L_body_id = L_body_val->m_id  ;


    if (m_stats) {
      m_stats->updateStats (E_MESSAGE_COMPONENT,
			    E_SEND,
			    L_body_id);
    }

    L_body_fieldValues = &m_header_body_value_table[L_body_id] ;
    L_type_id = L_body_fieldValues->m_type_id ;
    L_type = m_type_def_table[L_type_id].m_type ;

    if (L_type == E_TYPE_STRING) {
      L_valueSize = 
	L_body_val -> m_value.m_val_binary.m_size ;
    } else {
      L_valueSize = 
	m_type_def_table[L_type_id].m_size ;
    }
   
    L_header_body_size = 0 ;
    L_save_length_ptr = NULL ;
    for(L_body_fieldIdx=0; 
	L_body_fieldIdx < m_nb_field_header_body; 
	L_body_fieldIdx++) {

      if (L_body_fieldValues->m_present[L_body_fieldIdx]) {
      L_current_size = L_body_fieldValues->m_size[L_body_fieldIdx] ;
      L_header_body_size += L_current_size ;
      L_total_size += L_current_size ;

      if (L_total_size > *P_size) {
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                      "Buffer max size reached [" << *P_size << "]");
        L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
        break ;
      }

      if (L_body_fieldIdx == (unsigned long)m_header_body_length_id) {

        L_save_length = L_valueSize ;
	L_length_size = L_current_size ;
	L_save_length_ptr = L_ptr ;
      } else {
	
	if (L_body_fieldValues->m_value_setted[L_body_fieldIdx] == false) {
	  L_current_value = (unsigned long) 0 ;
	} else {
	  L_valueIdx = L_body_fieldValues
	    ->m_id_value_setted[L_body_fieldIdx];
	  L_current_value = L_body_fieldValues
	    ->m_values[L_valueIdx].m_value.m_val_number ;
	}

	convert_ul_to_bin_network(L_ptr,
				  L_current_size,
				  L_current_value) ;
	
      }
      L_ptr += L_current_size ;
    }
    } // for(L_body_fieldIdx...

    if (L_error != C_ProtocolFrame::E_MSG_OK) {
      break ;
    }

    // optional field value management
    if (m_header_body_start_optional_id != -1) { // optional part to be added

      for(L_body_fieldIdx=m_header_body_start_optional_id; 
	  L_body_fieldIdx < m_max_nb_field_header_body; 
	  L_body_fieldIdx++) {

	L_body_fieldDescr = &m_header_body_field_table[L_body_fieldIdx];
	if (L_body_fieldValues->m_value_setted[L_body_fieldIdx] == true) {

          L_current_size = L_body_fieldValues->m_size[L_body_fieldIdx] ;
          L_header_body_size += L_current_size ;
      
	  // L_current_size = L_body_fieldDescr -> m_size ;
	  L_total_size += L_current_size ;

          if (L_total_size > *P_size) {
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                          "Buffer max size reached [" << *P_size << "]");
            L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
            break ;
          }

	  L_valueIdx = L_body_fieldValues
	    ->m_id_value_setted[L_body_fieldIdx];
	  L_current_value = L_body_fieldValues
	    ->m_values[L_valueIdx].m_value.m_val_number ;
	  convert_ul_to_bin_network(L_ptr,
				    L_current_size,
				    L_current_value) ;
	  L_ptr += L_current_size ;
	}
      } // for (L_body_fieldIdx...

    } // if (m_header_body_start_optional_id != -1)

    if (L_error == C_ProtocolFrame::E_MSG_OK) {
      if (L_save_length_ptr) {
        if(!get_header_length_excluded()) {
          L_save_length += L_header_body_size ;
        }
        convert_ul_to_bin_network(L_save_length_ptr,
                                  L_length_size,
                                  L_save_length) ;
      }
    }

    GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  value encoding for = " 
                 << L_body_fieldValues->m_name);


    if (L_error != C_ProtocolFrame::E_MSG_OK) {
      break ;
    }  


    if ((L_total_size+L_valueSize) > *P_size) {
      GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                    "Buffer max size reached [" << *P_size << "]");
      L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
      break ;
    }

    // now add the value of the body
    switch (L_type) {

    case E_TYPE_NUMBER:
      convert_ul_to_bin_network(L_ptr,
				L_valueSize,
				L_body_val -> m_value.m_val_number);

      GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  with number value = " 
                   << L_body_val->m_value.m_val_number);
      break ;

    case E_TYPE_SIGNED:
      convert_l_to_bin_network(L_ptr,
			       L_valueSize,
			       L_body_val -> m_value.m_val_signed);

      GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  with signed value = " 
                   << L_body_val->m_value.m_val_signed);
      break ;

    case E_TYPE_STRING: {
      size_t L_padding ;
      memcpy(L_ptr, L_body_val->m_value.m_val_binary.m_value, L_valueSize);
      if (m_padding_value) {
	L_padding = L_valueSize % m_padding_value ;
	if (L_padding) { 
          L_padding = m_padding_value - L_padding ; 

          if ((L_total_size+L_valueSize+L_padding) > *P_size) {
            GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                          "Buffer max size reached [" << *P_size << "]");
            L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
            break ;
          }

        }
	while (L_padding) {

          

	  *(L_ptr+L_valueSize) = '\0' ;
	  L_valueSize++ ;
	  L_padding-- ;
	}
      } else {
	L_padding = 0 ;
      }

      GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  with string value (size: " 
                   << L_valueSize << " and padding: " << L_padding);
    }
	break ;

    case E_TYPE_STRUCT: {
      size_t   L_sub_value_size = L_valueSize/2 ;   

      convert_ul_to_bin_network(L_ptr,
				L_sub_value_size,
				L_body_val -> m_value.m_val_struct.m_id_1);
      
      convert_ul_to_bin_network(L_ptr + L_sub_value_size,
				L_sub_value_size,
				L_body_val -> m_value.m_val_struct.m_id_2);

      GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  with struct value = [" 
                   << L_body_val -> m_value.m_val_struct.m_id_1 << ";" 
                   << L_body_val -> m_value.m_val_struct.m_id_2);
      }
	break ;

    case E_TYPE_GROUPED:
      GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  with grouped value " );

      L_sub_size = *P_size - L_total_size ;

      L_sub_size = *P_size - L_total_size ;

      L_error =  encode_body(L_body_val->m_value.m_val_number,
                             L_body_val->m_sub_val,
                             L_ptr,
                             &L_sub_size);
      
      if (L_error == C_ProtocolFrame::E_MSG_OK) {
        L_total_size += L_sub_size ;

        if (L_total_size > *P_size) {
          GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                        "Buffer max size reached [" << *P_size << "]");
          L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
          break ;
        }

        L_save_length += L_sub_size ;
        convert_ul_to_bin_network(L_save_length_ptr,
                                  L_length_size,
                                  L_save_length) ;
        L_ptr += L_sub_size ;
      }

      break ;
      
    case E_TYPE_NUMBER_64:
      convert_ull_to_bin_network(L_ptr,
				 L_valueSize,
				 L_body_val -> m_value.m_val_number_64);

      GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  with number64 value = " 
                   << L_body_val->m_value.m_val_number_64);
      break ;

    case E_TYPE_SIGNED_64:
      convert_ll_to_bin_network(L_ptr,
			        L_valueSize,
			        L_body_val -> m_value.m_val_signed_64);

      GEN_DEBUG(1, "C_ProtocolBinary::encode_body()  with signed64 value = " 
                   << L_body_val->m_value.m_val_signed_64);
      break ;

    default:
      GEN_FATAL(E_GEN_FATAL_ERROR, 
	    "Encoding method not implemented for this value");
      break ;
    }



    if (L_error != C_ProtocolFrame::E_MSG_OK) {
      break ;
    }  

    
    L_total_size += L_valueSize ;
    L_ptr += L_valueSize ;
  } // for (L_i ...

  if (L_error == C_ProtocolFrame::E_MSG_OK) {
    *P_size = L_total_size ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::encode_body() end");

  return (L_error);
}


int C_ProtocolBinary::find_header_field_id (char *P_name) {

  int               L_id = -1 ;
  T_IdMap::iterator L_it ;

  GEN_DEBUG(1, "C_ProtocolBinary::find_header_field_id() start");
  L_it = m_header_id_map 
    -> find (T_IdMap::key_type(P_name)) ;
  
  if (L_it != m_header_id_map->end()) {
    L_id = L_it->second ;
  } 

  GEN_DEBUG(1, "C_ProtocolBinary::find_header_field_id() end");

  return (L_id) ;
}

int C_ProtocolBinary::find_header_body_id (char *P_name) {
  int               L_id = -1 ;
  T_IdMap::iterator L_it ;

  GEN_DEBUG(1, "C_ProtocolBinary::find_header_field_id() start");
  L_it = m_header_body_id_map 
    -> find (T_IdMap::key_type(P_name)) ;
  
  if (L_it != m_header_body_id_map->end()) {
    L_id = L_it->second ;
  } 

  GEN_DEBUG(1, "C_ProtocolBinary::find_header_field_id() end");

  return (L_id) ;
}

int C_ProtocolBinary::find_body_value_id (char *P_name) {
  int               L_id = -1 ;
  T_IdMap::iterator L_it ;

  GEN_DEBUG(1, "C_ProtocolBinary::find_header_field_id() start");
  L_it = m_header_body_value_id_map 
    -> find (T_IdMap::key_type(P_name)) ;
  
  if (L_it != m_header_body_value_id_map->end()) {
    L_id = L_it->second ;
  } 

  GEN_DEBUG(1, "C_ProtocolBinary::find_header_field_id() end");

  return (L_id) ;
}

void  C_ProtocolBinary::get_header_values (int          P_id,
				     T_pValueData P_val) {
  
  unsigned long         L_fieldIdx     ;
  T_pHeaderValue        L_val ;

  L_val = &(m_header_value_table[P_id]);

  for(L_fieldIdx=0; L_fieldIdx < m_nb_field_header; L_fieldIdx++) {

    if (L_val -> m_value_setted[L_fieldIdx] == true) {
      P_val[L_fieldIdx] 
	= L_val->m_values [L_val->m_id_value_setted[L_fieldIdx]] ;
    } else {
      P_val[L_fieldIdx].m_type = E_TYPE_NUMBER ;
      P_val[L_fieldIdx].m_value.m_val_number = (unsigned long) 0 ;
    }

  }

}

void C_ProtocolBinary::set_header_values (T_pValueData P_dest, 
                                          T_pValueData P_orig) {

  unsigned long         L_fieldIdx     ;

  for(L_fieldIdx=0; L_fieldIdx < m_nb_field_header; L_fieldIdx++) {
    P_dest[L_fieldIdx] = P_orig [L_fieldIdx] ;
  }
}

void C_ProtocolBinary::set_header_value (int          P_id, 
                                         T_pValueData P_dest, 
                                         T_pValueData P_orig) {

  int                L_type_id ;
  T_pHeaderField     L_fieldVal  ;
  T_TypeType         L_type ;

  L_fieldVal = &(m_header_field_table[P_id]);
  L_type_id = L_fieldVal->m_type_id ;
  if (L_type_id == - 1) {
    L_type = E_TYPE_NUMBER;
  } else {
    L_type = m_type_def_table[L_type_id].m_type ;
  }

  if (L_type == P_orig->m_type) {
    switch (L_type) {
    case E_TYPE_NUMBER:
      P_dest->m_value.m_val_number = P_orig->m_value.m_val_number ;
      break ;

    case E_TYPE_SIGNED:
      P_dest->m_value.m_val_number = P_orig->m_value.m_val_signed ;
      break ;

    case E_TYPE_STRING:
      FREE_TABLE(P_dest->m_value.m_val_binary.m_value);
      P_dest->m_value.m_val_binary.m_size
        = P_orig->m_value.m_val_binary.m_size ;
      ALLOC_TABLE(P_dest->m_value.m_val_binary.m_value,
                  unsigned char *,
                  sizeof(unsigned char),
                  P_orig->m_value.m_val_binary.m_size);
      memcpy(P_dest->m_value.m_val_binary.m_value,
             P_orig->m_value.m_val_binary.m_value,
             P_dest->m_value.m_val_binary.m_size);
      break ;

    case E_TYPE_STRUCT:
      P_dest->m_value.m_val_struct = P_orig->m_value.m_val_struct ;
      break ;

    case E_TYPE_NUMBER_64:
      P_dest->m_value.m_val_number_64 = P_orig->m_value.m_val_number_64 ;
      break ;

    case E_TYPE_SIGNED_64:
      P_dest->m_value.m_val_number_64 = P_orig->m_value.m_val_signed_64 ;
      break ;

    default:
      GEN_FATAL(E_GEN_FATAL_ERROR, "Type value not implemented");
      break ;
    }
  } else {
    GEN_FATAL(E_GEN_FATAL_ERROR,
          "Type ["
          << L_type
          << "] and ["
          << P_orig->m_type
          << "] not compatible for setting for ["
          << L_fieldVal->m_name
          << "]");
  }

}

void C_ProtocolBinary::delete_header_value(T_pValueData P_res) {

  int                L_type_id = 0 ;
  T_pHeaderField     L_fieldVal  ;
  T_TypeType         L_type ;

  GEN_DEBUG(1, "C_ProtocolBinary::delete_header_value() start");
  GEN_DEBUG(1, "C_ProtocolBinary::delete_header_value() Value Id: " 
               << P_res->m_id);


  L_fieldVal = &(m_header_field_table[P_res->m_id]);

  // retrieve type definition of the field
  L_type_id = L_fieldVal->m_type_id ;
  if (L_type_id == - 1) {
    L_type = E_TYPE_NUMBER;
  } else {
    L_type = m_type_def_table[L_type_id].m_type ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::delete_header_value() Type: " 
               << L_type);

  switch (L_type) {
  case E_TYPE_STRING :
    FREE_TABLE(P_res->m_value.m_val_binary.m_value);
    P_res->m_value.m_val_binary.m_size = 0 ;
    break;
  default:
    break ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::delete_header_value() end");
}

void C_ProtocolBinary::set_body_values (int P_nb, 
				  T_pBodyValue P_dest, 
				  T_pBodyValue P_orig) {

  int                L_i, L_id, L_type_id ;
  T_pHeaderBodyValue L_body_fieldValues  ;
  T_TypeType         L_type ;


  for(L_i = 0; L_i < P_nb ; L_i++) {

    L_id = P_orig[L_i].m_id ;
    P_dest[L_i].m_id = L_id ;

    L_body_fieldValues = &m_header_body_value_table[L_id] ;
    L_type_id = L_body_fieldValues->m_type_id ;
    L_type = m_type_def_table[L_type_id].m_type ;
    

    switch (L_type) {
    case E_TYPE_NUMBER:
    case E_TYPE_SIGNED:
    case E_TYPE_NUMBER_64:
    case E_TYPE_SIGNED_64:
      P_dest[L_i].m_value = P_orig[L_i].m_value ;
      break ;
    case E_TYPE_STRING:
      P_dest[L_i].m_value.m_val_binary.m_size 
	= P_orig[L_i].m_value.m_val_binary.m_size ; 
      ALLOC_TABLE(P_dest[L_i].m_value.m_val_binary.m_value,
		  unsigned char *,
		  sizeof(unsigned char),
		  P_dest[L_i].m_value.m_val_binary.m_size);
      memcpy(P_dest[L_i].m_value.m_val_binary.m_value,
	     P_orig[L_i].m_value.m_val_binary.m_value,
	     P_orig[L_i].m_value.m_val_binary.m_size);
      break ;
    case E_TYPE_STRUCT:
      P_dest[L_i].m_value = P_orig[L_i].m_value ;
      break ;
    case E_TYPE_GROUPED:
      P_dest[L_i].m_value.m_val_number = P_orig[L_i].m_value.m_val_number ;
      if (P_orig[L_i].m_value.m_val_number > 0) {
      ALLOC_TABLE(P_dest[L_i].m_sub_val,
		  T_pBodyValue,
		  sizeof(T_BodyValue),
		  P_dest[L_i].m_value.m_val_number);
      set_body_values(P_dest[L_i].m_value.m_val_number,
		      P_dest[L_i].m_sub_val,
		      P_orig[L_i].m_sub_val);
      } else {
        P_dest[L_i].m_sub_val = NULL ;
      }
      break ;
    default:
      GEN_FATAL(E_GEN_FATAL_ERROR, "Unsupported body type value");
      break;
    }

  }
  

}

void C_ProtocolBinary::reset_header_values (int          P_nb, 
				            T_pValueData P_val) {

  int                L_i;

  GEN_DEBUG(1, "C_ProtocolBinary::reset_header_values() start P_nb: " << P_nb);

  for(L_i = 0; L_i < P_nb ; L_i++) {

    GEN_DEBUG(1, "C_ProtocolBinary::reset_header_values() Value Id: " 
                 << P_val[L_i].m_id);

    delete_header_value(&(P_val[L_i]));

  }
  GEN_DEBUG(1, "C_ProtocolBinary::reset_header_values() end");
  
}

void C_ProtocolBinary::reset_body_values (int          P_nb, 
				          T_pBodyValue P_val) {

  int                L_i;

  GEN_DEBUG(1, "C_ProtocolBinary::reset_body_values() start P_nb: " << P_nb);

  for(L_i = 0; L_i < P_nb ; L_i++) {


    GEN_DEBUG(1, "C_ProtocolBinary::reset_body_values() Value Id: " 
                 << P_val[L_i].m_id);

    
    delete_body_value(&(P_val[L_i]));

  }
  GEN_DEBUG(1, "C_ProtocolBinary::reset_body_values() end");
  
}

void C_ProtocolBinary::get_body_values (int P_id, T_pBodyValue P_val, int *P_nbVal) {
  
  T_pBodyValueDef L_body_val_def  ;
  int             L_nb_val, L_idx ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_body_values() start");
  L_body_val_def = &(m_body_value_table[P_id]);
  L_nb_val = L_body_val_def->m_nb_values ;
  L_idx = 0 ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_body_values() L_nb_val [" << L_nb_val << "]");
  while (L_nb_val > 0) {
    set_body_value(&(P_val[L_idx]), &(m_body_value_table[P_id].m_value_table[L_idx]));
    L_idx++ ;
    L_nb_val-- ;
    (*P_nbVal)++ ; // to do: check MAX_BODY_VALUES
  }
  GEN_DEBUG(1, "C_ProtocolBinary::get_body_values() end");
}


iostream_output& C_ProtocolBinary::print_header(iostream_output& P_stream,
					  int           P_headerId,
					  T_pValueData  P_val) {

  unsigned long L_fieldIdx     ;
  char         *L_name         ;

  T_TypeType    L_type         ;
  int           L_type_id      ;

  L_name = (P_headerId == -1) ? 
    (char*)"Unknown" : m_header_value_table[P_headerId].m_name ;

  GEN_DEBUG(1, "C_ProtocolBinary::print_header() start header_name:" << L_name 
              << " (id:" << P_headerId << ")");

  P_stream << L_name << iostream_endl ;

  for(L_fieldIdx=0; L_fieldIdx < m_nb_field_header; L_fieldIdx++) {

    P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) ;
    P_stream << "[" << m_header_field_table[L_fieldIdx].m_name
             << "] = [" ;

    L_type_id = m_header_field_table[L_fieldIdx].m_type_id ;

#ifdef MSG_DETAIL_DUMP
    P_stream << " size [" << m_header_field_table[L_fieldIdx].m_size << "]";
#endif

    if (L_type_id == -1)
    {
      // force the type of header field to number
      L_type = E_TYPE_NUMBER;

#ifdef MSG_DETAIL_DUMP
      P_stream << " type [number]";
#endif
    } else {
      // get the header field type
      L_type = m_type_def_table[L_type_id].m_type ;;

#ifdef MSG_DETAIL_DUMP
      P_stream << " type [" << m_type_def_table[L_type_id].m_type << "]";
#endif
    }

    P_stream << " ";

#ifdef MSG_DETAIL_DUMP
    P_stream << " value [";
#endif

    switch (L_type) {
      case E_TYPE_NUMBER:
      case E_TYPE_SIGNED:
      case E_TYPE_STRUCT:
      case E_TYPE_NUMBER_64:
      case E_TYPE_SIGNED_64:
        P_stream << P_val[L_fieldIdx] ;
	break;
      case E_TYPE_STRING: {

        static char L_hexa_buf [50] ;
        const  size_t L_cNum = 16 ; 
	size_t L_j, L_nb ;
	unsigned char*L_cur ;
	size_t L_buffer_size = P_val[L_fieldIdx].m_value.m_val_binary.m_size;
		        
	L_nb = L_buffer_size / L_cNum ;
	L_cur = P_val[L_fieldIdx].m_value.m_val_binary.m_value ;
	P_stream << " " ;
	if (L_cur != NULL)
	{
	  for (L_j = 0 ; L_j < L_nb; L_j++) {
            P_stream << iostream_endl ;
            P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) ;

	    pretty_binary_buffer(L_cur, L_cNum, L_hexa_buf);
	    P_stream << L_hexa_buf ;
	    L_cur += L_cNum ;
	  }

	  L_nb = L_buffer_size % L_cNum ;
	  if (L_nb != 0) {
            P_stream << iostream_endl ;
            P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) ;

	    pretty_binary_buffer(L_cur, L_nb, L_hexa_buf);
	    P_stream << L_hexa_buf ;
	  }
          P_stream << iostream_endl ;
          P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) ;
        } else {
          P_stream << "NULL(empty)";
	}
      } 
      break ;

      default:
        GEN_FATAL(E_GEN_FATAL_ERROR, "type " << L_type << " not implemented");
        break ;
    }

#ifdef MSG_DETAIL_DUMP
    P_stream << "]";
#endif

    P_stream << "]" << iostream_endl ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::print_header() end:" );

  return (P_stream);
}

iostream_output& C_ProtocolBinary::print_body  (iostream_output&  P_stream, 
						int          P_nb,
						T_pBodyValue P_val,
                                                int          P_level) {

  int                L_i, L_id;
  T_pHeaderBodyValue L_body_fieldValues  ;
  unsigned long      L_fieldIdx     ;
  int                L_valueIdx ;

  int                L_type_id ;
  T_TypeType         L_type ;
  char               L_levelStr[255] = ""; // TEMP

  GEN_DEBUG(1, "C_ProtocolBinary::print_body() start nb: " << P_nb);
  GEN_DEBUG(1, "C_ProtocolBinary::print_body() m_nb_field_header_body: " 
		  << m_nb_field_header_body);
  GEN_DEBUG(1, "C_ProtocolBinary::print_body() Level: " << P_level);

  for (L_i = 0; L_i < P_level; L_i++) {
    strcat(L_levelStr," |");
  }
  for (L_i = 0; L_i < P_nb; L_i++) {
    L_id = P_val[L_i].m_id ;
    L_body_fieldValues = &m_header_body_value_table[L_id] ;

    GEN_DEBUG(1, "C_ProtocolBinary::print_body() display " <<
        L_body_fieldValues->m_name << "(id: " << L_id << ")");

    P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) << L_levelStr 
             << " ";
    P_stream << "[" << L_body_fieldValues->m_name << iostream_endl ; 

    // print field setted
    P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) << L_levelStr
             << " " ;

    // Display name
    for(L_fieldIdx=0; L_fieldIdx < m_nb_field_header_body; L_fieldIdx++) {
      
      if (L_body_fieldValues->m_value_setted[L_fieldIdx] == true) {
        L_valueIdx = L_body_fieldValues
          ->m_id_value_setted[L_fieldIdx];
        
        P_stream << "(" 
                 << m_header_body_field_table[L_fieldIdx].m_name 
                 << " = "
                 << L_body_fieldValues
          ->m_values[L_valueIdx].m_value.m_val_number
                 << ")" ;
      }
    } // for
    
    
    // Display optional part
    if (m_header_body_start_optional_id != -1) {
      for(L_fieldIdx=m_header_body_start_optional_id; 
          L_fieldIdx < m_max_nb_field_header_body; L_fieldIdx++) {
        
        if (L_body_fieldValues->m_value_setted[L_fieldIdx] == true) {
          
          L_valueIdx = L_body_fieldValues
            ->m_id_value_setted[L_fieldIdx];
          P_stream << "(" 
                   << m_header_body_field_table[L_fieldIdx].m_name 
                   << " = "
		   << L_body_fieldValues
            ->m_values[L_valueIdx].m_value.m_val_number
                   << ")" ;
        }
      } // for
      
    }
  
    P_stream << "] = [" ;

    L_type_id = L_body_fieldValues->m_type_id ;

    GEN_DEBUG(1, "C_ProtocolBinary::print_body() display type " <<
        m_type_def_table[L_type_id].m_name << "(id: " << L_type_id << ")");

    L_type = m_type_def_table[L_type_id].m_type ;

    switch (L_type) {
      case E_TYPE_NUMBER:
#ifdef MSG_DETAIL_DUMP
        P_stream << " size [" << sizeof(T_UnsignedInteger32) << "]";
        P_stream << " type [" << m_type_def_table[L_type_id].m_name << "]";
        P_stream << " value [";
#endif
        P_stream << P_val[L_i].m_value.m_val_number ;
	break;
      case E_TYPE_SIGNED:
#ifdef MSG_DETAIL_DUMP
        P_stream << " size [" << sizeof(T_Integer32) << "]";
        P_stream << " type [" << m_type_def_table[L_type_id].m_name << "]";
        P_stream << " value [";
#endif
        P_stream << P_val[L_i].m_value.m_val_signed ;
	break;
      case E_TYPE_STRING: {
        static char L_hexa_buf [50] ;
        const size_t L_cNum = 16 ;
        size_t L_i2, L_nb, L_buffer_size ;
        unsigned char*L_cur ;

#ifdef MSG_DETAIL_DUMP
        P_stream << " size [" 
                 << P_val[L_i].m_value.m_val_binary.m_size
                 << "]";
        P_stream << " type [" << m_type_def_table[L_type_id].m_name << "]";
        P_stream << " value [";
#endif
        L_buffer_size = P_val[L_i].m_value.m_val_binary.m_size ;

        L_nb = L_buffer_size / L_cNum ;
        L_cur = P_val[L_i].m_value.m_val_binary.m_value ;
        if (L_cur != NULL)
        {
          for (L_i2 = 0 ; L_i2 < L_nb; L_i2++) {
            P_stream << iostream_endl ;
            P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) 
                     << L_levelStr << " ";

            pretty_binary_buffer(L_cur, L_cNum, L_hexa_buf);
            P_stream << L_hexa_buf ;
            L_cur += L_cNum ;
          }
          L_nb = L_buffer_size % L_cNum ;
          if (L_nb != 0) {
            P_stream << iostream_endl ;
            P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) 
                     << L_levelStr << " ";

            pretty_binary_buffer(L_cur, L_nb, L_hexa_buf);
            P_stream << L_hexa_buf ;
          }
          P_stream << iostream_endl ;
          P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) 
                   << L_levelStr << " ";
        } else {
          P_stream << "NULL(empty)";
        }
        }
	break;
      case E_TYPE_STRUCT:
#ifdef MSG_DETAIL_DUMP
        P_stream << " size [" << (2*sizeof(T_Integer32)) << "]";
        P_stream << " type [" << m_type_def_table[L_type_id].m_name << "]";
        P_stream << " value [";
#endif
        P_stream  << P_val[L_i].m_value.m_val_struct.m_id_1 << ";" 
	      << P_val[L_i].m_value.m_val_struct.m_id_2 ;
	break;
      case E_TYPE_GROUPED:
#ifdef MSG_DETAIL_DUMP
        P_stream << " grouped";
        P_stream << " type [" << m_type_def_table[L_type_id].m_name << "]";
        P_stream << " value [";
#endif
        P_stream << iostream_endl ;
        print_body(P_stream, P_val[L_i].m_value.m_val_number, 
		   P_val[L_i].m_sub_val, P_level+1);
        P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) 
                 << L_levelStr << " ";
	break;
      case E_TYPE_NUMBER_64:
#ifdef MSG_DETAIL_DUMP
        P_stream << " size [" << sizeof(T_UnsignedInteger64) << "]";
        P_stream << " type [" << m_type_def_table[L_type_id].m_name << "]";
        P_stream << " value [";
#endif
        P_stream << P_val[L_i].m_value.m_val_number_64 ;
	break;
      case E_TYPE_SIGNED_64:
#ifdef MSG_DETAIL_DUMP
        P_stream << " size [" << sizeof(T_UnsignedInteger64) << "]";
        P_stream << " type [" << m_type_def_table[L_type_id].m_name << "]";
        P_stream << " value [";
#endif
        P_stream << P_val[L_i].m_value.m_val_signed_64 ;
	break;
      default:
        GEN_FATAL(E_GEN_FATAL_ERROR, "type " << L_type << " not implemented");
        break ;
    }

#ifdef MSG_DETAIL_DUMP
    P_stream << "]" << iostream_endl ;
    P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) 
             << L_levelStr << " ";
#endif

    P_stream << "]" << iostream_endl ;

  }

  GEN_DEBUG(1, "C_ProtocolBinary::print_body() end");

  return (P_stream) ;

}

void C_ProtocolBinary::get_body_value (T_pValueData P_dest, T_pBodyValue P_orig) {
  
  int L_id, L_type_id ;
  T_pHeaderBodyValue L_body_fieldValues  ;
  T_TypeType         L_type ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_body_value() ValData BodyVal start");

  L_id = P_orig -> m_id ;
  L_body_fieldValues = &m_header_body_value_table[L_id] ;
  L_type_id = L_body_fieldValues->m_type_id ;
  L_type = m_type_def_table[L_type_id].m_type ;

  P_dest->m_id = L_id ;
  P_dest->m_type = L_type ;

  switch (L_type) {
  case E_TYPE_NUMBER:
    P_dest->m_value.m_val_number = P_orig->m_value.m_val_number ;
    break ;
  case E_TYPE_SIGNED:
    P_dest->m_value.m_val_number = P_orig->m_value.m_val_signed ;
    break ;
  case E_TYPE_STRING:
    P_dest->m_value.m_val_binary.m_size = P_orig->m_value.m_val_binary.m_size ;
    ALLOC_TABLE(P_dest->m_value.m_val_binary.m_value,
		unsigned char *,
		sizeof(unsigned char),
		P_dest->m_value.m_val_binary.m_size);
    memcpy(P_dest->m_value.m_val_binary.m_value,
	   P_orig->m_value.m_val_binary.m_value,
	   P_dest->m_value.m_val_binary.m_size);
    break ;

  case E_TYPE_STRUCT:
    P_dest->m_value.m_val_struct = P_orig->m_value.m_val_struct ;
    break ;

  case E_TYPE_NUMBER_64:
    P_dest->m_value.m_val_number_64 = P_orig->m_value.m_val_number_64 ;
    break ;
  case E_TYPE_SIGNED_64:
    P_dest->m_value.m_val_number_64 = P_orig->m_value.m_val_signed_64 ;
    break ;

  default:
    GEN_FATAL(E_GEN_FATAL_ERROR, "Type value not implemented");
    break ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::get_body_value() ValData BodyVal end");
}

void C_ProtocolBinary::set_body_value (T_pBodyValue P_dest, T_pValueData P_orig) {

  int L_id, L_type_id ;
  T_pHeaderBodyValue L_body_fieldValues  ;
  T_TypeType         L_type ;

  L_id = P_dest -> m_id ;
  L_body_fieldValues = &m_header_body_value_table[L_id] ;
  L_type_id = L_body_fieldValues->m_type_id ;
  L_type = m_type_def_table[L_type_id].m_type ;

  if (L_type == P_orig->m_type) {
    switch (L_type) {
    case E_TYPE_NUMBER:
      P_dest->m_value.m_val_number = P_orig->m_value.m_val_number ;
      break ;

    case E_TYPE_SIGNED:
      P_dest->m_value.m_val_number = P_orig->m_value.m_val_signed ;
      break ;

    case E_TYPE_STRING:
      FREE_TABLE(P_dest->m_value.m_val_binary.m_value);
      P_dest->m_value.m_val_binary.m_size 
	= P_orig->m_value.m_val_binary.m_size ;
      ALLOC_TABLE(P_dest->m_value.m_val_binary.m_value,
		  unsigned char *,
		  sizeof(unsigned char),
		  P_orig->m_value.m_val_binary.m_size);
      memcpy(P_dest->m_value.m_val_binary.m_value,
	     P_orig->m_value.m_val_binary.m_value,
	     P_dest->m_value.m_val_binary.m_size);
      break ;

    case E_TYPE_STRUCT:
      P_dest->m_value.m_val_struct = P_orig->m_value.m_val_struct ;
      break ;

    case E_TYPE_NUMBER_64:
      P_dest->m_value.m_val_number_64 = P_orig->m_value.m_val_number_64 ;
      break ;

    case E_TYPE_SIGNED_64:
      P_dest->m_value.m_val_number_64 = P_orig->m_value.m_val_signed_64 ;
      break ;

    default:
      GEN_FATAL(E_GEN_FATAL_ERROR, "Type value not implemented");
      break ;
    }
  } else {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Type ["
	  << L_type
	  << "] and ["
	  << P_orig->m_type
	  << "] not compatible for setting for ["
	  << L_body_fieldValues->m_name
	  << "]");
  }
  
}

bool C_ProtocolBinary::check_presence_needed (T_pCondPresence P_condition,
					unsigned long  *P_values) {
  bool L_ret ;

  L_ret = (P_values[P_condition->m_f_id] & (P_condition->m_mask)) ;

  return (L_ret);
}

T_TypeType C_ProtocolBinary::get_header_value_type (int P_id) {

  T_TypeType L_ret = E_TYPE_NUMBER ;
  int                L_type_id ;

  T_pHeaderField        L_fieldDescr   ;

  L_fieldDescr = &m_header_field_table[P_id] ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_value_type() "
                          << "Name \"" << L_fieldDescr->m_name
                          << "\" (P_id:" << P_id << ")");
  
  L_type_id = L_fieldDescr->m_type_id ;


  if (L_type_id != -1)
  {
    L_ret = m_type_def_table[L_type_id].m_type ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_value_type() "
                          << "Type is \"" << L_ret
                          << "\" (Type Id:" << L_type_id << ")" );

  return (L_ret);
}


T_TypeType C_ProtocolBinary::get_body_value_type (int P_id) {

  T_TypeType L_ret = E_UNSUPPORTED_TYPE ;
  T_pHeaderBodyValue L_headerBodyValue ;
  int                L_type_id ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_body_value_type() "
                << "Name \"" << m_header_body_value_table[P_id].m_name //m_header_body_field_table[P_id].m_name
                << "\" (P_id: " << P_id << ")");

  L_headerBodyValue = &m_header_body_value_table[P_id];
  L_type_id = L_headerBodyValue->m_type_id ;
  L_ret = m_type_def_table[L_type_id].m_type ;
  
  GEN_DEBUG(1, "C_ProtocolBinary::get_body_value_type() "
                          << "Type is \"" << L_ret
                          << "\" (Type Id is " << L_type_id << ")");

  return (L_ret);
}

C_ProtocolBinary::T_MsgIdType C_ProtocolBinary::get_msg_id_type () {
  return(m_msg_id_type) ;
}

T_TypeType C_ProtocolBinary::get_msg_id_value_type () {
  return(m_msg_id_value_type) ;
}

int C_ProtocolBinary::get_msg_id () {
  return(m_msg_id_id);
}

C_ProtocolBinary::T_MsgIdType C_ProtocolBinary::get_out_of_session_id_type () {
  return(m_type_id_out_of_session) ;
}

int C_ProtocolBinary::get_out_of_session_id () {
  return(m_id_out_of_session);
}

void C_ProtocolBinary::reset_value_data (T_pValueData P_val) {

  switch (P_val->m_type) {
  case E_TYPE_STRING:
    if (P_val->m_value.m_val_binary.m_size) {
      FREE_TABLE(P_val->m_value.m_val_binary.m_value);
    }
    break ;
  case E_TYPE_GROUPED:
    GEN_FATAL(E_GEN_FATAL_ERROR, "reset grouped value not implemented");
    break ;
  default:
    break;
  }
  P_val-> m_type = E_TYPE_NUMBER ;
  P_val-> m_value.m_val_number = 0 ;
}


void C_ProtocolBinary::convert_to_string (T_pValueData P_res, 
				    T_pValueData P_val) {

  char   L_tmp_val[50] ;

  L_tmp_val[0] = '\0' ;

  switch (P_val->m_type) {
  case E_TYPE_NUMBER:
    sprintf(L_tmp_val, "%ld", P_val->m_value.m_val_number);
    P_res->m_type = E_TYPE_STRING ;
    P_res->m_value.m_val_binary.m_size = strlen (L_tmp_val) ;
    ALLOC_TABLE(P_res->m_value.m_val_binary.m_value,
		unsigned char*,
		sizeof(unsigned char),
		P_res->m_value.m_val_binary.m_size);
    memcpy(P_res->m_value.m_val_binary.m_value,
	   L_tmp_val,
	   P_res->m_value.m_val_binary.m_size);
    break ;
  default:
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "convert to string not implemented for this type");
    break;
  }
  P_val-> m_type = E_TYPE_NUMBER ;
  P_val-> m_value.m_val_number = 0 ;
}

C_ProtocolBinary::T_pHeaderBodyValue 
C_ProtocolBinary::get_header_body_value_description (int P_id) {
  C_ProtocolBinary::T_pHeaderBodyValue L_value ;
  L_value = (P_id <(int) m_nb_header_body_values) ?
    &m_header_body_value_table[P_id] : NULL ;
  return (L_value) ;
}

C_ProtocolBinary::T_pHeaderValue 
C_ProtocolBinary::get_header_value_description (int P_id) {
  C_ProtocolBinary::T_pHeaderValue L_result ;

  L_result = (P_id < (int)m_nb_field_header_body) ?
    &m_header_value_table[P_id] : NULL ;

  return (L_result);
}

// C_ProtocolBinaryFrame::T_pMsgError
C_MessageFrame* C_ProtocolBinary::decode_message(unsigned char *P_buffer, 
						 size_t        *P_size, 
						 C_ProtocolFrame::T_pMsgError P_error) {
  C_MessageBinary *L_msg ;

  size_t           L_size = *P_size ;

  //  NEW_VAR(L_msg, C_MessageBinary(this));
  L_msg=(C_MessageBinary*)create_new_message(NULL);

  (*P_size) = L_msg -> decode(P_buffer, L_size, P_error);
  // C_ProtocolBinaryFrame::E_MSG_OK:

  switch (*P_error) {
  case C_ProtocolFrame::E_MSG_OK:
    // GEN_LOG_EVENT(LOG_LEVEL_MSG, 
    //	  "Received [" << *L_msg << "]");
    break ;
  case C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS:
    DELETE_VAR(L_msg);
    break ;
  default:
    GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Unrecognized message received") ;
    DELETE_VAR(L_msg);
    break ;
  }

  return (L_msg);
}

C_ProtocolFrame::T_MsgError 
C_ProtocolBinary::encode_message(C_MessageFrame *P_msg,
				 unsigned char  *P_buffer,
				 size_t         *P_buffer_size) {

  C_ProtocolFrame::T_MsgError  L_error =  C_ProtocolFrame::E_MSG_OK;
  C_MessageBinary             *L_msg   = (C_MessageBinary*) P_msg ;


  L_msg -> encode (P_buffer, P_buffer_size, &L_error) ;

  if (L_error == C_ProtocolFrame::E_MSG_OK) {
    GEN_LOG_EVENT(LOG_LEVEL_MSG, 
                  "Send [" << *L_msg << "]");
  } else {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                  "Error while sending [" << *L_msg << "]");
  }

  return (L_error);
}

C_MessageFrame* C_ProtocolBinary::create_new_message(C_MessageFrame *P_msg) {

  C_MessageBinary *L_msg ;

  GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() start: msg pt");
  NEW_VAR(L_msg, C_MessageBinary(this));
  if (P_msg != NULL) {
    (*L_msg) = *((C_MessageBinary*)P_msg) ;
  }
  GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() end: msg pt");
  return (L_msg);
}

C_MessageFrame* C_ProtocolBinary::create_new_message(void                *P_xml, 
		                                     T_pInstanceDataList  P_list,
						     int                 *P_nb_value) {


  C_XmlData *L_Xml = (C_XmlData*) P_xml ;

  bool                            L_msgOk = true ;

  C_MessageBinary                 *L_msg ;
  char                            *L_currentName ;
  
  //   C_ProtocolBinaryFrame::T_MsgError L_decode_result ;
  C_ProtocolFrame::T_MsgError L_decode_result ;
  
  unsigned char            *L_buf ;
  size_t                    L_size ;
  
  int                       L_header_val_id ;
  int                       L_body_val_id   ;
  
  GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() start: xml");
  
  // Allocate a new message
  NEW_VAR(L_msg, C_MessageBinary(this));

  // two cases: direct buffer to be decoded or
  //            direct message definition

  // Get the message name from XML definition
  L_currentName = L_Xml -> get_name () ;

  GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() msg xml name is "
	    << L_currentName );
      
  if (strcmp(L_currentName, (char*)"CDATA") == 0) {
	
    // direct buffer data definition
    // unsigned long L_ret_decode ;
    L_currentName = L_Xml->find_value((char*)"value");
    
    L_buf = convert_hexa_char_to_bin(L_currentName, &L_size);
    (void) L_msg -> decode(L_buf, L_size, &L_decode_result);
    
    //    if (L_decode_result != C_ProtocolBinaryFrame::E_MSG_OK) 
    if (L_decode_result != C_ProtocolFrame::E_MSG_OK) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
		"Error while decoding direct data buffer ["
		<< L_currentName << "]");
      L_msgOk = false ;
    }
	
  } else if (strcmp(L_currentName, get_header_name()) == 0) {
	
    // message decoding
    L_currentName = L_Xml -> find_value ((char*) "name") ;
    if (L_currentName == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
		"name value is mandatory for ["
		<< get_header_name()
		<< "]");
      L_msgOk = false ;
    } else {
      GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() " 
		<< "the name of this message is " << L_currentName );

      L_header_val_id = get_header_value_id(L_currentName) ;

      GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() " 
		<< "L_header_val_id = " << L_header_val_id );

      if (L_header_val_id == -1) {
	GEN_ERROR(E_GEN_FATAL_ERROR,
		  "unknown name ["
		  << L_currentName << "] for [" << get_header_name() << "]");
	L_msgOk = false ;

      } else { 

        // now search for body values
	T_pXmlData_List           L_listBodyVal ;
	T_XmlData_List::iterator  L_bodyValIt ;
	C_XmlData                *L_bodyData ;
	// int                       L_nbBodyVal ;
	char                     *L_bodyName, *L_bodyValue ;
	C_ProtocolBinary::T_BodyValue   L_bodyVal ;
        T_TypeType                L_type ; 

	L_msg->set_header_id_value(L_header_val_id);
	
	L_listBodyVal = L_Xml -> get_sub_data() ;
	if (L_listBodyVal != NULL) {
	  // L_nbBodyVal = L_listBodyVal->size() ;
	  for (L_bodyValIt  = L_listBodyVal->begin() ;
	       L_bodyValIt != L_listBodyVal->end() ;
	       L_bodyValIt++) {
	    L_bodyData = *L_bodyValIt ;
	    
	    // GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() " << 
		      // "L_bodyData name is [" << L_bodyData->get_name() << "]");
	    // GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() " << 
		      // "get_header_body_name() is [" 
		      // << get_header_body_name() << "]");
	      
	    if (strcmp(L_bodyData->get_name(), 
		       get_header_body_name())==0) {
	      
	      L_bodyName = L_bodyData->find_value((char*)"name");
	      if (L_bodyName == NULL) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "name value is mandatory for ["
			  << get_header_body_name()
			  << "] definition");
		L_msgOk = false ;
	      }
		
	      GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() "
			<< "L_bodyName is    [" << L_bodyName << "]");

	      L_body_val_id = get_header_body_value_id(L_bodyName) ;
	      if (L_body_val_id == -1) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "No definition found for ["
			  << L_bodyName << "]");
		L_msgOk = false ;
		break ;
	      } 
		
	      GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() " 
			<< "L_body_val_id is " 
			<< L_body_val_id );
	      
              L_type = get_body_value_type (L_body_val_id) ;
	      if (L_type 
		  == E_TYPE_GROUPED) {

	        GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() " 
			     << L_bodyName << " is  grouped element" );


                if (process_grouped_type(L_bodyData, L_body_val_id, 
				         &L_bodyVal) != 0) {
		  GEN_ERROR(E_GEN_FATAL_ERROR,
		            "Grouped Type processing Error for "
		            << L_bodyName);
		  L_msgOk = false ;
		  break ;
		}


                // Add the grouped value in message
		L_msg->set_body_value(&L_bodyVal);

	        // Now do not forget to clean L_bodyVal
                reset_grouped_body_value(&L_bodyVal);
                FREE_TABLE(L_bodyVal.m_sub_val);
	      } else { 
		bool L_toBeDelete = false;

                // not grouped value
		L_bodyValue = L_bodyData->find_value((char*)"value");
		if (L_bodyValue == NULL) {
		  GEN_ERROR(E_GEN_FATAL_ERROR,
			    "value is mandatory for ["
			    << get_header_body_name()
			    << "] definition");
		  L_msgOk = false ;
		  break ;
		}
		
		GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() " 
			  << "L_bodyValue is \"" << L_bodyValue << "\"" );
		
		if (set_body_value(L_body_val_id, 
				   L_bodyValue,
				   1,
				   &L_bodyVal,
				   &L_toBeDelete) == 0) {

                  if (m_header_body_field_separator == NULL) {
                    L_msg->set_body_value(&L_bodyVal);
                  } else {
                    T_ValueData L_tmp_value ;
                    bool        L_exist     ;
                    L_tmp_value.m_value = L_bodyVal.m_value ;
                    L_tmp_value.m_id = L_bodyVal.m_id ;
                    L_tmp_value.m_type = L_type;
                    L_exist = L_msg->set_body_value(L_body_val_id,&L_tmp_value);
                    if (L_exist == false) {
                      L_msg->set_body_value(&L_bodyVal);
                    }
                  }

		  if (L_toBeDelete) {
	            // Now do not forget to clean L_bodyVal
                   delete_body_value(&L_bodyVal);
		  }

		} else {
		  GEN_ERROR(E_GEN_FATAL_ERROR,
			    "Bad format for ["
			    << L_bodyValue << "]");
		  L_msgOk = false ;
		  break ;
		}
	      } // end to else not grouped
            } else if (strcmp(L_bodyData->get_name(),
                              (char*)"setfield")==0) {
              unsigned long   L_val_setfield = 0 ;
              int             L_id_setfield   = 0 ;
	      L_msgOk = (analyze_setfield(L_bodyData, &L_id_setfield ,&L_val_setfield) == -1) 
                ? false : true  ;
              if (L_msgOk) {
                L_msg->set_header_value(L_id_setfield, L_val_setfield);
              }
              
	    } else {
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
			"Unkown section ["
			<< L_bodyData->get_name()
			<< "]");
	      L_msgOk = false;
	      break ;
	    }
	    if (L_msgOk == false) break ;
	  } // for (L_bodyValIt
	} // if (L_listBodyVal != NULL)
      }
    }
  }

  // TO-REMOVE
  if (L_msgOk == false) {
    DELETE_VAR(L_msg);
  } else {
    // Store the description message in list
    m_messageList->push_back(L_msg);
  }

  GEN_DEBUG(1, "C_ProtocolBinary::create_new_message() end: xml");

  return (L_msg);
}

void C_ProtocolBinary::log_buffer(char* P_header, 
                                  unsigned char *P_buffer, 
                                  size_t P_buffer_size) {
  if (genTraceLevel & gen_mask_table[LOG_LEVEL_BUF]) {
    static char L_hexa_buf [50] ;
    const  size_t L_cNum = 16 ;
    size_t L_i, L_nb ;
    unsigned char*L_cur ;
    size_t L_buffer_size = P_buffer_size ;
    
    GEN_LOG_EVENT(LOG_LEVEL_BUF,
		  "Buffer " << P_header << " size ["
		  << L_buffer_size
		  << "]");
    L_nb = L_buffer_size / L_cNum ;
    L_cur = P_buffer ;
    for (L_i = 0 ; L_i < L_nb; L_i++) {
      pretty_binary_buffer(L_cur, L_cNum, L_hexa_buf);
      GEN_LOG_EVENT_NO_DATE(LOG_LEVEL_BUF, 
			    L_hexa_buf) ;
      L_cur += L_cNum ;
    }
    L_nb = L_buffer_size % L_cNum ;
    if (L_nb != 0) {
      pretty_binary_buffer(L_cur, L_nb, L_hexa_buf);
      GEN_LOG_EVENT_NO_DATE(LOG_LEVEL_BUF, 
 				L_hexa_buf) ;
    }
  }
}


char* C_ProtocolBinary::message_name() {
  return (get_header_name());
}

char* C_ProtocolBinary::message_component_name() {
  return (get_header_body_name ());
}

T_pNameAndIdList C_ProtocolBinary::message_name_list     () {

  T_NameAndId                  L_elt                     ;
  size_t                          L_i                       ;

  for (L_i = 0 ; L_i < m_nb_header_values ; L_i ++) {
    
    ALLOC_TABLE(L_elt.m_name,
    		char*, sizeof(char),
    		(strlen(m_header_value_table[L_i].m_name)+1));

    strcpy(L_elt.m_name,m_header_value_table[L_i].m_name) ;
    L_elt.m_id = m_header_value_table[L_i].m_id ;

    m_message_name_list->push_back(L_elt);    

  }

  return (m_message_name_list);
  
}

T_pNameAndIdList C_ProtocolBinary::message_component_name_list    () {

  T_NameAndId                  L_elt                     ;
  size_t                          L_i                       ;

  for (L_i = 0 ; L_i < m_nb_header_body_values ; L_i ++) {
    
    ALLOC_TABLE(L_elt.m_name,
    		char*, sizeof(char),
    		(strlen(m_header_body_value_table[L_i].m_name)+1));
    
    strcpy(L_elt.m_name,m_header_body_value_table[L_i].m_name) ;
    L_elt.m_id = m_header_body_value_table[L_i].m_id ;
    
    m_message_comp_name_list->push_back(L_elt);
    
  }
  
  return (m_message_comp_name_list) ;
  
  
}


char* C_ProtocolBinary::message_name(int P_headerId) {
  if (P_headerId == -1) {
    return (get_header_name());
  } else {
    return (m_header_value_table[P_headerId].m_name);
  }
}

// field management
int C_ProtocolBinary::find_field(char *P_name) {
  int L_id ;

  L_id = find_header_field_id (P_name);
  if (L_id == -1) {
    L_id = find_body_value_id(P_name);
    if (L_id != -1) {
      L_id += m_max_nb_field_header ;
    }
  }
  return (L_id);
}

T_TypeType C_ProtocolBinary::get_field_type (int P_id,
					     int P_sub_id) {
  int        L_id = P_id ;
  T_TypeType L_type = E_TYPE_NUMBER ;
  
  GEN_DEBUG(1, "C_ProtocolBinary::get_field_type() "
                          << "P_id is " << P_id 
                          << " (max header is " << m_max_nb_field_header
                          << ") P_sub_id is " << P_sub_id );

  if (L_id >= (int)m_max_nb_field_header) {
    L_id -= m_max_nb_field_header ;
    L_type = get_body_value_type(L_id);
  } else {
    L_type = get_header_value_type(L_id);
  }
  return (L_type);
}

int C_ProtocolBinary::retrieve_field_id(int P_id, T_pMsgIdType P_type) {
  int L_id = P_id ;
  if (L_id < (int)m_max_nb_field_header) {
    *P_type = E_MSG_ID_HEADER ;
  } else {
    L_id -= m_max_nb_field_header ;

    *P_type = E_MSG_ID_BODY ;
  }
  return (L_id);
}

bool C_ProtocolBinary::check_sub_entity_needed (int P_id) {
  return (false);
}

unsigned long  C_ProtocolBinary::get_m_max_nb_field_header () {
  return (m_max_nb_field_header);
}

int C_ProtocolBinary::process_grouped_type(C_XmlData *P_bodyData,
	                                   int P_body_grouped_val_id,
					   T_pBodyValue P_pBodyVal) {
  int L_nRet = 0;

  T_pXmlData_List           L_listGroupedVal ;
  T_XmlData_List::iterator  L_groupedValIt ;
  C_XmlData                *L_groupedVal ;
  int                       L_nb_groupedVal ;
  char                     *L_groupedValName, *L_groupedValValue ;
  int                       L_groupedValId, L_groupedValIndex;
		  
  GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() start");
  GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() Body Value id: " 
			     << P_body_grouped_val_id );
//  GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() Body Value pt  : " 
//			     << P_pBodyVal );



  // Retrieve the grouped value list
  L_listGroupedVal = P_bodyData -> get_sub_data() ;

  if (L_listGroupedVal != NULL) {
    L_nb_groupedVal = L_listGroupedVal->size() ;
		  
    GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() " 
			<< "Number of element in grouped type is " 
			<< L_nb_groupedVal );

    // Add the grouped type in the body value structure
    L_nRet = set_body_value(P_body_grouped_val_id,
		   NULL,
		   L_nb_groupedVal,
		   P_pBodyVal);
		  
    if (L_nRet == 0)
    {
      L_groupedValIndex = 0 ;

      for (L_groupedValIt = L_listGroupedVal->begin() ;
	   L_groupedValIt != L_listGroupedVal->end() ;
	   L_groupedValIt++) {

        // Get the sub value description
        L_groupedVal = *L_groupedValIt ;
		    
        // GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() " 
		     // << "Grouped Value [" << L_groupedValIndex 
                     // << "] get header field name [" << L_groupedVal->get_name()
                     // << "] to be compare with ["<< get_header_body_name() << "]");
	      
        // Check component header tag
        if (strcmp(L_groupedVal->get_name(), 
	           get_header_body_name())==0) {
		      
          // retrieve the name, the id and value
          // of the field
          L_groupedValName = 
		  L_groupedVal -> find_value ((char*)"name");

	  L_groupedValValue = 
		  L_groupedVal -> find_value ((char*)"value");

	  if (L_groupedValName == NULL) {
	    GEN_ERROR(E_GEN_FATAL_ERROR, 
				  "name value mandatory for header field "
				  << L_groupedVal->get_name());
	    L_nRet = 3 ;
	    break ;
	  }

	  L_groupedValId = 
		  get_header_body_value_id(L_groupedValName) ;

	  if (L_groupedValId == -1) {
	    GEN_ERROR(E_GEN_FATAL_ERROR,
		    "No definition found for field "
		    << L_groupedValName);
	    L_nRet = 4 ;
	    break ;
	  } ;

	  GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() " 
	             << "Grouped Value [" << L_groupedValIndex 
                     << "] " << L_groupedVal->get_name() 
                     << " name [" << L_groupedValName 
                     << "] with id: "<< L_groupedValId);

			
	  // Special grouped type case
          if (get_body_value_type (L_groupedValId) 
	         == E_TYPE_GROUPED) {

	    GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() " 
	             << "L_groupedVal [" << L_groupedValIndex 
                     << "] name " << L_groupedValName 
                     << " is a grouped type");

	    // Now try to recursive call
	    if (process_grouped_type(L_groupedVal, L_groupedValId, 
				     &(P_pBodyVal -> m_sub_val[L_groupedValIndex])) != 0) {
	        GEN_ERROR(E_GEN_FATAL_ERROR, 
	              "Recursive Grouped error");
	        GEN_ERROR(E_GEN_FATAL_ERROR,
		        "Grouped Type processing Error for "
		         << L_groupedValName);
	       L_nRet = 99 ;
	        break ;
	    }
          } else {

            // Single value processing case
	    // then a value must be found in message definition
	    if (L_groupedValValue == NULL) {
	      GEN_ERROR(E_GEN_FATAL_ERROR, 
		        "value mandatory for field "
		        << L_groupedValName);
	      L_nRet = 5 ;
	      break ;
            }

	    // Now add the sub value
	    if (set_body_sub_value(L_groupedValIndex,
				   L_groupedValId,
				   L_groupedValValue,
				   P_pBodyVal) != 0) {
              GEN_ERROR(E_GEN_FATAL_ERROR,
		        "Bad format for ["
		        << L_groupedValValue << "] for field "
		        << L_groupedValName);
	      L_nRet = 6 ;
	      break ;
	    }

	  }
        } else {
          GEN_ERROR(E_GEN_FATAL_ERROR, 
		     "Unknown header field section [" 
		     << L_groupedVal->get_name()
		     << "]" );
		    L_nRet = 2 ;
		    break ;
        }
        L_groupedValIndex++ ;
      }
    }
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "Grouped value with no value");
    L_nRet = 1 ;
  }

  GEN_DEBUG(1, "C_ProtocolBinary::process_grouped_type() end return: " 
			     << L_nRet );
  return L_nRet;
}

bool C_ProtocolBinary::get_complex_header_presence() {
  return m_header_complex_type_presence;
}


void C_ProtocolBinary::update_stats (T_ProtocolStatDataType   P_type,
				       T_ProtocolStatDataAction P_action,
				       int                      P_id) {
}

int C_ProtocolBinary::analyze_sessions_id_from_xml (C_XmlData *P_def) {


  int                       L_ret = 0                      ;
  char                     *L_session_id_name = NULL       ;
  char                     *L_outof_session_id_name = NULL ;
  C_XmlData                *L_data                         ;

  char                     *L_session_id_position_name = NULL ;
  char                     *L_endstr           ;


  L_data = P_def ;
  // header values dictionnary definitions
  L_session_id_name = L_data->find_value ((char*)"session-id") ;
  
  GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() L_session_id_name is " << 
            L_session_id_name);
  
  if (L_session_id_name == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "session-id value is mandatory for ["
              << m_header_name << "] section");
    L_ret = -1 ;
  } 

  if (L_ret != -1) {
    L_outof_session_id_name 
      = L_data->find_value ((char*)"out-of-session-id") ;
    if (L_outof_session_id_name == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
                "out-of-session-id value is mandatory for ["
                << m_header_name << "] section");
      L_ret = -1 ;
    } 
  }

  if (L_ret != -1) {
    L_session_id_position_name 
      = L_data->find_value ((char*)"session-id-position") ;
    if (L_session_id_position_name != NULL) {
      m_session_id_position = (int)strtoul_f (L_session_id_position_name, &L_endstr, 10);
      if (L_endstr[0] != '\0') {
        GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                  << m_session_id_position << "] not a number");
        L_ret = -1 ;
      }
    }
  }
  
  if (L_ret != -1) {
    GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
	      << "Session Id name is " << L_session_id_name );

    // retrieve informations for session_id
    if (L_session_id_name) {
      m_msg_id_id = find_header_field_id (L_session_id_name) ;
      
      GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                << "m_msg_id_id is " << m_msg_id_id );
      
      if (m_msg_id_id != -1) {
        GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                  << "m_msg_id_id is an header one it type is number");
        m_msg_id_type = E_MSG_ID_HEADER ;
        m_msg_id_value_type = E_TYPE_NUMBER ;
      } else {
        GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                  << "m_msg_id_id is an body one ");
        m_msg_id_id = find_body_value_id (L_session_id_name) ;

        GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                  << "m_msg_id_id is " << m_msg_id_id );
        if (m_msg_id_id != -1) {
          m_msg_id_type = E_MSG_ID_BODY ;
          m_msg_id_value_type = get_body_value_type (m_msg_id_id);

          GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                    << "m_msg_id_id is " << m_msg_id_id << " its type is " 
                    << m_msg_id_value_type);
        } else {
          GEN_ERROR(E_GEN_FATAL_ERROR,
                    "No definition found for session-id ["
                    << L_session_id_name << "]");
          L_ret = -1 ;
        }
      }
    }

    if (L_ret != -1) {
      if (L_outof_session_id_name) {
        GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                  << "Out Of Session Id name is " 
                  << L_outof_session_id_name );
        
        m_id_out_of_session = find_header_field_id (L_outof_session_id_name) ;
        
        GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                  << "m_id_out_of_session is " 
                  << m_id_out_of_session );
        
        if (m_id_out_of_session != -1) {
          m_type_id_out_of_session = E_MSG_ID_HEADER ;
          GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                    << "m_type_id_out_of_session is " 
                    << m_type_id_out_of_session);
        } else {
          m_id_out_of_session = find_body_value_id (L_outof_session_id_name) ;
          if (m_id_out_of_session != -1) {
            m_type_id_out_of_session = E_MSG_ID_BODY ;
            GEN_DEBUG(1, "C_ProtocolBinary::xml_interpretor() " 
                      << "m_type_id_out_of_session is  " 
                      << m_type_id_out_of_session);
          } else {
            GEN_ERROR(E_GEN_FATAL_ERROR,
                      "No definition found for out-of-session-id ["
                      << L_session_id_name << "]");
            L_ret = -1 ;
          }
        }
      } // if (L_outof_session_id_name)
    }
  }

  return (L_ret) ;
}

C_ProtocolBinary::T_pManagementSessionId 
C_ProtocolBinary::get_manage_session_elt(int P_id) {
  return (NULL) ;
}


int C_ProtocolBinary::get_nb_management_session () {
  return (0);
}

bool C_ProtocolBinary::check_present_session (int P_msg_id,int P_id) {
  return (true);
}

bool C_ProtocolBinary::find_present_session (int P_msg_id,int P_id) {
  return (true);
}

char* C_ProtocolBinary::get_header_body_field_separator() {
  return (m_header_body_field_separator);
}

int C_ProtocolBinary::analyze_setfield(C_XmlData          *P_data, 
                                       int                *P_fieldId,
                                       unsigned long      *P_fieldValueUl) {

  int                       L_ret = 0       ;
  C_XmlData                *L_data          ;
  char                     *L_fieldName     ;
  char                     *L_fieldValue    ;
  T_IdMap::iterator         L_IdMapIt       ;
  char                     *L_endstr        ;

  unsigned long             L_fieldValueUl  ;  
  int                       L_fieldId       ;


  L_data = P_data ;

  
  L_fieldName = L_data->find_value((char*)"name") ;
  if (L_fieldName == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "setfield name value is mandatory");
    L_ret = -1 ;
  }
  
  L_IdMapIt = 
    m_header_id_map->find(T_IdMap::key_type(L_fieldName));
  
  if (L_IdMapIt != m_header_id_map->end()) {
    L_fieldId = L_IdMapIt->second ;
    (*P_fieldId) = L_fieldId ;
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, 
              "Field ["
              << L_fieldName << "] not defined");
    L_ret = -1 ;
  }
  
  L_fieldValue = L_data->find_value((char*)"value") ;
  if (L_fieldValue == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "setfield value is mandatory");
    L_ret = -1 ;
  }
  
  L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,10) ;
  
  if (L_endstr[0] != '\0') {
    L_fieldValueUl = strtoul_f (L_fieldValue, &L_endstr,16) ;
    if (L_endstr[0] != '\0') {
      GEN_ERROR(E_GEN_FATAL_ERROR, "typedef size value ["
                << L_fieldValue << "] bad format");
      L_ret = -1 ;
    } else {
      (*P_fieldValueUl) = L_fieldValueUl ;
    }
  }

  return (L_ret) ;
}

