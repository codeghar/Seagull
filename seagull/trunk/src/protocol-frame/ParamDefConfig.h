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

#ifndef    _C_PARAMDEFCONFIG_H
#define    _C_PARAMDEFCONFIG_H


typedef struct _paramdef_t {
  char *m_name        ; 
  char *m_default     ;
  char *m_mandatory   ;
} T_ParamDef, *T_pParamDef ;
typedef list_t<T_ParamDef> T_ParamDefList, *T_pParamDefList ;



#endif // _C_PARAMDEFCONFIG_H
