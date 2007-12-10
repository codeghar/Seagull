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

#ifndef _C_TRANSPORT_
#define _C_TRANSPORT_

#include "select_t.hpp"
#include "iostream_t.hpp"

#include "C_ProtocolFrame.hpp"
#include "C_TransportEvent.hpp"
#include "ReceiveMsgContext.h"
#include "T_ConfigValue.h"

typedef enum _open_status {
  E_OPEN_OK = 0,
  E_OPEN_DELAYED,
  E_OPEN_FAILED
} T_OpenStatus, *T_pOpenStatus ;

typedef void (*T_logFunction)(char*);


class C_Transport {

public:

           C_Transport () {} ;
  virtual ~C_Transport () {} ;

  virtual int init (char* P_buf, 
		    T_logFunction P_logError, 
		    T_logFunction P_logInfo) = 0 ;

  virtual int config (T_pConfigValueList P_config_param_list) = 0 ;
  virtual int close () = 0 ;

  virtual int open (int              P_channel_id,
		    char*            P_buf, 
		    T_pOpenStatus    P_status,
		    C_ProtocolFrame *P_protocol) = 0 ;
  virtual int close (int P_id) = 0 ;

  virtual int         pre_select (int                   P_n, 
				  fd_set               *P_readfds,  
				  fd_set               *P_writefds,
				  fd_set               *P_exceptfds, 
				  struct timeval       *P_timeout, 
				  int                  *P_cnx, 
				  size_t                P_nb) = 0 ;


  virtual int             post_select     (int                  P_n, 
					   fd_set              *P_readfds,  
					   fd_set              *P_writefds,
					   fd_set              *P_exceptfds, 
					   T_pC_TransportEvent  P_eventTable,
					   size_t              *P_nb) = 0 ;
  
  virtual bool            get_message(int P_id, T_pReceiveMsgContext P_ctxt) = 0 ;
  virtual int             send_message(int P_id, C_MessageFrame *P_msg) = 0 ;

  virtual int             set_option (int P_Channel_Id, char *P_buf) {return (0);} ;
  //  virtual int             set_context (int P_id, void *P_context) = 0 ;
  virtual T_SelectDef     select_definition () = 0;

} ;

typedef C_Transport  *T_pTransport ;
typedef T_pTransport *T_ppTransport ;

// transport creation related generic functions / structures
typedef T_pTransport (*T_TransportCreateInstance) (void) ;
typedef void         (*T_TransportDeleteInstance) (T_ppTransport) ;

typedef struct _trans_struct {
  T_TransportCreateInstance m_create ;
  T_TransportDeleteInstance m_delete ;
} T_ExternalTransportDef, *T_pExternalTransportDef ;


#endif // _C_TRANSPORT__

