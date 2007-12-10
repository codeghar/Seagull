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

#ifndef _C_SECURE_SOCKET_H
#define _C_SECURE_SOCKET_H


#include "ssl_t.hpp"
#include "C_Socket.hpp"

class C_SecureSocket {
public:
  C_SecureSocket(SSL_CTX * P_ssl_ctx) ;
 ~C_SecureSocket() ;

  void ssl_error(int P_resultCode);


protected:
  SSL_CTX * m_ssl_ctx ;
  BIO     * m_bio ;
  SSL     * m_ssl ;


} ;

class C_SecureSocketListen : public C_SocketListen, public C_SecureSocket {

public:
  C_SecureSocketListen(SSL_CTX * P_ssl_ctx,
                       T_SocketType  P_type, 
                       T_pIpAddr     P_addr, 
                       int           P_channel_id,
                       size_t        P_read_buf_size,
                       size_t        P_segm_buf_size) ;
  C_SecureSocketListen(SSL_CTX * P_ssl_ctx, C_SocketListen& P_Socket);
  virtual ~C_SecureSocketListen() ;  
protected:
  virtual C_Socket*  create_socket_server(int *P_ret) ;

} ;

class C_SecureSocketServer : public C_SocketServer , public C_SecureSocket {

public:
  C_SecureSocketServer(SSL_CTX * P_ssl_ctx,
                       C_SocketListen *P_listen, 
                       int P_channel_id,
                       size_t P_read_buf_size,
                       size_t P_segm_buf_size);
  
  C_SecureSocketServer(SSL_CTX * P_ssl_ctx,
                       T_SocketType  P_type, 
                       T_pIpAddr     P_addr, 
                       int           P_channel_id,
                       size_t        P_read_buf_size,
                       size_t        P_segm_buf_size) ;
  
  C_SecureSocketServer(SSL_CTX * P_ssl_ctx, C_SocketServer& P_Socket);
  virtual ~C_SecureSocketServer();
  virtual int _call_read() ;

  virtual int _open(size_t                 P_buffer_size, 
                    C_ProtocolBinaryFrame *P_protocol) ;
  int _secure_mode() ;
  virtual int _call_write(unsigned char *P_buf, size_t P_size);
  virtual C_Socket* process_fd_in_progess(fd_set *P_rSet, 
                                          fd_set *P_wSet, 
                                          C_TransportEvent *P_event) ;


} ;

class C_SecureSocketClient : public C_SocketClient, public C_SecureSocket {
public:
  C_SecureSocketClient(SSL_CTX * P_ssl_ctx,
                       T_SocketType  P_type, 
                       T_pIpAddr     P_addr, 
                       int           P_channel_id,
                       size_t        P_read_buf_size,
                       size_t        P_segm_buf_size) ;
  C_SecureSocketClient(SSL_CTX * P_ssl_ctx, C_SocketClient& P_Socket);
  virtual ~C_SecureSocketClient() ;
  virtual int _call_read() ; 

  virtual int _open (T_pOpenStatus P_status,
                     size_t        P_buffer_size,
                     C_ProtocolBinaryFrame *P_protocol);
  int _secure_mode() ;
  virtual C_Socket* process_fd_in_progess(fd_set *P_rSet, 
                                          fd_set *P_wSet, 
                                          C_TransportEvent *P_event) ;
  virtual int _call_write(unsigned char *P_buf, size_t P_size);
  
} ;

#endif // _C_SECURE_SOCKET_H


