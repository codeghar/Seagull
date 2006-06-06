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


C_ProtocolFrame::T_MsgError parse_xml (char   *P_buf,
                                       size_t *P_size) {

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


