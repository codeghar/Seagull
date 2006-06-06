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

#include "C_RegExp.hpp"
#include "string_t.hpp"
#include "Utils.hpp"

#define MAX_MATCH 10


C_RegExp::C_RegExp(char *P_RegularExpression, 
                   int  *P_error_code,                        
                   int   P_nb_match,
                   int   P_sub_match,
                   int   P_line) {
  m_regularExpression = P_RegularExpression ;
  m_nb_match = P_nb_match ;
  m_sub_match = P_sub_match ;
  m_line = P_line ;
  (*P_error_code) = regcomp(&m_internalRegExp, P_RegularExpression, REG_EXTENDED | REG_NEWLINE);
}

char* C_RegExp::setSubString(char* P_source, int P_start, int P_stop)
{
  char   *L_target = NULL ;

  if(P_source != NULL) {

    //        std::cerr << "P_source " << P_source 
    //      << " P_start " << P_start
    //      << " P_stop " << P_stop << std::endl;

    if(P_stop > P_start) {
      ALLOC_TABLE(L_target, char*, sizeof(char), (P_stop-P_start+1));
      memcpy(L_target, &(P_source[P_start]), (P_stop-P_start));
      L_target[(P_stop-P_start)] = '\0';
    }
  }

  return (L_target);
}

char* C_RegExp::execute(char *P_buffer) {

  regmatch_t L_pmatch[MAX_MATCH]      ;
  int        L_error                  ;
  int        L_i                      ;
  char*      L_result      = NULL     ;
  char*      L_buffer      = P_buffer ;
  char*      L_line        = NULL     ;
  char*      L_ptr         = NULL     ;
  
  memset((void*)L_pmatch, 0, sizeof(regmatch_t)*MAX_MATCH);

  if (m_line > 0) {
    // explicit line of the buffer = m_line
    int   L_current_line = -1 ;

    L_ptr = L_buffer ;
    L_line = L_ptr;

    while (L_current_line != m_line) {
      L_ptr = strstr(L_ptr, "\n");
      if (L_ptr) { 
        L_current_line++; 
        if (L_current_line != m_line) {
          L_ptr++;
          if (*L_ptr) { L_line = L_ptr ;}
        } else {
          *L_ptr = 0 ;
          L_buffer = L_line ;
        }
      } else {
        L_ptr = NULL ;
        L_buffer = NULL ;
        break ;
      }
    }
  }
  

  if (L_buffer) {
    L_error = regexec(&m_internalRegExp, L_buffer, MAX_MATCH, L_pmatch, REG_EXTENDED | REG_NEWLINE);
    if (L_error == 0) {
      for(L_i=0; L_i < MAX_MATCH; L_i++) {
        if(L_pmatch[L_i].rm_eo == -1) break ;
        if (L_i == m_sub_match) {
          L_result = setSubString(L_buffer, 
                                  L_pmatch[L_i].rm_so, L_pmatch[L_i].rm_eo);
        }
      }
      if (L_i != m_nb_match) {
        FREE_TABLE(L_result);
      }
    }
  }

  if (L_ptr) { *L_ptr = '\n'; }

  return(L_result);
}


int C_RegExp::execute(char* P_buffer, int *P_start, int *P_end) {

  int        L_ret         = -1       ;
  regmatch_t L_pmatch[MAX_MATCH]      ;
  int        L_error                  ;
  int        L_i                      ;
  char*      L_buffer      = P_buffer ;
  char*      L_line        = NULL     ;
  char*      L_ptr         = NULL     ;
  
  memset((void*)L_pmatch, 0, sizeof(regmatch_t)*MAX_MATCH);

  if (m_line > 0) {
    // explicit line of the buffer = m_line
    int   L_current_line = -1 ;

    L_ptr = L_buffer ;
    L_line = L_ptr;

    while (L_current_line != m_line) {
      L_ptr = strstr(L_ptr, "\n");
      if (L_ptr) { 
        L_current_line++; 
        if (L_current_line != m_line) {
          L_ptr++;
          if (*L_ptr) { L_line = L_ptr ;}
        } else {
          *L_ptr = 0 ;
          L_buffer = L_line ;
        }
      } else {
        L_ptr = NULL ;
        L_buffer = NULL ;
        break ;
      }
    }
  }
  

  if (L_buffer) {
    L_error = regexec(&m_internalRegExp, L_buffer, 
                      MAX_MATCH, L_pmatch, REG_EXTENDED | REG_NEWLINE);
    if (L_error == 0) {
      for(L_i=0; L_i < MAX_MATCH; L_i++) {
        if(L_pmatch[L_i].rm_eo == -1) break ;
        if (L_i == m_sub_match) {
          L_ret = 0 ;
          *P_start = L_pmatch[L_i].rm_so ;
          *P_end = L_pmatch[L_i].rm_eo ;
        }
      }
    }
  }

  if (L_ptr) { *L_ptr = '\n'; }

  return(L_ret);
}

char* C_RegExp::execute(char* P_buffer, int *P_size) {
  regmatch_t L_pmatch[MAX_MATCH]      ;
  int        L_error                  ;
  int        L_i                      ;
  char*      L_result      = NULL     ;
  char*      L_buffer      = P_buffer ;
  char*      L_line        = NULL     ;
  char*      L_ptr         = NULL     ;
  
  memset((void*)L_pmatch, 0, sizeof(regmatch_t)*MAX_MATCH);

  if (m_line > 0) {
    // explicit line of the buffer = m_line
    int   L_current_line = -1 ;

    L_ptr = L_buffer ;
    L_line = L_ptr;

    while (L_current_line != m_line) {
      L_ptr = strstr(L_ptr, "\n");
      if (L_ptr) { 
        L_current_line++; 
        if (L_current_line != m_line) {
          L_ptr++;
          if (*L_ptr) { L_line = L_ptr ;}
        } else {
          *L_ptr = 0 ;
          L_buffer = L_line ;
        }
      } else {
        L_ptr = NULL ;
        L_buffer = NULL ;
        break ;
      }
    }
  }
  
  if (L_buffer) {
    L_error = regexec(&m_internalRegExp, L_buffer, MAX_MATCH, L_pmatch, REG_EXTENDED | REG_NEWLINE);
    if (L_error == 0) {
      for(L_i=0; L_i < MAX_MATCH; L_i++) {
        if(L_pmatch[L_i].rm_eo == -1) break ;
        if (L_i == m_sub_match) {
          *P_size = L_pmatch[L_i].rm_eo - L_pmatch[L_i].rm_so ;
          L_result = setSubString(L_buffer, 
                                  L_pmatch[L_i].rm_so, L_pmatch[L_i].rm_eo);
        }
      }
      if (L_i != m_nb_match) {
        FREE_TABLE(L_result);
      }
    }
  }
  
  if (L_ptr) { *L_ptr = '\n'; }

  return(L_result);

}



C_RegExp::~C_RegExp() {
  m_regularExpression = NULL ;
  regfree(&m_internalRegExp); 
}











