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
#ifndef _SCTP_SOCKET_
#define _SCTP_SOCKET_


#include "C_Socket.hpp"
#include "socketSCTP_t.hpp"

class C_SocketSCTPListen : public C_SocketListen {

public:
  C_SocketSCTPListen(T_SocketType  P_type, 
                       T_pIpAddr     P_addr, 
                       int           P_channel_id,
                       size_t        P_read_buf_size,
                       size_t        P_segm_buf_size) ;
  C_SocketSCTPListen(C_SocketSCTPListen& P_Socket);
  virtual ~C_SocketSCTPListen() ;  
  virtual void    set_properties () ;
  C_Socket* process_fd_set(fd_set           *P_rSet, 
			       fd_set           *P_wSet,
			       C_TransportEvent *P_event) ;
  virtual int   _open_init(int                   P_socket_domain, 
                 size_t                P_buffer_size,
                 C_ProtocolBinaryFrame *P_protocol);
  virtual int   _open (size_t                P_buffer_size, 
                 C_ProtocolBinaryFrame *P_protocol);
  virtual void  _close () ;
protected:
  size_t  m_recv_buflen ;
  char   *m_recv_buf ;
};

 

class C_SocketSCTPWithData : public C_SocketWithData {
public:
  C_SocketSCTPWithData(int P_channel_id,
                       size_t P_read_buf_size,
                       size_t P_segm_buf_size);
  C_SocketSCTPWithData(T_SocketType P_type, 
		                   T_pIpAddr P_addr, 
		                   int P_channel_id,
                       size_t P_read_buf_size,
                       size_t P_segm_buf_size) ;
  C_SocketSCTPWithData(C_SocketSCTPWithData &P_Socket);


  virtual void                   set_properties () ;
  
  virtual int _open(int                   P_socket_domain, 
                 size_t                P_buffer_size,
                 C_ProtocolBinaryFrame *P_protocol);
  
  C_SocketSCTPWithData* process_fd_set (fd_set           *P_rSet,
                                        fd_set           *P_wSet,
                                        C_TransportEvent *P_event);
  size_t received_buffer(unsigned char *P_data, 
                         size_t         P_size_buf,
                         struct timeval *P_time);
  virtual int _call_read();                        
  virtual int _call_write(unsigned char* P_data, size_t P_size)  ;
  virtual int _write(unsigned char* P_data, size_t P_size)  ;
  virtual void   _close () ;

protected:

   
   bool                m_data_recv ;
 
   void                sctp_event_handler (C_TransportEvent *P_event) ;
   bool                sctp_recv_msg (struct msghdr *msg, ssize_t *nrp, size_t cmsglen);
   void                sctp_dispatch_msg (C_TransportEvent *P_event) ;
   
} ;

class C_SocketSCTPServer : public C_SocketSCTPWithData {

public:
  C_SocketSCTPServer(C_SocketSCTPListen *P_listen, 
		 int             P_channel_id,
		 size_t          P_read_buf_size,
		 size_t          P_segm_buf_size);
  C_SocketSCTPServer(T_SocketType  P_type, 
		 T_pIpAddr     P_addr, 
		 int           P_channel_id,
		 size_t        P_read_buf_size,
		 size_t        P_segm_buf_size) ;
  C_SocketSCTPServer (C_SocketSCTPServer& P_Socket);
  virtual ~C_SocketSCTPServer();

  virtual int _open (size_t P_buffer_size, C_ProtocolBinaryFrame *P_protocol) ;
  virtual int _read ();

  int _open_udp (size_t P_buffer_size, 
                 C_ProtocolBinaryFrame *P_protocol) ;

private:
  C_SocketSCTPListen     *m_listen_sock ;
  T_pIpAddr           m_source_udp_addr_info ;

} ;


class C_SocketSCTPClient : public C_SocketSCTPWithData {
public:
  C_SocketSCTPClient(T_SocketType  P_type, 
		                 T_pIpAddr     P_addr, 
		                 int           P_channel_id,
                     size_t        P_read_buf_size,
                     size_t        P_segm_buf_size) ;
  C_SocketSCTPClient(C_SocketSCTPClient &P_Socket);
  virtual ~C_SocketSCTPClient() ;

  virtual int _open (T_pOpenStatus P_status,
                     size_t        P_buffer_size,
                     C_ProtocolBinaryFrame *P_protocol);
  int _read ();

} ;



#endif // _SCTP_SOCKET_
