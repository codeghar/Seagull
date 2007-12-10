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

#ifndef _C_MSGBUILDCONTEXTFACTORY_H
#define _C_MSGBUILDCONTEXTFACTORY_H

#include "C_MsgBuildContext.hpp"

class C_MsgBuildContextFactory {
public:
           C_MsgBuildContextFactory() {} ;
  virtual ~C_MsgBuildContextFactory() {} ;

  virtual C_MsgBuildContext* create_context() = 0 ;
  virtual void               delete_context(C_MsgBuildContext**) = 0 ;
  virtual C_MsgBuildContext::T_ContextFunction get_function (char *) = 0 ;
  virtual C_MsgBuildContext::T_ContextStringFunction get_string_function 
  (char *) = 0 ;

} ;

typedef C_MsgBuildContextFactory* (*T_CreateContextFactory) () ;
typedef void (*T_DeleteContextFactory) (C_MsgBuildContextFactory**);
typedef struct _factory_info_t {
  T_CreateContextFactory m_create ;
  T_DeleteContextFactory m_delete ;
} T_FactoryInfo, *T_pFactoryInfo ;

#endif // _C_MSGBUILDCONTEXTFACTORY_H
