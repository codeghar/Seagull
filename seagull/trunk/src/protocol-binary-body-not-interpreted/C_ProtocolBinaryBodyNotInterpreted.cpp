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

#include "C_ProtocolBinaryBodyNotInterpreted.hpp"
#include "C_MessageBinaryBodyNotInterpreted.hpp"
#include "GeneratorError.h"
#include "GeneratorTrace.hpp"

#include "ProtocolData.hpp"

#include "BufferUtils.hpp"


C_ProtocolBinaryBodyNotInterpreted::C_ProtocolBinaryBodyNotInterpreted
() : C_ProtocolBinary() {
  m_field_present_table = NULL ;
}

C_ProtocolBinaryBodyNotInterpreted::~C_ProtocolBinaryBodyNotInterpreted() {

  unsigned long L_j ;

  if (m_field_present_table != NULL ) {
    for (L_j = 0; L_j < m_nb_header_values ; L_j++) {
      FREE_TABLE (m_field_present_table[L_j]);
    }
    FREE_TABLE (m_field_present_table) ;
  }

  if (m_header_body_position_size_map != NULL) {
    if (!m_header_body_position_size_map->empty()) {
      m_header_body_position_size_map->erase(m_header_body_position_size_map->begin(), 
                                             m_header_body_position_size_map->end());
    }
  }
  DELETE_VAR(m_header_body_position_size_map);

}

int C_ProtocolBinaryBodyNotInterpreted::get_header_body_values_from_xml (C_XmlData *P_def) {

  int                       L_ret = 0                                      ;
  C_XmlData                *L_data                                         ;
  char                     *L_value, *L_name, *L_type, *L_endstr           ;
  char                     *L_value_unit, *L_value_size, *L_value_position ;
  T_XmlData_List::iterator  L_listIt                                       ;
  T_pXmlData_List           L_subListDefine                                ;
  int                       L_id                                           ;
  T_IdMap::iterator         L_IdMapIt                                      ;
  int                       L_typeId                                       ;
  int                       L_fieldDefCpt                                  ;

  unsigned long             L_size                                         ;
  unsigned long             L_position                                     ;

  T_HeaderBodyPositionSize  L_position_size                                ;



  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::get_header_body_values_from_xml() start");


  m_nb_header_body_values++ ;
  
  ALLOC_TABLE(m_header_body_value_table,
              T_pHeaderBodyValue,
              sizeof(T_HeaderBodyValue),
              m_nb_header_body_values);
  
  L_id = 0 ;
  L_position = 0 ;
  L_typeId = 0 ;
  L_size = 0 ;
  
  L_fieldDefCpt = 0;
  L_subListDefine = P_def->get_sub_data() ;
  
  
  m_header_body_value_id_map
    ->insert(T_IdMap::value_type((char*)"body", L_id));
  
  m_header_body_value_table[L_id].m_id = L_id ;
  m_header_body_value_table[L_id].m_name = (char*)"body" ;
  m_header_body_value_table[L_id].m_type_id = L_typeId ;
  m_header_body_value_table[L_id].m_nb_set=0;
  
  L_position_size.m_position = L_position ;
  L_position_size.m_size = L_size ;
  
  m_header_body_decode_map
    ->insert(T_DecodeMap::value_type(L_position, L_id));

  
  m_header_body_position_size_map
    ->insert(T_HeaderBodyPositionSizeMap::value_type(L_id, L_position_size));
  
  
  L_id ++ ;
  
  if (m_use_open_id) {
    m_session_id_id += L_id ;
  }

  for(L_listIt  = L_subListDefine->begin() ;
      L_listIt != L_subListDefine->end() ;
      L_listIt++,  L_fieldDefCpt ++) {

    L_position = 0 ;
    L_typeId = 0 ;
    L_size = 0 ;

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


      L_value_position = L_data->find_value((char*)"position") ;
      if (L_value_position == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, "define position value is mandatory");
	L_ret = -1 ;
	break ;
      } 


      L_position = (int)strtoul_f (L_value_position, &L_endstr, 10);
      if (L_endstr[0] != '\0') {
        GEN_ERROR(E_GEN_FATAL_ERROR, "bad format, ["
                  << L_value_position << "] not a number");
        L_ret = -1 ;
        break;
      }
      
      
      L_value_unit = L_data->find_value((char*)"unit") ;
      if (L_value_unit != NULL) {
        if (strcmp(L_value_unit, (char*)"octet") != 0) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "Unsupported unit ["
	        << L_value_unit << "] value on fielddef [" << L_fieldDefCpt << "] defintion");
	  L_ret = -1 ;
	  break;
        }
      }

      L_value_size = L_data->find_value((char*)"size") ;
      if (L_value_size == NULL) {
        GEN_ERROR(E_GEN_FATAL_ERROR, 
                  "fielddef [" << L_fieldDefCpt << "] size value is mandatory ["
                  << L_name << "]");
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
                    << L_value_size 
                    << "] bad format  on fielddef [" 
                    << L_fieldDefCpt << "] defintion");
          L_ret = -1 ;
          break ;
        }
      }

      if (!((strcmp(L_type,(char*)"String") == 0 ) ||
            (strcmp(L_type,(char*)"string") == 0 ))) {
        if ( L_size > sizeof(unsigned long)) {
          GEN_ERROR(E_GEN_FATAL_ERROR, "fielddef [" 
                    << L_fieldDefCpt << "] max size value ["  
                    << sizeof(unsigned long) << "]");
          L_ret = -1 ;
          break ;
        }
      }
      
      m_header_body_value_table[L_id].m_id = L_id ;
      m_header_body_value_table[L_id].m_name = L_name ;
      m_header_body_value_table[L_id].m_type_id = L_typeId ;
      m_header_body_value_table[L_id].m_nb_set=0;

      L_position_size.m_position = L_position ;
      L_position_size.m_size = L_size ;
      if (L_ret != -1) {
 	m_header_body_value_id_map
 	  ->insert(T_IdMap::value_type(L_name, L_id));
 	m_header_body_decode_map
 	  ->insert(T_DecodeMap::value_type(L_position, L_id));

        m_header_body_position_size_map
          ->insert(T_HeaderBodyPositionSizeMap::value_type(L_id, L_position_size));

	L_id ++ ;
      }
    }
    if (L_ret == -1) break ;
  }

//    {
//      T_IdMap::iterator L_IdMapIt ;
    
//      for (L_IdMapIt = m_header_body_value_id_map->begin();
//           L_IdMapIt != m_header_body_value_id_map->end() ;
//           L_IdMapIt++) {
//        std::cerr << "id ********* iss " 
//                  << L_IdMapIt->second 
//                  << " and name is "
//                  << L_IdMapIt->first
//                  << std::endl;
//      }
//    }
  
  
  GEN_DEBUG(1, "C_ProtocolBinary::get_header_body_values_from_xml() end");

  return (L_ret);
}


int C_ProtocolBinaryBodyNotInterpreted::analyze_body_from_xml (C_XmlData *P_def) {
  int                       L_ret = -1                 ;
  C_XmlData                *L_data                    ;
  
  T_pXmlData_List           L_subListHeader                 ;
  T_XmlData_List::iterator  L_listItHeader                  ;
 
  
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::analyze_body_from_xml() start");

  L_subListHeader = P_def->get_sub_data();
  
  if (L_subListHeader != NULL) {
    
    for (L_listItHeader = L_subListHeader->begin() ;
         L_listItHeader != L_subListHeader->end() ;
         L_listItHeader++) {
      L_data = *L_listItHeader ;

      if ((L_data->get_name() != NULL ) && (strcmp(L_data->get_name(), (char*)"header") == 0)) {
        m_header_body_name = L_data->find_value((char*)"name");
        if (m_header_body_name == NULL) {
          GEN_ERROR(E_GEN_FATAL_ERROR,"header name value is mandatory");
          L_ret = -1 ;
          break;
        }

        if (L_data->get_sub_data() != NULL) {
          // header definition for body values
          m_max_nb_field_header_body = (L_data->get_sub_data())->size() ;
          if (m_max_nb_field_header_body != 0 ) {
            GEN_ERROR(E_GEN_FATAL_ERROR, 
                      "boy not interpreted, you can't define header");
            L_ret = -1 ;
            break;
          }
        }
        
        L_ret = 0 ;
      }
    }
  }

  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::analyze_body_from_xml() end");
  return (L_ret) ;
}


int C_ProtocolBinaryBodyNotInterpreted::xml_interpretor(C_XmlData *P_def) {


  C_XmlData                *L_data                           ;
  T_pXmlData_List           L_subList                        ; 
  T_XmlData_List::iterator  L_listIt                         ;
  char                     *L_value                          ;
  int                       L_ret = 0                        ;
  bool                      L_headerFound = false            ;

  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::xml_interpretor() start");


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

  NEW_VAR(m_header_body_position_size_map, T_HeaderBodyPositionSizeMap());
  m_header_body_position_size_map->clear();

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
        L_ret = analyze_dictionnary_from_xml (L_data); 
        if (L_ret == -1) break ;
      } // L_value == dico

    } // L_listIt

    if (L_headerFound == false) {
      L_ret = -1 ;
      GEN_ERROR(E_GEN_FATAL_ERROR, "No header definition found for protocol");
    }
  }  

  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::xml_interpretor() end");

  return (L_ret);
}

int C_ProtocolBinaryBodyNotInterpreted::get_header_values_from_xml (C_XmlData *P_def) {

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
  unsigned long             L_fieldCode = 0 ; 
  bool                      L_codeFound ;
  unsigned long             L_i, L_j ;

  unsigned long             L_nb_setfield, L_nb_bodyval ;
  char                     *L_body_name, *L_body_value ;
  int                       L_valueIdx, L_valueId ;

  GEN_DEBUG(1, "C_ProtocolBinary::get_header_values_from_xml() start");

  
  // not-present
  if (m_nb_header_values != 0 ) {
    ALLOC_TABLE(m_field_present_table,
                bool**,
                sizeof(bool*),
                m_nb_header_values);
    
    if (m_nb_header_body_values != 0) {
      for (L_i = 0; L_i < m_nb_header_values ; L_i++) {
        ALLOC_TABLE (m_field_present_table[L_i], bool*, sizeof(bool), m_nb_header_body_values);
        for (L_j = 0; L_j < m_nb_header_body_values ; L_j++) {
          m_field_present_table[L_i][L_j] = true ;
        }
      }
    }
  }

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
              if (strcmp(L_data->get_name(), (char*)"not-present") != 0) {
                GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown ["
                          << L_data->get_name() << "] section.[setfield] or ["
                          << m_header_body_name << "] section is mandatory");
                L_ret = -1 ;
                break ;
              }
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

	      if (C_ProtocolBinary::set_body_value(L_valueId,
				 L_body_value,
				 1,
				 &(m_body_value_table[L_id].m_value_table[L_valueIdx])) == 0) {

		if ((m_header_type_id == -1) && (L_valueId == m_header_type_id_body)) {

		  L_fieldCode = m_body_value_table[L_id].m_value_table[L_valueIdx].m_value.m_val_number ;
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
	    } else if (strcmp(L_data->get_name(), (char*)"not-present") == 0) {

	      L_fieldName = L_data->find_value((char*)"name") ;
	      if (L_fieldName == NULL) {
		GEN_ERROR(E_GEN_FATAL_ERROR, "not-present name value is mandatory");
		L_ret = -1 ;
		break ;
	      }
              
              L_ret = get_header_body_value_id(L_fieldName) ;
	      if (L_ret != -1) {
                m_field_present_table[L_id][L_ret] = false ;
                L_ret = 0 ;
	      } else {
		GEN_ERROR(E_GEN_FATAL_ERROR, 
                          "Field ["
                          << L_fieldName << "] not defined");
		break;
	      }
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

  
//    for (L_i = 0; L_i < m_nb_header_values ; L_i++) {
//        for (L_j = 0; L_j < m_nb_header_body_values ; L_j++) {
//          std::cerr << "m_field_present_table[" << L_i << "][" << L_j << "]"  
//                    << m_field_present_table[L_i][L_j] << std::endl;
//      }
//    }
  
  return (L_ret);
}

C_MessageFrame* C_ProtocolBinaryBodyNotInterpreted::create_new_message(C_MessageFrame *P_msg) {

  C_MessageBinaryBodyNotInterpreted *L_msg ;

  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::create_new_message() start: msg pt");
  NEW_VAR(L_msg, C_MessageBinaryBodyNotInterpreted(this));
  if (P_msg != NULL) {
    (*L_msg) = *((C_MessageBinaryBodyNotInterpreted*)P_msg) ;
  }
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::create_new_message() end: msg pt");
  return (L_msg);
}

C_MessageFrame* C_ProtocolBinaryBodyNotInterpreted::create_new_message(void                *P_xml, 
                                                                       T_pInstanceDataList  P_list,
                                                                       int                 *P_nb_value) {

  C_XmlData                            *L_Xml = (C_XmlData*) P_xml ;
  bool                                  L_msgOk = true             ;
  C_MessageBinaryBodyNotInterpreted    *L_msg                      ;
  char                                 *L_currentName              ;
  int                                   L_header_val_id            ;
  int                                   L_body_val_id              ;
  
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::create_new_message() start: xml");
  
  // Allocate a new message
  NEW_VAR(L_msg, C_MessageBinaryBodyNotInterpreted(this));

  // Get the message name from XML definition
  L_currentName = L_Xml -> get_name () ;

  if (strcmp(L_currentName, get_header_name()) == 0) {
    // message decoding
    L_currentName = L_Xml -> find_value ((char*) "name") ;
    if (L_currentName == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
		"name value is mandatory for ["
		<< get_header_name()
		<< "]");
      L_msgOk = false ;
    } else {
      GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::create_new_message() " 
		<< "the name of this message is " << L_currentName );

      L_header_val_id = get_header_value_id(L_currentName) ;

      GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::create_new_message() " 
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

	L_msg->set_header_id_value(L_header_val_id);
	
	L_listBodyVal = L_Xml -> get_sub_data() ;
	
	if (L_listBodyVal != NULL) {
	  // L_nbBodyVal = L_listBodyVal->size() ;
	  for (L_bodyValIt  = L_listBodyVal->begin() ;
	       L_bodyValIt != L_listBodyVal->end() ;
	       L_bodyValIt++) {
	    L_bodyData = *L_bodyValIt ;
	    
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
	      
	      if (get_body_value_type (L_body_val_id) 
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
		L_msg->C_MessageBinary::set_body_value(&L_bodyVal);

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
		
		if (C_ProtocolBinary::set_body_value(L_body_val_id, 
				   L_bodyValue,
				   1,
				   &L_bodyVal,
				   &L_toBeDelete) == 0) {
		  L_msg->C_MessageBinary::set_body_value(&L_bodyVal);

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
		
	      } 

            } else if (strcmp(L_bodyData->get_name(),
                              (char*)"setfield")==0) {
              T_ValueData           L_val_setfield   ;
              L_val_setfield.m_type  = E_TYPE_NUMBER ;
	      L_msgOk = (analyze_setfield(L_bodyData, &L_val_setfield) == -1) 
                ? false : true  ;
              if (L_msgOk) {
                L_msg->C_MessageBinary::set_header_value((int)L_val_setfield.m_id, &L_val_setfield);
                resetMemory(L_val_setfield);
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
	  }
	} // BodyVal = NULL
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

  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::create_new_message() end: xml");

  //  L_msg->dump(std::cerr);

  return (L_msg);
}

void C_ProtocolBinaryBodyNotInterpreted::get_field_position 
(T_pHeaderBodyPositionSize P_pos, int P_id) {
  T_HeaderBodyPositionSizeMap::iterator L_positionSizeIt    ;
  L_positionSizeIt = m_header_body_position_size_map
    ->find(T_HeaderBodyPositionSizeMap::key_type(P_id));

  if (L_positionSizeIt != m_header_body_position_size_map->end()) {
    *P_pos = L_positionSizeIt->second ;
  } else {
    P_pos -> m_size = 0 ;
  }
}


C_ProtocolFrame::T_MsgError C_ProtocolBinaryBodyNotInterpreted::encode_body (int            P_nbVal, 
                                                                             T_pBodyValue   P_val,
                                                                             unsigned char *P_buf, 
                                                                             size_t        *P_size) {

  unsigned char     *L_ptr = P_buf       ;
  int                L_body_id           ;
  T_pBodyValue       L_body_val          ;

  unsigned long      L_valueSize         ;

  C_ProtocolFrame::T_MsgError  L_error = C_ProtocolFrame::E_MSG_OK;


  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::encode_body() start");

  L_body_val = &P_val[0] ;
  L_body_id = L_body_val->m_id  ;
  
  if (m_stats) {
    m_stats->updateStats (E_MESSAGE_COMPONENT,
                          E_SEND,
                          L_body_id);
  }
  
  L_valueSize = L_body_val -> m_value.m_val_binary.m_size ;

  if (L_valueSize > *P_size) {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                  "Buffer maximum size reached [" << *P_size << "]");
    L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
  } else {
    memcpy(L_ptr, L_body_val->m_value.m_val_binary.m_value, L_valueSize);
    L_ptr += L_valueSize ;
    *P_size = L_valueSize ;
  }
  
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::encode_body() end");

  return (L_error);
}

C_ProtocolFrame::T_MsgError C_ProtocolBinaryBodyNotInterpreted::encode_body_without_stat (int            P_nbVal, 
                                                                                          T_pBodyValue   P_val,
                                                                                          unsigned char *P_buf, 
                                                                                          size_t        *P_size) {

  unsigned char     *L_ptr = P_buf       ;
  T_pBodyValue       L_body_val          ;

  unsigned long      L_valueSize         ;

  C_ProtocolFrame::T_MsgError  L_error = C_ProtocolFrame::E_MSG_OK;


  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::encode_body_without_stat() start");

  L_body_val = &P_val[0] ;
  
  L_valueSize = L_body_val -> m_value.m_val_binary.m_size ;

  if (L_valueSize > *P_size) {
    GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR,
                  "Buffer maximum size reached [" << *P_size << "]");
    L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
  } else {
    memcpy(L_ptr, L_body_val->m_value.m_val_binary.m_value, L_valueSize);
    L_ptr += L_valueSize ;
    *P_size = L_valueSize ;
  }
  
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::encode_body_without_stat() end");

  return (L_error);
}

int C_ProtocolBinaryBodyNotInterpreted::decode_body(unsigned char *P_buf, 
                                                    size_t         P_size,
                                                    T_pBodyValue   P_valDec,
                                                    int           *P_nbValDec,
                                                    int           *P_headerId) {
  
  
  unsigned char        *L_ptr = P_buf ;
  int                   L_ret = 0 ;
  unsigned long         L_total_size, L_data_size ;
  T_DecodeMap::iterator L_decodeIt ;
  int                   L_nbValDec = 0 ;
  unsigned long         L_type_id_val ;
  int                   L_header_type_id = get_header_type_id();
  
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::decode_body() start");
  // ctrl a ajouter

  L_total_size = 0 ;
  *P_nbValDec  = 0 ;
  L_nbValDec   = 0 ;  
  L_data_size = P_size ;
  
  if (m_stats) {
    m_stats->updateStats (E_MESSAGE_COMPONENT,
                          E_RECEIVE,
                          0);
  }
  
  P_valDec[L_nbValDec].m_id = 0 ;
  P_valDec[L_nbValDec].m_value.m_val_binary.m_size = L_data_size ;
  
  ALLOC_TABLE(P_valDec[L_nbValDec].m_value.m_val_binary.m_value,
              unsigned char*,
              sizeof(unsigned char),
              L_data_size);
  
  memcpy(P_valDec[L_nbValDec].m_value.m_val_binary.m_value,
         L_ptr,
         L_data_size);

  
  // if (get_header_type_id() == -1) 
  // Check if header type exist
  if (L_header_type_id == -1) {
    
    // Now check if the current field is the msg type one
    if (!m_header_type_id_body) {
      
      L_type_id_val = P_valDec[L_nbValDec].m_value.m_val_number ;
      
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
  L_total_size += L_data_size ;

  L_nbValDec ++ ;
  *P_nbValDec = L_nbValDec ;
  
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::decode_body() end nb Val: " << L_nbValDec
            << " ret: " << L_ret << "\n");

  return (L_ret) ;
  
}


int C_ProtocolBinaryBodyNotInterpreted::analyze_sessions_from_xml (C_XmlData *P_def) {
  return (C_ProtocolBinary::analyze_sessions_id_out_from_xml(P_def));
}


bool C_ProtocolBinaryBodyNotInterpreted::check_present_session (int P_msg_id,int P_id) {
  return (m_field_present_table[P_msg_id][P_id]);
}

bool C_ProtocolBinaryBodyNotInterpreted::find_present_session (int P_msg_id,int P_id) {

  bool           L_result ;
  int            L_id = P_id ;
  
  if (L_id < (int)m_max_nb_field_header) {
    L_result = true ;
  } else {
    L_id -= m_max_nb_field_header ;
    if (L_id >= 0 ) {
      return (check_present_session(P_msg_id,L_id));
    } else {
      L_result = false ;
    }
  }
  return (L_result);
}




iostream_output& C_ProtocolBinaryBodyNotInterpreted::print_body  (iostream_output&  P_stream, 
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

  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::print_body() start nb: " << P_nb);
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::print_body() m_nb_field_header_body: " 
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

    if (L_body_fieldValues->m_nb_set != 0 ) {
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
        static char L_ascii_buf [50] ;
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

            pretty_binary_printable_buffer(L_cur, L_cNum, L_hexa_buf, L_ascii_buf);
            P_stream << "[" ;
            P_stream << L_hexa_buf ;
            P_stream << "] <=> [" ;
            P_stream << L_ascii_buf;
            P_stream << "]" ;

            L_cur += L_cNum ;
          }
          L_nb = L_buffer_size % L_cNum ;
          if (L_nb != 0) {
            P_stream << iostream_endl ;
            P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) 
                     << L_levelStr << " ";
            pretty_binary_printable_buffer(L_cur, L_nb, L_hexa_buf, L_ascii_buf);
            P_stream << "[" ;
            P_stream << L_hexa_buf ;
            P_stream << "] <=> [" ;
            P_stream << L_ascii_buf;
            P_stream << "]" ;
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

