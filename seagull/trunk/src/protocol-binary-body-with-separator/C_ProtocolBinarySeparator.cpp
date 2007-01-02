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

#include "C_ProtocolBinarySeparator.hpp"

#include "GeneratorTrace.hpp"
#include "BufferUtils.hpp"
#include "GeneratorError.h"
#include "string_t.hpp"
#include "C_MessageBinarySeparator.hpp"


C_ProtocolBinarySeparator::C_ProtocolBinarySeparator() : C_ProtocolBinary() {
  GEN_DEBUG(1, "C_ProtocolBinarySeparator::C_ProtocolBinarySeparator() start");
  m_header_body_field_separator_size = 0;
  m_session_id = -1 ; 
  GEN_DEBUG(1, "C_ProtocolBinarySeparator::C_ProtocolBinarySeparator() end" );
}

C_ProtocolBinarySeparator::~C_ProtocolBinarySeparator() {
  GEN_DEBUG(1, "C_ProtocolBinarySeparator::~C_ProtocolBinarySeparator() start" );
  m_header_body_field_separator_size = 0;
  GEN_DEBUG(1, "C_ProtocolBinarySeparator::~C_ProtocolBinarySeparator() end" );
}

void  C_ProtocolBinarySeparator::construction_data(C_XmlData            *P_def, 
						   char                **P_name,
						   T_pContructorResult   P_res) {

  C_ProtocolBinary::construction_data(P_def, P_name, P_res);

  GEN_DEBUG(1, "C_ProtocolBinarySeparator::construction_data() start" );

  if (*P_res == E_CONSTRUCTOR_OK) {
    if (m_header_body_field_separator != NULL) {
      m_header_body_field_separator_size 
        = strlen(m_header_body_field_separator) ;
    }

    m_session_id = m_msg_id_id ; 
    if (m_session_id_position == -1) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
		"session-id-position not found in dictionnary");
      *P_res = E_CONSTRUCTOR_KO ;
    } 
  }
  GEN_DEBUG(1, "C_ProtocolBinarySeparator::construction_data() end" );
}

C_MessageFrame* C_ProtocolBinarySeparator::create_new_message(C_MessageFrame *P_msg) {

  C_MessageBinarySeparator *L_msg ;

  GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() start: msg pt");
  NEW_VAR(L_msg, C_MessageBinarySeparator(this));
  if (P_msg != NULL) {
    (*L_msg) = *((C_MessageBinarySeparator*)P_msg) ;
  }
  GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() end: msg pt");
  return (L_msg);
}

C_MessageFrame* C_ProtocolBinarySeparator::create_new_message(void                *P_xml, 
		                                     T_pInstanceDataList  P_list,
						     int                 *P_nb_value) {


  C_XmlData *L_Xml = (C_XmlData*) P_xml ;

  bool                             L_msgOk = true  ;
  C_MessageBinarySeparator        *L_msg           ;
  char                            *L_currentName   ;
  
  C_ProtocolFrame::T_MsgError      L_decode_result ;
  
  unsigned char                   *L_buf           ;
  size_t                           L_size          ;
  
  int                              L_header_val_id ;
  int                              L_body_val_id   ;
  
  GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() start: xml");
  
  // Allocate a new message

  NEW_VAR(L_msg, C_MessageBinarySeparator(this));

  // Get the message name from XML definition
  L_currentName = L_Xml -> get_name () ;

  GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() msg xml name is "
	    << L_currentName );
      
  if (strcmp(L_currentName, (char*)"CDATA") == 0) {
	
    // direct buffer data definition
    L_currentName = L_Xml->find_value((char*)"value");
    
    L_buf = convert_hexa_char_to_bin(L_currentName, &L_size);
    (void) L_msg -> decode(L_buf, L_size, &L_decode_result);
    
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
      GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() " 
		<< "the name of this message is " << L_currentName );

      L_header_val_id = get_header_value_id(L_currentName) ;

      GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() " 
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
	char                     *L_bodyName, *L_bodyValue ;
	C_ProtocolBinary::T_BodyValue   L_bodyVal ;
        T_TypeType                L_type ; 

	L_msg->set_header_id_value(L_header_val_id);
	
	L_listBodyVal = L_Xml -> get_sub_data() ;
	if (L_listBodyVal != NULL) {
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
		
	      GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() "
			<< "L_bodyName is    [" << L_bodyName << "]");

	      L_body_val_id = get_header_body_value_id(L_bodyName) ;
	      if (L_body_val_id == -1) {
		GEN_ERROR(E_GEN_FATAL_ERROR,
			  "No definition found for ["
			  << L_bodyName << "]");
		L_msgOk = false ;
		break ;
	      } 
		
	      GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() " 
			<< "L_body_val_id is " 
			<< L_body_val_id );
	      
              L_type = get_body_value_type (L_body_val_id) ;
	      if (L_type 
		  == E_TYPE_GROUPED) {

	        GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() " 
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
		
		GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() " 
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
		
	      }
            } else if (strcmp(L_bodyData->get_name(),
                              (char*)"setfield")==0) {
              unsigned long   L_val_setfield = 0 ;
              int             L_id_setfield   = 0 ;
	      L_msgOk = (analyze_setfield(L_bodyData, &L_id_setfield ,&L_val_setfield) == -1) 
                ? false : true  ;
              if (L_msgOk) {
                L_msg->C_MessageBinary::set_header_value(L_id_setfield, L_val_setfield);
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
	}
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

  GEN_DEBUG(1, "C_ProtocolBinarySeparator::create_new_message() end: xml");

  return (L_msg);
}


C_ProtocolFrame::T_MsgError C_ProtocolBinarySeparator::encode_body (int            P_nbVal, 
								    T_pBodyValue   P_val,
								    unsigned char *P_buf, 
								    size_t        *P_size) {

  unsigned char     *L_ptr = P_buf ;
  int                L_i, L_body_id ;
  size_t             L_total_size   = 0 ;
  size_t             L_current_size = 0 ;
  T_pHeaderBodyValue L_body_fieldValues  ;
  T_pBodyValue       L_body_val ;
  unsigned long      L_valueSize  ;
  int                L_type_id ;
  T_TypeType         L_type ;

  unsigned char *L_save_length_ptr = NULL;
  unsigned long  L_save_length = 0;
  size_t         L_length_size = 0;

  size_t         L_sub_size ;
  C_ProtocolFrame::T_MsgError  L_error = C_ProtocolFrame::E_MSG_OK;
  

  GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body() start");

  L_total_size = 0 ;
  L_current_size = 0 ;

  for (L_i = 0; L_i < P_nbVal ; L_i ++) {

    L_body_val = &P_val[L_i] ;
    L_body_id = L_body_val->m_id  ;



    if (m_stats) {
      if (L_body_val -> m_value.m_val_binary.m_size != 0) {
        m_stats->updateStats (E_MESSAGE_COMPONENT,
                              E_SEND,
                              L_body_id);
      }
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
   
    // now add the value of the body
    switch (L_type) {

    case E_TYPE_NUMBER:
      convert_ul_to_bin_network(L_ptr,
				L_valueSize,
				L_body_val -> m_value.m_val_number);

      GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body()  with number value = " 
                   << L_body_val->m_value.m_val_number);
      break ;

    case E_TYPE_SIGNED:
      convert_l_to_bin_network(L_ptr,
			       L_valueSize,
			       L_body_val -> m_value.m_val_signed);

      GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body()  with signed value = " 
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

      GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body()  with string value (size: " 
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

      GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body()  with struct value = [" 
                   << L_body_val -> m_value.m_val_struct.m_id_1 << ";" 
                   << L_body_val -> m_value.m_val_struct.m_id_2);
      }
	break ;

    case E_TYPE_GROUPED:
      GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body()  with grouped value " );

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

      GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body()  with number64 value = " 
                   << L_body_val->m_value.m_val_number_64);
      break ;

    case E_TYPE_SIGNED_64:
      convert_ll_to_bin_network(L_ptr,
			        L_valueSize,
			        L_body_val -> m_value.m_val_signed_64);

      GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body()  with signed64 value = " 
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

    // Now add the separator
    if (m_header_body_field_separator_size > 0) {
      memcpy(L_ptr,
             m_header_body_field_separator,
             m_header_body_field_separator_size);
      
      L_total_size += m_header_body_field_separator_size ;
      L_ptr += m_header_body_field_separator_size ;
    }
  } // for (L_i ...

  if (L_error == C_ProtocolFrame::E_MSG_OK) {
    *P_size = L_total_size ;
  }

  GEN_DEBUG(1, "C_ProtocolBinarySeparator::encode_body() end");

  return (L_error);
}



int C_ProtocolBinarySeparator::decode_body(unsigned char *P_buf, 
                                           size_t         P_size,
                                           T_pBodyValue   P_valDec,
                                           int           *P_nbValDec,
                                           int           *P_headerId) {

  int                   L_max_values         = *P_nbValDec      ;
  int                   L_ret                =  0               ; 
  unsigned long         L_total_size, L_data_size, L_data_type  ;
  int                   L_i                                     ;

  unsigned char        *L_buf            = NULL                 ;
  char                  L_char                                  ;

  unsigned char        *L_ptr            = NULL                 ; 
  char                 *L_pos                                   ;


  GEN_DEBUG(1, "C_ProtocolBinarySeparator::decode_body() start");

  if (memcmp((P_buf + (P_size - m_header_body_field_separator_size)),
             m_header_body_field_separator,
             m_header_body_field_separator_size) == 0) {
    ALLOC_TABLE(L_buf, unsigned char*, sizeof(unsigned char), P_size +1);
    memcpy(L_buf, P_buf, P_size);
    L_buf[P_size] = 0 ;
  } else {
    L_char = P_buf[(P_size-1)] ;
    P_buf[(P_size-1)] = 0 ;
  }
  
  L_ptr = (L_buf == NULL) ? P_buf : L_buf ;
  
  L_total_size = 0 ;
  *P_nbValDec  = 0 ;
  
  L_i = 0 ;

  while (L_total_size < P_size) {

    L_data_size = 0  ;
    L_data_type = 0  ;

    L_pos = strstr((char*)L_ptr, m_header_body_field_separator);
    if (L_pos != NULL) {
      L_data_size  = (L_pos - (char*)L_ptr) ;
    }

    if ((L_total_size + L_data_size) <= P_size) {
      
      if (L_i == m_session_id_position) {
        P_valDec[L_i].m_id = m_session_id ;
      } else {
        P_valDec[L_i].m_id = -1 ;
      }

      P_valDec[L_i].m_value.m_val_binary.m_size = L_data_size ;
      
      ALLOC_TABLE(P_valDec[L_i].m_value.m_val_binary.m_value,
                  unsigned char*,
                  sizeof(unsigned char),
                  L_data_size);

      memcpy(P_valDec[L_i].m_value.m_val_binary.m_value,
             L_ptr,
             L_data_size);

      L_ptr = (unsigned char*)L_pos + 1 ;
      L_total_size += L_data_size + m_header_body_field_separator_size ;
      L_i++ ;      
    } else {
      GEN_ERROR (E_GEN_FATAL_ERROR, "message size error (body size)");
      L_ret = -1 ;
      break ;
    }
    
    if (L_ret == -1) break ;

    if (L_i == L_max_values) {
      GEN_FATAL(E_GEN_FATAL_ERROR, "Maximum number of values reached ["
                << L_max_values << "]");
      L_ret = -1 ;
      break ;
    }
  } // End while

  if (L_ret == -1 ) {
    if (!L_buf) { P_buf[(P_size-1)] = L_char ; }
  }

  *P_nbValDec  = L_i ;

  
  if (L_total_size != P_size) { 
    if (!L_buf) { P_buf[(P_size-1)] = L_char ; }
    L_ret = -1 ; 
  } 

  if (L_buf) {
    FREE_TABLE(L_buf);
  } 

  GEN_DEBUG(1, "C_ProtocolBinarySeparator::decode_body() end");

  return (L_ret) ;

}
