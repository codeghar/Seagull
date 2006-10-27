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


#include "cmd_line_t.hpp"
#include "Utils.hpp"
#include "string_t.hpp"
#include "iostream_t.hpp"


cmd_line_pt create_cmd_line (int P_nb_args) {
  cmd_line_pt L_result ;
  int L_i ;

  ALLOC_VAR(L_result,
            cmd_line_pt,
            sizeof(cmd_line_t));
  L_result->m_nb_args = P_nb_args ;
  ALLOC_TABLE(L_result->m_args,
            char**,
            sizeof(char*),
            P_nb_args);
  for (L_i=0; L_i < P_nb_args ; L_i++) {
    L_result->m_args[L_i] = NULL ;
  }
  return(L_result);
}


void copy_cmd_line (cmd_line_pt P_dest, int P_argc, char**P_argv) {
  int L_i;

  P_dest->m_nb_args = P_argc ;
  for(L_i = 0 ; L_i < P_argc ; L_i ++) {
    if (P_dest->m_args[L_i]) {
      FREE_TABLE(P_dest->m_args[L_i]);
    }
    ALLOC_TABLE(P_dest->m_args[L_i],
                char*,
                sizeof(char),
                strlen(P_argv[L_i])+1);
    
    memcpy(P_dest->m_args[L_i],
           P_argv[L_i],
           strlen(P_argv[L_i]));
    P_dest->m_args[L_i][strlen(P_argv[L_i])] = 0 ;
  }
}


void  destroy_cmd(cmd_line_pt P_cmd_line) {

  int L_i;

  for(L_i = 0 ; L_i < P_cmd_line->m_nb_args ; L_i ++) {
    FREE_TABLE(P_cmd_line->m_args[L_i]);
  }
}


