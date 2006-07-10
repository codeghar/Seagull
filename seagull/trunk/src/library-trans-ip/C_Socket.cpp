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

#include "C_Socket.hpp"
#include "Utils.hpp"

#include "iostream_t.hpp"

#include <cerrno>     // for errno
#include <cstring>    // for strerror
#include <unistd.h>   // for close()
#include <fcntl.h>

//#define BUFLEN       1024
//#define DECODEBUFLEN 2048
//#define DECODEBUFLEN 4096

#define SOCKET_ERROR(l,m) iostream_error << m << iostream_endl << iostream_flush
#ifdef DEBUG_MODE
#define SOCKET_DEBUG(l,m) iostream_error << m << iostream_endl << iostream_flush
#else
#define SOCKET_DEBUG(l,m)
#endif

C_Socket::C_Socket(int P_channel_id) {
   SOCKET_DEBUG(1, "C_Socket::C_Socket (" 
		<< P_channel_id 
		<< ") id="
		<< m_socket_id);
   m_socket_id = -1 ;
   m_channel_id = P_channel_id ;
   m_state = E_SOCKET_STATE_NOT_READY ;
   m_decode = NULL ;
   m_recv_msg_list = NULL ;

}

C_Socket::C_Socket(T_SocketType P_type, 
		   int          P_port, 
		   int          P_channel_id) {
  
  SOCKET_DEBUG(1, "C_Socket::C_Socket (" 
	       << P_port << "," << P_channel_id << ") id=" << m_socket_id);

  m_socket_id = -1 ;
  m_type = P_type ;
  m_src_port = P_port ;

  m_channel_id = P_channel_id ;
  m_state = E_SOCKET_STATE_NOT_READY ;
  m_decode = NULL ;
  m_recv_msg_list = NULL ;
}

C_Socket::C_Socket(T_SocketType P_type, 
		   int          P_channel_id) {
  
  SOCKET_DEBUG(1, "C_Socket::C_Socket (" 
	       << P_channel_id << ") id=" << m_socket_id);
  m_socket_id = -1 ;
  m_type = P_type ;
  m_channel_id = P_channel_id ;
  m_state = E_SOCKET_STATE_NOT_READY ;
  m_decode = NULL ;
  m_recv_msg_list = NULL ;
}

C_pDataDecode C_Socket::get_decode() {
  return (m_decode);
}
C_ProtocolBinaryFrame* C_Socket::get_protocol() {
  return (m_protocol);
}
T_pRcvMsgCtxtList C_Socket::get_list() {
  return (m_recv_msg_list);
}


void C_Socket::set_channel_id(int P_channel_id) {
  SOCKET_DEBUG(1, "C_Socket::set_channel_id(" << P_channel_id << ") id=" 
	       << m_socket_id);
  m_channel_id = P_channel_id ;
}

int C_Socket::_open(int              P_socket_domain, 
		    size_t           P_buffer_size,
		    C_ProtocolBinaryFrame *P_protocol) {
  
  int L_socket_type   ; /* socket type */
  int L_socket_domain ; /* socket domain */

  SOCKET_DEBUG(1, "C_Socket::_open ()");

  m_buffer_size = P_buffer_size ;
  L_socket_domain = P_socket_domain ; 

  /* first, determine type of socket to be used */
  switch (m_type) {
  case E_SOCKET_TCP_MODE:
    L_socket_type = SOCK_STREAM ;
    break ;
  case E_SOCKET_UDP_MODE:
    L_socket_type = SOCK_DGRAM ;
    break ;
  default:
    SOCKET_ERROR(0, "Unsupported transport type");
    exit (1);
  }
  
  /* allocate a free socket                 */
  /* Internet address family, Stream socket */
  m_socket_id = call_socket(L_socket_domain, L_socket_type, 0);
  
  SOCKET_DEBUG(1, "m_socket_id [" << m_socket_id << "]");

  if (m_socket_id < 0) {
    perror("socket: allocation failed");
  }

  m_protocol = P_protocol ;
  
  return (0) ;
}

C_Socket::~C_Socket() {
  SOCKET_DEBUG(1, "C_Socket::~C_Socket ()");
}

int C_Socket::get_id () {
  SOCKET_DEBUG(1, "C_Socket::get_id ()");
  return (m_socket_id);
}

int C_Socket::get_channel_id () {
  SOCKET_DEBUG(1, "C_Socket::get_id ()");
  return (m_channel_id);
}


void C_Socket::_close () {
  SOCKET_DEBUG(1, "C_Socket::_close ()");
  call_shutdown(m_socket_id, SHUT_RDWR);
  if (call_close(m_socket_id) != 0) {
    SOCKET_ERROR(0, 
		 "close socket ["
		 << m_socket_id
		 << "] failed ["
		 << errno
		 << "] ["
		 << strerror(errno)
		 << "]");
  }
}


void C_Socket::set_fd_set(fd_set *P_rSet, fd_set *P_wSet) {
  SOCKET_DEBUG(1, "C_Socket::set_fd_set () id=" << m_socket_id);
  switch (m_state) {
  case E_SOCKET_STATE_READY:
    FD_SET(m_socket_id, P_rSet);
    break ;
  case E_SOCKET_STATE_INPROGESS:
    FD_SET(m_socket_id, P_wSet);
    break ;
  case E_SOCKET_STATE_NOT_READY:
    break ;
  }
}

void C_Socket::set_properties() {

  int           L_sock_opt ;
  unsigned int  L_max_buf_size ;
  struct linger L_linger ;
  int           L_flags ;

  if (m_type == E_SOCKET_TCP_MODE) {
    L_sock_opt = 1 ;
    // fast reuse of the socket
    if (call_setsockopt(m_socket_id, 
		       SOL_SOCKET, SO_REUSEADDR, (void *)&L_sock_opt,
                   sizeof (L_sock_opt)) == -1) {
      SOCKET_ERROR(1, "setsockopt(SO_REUSEADDR) failed");
    }
    // no data grouped
    L_sock_opt = 1 ;
    if (call_setsockopt (m_socket_id, 
			IPPROTO_TCP, TCP_NODELAY, (void *)&L_sock_opt,
		    sizeof (L_sock_opt)) == -1) {
      SOCKET_ERROR(1, "setsockopt(TCP_NODELAY) failed");
    }
    // max wait time after a close
    L_linger.l_onoff = 1;
    L_linger.l_linger = 0;
    if (call_setsockopt (m_socket_id, SOL_SOCKET, SO_LINGER,
		    &L_linger, sizeof (L_linger)) < 0) {
      SOCKET_ERROR(1, "Unable to set SO_LINGER option");
    }
  }

  // size of recv buf
  L_max_buf_size = m_buffer_size ;
  if(call_setsockopt(m_socket_id, SOL_SOCKET, SO_SNDBUF,
		    &L_max_buf_size, sizeof(L_max_buf_size))) {
    SOCKET_ERROR(1,"Unable to set socket sndbuf");
  }

  // size of send buff
  L_max_buf_size = m_buffer_size ;
  if(call_setsockopt(m_socket_id, SOL_SOCKET, SO_RCVBUF,
		    &L_max_buf_size, sizeof(L_max_buf_size))) {
    SOCKET_ERROR(1, "Unable to set socket rcvbuf");
  }

  // non blocking mode (for connect only ?)
  L_flags = call_fcntl(m_socket_id, F_GETFL , NULL);
  L_flags |= O_NONBLOCK;
  call_fcntl(m_socket_id, F_SETFL , L_flags);

}

C_SocketListen::C_SocketListen(T_SocketType P_type, 
			       T_pIpAddr    P_addr, 
			       int          P_channel_id,
                               size_t       P_read_buf_size,
                               size_t       P_segm_buf_size)   
  : C_Socket(P_type, P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketListen::C_SocketListen() id=" << m_socket_id);
  m_source_addr_info = P_addr ;
  m_read_buf_size = P_read_buf_size ;
  m_segm_buf_size = P_segm_buf_size ;
}


C_SocketListen::~C_SocketListen() {
  SOCKET_DEBUG(0, "C_SocketListen::~C_SocketListen() id=" << m_socket_id);
}

size_t C_SocketListen::received_buffer (unsigned char  *P_data, 
					size_t          P_size_buf,
					struct timeval *P_time) {
  SOCKET_DEBUG(0, "C_SocketListen::received_buffer() id=" << m_socket_id);
  return (0); 
}

int C_SocketListen::_open (size_t P_buffer_size, 
			   C_ProtocolBinaryFrame *P_protocol) {

   int		     L_rc          ; /* system calls return value storage */

   SOCKET_DEBUG(0, "C_SocketListen::_open()");

   L_rc = C_Socket::_open(get_domain(m_source_addr_info), 
			  P_buffer_size,
			  P_protocol) ;

   if (L_rc == 0) {
  
     set_properties() ;

     /* bind the socket to the newly formed address */
     L_rc = call_bind(m_socket_id, 
		 (sockaddr *)(void *)&(m_source_addr_info->m_addr),
		 SOCKADDR_IN_SIZE(&(m_source_addr_info->m_addr)));
   /* check there was no error */
     if (L_rc) {
       SOCKET_ERROR(1, "bind [" << strerror(errno) << "]");
     } else {
       
       if (m_type == E_SOCKET_TCP_MODE) {
	 /* ask the system to listen for incoming connections	*/
	 /* to the address we just bound. specify that up to		*/
	 /* 5 pending connection requests will be queued by the	*/
	 /* system, if we are not directly awaiting them using	*/
	 /* the accept() system call, when they arrive.		*/
	 L_rc = call_listen(m_socket_id, 5);
	 
	 /* check there was no error */
	 if (L_rc) {
	   SOCKET_ERROR(1, "listen [" << strerror(errno) << "]");
	 }
       }

       m_state = E_SOCKET_STATE_READY ;
     }
   }

   return (L_rc);
  
}

C_Socket* C_SocketListen::process_fd_set (fd_set           *P_rSet, 
					  fd_set           *P_wSet,
					  C_TransportEvent *P_event) {

  C_SocketServer  *L_socket = NULL ;
  int              L_ret ;
  
  SOCKET_DEBUG(0, "C_SocketListen::process_fd_set() id=" << m_socket_id);

  if (FD_ISSET(m_socket_id, P_rSet)) {
    NEW_VAR(L_socket, C_SocketServer(this, m_channel_id, 
                                     m_read_buf_size, m_segm_buf_size));
    L_ret = L_socket->_open(m_buffer_size, m_protocol) ;
    if (L_ret == 0) {
      SOCKET_DEBUG(0, "C_SocketWithData::process_fd_set() CNX");
      P_event->m_type = C_TransportEvent::E_TRANS_CONNECTION ;
      P_event->m_channel_id = m_channel_id ;
      P_event->m_id = L_socket->get_id() ;
    } else {
      SOCKET_DEBUG(0, "C_SocketWithData::process_fd_set() NOEVENT open failed");
      DELETE_VAR(L_socket);
      P_event->m_type = C_TransportEvent::E_TRANS_NO_EVENT ;
    }
  } else {
    SOCKET_DEBUG(0, "C_SocketWithData::process_fd_set() NOEVENT");
    P_event->m_type = C_TransportEvent::E_TRANS_NO_EVENT ;
  }
  
  return (L_socket);
}

T_SockAddrStorage* C_SocketListen::get_source_address() {
  return(&(m_source_addr_info->m_addr));
}

// UDP
T_SocketType C_SocketListen::get_trans_type() {
  return(m_type);
}
// UDP

C_SocketServer::C_SocketServer(C_SocketListen *P_listen, 
			       int P_channel_id,
                               size_t P_read_buf_size,
                               size_t P_segm_buf_size)
  : C_SocketWithData(P_channel_id, P_read_buf_size, P_segm_buf_size) {
  SOCKET_DEBUG(0, "C_SocketServer::C_SocketServer() id=" << m_socket_id);
  m_listen_sock = P_listen ;
  // UDP 
  m_type = m_listen_sock->get_trans_type();
  if (m_type == E_SOCKET_UDP_MODE) {
    // std::cerr << "m_socket_id before " << m_socket_id << std::endl;
    m_socket_id = m_listen_sock->get_id();
    // std::cerr << "m_socket_id " << m_socket_id << std::endl;
  }
  // UDP
  // std::cerr << "m_type " << m_type << std::endl;
}

C_SocketServer::~C_SocketServer() {
  SOCKET_DEBUG(0, "C_SocketServer::~C_SocketServer() id=" << m_socket_id);
}

int C_SocketServer::_open(size_t P_buffer_size, 
			  C_ProtocolBinaryFrame *P_protocol) {

  tool_socklen_t L_size   ;
  int            L_rc = 0 ;

  SOCKET_DEBUG(0, "C_SocketServer::_open()");

  // UDP
  if (m_type == E_SOCKET_TCP_MODE) {

  L_size = SOCKADDR_IN_SIZE(m_listen_sock->get_source_address());
  memset(&m_accepted_addr, 0, L_size);

  m_socket_id = call_accept (m_listen_sock->get_id(), 
			     (sockaddr *)(void *)&m_accepted_addr,
			     &L_size);
  
  } // UDP

  m_protocol = P_protocol ;

  m_buffer_size = P_buffer_size ;

  set_properties () ;

  if (m_socket_id < 0) {
    SOCKET_ERROR(0, "accept failed for listen [" << m_listen_sock->get_id()
		 << "] [" << strerror(errno) << "]");
    L_rc = -1 ;
  } else {
    m_state = E_SOCKET_STATE_READY ;
  }

  return (L_rc);
}

C_SocketClient::C_SocketClient(T_SocketType P_type, 
			       T_pIpAddr    P_addr, 
			       int          P_channel_id,
                               size_t       P_read_buf_size,
                               size_t       P_segm_buf_size)
  : C_SocketWithData(P_type, P_addr, P_channel_id, 
                     P_read_buf_size, P_segm_buf_size) {
  SOCKET_DEBUG(0, "C_SocketClient::C_SocketClient() id=" << m_socket_id);
}

C_SocketClient::~C_SocketClient() {
  SOCKET_DEBUG(0, "C_SocketClient::~C_SocketClient() id=" << m_socket_id);
}

int C_SocketClient::_open(T_pOpenStatus  P_status,
			  size_t         P_buffer_size,
			  C_ProtocolBinaryFrame *P_protocol) {

  int             L_rc = 0 ;
  
  SOCKET_DEBUG(0, "C_SocketClient::_open()");
  L_rc = C_Socket::_open(get_domain(m_remote_addr_info), 
			 P_buffer_size,
			 P_protocol) ;

  if (L_rc == 0) {
    set_properties() ;
    // UDP
  if (m_type == E_SOCKET_TCP_MODE) {

    L_rc = call_connect (m_socket_id, 
		    (struct sockaddr*)(void*)&(m_remote_addr_info->m_addr),
		    SOCKADDR_IN_SIZE(&(m_remote_addr_info->m_addr))) ;
    if (L_rc) {
      if (errno != EINPROGRESS) {
	SOCKET_ERROR(1, "connect failed [" 
		     << L_rc << "][" 
		     << strerror(errno) << "]");
	*P_status = E_OPEN_FAILED ;
      } else {
	m_state = E_SOCKET_STATE_INPROGESS ;
	*P_status = E_OPEN_DELAYED ;
	L_rc = 0 ;
      }
      
    } else {
      m_state = E_SOCKET_STATE_READY ;
      *P_status = E_OPEN_OK ;
    }
  } else {
    
    // UDP
    // bind
       m_state = E_SOCKET_STATE_READY ;
       *P_status = E_OPEN_OK ;
  }
  // UDP

  }
  return (L_rc);
}

C_SocketWithData::C_SocketWithData(int P_channel_id,
                                   size_t P_read_buf_size,
                                   size_t P_segm_buf_size) 
  : C_Socket (P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketWithData::C_SocketWithData() id=" << m_socket_id);
  m_data_queue.clear();
  NEW_VAR(m_decode, C_DataDecode(P_segm_buf_size));
  NEW_VAR(m_recv_msg_list, T_RcvMsgCtxtList());
  m_recv_msg_list->clear();

  m_read_buf_size = P_read_buf_size ;
  ALLOC_TABLE(m_read_buf, char*, sizeof(char), m_read_buf_size);

}

C_SocketWithData::C_SocketWithData(T_SocketType P_type,
				   T_pIpAddr    P_addr,
				   int          P_channel_id,
                                   size_t       P_read_buf_size,
                                   size_t       P_segm_buf_size)
  : C_Socket (P_type, P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketWithData::C_SocketWithData() id=" << m_socket_id);
  m_data_queue.clear();
  m_remote_addr_info = P_addr;

  NEW_VAR(m_decode, C_DataDecode(P_segm_buf_size));
  NEW_VAR(m_recv_msg_list, T_RcvMsgCtxtList());
  m_recv_msg_list->clear();

  m_read_buf_size = P_read_buf_size ;
  ALLOC_TABLE(m_read_buf, char*, sizeof(char), m_read_buf_size);

}


C_SocketWithData::~C_SocketWithData() {
  SOCKET_DEBUG(0, "C_SocketWithData::~C_SocketWithData()");
  if (!m_data_queue.empty()) {
    m_data_queue.erase(m_data_queue.begin(), m_data_queue.end());
  }
  DELETE_VAR(m_decode);
  if (!m_recv_msg_list->empty()) {
    // to do delete message
    m_recv_msg_list->erase(m_recv_msg_list->begin(),
			   m_recv_msg_list->end());
  }
  DELETE_VAR(m_recv_msg_list);

  m_read_buf_size = 0 ;
  FREE_TABLE(m_read_buf);
}

C_Socket* C_SocketWithData::process_fd_set (fd_set* P_rSet,
					    fd_set* P_wSet,
					    C_TransportEvent *P_event) {
  int             L_rc ;
  T_pDataRcv      L_data ;
  int             L_sock_opt ;
  socklen_t       L_opt_len ;
  struct timezone L_timeZone ;             

  char           *L_buf = m_read_buf ;

  SOCKET_DEBUG(0, "C_SocketWithData::process_fd_set() id=" << m_socket_id);


  switch (m_state) {

  case E_SOCKET_STATE_READY:

    if (FD_ISSET(m_socket_id, P_rSet)) {
      L_rc = call_read(m_socket_id, L_buf, m_read_buf_size);
      // UDP
      if (L_rc <= 0) {
	SOCKET_DEBUG(0, "C_SocketWithData::process_fd_set() CLOSED");
	P_event->m_type = C_TransportEvent::E_TRANS_CLOSED ;
	P_event->m_channel_id = m_channel_id ;
	P_event->m_id = m_socket_id ;
      } else {
	SOCKET_DEBUG(0, "C_SocketWithData::process_fd_set() RCV " << L_rc);

	ALLOC_VAR(L_data, T_pDataRcv, sizeof(T_DataRcv));
	gettimeofday(&(L_data->m_time), &L_timeZone);

	P_event->m_type = C_TransportEvent::E_TRANS_RECEIVED ;
	P_event->m_channel_id = m_channel_id ;
	P_event->m_id = m_socket_id ;
	L_data->m_size = L_rc ;
	ALLOC_TABLE(L_data->m_data, 
		    unsigned char*, 
		    sizeof(unsigned char*), 
		    L_rc);
	memcpy(L_data->m_data, L_buf, L_rc);
	m_data_queue.push_back(L_data);
      }
    } else {
      SOCKET_DEBUG(0, "C_SocketWithData::process_fd_set() NOEVENT");
      P_event->m_type = C_TransportEvent::E_TRANS_NO_EVENT ;
    }
    break ;

  case E_SOCKET_STATE_INPROGESS:
    L_opt_len = sizeof (L_sock_opt);

    if (FD_ISSET(m_socket_id, P_wSet)) {
      if (getsockopt(m_socket_id, SOL_SOCKET, SO_ERROR, (void *)&L_sock_opt,
		     &L_opt_len) == -1) {
	SOCKET_ERROR(1, "getsockopt(SO_ERROR) failed");
      }
      if (L_sock_opt == 0) {
	m_state = E_SOCKET_STATE_READY ;
	P_event->m_type = C_TransportEvent::E_TRANS_OPEN ;
	P_event->m_channel_id = m_channel_id ;
	P_event->m_id = m_socket_id ;

      } else {
	P_event->m_type = C_TransportEvent::E_TRANS_OPEN_FAILED ;
	P_event->m_channel_id = m_channel_id ;
	P_event->m_id = m_socket_id ;
	SOCKET_ERROR(1, "delayed operation error for ["
		     << m_socket_id << "]");
      }
    }
    break ;

  case E_SOCKET_STATE_NOT_READY:
    break ;

  }

  return (NULL);
}

size_t C_SocketWithData::received_buffer(unsigned char *P_data, 
					 size_t          P_size_buf,
					 struct timeval *P_time) {

  size_t                  L_size = 0 ;
  T_DataRcvList::iterator L_it ;
  T_pDataRcv              L_data ;
  
  SOCKET_DEBUG(0, "C_SocketWithData::received_buffer() id=" << m_socket_id);

  if (!m_data_queue.empty()) {
    L_it = m_data_queue.begin();
    L_data = *L_it ;
    m_data_queue.erase(L_it);
    L_size = L_data->m_size ;
    SOCKET_DEBUG(0, "C_SocketWithData::received_buffer() size=" << L_size);
    memcpy(P_data, L_data->m_data, L_size);
    //    memmove(P_data, L_data->m_data, L_size);
    //    SOCKET_DEBUG(0, "C_SocketWithData::received_buffer() L_buf=" 
    //		 << L_data->m_data);
    *P_time = L_data->m_time ;
    FREE_TABLE(L_data->m_data);
    FREE_VAR(L_data) ;
  }

  SOCKET_DEBUG(0, "C_SocketWithData::received_buffer() return=" 
	       << L_size);
  return (L_size);
}

