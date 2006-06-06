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

#ifndef _C_RECEIVE_CONTEXT_
#define _C_RECEIVE_CONTEXT_

#include "C_ContextFrame.hpp"
#include <sys/time.h>
#include "C_MessageFrame.hpp"
#include "list_t.hpp"

class C_ReceiveMsgContext : public C_ContextFrame {
 public:
  C_ReceiveMsgContext() : C_ContextFrame() {} ;
  ~C_ReceiveMsgContext() {} ;
  // message information
  C_MessageFrame  *m_msg ;
  // time information
  struct timeval   m_time ;
  // channel retrieve information
  int              m_channel  ;
  int              m_response ;
} ;


typedef C_ReceiveMsgContext T_ReceiveMsgContext, *T_pReceiveMsgContext ;



/*  typedef struct _rcvMsgCtxt { */
/*    // message information */
/*    C_MessageFrame  *m_msg ; */
/*    // time information */
/*    struct timeval   m_time ; */
/*    // channel retrieve information */
/*    int              m_channel  ; */
/*    int              m_response ; */
/*  } T_ReceiveMsgContext, *T_pReceiveMsgContext ; */

typedef list_t<T_ReceiveMsgContext> T_RcvMsgCtxtList, *T_pRcvMsgCtxtList ;

#endif // _C_RECEIVE_CONTEXT_
