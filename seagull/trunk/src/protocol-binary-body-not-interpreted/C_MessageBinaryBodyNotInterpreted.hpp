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

#ifndef _C_MESSAGEBINARYBODYNOTINTERPRETED_H
#define _C_MESSAGEBINARYBODYNOTINTERPRETED_H

#include "C_MessageBinary.hpp"
#include "C_ProtocolBinaryBodyNotInterpreted.hpp"
#include "C_ContextFrame.hpp"

class C_MessageBinaryBodyNotInterpreted : public C_MessageBinary {

public:
  C_MessageBinaryBodyNotInterpreted(C_ProtocolBinaryBodyNotInterpreted *P_protocol);
  virtual ~C_MessageBinaryBodyNotInterpreted();


  virtual T_pValueData get_session_id (C_ContextFrame *P_ctxt) ;

  virtual T_pValueData getSessionFromBody(int P_id);


  virtual bool get_body_value (T_pValueData P_res, int P_id) ;
  virtual bool set_body_value (int P_id, T_pValueData P_val) ;
  

private:

  // optim for prevent dynamic cast
  C_ProtocolBinaryBodyNotInterpreted* m_local_protocol ;


} ;

typedef C_MessageBinaryBodyNotInterpreted *T_pC_MessageBinaryBodyNotInterpreted;

#endif // _C_MESSAGEBINARYBODYNOTINTERPRETED_H

