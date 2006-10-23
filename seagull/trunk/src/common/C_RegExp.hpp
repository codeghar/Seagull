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

#ifndef _C_REGEXP_H_
#define _C_REGEXP_H_

#include "types_t.hpp"
#include <regex.h>


class C_RegExp {

public:

  C_RegExp(char* P_RegularExpression, 
           int *P_error_code,
           int   P_nb_match  = 1,  // number of match expected (needed if sub expressions) 
           int   P_sub_match = 0,  // index of the sub matched expression needed 
                                   // 0 = no sub expression
           int   P_line = -1       // apply the matching process to the line number P_line
                                   // -1 = apply to all the buffer P_buffer
           );
  ~C_RegExp();

  char* execute(char* P_buffer) ;
  int   execute(char* P_buffer, int *P_start, int *P_end);
  char* execute(char* P_buffer, int *P_size);


private:
  
  char*    m_regularExpression;
  regex_t  m_internalRegExp;
  int      m_nb_match ;
  int      m_sub_match ;
  int      m_line ;

  char* setSubString(char* P_source, int P_start, int P_stop);

}; 



#endif // _C_REGEXP_H_


