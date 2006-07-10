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

#include "parser_h248.hpp"
#include "iostream_t.hpp"
#include <regex.h>
#include "string_t.hpp"
#include "Utils.hpp"


C_ProtocolFrame::T_MsgError parse_h248 (char   *P_buf,
                                        size_t *P_size,
                                        char   *P_buf_header,
                                        size_t  P_size_header) {

  C_ProtocolFrame::T_MsgError L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS ;

  size_t L_size = *P_size ;
  int    L_nb_acc = 1 ;
  size_t L_i = 0 ;

  while (L_size) {
    switch (P_buf[L_i]) {
    case '{':
      L_nb_acc++ ;
      break ;
    case '}':
      L_nb_acc-- ;
      break ;
    default :
      break ;
    }
    L_size-- ;

    L_i++;
    
    if (L_nb_acc == 0) {
      L_error = C_ProtocolFrame::E_MSG_OK ;
      *P_size -= (L_i+1) ;
      break ;
    }
  }
  
  return (L_error) ;
}


char* skip_blank(char    *P_ptr, 
                 char    *P_buffer, 
                 size_t   P_size_buffer,
                 size_t  *P_size) {
  
  char     *L_blank_ptr    = NULL     ;
  char     *L_new_ptr      = P_ptr    ;


  L_blank_ptr = P_ptr ;
  while (((L_blank_ptr) && (L_blank_ptr < (P_buffer + P_size_buffer))) &&
         ((*L_blank_ptr == ' ') ||
          (*L_blank_ptr == '\t'))) { L_blank_ptr++ ; }
  if (L_blank_ptr != P_ptr) {
    *(P_size) = (L_blank_ptr - P_ptr) ;
    L_new_ptr = L_blank_ptr ;
  }

  return (L_new_ptr) ;
}


char * filter_h248(char* P_buffer) {

  size_t    L_size         = 0        ;

  size_t    L_size_buffer  = 0        ;
  size_t    L_size_end     = 0        ;
  
  char     *L_pos          = NULL     ;
  char     *L_ptr          = P_buffer ;

  char     *L_result       = NULL     ;
  char     *L_new          = NULL     ;

  bool      L_skip_blank   = true     ;
  size_t    L_size_blank   = 0        ;


  if ((P_buffer != NULL) && 
      ((L_size_buffer = strlen(P_buffer)) > 0 )) {

    L_size_end = L_size_buffer ;

    ALLOC_TABLE(L_result, 
		char*, 
		sizeof(char), 
		L_size_buffer);

    if ((strchr(L_ptr,'\n')) == NULL) {
      L_new = L_result ;
      L_size = L_size_buffer ;

      // skip blank
      if (L_skip_blank) {
        L_ptr = skip_blank(L_ptr,P_buffer, L_size_buffer, &L_size_blank) ;
        L_size -= L_size_blank ;
      }

      memcpy(L_new, L_ptr, L_size);
      L_new += (L_size+1) ;

      *L_new = '\0' ;
      *(L_new-1) = '\n' ;

    } else {
      // if '\n' exists

      while(   (L_ptr) 
            && (L_pos = strchr(L_ptr,'\n')) != NULL) {

        L_size_blank = 0 ;
        // L_size : from start to '\n' not included
	L_size = L_pos - L_ptr ;
        // skip blank
        if (L_skip_blank) {
          L_ptr = skip_blank(L_ptr,P_buffer, L_size_buffer, &L_size_blank) ;
          L_size -= L_size_blank ;
          L_size_end -= L_size_blank ;
        }

        if (L_new == NULL) { L_new = L_result ; } else { L_new += 1 ; }
	memcpy(L_new, L_ptr, L_size);

	L_new += L_size ;
        // test end needed ? for L_ptr
        if ((L_pos + 1) <= (P_buffer+L_size_buffer)) { 
          L_ptr = L_pos + 1 ; 
        } else { 
          L_ptr = NULL ; 
        }

        L_size_end -= (L_size + 1) ;

        *(L_new) = '\n' ;

      } // while


      // ctrl the end of buffer
      if (L_size_end > 0) {

        L_size = L_size_end ;
        L_size_blank = 0 ;

        // skip blank
        if (L_skip_blank) {
          L_ptr = skip_blank(L_ptr,P_buffer, L_size_buffer, &L_size_blank) ;
          L_size -= L_size_blank ;
        }


        if (L_size) {
          L_new +=1 ;

          memcpy(L_new, L_ptr, L_size);
          L_new += (L_size+1) ;

          *L_new = '\0' ;
          *(L_new-1) = '\n' ;
        } else {
          // add final '\0' 
          L_new += 1 ;
          *L_new = '\0' ;
        }
      } else {
        // add final '\0' 
        L_new += 1 ;
        *L_new = '\0' ;
        
      }
    }
  }

  return (L_result);

}

