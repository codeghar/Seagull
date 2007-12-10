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

#include "parser.hpp"
#include "iostream_t.hpp"
#include <regex.h>
#include "string_t.hpp"
#include "Utils.hpp"

/** Method to parse XML like payload.
 * 
 */
C_ProtocolFrame::T_MsgError parse_xml (char   *P_buf,
                                       size_t *P_size,
                                       char   *P_buf_header,
                                       size_t  P_size_header) {

  C_ProtocolFrame::T_MsgError L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING ;
  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[2] ;
  size_t     L_size = 0 ;
  char       L_tag[100] ;
  regoff_t   L_next ;

  size_t     L_parsed_size = 0 ;

  regex_t    L_reg_final ;
  string_t   L_string = "<[[:blank:]]*[/]" ;

  int        L_i = 0 ;
  char      *L_ptr = P_buf ;

  int        L_nb_loop = 0 ;

  // no body 

  if ((*P_size >= 2) && (P_buf[0] == '\r') && (P_buf[1]='\n')) {
    // there is no body 
    *P_size -= 2 ;
    L_error = C_ProtocolFrame::E_MSG_OK ;
  } else {


  L_status = regcomp (&L_reg_expr, 
		      "[[:blank:]]*<[[:blank:]]*([!-=?-z]*)",
		      REG_EXTENDED) ;
  
  if (L_status != 0) {

    regerror(L_status, &L_reg_expr, L_buffer, 100);
    regfree (&L_reg_expr) ;

  } else {


    while (L_nb_loop < 2) {
  
      L_tag[0]='\0' ;

      L_status = regexec (&L_reg_expr, L_ptr, 2, L_pmatch, 0) ;


      if (L_status == 0) {
        
        L_parsed_size += L_pmatch[0].rm_eo ;
        
        L_next = L_pmatch[0].rm_eo ;
        L_size = L_pmatch[1].rm_eo - L_pmatch[1].rm_so ; // tag xml
        
        memcpy(L_tag, L_ptr+L_pmatch[1].rm_so, L_size);
        L_tag[L_size]='\0' ;
        
        if (strcmp(L_tag,(char*)"?xml") == 0 ) {
          while ( ((L_next + L_i) <= (int)*P_size ) && (*(L_ptr+L_next+L_i) != '>') ) {
            L_i++;
          }

          L_ptr += L_next + L_i + 1 ;

          L_parsed_size += (L_i+1) ;

        } else {
          L_string += L_tag ;
          L_string += "[[:blank:]]*>[[:blank:]]*" ;
          L_status = regcomp (&L_reg_final, 
                              L_string.c_str(),
                              REG_EXTENDED) ;
          
          if (L_status != 0) {
            regerror(L_status, &L_reg_final, L_buffer, 100);
            regfree (&L_reg_final) ;
            break ;
          } else {
            L_status = regexec (&L_reg_final, L_ptr+L_next, 
                                1, L_pmatch, 0) ;
            regfree (&L_reg_final) ;
            
            if (L_status == 0) {

              L_parsed_size += L_pmatch[0].rm_eo ;

              // find \r\n at the end
              if ((L_parsed_size+2) <= *P_size) {
                if (   (*((L_ptr+L_next)+L_pmatch[0].rm_eo) == '\r')
                       && (*((L_ptr+L_next)+L_pmatch[0].rm_eo+1) == '\n')) {
                  L_parsed_size += 2 ; // \r\n parsed
                  L_error = C_ProtocolFrame::E_MSG_OK ;
                  break ;
                } else {
                  L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING ;
                  break ;
                }
              } else {
                L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS ;
                break ;
              }
              
            } else {
              L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING_SIZE_LESS ;
              break ;
            }
            
          }
        } 
      } else {
        L_error = C_ProtocolFrame::E_MSG_ERROR_DECODING ;
        break ;
      }
      
      L_nb_loop ++ ;
    } // while 

    regfree (&L_reg_expr) ;
    if (L_error == C_ProtocolFrame::E_MSG_OK) {
      *P_size -= L_parsed_size ;
    }

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




char * filter_xml(char* P_buffer) {

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
		(2*L_size_buffer));
    

    if ((strchr(L_ptr,'\n')) == NULL) {

      L_new = L_result ;
      L_size = L_size_buffer ;

      // skip blank
      if (L_skip_blank) {
        L_ptr = skip_blank(L_ptr,P_buffer, L_size_buffer, &L_size_blank) ;
        L_size -= L_size_blank ;
      }

      memcpy(L_new, L_ptr, L_size);
      L_new += (L_size - 1) ;
      if (*L_new != '\r') {
        L_new += 1 ;
        *L_new = '\r' ;
      }
      L_new += 2 ;
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
	L_new += (L_size - 1) ;
        // test end needed ? for L_ptr
        if ((L_pos + 1) <= (P_buffer+L_size_buffer)) { 
          L_ptr = L_pos + 1 ; 
        } else { 
          L_ptr = NULL ; 
        }

        L_size_end -= (L_size + 1) ;

        if (*L_new != '\r') {
          L_new += 1 ;
          *(L_new) = '\r' ;
        }
        L_new += 1 ;
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
          L_new += (L_size-1) ;
        
          if (*L_new != '\r') {
            L_new += 1 ;
            *(L_new) = '\r' ;
          }
          L_new += 2 ;
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

  if (L_result != NULL) {
    L_ptr = L_result ;
    while ((L_ptr = strstr(L_ptr, "\r\n\r\n")) != NULL ) {
      memmove(L_ptr+2, L_ptr+4, strlen(L_ptr+4));
      L_ptr += 2 ;
    }
  }

  return (L_result);
}
