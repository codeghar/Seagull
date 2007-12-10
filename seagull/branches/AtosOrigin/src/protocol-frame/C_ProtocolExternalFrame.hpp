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

#ifndef    _C_PROTOCOLEXTERNALFRAME_H
#define    _C_PROTOCOLEXTERNALFRAME_H

// Protocol External Frame Definition Class

#include "C_ProtocolFrame.hpp"
#include "C_MsgBuildContext.hpp"
#include "C_MsgBuildContextFactory.hpp"
#include "ReceiveMsgContext.h"


class C_ProtocolExternalFrame : public C_ProtocolFrame { 

public:


//  typedef struct _paramdef_t {
//    char *m_name        ; 
//    char *m_default     ;
//    char *m_mandatory   ;
//  } T_ParamDef, *T_pParamDef ;
//  typedef list_t<T_ParamDef> T_ParamDefList, *T_pParamDefList ;


           C_ProtocolExternalFrame() { m_type = E_PROTOCOL_EXTERNAL; } ;
  virtual ~C_ProtocolExternalFrame() {} ;

  virtual  T_MsgError from_external (C_MsgBuildContext* P_build,
				     T_pReceiveMsgContext P_Ctx) = 0 ;
  virtual  T_MsgError to_external (C_MsgBuildContext* P_build,
				   C_MessageFrame *P_msg) = 0 ;

  virtual  C_MsgBuildContextFactory* get_factory() = 0 ;

  virtual  void log_message (char *P_header, C_MessageFrame *P_msg) = 0 ;

protected:
private:

} ;

typedef C_ProtocolExternalFrame *T_pC_ProtocolExternalFrame ;

#endif  // _C_PROTOCOLEXTERNALFRAME_H


