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

#include "C_MessageText.hpp"
#include "C_ProtocolText.hpp"
#include "ProtocolData.hpp"


#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "C_ContextFrame.hpp"
#include "ReceiveMsgContext.h"
#include "C_CallContext.hpp"

#define m_body_separator_size           m_protocol->m_body_separator_size
#define m_body_separator                m_protocol->m_body_separator
#define m_message_type_field_id         m_protocol->m_message_type_field_id
#define m_session_id_id                 m_protocol->m_session_id_id
#define m_session_method                m_protocol->m_session_method
#define m_field_separator_size          m_protocol->m_field_separator_size
#define m_field_separator               m_protocol->m_field_separator

#define m_field_body_separator_size     m_protocol->m_field_body_separator_size
#define m_field_body_separator          m_protocol->m_field_body_separator

#define m_names_fields                  m_protocol->m_names_fields

#define m_methods                       m_protocol->m_methods
#define m_nb_methods                    m_protocol->m_nb_methods


C_ProtocolFrame::T_MsgError C_MessageText::EncodeWithContentLength (int P_index) {

  C_ProtocolFrame::T_MsgError L_error        = C_ProtocolFrame::E_MSG_OK  ;

  char                        L_size_char[10]                             ;
  size_t                      L_size_body    = 0                          ;
  T_ValueData                 L_size                                      ;
  int                         L_result       = -1                         ;
  int                         L_ret          = 0                          ;

  GEN_DEBUG(1, "C_MessageText::EncodeWithContentLength() start");

  L_size_body = (m_body == NULL) ? 0 : m_body->m_value.m_val_binary.m_size ;
  snprintf(L_size_char, 10, "%d", L_size_body);
  L_size = valueFromString(L_size_char, E_TYPE_STRING, L_result);

  if (L_result == -1) { 
    L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
  } else {
    L_size.m_type = E_TYPE_STRING ;
    L_ret = m_protocol->set_field_value(this, (m_methods[P_index]->m_param_encode).m_id, &L_size) ;
    if (L_ret == -1) {
      L_error = C_ProtocolFrame::E_MSG_ERROR_ENCODING ;
    }
    resetMemory(L_size);
  }

  GEN_DEBUG(1, "C_MessageText::EncodeWithContentLength() end");
  return (L_error);
}

C_ProtocolFrame::T_MsgError C_MessageText::NoEncode (int P_index) {

  C_ProtocolFrame::T_MsgError L_error        = C_ProtocolFrame::E_MSG_OK  ;

  GEN_DEBUG(1, "C_MessageText::EncodeWithContentLength() start");
  GEN_DEBUG(1, "C_MessageText::EncodeWithContentLength() end");

  return (L_error);
}


C_ProtocolFrame::T_MsgError C_MessageText::DecodeBodyWithParser (int P_index,
                                                                 char *P_ptr,
                                                                 size_t *P_size) {

  C_ProtocolFrame::T_MsgError L_error        = C_ProtocolFrame::E_MSG_OK  ;
  size_t                      L_size_body = *P_size ;

  // std::cerr << "DecodeBodyWithParser start "  << std::endl;
  // std::cerr << "DecodeBodyWithParser L_size_body is " << L_size_body << std::endl;

  L_error = (*(((m_methods[P_index])->m_param_decode).m_parser))(P_ptr, 
                                                                 P_size,
                                                                 (char *)m_header->m_value.m_val_binary.m_value,
                                                                 (size_t)m_header->m_value.m_val_binary.m_size);
  if (L_error == C_ProtocolFrame::E_MSG_OK) {
    L_size_body -= *P_size ;
    ALLOC_VAR(m_body, T_pValueData, sizeof(T_ValueData));
    m_body->m_type = E_TYPE_STRING ;
    m_body->m_value.m_val_binary.m_size = L_size_body ;
    ALLOC_TABLE(m_body->m_value.m_val_binary.m_value,
                unsigned char*,
                sizeof(unsigned char),
                m_body->m_value.m_val_binary.m_size);
      
    memcpy(m_body->m_value.m_val_binary.m_value,
           P_ptr,
           m_body->m_value.m_val_binary.m_size);
  }
  //  std::cerr << "DecodeBodyWithParser end "  << std::endl;
  return (L_error);
}


C_ProtocolFrame::T_MsgError C_MessageText::DecodeBodyWithContentLength (int P_index,
                                                                        char *P_ptr, 
                                                                        size_t *P_size) {

  char                       *L_value        = NULL                       ;
  size_t                      L_size_body    = 0                          ;
  C_ProtocolFrame::T_MsgError L_error        = C_ProtocolFrame::E_MSG_OK  ;

  GEN_DEBUG(1, "C_MessageText::DecodeBodyWithContentLength() start");

  if (*P_size == 0) {
    // there is no body 
    return (L_error);
  }

  L_value = m_protocol->get_field_value(this,(m_methods[P_index]->m_param_decode).m_id) ;
  
  if (L_value != NULL) {

    // verify format ?
    L_size_body = atoi(L_value);
    if (L_size_body <= (*P_size)) {
      ALLOC_VAR(m_body, T_pValueData, sizeof(T_ValueData));
      m_body->m_type = E_TYPE_STRING ;
      m_body->m_value.m_val_binary.m_size = L_size_body ;
      ALLOC_TABLE(m_body->m_value.m_val_binary.m_value,
                  unsigned char*,
                  sizeof(unsigned char),
                  m_body->m_value.m_val_binary.m_size);
      
      memcpy(m_body->m_value.m_val_binary.m_value,
             P_ptr,
             m_body->m_value.m_val_binary.m_size);
      (*P_size) = (*P_size) - L_size_body ;    
    } else {
      L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS ;
    }
    FREE_TABLE(L_value);
  } else {
    L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING ;
  }

  GEN_DEBUG(1, "C_MessageText::DecodeBodyWithContentLength() end");  
  return (L_error);
}


C_MessageText::C_MessageText(C_ProtocolText *P_protocol) {
  m_protocol   = P_protocol ;
  m_header     = NULL       ; 
  m_body       = NULL       ;
  m_id         = -1         ;
  m_session_id = NULL       ;
}

C_MessageText::C_MessageText(C_MessageText &P_val) {

  GEN_DEBUG(1, "C_MessageText::C_MessageText() copy message start");
  m_protocol = P_val.m_protocol ;
  m_id = P_val.m_id ;

  ALLOC_VAR(m_header, T_pValueData, sizeof(T_ValueData));
  m_header->m_type = E_TYPE_NUMBER ;

  copyValue(*m_header, *P_val.m_header, false);  
  
  ALLOC_VAR(m_body, T_pValueData, sizeof(T_ValueData));
  m_body->m_type = E_TYPE_NUMBER ;

  copyValue(*m_body, *P_val.m_body, false);

  m_session_id = NULL ;
  GEN_DEBUG(1, "C_MessageText::C_MessageText() copy message end");
}

C_MessageText::C_MessageText(C_ProtocolText *P_protocol,
                             T_pValueData    P_header,
                             T_pValueData    P_body,
                             int             *P_id) {

  char * L_message_type   = NULL ;

  GEN_DEBUG(1, "C_MessageText::C_MessageText() start");
  m_protocol = P_protocol ;

  m_session_id = NULL     ;


  m_header   = P_header   ;
  m_body     = P_body     ;

  L_message_type 
    = m_protocol->get_field_value(this, m_message_type_field_id);
  if (L_message_type == NULL) { 
    // message unknown
    *P_id = -1 ;
    m_id = -1 ;
    GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown (not in dictionary) message [" 
                << this->get_text_value(m_header) 
                << "]");
  } else {
    // decode id
    // std::cerr << "L_message_type    " << L_message_type << std::endl;
    m_id = m_protocol->get_message_id(L_message_type);
    *P_id = m_id ;
    // std::cerr << "m_id " << m_id << std::endl;
  }
  FREE_TABLE(L_message_type);
  GEN_DEBUG(1, "C_MessageText::C_MessageText() end");
}


int C_MessageText::set_text_value(bool P_header_body, T_pValueData P_value, 
                                  int P_start, int P_end) {

  int          L_ret = 0 ;

  GEN_DEBUG(1, "C_MessageText::set_text_value() start");

  T_pValueData L_data = (P_header_body == true) ? m_header : m_body ;

  unsigned char *L_new_value = L_data->m_value.m_val_binary.m_value ;
  int            L_old_data_size = P_end - P_start ;
  int            L_delta_data_size = 0 ;
  int            L_new_size = 0 ;

  // std::cerr << "m_size "   << P_value->m_value.m_val_binary.m_size << std::endl;

  if (L_data) {
    // TO DO
    if (P_value->m_type == E_TYPE_STRING) {

      if (L_old_data_size != (int)P_value->m_value.m_val_binary.m_size) { 
        L_delta_data_size = P_value->m_value.m_val_binary.m_size - L_old_data_size ;

        // std::cerr << "L_delta_data_size " << L_delta_data_size << std::endl;
        L_new_size = L_data->m_value.m_val_binary.m_size + L_delta_data_size ;
        // std::cerr << "L_new_size " << L_new_size << std::endl;
        if (L_new_size > 0) {
          ALLOC_TABLE(L_new_value,
                      unsigned char*,
                      sizeof(unsigned char),
                      L_new_size);
          memcpy(L_new_value, L_data->m_value.m_val_binary.m_value, P_start);
          // add a control 
          memcpy((L_new_value+P_start+P_value->m_value.m_val_binary.m_size), 
                 (L_data->m_value.m_val_binary.m_value+P_end), 
                 (L_data->m_value.m_val_binary.m_size-P_end));
          FREE_TABLE(L_data->m_value.m_val_binary.m_value);
          L_data->m_value.m_val_binary.m_value = L_new_value ;
          L_data->m_value.m_val_binary.m_size = L_new_size   ;
        } else {
          L_ret = -1 ;
        }
      }
      if (L_ret == 0) {
        memcpy(L_new_value+P_start,
               P_value->m_value.m_val_binary.m_value,
               P_value->m_value.m_val_binary.m_size);
      }
    } else {
      L_ret = -1 ;
    }
    
  } else {
    L_ret = -1 ;
  }

  // std::cerr << "L_ret " << L_ret << std::endl;

  GEN_DEBUG(1, "C_MessageText::set_text_value() end");
  return (L_ret);
}


char* C_MessageText::get_text_value(bool P_header_body) {

  T_pValueData     L_value                   ;
  char            *L_result       = NULL     ;

  GEN_DEBUG(1, "C_MessageText::get_text_value() start");

  L_value = (P_header_body == true) ? m_header : m_body ;

  if (L_value != NULL) {
    ALLOC_TABLE(L_result, 
                char*, 
                sizeof(char), 
                (L_value->m_value.m_val_binary.m_size+1));
    memcpy(L_result, 
           (char*)L_value->m_value.m_val_binary.m_value,
           L_value->m_value.m_val_binary.m_size) ;
    L_result[L_value->m_value.m_val_binary.m_size] = 0 ;
  } 
  return (L_result) ;
}


T_pValueData C_MessageText::get_data_value(bool P_header_body) {

  T_pValueData L_value = NULL ;
  GEN_DEBUG(1, "C_MessageText::get_data_value() start");
  L_value = (P_header_body == true) ? m_header : m_body ;
  return (L_value);

}


C_MessageText::~C_MessageText() {
  GEN_DEBUG(1, "C_MessageText::~C_MessageText() start");
  if (m_header) resetMemory(*m_header)  ;
  FREE_VAR(m_header)      ;
  if (m_body) resetMemory(*m_body)    ;
  FREE_VAR(m_body)        ;
  m_protocol = NULL       ;
  m_id       = -1         ;

  if (m_session_id != NULL) {
    if ((m_session_id->m_type == E_TYPE_STRING)
        && (m_session_id->m_value.m_val_binary.m_size> 0)) {
      FREE_TABLE(m_session_id->m_value.m_val_binary.m_value);
      m_session_id->m_value.m_val_binary.m_size = 0 ;
    }
    FREE_VAR(m_session_id)  ;
  }

  GEN_DEBUG(1, "C_MessageText::~C_MessageText() end");
}

T_pValueData C_MessageText::getSessionFromOpenId (C_ContextFrame *P_ctxt) {

  T_pReceiveMsgContext L_recvCtx = NULL ;
  C_CallContext*       L_callCtx = NULL ;
  GEN_DEBUG(1, "C_MessageText::getSessionFromOpenId() start");
  
  if (m_session_id == NULL) {
    if (P_ctxt != NULL) {
      if ((L_callCtx = dynamic_cast<C_CallContext*>(P_ctxt)) != NULL) {
        ALLOC_VAR(m_session_id, T_pValueData, sizeof(T_ValueData));
        m_session_id->m_type = E_TYPE_NUMBER ;
        m_session_id->m_value.m_val_number = L_callCtx->m_channel_table[L_callCtx->m_channel_received] ;
        m_session_id->m_id = m_session_id_id ;

      } else if ((L_recvCtx = dynamic_cast<T_pReceiveMsgContext>(P_ctxt)) != NULL) {
        ALLOC_VAR(m_session_id, T_pValueData, sizeof(T_ValueData));
        m_session_id->m_type = E_TYPE_NUMBER ;
        m_session_id->m_value.m_val_number = L_recvCtx->m_response ;
        m_session_id->m_id = m_session_id_id ;
      }
    }
  }

  GEN_DEBUG(1, "C_MessageText::getSessionFromOpenId() end");
  return (m_session_id);
}

T_pValueData C_MessageText::getSessionFromField (C_ContextFrame *P_ctxt) {


  int                                          L_i                ;
  C_ProtocolText::T_pManagementSessionTextId   L_session_elt      ;
  int                                          L_nb_manag_session ;
  char                                        *L_value  = NULL    ;
  int                                          L_result = 0       ;


  GEN_DEBUG(1, "C_MessageText::getSessionFromField() start");

  if (m_session_id == NULL) {

    // retrieve the number of managment session
    L_nb_manag_session = m_protocol->get_nb_management_session () ;
    
    for (L_i = 0 ; L_i < L_nb_manag_session ; L_i++) {
      L_session_elt = m_protocol->get_manage_session_elt(L_i);
      L_value = m_protocol->get_field_value (this , L_session_elt->m_msg_id_id) ;
      if (L_value != NULL) {
        ALLOC_VAR(m_session_id, T_pValueData, sizeof(T_ValueData));
        *m_session_id = valueFromString(L_value, E_TYPE_STRING , L_result);
        if (L_result == -1) { 
          FREE_VAR(m_session_id);
        }
      }
      FREE_TABLE(L_value);

      if (m_session_id != NULL) {
        break;
      }
    } // for (L_i...)
  }
    
  GEN_DEBUG(1, "C_MessageText::getSessionFromField() end");

  return (m_session_id) ;
}

T_pValueData C_MessageText::get_session_id (C_ContextFrame *P_ctxt) {
  return (((this)->*(m_session_method))(P_ctxt)) ;
}


T_pValueData C_MessageText::get_out_of_session_id () {
  return (NULL);
}

bool C_MessageText::update_fields (C_MessageFrame* P_msg) {
  return (true) ;
}

bool C_MessageText::compare_types (C_MessageFrame* P_msg) {

  bool             L_result ;
  C_MessageText   *L_msg    ;

  GEN_DEBUG(1, "C_MessageText::compare_types() start");

  L_msg = dynamic_cast<C_MessageText*>(P_msg) ;

  L_result = (L_msg == NULL) ? false : (L_msg->m_id == m_id) ;

  GEN_DEBUG(1, "C_MessageText::compare_types() end");


  return (L_result);
}


bool C_MessageText::check(C_MessageFrame    *P_ref, 
                          unsigned int       P_levelMask,
                          T_CheckBehaviour   P_behave) {
  
  bool                       L_ret   = true  ;

  GEN_DEBUG(1, "C_MessageText::check() start");

  GEN_DEBUG(1, "C_MessageText::check() end");

  return (L_ret);
}


bool C_MessageText::check_field_presence (int              P_id,
                                          T_CheckBehaviour P_behave,
                                          int              P_instance,
                                          int              P_sub_id) {
  bool                                L_ret   = true  ;
  char                               *L_value = NULL  ; 

  GEN_DEBUG(1, "C_MessageText::check_field_presence() start");
  L_value = m_protocol->get_field_value(this,P_id) ;
  if (L_value) {
    FREE_TABLE(L_value);
  } else {
    GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
                          "check failed in [" 
                          <<  m_protocol->message_name (m_id) 
                          << "] " << m_protocol->message_name() 
                          << ", the field [" 
                          << m_names_fields[P_id]
                          << "] is not found");

    L_ret = false ;
  }
  GEN_DEBUG(1, "C_MessageText::check_field_presence() end");

  return (L_ret);
} 

bool C_MessageText::check_field_value (C_MessageFrame   *P_ref,
                                       int               P_id,
                                       T_CheckBehaviour  P_behave,
                                       int               P_instance,
                                       int               P_sub_id) {

  bool                                 L_ret   = true      ;
  char                                *L_value = NULL      ; 
  int                                  L_size  = -1        ;

  C_MessageText                       *L_ref               ;
  char                                *L_value_ref = NULL  ; 
  int                                  L_size_ref  = -1    ;


  GEN_DEBUG(1, "C_MessageText::check_field_value() start");

  L_ref = dynamic_cast<C_MessageText*>(P_ref);

  L_value_ref = m_protocol->get_field_value_to_check(L_ref,P_id, &L_size_ref) ;

  if (L_value_ref) {
    L_value = m_protocol->get_field_value_to_check(this,P_id, &L_size) ;
    if (L_value) {
      if (L_size != L_size_ref) {
        GEN_LOG_EVENT  (_check_behaviour_mask[P_behave], 
                        "check failed in [" 
                        <<  m_protocol->message_name (m_id)
                        << "] " << m_protocol->message_name() 
                        << ", the value of field ["
                        << m_names_fields[P_id]
                        << "] is incorrect");
        L_ret = false ;
      } else {
        if (memcmp(L_value_ref, L_value, L_size) != 0 ){
          GEN_LOG_EVENT  (_check_behaviour_mask[P_behave], 
                          "check failed in [" 
                          <<  m_protocol->message_name (m_id)
                          << "] " << m_protocol->message_name() 
                          << ", the value of field ["
                          << m_names_fields[P_id]
                          << "] is incorrect");
          L_ret = false ;
        }
      }
      FREE_TABLE(L_value);
    } else {
      GEN_LOG_EVENT  (_check_behaviour_mask[P_behave], 
                      "check failed in [" 
                      <<  m_protocol->message_name (m_id)
                      << "] " << m_protocol->message_name() 
                      << " received, the value of field ["
                      << m_names_fields[P_id]
                      << "] is incorrect");
      L_ret = false ;
    }
    FREE_TABLE(L_value_ref);
  } else {
    GEN_LOG_EVENT  (_check_behaviour_mask[P_behave], 
                    "check failed in [" 
                    <<  m_protocol->message_name (m_id)
                    << "] " << m_protocol->message_name() 
                    << " reference, the value of field ["
                    << m_names_fields[P_id]
                    << "] is incorrect");
    L_ret = false ;

  }
  GEN_DEBUG(1, "C_MessageText::check_field_value() end");
  return (L_ret);

}


bool C_MessageText::check_field_order (int              P_id,
                                       T_CheckBehaviour P_behave,
                                       int              P_position) {
  bool                                 L_ret   = true  ;
  GEN_DEBUG(1, "C_MessageText::check_field_order() start");

  GEN_DEBUG(1, "C_MessageText::check_field_order() end");
  return (L_ret);
}

bool C_MessageText::get_field_value(int P_id, 
                                    C_RegExp *P_reg,
                                    T_pValueData P_value) {


  bool                          L_found = true        ;
  T_ValueData                   L_value               ;
  char                         *L_buffer = NULL       ;
  int                           L_result = 0          ;


  L_buffer = m_protocol->get_field_value(this, P_id, P_reg);

  if (L_buffer != NULL) {
    L_value = valueFromString(L_buffer, E_TYPE_STRING, L_result);
    FREE_TABLE(L_buffer);
    if (L_result == -1) { 
      L_found = false ; 
    } else {
      L_value.m_id = P_id;
      (*P_value) = L_value ;
    }
  } else {
    L_found = false ;
  }


  GEN_DEBUG(1, "C_MessageText::get_field_value() end");

  return (L_found);
}

bool C_MessageText::get_field_value(int P_id, 
                                    int P_instance,
                                    int P_sub_id,
                                    T_pValueData P_value) {

  bool                          L_found = true        ;
  T_ValueData                   L_value               ;
  char                         *L_buffer = NULL       ;
  int                           L_result = 0          ;

  GEN_DEBUG(1, "C_MessageText::get_field_value() start");


  L_buffer = m_protocol->get_field_value(this, P_id);

  if (L_buffer != NULL) {
    L_value = valueFromString(L_buffer, E_TYPE_STRING, L_result);
    FREE_TABLE(L_buffer);
    if (L_result == -1) { 
      L_found = false ; 
    } else {
      L_value.m_id = P_id;
      (*P_value) = L_value ;
    }
  } else {
    L_found = false ;
  }
  
  GEN_DEBUG(1, "C_MessageText::get_field_value() end");

  return (L_found);
}


bool C_MessageText::set_field_value(T_pValueData P_value, 
                                    int P_id,
                                    int P_instance,
                                    int P_sub_id) {

  bool                          L_found = true        ;

  GEN_DEBUG(1, "C_MessageText::set_field_value() start");


  L_found = (m_protocol->set_field_value(this, P_id, P_value) == 0) ? true : false ;

  GEN_DEBUG(1, "C_MessageText::set_field_value() end");

  return (L_found);
}

T_TypeType C_MessageText::get_field_type (int P_id,
                                          int P_sub_id) {
  return (m_protocol->get_field_type(P_id,P_sub_id));
}


void C_MessageText::dump(iostream_output& P_stream) {
  GEN_DEBUG(1, "C_MessageText::dump() start");

  P_stream << *this << iostream_endl ;

  GEN_DEBUG(1, "C_MessageText::dump() end");
}


iostream_output& operator<< (iostream_output&   P_stream, 
			     C_MessageText& P_msg) {
  
  P_stream << "[" ;
  if (P_msg.m_header) {
    if (P_msg.m_id != -1) {
      P_msg.m_protocol->print_header_msg(P_stream,
                                         P_msg.m_id, 
                                         P_msg.m_header);
      if (P_msg.m_body) {
        P_msg.m_protocol->print_data_msg(P_stream,
                                         P_msg.m_body);
      }
    } else {
      P_stream << "Unknown text message" << iostream_endl ;
    }
  }
  
  
  P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) << "]" ;

  return (P_stream) ;
}

char* C_MessageText::name() {
  return( m_protocol->message_name(m_id));
}

int C_MessageText::get_id_message(){
  return (m_id) ;
}

void C_MessageText::set_id_message(int P_id){
   m_id = P_id ;
}

C_ProtocolFrame::T_MsgError C_MessageText::encode(unsigned char *P_buf,
                                                  size_t        *P_siz) {

  GEN_DEBUG(1, "C_MessageText::encode() start");

  C_ProtocolFrame::T_MsgError L_error        = C_ProtocolFrame::E_MSG_OK  ;
  size_t                      L_pos          = 0                          ;

  unsigned char *L_buf = P_buf ;

  int            L_index = 0 ;


  while (L_index < m_nb_methods) {
    L_error = ((this)->*(m_methods[L_index]->m_encode))(L_index);
    if (L_error == C_ProtocolFrame::E_MSG_OK) break ;
    L_index ++ ;
  }


  if (L_error != C_ProtocolFrame::E_MSG_OK) return (L_error);

  L_pos += m_header->m_value.m_val_binary.m_size ;


  if (L_pos <= (*P_siz)) {
    memcpy(L_buf,
           m_header->m_value.m_val_binary.m_value,
           m_header->m_value.m_val_binary.m_size);
    L_buf += m_header->m_value.m_val_binary.m_size ;
    L_pos += m_body_separator_size ;
    if (L_pos <= (*P_siz)) {
      memcpy(L_buf,
             m_body_separator,
             m_body_separator_size);
      L_buf += m_body_separator_size ;
      
      if (m_body != NULL) {
        L_pos += m_body->m_value.m_val_binary.m_size ;
        if (L_pos <= (*P_siz)) {
          memcpy(L_buf,
                 m_body->m_value.m_val_binary.m_value,
                 m_body->m_value.m_val_binary.m_size);
          *P_siz = L_pos ;
        } else {
          L_error = C_ProtocolTextFrame::E_MSG_ERROR_ENCODING ;
        }
      } else {
        *P_siz = L_pos ;
      }      
    } else {
      L_error = C_ProtocolTextFrame::E_MSG_ERROR_ENCODING ;
    }
  } else {
    L_error = C_ProtocolTextFrame::E_MSG_ERROR_ENCODING ;
  }

  GEN_DEBUG(1, "C_MessageText::encode() end");
  return (L_error);

}

unsigned long C_MessageText::decode(unsigned char               *P_buf,
                                    size_t                       P_siz,
                                    C_ProtocolFrame::T_pMsgError P_error) {

  GEN_DEBUG(1, "C_MessageText::decode() start");

  char              *L_ptr          = NULL  ; 
  unsigned char     *L_buf          = NULL  ;

  unsigned long      L_header_size_decoded = 0 ;

  char              *L_message_type = NULL  ;
  size_t             L_remaining    = P_siz ;
  char               L_char                 ;
  int                L_index        = 0     ;
  
  unsigned char     *L_wbuf         = NULL  ;

  *P_error        = C_ProtocolFrame::E_MSG_OK  ;

  if (memcmp((P_buf + (P_siz - m_body_separator_size)),
             m_body_separator,
             m_body_separator_size) == 0) {
    
    ALLOC_TABLE(L_buf, unsigned char*, sizeof(unsigned char), P_siz +1);
    memcpy(L_buf, P_buf, P_siz);
    L_buf[P_siz] = 0 ;
  } else {
    L_char = P_buf[(P_siz-1)] ;
    P_buf[(P_siz-1)] = 0 ;
  }


  L_wbuf = (L_buf == NULL) ? P_buf : L_buf ;

  // find body separator
  L_ptr = strstr((char*)L_wbuf, m_field_body_separator);

  if (L_ptr != NULL) {

    ALLOC_VAR(m_header, T_pValueData, sizeof(T_ValueData));
    m_header->m_type = E_TYPE_STRING;
    L_header_size_decoded = ((unsigned char*)L_ptr - L_wbuf) ;

    //    L_header_size_decoded += m_body_separator_size ;
    L_header_size_decoded += m_field_separator_size ;

    m_header->m_value.m_val_binary.m_size = L_header_size_decoded ;
    ALLOC_TABLE(m_header->m_value.m_val_binary.m_value,
                unsigned char*,
                sizeof(unsigned char),
                m_header->m_value.m_val_binary.m_size);

    memcpy(m_header->m_value.m_val_binary.m_value,
           L_wbuf,
           m_header->m_value.m_val_binary.m_size);

    L_remaining -= L_header_size_decoded ;
    L_remaining -= m_body_separator_size ;


    // temporary
    // L_ptr += 4 ; // \r\n\r\n
    L_ptr += m_field_body_separator_size ; // \r\n\r\n

    if (!L_buf) { P_buf[(P_siz-1)] = L_char ; }

    while (L_index < m_nb_methods) {
      (*P_error)  =  ((this)->*((m_methods[L_index])->m_decode))(L_index, L_ptr, &L_remaining) ;
      if ((*P_error) == C_ProtocolFrame::E_MSG_OK) break ;
      L_index ++ ;
    }

    switch (*P_error) {
    case C_ProtocolFrame::E_MSG_OK:
      L_message_type 
        = m_protocol->get_field_value(this, m_message_type_field_id);
      if (L_message_type == NULL) { 
        // message unknown
        *P_error  = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS  ;
        m_id = -1 ;
      } else {
        // decode id
        m_id = m_protocol->get_message_id(L_message_type);
      }
      FREE_TABLE(L_message_type);
      break ;
    case C_ProtocolFrame::E_MSG_ERROR_DECODING:
      break ;
    case C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS:
      L_remaining += L_header_size_decoded ;
      L_remaining += m_body_separator_size ;
      break ;
    default:
      break ;
    }

  } else {
    *P_error = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS ;
    if (!L_buf) { P_buf[(P_siz-1)] = L_char ; }
  }


  if (L_buf) {
    FREE_TABLE(L_buf);
  } 

  GEN_DEBUG(1, "C_MessageText::decode() with " << L_remaining);

  return (L_remaining) ;
}

void C_MessageText::update_message_stats () {
}

int  C_MessageText::get_buffer (T_pValueData P_dest ,
                                T_MessagePartType P_header_body_type) {

  int               L_ret   = 0    ;

  switch (P_header_body_type) {
  case E_HEADER_TYPE :
    copyValue(*P_dest, *m_header, false);
    break;
  case E_BODY_TYPE   :
    copyValue(*P_dest, *m_body, false);
    break;
  case E_ALL_TYPE    :
  default:
    L_ret = -1 ;
    break ;
  }
  return (L_ret) ;
}
