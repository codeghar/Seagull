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

#include "C_ProtocolText.hpp"

#include "GeneratorError.h"
#include "C_MessageText.hpp"
#include "BufferUtils.hpp"
#include <cstdlib> // for strtoul
#include "GeneratorTrace.hpp"
#include "ProtocolData.hpp"
#include "dlfcn_t.hpp"
#include "ParserFrame.hpp"
#include <regex.h>


C_ProtocolText::C_ProtocolText() : C_ProtocolTextFrame() {

  m_header_fields_list          = NULL ;
  m_body_fields_list            = NULL ;
  m_fields_name_map             = NULL ;
  m_id_counter                  = -1   ;
  m_fields_desc_table           = NULL ;
  m_header_fields_dico          = NULL ;
  m_value_sessions_table        = NULL ;
  NEW_VAR(m_message_decode_map, T_DecodeMap());
  m_message_decode_map->clear();

  m_body_separator              = NULL ;

  m_body_separator_size         = 0    ;
  m_message_type_field_id       = -1   ;
  m_session_id_id               = -1   ;
  m_session_method              = &C_MessageText::getSessionFromField ;

  m_nb_methods                  = 0    ;
  m_methods                     = NULL ; 


  m_nb_fields_desc_table        = 0    ;

  NEW_VAR (m_message_name_list, T_NameAndIdList()) ;

  // m_names_table             = NULL    ;
  m_names_fields            = NULL    ;

  m_def_method_list         = NULL    ;  

}

void  C_ProtocolText::analyze_data( C_XmlData            *P_def, 
                                    char                **P_name,
                                    T_pConfigValueList    P_config_value_list,
                                    T_pContructorResult   P_res) {

  T_ConstructorResult  L_res = E_CONSTRUCTOR_OK ;

  char        *L_body_separator    = NULL ;

  GEN_DEBUG(1, "C_ProtocolText::C_ProtocolText() start");

  if (P_def) {
    *P_name = P_def->find_value((char*)"name") ;
    if (*P_name == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "No name for protocol definition");
      L_res = E_CONSTRUCTOR_KO ;
    }
    
    if (L_res == E_CONSTRUCTOR_OK) {
      m_name = *P_name ;

      L_body_separator = P_def->find_value((char *)"body-separator");
      if (L_body_separator == NULL) {
        GEN_ERROR(E_GEN_FATAL_ERROR, 
                  "No body separator for protocol definition");
        L_res = E_CONSTRUCTOR_KO ;
      } else {
        if (strlen(L_body_separator) == 0) {
          GEN_ERROR(E_GEN_FATAL_ERROR, 
                    "Empty body separator for protocol definition");
          L_res = E_CONSTRUCTOR_KO ;
        } else {
          set_body_separator(L_body_separator);
        }
      }

      if (L_res == E_CONSTRUCTOR_OK) {
        *P_res = (xml_interpretor(P_def,P_name,P_config_value_list) == 0) 
          ? E_CONSTRUCTOR_OK : E_CONSTRUCTOR_KO ;
        if (*P_res == E_CONSTRUCTOR_OK) {
          *P_res = (set_body_method () == 0) 
            ? E_CONSTRUCTOR_OK : E_CONSTRUCTOR_KO ;
        }
      } else {
        *P_res = L_res ;
      }
    } else {
      *P_res = L_res ;
    }
  } else {
    *P_res = E_CONSTRUCTOR_KO ;
  }

  GEN_DEBUG(1, "C_ProtocolText::C_ProtocolText() end");

}


int C_ProtocolText::analyze_body_method_param (int  P_index,
                                               char *P_body_method_param) {

  int                         L_ret          = 0             ;
  char                       *L_lib_name     = NULL          ;
  char                       *L_fun_name     = NULL          ;

  void                       *L_library_handle               ; 
  void                       *L_function                     ;


  L_lib_name = find_text_value(P_body_method_param,(char*)"lib")  ;
  if (L_lib_name == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "no name for the library for the parser (lib=...)");
    L_ret = -1;
  } else {
    L_library_handle = dlopen(L_lib_name, RTLD_LAZY);
    if (L_library_handle == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "Unable to open library file [" 
                << L_lib_name
                << "] error [" << dlerror() << "]");
      L_ret = -1 ;
    }
  }

  L_fun_name = find_text_value(P_body_method_param,(char*)"function") ;
  if (L_fun_name == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "no name for the function for the parser (function=...)");
    L_ret = -1;
  } else {
    if (L_library_handle) { 
      L_function = dlsym(L_library_handle, L_fun_name);
      if (L_function == NULL) {
        GEN_ERROR(E_GEN_FATAL_ERROR, "Error [" << dlerror() << "]");
        L_ret = -1 ;
      } else {

        set_body_decode_method(P_index,
                               E_BODY_METHOD_PARSE,
                               &C_MessageText::DecodeBodyWithParser,
                               (void*)L_function);

        set_encode_method(P_index,
                          E_BODY_METHOD_PARSE,
                          &C_MessageText::NoEncode,
                          (void*)L_function);

      }
    }
  }

  return (L_ret);

}


int C_ProtocolText::set_body_method() {

  int                       L_ret          = 0 ;
  T_DefMethodList::iterator L_def_method_it    ;
  T_FieldNameMap::iterator  L_it               ;
  
  

  if (!m_def_method_list->empty()) {
    if (m_def_method_list->size() > 0) {

      int L_i ;

      set_number_methods((int)m_def_method_list->size());
      L_i = 0 ;

      for (L_def_method_it = m_def_method_list->begin();
           L_def_method_it != m_def_method_list->end();
           L_def_method_it++) {

        switch (L_def_method_it->m_method) {
        case E_BODY_METHOD_LENGTH:
          // analyze id to decode
          L_it = m_fields_name_map->find(T_FieldNameMap::key_type(L_def_method_it->m_param));
          if (L_it == m_fields_name_map->end()) {
            GEN_ERROR(E_GEN_FATAL_ERROR, 
                      "field [" << L_def_method_it->m_param << "] not defined");
            L_ret = -1 ;
          } else {
            set_body_decode_method(L_i,
                                   L_def_method_it->m_method,
                                   &C_MessageText::DecodeBodyWithContentLength,
                                                  (void*)&((L_it->second)->m_id));
            set_encode_method(L_i,
                              L_def_method_it->m_method,
                              &C_MessageText::EncodeWithContentLength,
                              (void*)&((L_it->second)->m_id));
          }
          break ;
        case E_BODY_METHOD_PARSE:
          // load the library
          L_ret = analyze_body_method_param(L_i, L_def_method_it->m_param) ;
          break ;
        }
        if (L_ret != 0) break ;
        L_i ++ ;
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "no body decode method defined");
      L_ret = -1 ;
    }
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, 
              "no body decode method defined");
    L_ret = -1 ;
  }

  return (L_ret);
}


C_ProtocolText::~C_ProtocolText() {

  T_FieldDefList::iterator     L_it             ;
  int                          L_i              ;
  T_pFieldDef                  L_field_def ;

  m_id_counter = -1 ;

  if ((m_header_fields_list != NULL ) && (!m_header_fields_list->empty())) {
    for (L_it = m_header_fields_list->begin();
         L_it != m_header_fields_list->end() ;
         L_it++) {
      L_field_def = *L_it;
      if (!(L_field_def->m_expr_list)->empty()) {
        (L_field_def->m_expr_list)->erase((L_field_def->m_expr_list)->begin(),
                                          (L_field_def->m_expr_list)->end());
      }
    }
    m_header_fields_list->erase(m_header_fields_list->begin(),
                                m_header_fields_list->end());
    DELETE_VAR (m_header_fields_list) ;
  }

  if ((m_body_fields_list != NULL) && (!m_body_fields_list->empty())) {
    for (L_it = m_body_fields_list->begin();
         L_it != m_body_fields_list->end() ;
         L_it++) {
      L_field_def = *L_it;
      if (!(L_field_def->m_expr_list)->empty()) {
        (L_field_def->m_expr_list)->erase((L_field_def->m_expr_list)->begin(),
                                          (L_field_def->m_expr_list)->end());
      }
    }
    m_body_fields_list->erase(m_body_fields_list->begin(),
                              m_body_fields_list->end());
    DELETE_VAR (m_body_fields_list) ;
  }


  // delete m_field_body_name_map
  if (m_fields_name_map != NULL) {
    if (!m_fields_name_map->empty()) {
      m_fields_name_map->erase(m_fields_name_map->begin(),
                               m_fields_name_map->end());
    }
    DELETE_VAR(m_fields_name_map);
  }

  if (m_header_fields_dico != NULL ) {
    if (!m_header_fields_dico->empty()) {
      m_header_fields_dico->erase(m_header_fields_dico->begin(),
				  m_header_fields_dico->end());
    }
    DELETE_VAR(m_header_fields_dico);
  }

  if (m_message_names_table != NULL) {
    for(L_i = 0; L_i < m_nb_message_names; L_i++) {
      FREE_TABLE(m_message_names_table[L_i]);
    }
    FREE_TABLE(m_message_names_table);
  }

  if (!m_message_decode_map->empty()) {
    m_message_decode_map
      ->erase(m_message_decode_map->begin(),
              m_message_decode_map->end());
  }
  DELETE_VAR(m_message_decode_map);

  if ((m_names_fields != NULL) && (m_nb_fields_desc_table > 0)) {
    for(L_i = 0; L_i < m_nb_fields_desc_table ; L_i++) {
      FREE_TABLE(m_names_fields[L_i]);
    }
    FREE_TABLE(m_names_fields);
  }

  FREE_TABLE(m_fields_desc_table);
  m_nb_fields_desc_table        = 0    ;

  if (m_value_sessions_table_size != 0) {
    FREE_TABLE(m_value_sessions_table);
    m_value_sessions_table_size = 0 ;
  }

  m_body_separator              = NULL ;
  m_body_separator_size         = 0    ;
  m_message_type_field_id       = -1   ;
  m_session_id_id               = -1   ;
  m_session_method              = &C_MessageText::getSessionFromField ;


  destroy_methods();

  m_nb_methods                  = 0    ;
  m_methods                     = NULL ; 

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

  DELETE_VAR(m_stats);


  if(!m_def_method_list -> empty()) {
    m_def_method_list -> erase (m_def_method_list->begin(), m_def_method_list->end());
  }
  DELETE_VAR (m_def_method_list) ;

  
}


C_MessageFrame* C_ProtocolText::decode_message(unsigned char *P_buffer, 
                                               size_t        *P_size, 
                                               C_ProtocolFrame::T_pMsgError P_error) {

  C_MessageText                   *L_msg             ;
  int                              L_msg_id          ;

  L_msg = (C_MessageText*)create_new_message(NULL);

  (*P_size) = L_msg -> decode (P_buffer, *P_size, P_error);

  switch (*P_error) {
  case C_ProtocolFrame::E_MSG_OK: {

    if (m_stats) {
      L_msg_id = L_msg -> get_id_message () ;
      m_stats->updateStats (E_MESSAGE,
			    E_RECEIVE,
			    L_msg_id);
    }

    GEN_LOG_EVENT(LOG_LEVEL_MSG, 
  		  "Received [" << *L_msg << "]");
  }
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
C_ProtocolText::encode_message(C_MessageFrame *P_msg,
                               unsigned char  *P_buffer,
                               size_t         *P_buffer_size) {

  C_ProtocolFrame::T_MsgError      L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
  C_MessageText                   *L_msg   = dynamic_cast<C_MessageText*>(P_msg)   ;

  int                              L_msg_id                                        ;


  if (L_msg != NULL) {

    L_msg_id = L_msg -> get_id_message () ;
    L_error = L_msg->encode(P_buffer, P_buffer_size);
    if (L_error == C_ProtocolFrame::E_MSG_OK ) {
      if (m_stats) {
        m_stats->updateStats(E_MESSAGE,
                             E_SEND,
                             L_msg_id);
      }
      GEN_LOG_EVENT(LOG_LEVEL_MSG, 
                    "Send [" << *L_msg << "]");
    }
  }
  
  return (L_error);
}

void C_ProtocolText::log_buffer(char* P_header, 
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

char* C_ProtocolText::message_name() {
  return (m_header_name);
}

char* C_ProtocolText::message_component_name() {
  return (m_body_name);
}

T_pNameAndIdList C_ProtocolText::message_name_list     () {

  T_NameAndId             L_elt                   ;
  int                     L_i                     ;
  
  
  for (L_i = 0 ; L_i <  m_nb_message_names ; L_i ++) {
    ALLOC_TABLE(L_elt.m_name,
    		char*, sizeof(char),
    		(strlen(m_message_names_table[L_i])+1));
    
    strcpy(L_elt.m_name, m_message_names_table[L_i]);
    L_elt.m_id = L_i ;
    m_message_name_list->push_back(L_elt);
  }
  
  return (m_message_name_list);
}

T_pNameAndIdList C_ProtocolText::message_component_name_list    () {

  return (NULL) ;
}


char* C_ProtocolText::message_name(int P_id) {
  return (m_message_names_table[P_id]);
}


int C_ProtocolText::find_field(char *P_name) {
  int L_id ;
  
  for(L_id = 0 ; L_id < m_nb_fields_desc_table; L_id++) {
    if (strcmp(m_fields_desc_table[L_id]->m_name, P_name)==0) {
      return (L_id);
    }
  }
  return (-1);
}


T_TypeType C_ProtocolText::get_field_type (int P_id,
                                           int P_sub_id) {
  T_TypeType L_type = E_TYPE_STRING ;
  return (L_type);
}

bool C_ProtocolText::check_sub_entity_needed (int P_id) {
  return (false);
}

bool C_ProtocolText::find_present_session (int P_msg_id,int P_id) {
  return (true);
}


C_MessageFrame* C_ProtocolText::create_new_message (C_MessageFrame *P_msg) {
  C_MessageText *L_dest = NULL ;

  C_MessageText *L_source = dynamic_cast<C_MessageText *>(P_msg) ;
  
  if (L_source != NULL) {
    // L_source->dump(std::cerr);
    NEW_VAR(L_dest, C_MessageText(*L_source));
  } else {
    NEW_VAR(L_dest, C_MessageText(this));
  }

  return (L_dest);
}


int C_ProtocolText::analyze_messsage_scen (C_XmlData            *P_data, 
                                           T_pCDATAValueList     P_cdata_value_list) {

  int                         L_ret                = 0    ;
  T_pXmlData_List             L_data_scen          = NULL ;
  T_XmlData_List::iterator    L_it                        ;
  T_CDATAValue                L_cdata_val                 ;

  L_data_scen = P_data->get_sub_data();
  if (L_data_scen == NULL){
    GEN_ERROR(E_GEN_FATAL_ERROR, "C_ProtocolText::analyze_messsage_scen() "
              << "no value found for this message [" << m_header_name << "]");
    L_ret = -1 ;
  } 

  if (L_ret != -1) {
    for (L_it = L_data_scen->begin();
         L_it != L_data_scen->end();
         L_it++) {
      L_cdata_val.m_value = (*L_it)->find_value((char*)"value");
      P_cdata_value_list->push_back(L_cdata_val);
    }
  }

  return (L_ret);

}

C_MessageFrame* C_ProtocolText::create_new_message(void                *P_xml, 
                                                   T_pInstanceDataList  P_list,
                                                   int                 *P_nb_value) {

  C_MessageText                  *L_msg                = NULL               ;
  C_XmlData                      *P_data               = (C_XmlData *)P_xml ;
  int                             L_ret                = 0                  ;
  T_pCDATAValueList               L_list_value         = NULL               ;

  T_CDATAValueList::iterator      L_cdata_it                                ;
  char                           *L_header             = NULL               ;
  char                           *L_body               = NULL               ;
  int                             L_i                  = 0                  ;

  NEW_VAR(L_list_value, T_CDATAValueList());

  // analyze message from scenario 
  L_ret = analyze_messsage_scen(P_data, L_list_value) ;

  if (L_ret != -1) {
    if ((L_list_value != NULL ) && (!(L_list_value)->empty())) {  
      for (L_cdata_it = (L_list_value)->begin();
           L_cdata_it != (L_list_value)->end();
           L_cdata_it++) {
        if (L_i == 0) {
          L_header = (*L_cdata_it).m_value ;
        } else if (L_i == 1) {
          L_body = (*L_cdata_it).m_value ;
        } else {
          // To Do if we have many value
          break ;
        }
        L_i++ ;
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "C_ProtocolText::create_new_message() "
                << "no value found for this message [" << m_header_name << "]");
      L_ret = -1 ;
    }
  }
    
  if (L_ret != -1) {
    L_msg = create(this,L_header,L_body);
  }
  
  // delete L_list_value 
  if (L_list_value != NULL ) {
    if (!L_list_value->empty()) {
      L_list_value->erase(L_list_value->begin(),
                          L_list_value->end());
    }
    DELETE_VAR(L_list_value);
  }

  return (L_msg);

}


C_ProtocolText::T_FieldHeaderList* 
C_ProtocolText::analyze_header_value (C_XmlData          *P_data, 
                                      T_FieldHeaderList  *P_fielddef_header_list,
                                      int                *P_ret) {

  char                     *L_value              ;
  T_FieldHeader             L_fielddef_header    ;

  *P_ret = 0 ;
  
  GEN_DEBUG(1, "C_ProtocolText::analyze_header_value() start");

  // ctrl fields 
  L_value = P_data->find_value((char *)"name");

  GEN_DEBUG(1, "C_ProtocolText::analyze_header_value() "
	    << "[" << m_header_name << "] = [" << L_value << "]");

  if (L_value == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "name definition value is mandatory for this section");
    *P_ret = -1 ;
  } else {
    L_fielddef_header.m_name = L_value ;
  }

  if (*P_ret != -1) {
    if (P_fielddef_header_list == NULL) {
      NEW_VAR(P_fielddef_header_list, T_FieldHeaderList());
    }
    P_fielddef_header_list->push_back(L_fielddef_header);
  }

  GEN_DEBUG(1, "C_ProtocolText::analyze_header_value() end");

  return (P_fielddef_header_list);
}


int C_ProtocolText::analyze_dictionnary_from_xml (C_XmlData *P_def) {
  
  int                       L_ret = 0                      ;
  int                       L_i                            ;
  
  T_pXmlData_List           L_subListDico                  ;  
  T_XmlData_List::iterator  L_listItDico                   ; 


  GEN_DEBUG(1, "C_ProtocolText::analyze_dictionnary_from_xml() start");

  // analyze session-id
  L_ret = analyze_sessions_id_from_xml(P_def);
  
  if (L_ret != -1) {
    L_subListDico = P_def->get_sub_data();
    if (L_subListDico != NULL) {
      for (L_listItDico = L_subListDico->begin() ;
           L_listItDico != L_subListDico->end() ;
           L_listItDico++) {
        if (((*L_listItDico)->get_name() != NULL) && 
            (strcmp((*L_listItDico)->get_name(), m_header_name) == 0)) {
          m_header_fields_dico = analyze_header_value(*L_listItDico, m_header_fields_dico, &L_ret);
          if (L_ret == -1) break ;
        }
        
        // TO DO Add m_body_name...
        //  if (((*L_listItDico)->get_name() != NULL) && 
        //              (strcmp((*L_listItDico)->get_name(), m_body_name) == 0)) {
        //   m_header_fields_dico = analyze_body_value(*L_listItDico, m_header_fields_dico, &L_ret);
        //   if (L_ret == -1) break ;
        // }

      } // for (L_listItDico = L_subListDico->begin()
    } // if (L_subListDico != NULL)
  }

  if (L_ret != -1) {
    if (!m_header_fields_dico->empty()) {
      m_nb_message_names =  m_header_fields_dico->size();
      ALLOC_TABLE(m_message_names_table, char**, sizeof(char*),m_nb_message_names);
      for (L_i = 0; L_i < m_nb_message_names ; L_i++) {
        m_message_names_table[L_i] = NULL ;
      } 
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "messages not found in dictionnary"); 
      L_ret = -1 ;
    }
  }

  GEN_DEBUG(1, "C_ProtocolText::analyze_dictionnary_from_xml() end");

  return (L_ret);
}

int C_ProtocolText::analyze_sessions_id_from_xml (C_XmlData *P_def) {


  int                                     L_ret = 0                 ;
  C_XmlData::T_pXmlField_List             L_fields_list             ;
  C_XmlData::T_XmlField_List::iterator    L_fieldIt                 ;

  T_ManagementSessionTextId               L_management_session      ;
  T_ManagementSessionTextIdList           L_list_management_session ;
  T_ManagementSessionTextIdList::iterator L_it                      ;

  char                                   *L_name_value = NULL       ;
  int                                     L_msg_id_id               ;
  int                                     L_id                      ;
  bool                                    L_use_open_id = false     ;
  int                                     L_session_id_id           ;

  GEN_DEBUG(1, "C_ProtocolText::analyze_sessions_id_from_xml() start");  

  L_fields_list = P_def->get_fields();
  if (!L_fields_list->empty()) {
    for (L_fieldIt  = L_fields_list->begin() ;
	L_fieldIt != L_fields_list->end() ;
        L_fieldIt++) {

      if (strcmp((*L_fieldIt)->get_name() , (char*)"session-method") == 0 ) {
  	L_name_value = (*L_fieldIt)->get_value();
        if (L_name_value == NULL) {
          GEN_ERROR(E_GEN_FATAL_ERROR,
                    (*L_fieldIt)->get_name()
                    << " value is mandatory for ["
                    << m_header_name 
                    << "] section");
          L_ret = -1 ;
          break ;
        }
        if (strcmp(L_name_value, (char*)"open-id") == 0 ) {
          L_use_open_id = true ;
          L_session_id_id = new_id();
          set_session_id_id(L_session_id_id);
          use_open_id();
        } else if (strcmp(L_name_value, (char*)"field") != 0 ) {
          GEN_ERROR(E_GEN_FATAL_ERROR,
                    (*L_fieldIt)->get_name()
                    << " value is mandatory for ["
                    << m_header_name 
                    << "] section");
          L_ret = -1 ;
          break ;
        }
      }
      
      if((strcmp((*L_fieldIt)->get_name() , (char*)"session-id") == 0 ) || 
         (strcmp((*L_fieldIt)->get_name() , (char*)"out-of-session-id") == 0 )) {

        if (L_use_open_id == false) {

          L_name_value = (*L_fieldIt)->get_value();
          if (L_name_value == NULL) {
            GEN_ERROR(E_GEN_FATAL_ERROR,
                      (*L_fieldIt)->get_name()
                      << " is mandatory for ["
                      << m_header_name 
                      << "] section");
            L_ret = -1 ;
            break ;
          }
          
          // retrieve informations for session_id
          L_msg_id_id = find_field_id (L_name_value) ;
          GEN_DEBUG(1, "C_ProtocolText::analyze_sessions_id_from_xml() " 
                    << "L_msg_id_id is " << L_msg_id_id );
          if (L_msg_id_id != -1) {
            L_management_session.m_msg_id_id = L_msg_id_id ;
            L_management_session.m_msg_id_value_type = E_TYPE_STRING ;
            L_list_management_session.push_back(L_management_session);
          } else {
            GEN_ERROR(E_GEN_FATAL_ERROR,
                      "No definition found for "
                      << (*L_fieldIt)->get_name() 
                      << " ["
                      << L_name_value << "]");
            L_ret = -1 ;
            break ;
          }
        } else { // if L_use_open_id == true
          GEN_ERROR(E_GEN_FATAL_ERROR,
                    "Definition found for "
                    << (*L_fieldIt)->get_name() 
                    << " with session-method=open-id");
            L_ret = -1 ;
            break ;
        }
        
      } // if((strcmp((*L_fieldIt)..)))
    } // for (L_fieldIt  =...)

    if (L_ret != -1) {
      if (L_use_open_id == false) {

        if (!L_list_management_session.empty()) {
          m_value_sessions_table_size = L_list_management_session.size() ;
          ALLOC_TABLE(m_value_sessions_table,
                      T_pManagementSessionTextId,
                      sizeof(T_ManagementSessionTextId),
                      m_value_sessions_table_size);
          
          L_id = 0 ;
          for (L_it  = L_list_management_session.begin();
               L_it != L_list_management_session.end()  ;
               L_it++) {
            m_value_sessions_table[L_id] = *L_it ;
            L_id ++ ;
          }
          
          L_list_management_session.erase(L_list_management_session.begin(),
                                          L_list_management_session.end());
          
//            for (L_id = 0 ; L_id < m_value_sessions_table_size ; L_id++) {
//              std::cerr << "m_value_sessions_table[" 
//                        << L_id 
//                        << "]" 
//                        << m_value_sessions_table[L_id].m_msg_id_id
//                        << " " 
//                        << m_value_sessions_table[L_id].m_msg_id_value_type
//                        << std::endl;
            
//            }
          
        } else {
          GEN_ERROR(E_GEN_FATAL_ERROR,
                    "No session-id nor out-of-session definition found"); 
          L_ret = -1 ;
        }
      } // if L_use_open_id == false
    } // if (L_ret != -1)
  } else { // if (!L_fields_list->empty())
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "No session-id nor out-of-session found in dictionary definition");
    L_ret = -1 ;
  }
  
  
  GEN_DEBUG(1, "C_ProtocolBinaryBodyNotInterpreted::analyze_sessions_id_from_xml() end");

  return (L_ret);
}

int C_ProtocolText::analyze_from_xml (C_XmlData *P_def, 
                                      T_FieldDefList *P_fielddef_list,
                                      char **P_section_name,
                                      char **P_type,
                                      bool P_header_body) {

  int                                  L_ret = 0                  ;
  T_pXmlData_List                      L_fielddef_xml_list = NULL ;
  C_XmlData                           *L_fielddef = NULL          ;
  T_XmlData_List::iterator             L_fielddef_it              ;
  T_pXmlData_List                      L_regexp_xml_list = NULL ;
  C_XmlData                           *L_regexp = NULL          ;
  T_XmlData_List::iterator             L_regexp_it              ;

  T_RegExpStr                          *L_regexp_data            ;
  T_FieldDef                           *L_fielddef_data         ;


  GEN_DEBUG(1, "C_ProtocolText::analyze_from_xml() start");

  *P_section_name = P_def->find_value((char *)"name");
  if (*P_section_name == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "name definition value is mandatory for header section");
    L_ret = -1 ;
  }

  *P_type   = P_def->find_value((char *)"type");
  if (*P_type == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "type definition value is mandatory for header section");
    L_ret = -1 ;
  }

  if (L_ret != -1) {

    L_fielddef_xml_list = P_def->get_sub_data() ;
    for(L_fielddef_it  = L_fielddef_xml_list->begin() ;
        L_fielddef_it != L_fielddef_xml_list->end() ;
        L_fielddef_it++) {
      L_fielddef = *L_fielddef_it ;
      if (strcmp(L_fielddef->get_name(), (char*)"fielddef") == 0) {
        NEW_VAR(L_fielddef_data, T_FieldDef());
        NEW_VAR(L_fielddef_data->m_expr_list, T_ExpStrLst());
        L_fielddef_data->m_name = L_fielddef->find_value((char*)"name") ;
        if (L_fielddef_data->m_name == NULL) {
          GEN_ERROR(E_GEN_FATAL_ERROR, 
                    "fielddef name value is mandatory");
          L_ret = -1 ;
          break ;
        } else {
          L_fielddef_data->m_format = L_fielddef->find_value((char*)"format") ;
          L_regexp_xml_list = L_fielddef->get_sub_data() ;
          for(L_regexp_it  = L_regexp_xml_list->begin() ;
              L_regexp_it != L_regexp_xml_list->end() ;
              L_regexp_it++) {

            L_regexp = *L_regexp_it ;
            if (strcmp(L_regexp->get_name(), (char*)"regexp") == 0) {
              
              NEW_VAR(L_regexp_data, T_RegExpStr());

              L_regexp_data->m_name = L_regexp->find_value((char*)"name") ;
              L_regexp_data->m_expr = L_regexp->find_value((char*)"expr") ;

              // default values ?
              L_regexp_data->m_nb_match = (L_regexp->find_value((char*)"nbexpr") == NULL) ?
                1 : atoi(L_regexp->find_value((char*)"nbexpr")) ;
              L_regexp_data->m_sub_match = (L_regexp->find_value((char*)"subexpr") == NULL) ? 
                0 : atoi(L_regexp->find_value((char*)"subexpr")) ;
              L_regexp_data->m_line = (L_regexp->find_value((char*)"line") == NULL) ?
                -1 : atoi(L_regexp->find_value((char*)"line")) ;

              (L_fielddef_data->m_expr_list)->push_back(L_regexp_data);
              
            } // if (strcmp(L_regexp->get_name(), (char*)"regexp") == 0)
          } // for(L_regexp_it...

          P_fielddef_list->push_back(L_fielddef_data);
        }
      }
    }
  } // if (L_ret != -1)

  GEN_DEBUG(1, "C_ProtocolText::analyze_from_xml() end");
  return (L_ret);
}


C_ProtocolText::T_pFieldDesc 
C_ProtocolText::check_field(T_pFieldDef P_field_def, bool P_header) {

  T_pFieldDesc                  L_ret = NULL ;
  T_FieldNameMap::iterator      L_it ;
  size_t                        L_size = 0 ;

  T_ExpStrLst::iterator         L_list_it ;
  T_pRegExpStr                  L_reg_str ;
  int                           L_error_comp = 0 ;
  char                         *L_value = NULL ;
  int                           L_start = -1 ;
  int                           L_end = -1 ;
  int                           L_result = -1 ;


  GEN_DEBUG(1, "C_ProtocolText::check_field() start");

  L_it = m_fields_name_map->find(T_FieldNameMap::key_type(P_field_def->m_name));
  if (L_it != m_fields_name_map->end()) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "field [" << P_field_def->m_name << "] already defined");
    return (L_ret);
  }

  ALLOC_VAR(L_ret, T_pFieldDesc, sizeof(T_FieldDesc));

  L_size = strlen(P_field_def->m_name) ;
  ALLOC_TABLE(L_ret->m_name, char*, sizeof(char), L_size+1);
  strcpy(L_ret->m_name, P_field_def->m_name);
  
  L_ret->m_id = new_id();
  L_ret->m_header_body = P_header ;
  L_ret->m_format = NULL ;

  NEW_VAR(L_ret->m_reg_exp_lst, T_RegExpLst());

  for (L_list_it = P_field_def->m_expr_list->begin() ;
       L_list_it != P_field_def->m_expr_list->end();
       L_list_it ++) {
    L_reg_str = *L_list_it ;
    C_RegExp *L_cRegExp = NULL ;
    NEW_VAR(L_cRegExp, C_RegExp(L_reg_str->m_expr, &L_error_comp,
                                L_reg_str->m_nb_match,
                                L_reg_str->m_sub_match,
                                L_reg_str->m_line));

    if (L_error_comp == 0) {
      (L_ret->m_reg_exp_lst)->push_back(L_cRegExp);

      if (P_field_def->m_format) {
        if (L_value == NULL) {
          L_value = L_cRegExp->execute(P_field_def->m_format);
          if (L_value != NULL) {
            if (strcmp(L_value, (char*)"$(field-value)") != 0) {
              // format ok found => create a T_pValueData for 
              FREE_TABLE(L_value);
            } else {
              L_result = L_cRegExp->execute(P_field_def->m_format,
                                            &L_start, 
                                            &L_end);
            }
          }
        }
      }

    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "In field [" 
                << P_field_def->m_name 
                << "] bad regular expression ["
                << L_reg_str->m_expr << "]");
      return (NULL);
    }
  }

  if (P_field_def->m_format != NULL) {
    if (L_value == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "In field [" 
                << P_field_def->m_name 
                << "] bad format expression: need $(field-value) presence");
      return (NULL);
    } else {
      ALLOC_VAR(L_ret->m_format, T_pValueData, sizeof(T_ValueData));
      L_ret->m_format->m_type = E_TYPE_STRING ;
      L_ret->m_format->m_value.m_val_binary.m_size = 0 ;
      L_ret->m_format->m_value.m_val_binary.m_value = NULL ;

      *(L_ret->m_format) = valueFromString(P_field_def->m_format,
                                        E_TYPE_STRING,
                                        L_result);
      if (L_result == -1) {
        GEN_ERROR(E_GEN_FATAL_ERROR, "In field [" 
                  << P_field_def->m_name 
                  << "] bad format expression");
        FREE_TABLE(L_value);
        return (NULL);
      } 

      L_ret->m_format_value_start = L_start ;
      L_ret->m_format_value_end = L_end ;

      FREE_TABLE(L_value);
    }
  }



  m_fields_name_map->insert(T_FieldNameMap::value_type(P_field_def->m_name, L_ret));
  
  GEN_DEBUG(1, "C_ProtocolText::check_field() end");

  return (L_ret);
}



int C_ProtocolText::xml_interpretor(C_XmlData *P_def,
                                    char **P_name,
                                    T_pConfigValueList P_config_value_list) {

  int                           L_ret          = 0           ;
  T_pXmlData_List               L_data         = NULL        ;
  T_XmlData_List::iterator      L_it                         ;
  C_XmlData                    *L_dataDico     = NULL        ;
  bool                          L_headerFound  = false       ;

  T_FieldDefList::iterator      L_fielddef_it                ;
  T_pFieldDesc                  L_field_desc   = NULL        ;
  int                           L_id                         ;
  T_FieldHeaderList::iterator   L_message_it                 ;
  int                           L_i                          ;


  GEN_DEBUG(1, "C_ProtocolText::xml_interpretor() start");

  NEW_VAR(m_fields_name_map, T_FieldNameMap());
  m_fields_name_map->clear();

  L_data = P_def->get_sub_data() ;
  if (L_data == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "No protocol definition");
    L_ret = -1 ;
  } 


  if (L_ret != -1) {
    for (L_it = L_data->begin();
	 L_it != L_data->end();
	 L_it++) {
      if (strcmp((*L_it)->get_name(), "body-method") == 0) {
	NEW_VAR(m_def_method_list, T_DefMethodList());
	L_ret = analyze_body_method_from_xml (*L_it,m_def_method_list); 
	break ;
      }
    }
  }



  if (L_ret != -1) {
    for(L_it  = L_data->begin() ;
	L_it != L_data->end() ;
	L_it++) {
      
      // Message Header definition
      if (strcmp((*L_it)->get_name(), "header") ==0) {
	L_headerFound = true ;
	NEW_VAR(m_header_fields_list, T_FieldDefList());
        L_ret = analyze_from_xml (*L_it, 
                                  m_header_fields_list,
                                  &m_header_name,
                                  &m_header_type,
                                  false) ;
	if (L_ret == -1) break ;
      }
    }
    
    if (L_headerFound == false) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No header definition found for protocol ["
                << *P_name << "]");
      L_ret = -1 ;
    }
  } // if (L_ret != -1)
  
  if (L_ret != -1) {
    for (L_it = L_data->begin();
	 L_it != L_data->end();
	 L_it++) {
      if (strcmp((*L_it)->get_name(), "body") == 0) {
	NEW_VAR(m_body_fields_list, T_FieldDefList());
	L_ret = analyze_from_xml (*L_it, 
                                  m_body_fields_list, 
                                  &m_body_name,
                                  &m_body_type,
                                  true) ;
	break ;
      }
    }
  } // if (L_ret != -1)
  
  if (L_ret != -1) {
    if (!m_header_fields_list->empty()) {
      m_nb_header_fields = m_header_fields_list->size()  ;
      if (m_nb_header_fields > 0) {
        for (L_fielddef_it  = m_header_fields_list->begin();
             L_fielddef_it != m_header_fields_list->end();
             L_fielddef_it++) {
          L_field_desc = check_field ((*L_fielddef_it), true) ;
          if (L_field_desc == NULL) { L_ret = -1 ; break ; }
        }
      } 
    } 
  } // if (L_ret != -1)

  if (L_ret != -1) {
    if (!m_body_fields_list->empty()) {
      m_nb_body_fields = m_body_fields_list->size()  ;
      if (m_nb_body_fields > 0) {
        for (L_fielddef_it = m_body_fields_list->begin();
             L_fielddef_it != m_body_fields_list->end();
             L_fielddef_it++) {
          L_field_desc = check_field ((*L_fielddef_it), false) ; 
          if (L_field_desc == NULL) { L_ret = -1 ; break ; }
        }
      } 
    } 
  } // if (L_ret != -1)

  if (L_ret != -1) {
    for (L_it = L_data->begin();
	 L_it != L_data->end();
	 L_it++) {
      
      if (strcmp((*L_it)->get_name(), "dictionary") == 0) {
	L_dataDico = *L_it ;
        if (L_dataDico == NULL) {
          GEN_ERROR(E_GEN_FATAL_ERROR, "No dictionnary found (or no value) in protocol ["
                    << *P_name << "]");
          L_ret = -1 ;
        } else {
          L_ret = analyze_dictionnary_from_xml (L_dataDico);
        }
	break ;
      } 
    } 
  } // if (L_ret != -1)



  if (L_ret != -1) {
    L_id = 0 ;
    for (L_message_it = m_header_fields_dico->begin();
         L_message_it != m_header_fields_dico->end();
         L_message_it++) {

      ALLOC_TABLE(m_message_names_table[L_id],
  		  char*, sizeof(char),
  		  (strlen(L_message_it->m_name))+1);
      
      strcpy(m_message_names_table[L_id],L_message_it->m_name) ;

      m_message_decode_map->insert(T_DecodeMap::value_type(L_message_it->m_name,
                                                           L_id));

      L_id ++ ;      
    }
  }

  if (L_ret != -1) {
    L_id = find_field_id(m_header_type);
    if (L_id != -1) {
      set_message_type_field_id(L_id);
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "No definition found for field [" 
                << m_header_type << "] used for type definition");
      L_ret = -1 ;
    }
  }

  if (L_ret != -1) {
    T_FieldNameMap::iterator L_field_it;
    m_nb_fields_desc_table        = m_nb_header_fields + m_nb_body_fields    ;
    if (m_nb_fields_desc_table > 0) {

      ALLOC_TABLE(m_names_fields, char**, sizeof(char*),m_nb_fields_desc_table);
      for (L_i = 0; L_i < m_nb_fields_desc_table ; L_i++) {
	m_names_fields[L_i] = NULL ;
      } 

      ALLOC_TABLE(m_fields_desc_table,
                  T_pFieldDesc*,
                  sizeof(T_pFieldDesc),
                  m_nb_fields_desc_table);
      for (L_field_it = m_fields_name_map->begin();
           L_field_it != m_fields_name_map->end();
           L_field_it++) {

        L_id = L_field_it->second->m_id ;

        ALLOC_TABLE(m_names_fields[L_id],
                    char*, sizeof(char),
                    strlen((L_field_it->second)->m_name)+1);
        
        strcpy(m_names_fields[L_id],(L_field_it->second)->m_name) ;

        m_fields_desc_table[L_id]=L_field_it->second;
      }
    }
  }
  
  GEN_DEBUG(1, "C_ProtocolText::xml_interpretor() end");
  
  return (L_ret);
}


int C_ProtocolText::new_id() {
  m_id_counter ++ ;
  return(m_id_counter);
}


int C_ProtocolText::find_field_id (char*P_name) {

  int L_id = -1 ;
  T_FieldNameMap::iterator     L_it                      ;

  GEN_DEBUG(1, "C_ProtocolText::find_field_id() start");  

  // retrieve id and type of session id
  if (P_name != NULL ) {
    // retrieve session id in header field map (m_fields_name_map)
    L_it = m_fields_name_map->find(T_FieldNameMap::key_type(P_name));
    if (L_it != m_fields_name_map->end()) {
      L_id = (L_it->second)->m_id ;
    } 
  } 
  GEN_DEBUG(1, "C_ProtocolText::find_field_id() end");  

  return (L_id);
}


// analyze presence of configuration-parameters in dico
// bool                     L_found = false ;
// T_pParamDefList               L_config_params_dico = NULL ;
// T_ParamDefList::iterator      L_config_params_dico_it     ;
//    if (L_ret != -1) {
//      for (L_it = L_data->begin();
//  	 L_it != L_data->end();
//  	 L_it++) {
//        if (strcmp((*L_it)->get_name(), "configuration-parameters") == 0) {
//  	L_found = true ;
//  	NEW_VAR(L_config_params_dico, T_ParamDefList());
//  	L_ret = xml_configuration_parameters (*L_it, L_config_params_dico);
//  	break ;
//        }
//      }
    

//      if (L_ret != -1) {
//        if (L_found == true) {
//  	L_found = false ;
//  	if ((L_config_params_dico != NULL ) && (!L_config_params_dico->empty())) {
//  	  m_nb_config_params = L_config_params_dico->size();
//  	  if (m_nb_config_params != 0) {
//  	    // ctrl between parameters from config and dico
//  	    for (L_config_params_dico_it = L_config_params_dico->begin();
//  		 L_config_params_dico_it != L_config_params_dico->end();
//  		 L_config_params_dico_it++) {
//  	      L_ret = update_config_params (*L_config_params_dico_it, P_config_value_list) ;
//  	      if (L_ret == -1) { break;}
//  	    }

//  	  }
	
//  	} else {
//  	  GEN_ERROR(E_GEN_FATAL_ERROR, "Error in  configuration parameters definition for protocol ["
//  		    << *P_name << "]");
//  	  L_ret = -1 ;
//  	}
      
//        } else {
//  	GEN_ERROR(E_GEN_FATAL_ERROR, "No configuration parameters definition found for protocol ["
//  		  << *P_name << "]");
//  	L_ret = -1 ;
//        }
//      } else {
//        GEN_ERROR(E_GEN_FATAL_ERROR, "Error in configuration parameters definition found for protocol ["
//  		  << *P_name << "]");
//  	L_ret = -1 ;
//      }
//    }


int   C_ProtocolText::set_field_value(C_MessageText *P_msg, 
                                      int            P_id, 
                                      T_pValueData   P_value) {

  int                            L_ret        = 0    ;
  T_pFieldDesc                   L_field_desc = NULL ;
  T_RegExpLst::iterator          L_regexp_it         ; 
  C_RegExp                      *L_cRegExp    = NULL ; 
  int                            L_start      = -1   ;
  int                            L_end        = -1   ;

  char                          *L_string_value = NULL ;
  bool                           L_match = false ;


  GEN_DEBUG(1, "C_ProtocolText::set_field_value() start");  

  if (P_id <= (m_nb_body_fields + m_nb_header_fields)) {
    L_field_desc = m_fields_desc_table[P_id] ;
    
    L_string_value = P_msg->get_text_value(L_field_desc->m_header_body) ;
    for (L_regexp_it = (L_field_desc->m_reg_exp_lst)->begin();
         L_regexp_it != (L_field_desc->m_reg_exp_lst)->end();
         L_regexp_it++) {
      L_cRegExp = *L_regexp_it ;
      
      
      L_ret = L_cRegExp->execute(L_string_value,
                                 &L_start, 
                                 &L_end);
      
      if (L_ret == 0) { 
        L_match = true ;  
        break ; 
      }
    } // for rege

    if (L_match == false) {
      // first, insert format
      T_pValueData L_value = P_msg->get_data_value(L_field_desc->m_header_body) ;
            
      if (L_field_desc->m_format != NULL) {
        copyBinaryVal(*L_value,  
                      L_value->m_value.m_val_binary.m_size, 
                      (L_field_desc->m_format)->m_value.m_val_binary.m_size,
                      *(L_field_desc->m_format));
      

      // re-execute match 
        L_ret = L_cRegExp->execute((char*)L_value->m_value.m_val_binary.m_value,
                                   &L_start,
                                   &L_end);
      } else {
        GEN_LOG_EVENT(LOG_LEVEL_TRAFFIC_ERR, 
		      "no match regexp for your field ["
		      << L_field_desc->m_name << "]");
        L_ret = -1 ;
      }
    }

    if (L_ret != -1) {
      P_msg->set_text_value(L_field_desc->m_header_body,P_value,L_start,L_end);
    }
  }

  GEN_DEBUG(1, "C_ProtocolText::set_field_value() end");  
  return (L_ret);
}

char* C_ProtocolText::get_field_value(C_MessageText *P_msg, 
                                      int P_id,
                                      C_RegExp  *P_regexp_data) {

  char                      *L_value      = NULL ;
  T_pFieldDesc               L_field_desc = NULL ;

  GEN_DEBUG(1, "C_ProtocolText::get_field_value() start");  

  if (P_id <= (m_nb_body_fields + m_nb_header_fields)) {
    L_field_desc = m_fields_desc_table[P_id] ;
    L_value = P_regexp_data->execute(P_msg->get_text_value(L_field_desc->m_header_body));
  }

  GEN_DEBUG(1, "C_ProtocolText::get_field_value() end");  

  return (L_value);
  
}


char* C_ProtocolText::get_field_value(C_MessageText *P_msg, int P_id) {

  char                      *L_value      = NULL ;
  T_pFieldDesc               L_field_desc = NULL ;
  T_RegExpLst::iterator      L_regexp_it         ; 
  C_RegExp*                  L_cRegExp    = NULL ; 

  GEN_DEBUG(1, "C_ProtocolText::get_field_value() start");  

  if (P_id <= (m_nb_body_fields + m_nb_header_fields)) {
    L_field_desc = m_fields_desc_table[P_id] ;
    for (L_regexp_it = (L_field_desc->m_reg_exp_lst)->begin();
         L_regexp_it != (L_field_desc->m_reg_exp_lst)->end();
         L_regexp_it++) {
      L_cRegExp = *L_regexp_it ;
      L_value = L_cRegExp->execute(P_msg->get_text_value(L_field_desc->m_header_body));

      if (L_value != NULL) break ;
    }
  }

  GEN_DEBUG(1, "C_ProtocolText::get_field_value() end");  

  return (L_value);
  
}

char* C_ProtocolText::get_field_value_to_check(C_MessageText *P_msg, 
                                               int P_id,
                                               int *P_size) {

  char                          *L_value      = NULL ;
  T_pFieldDesc                   L_field_desc = NULL ;
  T_RegExpLst::iterator          L_regexp_it         ; 
  C_RegExp*                      L_cRegExp    = NULL ;
  int                            L_size       = -1   ;

  GEN_DEBUG(1, "C_ProtocolText::get_field_value_to_check() start");  

  if (P_id <= (m_nb_body_fields + m_nb_header_fields)) {
    L_field_desc = m_fields_desc_table[P_id] ;
    for (L_regexp_it = (L_field_desc->m_reg_exp_lst)->begin();
         L_regexp_it != (L_field_desc->m_reg_exp_lst)->end();
         L_regexp_it++) {
      L_cRegExp = *L_regexp_it ;

      L_value = L_cRegExp->execute(P_msg->get_text_value(L_field_desc->m_header_body),
                                   &L_size);
      if (L_value != NULL) {
        *P_size = L_size ;
        break ;
      }
    }
  }

  GEN_DEBUG(1, "C_ProtocolText::get_field_value_to_check() end");  

  return (L_value);
  
}

C_ProtocolText::T_pManagementSessionTextId 
C_ProtocolText::get_manage_session_elt(int P_id) {
  
  T_ManagementSessionTextId  *L_ret = NULL ;
  
  if ((P_id < m_value_sessions_table_size) && (P_id >= 0)) {
    L_ret = &m_value_sessions_table[P_id] ;
  }
  return (L_ret) ;
}


int C_ProtocolText::get_nb_management_session () {
  return (m_value_sessions_table_size);
}

int C_ProtocolText::get_message_id (char *P_name) {
  int L_ret = -1 ;
  T_DecodeMap::iterator L_it ;
  L_it = m_message_decode_map->find(T_DecodeMap::key_type(P_name)) ;
  if (L_it != m_message_decode_map->end()) {
    L_ret = L_it->second ;
  }
  return (L_ret);
}

iostream_output& C_ProtocolText::print_header_msg(iostream_output& P_stream,
                                                  int              P_id,
                                                  T_pValueData     P_header) {

  P_stream << m_message_names_table[P_id] ;
  print_data_msg(P_stream,P_header);
  return (P_stream);

}

iostream_output& C_ProtocolText::print_data_msg(iostream_output& P_stream,
                                            T_pValueData     P_data) {

 
  static char L_hexa_buf [50] ;
  static char L_ascii_buf [50] ;
  const  size_t L_cNum = 16 ; 
  size_t L_j, L_nb ;
  unsigned char*L_cur ;
  size_t L_buffer_size = P_data->m_value.m_val_binary.m_size;

  GEN_DEBUG(1, "C_ProtocolText::print_data() start");



  L_nb = L_buffer_size / L_cNum ;
  L_cur = P_data->m_value.m_val_binary.m_value ;
  if (L_cur != NULL) {
    for (L_j = 0 ; L_j < L_nb; L_j++) {
      P_stream << iostream_endl ;
      P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) ;

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
      P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) ;

      pretty_binary_printable_buffer(L_cur, L_nb, L_hexa_buf, L_ascii_buf);
      P_stream << "[" ;
      P_stream << L_hexa_buf ;
      P_stream << "] <=> [" ;
      P_stream << L_ascii_buf;
      P_stream << "]" ;
    }
    
    
    P_stream << iostream_endl ;

  } else {
    P_stream << "NULL(empty)";
  }
  
  GEN_DEBUG(1, "C_ProtocolText::print_data() end");

  return (P_stream);

}



char* C_ProtocolText::find_text_value (char *P_buf, char *P_field) {

  char *L_value = NULL ;

  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[3] ;
  size_t     L_size = 0 ;

  string_t   L_string = "" ;
  
  L_string  = "([[:blank:]]*" ;
  L_string += P_field ;
  L_string += "[[:blank:]]*=[[:blank:]]*)([^[:blank:];]+)";

  L_status = regcomp (&L_reg_expr, 
		      L_string.c_str(),
		      REG_EXTENDED) ;

  if (L_status != 0) {
    regerror(L_status, &L_reg_expr, L_buffer, 100);
    regfree (&L_reg_expr) ;

  } else {
  
    L_status = regexec (&L_reg_expr, P_buf, 3, L_pmatch, 0) ;
    regfree (&L_reg_expr) ;

    if (L_status == 0) {
      
      L_size = L_pmatch[2].rm_eo - L_pmatch[2].rm_so ;
      
      ALLOC_TABLE(L_value, char*, sizeof(char), L_size+1);
      memcpy(L_value, &(P_buf[L_pmatch[2].rm_so]), L_size);
      L_value[L_size]='\0' ;
      
    } 
  }


  return (L_value);
}

int C_ProtocolText::analyze_body_method_from_xml(C_XmlData *P_data, 
                                                 T_DefMethodList *P_def_method_list) { 



  int                       L_ret                = 0        ;

  T_pXmlData_List           L_defmethod_xml_list = NULL     ;
  C_XmlData                *L_defmethod          = NULL     ;
  T_XmlData_List::iterator  L_defmethod_it                  ;

  T_DefMethod               L_defmethod_data                ;
  char                     *L_method                        ;

  GEN_DEBUG(1, "C_ProtocolText::analyze_body_method_from_xml() start");

  L_defmethod_xml_list = P_data->get_sub_data() ;

  for(L_defmethod_it  = L_defmethod_xml_list->begin() ;
      L_defmethod_it != L_defmethod_xml_list->end() ;
      L_defmethod_it++) {

    L_defmethod = *L_defmethod_it ;

    if (strcmp(L_defmethod->get_name(), (char*)"defmethod") == 0) {
      L_defmethod_data.m_name = L_defmethod->find_value((char*)"name") ;
      if (L_defmethod_data.m_name == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "defmehod name value is mandatory");
	L_ret = -1 ;
	break ;
      }

      L_method = L_defmethod->find_value((char*)"method") ;
      if (L_method == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "defmehod method value is mandatory");
	L_ret = -1 ;
	break ;
      } else {
        if (strcmp(L_method, (char*)"length") == 0) {
          L_defmethod_data.m_method = E_BODY_METHOD_LENGTH ;
        } else if (strcmp(L_method , (char*)"parser") == 0 ) {
          L_defmethod_data.m_method = E_BODY_METHOD_PARSE ;
        } else {
          GEN_ERROR(E_GEN_FATAL_ERROR, 
                    "Body method for protocol definition must be length or parser");
          L_ret = -1 ;
          break ;
        }
      }

      

      L_defmethod_data.m_param = L_defmethod->find_value((char*)"param") ;
      if (L_defmethod_data.m_param == NULL) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "defmehod param value is mandatory");
	L_ret = -1 ;
	break ;
      }

      if (L_ret == -1) { 
	break ; 
      } else {
        L_defmethod_data.m_default = L_defmethod->find_value((char*)"default") ;
	P_def_method_list->push_back(L_defmethod_data);
      }

    } // strcmp defmethod
  }

  GEN_DEBUG(1, "C_ProtocolText::analyze_body_method_from_xml() end");

  return (L_ret);
}


C_MessageText* C_ProtocolText::create(C_ProtocolText *P_protocol,
                                      char           *P_header,
                                      char           *P_body) {
  
  // Regexep 
  int                      L_result = 0 ;
  T_pValueData             L_value_header, L_value_body ;
  C_MessageText           *L_msg = NULL ;
  int                      L_id = -1 ;

  char                    *L_buffer_header          = NULL   ;
  char                    *L_buffer_body            = NULL   ;
    
  GEN_DEBUG(1, "C_ProtocolText::create() start");

  ALLOC_VAR(L_value_header, T_pValueData, sizeof(T_ValueData));
  L_value_header->m_type = E_TYPE_STRING ;
  L_value_header->m_value.m_val_binary.m_size = 0 ;
  L_value_header->m_value.m_val_binary.m_value = NULL ;

  if (P_header != NULL) {

    L_buffer_header = format_buffer(P_header);
    if (L_buffer_header == NULL) {
      L_result = -1 ;
    } else {
      *L_value_header = valueFromString(L_buffer_header, E_TYPE_STRING , L_result);
    }
  }

  if (L_result == -1) { 
    FREE_VAR(L_value_header);
    FREE_TABLE(L_buffer_header);
    return(NULL); 
  }

  ALLOC_VAR(L_value_body, T_pValueData, sizeof(T_ValueData));
  L_value_body->m_type = E_TYPE_STRING ;
  L_value_body->m_value.m_val_binary.m_size = 0 ;
  L_value_body->m_value.m_val_binary.m_value = NULL ;
  if (P_body != NULL) {
    L_buffer_body = format_buffer(P_body);
    if (L_buffer_body == NULL) {
      L_result = -1 ;
    } else {
      *L_value_body = valueFromString(L_buffer_body, E_TYPE_STRING , L_result);
    }
  }


  if (L_result == -1) { 
    FREE_VAR(L_value_header);
    FREE_VAR(L_value_body);

    FREE_TABLE(L_buffer_header);
    FREE_TABLE(L_buffer_body);

    return (NULL); 
  } 

  NEW_VAR(L_msg, C_MessageText(P_protocol, 
                               L_value_header, L_value_body, &L_id));

  if (L_id == -1) {
    DELETE_VAR(L_msg) ;
  } 


  FREE_TABLE(L_buffer_header);
  FREE_TABLE(L_buffer_body);

  GEN_DEBUG(1, "C_ProtocolText::create() end");
  return (L_msg);
}

void C_ProtocolText::set_body_separator (char*  P_body_separator) {
  m_body_separator = P_body_separator ;
  m_body_separator_size = strlen(m_body_separator);
}

void C_ProtocolText::set_message_type_field_id (int P_id) {
  m_message_type_field_id = P_id ;
}

void C_ProtocolText::set_session_id_id (int P_id) {
  m_session_id_id = P_id ;
}


void C_ProtocolText::set_number_methods (int P_nb) {
  int L_i ;

  m_nb_methods = P_nb ;
  ALLOC_TABLE(m_methods,
              T_BodyDecodeData**,
              sizeof(T_pBodyDecodeData),
              m_nb_methods);
  for (L_i = 0 ; L_i < m_nb_methods; L_i++) {
    m_methods[L_i] = NULL ;
  }
  
}


void C_ProtocolText::set_body_decode_method (int                     P_index,
                                             T_BodyMethodType        P_type,
                                             T_BodyDecodeMethod      P_method,
                                             void                   *P_param) {
  if (m_methods[P_index] == NULL) {
    ALLOC_VAR(m_methods[P_index],
              T_pBodyDecodeData,
              sizeof(T_BodyDecodeData));
  }
  m_methods[P_index]->m_decode = P_method ;
  switch (P_type) {
  case E_BODY_METHOD_LENGTH:
    (m_methods[P_index]->m_param_decode).m_id =  *((int*)P_param) ;
    break ;
  case E_BODY_METHOD_PARSE:
    (m_methods[P_index]->m_param_decode).m_parser = (T_ParserFunction) P_param ;
    break ;
  }
}


void C_ProtocolText::set_encode_method (int                 P_index,
                                        T_BodyMethodType    P_type,
                                        T_EncodeMethod      P_method,
                                        void               *P_param) {
  if (m_methods[P_index] == NULL) {
    ALLOC_VAR(m_methods[P_index],
              T_pBodyDecodeData,
              sizeof(T_BodyDecodeData));
  }
  m_methods[P_index]->m_encode = P_method ;
  switch (P_type) {
  case E_BODY_METHOD_LENGTH:
    (m_methods[P_index]->m_param_encode).m_id =  *((int*)P_param) ;
    break ;
  case E_BODY_METHOD_PARSE:
    (m_methods[P_index]->m_param_encode).m_parser = (T_ParserFunction) P_param ;
    break ;
  }
}


void C_ProtocolText::destroy_methods () {
  int L_i ;
  for (L_i = 0 ; L_i < m_nb_methods; L_i++) {
    FREE_VAR(m_methods[L_i]);
  }
  FREE_TABLE(m_methods);
  m_nb_methods = 0 ;
}

void C_ProtocolText::use_open_id () {
  m_session_method = &C_MessageText::getSessionFromOpenId ;
}

char* C_ProtocolText::skip_blank(char    *P_ptr, 
                                 char    *P_buffer, 
                                 size_t   P_size_buffer,
                                 size_t  *P_size) {

  char     *L_blank_ptr    = NULL     ;
  char     *L_new_ptr      = P_ptr    ;


  L_blank_ptr = P_ptr ;
  while (((L_blank_ptr) && (L_blank_ptr < (P_buffer + P_size_buffer))) &&
         ((*L_blank_ptr == ' ') ||
          (*L_blank_ptr == '\t'))) { L_blank_ptr++ ; }
  if (L_blank_ptr != P_ptr) {
    *(P_size) = (L_blank_ptr - P_ptr) ;
    L_new_ptr = L_blank_ptr ;
  }

  return (L_new_ptr) ;
}


char * C_ProtocolText::format_buffer(char* P_buffer) {

  size_t    L_size         = 0        ;

  size_t    L_size_buffer  = 0        ;
  size_t    L_size_end     = 0        ;
  
  char     *L_pos          = NULL     ;
  char     *L_ptr          = P_buffer ;

  char     *L_result       = NULL     ;
  char     *L_new          = NULL     ;

  bool      L_skip_blank   = true     ;
  size_t    L_size_blank   = 0        ;


  if ((P_buffer != NULL) && 
      ((L_size_buffer = strlen(P_buffer)) > 0 )) {

    L_size_end = L_size_buffer ;

    ALLOC_TABLE(L_result, 
		char*, 
		sizeof(char), 
		(2*L_size_buffer));
    

    if ((strchr(L_ptr,'\n')) == NULL) {

      L_new = L_result ;
      L_size = L_size_buffer ;

      // skip blank
      if (L_skip_blank) {
        L_ptr = skip_blank(L_ptr,P_buffer, L_size_buffer, &L_size_blank) ;
        L_size -= L_size_blank ;
      }

      memcpy(L_new, L_ptr, L_size);
      L_new += (L_size - 1) ;
      if (*L_new != '\r') {
        L_new += 1 ;
        *L_new = '\r' ;
      }
      L_new += 2 ;
      *L_new = '\0' ;
      *(L_new-1) = '\n' ;

    } else {
      // if '\n' exists

      while(   (L_ptr) 
            && (L_pos = strchr(L_ptr,'\n')) != NULL) {

        L_size_blank = 0 ;
        // L_size : from start to '\n' not included
	L_size = L_pos - L_ptr ;
        // skip blank
        if (L_skip_blank) {

          L_ptr = skip_blank(L_ptr,P_buffer, L_size_buffer, &L_size_blank) ;
          L_size -= L_size_blank ;
          L_size_end -= L_size_blank ;

        }

        if (L_new == NULL) { L_new = L_result ; } else { L_new += 1 ; }
	memcpy(L_new, L_ptr, L_size);
	L_new += (L_size - 1) ;
        // test end needed ? for L_ptr
        if ((L_pos + 1) <= (P_buffer+L_size_buffer)) { 
          L_ptr = L_pos + 1 ; 
        } else { 
          L_ptr = NULL ; 
        }

        L_size_end -= (L_size + 1) ;

        if (*L_new != '\r') {
          L_new += 1 ;
          *(L_new) = '\r' ;
        }
        L_new += 1 ;
        *(L_new) = '\n' ;

      } // while

        
      // ctrl the end of buffer
      if (L_size_end > 0) {

        L_size = L_size_end ;

        // skip blank
        if (L_skip_blank) {

          L_ptr = skip_blank(L_ptr,P_buffer, L_size_buffer, &L_size_blank) ;
          L_size -= L_size_blank ;
        }

        if (L_size) {
          L_new +=1 ;

          memcpy(L_new, L_ptr, L_size);
          L_new += (L_size-1) ;
        
          if (*L_new != '\r') {
            L_new += 1 ;
            *(L_new) = '\r' ;
          }
          L_new += 2 ;
          *L_new = '\0' ;
          *(L_new-1) = '\n' ;
        } else {
          // add final '\0' 
          L_new += 1 ;
          *L_new = '\0' ;
        }
      } else {
        // add final '\0' 
        L_new += 1 ;
        *L_new = '\0' ;
        
      }
    }
  }

  if (L_result != NULL) {
    L_ptr = L_result ;
    while ((L_ptr = strstr(L_ptr, "\r\n\r\n")) != NULL ) {
      memmove(L_ptr+2, L_ptr+4, strlen(L_ptr+4));
      L_ptr += 2 ;
    }
  }

  return (L_result);
}



