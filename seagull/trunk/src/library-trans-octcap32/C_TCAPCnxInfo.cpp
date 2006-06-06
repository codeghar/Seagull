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

#include "C_TCAPCnxInfo.hpp"

#include "Utils.hpp"

T_pCnxInfo create_CnxInfo () {
  T_pCnxInfo L_ret = NULL ;
  ALLOC_VAR(L_ret, T_pCnxInfo, sizeof(T_CnxInfo));
  clear_CnxInfo(L_ret);
  return (L_ret);
}

void  clear_CnxInfo(T_pCnxInfo P_CnxInfo) {
  if (P_CnxInfo != NULL) {
    P_CnxInfo->m_cnx_id      = -1   ;
    P_CnxInfo->m_class_value = NULL ;
    P_CnxInfo->m_ossn        = -1   ;
    P_CnxInfo->m_app_id      = -1   ;
    P_CnxInfo->m_inst_id     = -1   ;
  }
}

void  delete_CnxInfo(T_pCnxInfo *P_CnxInfo) {

  T_pCnxInfo L_CnxInfo = NULL ;
  if (P_CnxInfo != NULL) {
    L_CnxInfo = *P_CnxInfo ;

    FREE_VAR(L_CnxInfo->m_class_value);
    *P_CnxInfo = NULL ;
  }
  
}
