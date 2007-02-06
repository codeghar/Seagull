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

#include "C_MessageTlv.hpp"
#include "BufferUtils.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "ProtocolData.hpp"

C_MessageTlv::C_MessageTlv(C_ProtocolTlv *P_protocol) {

  unsigned long L_nbFields, L_i ;

  GEN_DEBUG(1, "C_MessageTlv::C_MessageTlv() start");

   m_header_id = -1 ;
   m_nb_body_values = 0 ;

   
   ALLOC_TABLE(m_body_val, 
               C_ProtocolTlv::T_pBodyValue,
               sizeof(C_ProtocolTlv::T_BodyValue),
               MAX_BODY_VALUES);

   for(L_i = 0 ; L_i < MAX_BODY_VALUES ; L_i++) {
     m_body_val[L_i].m_id = -1 ;
   }
   
   m_protocol = P_protocol ;
   m_call_id = 0 ;
   L_nbFields = m_protocol->get_nb_field_header() ;
   ALLOC_TABLE(m_header_values, 
               T_pValueData, 
               sizeof(T_ValueData), 
               L_nbFields);
   
   for(L_i = 0 ; L_i < L_nbFields; L_i++) {
     // Reset Value Data ok because struct used
     memset(&m_header_values[L_i], 0, sizeof(T_ValueData));
     
     // Set the id in header value
     m_header_values[L_i].m_id = L_i ;
   }

   // Init m_id
   memset (&m_id, 0, sizeof(T_ValueData));
   m_id.m_type = E_TYPE_NUMBER ;
   
   m_header_body_field_separator = m_protocol->get_header_body_field_separator () ;


   GEN_DEBUG(1, "C_MessageTlv::C_MessageTlv() end");
}

void C_MessageTlv::set_header_value (int P_id, unsigned long P_val) {

  GEN_DEBUG(1, "C_MessageTlv::set_header_value() start: P_id:" << P_id << " value ul:" << P_val);
  m_header_values[P_id].m_type = E_TYPE_NUMBER ;
  m_header_values[P_id].m_value.m_val_number = P_val ;
  GEN_DEBUG(1, "C_MessageTlv::set_header_value() end");

}

void C_MessageTlv::set_header_value (char* P_name, unsigned long P_val) {

  C_ProtocolTlv::T_pHeaderField L_desc ;
  int                        L_id   ;

  GEN_DEBUG(1, "C_MessageTlv::set_header_value() start: name:" << P_name << " value ul:" << P_val);
  L_desc = m_protocol -> get_header_field_description(P_name) ;
  if (L_desc != NULL) {
    L_id = L_desc -> m_id ;
    m_header_values[L_id].m_type = E_TYPE_NUMBER ;
    m_header_values[L_id].m_value.m_val_number = P_val ;
  }
  GEN_DEBUG(1, "C_MessageTlv::set_header_value() end");

}

void C_MessageTlv::set_header_value (char* P_name, char *P_val) {
  C_ProtocolTlv::T_pHeaderField L_desc ;
  int                        L_id   ;

  GEN_DEBUG(1, "C_MessageTlv::set_header_value() start: name:" << P_name << " value char:" << *P_val);
  L_desc = m_protocol -> get_header_field_description(P_name) ;
  if (L_desc != NULL) {
    L_id = L_desc -> m_id ;
    m_header_values[L_id].m_type = E_TYPE_NUMBER ;
    m_header_values[L_id].m_value.m_val_number = convert_char_to_ul(P_val) ;
  }
  GEN_DEBUG(1, "C_MessageTlv::set_header_value() end");
}

void C_MessageTlv::set_header_value (int P_id, T_pValueData P_val) {

  GEN_DEBUG(1, "C_MessageTlv::set_header_value() start: P_id:" << P_id );
  m_protocol->set_header_value(P_id, &(m_header_values[P_id]), P_val);
  GEN_DEBUG(1, "C_MessageTlv::set_header_value() end");
}


C_MessageTlv::~C_MessageTlv() {

  unsigned long L_i ;
  unsigned long L_nbFields;

  GEN_DEBUG(1, "\nC_MessageTlv::~C_MessageTlv() start: name " << name());
  m_call_id = 0 ;

  if (m_protocol->get_complex_header_presence())
  {
    L_nbFields = m_protocol->get_nb_field_header();
    GEN_DEBUG(2, "nb header fields = " << L_nbFields);

    for(L_i = 0; L_i < L_nbFields; L_i++) {
      m_protocol->delete_header_value(&m_header_values[L_i]);
      GEN_DEBUG(2, "delete_header value [" << L_i << "] OK");
    }
  }
  FREE_TABLE(m_header_values) ;
  GEN_DEBUG(2, "FREE_TABLE(m_header_values) OK\n");

  m_header_id = -1 ;
  
  GEN_DEBUG(2, "m_nb_body_values = " << m_nb_body_values);

  for(L_i = 0; L_i < (unsigned long)m_nb_body_values; L_i++) {
    m_protocol->delete_body_value(&m_body_val[L_i]);

    GEN_DEBUG(2, "delete body value [" << L_i << "] OK");
  }
  FREE_TABLE(m_body_val);
  GEN_DEBUG(2, "FREE_TABLE(m_body_val) OK");
  m_nb_body_values = 0 ;

  m_protocol->reset_value_data (&m_id);
  m_header_body_field_separator = NULL ;
  m_protocol = NULL ;

  GEN_DEBUG(1, "C_MessageTlv::~C_MessageTlv() end");

}

bool C_MessageTlv::update_fields (C_MessageFrame* P_msg) {
  return (true);
}

bool C_MessageTlv::compare_types (C_MessageFrame* P_msg) {

  bool             L_result ;
  C_MessageTlv *L_msg ;

  L_msg = dynamic_cast<C_MessageTlv*>(P_msg) ;

  GEN_DEBUG(1, "C_MessageTlv::compare_types() start");
  if (L_msg == NULL) {
    L_result = false ;
  } else {
    GEN_DEBUG(1, "C_MessageTlv::compare_types() get_type() " << get_type());
    L_result = (L_msg -> get_type()  == get_type()) ? true : false ;
    GEN_DEBUG(1, "C_MessageTlv::compare_types()  L_result of compare the type of message" << L_result);
  }
  GEN_DEBUG(1, "C_MessageTlv::compare_types() end");

  return (L_result) ;
}

unsigned long  C_MessageTlv::get_type () {

  int           L_idx = m_protocol -> get_header_type_id() ;
  unsigned long L_type = 0;
  bool          L_found = false ;
  int           L_i ;

  GEN_DEBUG(1, "C_MessageTlv::get_type() start");

  if (L_idx != -1) {
      L_type = m_header_values[L_idx].m_value.m_val_number ;
      GEN_DEBUG(1, "C_MessageTlv::get_type() when L_idx != -1 the type is " 
		<< L_type );
  } else {
      L_idx = m_protocol -> get_header_type_id_body() ;
      GEN_DEBUG(1, "C_MessageTlv::get_type() the L_idx is " 
		<< L_idx);
      GEN_DEBUG(1, "C_MessageTlv::get_type() the m_nb_body_values is " 
		<< m_nb_body_values);

      for (L_i=0; L_i < m_nb_body_values ; L_i++) { // remenber: to be optimized
          if (m_body_val[L_i].m_id == L_idx) { 
	    L_found=true; 
	    break; 
	  }
      }
      
      GEN_DEBUG(1, "C_MessageTlv::get_type() the L_i is " 
		<< L_i) ;

      if (L_found == true) {
        L_type = m_body_val[L_i].m_value.m_val_number ;

        GEN_DEBUG(1, "C_MessageTlv::get_type()  L_type " << L_type << 
                  " and m_body_val[L_i].m_value.m_val_signed " 
                  << m_body_val[L_i].m_value.m_val_signed);
        GEN_DEBUG(1, "C_MessageTlv::get_type()  L_type " << L_type << 
                  " and m_body_val[L_i].m_value.m_val_number " 
                  << m_body_val[L_i].m_value.m_val_number);
      }
  }

  GEN_DEBUG(1, "C_MessageTlv::get_type() returns " << L_type);
  return (L_type) ;
}

void  C_MessageTlv::set_type (unsigned long P_type) {
  int L_idx = m_protocol -> get_header_type_id() ;
  GEN_DEBUG(1, "C_MessageTlv::set_type() start");
  m_header_values[L_idx].m_type = E_TYPE_NUMBER ;
  m_header_values[L_idx].m_value.m_val_number = P_type ;
  GEN_DEBUG(1, "C_MessageTlv::set_type() end");
}

unsigned long  C_MessageTlv::get_call_id () {
  GEN_DEBUG(1, "C_MessageTlv::get_call_id() start");
  GEN_DEBUG(1, "C_MessageTlv::get_call_id() end");
  return (m_call_id);
}


void C_MessageTlv::set_call_id (unsigned long P_id) {
  GEN_DEBUG(1, "C_MessageTlv::set_call_id() start");
  m_call_id = P_id ;
  GEN_DEBUG(1, "C_MessageTlv::set_call_id() end");
}

unsigned long C_MessageTlv::decode (unsigned char *P_buffer, 
                                    size_t         P_size,
                                    C_ProtocolFrame::T_pMsgError P_error) {
  
  unsigned char *L_ptr = P_buffer ;
  unsigned long  L_ret = 0 ; 
  C_ProtocolContext   *L_protocol_ctxt = m_protocol->create_protocol_context() ;

  
  L_ret  = m_protocol -> decode_header (&L_ptr, 
                                        P_size,
                                        m_header_values,
                                        &m_header_id,
                                        L_protocol_ctxt) ;
  
  
  switch (L_ret) {

  case 0:

    if ((m_header_id != -1) || (m_protocol->get_header_type_id() == -1)) {
      
      m_nb_body_values = MAX_BODY_VALUES ;
      L_ret = m_protocol->decode_body(&L_ptr,
                                      m_body_val,
                                      &m_nb_body_values,
                                      &m_header_id,
                                      L_protocol_ctxt);

      switch (L_ret) {

      case 0:
	
        GEN_DEBUG(1, "C_MessageTlv::decode() message decoded ");
        *P_error = C_ProtocolFrame::E_MSG_OK ;
        L_ret = L_protocol_ctxt->get_counter_ctxt(0) ;
	
	break ;

      case 1:

          GEN_ERROR(E_GEN_FATAL_ERROR, "Unrecognized buffer header");	
          *P_error = C_ProtocolFrame::E_MSG_ERROR_DECODING;
	  L_ret = 0 ;
	  break ;

      default: // 2
	
	*P_error = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS ;
	L_ret = 0 ;
	break ;
      }

    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Unrecognized buffer header");	
      *P_error = C_ProtocolFrame::E_MSG_ERROR_DECODING;
      L_ret = 0 ;
    }

    break ;

  case 1:

    GEN_ERROR(E_GEN_FATAL_ERROR, "Unrecognized buffer header");	
    *P_error = C_ProtocolFrame::E_MSG_ERROR_DECODING;
    L_ret = 0 ;      
    break ;

  default: // 2
    
    *P_error = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS ;
    L_ret = 0 ;    
    break ;

  }

  
  m_protocol->delete_protocol_context (&L_protocol_ctxt);
      

  GEN_DEBUG(1, "C_MessageTlv::decode() end");
  
  return (L_ret) ;
}


void C_MessageTlv::encode (unsigned char* P_buffer, 
			   size_t*        P_size, //init value = buf size
			   C_ProtocolFrame::T_pMsgError P_error) {

  unsigned char *L_ptr = P_buffer ;
  size_t         L_size = *P_size ;
  C_ProtocolContext   *L_protocol_ctxt = m_protocol->create_protocol_context() ;
  
  GEN_DEBUG(1, "C_MessageTlv::encode() start ");


  C_ProtocolFrame::T_MsgError L_error = C_ProtocolFrame::E_MSG_OK ;

  // start with header
  m_protocol->encode_header (m_header_id, 
			     m_header_values, 
			     L_ptr, 
			     &L_size,
			     L_protocol_ctxt,
			     &L_error);

  if (L_error == C_ProtocolFrame::E_MSG_OK) {

    L_ptr += L_size ;
    L_size = *P_size - L_size ;

    L_error = m_protocol->encode_body(m_nb_body_values,
                                      m_body_val,
                                      L_ptr,
                                      &L_size,
                                      L_protocol_ctxt);
    

    if (L_error == C_ProtocolFrame::E_MSG_OK) {

      m_protocol->propagate_ctxt_global (L_protocol_ctxt);
      *P_size = L_protocol_ctxt->get_counter_ctxt(0); 
      m_protocol->delete_protocol_context (&L_protocol_ctxt);

    }

  }

  *P_error = L_error ;
  
  GEN_DEBUG(1, "C_MessageTlv::encode() end");
}

void C_MessageTlv::set_header_id_value (int P_id) {
  
  GEN_DEBUG(1, "C_MessageTlv::set_header_id_value(" << P_id << ") start");
  m_header_id = P_id ;
  m_protocol->get_header_values (P_id, m_header_values) ;
  m_protocol->get_body_values (P_id, m_body_val, &m_nb_body_values);
  
  GEN_DEBUG(1, "C_MessageTlv::set_header_id_value() end");
}

void C_MessageTlv::set_body_value (C_ProtocolTlv::T_pBodyValue P_val) {
  
  GEN_DEBUG(1, "C_MessageTlv::set_body_value() start current nb: " << m_nb_body_values );
//  GEN_DEBUG(1, "C_MessageTlv::set_body_value() P_val: " << P_val);

  if (m_nb_body_values == MAX_BODY_VALUES) {
    GEN_FATAL(E_GEN_FATAL_ERROR, 
	  "Maximum number of values ["
	  << MAX_BODY_VALUES << "] reached" << iostream_endl) ;
  } else {
    m_protocol->set_body_value(&m_body_val[m_nb_body_values], P_val);
    m_nb_body_values++ ;
  }

  GEN_DEBUG(1, "C_MessageTlv::set_body_value() end - new nb: " << m_nb_body_values);
}

C_MessageTlv& C_MessageTlv::operator= (C_MessageTlv & P_val) {

  m_protocol  = P_val.m_protocol ;
  m_call_id   = P_val.m_call_id ;
  m_header_id = P_val.m_header_id ;

  if (m_protocol->get_complex_header_presence())
  {
    m_protocol -> reset_header_values(m_protocol->get_nb_field_header(), 
                                      m_header_values);
  }
  m_protocol -> reset_body_values(m_nb_body_values, m_body_val);

  m_nb_body_values = P_val.m_nb_body_values ;
  m_protocol -> set_header_values (m_header_values, P_val.m_header_values);
  m_protocol -> set_body_values (m_nb_body_values, m_body_val, P_val.m_body_val);

  return (*this) ;
}

void C_MessageTlv::get_header_value (T_pValueData P_res, 
				      int P_id) {
  *P_res = m_header_values[P_id] ;
}

void C_MessageTlv::get_body_value (T_pValueData P_res, 
				    int P_id) {

  int  L_i ;
  bool L_found = false ;


  // Search the body value in the array
  for (L_i=0 ; L_i < m_nb_body_values ; L_i++) {
    if (m_body_val[L_i].m_id == P_id) { 
      L_found=true; 
      break; 
    }
  }
  if (L_found == true) {
    m_protocol->get_body_value(P_res, &m_body_val[L_i]) ;
  } 
}

bool C_MessageTlv::set_body_value (int P_id, T_pValueData P_val) {

  int  L_i ;
  bool L_found = false ;

  for (L_i=0 ; L_i < m_nb_body_values ; L_i++) {
    if (m_body_val[L_i].m_id == P_id) { L_found=true; break; }
  }
  if (L_found == true) {
    m_protocol->set_body_value(&m_body_val[L_i], P_val) ;
  }

  return (L_found);
  
}


iostream_output& operator<< (iostream_output& P_stream, C_MessageTlv &P_msg) {
  P_stream << "[" ;
  P_msg.m_protocol->print_header(P_stream, 
				 P_msg.m_header_id, 
				 P_msg.m_header_values) ;
  P_msg.m_protocol->print_body(P_stream,
			       P_msg.m_nb_body_values,
			       P_msg.m_body_val);
  P_stream << GEN_HEADER_LOG << GEN_HEADER_LEVEL(LOG_LEVEL_MSG) << "]" ;
  return (P_stream) ;
}

C_ProtocolTlv* C_MessageTlv::get_protocol() {
  return (m_protocol);
}

T_pValueData C_MessageTlv::get_session_id (C_ContextFrame *P_ctxt) {

  T_pValueData L_id_value = NULL ;
  int                      L_i, L_id  ;
  bool                     L_found = false ;

  GEN_DEBUG(1, "C_MessageTlv::get_session_id() start");

  L_id = m_protocol->get_msg_id() ;

  GEN_DEBUG(1, "L_id = " << L_id << " type is " << 
               m_protocol->get_msg_id_type() << " (0: header, 1:body)");

  switch (m_protocol->get_msg_id_type()) {
  case C_ProtocolTlv::E_MSG_ID_HEADER:
    GEN_DEBUG(1, "Header Id :");
    L_id_value = &m_header_values[L_id];

    GEN_DEBUG(1, "value :\n\tm_id :" << L_id_value->m_id << 
                 "\tm_type: " << L_id_value->m_type << " ");

    break ;
  case C_ProtocolTlv::E_MSG_ID_BODY:

    GEN_DEBUG(1, "Body Id : nb value is " << m_nb_body_values << " ");

    for (L_i=0 ; L_i < m_nb_body_values ; L_i++) {

      GEN_DEBUG(1, "Body Id [" << L_i << "] = " << m_body_val[L_i].m_id << " ");

      if (m_body_val[L_i].m_id == L_id) { L_found=true; break; }


    }
    if (L_found == true) {
      m_protocol->reset_value_data(&m_id);
      m_protocol->get_body_value(&m_id, &m_body_val[L_i]) ;
      L_id_value = &m_id ;

      GEN_DEBUG(1, "value :\n\tm_id :" << L_id_value->m_id << 
                   "\tm_type: " << L_id_value->m_type << " ");

    }
    break ;
  }
  GEN_DEBUG(1, "C_MessageTlv::get_session_id() end");

  if (L_id_value == NULL) {
    L_id_value = get_out_of_session_id();
  }

  return (L_id_value) ;

}

T_pValueData C_MessageTlv::get_out_of_session_id () {

  T_pValueData L_id_value = NULL ;

  int                      L_i, L_id  ;
  bool                     L_found = false ;

  // Gilles PB
  T_ValueData L_tmp_id_value ;
  

  GEN_DEBUG(1, "C_MessageTlv::get_out_of_session_id() start");

  L_id = m_protocol->get_out_of_session_id() ;

  GEN_DEBUG(1, "L_id = " << L_id << " with type: " 
               << m_protocol->get_out_of_session_id_type() 
               << " (0: header, 1:body)");

  switch (m_protocol->get_out_of_session_id_type ()) {
  case C_ProtocolTlv::E_MSG_ID_HEADER:
    GEN_DEBUG(1, "Header Id :");

    L_id_value = &m_header_values[L_id];

    GEN_DEBUG(1, "value :\n  m_id :" << L_id_value->m_id << 
                 "\n  m_type: " << L_id_value->m_type << " ");

    if (m_protocol->get_msg_id_value_type() 
	== E_TYPE_STRING) {
      GEN_DEBUG(1, "C_MessageTlv::get_out_of_session_id() reset PB");
      memcpy(&L_tmp_id_value,L_id_value,sizeof(L_tmp_id_value)) ;

      m_protocol->reset_value_data(&m_id);
      m_protocol->convert_to_string(&m_id, &L_tmp_id_value);

      L_id_value = &m_id ;
    }
    break ;
    case C_ProtocolTlv::E_MSG_ID_BODY:
      GEN_DEBUG(1, "Body Id : nb value is " << m_nb_body_values << " ");

      L_found = false ;
      for (L_i=0 ; L_i < m_nb_body_values ; L_i++) {

        GEN_DEBUG(1, "Body Id [" << L_i << "] = " << m_body_val[L_i].m_id << " ");
	if (m_body_val[L_i].m_id == L_id) { L_found=true; break; }
      }
      if (L_found == true) {
	m_protocol->reset_value_data(&m_id);
	m_protocol->get_body_value(&m_id, &m_body_val[L_i]) ;
	L_id_value = &m_id ;

        GEN_DEBUG(1, "value :\n  m_id :" << L_id_value->m_id << 
                   "\n  m_type: " << L_id_value->m_type << " ");
      }
      break ;
  }
  GEN_DEBUG(1, "C_MessageTlv::get_out_of_session_id() end");
  
 return (L_id_value) ;
}

bool C_MessageTlv::check(C_MessageFrame    *P_ref, 
			    unsigned int       P_levelMask,
			    T_CheckBehaviour   P_behave) {

  bool                       L_ret   = true  ;
  int                        L_nb            ;
  int                        L_i, L_j, L_id  ;
  bool                       L_found = false ;

  C_MessageTlv  *L_ref ;

  C_ProtocolTlv::T_pHeaderBodyValue L_descr         ;
  C_ProtocolTlv::T_pHeaderValue L_descrVal      ; 

  L_ref = dynamic_cast<C_MessageTlv*>(P_ref);
  
  // check body
  L_nb = L_ref->m_nb_body_values ;
  
  if (P_levelMask & _check_level_mask[E_CHECK_LEVEL_FIELD_PRESENCE]) {
    // check that the fields of the scenario are present
    for (L_i = 0 ; L_i < L_nb; L_i++) {
      L_found = false ;
      L_id = L_ref->m_body_val[L_i].m_id ;
      for(L_j=0; L_j <m_nb_body_values; L_j++) {
	if (m_body_val[L_j].m_id == L_id) {
	  L_found = true ;
	  break ;
	}
      }
      if (L_found == false) {
	L_ret = false ;
	L_descr = m_protocol->get_header_body_value_description(L_id);
	if (L_descr != NULL) {
	  L_descrVal = m_protocol->get_header_value_description(m_header_id);

	  GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			    "check failed in [" 
			    << m_protocol -> get_header_name() 
			    << "] ["
			    << L_descrVal->m_name
			    << "]");
	  GEN_LOG_EVENT_NO_DATE(_check_behaviour_mask[P_behave],
			    "                [" 
			    << m_protocol->get_header_body_name()
			    << "] [" << L_descr->m_name << "] not found");
	}
      } else {
	// check of the value to be done here
      }
    }
  }



  if (P_levelMask & _check_level_mask[E_CHECK_LEVEL_FIELD_ADDED]) {
    
    // check that there is no field added from the scenario
    for (L_i = 0 ; L_i < m_nb_body_values; L_i++) {
      L_found = false ;
      L_id = m_body_val[L_i].m_id ;
      for(L_j=0; L_j <m_nb_body_values; L_j++) {
	if (L_ref->m_body_val[L_j].m_id == L_id) {
	  L_found = true ;
	  break ;
	}
      }
      if (L_found == false) {
	L_ret = false ;
	L_descr = m_protocol->get_header_body_value_description(L_id);
	if (L_descr != NULL) {
	  L_descrVal = m_protocol->get_header_value_description(m_header_id);
	  

	  GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			    "check failed in [" 
			    << m_protocol -> get_header_name() 
			    << "] ["
			    << L_descrVal->m_name
			    << "]");
	  GEN_LOG_EVENT_NO_DATE(_check_behaviour_mask[P_behave], 
			    "     additional ["
			    << m_protocol->get_header_body_name()
			    << "] [" 
			    << L_descr->m_name << "] found");

	}
      }
    }
  }

  return (L_ret) ;
}

bool C_MessageTlv::check_field_presence (int              P_id,
					    T_CheckBehaviour P_behave,
					    int              P_instance,
					    int              P_sub_id) {
  bool                                 L_ret   = true  ;
  int                                  L_i ;
  bool                                 L_found = false ;
  C_ProtocolTlv::T_pHeaderBodyValue L_descr         ;
  C_ProtocolTlv::T_pHeaderValue     L_descrVal      ; 
  unsigned long                        L_max_nb_field_header ;


  L_max_nb_field_header = m_protocol->get_m_max_nb_field_header () ;

  if (P_id < (int)L_max_nb_field_header) { return (true) ; }

  // check that the fields of the scenario are present
  for (L_i = 0 ; L_i < m_nb_body_values; L_i++) {
    if ((m_body_val[L_i].m_id + (int)L_max_nb_field_header) == P_id) {
      L_found = true ;
      break ;
    }
  }

  if (L_found == false) {
    L_ret = false ;
    L_descr = m_protocol->get_header_body_value_description(P_id - L_max_nb_field_header);
    if (L_descr != NULL) {
      L_descrVal = m_protocol->get_header_value_description(m_header_id);
      
      if (L_descrVal != NULL) {
        GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
                              "check failed [" 
                              << m_protocol -> get_header_name() 
                              << "] ["
                              << L_descrVal->m_name
                              << "]");
      } else {
	GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			      "check failed [" 
			      << m_protocol -> get_header_name() 
			      << "] ["
			      << m_protocol->message_name(m_header_id)
			      << "]");
      }
      
      GEN_LOG_EVENT_NO_DATE(_check_behaviour_mask[P_behave],
                            "                [" 
                            << m_protocol->get_header_body_name()
                            << "] [" << L_descr->m_name << "] not found");
    }
  }

  return (L_ret) ;
}

bool C_MessageTlv::check_field_value (C_MessageFrame   *P_ref,
		                         int               P_id,
					 T_CheckBehaviour  P_behave,
					 int               P_instance,
					 int               P_sub_id) {

  GEN_DEBUG(1, "C_MessageTlv::check_field_value() start");

  C_MessageTlv                     *L_ref                 ;
  unsigned long                        L_max_nb_field_header ;
  int                                  L_id     = P_id       ;
  bool                                 L_check  = false      ;

  T_ValueData                          L_value_ref           ;
  T_ValueData                          L_value               ;


  C_ProtocolTlv::T_pHeaderBodyValue L_descr               ;
  C_ProtocolTlv::T_pHeaderField     L_headerField         ;


  L_ref                 = dynamic_cast<C_MessageTlv*>(P_ref)    ;
  L_max_nb_field_header = m_protocol->get_m_max_nb_field_header () ;

  if (L_id >= (int) L_max_nb_field_header) {
    // case body
    L_id -= L_max_nb_field_header ;
    L_ref->get_body_value (&L_value_ref, L_id);
    get_body_value (&L_value, L_id);
    L_check = (L_value_ref == L_value) ;
  } else {
    // case header
    L_check = (L_ref->m_header_values[L_id] == m_header_values[L_id]) ;
  }

  if (L_check == false) {
    if (P_id <  (int) L_max_nb_field_header) {
      L_headerField = m_protocol->get_header_field_description(L_id);
      if (L_headerField != NULL) {
        GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
                              "check failed in [" 
                              <<  m_protocol->message_name(L_ref->m_header_id) 
                              << "] " << m_protocol->message_name() 
                              << ", value of field" 
                              << " ["
                              << L_headerField->m_name
                              << "] is incorrect. Expected ["
                              << L_value_ref 
                              << "] but got ["
                              << "L_value" << "]");
      }
    } else {
      L_descr = m_protocol->get_header_body_value_description(L_id);
      if (L_descr != NULL) {
        GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
                              "check failed in [" 
                              <<  m_protocol->message_name(L_ref->m_header_id) 
                              << "] " << m_protocol->message_name() 
                              << ", value of " << m_protocol->message_component_name ()
                              << " ["
                              << L_descr->m_name
                              << "]. Expected ["
                              << L_value_ref
                              << "] but got ["
                              << L_value << "]");
      } else {
        GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
                              "check failed in [" 
                              <<  m_protocol->message_name(L_ref->m_header_id) 
                              << "] " << m_protocol->message_name() 
                              << ". Expected ["
                              << L_value_ref
                              << "], but got ["
                              << L_value << "]");
      }
    }
  } // if (L_check == false)

  GEN_DEBUG(1, "C_MessageTlv::check_field_value() end ret: " << L_check);

  return (L_check) ;
}

bool C_MessageTlv::check_field_order (int              P_id,
					 T_CheckBehaviour P_behave,
					 int              P_position) {
  bool                                 L_ret   = true  ;

  GEN_DEBUG(1, "C_MessageTlv::check_field_order() start");
  GEN_DEBUG(1, "C_MessageTlv::check_field_order() P_id:       " << P_id);
  GEN_DEBUG(1, "C_MessageTlv::check_field_order() P_behave:   " << P_behave);
  GEN_DEBUG(1, "C_MessageTlv::check_field_order() P_position: " 
		  << P_position);
  GEN_DEBUG(1, "C_MessageTlv::check_field_order() end ret: " << L_ret);

  return (L_ret) ;
}
// field management
bool C_MessageTlv::set_field_value(T_pValueData P_value, 
				      int P_id,
				      int P_instance,
				      int P_sub_id) {

  bool                          L_found = true ;

  C_ProtocolTlv::T_MsgIdType L_id_type ;
  int                           L_id = m_protocol->retrieve_field_id(P_id, &L_id_type);

  switch (L_id_type) {
  case C_ProtocolTlv::E_MSG_ID_HEADER:
    set_header_value(L_id, P_value);
    break ;
  case C_ProtocolTlv::E_MSG_ID_BODY:
    set_body_value(L_id, P_value);
    break ;
  }

  return (L_found) ;
}

bool C_MessageTlv::get_field_value(int P_id, 
				      int P_instance,
				      int P_sub_id,
				      T_pValueData P_value) {
  bool                          L_found = true ;
  C_ProtocolTlv::T_MsgIdType L_id_type ;
  int                           L_id = m_protocol->retrieve_field_id(P_id, &L_id_type);

  GEN_DEBUG(1 , "C_MessageTlv::get_field_value() start ");
  GEN_DEBUG(1 , "C_MessageTlv::get_field_value() L_id_type = " 
		  << L_id_type << " (0 = hd, 1 = bd)");

  switch (L_id_type) {
  case C_ProtocolTlv::E_MSG_ID_HEADER:
    get_header_value (P_value, L_id);
    break ;
  case C_ProtocolTlv::E_MSG_ID_BODY:
    get_body_value (P_value, L_id);
    break ;
  }
  
  GEN_DEBUG(1 , "C_MessageTlv::get_field_value() end ret = " << L_found);
  return (L_found);
}

T_TypeType C_MessageTlv::get_field_type (int P_id,
					    int P_sub_id) {
  return (m_protocol->get_field_type(P_id,P_sub_id));
}

void C_MessageTlv::dump(iostream_output& P_stream) {
  GEN_DEBUG(1, "C_MessageTlv::dump() start");

  P_stream << *this << iostream_endl ;

  GEN_DEBUG(1, "C_MessageTlv::dump() end");
}

char* C_MessageTlv::name() {
//  GEN_DEBUG(1, "C_MessageTlv::name() name:[" 
//                <<  m_protocol -> message_name(m_header_id) << "]");
  return( m_protocol -> message_name(m_header_id));
}

int C_MessageTlv::get_id_message(){
  return (m_header_id) ;
}



bool C_MessageTlv::get_field_value(int P_id, 
                                    C_RegExp *P_reg,
                                    T_pValueData P_value) {
  return (true) ;
}


void   C_MessageTlv::update_message_stats  () {
}

int    C_MessageTlv::get_buffer (T_pValueData P_dest,
                                    T_MessagePartType P_header_body_type) {
  return (0) ;
}
