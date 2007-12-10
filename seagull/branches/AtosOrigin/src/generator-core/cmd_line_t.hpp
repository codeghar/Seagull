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

#ifndef _C_COMMANDLINET_H_
#define _C_COMMANDLINET_H_

typedef struct _struct_cmd_line_t {
  int m_nb_args ;
  char ** m_args  ;
} cmd_line_t, *cmd_line_pt ;

// void copy_cmd_line (cmd_line_pt P_src, cmd_line_pt P_dest);
cmd_line_pt create_cmd_line (int P_nb_args);
void copy_cmd_line (cmd_line_pt P_dest, int P_argc, char**P_argv) ;
void  destroy_cmd(cmd_line_pt P_cmd_line);


#endif // _C_COMMANDLINET_H_
