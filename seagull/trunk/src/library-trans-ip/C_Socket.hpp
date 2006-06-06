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

#ifndef _CPP_SOCKET_
#define _CPP_SOCKET_

#include "list_t.hpp"
#include "map_t.hpp"

#include "S_IpAddr.hpp"
#include <netinet/tcp.h>
#include <netdb.h>

#include "C_Transport.hpp"
#include "C_TransportEvent.hpp"
#include "C_DataDecode.hpp"
#include "C_ProtocolBinaryFrame.hpp"

#include "ReceiveMsgContext.h"


typedef enum _enum_socket_mode {
  E_SOCKET_TCP_MODE,
  E_SOCKET_UDP_MODE
} T_SocketType, *T_pSocketType ;

typedef struct _data_rcv {
  size_t         m_size ;
  struct timeval m_time ;
  unsigned char *m_data ;
} T_DataRcv, *T_pDataRcv ;

typedef enum _socket_state {
  E_SOCKET_STATE_NOT_READY = 0,
  E_SOCKET_STATE_INPROGESS,
  E_SOCKET_STATE_READY
} T_SocketState, *T_pSocketState ;

typedef list_t<T_pDataRcv> T_DataRcvList, *T_pDataRcvList ;

class C_Socket {
public:
   C_Socket (int P_channel_id) ;
   C_Socket (T_SocketType P_type, int P_port, int P_channel_id);
   C_Socket (T_SocketType P_type, int P_channel_id);

   int _open (int    P_socket_domain, 
	      size_t P_buffer_size,
	      C_ProtocolBinaryFrame *P_protocol);

  virtual ~C_Socket () ;
  virtual size_t received_buffer (unsigned char  *P_data, 
				  size_t          P_size_buf,
				  struct timeval *P_time) = 0 ;
  int     get_id () ;
  int     get_channel_id();
  void    _close () ;
  void    set_fd_set (fd_set *P_rSet, fd_set *P_wSet);
  virtual C_Socket* process_fd_set(fd_set           *P_rSet, 
				   fd_set           *P_wSet,
				   C_TransportEvent *P_event) = 0 ;
  void          set_channel_id(int P_channel_id);
  void          set_properties () ;
  C_pDataDecode          get_decode () ;
  C_ProtocolBinaryFrame* get_protocol() ;
  T_pRcvMsgCtxtList      get_list() ;

protected:

  int                 m_socket_id   ; 
  int                 m_channel_id     ;
  T_SocketType        m_type        ;
  int                 m_src_port    ;
  T_SocketState       m_state       ;
  size_t              m_buffer_size ;

  // decode process temporary
  C_ProtocolBinaryFrame  *m_protocol ;
  C_pDataDecode           m_decode   ;

  T_pRcvMsgCtxtList       m_recv_msg_list ;

} ;

class C_SocketWithData : public C_Socket {

public:

  C_SocketWithData(int P_channel_id,
                   size_t P_read_buf_size,
                   size_t P_segm_buf_size);
  C_SocketWithData(T_SocketType P_type, 
		   T_pIpAddr P_addr, 
		   int P_channel_id,
                   size_t P_read_buf_size,
                   size_t P_segm_buf_size) ;
  virtual ~C_SocketWithData() ;
  C_Socket* process_fd_set(fd_set *P_rSet, fd_set *P_wSet, C_TransportEvent *P_event) ;
  size_t received_buffer (unsigned char  *P_data, 
			  size_t          P_size_buf,
			  struct timeval *P_time) ;
  
protected:

  T_DataRcvList           m_data_queue ;
  T_pIpAddr               m_remote_addr_info ;
  T_SockAddrStorage       m_accepted_addr ;

  size_t                  m_read_buf_size ;
  char                   *m_read_buf      ;

} ;

class C_SocketListen : public C_Socket {

public:

             C_SocketListen(T_SocketType  P_type, 
			    T_pIpAddr     P_addr, 
			    int           P_channel_id,
                            size_t        P_read_buf_size,
                            size_t        P_segm_buf_size) ;
  virtual   ~C_SocketListen() ;
  int       _open (size_t P_buffer_size, C_ProtocolBinaryFrame *P_protocol) ;
  C_Socket* process_fd_set(fd_set           *P_rSet, 
			   fd_set           *P_wSet,
			   C_TransportEvent *P_event) ;
  size_t received_buffer  (unsigned char  *P_data, 
			   size_t          P_size_buf,
			   struct timeval *P_time) ;
  T_SockAddrStorage * get_source_address () ;
  
protected:
  
  T_pIpAddr           m_source_addr_info ;

private:

  size_t              m_read_buf_size ;
  size_t              m_segm_buf_size ;

} ;

class C_SocketServer : public C_SocketWithData {

public:
           C_SocketServer(C_SocketListen *P_listen, 
			  int P_channel_id,
                          size_t P_read_buf_size,
                          size_t P_segm_buf_size);
  virtual ~C_SocketServer();
  int _open (size_t P_buffer_size, C_ProtocolBinaryFrame *P_protocol) ;

private:
  C_SocketListen *m_listen_sock ;

} ;


class C_SocketClient : public C_SocketWithData {
public:
  C_SocketClient(T_SocketType  P_type, 
		 T_pIpAddr     P_addr, 
		 int           P_channel_id,
                 size_t        P_read_buf_size,
                 size_t        P_segm_buf_size) ;
  virtual ~C_SocketClient() ;
  int _open (T_pOpenStatus P_status,
	     size_t        P_buffer_size,
	     C_ProtocolBinaryFrame *P_protocol);
} ;

typedef map_t<int, C_Socket*> T_SocketMap, *T_pSocketMap ;

#endif // _CPP_SOCKET_





