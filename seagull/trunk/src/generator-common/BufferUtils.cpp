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

#include "BufferUtils.hpp"
#include "Utils.hpp"
#include <cstring>

#include "GeneratorTrace.hpp"

T_UnsignedInteger32 convert_char_to_ul(char *P_data) {
  char                *L_tmp ;
  T_UnsignedInteger32  L_result ;
  L_result = strtoul_f(P_data, &L_tmp, 10);
  return (L_result);
}

void convert_ul_to_bin_network(unsigned char      *P_buf, 
			       size_t              P_size, 
			       T_UnsignedInteger32 P_val) {
  T_UnsignedInteger32 L_val = P_val ;
  unsigned char      *L_ul = (unsigned char*) &L_val ;
  unsigned char      *L_res = P_buf ;

  GEN_DEBUG(1, "convert_ul_to_bin_network ("
	<< &P_buf << ", size: " << P_size << ", Val: 0x" 
        << iostream_hex << P_val << iostream_dec
	<< ")");
  
#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = (unsigned char*)&P_val;
  pretty_binary_buffer(L_cur, sizeof(T_UnsignedInteger32), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: "
               << (sizeof(T_UnsignedInteger32) - P_size));

  L_val = htonl_f (L_val) ;

#ifdef DEBUG_MODE
  L_cur = (unsigned char*)&L_val;
  pretty_binary_buffer(L_cur, sizeof(T_UnsignedInteger32), L_hexa_buf);
  GEN_DEBUG(1, "htonl   : " << L_hexa_buf);
#endif

  L_ul  += (sizeof(T_UnsignedInteger32) - P_size);

  memcpy ( (void *) L_res,
	   (void *) L_ul,
	   P_size) ;

#ifdef DEBUG_MODE
  L_cur = L_res;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "convert_ul_to_bin_network() end");
}

void convert_l_to_bin_network(unsigned char   *P_buf, 
			      size_t           P_size, 
			      T_Integer32      P_val) {
  T_Integer32         L_val = P_val ;
  unsigned char      *L_ul = (unsigned char*) &L_val ;
  unsigned char      *L_res = P_buf ;

  GEN_DEBUG(1, "convert_l_to_bin_network ("
	<< &P_buf << ", size: " << P_size << ", Val: 0x" 
        << iostream_hex << P_val << iostream_dec
	<< ")");
  
#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = (unsigned char*)&P_val;
  pretty_binary_buffer(L_cur, sizeof(T_Integer32), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: "
               << (sizeof(T_UnsignedInteger32) - P_size));

  L_val = htonl_f (L_val) ;

#ifdef DEBUG_MODE
  L_cur = (unsigned char*)&L_val;
  pretty_binary_buffer(L_cur, sizeof(T_Integer32), L_hexa_buf);
  GEN_DEBUG(1, "htonl   : " << L_hexa_buf);
#endif

  L_ul  += (sizeof(T_UnsignedInteger32) - P_size);

  memcpy ( (void *) L_res,
	   (void *) L_ul,
	   P_size) ;

#ifdef DEBUG_MODE
  L_cur = L_res;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "convert_l_to_bin_network() end");
}

T_UnsignedInteger32 convert_bin_network_to_ul(unsigned char*P_buf, size_t P_size) {

  T_UnsignedInteger32 L_ret = 0 ;
  unsigned char      *L_ptr     ;

  GEN_DEBUG(1, "convert_bin_network_to_ul ("
	<< &P_buf << ", size: " << P_size
	<< ")");

#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = P_buf;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: "
               << (sizeof(T_UnsignedInteger32) - P_size));

  if (P_size > sizeof(T_UnsignedInteger32)) {
     GEN_ERROR(E_GEN_FATAL_ERROR,"Current Size: "
               << P_size << " is greater than expecting one: "
               <<sizeof(T_UnsignedInteger32));
  } else {
    // from network => MSB representation
    L_ptr = (unsigned char*) &L_ret ;
    L_ptr += (sizeof(T_UnsignedInteger32) - P_size) ;
    memcpy( (void*)  L_ptr,
	    (void*)  P_buf,
	    (size_t) P_size ) ;
    // convert to local representation
    L_ret = ntohl_f (L_ret) ;
  }

#ifdef DEBUG_MODE
  L_cur = (unsigned char*)&L_ret;
  pretty_binary_buffer(L_cur, sizeof(T_UnsignedInteger32), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  return (L_ret) ;
}

T_Integer32 convert_bin_network_to_l(unsigned char*P_buf, size_t P_size) {

  T_Integer32      L_ret = 0 ;
  unsigned char   *L_ptr     ;
#if LONG_MAX==9223372036854775807L
  bool             L_neg_value = false ;
#endif
  unsigned int     L_decal = sizeof(T_Integer32) - P_size;

  GEN_DEBUG(1, "convert_bin_network_to_l ("
	<< &P_buf << ", size: " << P_size
	<< ")");

#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = P_buf;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: " << L_decal);

  if (P_size > sizeof(T_Integer32)) {
     GEN_ERROR(E_GEN_FATAL_ERROR,"Current Size: "
               << P_size << " is greater than expecting one: "
               <<sizeof(T_Integer32));
  } else {
#if LONG_MAX==9223372036854775807L
    // check if value < 0
    if (P_buf[0] & 128) { 
      L_neg_value = true ;
    }
#endif
    
    // from network => MSB representation
    L_ptr = (unsigned char*) &L_ret ;
    L_ptr += L_decal;
    memcpy( (void*)  L_ptr,
	    (void*)  P_buf,
	    (size_t) P_size ) ;
    // convert to local representation
    L_ret = ntohl_f (L_ret) ;

#if LONG_MAX==9223372036854775807L
    if (L_neg_value == true) { 
      if (L_decal > 0) { 
        memset(L_ptr, 0xff, L_decal);
      }
    }
#endif
  }

#ifdef DEBUG_MODE
  L_cur = (unsigned char*)&L_ret;
  pretty_binary_buffer(L_cur, sizeof(T_Integer32), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  return (L_ret) ;
}


void convert_bin_to_hexa_char(unsigned char*P_buf, size_t P_size, char* P_res) {
  
  unsigned long  L_i ;
  char          *L_res = P_res ;

  for(L_i = 0; L_i < P_size; L_i++) {
    (void)sprintf(L_res,"%02x", (int)P_buf[L_i]);
    L_res += 2 ;
  }

}


unsigned char *convert_hexa_char_to_bin(char* P_data, size_t *P_bufSize) {

  char           L_value[3]        ;
  unsigned char *L_buf      = NULL ;
  int            L_i, L_k          ;
  unsigned int   L_binValue        ;
  
  *P_bufSize = (size_t) 0 ;

  if ((strlen(P_data) % 2) == 0) {
    *P_bufSize = (size_t) strlen(P_data) / 2 ;
    ALLOC_TABLE(L_buf, unsigned char*, 
                sizeof(unsigned char), (*P_bufSize));
    L_k = 0 ;
    L_value[2] = '\0' ;
    for (L_i=0 ; L_i < (int)(*P_bufSize); L_i++) {
      L_value[0]=P_data[L_k] ;
      L_value[1]=P_data[L_k+1] ;
      sscanf(L_value, "%x", &L_binValue);
      L_buf[L_i] = (unsigned char) L_binValue ;
      L_k += 2 ;
    }
    
  } 

  return (L_buf) ;
}

void pretty_binary_buffer (unsigned char*P_buf, size_t P_size, char* P_res) {
  
  unsigned long  L_i ;
  char          *L_res = P_res ;

  for(L_i = 0; L_i < P_size; L_i++) {
    (void)sprintf(L_res,"%02x ", (int)P_buf[L_i]);
    L_res += 3 ;
  }

}

void pretty_binary_printable_buffer (unsigned char*P_buf, size_t P_size, char* P_res, char *P_print) {
  
  unsigned long  L_i ;
  char          *L_res = P_res ;
  char          *L_print = P_print ;

  for(L_i = 0; L_i < P_size; L_i++) {
    (void)sprintf(L_res,"%02x ", (int)P_buf[L_i]);
    (void)sprintf(L_print,"%c", ((((int)P_buf[L_i] >= 32) && ((int)P_buf[L_i] <= 126)) ? P_buf[L_i] : '.'));
    L_res += 3 ;
    L_print += 1 ;
  }

}
T_UnsignedInteger64 convert_bin_network_to_ull(unsigned char*P_buf, 
                                               size_t        P_size) {

  T_UnsignedInteger64 L_ret = 0 ;
  unsigned char      *L_ptr     ;

  GEN_DEBUG(1, "convert_bin_network_to_ull ("
	<< &P_buf << ", size: " << P_size
	<< ")");

#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = P_buf;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: "
               << (sizeof(T_UnsignedInteger64) - P_size));

  if (P_size > sizeof(T_UnsignedInteger64)) {
     GEN_ERROR(E_GEN_FATAL_ERROR,"Current Size: "
               << P_size << " is greater than expecting one: "
               <<sizeof(T_UnsignedInteger64));
  } else {
    // from network => MSB representation
    L_ptr = (unsigned char*) &L_ret ;
    L_ptr += (sizeof(T_UnsignedInteger64) - P_size) ;
    memcpy( (void*)  L_ptr,
	    (void*)  P_buf,
	    (size_t) P_size ) ;
    // convert to local representation
    L_ret = ntohll_f (L_ret) ;
  }

#ifdef DEBUG_MODE
  L_cur = (unsigned char*) &L_ret;
  pretty_binary_buffer(L_cur, sizeof(T_UnsignedInteger64), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  return (L_ret) ;
}

T_Integer64 convert_bin_network_to_ll(unsigned char*P_buf, size_t P_size) {

  T_Integer64      L_ret = 0 ;
  unsigned char   *L_ptr     ;

  GEN_DEBUG(1, "convert_bin_network_to_ll ("
	<< &P_buf << ", size: " << P_size
	<< ")");
  
#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = P_buf;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: "
               << (sizeof(T_UnsignedInteger64) - P_size));

  if (P_size > sizeof(T_UnsignedInteger64)) {
     GEN_ERROR(E_GEN_FATAL_ERROR,"Current Size: "
               << P_size << " is greater than expecting one: "
               <<sizeof(T_UnsignedInteger64));
  } else {
    // from network => MSB representation
    L_ptr = (unsigned char*) &L_ret ;
    L_ptr += (sizeof(T_UnsignedInteger64) - P_size) ;
    memcpy( (void*)  L_ptr,
	    (void*)  P_buf,
	    (size_t) P_size ) ;
    // convert to local representation
    L_ret = ntohll_f (L_ret) ;
  }

#ifdef DEBUG_MODE
  L_cur = (unsigned char *)&L_ret;
  pretty_binary_buffer(L_cur, sizeof(T_Integer64), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  return (L_ret) ;
}

void convert_ull_to_bin_network(unsigned char      *P_buf, 
			        size_t              P_size, 
			        T_UnsignedInteger64 P_val) {
  T_UnsignedInteger64 L_val = P_val ;
  unsigned char      *L_ul = (unsigned char*) &L_val ;
  unsigned char      *L_res = P_buf ;

  GEN_DEBUG(1, "convert_ull_to_bin_network ("
	<< &P_buf << ", size: " << P_size << ", Val: 0x" 
        << iostream_hex << P_val << iostream_dec
	<< ")");
  
#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = (unsigned char *)&P_val;
  pretty_binary_buffer(L_cur, sizeof(T_UnsignedInteger64), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: "
               << (sizeof(T_UnsignedInteger64) - P_size));

  L_val = htonll_f (L_val) ;

#ifdef DEBUG_MODE
  L_cur = (unsigned char *)&L_val;
  pretty_binary_buffer(L_cur, sizeof(T_UnsignedInteger64), L_hexa_buf);
  GEN_DEBUG(1, "htonll  : " << L_hexa_buf);
#endif

  L_ul  += (sizeof(T_UnsignedInteger64) - P_size);

  memcpy ( (void *) L_res,
	   (void *) L_ul,
	   P_size) ;

#ifdef DEBUG_MODE
  L_cur = L_res;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "convert_ull_to_bin_network() end");
}

void convert_ll_to_bin_network(unsigned char   *P_buf, 
			       size_t           P_size, 
			       T_Integer64      P_val) {
  T_Integer64         L_val = P_val ;
  unsigned char      *L_ul = (unsigned char*) &L_val ;
  unsigned char      *L_res = P_buf ;

  GEN_DEBUG(1, "convert_ll_to_bin_network ("
	<< &P_buf << ", size: " << P_size << ", Val: 0x" 
        << iostream_hex << P_val << iostream_dec
	<< ")");

#ifdef DEBUG_MODE
  static char   L_hexa_buf [50] ;
  unsigned char*L_cur ;

  L_cur = (unsigned char*)&P_val;
  pretty_binary_buffer(L_cur, sizeof(T_Integer64), L_hexa_buf);
  GEN_DEBUG(1, "host val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "decal size: "
               << (sizeof(T_UnsignedInteger64) - P_size));

  L_val = htonll_f (L_val) ;

#ifdef DEBUG_MODE
  L_cur = (unsigned char *)&L_val;
  pretty_binary_buffer(L_cur, sizeof(T_Integer64), L_hexa_buf);
  GEN_DEBUG(1, "htonll  : " << L_hexa_buf);
#endif

  L_ul  += (sizeof(T_UnsignedInteger64) - P_size);

  memcpy ( (void *) L_res,
	   (void *) L_ul,
	   P_size) ;

#ifdef DEBUG_MODE
  L_cur = L_res;
  pretty_binary_buffer(L_cur, P_size, L_hexa_buf);
  GEN_DEBUG(1, "net val: " << L_hexa_buf);
#endif

  GEN_DEBUG(1, "convert_ll_to_bin_network() end");
}

