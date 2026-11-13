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
  char   L_hexa_buf [50] ;
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
  char   L_hexa_buf [50] ;
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
  char   L_hexa_buf [50] ;
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
  char   L_hexa_buf [50] ;
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


// Version con limite: escribe "xx" por byte (minusculas, sin separador) y
// siempre termina en 0. Si no cabe, trunca a bytes completos, sin marca, para
// que la salida siga siendo valida para convert_hexa_char_to_bin.
// Devuelve el numero de bytes de P_buf convertidos.
size_t convert_bin_to_hexa_char_n (const unsigned char* P_buf, size_t P_size,
                                   char* P_res, size_t P_res_size) {
  static constexpr char L_hex[] = "0123456789abcdef" ;
  if ((P_res == nullptr) || (P_res_size == 0)) { return (0) ; }
  if (P_buf == nullptr) { P_size = 0 ; }

  size_t L_max = (P_res_size - 1) / 2 ;
  size_t L_n   = (P_size < L_max) ? P_size : L_max ;
  char*  L_out = P_res ;
  for (size_t L_i = 0 ; L_i < L_n ; L_i++) {
    *L_out++ = L_hex[P_buf[L_i] >> 4] ;
    *L_out++ = L_hex[P_buf[L_i] & 0x0f] ;
  }
  *L_out = 0 ;
  return (L_n) ;
}

// API original (HP, 2006), sin callers en el arbol. Se conserva por
// compatibilidad: seagull se enlaza con ENABLE_EXPORTS y bibliotecas externas
// cargadas con dlopen pueden usarla. Contrato: P_res de 2*P_size+1 bytes.
void convert_bin_to_hexa_char(unsigned char*P_buf, size_t P_size, char* P_res) {
  if (P_res == nullptr) { return ; }
  (void)convert_bin_to_hexa_char_n(P_buf, P_size, P_res, (2 * P_size) + 1) ;
}


// Valor de un digito hexadecimal, o -1 si P_c no lo es.
static inline int hex_nibble (unsigned char P_c) {
  if ((P_c >= '0') && (P_c <= '9')) { return (P_c - '0') ; }
  if ((P_c >= 'a') && (P_c <= 'f')) { return (P_c - 'a' + 10) ; }
  if ((P_c >= 'A') && (P_c <= 'F')) { return (P_c - 'A' + 10) ; }
  return (-1) ;
}

// Convierte una cadena hexadecimal ("00017f000001", sin prefijo 0x) en bytes.
// Estricta: longitud par y solo [0-9a-fA-F] (sin blancos, signos ni basura).
// Error: devuelve NULL con *P_bufSize = 0, igual que antes con longitud impar,
// asi que los callers que ya comprueban NULL no cambian. Valida antes de
// reservar memoria. El buffer se reserva con ALLOC_TABLE (malloc): liberar con
// FREE_TABLE. Cadena vacia: tamano 0 y buffer reservado, como antes.
unsigned char *convert_hexa_char_to_bin(char* P_data, size_t *P_bufSize) {

  unsigned char *L_buf = NULL ;

  *P_bufSize = (size_t) 0 ;
  if (P_data == NULL) { return (NULL) ; }

  size_t L_len = strlen(P_data) ;
  if ((L_len % 2) != 0) { return (NULL) ; }
  for (size_t L_i = 0 ; L_i < L_len ; L_i++) {
    if (hex_nibble((unsigned char)P_data[L_i]) < 0) { return (NULL) ; }
  }

  *P_bufSize = L_len / 2 ;
  ALLOC_TABLE(L_buf, unsigned char*,
              sizeof(unsigned char), (*P_bufSize));
  for (size_t L_i = 0 ; L_i < *P_bufSize ; L_i++) {
    L_buf[L_i] = (unsigned char)((hex_nibble((unsigned char)P_data[2 * L_i]) << 4)
                                 | hex_nibble((unsigned char)P_data[(2 * L_i) + 1])) ;
  }

  return (L_buf) ;
}

static constexpr char C_HEX_DIGITS[] = "0123456789abcdef" ;

static inline char printable_char (unsigned char P_c) {
  return (((P_c >= 32) && (P_c <= 126)) ? static_cast<char>(P_c) : '.') ;
}

// Escribe "xx " por byte y siempre termina en 0. Si no cabe, trunca y
// marca con "...". Devuelve el numero de bytes de P_buf representados.
size_t pretty_binary_buffer_n (const unsigned char* P_buf, size_t P_size,
                               char* P_res, size_t P_res_size) {
  if ((P_res == nullptr) || (P_res_size == 0)) { return (0) ; }
  if (P_buf == nullptr) { P_size = 0 ; }

  size_t L_n     = P_size ;
  bool   L_trunc = (P_size > ((P_res_size - 1) / 3)) ;
  if (L_trunc) { L_n = (P_res_size >= 4) ? ((P_res_size - 4) / 3) : 0 ; }

  char*  L_out = P_res ;
  for (size_t L_i = 0 ; L_i < L_n ; L_i++) {
    *L_out++ = C_HEX_DIGITS[P_buf[L_i] >> 4] ;
    *L_out++ = C_HEX_DIGITS[P_buf[L_i] & 0x0f] ;
    *L_out++ = ' ' ;
  }
  if (L_trunc && (P_res_size >= 4)) { memcpy(L_out, "...", 3) ; L_out += 3 ; }
  *L_out = 0 ;
  return (L_n) ;
}

size_t pretty_ascii_printable_buffer_n (const unsigned char* P_buf, size_t P_size,
                                        char* P_print, size_t P_print_size) {
  if ((P_print == nullptr) || (P_print_size == 0)) { return (0) ; }
  if (P_buf == nullptr) { P_size = 0 ; }

  size_t L_n = (P_size < P_print_size) ? P_size : (P_print_size - 1) ;
  for (size_t L_i = 0 ; L_i < L_n ; L_i++) {
    P_print[L_i] = printable_char(P_buf[L_i]) ;
  }
  P_print[L_n] = 0 ;
  return (L_n) ;
}

size_t pretty_binary_printable_buffer_n (const unsigned char* P_buf, size_t P_size,
                                         char* P_res, size_t P_res_size,
                                         char* P_print, size_t P_print_size) {
  size_t L_h = pretty_binary_buffer_n(P_buf, P_size, P_res, P_res_size) ;
  size_t L_a = pretty_ascii_printable_buffer_n(P_buf, P_size, P_print, P_print_size) ;
  return ((L_h < L_a) ? L_h : L_a) ;
}


T_UnsignedInteger64 convert_bin_network_to_ull(unsigned char*P_buf, 
                                               size_t        P_size) {

  T_UnsignedInteger64 L_ret = 0 ;
  unsigned char      *L_ptr     ;

  GEN_DEBUG(1, "convert_bin_network_to_ull ("
	<< &P_buf << ", size: " << P_size
	<< ")");

#ifdef DEBUG_MODE
  char   L_hexa_buf [50] ;
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
  char   L_hexa_buf [50] ;
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
  char   L_hexa_buf [50] ;
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
  char   L_hexa_buf [50] ;
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

