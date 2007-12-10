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

#ifndef _C_TCAP_CNX_INFO_HPP
#define _C_TCAP_CNX_INFO_HPP

#include "list_t.hpp"
#include "map_t.hpp"



typedef struct _struct_cnx_info {
  int                  m_cnx_id      ;
  char                *m_class_value ;
  int                  m_ossn        ; 
  int                  m_app_id      ;
  int                  m_inst_id     ;
  // other fields 
} T_CnxInfo, *T_pCnxInfo ;



void       clear_CnxInfo(T_pCnxInfo P_CnxInfo) ;
T_pCnxInfo create_CnxInfo() ;
void       delete_CnxInfo(T_pCnxInfo *P_CnxInfo) ;

typedef list_t<T_pCnxInfo> T_CnxInfoList, *T_pCnxInfoList ;
typedef map_t<int, T_pCnxInfo> T_CnxInfoMap, *T_pCnxInfoMap ;


#endif // _C_TCAP_CNX_INFO_HPP



