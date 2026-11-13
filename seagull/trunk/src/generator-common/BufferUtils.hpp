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

#ifndef _BUFFER_UTILS_H
#define _BUFFER_UTILS_H

#include <cstdlib>

#include "integer_t.hpp" // for strtoll 

size_t convert_bin_to_hexa_char_n (const unsigned char* P_buf, size_t P_size,
                                   char* P_res, size_t P_res_size) ;
// API original: P_res debe tener 2*P_size+1 bytes. Se mantiene por compatibilidad.
[[deprecated("usar convert_bin_to_hexa_char_n")]]
void convert_bin_to_hexa_char(unsigned char*P_buf, size_t P_size, char* P_res);

unsigned char *convert_hexa_char_to_bin(char* P_data, size_t *P_bufSize);

T_UnsignedInteger32 convert_bin_network_to_ul(unsigned char*P_buf, size_t P_size) ;

void convert_ul_to_bin_network(unsigned char      *P_buf, 
			       size_t              P_size, 
			       T_UnsignedInteger32 P_val) ;

void convert_l_to_bin_network(unsigned char   *P_buf, 
			      size_t           P_size, 
			      T_Integer32      P_val) ;

T_Integer32 convert_bin_network_to_l(unsigned char*P_buf, 
			             size_t P_size);

T_UnsignedInteger32 convert_char_to_ul (char * P_data) ;


// Versiones con limite: escriben como maximo P_*_size bytes (incluido el 0 final).
size_t pretty_binary_buffer_n (const unsigned char* P_buf, size_t P_size,
                               char* P_res, size_t P_res_size) ;
size_t pretty_binary_printable_buffer_n (const unsigned char* P_buf, size_t P_size,
                                         char* P_res, size_t P_res_size,
                                         char* P_print, size_t P_print_size) ;
size_t pretty_ascii_printable_buffer_n (const unsigned char* P_buf, size_t P_size,
                                        char* P_print, size_t P_print_size) ;

// Envoltorios que deducen el tamano del array destino. No existe version con
// char* suelto: pasar un puntero sin tamano no compila.
template <size_t N>
inline void pretty_binary_buffer (const unsigned char* P_buf, size_t P_size,
                                  char (&P_res)[N]) {
  (void)pretty_binary_buffer_n(P_buf, P_size, P_res, N) ;
}
template <size_t N, size_t M>
inline void pretty_binary_printable_buffer (const unsigned char* P_buf, size_t P_size,
                                            char (&P_res)[N], char (&P_print)[M]) {
  (void)pretty_binary_printable_buffer_n(P_buf, P_size, P_res, N, P_print, M) ;
}
template <size_t M>
inline void pretty_ascii_printable_buffer (const unsigned char* P_buf, size_t P_size,
                                           char (&P_print)[M]) {
  (void)pretty_ascii_printable_buffer_n(P_buf, P_size, P_print, M) ;
}

T_UnsignedInteger64 convert_bin_network_to_ull(unsigned char*P_buf, 
                                               size_t P_size);

T_Integer64 convert_bin_network_to_ll(unsigned char*P_buf, 
                                      size_t P_size);

void convert_ull_to_bin_network(unsigned char      *P_buf, 
			        size_t              P_size, 
			        T_UnsignedInteger64 P_val) ;

void convert_ll_to_bin_network(unsigned char   *P_buf, 
			       size_t           P_size, 
			       T_Integer64      P_val) ;

#endif
