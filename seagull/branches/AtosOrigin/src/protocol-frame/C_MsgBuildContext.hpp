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

#ifndef _C_MSGBUILDCONTEXT_H
#define _C_MSGBUILDCONTEXT_H

#include "C_MessageFrame.hpp"

class C_MsgBuildContext {
public:
  typedef bool (C_MsgBuildContext::* T_ContextFunction) (T_pValueData);
  typedef bool (C_MsgBuildContext::* T_ContextStringFunction) (char**,T_pValueData);
           C_MsgBuildContext() {} ;
  virtual ~C_MsgBuildContext() {} ;
  virtual void init_from_external() = 0 ; 
  virtual void init_to_external() = 0 ; 
} ;


#endif // _C_MSGBUILDCONTEXT_H
