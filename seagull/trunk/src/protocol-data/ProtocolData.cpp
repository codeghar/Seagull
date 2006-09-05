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

#include "ProtocolData.hpp"
#include "string_t.hpp"
#include "integer_t.hpp"

#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "BufferUtils.hpp"
#include "Utils.hpp"

const char* type_type_table[] = {
  "number",
  "signed",
  //  "real",
  "string",
  "composed",
  "grouped",
  "number64",
  "signed64" } ;

static const char* default_value_table[] = {
  "0",
  "0",
  "" } ;

iostream_output& operator<<(iostream_output&           P_ostream, 
			    T_TypeType&                P_value) {
  switch (P_value) {
  case E_TYPE_NUMBER:
  case E_TYPE_SIGNED:
  case E_TYPE_STRING:
  case E_TYPE_STRUCT:
  case E_TYPE_GROUPED:
  case E_TYPE_NUMBER_64:
  case E_TYPE_SIGNED_64:
    P_ostream << type_type_table[(int)P_value] ;
    break ;
  default:
    P_ostream << "UNSUPPORTED_TYPE" ;
    break ;
  }
  return (P_ostream);
}

iostream_output& operator<< (iostream_output&         P_ostream, 
			     T_ValueData& P_valueData){

  switch (P_valueData.m_type) {

  case E_TYPE_NUMBER:
    P_ostream << P_valueData.m_value.m_val_number ;
    break ;
  case E_TYPE_SIGNED:
    P_ostream << P_valueData.m_value.m_val_signed ;
    break ;
  case E_TYPE_STRING: {
//      string_t L_value ;
//      L_value.append((char*)P_valueData.m_value.m_val_binary.m_value,
//  		   P_valueData.m_value.m_val_binary.m_size);
//      P_ostream << L_value ;


    if (P_valueData.m_value.m_val_binary.m_value != NULL) {
      static char L_hexa_buf [50] ;
      const  size_t L_cNum = 16 ;
      size_t L_i, L_nb ;
      unsigned char*L_cur ;
      size_t L_buffer_size = P_valueData.m_value.m_val_binary.m_size ;

      L_nb = L_buffer_size / L_cNum ;
      L_cur = P_valueData.m_value.m_val_binary.m_value ;
      P_ostream << " " ;	
      for (L_i = 0 ; L_i < L_nb; L_i++) {
        pretty_binary_buffer(L_cur, L_cNum, L_hexa_buf);
        P_ostream << L_hexa_buf ;
        L_cur += L_cNum ;
      }
      L_nb = L_buffer_size % L_cNum ;
      if (L_nb != 0) {
        pretty_binary_buffer(L_cur, L_nb, L_hexa_buf);
        P_ostream << L_hexa_buf ;
      }
    }


  }
    break ;




  case E_TYPE_STRUCT:
    P_ostream << P_valueData.m_value.m_val_struct.m_id_1 << ";" 
	      << P_valueData.m_value.m_val_struct.m_id_2 ;
    break ;

  case E_TYPE_NUMBER_64:
    P_ostream << P_valueData.m_value.m_val_number_64 ;
    break ;
  case E_TYPE_SIGNED_64:
    P_ostream << P_valueData.m_value.m_val_signed_64 ;
    break ;

  default:
    GEN_FATAL(E_GEN_FATAL_ERROR, "operator << not implemented");
    break ;
  }

  return (P_ostream) ;
}

bool operator< 
  (const T_ValueData & P_left, 
   const T_ValueData & P_rigth) {
  bool   L_ret = false;
  int    L_i ;
  size_t L_min ;
  bool   L_left_min = false ;
  

  if (P_left.m_type == P_rigth.m_type) {

    switch (P_left.m_type) {
      
    case E_TYPE_NUMBER:
      L_ret = P_left.m_value.m_val_number < P_rigth.m_value.m_val_number ;
      break ;
    case E_TYPE_SIGNED:
      L_ret = P_left.m_value.m_val_signed < P_rigth.m_value.m_val_signed ;
      break ;
    case E_TYPE_STRING: 
      
      L_i = 0 ;
      
      if (P_left.m_value.m_val_binary.m_size 
	  != P_rigth.m_value.m_val_binary.m_size) {
	
	if (P_left.m_value.m_val_binary.m_size 
	    < P_rigth.m_value.m_val_binary.m_size) {
	  L_min = P_left.m_value.m_val_binary.m_size  ;
	  L_left_min = true ;
	} else {
	  L_min = P_rigth.m_value.m_val_binary.m_size ;
	}
	
	while (P_left.m_value.m_val_binary.m_value[L_i] 
	       == P_rigth.m_value.m_val_binary.m_value[L_i]) {
	  L_i ++ ;
	  if (L_i == (int) L_min) break ;
	}
	if (L_i == (int) L_min) {
	  L_ret = L_left_min ;
	} else {
	  L_ret = P_left.m_value.m_val_binary.m_value[L_i] 
	    < P_rigth.m_value.m_val_binary.m_value[L_i];
	}
	
      } else {
	
	while (P_left.m_value.m_val_binary.m_value[L_i] 
	       == P_rigth.m_value.m_val_binary.m_value[L_i]) {
	  L_i ++ ;
	  if (L_i == (int)P_left.m_value.m_val_binary.m_size) {
	    break ;
	  }
	}
	if (L_i == (int)P_left.m_value.m_val_binary.m_size) {
	  L_ret = false ;
	} else {
	  L_ret = P_left.m_value.m_val_binary.m_value[L_i] 
	    < P_rigth.m_value.m_val_binary.m_value[L_i];
	}
      }
      break ;
      
    case E_TYPE_STRUCT:
      L_ret = P_left.m_value.m_val_struct.m_id_1 < P_rigth.m_value.m_val_struct.m_id_1 ;
      if (!L_ret) { L_ret = P_left.m_value.m_val_struct.m_id_2 < P_rigth.m_value.m_val_struct.m_id_2 ; }
      break ;
      
    case E_TYPE_NUMBER_64:
      L_ret = P_left.m_value.m_val_number_64 < P_rigth.m_value.m_val_number_64 ;
      break ;
    case E_TYPE_SIGNED_64:
      L_ret = P_left.m_value.m_val_signed_64 < P_rigth.m_value.m_val_signed_64 ;
      break ;

    default:
      GEN_FATAL(E_GEN_FATAL_ERROR, "operator < not implemented");
      break ;
    }
    
  } else {

    L_ret = (P_left.m_type < P_rigth.m_type) ;

  }


  return (L_ret);
}


bool operator==
  (const T_ValueData & P_left, 
   const T_ValueData & P_rigth) {
  bool   L_ret = false;
  int    L_i ;
  
  if (P_left.m_type == P_rigth.m_type) {

    switch (P_left.m_type) {
      
    case E_TYPE_NUMBER:
      L_ret = P_left.m_value.m_val_number == P_rigth.m_value.m_val_number ;
      break ;
    case E_TYPE_SIGNED:
      L_ret = P_left.m_value.m_val_signed == P_rigth.m_value.m_val_signed ;
      break ;
    case E_TYPE_STRING: 
      
      L_i = 0 ;
      
      if (P_left.m_value.m_val_binary.m_size 
	  != P_rigth.m_value.m_val_binary.m_size) {
        L_ret = false;
      } else {
	
	while (P_left.m_value.m_val_binary.m_value[L_i] 
	       == P_rigth.m_value.m_val_binary.m_value[L_i]) {
	  L_i ++ ;
	  if (L_i == (int)P_left.m_value.m_val_binary.m_size) {
	    break ;
	  }
	}
	if (L_i == (int)P_left.m_value.m_val_binary.m_size) {
	  L_ret = true ;
	} else {
	  L_ret = false ;
	}
      }
      break ;
      
    case E_TYPE_STRUCT:
      L_ret = P_left.m_value.m_val_struct.m_id_1 == P_rigth.m_value.m_val_struct.m_id_1 ;
      if (!L_ret) { L_ret = P_left.m_value.m_val_struct.m_id_2 == P_rigth.m_value.m_val_struct.m_id_2 ; }
      break ;
      
    case E_TYPE_NUMBER_64:
      L_ret = P_left.m_value.m_val_number_64 == P_rigth.m_value.m_val_number_64 ;
      break ;
    case E_TYPE_SIGNED_64:
      L_ret = P_left.m_value.m_val_signed_64 == P_rigth.m_value.m_val_signed_64 ;
      break ;

    default:
      GEN_FATAL(E_GEN_FATAL_ERROR, "operator == not implemented");
      break ;
    }
    
  } else {

    L_ret = (P_left.m_type == P_rigth.m_type) ;

  }


  return (L_ret);
}



T_TypeType typeFromString (char *P_type) {
  T_TypeType L_type = E_UNSUPPORTED_TYPE ;
  int        L_i ;

  for (L_i=0; L_i < (int)E_UNSUPPORTED_TYPE; L_i++) {
    if (strcmp(P_type, type_type_table[L_i])==0) {
      L_type = (T_TypeType) L_i ;
      break ;
    }
  }
  
  return (L_type);
}

T_ValueData valueFromString (char *P_value, T_TypeType P_type, int &P_result) {
  
  T_ValueData  L_value         ;
  char        *L_field_value   ;
  size_t       L_size          ;


  GEN_DEBUG(1, "ProtocolData::valueFromString() start");
  

  L_value.m_type = P_type ;
  L_field_value = P_value ;

  if (L_field_value == NULL) { P_result = -1 ; return (L_value); }
  L_size = strlen(L_field_value); 
  P_result = 0 ;

  GEN_DEBUG(1, "ProtocolData::valueFromString() "
	    << "L_field_value [" << L_field_value << "]");
  
  GEN_DEBUG(1, "ProtocolData::valueFromString() "
	    << " L_value.m_type  [" <<  L_value.m_type  << "]");

  switch (L_value.m_type) {
  
  case E_TYPE_SIGNED: {
    
    T_Integer32 L_signed_value ;
    char *L_endstr ;
    if (L_size > 0) {
      if ((L_size>2) 
	  && (L_field_value[0] == '0') 
	  && (L_field_value[1] == 'x')) { // hexa buffer value
	L_signed_value = strtol_f (L_field_value, &L_endstr,16) ; 
      } else {
	L_signed_value = strtol_f (L_field_value, &L_endstr,10) ;
      }
      
      if (L_endstr[0] != '\0') {
	GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		  << L_field_value << "] bad format");
	P_result = -1 ;
      } else {
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "signed value = " << L_signed_value);
	L_value.m_value.m_val_signed = L_signed_value ;
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Field empty where a signed number is expected");
      P_result = -1 ;
    }
  } 
  break ;

  case E_TYPE_NUMBER: {
    
    T_UnsignedInteger32 L_unsigned_value ;
    char *L_endstr ;
    if (L_size > 0) {
      if ((L_size>2) 
	  && (L_field_value[0] == '0') 
	  && (L_field_value[1] == 'x')) { // hexa buffer value
	L_unsigned_value = strtoul_f (L_field_value, &L_endstr,16) ; 
      } else {
	L_unsigned_value = strtoul_f (L_field_value, &L_endstr,10) ;
      }
      
      if (L_endstr[0] != '\0') {
	GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		  << L_field_value << "] bad format");
	P_result = -1  ;
      } else {
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "unsigned value =" << L_unsigned_value);
	L_value.m_value.m_val_number = L_unsigned_value ;
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Field empty where a number is expected");
      P_result = -1 ; 
    }
  }
  break ;
  
  case E_TYPE_STRING: {
    if ((L_size>2) 
	&& (L_field_value[0] == '0') 
	&& (L_field_value[1] == 'x')) { // hexa buffer value
      
      char  *L_ptr = L_field_value+2 ;
      size_t L_res_size ;
      
      L_value.m_value.m_val_binary.m_value  
	= convert_hexa_char_to_bin(L_ptr, &L_res_size);

      if (L_value.m_value.m_val_binary.m_value == NULL ) {
	GEN_ERROR(E_GEN_FATAL_ERROR, 
		  "Bad buffer size for hexadecimal buffer ["
		  << L_field_value << "]" ); 
	P_result = -1 ;
      } else {
	L_value.m_value.m_val_binary.m_size = L_res_size ;	  
      }
      
    } else { // direct string value
      
      L_value.m_value.m_val_binary.m_size = L_size ;
      
      if (L_size != 0) {
	ALLOC_TABLE(L_value.m_value.m_val_binary.m_value,
		    unsigned char*,
		    sizeof(unsigned char),
		    L_size);
	memcpy(L_value.m_value.m_val_binary.m_value,
	       L_field_value,
	       L_size);
      } else {
	L_value.m_value.m_val_binary.m_value = NULL ;
      }
    }
    
    GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
	      << "string value size=" 
	      << L_value.m_value.m_val_binary.m_size) ;
  }
  break ;

  case E_TYPE_SIGNED_64: {
    
    T_Integer64  L_signed_value ;
    char *L_endstr ;
    if (L_size > 0) {
      if ((L_size>2) 
	  && (L_field_value[0] == '0') 
	  && (L_field_value[1] == 'x')) { // hexa buffer value
	L_signed_value = strtoll_f (L_field_value, &L_endstr,16) ; 
      } else {
	L_signed_value = strtoll_f (L_field_value, &L_endstr,10) ;
      }
      
      if (L_endstr[0] != '\0') {
	GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		  << L_field_value << "] bad format");
	P_result = -1 ;
      } else {
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "signed64 value = " << L_signed_value);
	L_value.m_value.m_val_signed_64 = L_signed_value ;
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Field empty where a signed number64 is expected");
      P_result = -1 ;
    }
  } 
  break ;

  case E_TYPE_NUMBER_64: {
    
    T_UnsignedInteger64 L_unsigned_value ;
    char *L_endstr ;
    if (L_size > 0) {
      if ((L_size>2) 
	  && (L_field_value[0] == '0') 
	  && (L_field_value[1] == 'x')) { // hexa buffer value
	L_unsigned_value = strtoull_f (L_field_value, &L_endstr,16) ; 
      } else {
	L_unsigned_value = strtoull_f (L_field_value, &L_endstr,10) ;
      }
      
      if (L_endstr[0] != '\0') {
	GEN_ERROR(E_GEN_FATAL_ERROR, "field value ["
		  << L_field_value << "] bad format");
	P_result = -1  ;
      } else {
	GEN_DEBUG(1, "C_ExternalDataControl::analyze_data() "
		  << "unsigned64 value =" << L_unsigned_value);
	L_value.m_value.m_val_number_64 = L_unsigned_value ;
      }
    } else {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Field empty where a number64 is expected");
      P_result = -1 ; 
    }
  }
  break ;
  
  default:
    GEN_FATAL(E_GEN_FATAL_ERROR,
	      "Unsupported type for external data");
    P_result = -1 ;
    break ;
  }

  GEN_DEBUG(1, "ProtocolData::valueFromString() end");

  return (L_value);
}

const char* defaultStringValue(T_TypeType P_type) {
  if (P_type <= E_TYPE_STRING) {
    return(default_value_table[(int)P_type]);
  } else {
    return (NULL);
  }
}

void copyValue(T_ValueData& P_dest, 
	       T_ValueData& P_source, 
	       bool P_reset) {
  if (P_reset) {
    if (P_dest.m_type == E_TYPE_STRING) {
      FREE_TABLE(P_dest.m_value.m_val_binary.m_value);
      P_dest.m_value.m_val_binary.m_size = 0 ;
    }
  }
  P_dest = P_source ;
  if (P_dest.m_type == E_TYPE_STRING) {
    ALLOC_TABLE(P_dest.m_value.m_val_binary.m_value,
		unsigned char*,
		sizeof(unsigned char),
		P_dest.m_value.m_val_binary.m_size);
    memcpy(P_dest.m_value.m_val_binary.m_value,
	   P_source.m_value.m_val_binary.m_value,
	   P_dest.m_value.m_val_binary.m_size);
  }
}

void resetMemory(T_ValueData &P_value) {
  if (P_value.m_type == E_TYPE_STRING) {
    FREE_TABLE(P_value.m_value.m_val_binary.m_value);
    P_value.m_value.m_val_binary.m_size = 0 ;
    //P_value.m_type = E_TYPE_NUMBER ;
  }
}


void resetValue(T_Value& P_value) {
    FREE_TABLE(P_value.m_val_binary.m_value);
    P_value.m_val_binary.m_size = 0 ;
}


bool copyBinaryVal(T_Value& P_dest, int P_begin, int P_size,
		   T_Value& P_source){

  int                    L_size ;
  unsigned char         *L_new_value ;

  L_size = P_dest.m_val_binary.m_size - P_begin - P_size ;

  if (L_size < 0) {
    P_dest.m_val_binary.m_size -= L_size ;
    ALLOC_TABLE(L_new_value,
		unsigned char*,
		sizeof(unsigned char),
		P_dest.m_val_binary.m_size);
    memset(L_new_value, 0, P_dest.m_val_binary.m_size);
    //    memcpy(L_new_value, P_dest.m_val_binary.m_value,
    //	   P_dest.m_val_binary.m_size+L_size);
    if (P_dest.m_val_binary.m_value) FREE_TABLE(P_dest.m_val_binary.m_value);
    P_dest.m_val_binary.m_value = L_new_value ;

  }

  memcpy(P_dest.m_val_binary.m_value+P_begin,
	 P_source.m_val_binary.m_value,
	 P_size);

  return (true);
}


bool copyBinaryVal(T_ValueData& P_dest, int P_begin, int P_size,
		   T_ValueData& P_source){

  int                    L_size ;
  unsigned char         *L_new_value ;

  L_size = P_dest.m_value.m_val_binary.m_size - P_begin - P_size ;

  if (L_size < 0) {
    P_dest.m_value.m_val_binary.m_size -= L_size ;
    ALLOC_TABLE(L_new_value,
		unsigned char*,
		sizeof(unsigned char),
		P_dest.m_value.m_val_binary.m_size);
    memset(L_new_value, 0, P_dest.m_value.m_val_binary.m_size);
    memcpy(L_new_value, P_dest.m_value.m_val_binary.m_value,
	   P_dest.m_value.m_val_binary.m_size+L_size);
    FREE_TABLE(P_dest.m_value.m_val_binary.m_value);
    P_dest.m_value.m_val_binary.m_value = L_new_value ;

  }

  memcpy(P_dest.m_value.m_val_binary.m_value+P_begin,
	 P_source.m_value.m_val_binary.m_value,
	 P_size);

  return (true);
}


bool extractBinaryVal(T_ValueData& P_dest, int P_begin, int P_size,
		      T_ValueData& P_source){

  unsigned char         *L_ptr  ;

  // TO DO ctrl P_source NULL
  L_ptr =  P_source.m_value.m_val_binary.m_value;
  
  L_ptr += P_begin ;
  
  memcpy(P_dest.m_value.m_val_binary.m_value, 
	 L_ptr, P_size);

  return (true);
}

bool extractBinaryVal(T_Value& P_dest, int P_begin, int P_size,
		      T_Value& P_source){

  unsigned char         *L_ptr  ;

  // TO DO ctrl P_source NULL
  L_ptr =  P_source.m_val_binary.m_value;
  
  L_ptr += P_begin ;
  
  memcpy(P_dest.m_val_binary.m_value, 
	 L_ptr, P_size);

  return (true);
}

bool compare_value(T_Value& P_left, 
                   T_Value& P_rigth) {
  bool   L_ret = false;
  int    L_i ;
  L_i = 0 ;
  
  if (P_left.m_val_binary.m_size 
      != P_rigth.m_val_binary.m_size) {
    L_ret = false;
  } else {
    while (P_left.m_val_binary.m_value[L_i] 
           == P_rigth.m_val_binary.m_value[L_i]) {
      L_i ++ ;
      if (L_i == (int)P_left.m_val_binary.m_size) {
        break ;
      }
    }
    if (L_i == (int)P_left.m_val_binary.m_size) {
      L_ret = true ;
    } else {
      L_ret = false ;
    }
  }
  return (L_ret);
}

