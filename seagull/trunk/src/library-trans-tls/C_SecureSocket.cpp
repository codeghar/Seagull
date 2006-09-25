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

#include "Utils.hpp"
#include "C_SecureSocket.hpp"



#define SOCKET_ERROR(l,m) iostream_error << m << iostream_endl << iostream_flush
#ifdef DEBUG_MODE
#define SOCKET_DEBUG(l,m) iostream_error << m << iostream_endl << iostream_flush
#else
#define SOCKET_DEBUG(l,m)
#endif

C_SecureSocket::C_SecureSocket(SSL_CTX * P_ssl_ctx){
  m_ssl_ctx = P_ssl_ctx ;
  m_bio = NULL ;
  m_ssl = NULL ;
}


C_SecureSocket::~C_SecureSocket(){
  int   L_ret ;

  m_ssl_ctx = NULL ;

  if (m_ssl) {
    L_ret = SSL_shutdown(m_ssl);
    switch(L_ret) {
    case 1:
      break; 
    case 0:
    case -1:
      break; 
    default:
      SOCKET_ERROR(0, "SSL connect error");
      break; 
    }
    SSL_free(m_ssl);
    m_ssl = NULL ;
  }
}



static char* m_ssl_error_string[] = {
  "SSL_ERROR_NONE",
  "SSL_ERROR_SSL",
  "SSL_ERROR_WANT_READ",
  "SSL_ERROR_WANT_WRITE",
  "SSL_ERROR_WANT_X509_LOOKUP",
  "SSL_ERROR_SYSCALL",
  "SSL_ERROR_ZERO_RETURN",
  "SSL_ERROR_WANT_CONNECT"
} ;

void C_SecureSocket::ssl_error(int P_returnCode) {
  
  int L_ret ;
  L_ret = SSL_get_error(m_ssl, P_returnCode) ;

  if (P_returnCode <= SSL_ERROR_WANT_CONNECT) {
    SOCKET_ERROR(0, m_ssl_error_string[L_ret]);
  } else {
     SOCKET_ERROR(0, "SSL error " << L_ret);
  }
}


C_SecureSocketListen::C_SecureSocketListen(SSL_CTX * P_ssl_ctx,
                                           T_SocketType  P_type, 
                                           T_pIpAddr     P_addr, 
                                           int           P_channel_id,
                                           size_t        P_read_buf_size,
                                           size_t        P_segm_buf_size)
 : C_SocketListen(P_type, 
                  P_addr, 
                  P_channel_id,
                  P_read_buf_size,
                  P_segm_buf_size) ,
  C_SecureSocket (P_ssl_ctx) {
}

C_SecureSocketListen::~C_SecureSocketListen() {
}

C_Socket* C_SecureSocketListen::create_socket_server(int *P_ret) {
  C_SecureSocketServer* L_socket ;
  NEW_VAR(L_socket, C_SecureSocketServer(m_ssl_ctx, this, m_channel_id, 
                                         m_read_buf_size, m_segm_buf_size));
  (*P_ret) = L_socket->_open(m_buffer_size, m_protocol) ;
  return (L_socket);
}


C_SecureSocketServer::C_SecureSocketServer(SSL_CTX * P_ssl_ctx,
                                           C_SocketListen *P_listen, 
                                           int P_channel_id,
                                           size_t P_read_buf_size,
                                           size_t P_segm_buf_size)
  : C_SocketServer(P_listen, 
                   P_channel_id,
                   P_read_buf_size,
                   P_segm_buf_size),
  C_SecureSocket (P_ssl_ctx) {

}
  
C_SecureSocketServer::C_SecureSocketServer(SSL_CTX * P_ssl_ctx,
                                           T_SocketType  P_type, 
                                           T_pIpAddr     P_addr, 
                                           int           P_channel_id,
                                           size_t        P_read_buf_size,
                                           size_t        P_segm_buf_size)
: C_SocketServer(P_type, 
                 P_addr, 
                 P_channel_id,
                 P_read_buf_size,
                 P_segm_buf_size),
  C_SecureSocket (P_ssl_ctx) {
}
  
  
C_SecureSocketServer::~C_SecureSocketServer() {
}

int C_SecureSocketServer::_call_read(int P_socket, 
                                     char *P_buf, 
                                     size_t P_size) {

  int L_result = 0 ;
  //if (m_ssl->rwstate != SSL_NOTHING) {
    L_result = SSL_read(m_ssl, P_buf, P_size);
    if (L_result < 0) ssl_error(L_result) ;

    //}
  return (L_result) ;
}

int C_SecureSocketServer::_open(size_t P_buffer_size, 
			  C_ProtocolBinaryFrame *P_protocol) {
  int L_ret ;
  int L_result ;
  L_ret = C_SocketServer::_open(P_buffer_size, P_protocol);
  
  m_ssl = SSL_new(m_ssl_ctx);
  SSL_set_accept_state(m_ssl);
  if ((m_bio = BIO_new_socket(m_socket_id, BIO_CLOSE)) == NULL ) {
    SOCKET_ERROR(0, "Unable to create the BIO- in New TLS connection");
  } 
  else {
    SOCKET_ERROR(0, "create the BIO- in New TLS connection");
  }
  
  SSL_set_bio(m_ssl,m_bio,m_bio);
  
  L_result = SSL_accept(m_ssl) ;

  if (L_result < 0 ) {
    if (SSL_get_error(m_ssl, L_result) == SSL_ERROR_WANT_READ) {
      m_state = E_SOCKET_STATE_INPROGESS ;
      //*P_status = E_OPEN_DELAYED ;
      L_ret = 0 ;
    } else {
      ssl_error(L_result);
      L_ret = -1 ;
    }
  } 
  
  return (L_ret);
}

int C_SecureSocketServer::_call_write(int P_socket, 
                                     unsigned char* P_data,
                                     size_t P_size) {
  
  int L_result = 0 ;
  L_result = SSL_write(m_ssl,P_data,P_size);
  if (L_result < 0) ssl_error(L_result) ;
  return (L_result) ;

}

C_SecureSocketClient::C_SecureSocketClient(SSL_CTX * P_ssl_ctx,
                                           T_SocketType  P_type, 
                                           T_pIpAddr     P_addr, 
                                           int           P_channel_id,
                                           size_t        P_read_buf_size,
                                           size_t        P_segm_buf_size)
: C_SocketClient(P_type, 
                 P_addr, 
                 P_channel_id,
                 P_read_buf_size,
                 P_segm_buf_size),
  C_SecureSocket (P_ssl_ctx) {

}

C_SecureSocketClient::~C_SecureSocketClient() {

  
}

int C_SecureSocketClient::_call_read(int P_socket, 
                                     char *P_buf, 
                                     size_t P_size) {

  int L_result = 0 ;

  //if (m_ssl->rwstate != SSL_NOTHING) {
  L_result = SSL_read(m_ssl, P_buf, P_size);
  if (L_result < 0) ssl_error(L_result) ;
  //}

  return (L_result) ;
}

int C_SecureSocketClient::_call_write(int P_socket, 
                             unsigned char* P_data,
                             size_t P_size) {
  int L_result = 0 ;
  
  L_result = SSL_write(m_ssl,P_data,P_size);
  if (L_result < 0) ssl_error(L_result) ;
  
  return (L_result) ;
}

int C_SecureSocketClient::_open (T_pOpenStatus P_status,
                                 size_t        P_buffer_size,
                                 C_ProtocolBinaryFrame *P_protocol) {
  int L_ret ;
  int L_result ;

  L_ret = C_SocketClient::_open(P_status, P_buffer_size, P_protocol);
  m_ssl = SSL_new(m_ssl_ctx);
  SSL_set_connect_state(m_ssl) ;
  
  if ((m_bio = BIO_new_socket(m_socket_id, BIO_CLOSE)) == NULL ) {
    SOCKET_ERROR(0, "Unable to create the BIO- client in New TLS connection");
  } 
  
  SSL_set_bio(m_ssl,m_bio,m_bio);
  
  L_result = SSL_connect(m_ssl) ;
  if ( L_result < 0 ) {
    if (SSL_get_error(m_ssl, L_result) == SSL_ERROR_WANT_READ) {
      L_ret = 0 ;
    } else {
      ssl_error(L_result);
      L_ret = -1 ;
    }
  } 

  return (L_ret);
}

C_Socket* C_SecureSocketClient::process_fd_in_progess (fd_set* P_rSet,
                                                       fd_set* P_wSet,
                                                       C_TransportEvent *P_event) {
  C_Socket* L_socket = NULL ;
  int       L_result        ;

  if (FD_ISSET(m_socket_id, P_rSet)) {
    L_result = SSL_connect(m_ssl) ;
    if ( L_result < 0 ) {
      if (SSL_get_error(m_ssl, L_result) == SSL_ERROR_WANT_READ) {
        P_event->m_type = C_TransportEvent::E_TRANS_NO_EVENT ;
      } else {
        P_event->m_type = C_TransportEvent::E_TRANS_OPEN_FAILED ;
      }
    } else {
      m_state = E_SOCKET_STATE_READY ;
      m_ssl->rwstate = SSL_NOTHING ;
      P_event->m_type = C_TransportEvent::E_TRANS_OPEN ;
    }
    P_event->m_channel_id = m_channel_id ;
    P_event->m_id = m_socket_id ;
  }
  return (L_socket);
}

C_Socket* C_SecureSocketServer::process_fd_in_progess (fd_set* P_rSet,
                                                       fd_set* P_wSet,
                                                       C_TransportEvent *P_event) {
  C_Socket* L_socket = NULL ;
  int       L_result        ;

  if (FD_ISSET(m_socket_id, P_rSet)) {
    L_result = SSL_accept(m_ssl) ;
    if ( L_result < 0 ) {
      if (SSL_get_error(m_ssl, L_result) == SSL_ERROR_WANT_READ) {
        P_event->m_type = C_TransportEvent::E_TRANS_NO_EVENT ;
      } else {
        P_event->m_type = C_TransportEvent::E_TRANS_OPEN_FAILED ;
      }
    } else {
      m_state = E_SOCKET_STATE_READY ;
      P_event->m_type = C_TransportEvent::E_TRANS_OPEN ;
    }
    P_event->m_channel_id = m_channel_id ;
    P_event->m_id = m_socket_id ;
  }
  return (L_socket);
}







