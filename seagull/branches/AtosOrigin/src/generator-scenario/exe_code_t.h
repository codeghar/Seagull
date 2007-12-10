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

#ifndef _EXE_CODE_T_H
#define _EXE_CODE_T_H

typedef enum _enum_exe_code {
  E_EXE_NOERROR = 0,
  
  E_EXE_TRAFFIC_END,
  E_EXE_DEFAULT_END,
  E_EXE_ABORT_END,
  
  E_EXE_INIT_END,
  E_EXE_ERROR_MSG,
  E_EXE_ERROR_SEND,
  E_EXE_ERROR_CHECK,
  E_EXE_SUSPEND,
  E_EXE_IGNORE,
  E_EXE_ABORT_CHECK,
  E_EXE_TIMEOUT,
  E_EXE_ERROR
} T_exeCode ;


#endif // _EXE_CODE_T_H



