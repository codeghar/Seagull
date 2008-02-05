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

#ifndef _C_TRANS_IP_
#define _C_TRANS_IP_

#define NO_TRANS_IP_CONTEXT 0L

#include <sys/time.h>

#include "list_t.hpp"
#include "C_Transport.hpp"
#include "S_IpAddr.hpp"

#include "C_Socket.hpp"
#include "C_ProtocolBinaryFrame.hpp"

class C_TransIP : public C_Transport {

public:

   C_TransIP ();
  virtual ~C_TransIP ();

  virtual int init (char *P_buf,
                    T_logFunction P_logError,
                    T_logFunction P_logInfo) ;

  virtual int         config (T_pConfigValueList P_config_param_list) ;

  int         open (int              P_channel_id,
                    char            *P_buf, 
                    T_pOpenStatus    P_status,
                    C_ProtocolFrame *P_protocol);
  
  int         pre_select (int             P_n, 
			  fd_set         *P_readfds,  
			  fd_set         *P_writefds,
			  fd_set         *P_exceptfds, 
			  struct timeval *P_timeout, 
			  int            *P_cnx, 
			  size_t          P_nb) ;

  int         post_select (int                 P_n, 
			   fd_set             *P_readfds,  
			   fd_set             *P_writefds,
			   fd_set             *P_exceptfds, 
			   T_pC_TransportEvent P_eventTable,
			   size_t             *P_nb);

  int         send_message    (int             P_id,
			       C_MessageFrame *P_msg);

  bool        get_message (int P_id, T_pReceiveMsgContext P_ctxt) ;

  int         set_channel_id (int P_id, int P_channel_id);
  
  int         close (); 
  int         close (int P_id);
  
  T_SelectDef select_definition () ;

protected:

  T_SocketType    m_trans_type ;
  T_SocketMap     m_socket_map ;

  int             m_max_fd     ;

  bool            m_active     ;
  
  T_IpAddrMap     m_ip_addr_map ;

  virtual bool      analyze_init_string (char *P_buf) ;
  void              analyze_optional_init_string (char *P_buf) ;
  bool              analyze_open_string (char *P_buf, T_pIpAddr P_addr, bool active) ;

  virtual C_Socket* open              (int              P_channel_id, 
                                       T_pIpAddr        P_Addr,
                                       T_pOpenStatus    P_status,
                                       C_ProtocolBinaryFrame *P_protocol) ;
  int               extract_ip_addr(T_pIpAddr P_pIpAddr);
  int               resolve_addr(T_pIpAddr P_pIpAddr);
  int               inet_addr   (char                   **P_addr, 
				 T_SockAddrStorage       *P_AddrS);

  void              decode_from_protocol (C_Socket *P_socket);

  bool              analyze_ulong_value (char* P_buf, 
					 char* P_pattern,
					 size_t* P_value);

  bool              analyze_string_value (char* P_buf, 
					  char* P_pattern,
					  char* P_value);
  
  size_t                        m_buffer_size ;

  size_t	                m_encode_buffer_size, 
                                m_decode_buffer_size, 
                                m_read_buffer_size ;
  unsigned char                *m_encode_buffer, 
                               *m_decode_buffer ;

  list_t<T_SocketMap::iterator> m_delete_list ;
  list_t<C_Socket *>            m_insert_list ;
  list_t<T_SocketMap::iterator> m_close_list  ;
  list_t<int>                   m_close_event_list ;


  T_logFunction                 m_logError, 
                                m_logInfo ;

} ;

typedef C_TransIP *T_pC_TransIP ;

extern "C" T_pTransport create_cipio_instance () ;
extern "C" void         delete_cipio_instance (T_ppTransport) ;

#endif // _C_TRANS_IP_
