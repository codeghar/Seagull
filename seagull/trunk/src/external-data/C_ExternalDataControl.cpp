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

#include "C_ExternalDataControl.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "Utils.hpp"
#include "BufferUtils.hpp"
#include "list_t.hpp"
#include "integer_t.hpp"

#define   START_FIELD '"'
#define   STOP_FIELD  '"'
#define   NEXT_FIELD  ';'

C_ExternalDataControl::C_ExternalDataControl() {
   GEN_DEBUG(1, "C_ExternalDataControl::C_ExternalDataControl() start");
   m_number_line      = 0    ;
   m_allocted_nb_line = 0    ;
   m_max_size_line    = 0    ;
   m_nb_field         = 0    ;

   m_line_selected      = 0    ;
   m_line_selected_max  = 0    ; 

   m_file          = NULL ;
   m_data_table    = NULL ;
   m_regExpr1      = NULL ;
   m_regExpr2      = NULL ;
   m_regExpr3      = NULL ;
   m_regExpr4      = NULL ;
   m_field_type_table = NULL ;
   GEN_DEBUG(1, "C_ExternalDataControl::C_ExternalDataControl() end");
}

C_ExternalDataControl::~C_ExternalDataControl() {

   size_t         L_i, L_j ;
   T_pValueData   L_value  = NULL;

   GEN_DEBUG(1, "C_ExternalDataControl::~C_ExternalDataControl() start");
   m_file = NULL ;

   GEN_DEBUG(1, "C_ExternalDataControl::~C_ExternalDataControl() " 
		   << "m_allocted_nb_line = " << m_allocted_nb_line 
		   << " m_nb_field = " << m_nb_field);

   for (L_i = 0; L_i < m_allocted_nb_line ; L_i ++) {
     for (L_j = 0; L_j < m_nb_field; L_j++) {
       L_value = m_data_table[L_i][L_j] ;
       if (L_value != NULL) {
         if(L_value->m_type == E_TYPE_STRING) {
	   FREE_TABLE(L_value->m_value.m_val_binary.m_value);
         }
         FREE_VAR(L_value);
         m_data_table[L_i][L_j] = NULL;
       }
     }
     FREE_TABLE (m_data_table[L_i]);
     m_data_table[L_i] = NULL;
   }
   FREE_TABLE (m_data_table);
   FREE_TABLE(m_field_type_table);

   GEN_DEBUG(1, "C_ExternalDataControl::~C_ExternalDataControl() end");
}

bool         C_ExternalDataControl::init (char * P_file_name) {

  bool            L_result = false ;

  GEN_DEBUG(1, "C_ExternalDataControl::init() start");

  m_file_name = P_file_name ;

  // calculate the max buffer size 
  L_result = max_buffer_size(); 

  if(L_result == false ) {
    GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Unable to determine max buffer size in file ["
	      << P_file_name << "]");
    return (L_result);
  }
  
  // analyze this file
  L_result = create_regexp() ;
  if (L_result == false) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Regular expression building is failed");
    return (L_result);
  }

  L_result = analyze();

  delete_regexp();

  if (L_result == false) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "External data analysis failed");
    return (L_result);
  }

  L_result  = true ;
  GEN_DEBUG(1, "C_ExternalDataControl::init() end");
  return (L_result);
}

T_pValueData C_ExternalDataControl::get_value (int P_line, int P_field) {
  return (m_data_table[P_line][P_field]);
}

size_t C_ExternalDataControl::get_nb_field () {
  return (m_nb_field);
}

T_TypeType& C_ExternalDataControl::get_field_type (int P_field_id) {
  return ( m_field_type_table[P_field_id]);
}

bool         C_ExternalDataControl::max_buffer_size () {

  bool     L_result    = false ;
  size_t   L_max       = 0 ;
  size_t   L_char_line = 0 ;
  char     L_char  ;
  bool     L_line_with_data = false ;

  m_allocted_nb_line = 0 ;

  GEN_DEBUG(1, "C_ExternalDataControl::max_buffer_size() start on file ["
		  << m_file_name << "]");

  NEW_VAR(m_file, fstream_input(m_file_name));

  if (!m_file->good()) {
    DELETE_VAR(m_file);
    GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Unable to open file [" << m_file_name << "]");
    return (L_result) ;
  }

  // read one char 
  while (!m_file->eof()) {
     m_file->get(L_char);
     L_char_line ++ ;
     if (L_char == '\n') {
       if (L_max < L_char_line) {
	  L_max = L_char_line ;
       }
       L_char_line = 0 ;
       L_line_with_data = false ;
     }
     if (L_line_with_data == false) {
       if (L_char == ';') {
	 L_line_with_data = true ;
	 m_allocted_nb_line ++ ;
       }
     }
  }
  m_file->close();
  DELETE_VAR(m_file);

  if (L_max) {
     m_max_size_line = L_max ;
     L_result        = true ;
  }

  GEN_DEBUG(1, "m_allocted_nb_line is ["
		  << m_allocted_nb_line << "]");

  GEN_DEBUG(1, "max_char_buffer_size is [" 
	    << m_max_size_line << "]");
  GEN_DEBUG(1, "C_ExternalDataControl::max_char_buffer_size() end with " 
	    << L_result);
  return  (L_result) ;

}

bool         C_ExternalDataControl::analyze () {

  char          *L_line   = NULL     ;
  bool           L_result = false    ;
  bool           L_first_line = true ;
  int            L_pos    = 0        ;

  GEN_DEBUG(1, "C_ExternalDataControl::analyze() start");

  ALLOC_TABLE(L_line, char*, sizeof(char), m_max_size_line);

  GEN_DEBUG(1, "C_ExternalDataControl::analyze() m_file_name=" 
	    << m_file_name) ;

  NEW_VAR(m_file, fstream_input(m_file_name));

  if (!m_file->good()) {
    DELETE_VAR(m_file);
    GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Unable to open file [" << m_file_name << "]");
    return (L_result) ;
  }

  m_line_selected = 0 ; 

  do {

    L_pos = get_line (L_line) ;
    GEN_DEBUG(1, "C_ExternalDataControl::analyze() get_line() L_pos="
	      << L_pos) ;
    if (L_pos == -1) break ;
    
    // data found
    GEN_DEBUG(1, "data: pos=" << L_pos << " line=" << L_line+L_pos) ;

    if (L_first_line == true) {
      L_result = analyze_first_data (L_line+L_pos);
      L_first_line = false ;
    } else {
      L_result = analyze_data (L_line+L_pos);
      if (L_result == true) { m_line_selected ++ ; }
    }
    if (L_result == false) break ;

  } while (!m_file->eof()) ;


  if (L_result == true) {
    m_number_line = m_line_selected ;
    if (m_number_line > 1) {
      m_line_selected     = (m_number_line - 1) ; 
      m_line_selected_max = (m_number_line - 1) ;
    } 

    if (m_number_line == 0){
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "The file [" << m_file_name << "] doesn't contain any valid lines");
      L_result = false ;
    }
  }

  FREE_TABLE(L_line);

  m_file->close();
  DELETE_VAR(m_file);

  GEN_DEBUG(1, "C_ExternalDataControl::analyze() end");
  return (L_result);
}

int C_ExternalDataControl::get_line(char *P_line) {
 
  int    L_ret         = -1    ;
  int    L_line_size           ;
  
  char                    *L_search = NULL ;
  regmatch_t               L_pmatch        ;
  int                      L_status        ;

  GEN_DEBUG(1, "C_ExternalDataControl::get_line() start");
  P_line[0] = '\0' ;

  while (!m_file->eof()) {

    m_file->getline(P_line, m_max_size_line);
    L_line_size = strlen (P_line);

    GEN_DEBUG(1, "C_ExternalDataControl::get_line() P_line="
	      << P_line << " size=" << L_line_size);

    if (L_line_size == 0) continue ;

    // skip blank
    L_search = P_line ;
    L_status = regexec (m_regExpr1, L_search, 1, &L_pmatch, 0) ;

    // matching is OK
    if (L_status == 0) {

      if (L_pmatch.rm_eo == L_line_size) { L_ret = -1 ; continue ; }
      L_ret = L_pmatch.rm_eo ;
      L_search += L_ret ;	
      
    } else {
      // no blank on the beginning of this line
      L_ret = 0 ;
    }
    
    // skip comment 1
    L_status = regexec (m_regExpr2, L_search, 1, &L_pmatch, 0) ;
    // matching is OK
    if (L_status == 0) { 
      L_ret = -1 ; 
      continue ;
    }

    // skip comment 2
    L_status = regexec (m_regExpr3, L_search, 1, &L_pmatch, 0) ;
    // matching is OK
    if (L_status == 0) { 
      L_ret = -1 ; 
      continue ;
    }

    break ;
      
  }
  
  GEN_DEBUG(1, "C_ExternalDataControl::get_line() return " << L_ret);
  return (L_ret);

}


bool C_ExternalDataControl::create_regexp() {

  int                      L_status        ;
  char                     L_buffer[100]   ;
  

  GEN_DEBUG(1, "C_ExternalDataControl::create_regexp() start ");

  ALLOC_VAR(m_regExpr1, regex_t*, sizeof(regex_t));
  ALLOC_VAR(m_regExpr2, regex_t*, sizeof(regex_t));
  ALLOC_VAR(m_regExpr3, regex_t*, sizeof(regex_t));
  ALLOC_VAR(m_regExpr4, regex_t*, sizeof(regex_t));
  
  L_status = regcomp (m_regExpr1, "^[[:blank:]]+", REG_EXTENDED) ;
  if (L_status != 0) {
    regerror(L_status, m_regExpr1, L_buffer, 100);
    regfree (m_regExpr1) ;
    GEN_ERROR(E_GEN_FATAL_ERROR, "regcomp error: [" << L_buffer << "]");

    GEN_DEBUG(1, "C_ExternalDataControl::create_regexp() end Error ");

    return (false);
  }

  L_status = regcomp (m_regExpr2, "^#.*$", REG_EXTENDED) ;
  if (L_status != 0) {
    regerror(L_status, m_regExpr2, L_buffer, 100);
    regfree (m_regExpr2) ;
    GEN_ERROR(E_GEN_FATAL_ERROR, "regcomp error: [" << L_buffer << "]");

    GEN_DEBUG(1, "C_ExternalDataControl::create_regexp() end Error ");

    return (false);
  }

  L_status = regcomp (m_regExpr3, "^//.*$", REG_EXTENDED) ;
  if (L_status != 0) {
    regerror(L_status, m_regExpr3, L_buffer, 100);
    regfree (m_regExpr3) ;
    GEN_ERROR(E_GEN_FATAL_ERROR, "regcomp error: [" << L_buffer << "]");

    GEN_DEBUG(1, "C_ExternalDataControl::create_regexp() end Error ");

    return (false);
  }

  L_status = regcomp (m_regExpr4, "[[:blank:]]*;[[:blank:]]*", REG_EXTENDED) ;
  if (L_status != 0) {
    regerror(L_status, m_regExpr4, L_buffer, 100);
    regfree (m_regExpr4) ;
    GEN_ERROR(E_GEN_FATAL_ERROR, "regcomp error: [" << L_buffer << "]");

    GEN_DEBUG(1, "C_ExternalDataControl::create_regexp() end Error ");
    return (false);
  }



  GEN_DEBUG(1, "C_ExternalDataControl::create_regexp() end Ok ");

  return (true);
  
}

void C_ExternalDataControl::delete_regexp() {

  if (m_regExpr1) {
    regfree (m_regExpr1);
    FREE_VAR(m_regExpr1);
  }
  if (m_regExpr2) {
    regfree (m_regExpr2);
    FREE_VAR(m_regExpr2);
  }
  if (m_regExpr3) {
    regfree (m_regExpr3);
    FREE_VAR(m_regExpr3);
  }
  if (m_regExpr4) {
    regfree (m_regExpr4);
    FREE_VAR(m_regExpr4);
  }
}
 
bool C_ExternalDataControl::analyze_first_data (char*P_line) {

  char              *L_ptr = P_line ;
  char              *L_ptrField = NULL ;
  size_t             L_size = 0 ;
  size_t             L_next = 0 ;
  size_t             L_i, L_j ;
  bool               L_ret = true ;

  char              *L_type_name ;
  T_TypeType         L_type    = E_UNSUPPORTED_TYPE ;
  int                L_k       = 0 ;

  list_t<T_TypeType> L_type_list ;
  list_t<T_TypeType>::iterator L_it ;

  
  m_nb_field = 0 ;
  while ((L_ptrField = get_field (L_ptr, &L_size, &L_next)) != NULL) {
    GEN_DEBUG(1, "found: field=" 
	      << L_ptrField << " size=" 
	      << L_size << " next=" << L_next) ;

    L_ptr = L_ptrField + L_next ;
    L_next = 0 ;

    m_nb_field ++ ;
    ALLOC_TABLE(L_type_name, char*, sizeof(char), L_size+1);
    memcpy(L_type_name, L_ptrField, L_size);
    L_type_name[L_size]= '\0';

    // decode type
    L_type = E_UNSUPPORTED_TYPE ;
    for(L_k=0; L_k < (int) E_UNSUPPORTED_TYPE; L_k++) {
	if (strcmp(L_type_name, type_type_table[L_k])==0) {
	  L_type = (T_TypeType) L_k ;
	}
    }
    
    if (L_type == E_UNSUPPORTED_TYPE) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "typedef type value ["
		<< L_type_name << "] unsupported");
      L_ret = false ;
      break ;
    }

    switch (L_type) {
    case E_TYPE_NUMBER:
    case E_TYPE_SIGNED:
    case E_TYPE_STRING:
    case E_TYPE_NUMBER_64:
    case E_TYPE_SIGNED_64:
      break ;
    default:
      GEN_ERROR(E_GEN_FATAL_ERROR, "type ["
		<< L_type_name << "] unsupported for external data file");
      L_ret = false ;
      break ;
    }

    L_type_list.push_back(L_type);

    FREE_TABLE(L_type_name);

  }

  if (!L_type_list.empty()) {
  
    ALLOC_TABLE(m_field_type_table, T_TypeType*,
		sizeof(T_TypeType), m_nb_field);

    L_i = 0 ;
    for (L_it = L_type_list.begin();
	 L_it != L_type_list.end();
	 L_it++) {
      m_field_type_table[L_i] = *L_it ;
      L_i ++ ;
    }
    L_type_list.erase(L_type_list.begin(), L_type_list.end());
    
    GEN_DEBUG(1, "m_data_table: m_allocted_nb_line = " 
	      << m_allocted_nb_line << " m_nb_field =" 
	      << m_nb_field ) ;

    // Create the  m_data_table
    ALLOC_TABLE(m_data_table, T_pValueData**,
	      sizeof(T_pValueData*), m_allocted_nb_line);
    
    for (L_i = 0 ; L_i < m_allocted_nb_line ; L_i ++) {
      ALLOC_TABLE(m_data_table[L_i], T_pValueData*,
		sizeof(T_pValueData), m_nb_field);
    }
    
    // init the table 
    for (L_i = 0; L_i < m_allocted_nb_line ; L_i++) {
      for (L_j = 0; L_j < m_nb_field; L_j++) {
	m_data_table[L_i][L_j] = NULL ;
      }
    }
  } else {
    GEN_ERROR(E_GEN_FATAL_ERROR, "No field definition (first line) found in file ["
	      << m_file_name << "]");
    L_ret = false ;
  }

  return (L_ret);

}

bool C_ExternalDataControl::analyze_data (char*P_line) {

  bool  L_ret = true ;
  char *L_ptr = P_line ;
  char              *L_ptrField = NULL ;
  size_t             L_size = 0 ;
  size_t             L_next = 0 ;
  char              *L_field_value ;
  size_t             L_nb_field = 0 ;

  T_TypeType         L_field_type ;
  T_pValueData       L_value ;

  while ((L_ptrField = get_field (L_ptr, &L_size, &L_next)) != NULL) {


    ALLOC_TABLE(L_field_value, char*, sizeof(char), L_size+1);
    memcpy(L_field_value, L_ptrField, L_size);
    L_field_value[L_size]= '\0';

    GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() " 
	      << "found: field = " << L_field_value);

    // now convert field to value
    L_field_type = m_field_type_table[L_nb_field];
    ALLOC_VAR(L_value, T_pValueData, sizeof(T_ValueData));
    L_value->m_type = L_field_type ;

    GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
	      << "type of value = " << L_value->m_type);

    switch (L_field_type) {
    case E_TYPE_SIGNED: {
      
      T_Integer32 L_signed_value ;
      char *L_endstr ;

      if (strlen(L_field_value) > 0) {
	if ((strlen(L_field_value)>2) 
	    && (L_field_value[0] == '0') 
	    && (L_field_value[1] == 'x')) { // hexa buffer value
	  L_signed_value = strtol_f (L_field_value, &L_endstr,16) ; 
	} else {
	  L_signed_value = strtol_f (L_field_value, &L_endstr,10) ;
	}
	
	if (L_endstr[0] != '\0') {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		    << L_field_value << "] bad format in file ["
		    << m_file_name << "]");
	  L_ret = false ;
	}
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "signed value = " << L_signed_value);
	L_value->m_value.m_val_signed = L_signed_value ;
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Field empty where number value was expected"
		  << " in file ["
		  << m_file_name << "]");
	L_ret = false ;
      }

    } 
      break ;
    case E_TYPE_NUMBER: {
      
      T_UnsignedInteger32 L_unsigned_value ;
      char *L_endstr ;
      if (strlen(L_field_value) > 0) {
	if ((strlen(L_field_value)>2) 
	    && (L_field_value[0] == '0') 
	    && (L_field_value[1] == 'x')) { // hexa buffer value
	  L_unsigned_value = strtoul_f (L_field_value, &L_endstr,16) ; 
	} else {
	  L_unsigned_value = strtoul_f (L_field_value, &L_endstr,10) ;
	}


	if (L_endstr[0] != '\0') {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		    << L_field_value << "] bad format in file ["
		    << m_file_name << "]");
	  L_ret = false ;
	}
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "unsigned value =" << L_unsigned_value);
	L_value->m_value.m_val_number = L_unsigned_value ;
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "Field empty where number value was expected"
		  << " in file ["
		  << m_file_name << "]");
      }
    }
      break ;

    case E_TYPE_STRING: {

      if ((strlen(L_field_value)>2) 
	  && (L_field_value[0] == '0') 
	  && (L_field_value[1] == 'x')) { // hexa buffer value

	char  *L_ptr = L_field_value+2 ;
	size_t L_res_size ;

	L_value -> m_value.m_val_binary.m_value  
	  = convert_hexa_char_to_bin(L_ptr, &L_res_size);

	if (L_value -> m_value.m_val_binary.m_value == NULL ) {
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "Bad buffer size for hexadecimal buffer ["
		    << L_field_value << "]" ); 
	  L_ret = false ;
	} else {
	  L_value -> m_value.m_val_binary.m_size = L_res_size ;	  
	}

      } else { // direct string value
        
	L_value -> m_value.m_val_binary.m_size = L_size ;

	if (L_size != 0) {
	  ALLOC_TABLE(L_value -> m_value.m_val_binary.m_value,
		      unsigned char*,
		      sizeof(unsigned char),
		      L_size);
	  memcpy(L_value -> m_value.m_val_binary.m_value,
		 L_field_value,
		 L_size);
	} else {
	  L_value -> m_value.m_val_binary.m_value = NULL ;
	}
      }
      
      GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		<< "string value size=" 
		<< L_value -> m_value.m_val_binary.m_size) ;
    }
      break ;

    case E_TYPE_SIGNED_64: {
      
      T_Integer64 L_signed_value ;
      char *L_endstr ;

      if (strlen(L_field_value) > 0) {
	if ((strlen(L_field_value)>2) 
	    && (L_field_value[0] == '0') 
	    && (L_field_value[1] == 'x')) { // hexa buffer value
	  L_signed_value = strtoll_f (L_field_value, &L_endstr,16) ; 
	} else {
	  L_signed_value = strtoll_f (L_field_value, &L_endstr,10) ;
	}
	
	if (L_endstr[0] != '\0') {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		    << L_field_value << "] bad format in file ["
		    << m_file_name << "]");
	  L_ret = false ;
	}
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "signed64 value = " << L_signed_value);
	L_value->m_value.m_val_signed_64 = L_signed_value ;
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "empty field for number64 expected"
		  << " in file ["
		  << m_file_name << "]");
	L_ret = false ;
      }

    } 
      break ;
    case E_TYPE_NUMBER_64: {
      
      T_UnsignedInteger64 L_unsigned_value ;
      char *L_endstr ;
      if (strlen(L_field_value) > 0) {
	if ((strlen(L_field_value)>2) 
	    && (L_field_value[0] == '0') 
	    && (L_field_value[1] == 'x')) { // hexa buffer value
	  L_unsigned_value = strtoull_f (L_field_value, &L_endstr,16) ; 
	} else {
	  L_unsigned_value = strtoull_f (L_field_value, &L_endstr,10) ;
	}


	if (L_endstr[0] != '\0') {
	  GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		    << L_field_value << "] bad format in file ["
		    << m_file_name << "]");
	  L_ret = false ;
	}
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "unsigned64 value =" << L_unsigned_value);
	L_value->m_value.m_val_number_64 = L_unsigned_value ;
      } else {
	GEN_ERROR(E_GEN_FATAL_ERROR, "empty field for number64 expected"
		  << " in file ["
		  << m_file_name << "]");
      }
    }
      break ;

    default:
      GEN_FATAL(E_GEN_FATAL_ERROR,
      		"Unsupported type for external data");
      break ;
    } // end switch

    m_data_table[m_line_selected][L_nb_field] = L_value ;

    FREE_TABLE(L_field_value);
    
    if (L_ret == false) break ;

    L_ptr = L_ptrField + L_next ;
    L_next = 0 ;

    L_nb_field ++ ;
    if (L_nb_field > m_nb_field) break ;

  }  

  if (L_nb_field != m_nb_field) {
    GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Illegal number of field for line ["
	      << P_line << "] expected [" 
	      << m_nb_field << "] in texternal data file ["
	      << m_file_name << "]");
    L_ret = false ;
  } 

  return (L_ret);
  
}

char* C_ExternalDataControl::get_field(char *P_line, size_t *P_size, size_t *P_next) {
  
  char      *L_ptr = P_line ;
  char      *L_ptr_end ;
  char      *L_search = NULL ;
  size_t     L_size ;
  int        L_status = 0 ;
  regmatch_t L_pmatch ;
  

  L_size = strlen(L_ptr);
  if (L_size == 0) return (NULL);

  // field value
  if (L_ptr[0] == START_FIELD) {
    L_search = L_ptr + 1 ;

    if (*L_search != '\0') {
      while ((L_ptr_end = strchr(L_search, STOP_FIELD)) != NULL) {
	if (*(L_ptr_end-1) != '\\') break ;
	if ((L_search = L_ptr_end + 1) == '\0') { return (NULL) ; }
      }
      if (L_ptr_end == NULL) { 
	return (NULL); 
      } else {
	*P_size = L_ptr_end - L_search ;
      }
    } else {
      return (NULL);
    }
  } else {
    return (NULL);
  }

  // blank ; blank
  L_search = L_ptr_end + 1 ;

  if ((*L_search) == '\0') { return (NULL); }

  L_status = regexec (m_regExpr4, L_search, 1, &L_pmatch, 0) ;
  // matching is OK
  if (L_status == 0) { 
    *P_next = *P_size + (L_pmatch.rm_eo - L_pmatch.rm_so) + 1 ;
  } else {
    return (NULL);
  }

  return (L_ptr+1);

}











