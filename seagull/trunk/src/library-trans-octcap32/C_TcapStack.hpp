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

#ifndef _C_TCAPSTACK_
#define _C_TCAPSTACK_

#include "C_TransportEvent.hpp"
#include "C_ProtocolExternalFrame.hpp"
#include "ReceiveMsgContext.h"
#include "T_ConfigValue.h"
#include "C_Transport.hpp"

#include "select_t.hpp"
#include "dlfcn_t.hpp"
#include "list_t.hpp"


class C_TcapStack {
public:
           C_TcapStack(char *P_library_name);
  virtual ~C_TcapStack();
  virtual int init (T_logFunction P_logError, T_logFunction P_logInfo) = 0 ;
  virtual int config (T_pConfigValueList P_config_param_list) = 0 ;
  virtual int open (int P_channel_id, char *P_class, 
		    int P_appl, int P_inst, int P_ossn,
		    C_ProtocolExternalFrame *P_protocol) = 0 ;
  virtual int close (int P_cnx) = 0 ;

  virtual int pre_select  (int                P_n, 
		           fd_set            *P_readfds,  
			   fd_set            *P_writefds,
			   fd_set            *P_exceptfds, 
			   struct timeval    *P_timeout) = 0 ; 

  virtual int post_select (int                  P_n, 
		           fd_set              *P_readfds,  
			   fd_set              *P_writefds,
			   fd_set              *P_exceptfds, 
			   T_pC_TransportEvent  P_eventTable,
			   size_t              *P_nb) = 0 ;

  virtual bool get_message (int P_id, T_pReceiveMsgContext P_ctxt) = 0 ;
  virtual int  send_message(int P_id, C_MessageFrame *P_msg) = 0 ;


protected:
  char *m_library_name ;
private:
} ;

#endif // _C_TCAPSTACK_







