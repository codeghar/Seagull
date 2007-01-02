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


#include "TextUtils.hpp"
#include "Utils.hpp"
#include "string_t.hpp"
#include <regex.h>

char* find_text_value (char *P_buf, char *P_field) {

  char *L_value = NULL ;

  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[3] ;
  size_t     L_size = 0 ;

  string_t   L_string = "" ;
  
  L_string  = "([[:blank:]]*" ;
  L_string += P_field ;
  L_string += "[[:blank:]]*=[[:blank:]]*)([^[:blank:];]+)";

  L_status = regcomp (&L_reg_expr, 
		      L_string.c_str(),
		      REG_EXTENDED) ;

  if (L_status != 0) {
    regerror(L_status, &L_reg_expr, L_buffer, 100);
    regfree (&L_reg_expr) ;

  } else {
  
    L_status = regexec (&L_reg_expr, P_buf, 3, L_pmatch, 0) ;
    regfree (&L_reg_expr) ;

    if (L_status == 0) {
      
      L_size = L_pmatch[2].rm_eo - L_pmatch[2].rm_so ;
      
      ALLOC_TABLE(L_value, char*, sizeof(char), L_size+1);
      memcpy(L_value, &(P_buf[L_pmatch[2].rm_so]), L_size);
      L_value[L_size]='\0' ;
      
    } 
  }


  return (L_value);
}
