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

#ifndef _EXTERNAL_METHOD_DEF_H
#define _EXTERNAL_METHOD_DEF_H

#include "list_t.hpp"
#include "ExternalMethod.h"

typedef struct _defmethod_extern_t {
  char                 *m_name        ; 
  char                 *m_param       ;
} T_DefMethodExtern, *T_pDefMethodExtern ;
 
typedef list_t<T_DefMethodExtern> T_DefMethodExternList, 
  *T_pDefMethodExternList ;

typedef struct _defmethod_external_t {
  int              m_id ;
  T_ExternalMethod m_method ;
} T_DefMethodExternal, *T_pDefMethodExternal ;

T_ExternalMethod create_external_method (char* P_args);

#endif // _EXTERNAL_METHOD_DEF_H
