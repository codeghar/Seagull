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


void pretty_binary_buffer (unsigned char*P_buf, size_t P_size, char* P_res);

void pretty_binary_printable_buffer (unsigned char*P_buf, size_t P_size, char* P_res, char *P_print) ;

void pretty_ascii_printable_buffer (unsigned char*P_buf, size_t P_size, char *P_print) ;

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
