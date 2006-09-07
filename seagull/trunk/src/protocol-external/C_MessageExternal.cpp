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

#include "C_MessageExternal.hpp"
#include "Utils.hpp"
#include "ProtocolData.hpp"
#include "BufferUtils.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "C_ProtocolExternal.hpp"

#define min(a,b) (a)<(b) ? (a) : (b)
#define max(a,b) (a)>(b) ? (a) : (b)

C_MessageExternal::C_MessageExternal() : 
  C_MessageFrame(), 
  m_protocol(NULL),
  m_nb_values(0),
  m_all_values(NULL),
  m_ids(NULL){

  int L_i ;

  ALLOC_TABLE(m_header, T_pValueData, 
	      sizeof(T_ValueData), m_nb_header_fields);
  for (L_i = 0 ; L_i < m_nb_header_fields; L_i++) {
    m_header[L_i].m_type = E_TYPE_NUMBER ;
    m_header[L_i].m_value.m_val_number = 0 ;
    m_header[L_i].m_id = L_i ;
  }
  
  ALLOC_TABLE(m_body, T_pMultiValueData, 
	      sizeof(T_MultiValueData), m_nb_body_values);
  for(L_i = 0 ; L_i < m_nb_body_values; L_i++) {
    m_body[L_i].m_nb = 0 ;
    m_body[L_i].m_values = NULL ;
  }
  
  m_id = -1 ;
}

C_MessageExternal::~C_MessageExternal() {
  int L_i, L_j ;

  // first delete body values
  for (L_i = m_nb_header_fields; L_i < m_nb_values; L_i++) {
    for (L_j = 0; L_j < m_nb_body_fields; L_j++) {
      resetMemory(m_all_values[L_i][L_j]);
    }
    FREE_TABLE(m_all_values[L_i]);
  }
  FREE_TABLE(m_all_values);
  FREE_TABLE(m_ids);
  
  for(L_i = 0 ; L_i < m_nb_body_values; L_i++) {
    if (m_body[L_i].m_nb !=0) {
      FREE_TABLE(m_body[L_i].m_values);
      m_body[L_i].m_nb = 0 ;
    }
  }
  FREE_TABLE(m_body);
  for(L_i=0; L_i < m_nb_header_fields; L_i++) {
    if(m_header_not_present_table[m_id][L_i] == true ) {
      resetMemory(m_header[L_i]);
    }
  }
  FREE_TABLE(m_header);

  m_protocol = NULL ;
  m_nb_values = 0 ;
}

void C_MessageExternal::internal_reset() {

  int L_i, L_j ;

  for (L_i = 0 ; L_i < m_nb_header_fields; L_i++) {
    if(m_header_not_present_table[m_id][L_i] == true ) {
      resetMemory(m_header[L_i]);
    }
  }
  for(L_i = 0 ; L_i < m_nb_body_values; L_i++) {
    m_body[L_i].m_nb = 0 ;
    m_body[L_i].m_values = NULL ;
  }
  if (m_nb_values) {
    for (L_i = m_nb_header_fields; L_i < m_nb_values; L_i++) {
      for (L_j = 0; L_j < m_nb_body_fields; L_j++) {
	resetMemory(m_all_values[L_i][L_j]);
      }
      FREE_TABLE(m_all_values[L_i]);
    }
    FREE_TABLE(m_all_values);
    FREE_TABLE(m_ids);
    m_nb_values = 0 ;
  }
  m_protocol = NULL ;
  m_id = -1 ;

}

T_pValueData C_MessageExternal::get_session_id (C_ContextFrame *P_ctxt) { // session id
  T_pValueData L_value ;
  L_value = ((this)->*(m_get_id))(m_session_id,0);
  if (L_value == NULL) {
     L_value = get_out_of_session_id();
  }  
  return (L_value);
}

T_pValueData C_MessageExternal::get_out_of_session_id () {
  return (((this)->*(m_get_outof_session_id))(m_outof_session_id,0));
}


T_pValueData C_MessageExternal::get_id_header (int P_id, int P_instance) { 
   return (&m_header[P_id]);
}

T_pValueData C_MessageExternal::get_id_body (int   P_id, int P_instance) { 
  int L_id ;

  L_id = P_id - m_nb_header_fields -1 ;
  return (m_body[L_id].m_values[P_instance]);
}


T_pValueData C_MessageExternal::get_type () { 
  return (((this)->*(m_get_type))(m_type_id));
}

T_pValueData C_MessageExternal::get_type_header (int P_instance) { 
   return (&m_header[m_type_id]);
}

T_pValueData C_MessageExternal::get_type_body (int P_instance) { 
  int L_id ;

  L_id = m_type_id - m_nb_header_fields ;
  return (m_body[L_id].m_values[0]);

}

int C_MessageExternal::get_nb_header_fields () {
  return (m_nb_header_fields) ;
}
int C_MessageExternal::get_nb_body_values   () {
  return (m_nb_body_values) ;
}
int C_MessageExternal::get_nb_body_fields   () {
  return (m_nb_body_fields) ;
}

bool C_MessageExternal::update_fields (C_MessageFrame *P_ref) {
  return (true);
}
bool C_MessageExternal::compare_types (C_MessageFrame *P_ref) {
  bool               L_result ;
  C_MessageExternal *L_msg ;

  GEN_DEBUG(1, "C_MessageExternal::compare_types() start");
  L_msg = dynamic_cast<C_MessageExternal*>(P_ref) ;

  L_result = (L_msg == NULL) ? false : (L_msg->m_id == m_id) ;


  GEN_DEBUG(1, "C_MessageExternal::compare_types() end");
  return (L_result);
}

bool C_MessageExternal::check (C_MessageFrame  *P_ref, 
			       unsigned int     P_levelMask,
			       T_CheckBehaviour P_behave) {
  GEN_DEBUG(1, "C_MessageExternal::check() start");

  C_MessageExternal  *L_ref                ;

  int                 L_min_nb_values      ;
  int                 L_i                  ;
  bool                L_found = true       ;
  bool                L_additional = false ;

  L_ref = dynamic_cast<C_MessageExternal*>(P_ref);

  L_min_nb_values = min (L_ref->m_nb_values, m_nb_values);
  L_additional = (m_nb_values > L_ref->m_nb_values) ;
  
  // check value of component and operation-code
  if (L_found == true) {
    for (L_i = 0 ; L_i < L_min_nb_values ; L_i++) {
      if (L_ref->m_ids[L_i] !=
	  m_ids[L_i]) {
	L_found = false ;
	break;
      }
    } // for L_i
   
    if (L_found == false) {
      if ((P_levelMask & _check_level_mask[E_CHECK_LEVEL_FIELD_PRESENCE]) || 
	  (P_levelMask & _check_level_mask[E_CHECK_LEVEL_FIELD_ADDED])) {
	GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			      "check failed in [" 
			      <<  m_message_names[m_id] 
			      << "] " << m_protocol->message_name() 
                              << ", " << m_protocol->message_component_name ()
                              << " ["
			      << m_names[L_ref->m_ids[L_i]]
			      << "] not found");
	return (L_found);
      }
      
    }
   
    if (P_levelMask & _check_level_mask[E_CHECK_LEVEL_FIELD_ADDED]) {
      if (L_additional == true) { // additional values
	if (L_min_nb_values < m_nb_values) {
	  for (L_i = L_min_nb_values ; L_i < m_nb_values ; L_i++) {
	    GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
				  "check failed in [" 
				  <<  m_message_names[m_id] 
                                  << "] " << m_protocol->message_name() 
                                  << ", " << m_protocol->message_component_name ()
                                  << " ["
				  << m_names[this->m_ids[L_i]]
				  << "] not found");


	  }
	} else { //missing values
	  if (L_min_nb_values < L_ref->m_nb_values) {
	    for (L_i = L_min_nb_values ; L_i < L_ref->m_nb_values ; L_i++) {
	      GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
				    "check failed in [" 
				    <<  m_message_names[m_id] 
                                    << "] " << m_protocol->message_name() 
                                    << ", " << m_protocol->message_component_name ()
                                    << " ["
				    << m_names[L_ref->m_ids[L_i]]
				    << "] not found");
	    }
	  }
	}
      }
    }
  } // if L_found
   

  GEN_DEBUG(1, "C_MessageExternal::check() end");
  return (L_found);
}

bool C_MessageExternal::check_field_presence 
(int              P_id,
 T_CheckBehaviour P_behave,
 int              P_instance,
 int              P_sub_id) {
  
  bool            L_found = false ;

  GEN_DEBUG(1, "C_MessageExternal::check_field_presence() start");
  // this->dump(std::cerr);

  if (P_id < m_nb_header_fields) {
    if ((int) m_header[P_id].m_id == P_id) {
      L_found = true ;
    }
  } else {
    if (P_sub_id == -1) {
      if (m_body[P_id-m_nb_header_fields].m_nb != 0) {
	L_found = (P_instance <= m_body[P_id-m_nb_header_fields].m_nb ) ? true : false ;
      }
    } 
  }

  if (L_found == false) {
    if (P_id <  m_nb_header_fields) {
      GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			    "check failed in [" 
			    <<  m_message_names[m_id] 
                            << "] " << m_protocol->message_name() 
                            << ", " << m_protocol->message_component_name ()
                            << " ["
			    << m_names[P_id]
			    << "] not found");
      
    } else {
      if (P_sub_id == -1) {
	GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			      "check failed in [" 
			      <<  m_message_names[m_id] 
                              << "] " << m_protocol->message_name() 
                              << ", " << m_protocol->message_component_name ()
                              << " ["
			      << m_names[P_id]
			      << "] not found");
      }
    }
  }

  
  
  GEN_DEBUG(1, "C_MessageExternal::check_field_presence() end");
  
  return (L_found);
}


bool C_MessageExternal::check_field_value
(C_MessageFrame  *P_ref,
 int              P_id,
 T_CheckBehaviour P_behave,
 int              P_instance,
 int              P_sub_id) {
  
  bool            L_check = false ;
 
  C_MessageExternal  *L_ref                ;

  GEN_DEBUG(1, "C_MessageExternal::check_field_value() start");

  L_ref = dynamic_cast<C_MessageExternal*>(P_ref);

  if (P_id < m_nb_header_fields) {
    L_check = (L_ref->m_header[P_id] == m_header[P_id]) ;
  } else {
    if (P_sub_id != -1 ) {
      if ((P_instance <= m_body[P_id-m_nb_header_fields].m_nb) &&
          (m_body[P_id-m_nb_header_fields].m_nb != 0) &&
          (L_ref->m_ids[P_id] == m_ids[P_id])) {
	L_check =(m_body[P_id-m_nb_header_fields]
		  .m_values[P_instance][P_sub_id-m_nb_header_fields-m_nb_body_values]
		  ==
		  L_ref->m_body[P_id-m_nb_header_fields]
		  .m_values[P_instance][P_sub_id-m_nb_header_fields-m_nb_body_values]) ;
      } else {
        L_check = false;
      }
    } else {
      L_check = (P_instance <= m_body[P_id-m_nb_header_fields].m_nb) ;
    }

  }
  
  if (L_check == false) {
    if (P_sub_id != -1) {
      GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			    "check failed in [" 
			    <<  m_message_names[m_id] 
                            << "] " << m_protocol->message_name() 
                            << " and ["
			    << m_names[P_id]
			    << "] " << m_protocol->message_component_name ());
      GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			    "    value of field [" 
			    << m_names[P_sub_id]
			    << "] is incorrect");
      
    } else {
      if (P_id <  m_nb_header_fields) {
	GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			      "check failed in [" 
			      <<  m_message_names[m_id] 
                              << "] " << m_protocol->message_name() 
                              << ", value of " << m_protocol->message_component_name ()
                              << " ["
			      << m_names[P_id]
			      << "] is incorrect");

      } else {
	GEN_LOG_EVENT        (_check_behaviour_mask[P_behave], 
			      "check failed in [" 
			      <<  m_message_names[m_id] 
                              << "] " << m_protocol->message_name() 
                              << ", " << m_protocol->message_component_name ()
                              << " ["
			      << m_names[P_id]
			      << "] is incorrect ");
      }
    }
  }
  
  
  GEN_DEBUG(1, "C_MessageExternal::check_field_value() end");
  
  return (L_check);
}



bool C_MessageExternal::check_field_order
(int              P_id,
 T_CheckBehaviour P_behave,
 int              P_position) {

  
  bool            L_check = false ;
 
 
  GEN_DEBUG(1, "C_MessageExternal::check_field_value() start");

  if (P_id < m_nb_values) {
    L_check = ( m_ids[P_position] == P_id) ; 
  }

  if (L_check == false) {
    GEN_LOG_EVENT  (_check_behaviour_mask[P_behave], 
  		    "check order failed in [" 
  		    <<  m_message_names[m_id] 
                    << "] " << m_protocol->message_name() 
                    << ", received " << m_protocol->message_component_name ()
                    << " ["
		    << m_names[m_ids[P_position]+m_nb_header_fields]
		    << "] when expecting ["
  		    <<  m_names[P_id]
  		    << "]");
  }


  
  GEN_DEBUG(1, "C_MessageExternal::check_field_value() end");
  
  return (L_check);
}


T_TypeType C_MessageExternal::get_field_type  (int P_id, 
					       int P_sub_id) {
  GEN_DEBUG(1, "C_MessageExternal::get_field_type() start");
  T_TypeType L_type = E_TYPE_NUMBER;
  GEN_DEBUG(1, "C_MessageExternal::get_field_type() end");
  L_type = m_protocol->get_field_type(P_id,P_sub_id) ;
  return (L_type);
}

// bool fct (source, dest);

bool         C_MessageExternal::get_field_value (int P_id,
						 int P_instance,
						 int P_sub_id,
						 T_pValueData P_value) {

  GEN_DEBUG(1, "C_MessageExternal::get_field_value() start");

  // P_id : field_header_id, body_value_id or body_header_id
  // if body_value_id ou body_header_id => use instance
  // if body_header_id => must use a body_value_id 

  if (P_id < m_nb_header_fields) {
    copyValue(*(P_value),m_header[P_id],true) ;
  } else {
    copyValue(*(P_value),
	      m_body[P_id-m_nb_header_fields]
	      .m_values[P_instance][P_sub_id-m_nb_header_fields-m_nb_body_values]
	      ,true) ;
  }

  GEN_DEBUG(1, "C_MessageExternal::get_field_value() end");

  return (true);
}

bool         C_MessageExternal::set_field_value (T_pValueData P_value, 
						 int P_id,
						 int P_instance,
						 int P_sub_id) {

  GEN_DEBUG(1, "C_MessageExternal::set_field_value() start");

  if (P_id < m_nb_header_fields) {
    copyValue(m_header[P_id],*(P_value),true) ;
  } else {
    copyValue(
	      m_body[P_id-m_nb_header_fields]
	      .m_values[P_instance][P_sub_id-m_nb_header_fields-m_nb_body_values],
	      *(P_value),
	      true) ;
  }



  GEN_DEBUG(1, "C_MessageExternal::set_field_value() end");
  return (true);  
}


C_MessageExternal::C_MessageExternal(C_MessageExternal &P_val) {

  int L_i, L_j ;
  int L_id ;
  int L_instance ;
  int *L_body_instance ;

  ALLOC_TABLE(L_body_instance, int*, sizeof(int), 
	      m_nb_body_values);
  for (L_i = 0 ; L_i < m_nb_body_values; L_i ++) {
    L_body_instance[L_i] = 0 ;
  }

  m_protocol = P_val.m_protocol ;
  m_id = P_val.m_id ;
  m_nb_values = P_val.m_nb_values ;
  ALLOC_TABLE(m_header, T_pValueData, 
	      sizeof(T_ValueData), m_nb_header_fields);
  for (L_i = 0 ; L_i < m_nb_header_fields; L_i++) {
    m_header[L_i].m_type = E_TYPE_NUMBER ;
    copyValue(m_header[L_i], P_val.m_header[L_i], false);
  }
  ALLOC_TABLE(m_body, T_pMultiValueData, 
	      sizeof(T_MultiValueData), m_nb_body_values);

  for(L_i = 0 ; L_i < m_nb_body_values; L_i++) {
    m_body[L_i].m_nb = 0 ;
    m_body[L_i].m_values = NULL ;
  }

  ALLOC_TABLE(m_all_values,T_pValueData*, 
	      sizeof(T_pValueData), m_nb_values);
  ALLOC_TABLE(m_ids,int*, 
	      sizeof(int), m_nb_values);
  for (L_i = 0; L_i < m_nb_header_fields; L_i++) {
    m_all_values[L_i] = &(m_header[L_i]) ;
    m_ids[L_i]=L_i;
  }
  

  for (L_i = m_nb_header_fields; L_i < m_nb_values; L_i++) {

    ALLOC_TABLE(m_all_values[L_i],
		T_pValueData, sizeof(T_ValueData), m_nb_body_fields);
    m_ids[L_i] = P_val.m_ids[L_i] ;

    L_id = m_ids[L_i] ;

    for (L_j = 0 ; L_j < m_nb_body_fields; L_j ++) {
      copyValue(m_all_values[L_i][L_j],
		P_val.m_all_values[L_i][L_j],
		false);
    }
    if (L_body_instance[L_id] == 0) {
      L_body_instance[L_id] = P_val.m_body[L_id].m_nb ;
      ALLOC_TABLE(m_body[L_id].m_values, T_pValueData*,
		  sizeof(T_pValueData), L_body_instance[L_id]);
    } 
    
    L_instance = m_body[L_id].m_nb ;
    m_body[L_id].m_values[L_instance] = m_all_values[L_i] ;
    m_body[L_id].m_nb ++;
  }
  FREE_TABLE(L_body_instance);
}

C_MessageExternal& C_MessageExternal::operator= (C_MessageExternal &P_val) {

  int L_i, L_j ;
  int L_id ;
  int L_instance ;
  int *L_body_instance ;


  ALLOC_TABLE(L_body_instance, int*, sizeof(int), 
	      m_nb_body_values);
  for (L_i = 0 ; L_i < m_nb_body_values; L_i ++) {
    L_body_instance[L_i] = 0 ;
  }

  internal_reset() ;
  m_protocol = P_val.m_protocol ;
  m_id = P_val.m_id ;

  for (L_i = 0 ; L_i < m_nb_header_fields; L_i++) {
    copyValue(m_header[L_i], P_val.m_header[L_i], false);
  }
  m_nb_values = P_val.m_nb_values ;

  ALLOC_TABLE(m_all_values,T_pValueData*, 
	      sizeof(T_pValueData), m_nb_values);
  ALLOC_TABLE(m_ids,int*, 
	      sizeof(int), m_nb_values);
  for (L_i = 0; L_i < m_nb_header_fields; L_i++) {
    m_all_values[L_i] = &(m_header[L_i]) ;
    m_ids[L_i]=L_i;
  }
  for (L_i = m_nb_header_fields; L_i < m_nb_values; L_i++) {
    
    ALLOC_TABLE(m_all_values[L_i],
		T_pValueData, sizeof(T_ValueData), m_nb_body_fields);
    m_ids[L_i] = P_val.m_ids[L_i] ;
    L_id = m_ids[L_i] ;
    for (L_j = 0 ; L_j < m_nb_body_fields; L_j ++) {
      copyValue(m_all_values[L_i][L_j],
		P_val.m_all_values[L_i][L_j],
		false);
    }
    if (L_body_instance[L_id] == 0) {
      L_body_instance[L_id] = P_val.m_body[L_i].m_nb ;
      ALLOC_TABLE(m_body[L_id].m_values, T_pValueData*,
		  sizeof(T_pValueData), L_body_instance[L_id]);
    }
    L_instance = m_body[L_id].m_nb ;
    
    m_body[L_id].m_values[L_instance] = m_all_values[L_i] ;
    m_body[L_id].m_nb ++;
  }

  FREE_TABLE(L_body_instance);
  return (*this);
}

iostream_output& operator<< (iostream_output&   P_stream, 
			     C_MessageExternal& P_msg) {

  int L_i, L_j, L_k ;
  char *L_string_value ;

  P_stream << P_msg.m_message_names[P_msg.m_id] ;

  for (L_i = 0; L_i < P_msg.m_nb_header_fields ; L_i++) {
    if (P_msg.m_header_not_present_table[P_msg.m_id][L_i] == true) {
      P_stream << iostream_endl ;
      P_stream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL ;
      P_stream << "[" 
	       << P_msg.m_names[P_msg.m_ids[L_i]] 
	       << "] = "  ;

      L_string_value = (P_msg.m_protocol)
	->get_string_value(L_i, P_msg.m_all_values[L_i]);

      P_stream << "[" ;
      if (L_string_value == NULL) {
	P_stream << *P_msg.m_all_values[L_i] ;
      } else {
	P_stream << L_string_value ;
      }
      P_stream << "]" ;
    }
  }

  for (L_i = P_msg.m_nb_header_fields ; L_i < P_msg.m_nb_values ; L_i++) {

    P_stream << iostream_endl ;
    P_stream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL ;
    P_stream << "[" 
	     << P_msg.m_names[P_msg.m_ids[L_i]+P_msg.m_nb_header_fields] 
	     << "] = ["  ;

    L_k = P_msg.m_ids[L_i] ;

    for (L_j = 0 ; L_j < P_msg.m_nb_body_fields ; L_j++) {
      
      if (P_msg.m_body_not_present_table[L_k][L_j] == true) {
	P_stream << iostream_endl ;
	P_stream << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL ;
	P_stream << " [" 
		 << P_msg.m_names[P_msg.m_all_values[L_i][L_j].m_id]
  		 << "] = " ;
	L_string_value = 
	  (P_msg.m_protocol)
	  ->get_string_value_field_body(L_j, 
					&(P_msg.m_all_values[L_i][L_j]));
	
	P_stream << "[" ;
	if (L_string_value == NULL) {
	  P_stream << P_msg.m_all_values[L_i][L_j] ;
	} else {
	  P_stream << L_string_value ;
	}
	P_stream << "]" ;
      }
    }
    
    P_stream << iostream_endl << GEN_HEADER_LOG << GEN_HEADER_NO_LEVEL ;
    P_stream << " ]" ;
    
  }


  return (P_stream) ;
}

int C_MessageExternal::m_nb_header_fields = 0 ;
int C_MessageExternal::m_nb_body_values = 0 ;
int C_MessageExternal::m_nb_body_fields = 0 ;
int C_MessageExternal::m_session_id = 0     ;
int C_MessageExternal::m_outof_session_id = 0 ;
int C_MessageExternal::m_type_id = 0 ;
C_MessageExternal::T_GetId_instance C_MessageExternal::m_get_id = NULL ;


C_MessageExternal::T_GetId_instance C_MessageExternal::m_get_outof_session_id = NULL ;
C_MessageExternal::T_GetId C_MessageExternal::m_get_type = NULL ;


char** C_MessageExternal::m_names = NULL ;
char** C_MessageExternal::m_message_names = NULL ;

bool** C_MessageExternal::m_body_not_present_table = NULL ;
bool** C_MessageExternal::m_header_not_present_table = NULL ;

void C_MessageExternal::dump(iostream_output& P_stream) {

  P_stream << *this << iostream_endl ;

}

void C_MessageExternal::set_protocol_data (int P_nb_header_fields,
					   int P_nb_body_values,
					   int P_nb_body_fields,
					   int P_session_id,
					   int P_outof_session_id,
					   int P_type_id,
					   char **P_names,
					   char **P_message_names,
					   bool **P_body_not_present_table,
					   bool **P_header_not_present_table) {
 
  m_nb_header_fields =  P_nb_header_fields ;
  m_nb_body_values = P_nb_body_values ;
  m_nb_body_fields = P_nb_body_fields ;
  m_session_id = P_session_id ;
  m_outof_session_id = P_outof_session_id ;
  m_type_id = P_type_id ;

  if (m_session_id < (m_nb_header_fields - 1)) {
    m_get_id = &C_MessageExternal::get_id_header ;
  } else if (m_session_id < (m_nb_header_fields + m_nb_body_values - 1)) {
    m_get_id = &C_MessageExternal::get_id_body ;
  } 

  if (m_outof_session_id < (m_nb_header_fields - 1)) {
    m_get_outof_session_id = &C_MessageExternal::get_id_header ;
  } else if (m_outof_session_id < (m_nb_header_fields + m_nb_body_values - 1)) {
    m_get_outof_session_id = &C_MessageExternal::get_id_body ;
  } 

  if (m_type_id < (m_nb_header_fields - 1)) {
    m_get_type = &C_MessageExternal::get_type_header ;
  } else if (m_type_id< (m_nb_header_fields + m_nb_body_values - 1)) {
    m_get_type = &C_MessageExternal::get_type_body ;
  } 

  m_names = P_names ;
  m_message_names = P_message_names ;
  m_body_not_present_table = P_body_not_present_table ;
  m_header_not_present_table = P_header_not_present_table ;

}

C_MessageExternal::C_MessageExternal (C_ProtocolExternal *P_protocol,
				      T_pValueData        P_header, 
				      T_pValueDataList    P_body,       
				      list_t<int>        *P_body_id,
				      int                *P_instance_body) {

  int                       L_i, L_index ;
  int                       L_id, L_instance ;
  T_ValueDataList::iterator L_value_it ;
  list_t<int>::iterator     L_id_it    ;

  GEN_DEBUG(1, "C_MessageExternal::C_MessageExternal() start");
  
  m_protocol = P_protocol ;
  m_header = P_header ;
  m_nb_values = m_nb_header_fields ;  

  ALLOC_TABLE(m_body, T_pMultiValueData, 
	      sizeof(T_MultiValueData), m_nb_body_values);
  for(L_i = 0 ; L_i < m_nb_body_values; L_i++) {
    m_body[L_i].m_nb = 0 ;
    if (P_instance_body[L_i] != 0 ) {
      ALLOC_TABLE(m_body[L_i].m_values, T_pValueData*,
		  sizeof(T_pValueData), P_instance_body[L_i]);
    } else {
      m_body[L_i].m_values = NULL ;
    }
  }


  if (!P_body->empty()) {
    m_nb_values += P_body->size() ;
  }
  ALLOC_TABLE(m_all_values,T_pValueData*, 
	      sizeof(T_pValueData), m_nb_values);

  ALLOC_TABLE(m_ids,int*, 
	      sizeof(int), m_nb_values);
  L_index = 0 ;
  for (L_i = 0; L_i < m_nb_header_fields; L_i++) {
    m_all_values[L_index] = &(m_header[L_i]) ;
    m_ids[L_index] = L_i ;
    L_index++ ;
  }

  if (!P_body->empty()) {
    L_id_it = P_body_id->begin() ;
    for(L_value_it = P_body->begin();
	L_value_it != P_body->end();
	L_value_it++) {
      m_all_values[L_index] = *L_value_it ;
      L_id = *L_id_it;
      m_ids[L_index] = L_id ;
      L_instance = m_body[L_id].m_nb ;
      m_body[L_id].m_values[L_instance] = *L_value_it ;
      m_body[L_id].m_nb ++ ;
      L_index++;
      L_id_it++;
    }
  }

  GEN_DEBUG(1, "C_MessageExternal::C_MessageExternal() end");
}


char* C_MessageExternal::name() {
  return(m_message_names[m_id]);
}

int C_MessageExternal::get_id_message(){
  return (m_id) ;
}


bool C_MessageExternal::get_field_value(int P_id, 
                                        C_RegExp *P_reg,
                                        T_pValueData P_value) {
  return (true) ;
}

void C_MessageExternal::update_message_stats () {
}
