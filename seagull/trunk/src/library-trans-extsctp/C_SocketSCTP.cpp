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
#include "C_SocketSCTP.hpp"
#include "Utils.hpp"

#include "iostream_t.hpp"

#include <cerrno>     // for errno
#include <cstring>    // for strerror
#include <unistd.h>   // for close()
#include <fcntl.h>

#define BUFLEN       1024
#define DECODEBUFLEN 2048
#define MSGFLAG      0
#define MAX_OUTGOING 128
#define MAX_INCOMING 128

#define SOCKET_ERROR(l,m) iostream_error << m << iostream_endl << iostream_flush

#ifdef DEBUG_MODE
#define SOCKET_DEBUG(l,m) iostream_error << m << iostream_endl << iostream_flush
#else
#define SOCKET_DEBUG(l,m)
#endif

C_SocketSCTP::C_SocketSCTP(int P_channel_id) {
   SOCKET_DEBUG(1, "C_SocketSCTP::C_SocketSCTP (" 
		<< P_channel_id 
		<< ") id="
		<< m_socket_id);
   m_socket_id = -1 ;
   m_channel_id = P_channel_id ;
   m_state = E_SOCKET_STATE_NOT_READY ;
   m_decode = NULL ;
   m_recv_msg_list = NULL ;
   m_recv_buflen = BUFLEN ;
   ALLOC_TABLE(m_recv_buf, char*, sizeof(char), BUFLEN);
}

C_SocketSCTP::C_SocketSCTP(T_SocketType P_type, 
		   int          P_port, 
		   int          P_channel_id) {
  
  SOCKET_DEBUG(1, "C_SocketSCTP::C_SocketSCTP (" 
	       << P_port << "," << P_channel_id << ") id=" << m_socket_id);

  m_socket_id = -1 ;
  m_type = P_type ;
  m_src_port = P_port ;
  m_channel_id = P_channel_id ;
  m_state = E_SOCKET_STATE_NOT_READY ;
  m_recv_buflen = BUFLEN ;
  ALLOC_TABLE(m_recv_buf, char*, sizeof(char), BUFLEN);
  m_decode = NULL ;
  m_recv_msg_list = NULL ;
}

C_SocketSCTP::C_SocketSCTP(T_SocketType P_type, 
		   int          P_channel_id) {
  
  SOCKET_DEBUG(1, "C_SocketSCTP::C_SocketSCTP (" 
	       << P_channel_id << ") id=" << m_socket_id);
  m_socket_id = -1 ;
  m_type = P_type ;
  m_channel_id = P_channel_id ;
  m_state = E_SOCKET_STATE_NOT_READY ;
  m_recv_buflen = BUFLEN ;
  ALLOC_TABLE(m_recv_buf, char*, sizeof(char), BUFLEN);
  m_decode = NULL ;
  m_recv_msg_list = NULL ;
}

C_pSCTPDataDecode C_SocketSCTP::get_decode() {
  return (m_decode);
}
C_ProtocolBinaryFrame* C_SocketSCTP::get_protocol() {
  return (m_protocol);
}
T_pRcvMsgCtxtList C_SocketSCTP::get_list() {
  return (m_recv_msg_list);
}

void C_SocketSCTP::set_channel_id(int P_channel_id) {
  SOCKET_DEBUG(1, "C_SocketSCTP::set_channel_id(" << P_channel_id << ") id=" 
	       << m_socket_id);
  m_channel_id = P_channel_id ;
}

int C_SocketSCTP::_open(int                    P_socket_domain, 
			size_t                 P_buffer_size,
			C_ProtocolBinaryFrame *P_protocol) {
  
  int L_socketSCTP_type   ; /* socket type */
  int L_socket_domain ; /* socket domain */

  SOCKET_DEBUG(1, "C_SocketSCTP::_open ()");

  m_buffer_size = P_buffer_size ;
  L_socket_domain = P_socket_domain ; 

  /* first, determine type of socket to be used */
  switch (m_type) {
  case E_SOCKET_TCP_MODE:
    L_socketSCTP_type = SOCK_STREAM ;
    break ;
  case E_SOCKET_UDP_MODE:
    L_socketSCTP_type = SOCK_DGRAM ;
    break ;
  default:
    SOCKET_ERROR(0, "Unsupported transport type");
    exit (1);
  }
  
  /* allocate a free socket                 */
  /* Internet address family, Stream socket */
  m_socket_id = call_socket(L_socket_domain, L_socketSCTP_type, IPPROTO_SCTP);
  
  SOCKET_DEBUG(1, "m_socket_id [" << m_socket_id << "]");

  if (m_socket_id < 0) {
    perror("allocation failed");
  } else { 
    set_properties() ;
  } 
  
  m_protocol = P_protocol ;

  return (0) ;
}

C_SocketSCTP::~C_SocketSCTP() {
  FREE_TABLE(m_recv_buf);
  SOCKET_DEBUG(1, "C_SocketSCTP::~C_SocketSCTP ()");
}

int C_SocketSCTP::get_id () {
  SOCKET_DEBUG(1, "C_SocketSCTP::get_id ()");
  return (m_socket_id);
}

int C_SocketSCTP::get_channel_id () {
  SOCKET_DEBUG(1, "C_SocketSCTP::get_channel_id ()");
  return (m_channel_id);
}

void C_SocketSCTP::_close () {
  SOCKET_DEBUG(1, "C_SocketSCTP::_close ()");
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


void C_SocketSCTP::set_fd_set(fd_set *P_rSet, fd_set *P_wSet) {
  SOCKET_DEBUG(1, "C_SocketSCTP::set_fd_set () id=" << m_socket_id);
  switch (m_state) {
  case E_SOCKET_STATE_READY:
    FD_SET(m_socket_id, P_rSet);
    break ;
  case E_SOCKET_STATE_INPROGESS:
    FD_SET(m_socket_id, P_rSet);
    // FD_SET(m_socket_id, P_wSet); CONFIRM ?
    break ;
  case E_SOCKET_STATE_NOT_READY:
    break ;
  }
}

void C_SocketSCTP::set_properties() {

  //  int           L_sock_opt ;
  unsigned int  L_max_buf_size ;
  struct linger L_linger ;
  int           L_flags ;

  SOCKET_DEBUG(1, "C_SocketSCTP::set_properties()");
  if (m_type == E_SOCKET_TCP_MODE) {
    // max wait time after a close
    L_linger.l_onoff = 1;
    L_linger.l_linger = 0;
    if (call_setsockopt (m_socket_id, SOL_SOCKET, SO_LINGER,
		    &L_linger, sizeof (L_linger)) < 0) {
      SOCKET_ERROR(1, "Unable to set SO_LINGER option");
    }
  }

  // SCTP BEGIN
  {
    struct sctp_initmsg         init;
    sctp_event_subscribe        events;


    init.sinit_num_ostreams   = MAX_OUTGOING;
    init.sinit_max_instreams  = MAX_INCOMING;
    init.sinit_max_attempts   = 3;
    init.sinit_max_init_timeo = 30;
    if(call_setsockopt(m_socket_id, IPPROTO_SCTP, 
		       SCTP_INITMSG, (void*)&init, sizeof(init)) < 0) {
      SOCKET_ERROR(1, "Unable to set SCTP_INITMSG option");
    }

    events.sctp_data_io_event = 1;
    events.sctp_association_event = 1;
    events.sctp_address_event = 1;
    events.sctp_send_failure_event = 1;
    events.sctp_peer_error_event = 1;
    events.sctp_shutdown_event = 1;
    events.sctp_partial_delivery_event = 1;
    events.sctp_adaption_layer_event = 1;

    if(call_setsockopt(m_socket_id, IPPROTO_SCTP,
		       SCTP_EVENTS,&events,sizeof(events)) < 0) {
      SOCKET_ERROR(1, "Unable to set SCTP_EVENTS option");
    }

  // SCTP END
  
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

C_SocketSCTPListen::C_SocketSCTPListen(T_SocketType P_type, 
				       T_pIpAddr    P_addr, 
				       int          P_channel_id)   
  : C_SocketSCTP(P_type, P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketSCTPListen::C_SocketSCTPListen() id=" << m_socket_id);
  m_source_addr_info = P_addr ;
}


C_SocketSCTPListen::~C_SocketSCTPListen() {
  SOCKET_DEBUG(0, "C_SocketSCTPListen::~C_SocketSCTPListen() id=" << m_socket_id);
}

size_t C_SocketSCTPListen::received_buffer (unsigned char  *P_data, 
					    size_t          P_size_buf,
					    struct timeval *P_time) {
  SOCKET_DEBUG(0, "C_SocketSCTPListen::received_buffer() id=" << m_socket_id);
  return (0); 
}

int C_SocketSCTPListen::_open (size_t                 P_buffer_size, 
			       C_ProtocolBinaryFrame *P_protocol) {

   int		     L_rc          ; /* system calls return value storage */

   SOCKET_DEBUG(0, "C_SocketSCTPListen::_open()");

   L_rc = C_SocketSCTP::_open(get_domain(m_source_addr_info), 
			  P_buffer_size,
			  P_protocol) ;

   if (L_rc == 0) {
  
     //     set_properties() ;

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

C_SocketSCTP* C_SocketSCTPListen::process_fd_set (fd_set           *P_rSet, 
						  fd_set           *P_wSet,
						  C_TransportEvent *P_event) {

  C_SocketSCTPServer  *L_socket = NULL ;
  int                  L_ret ;
  
  SOCKET_DEBUG(0, "C_SocketSCTPListen::process_fd_set() id=" << m_socket_id);


  if (FD_ISSET(m_socket_id, P_rSet)) {
    NEW_VAR(L_socket, C_SocketSCTPServer(this, m_channel_id));
    L_ret = L_socket->_open(m_buffer_size, m_protocol) ;
    if (L_ret == 0) {
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::process_fd_set() CNX");
      P_event->m_type = C_TransportEvent::E_TRANS_CONNECTION ;
      P_event->m_channel_id = m_channel_id ;
      P_event->m_id = L_socket->get_id() ;
    } else {
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::process_fd_set() NOEVENT open failed");
      DELETE_VAR(L_socket);
      P_event->m_type = C_TransportEvent::E_TRANS_NO_EVENT ;
    }
  } else {
    SOCKET_DEBUG(0, "C_SocketSCTPWithData::process_fd_set() NOEVENT");
    P_event->m_type = C_TransportEvent::E_TRANS_NO_EVENT ;
  }
  
  return (L_socket);
}

T_SockAddrStorage* C_SocketSCTPListen::get_source_address() {
  return(&(m_source_addr_info->m_addr));
}

C_SocketSCTPServer::C_SocketSCTPServer(C_SocketSCTPListen *P_listen, 
				       int                 P_channel_id)
  : C_SocketSCTPWithData(P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketSCTPServer::C_SocketSCTPServer() id=" << m_socket_id);
  m_listen_sock = P_listen ;
}

C_SocketSCTPServer::~C_SocketSCTPServer() {
  SOCKET_DEBUG(0, "C_SocketSCTPServer::~C_SocketSCTPServer() id=" << m_socket_id);
}

int C_SocketSCTPServer::_open(size_t                 P_buffer_size,
			      C_ProtocolBinaryFrame *P_protocol) {

  tool_socklen_t L_size   ;
  int            L_rc = 0 ;

  SOCKET_DEBUG(0, "C_SocketSCTPServer::_open()");

  L_size = SOCKADDR_IN_SIZE(m_listen_sock->get_source_address());
  memset(&m_accepted_addr, 0, L_size);

  m_socket_id = call_accept (m_listen_sock->get_id(), 
			(sockaddr *)(void *)&m_accepted_addr,
			&L_size);

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

C_SocketSCTPClient::C_SocketSCTPClient(T_SocketType P_type, 
			       T_pIpAddr    P_addr, 
			       int          P_channel_id)
  : C_SocketSCTPWithData(P_type, P_addr, P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketSCTPClient::C_SocketSCTPClient() id=" << m_socket_id);
}

C_SocketSCTPClient::~C_SocketSCTPClient() {
  SOCKET_DEBUG(0, "C_SocketSCTPClient::~C_SocketSCTPClient() id=" << m_socket_id);
}

int C_SocketSCTPClient::_open(T_pOpenStatus          P_status,
			      size_t                 P_buffer_size,
			      C_ProtocolBinaryFrame *P_protocol) {

  int             L_rc = 0 ;
  
  SOCKET_DEBUG(0, "C_SocketSCTPClient::_open()");
  L_rc = C_SocketSCTP::_open(get_domain(m_remote_addr_info), 
			     P_buffer_size,
			     P_protocol) ;

  if (L_rc == 0) {
    //    set_properties() ;
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
  }
  return (L_rc);
}

C_SocketSCTPWithData::C_SocketSCTPWithData(int P_channel_id)
  : C_SocketSCTP (P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketSCTPWithData::C_SocketSCTPWithData() id=" << m_socket_id);
  m_data_queue.clear();
  m_data_recv = false ;
  NEW_VAR(m_decode, C_SCTPDataDecode(DECODEBUFLEN));
  NEW_VAR(m_recv_msg_list, T_RcvMsgCtxtList());
  m_recv_msg_list->clear();
}

C_SocketSCTPWithData::C_SocketSCTPWithData(T_SocketType P_type,
					   T_pIpAddr    P_addr,
					   int          P_channel_id)
  : C_SocketSCTP (P_type, P_channel_id) {
  SOCKET_DEBUG(0, "C_SocketSCTPWithData::C_SocketSCTPWithData() id=" << m_socket_id);
  m_data_queue.clear();
  m_remote_addr_info = P_addr;
  NEW_VAR(m_decode, C_SCTPDataDecode(DECODEBUFLEN));
  NEW_VAR(m_recv_msg_list, T_RcvMsgCtxtList());
  m_recv_msg_list->clear();
}


C_SocketSCTPWithData::~C_SocketSCTPWithData() {
  SOCKET_DEBUG(0, "C_SocketSCTPWithData::~C_SocketSCTPWithData()");
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
}

void C_SocketSCTPWithData::sctp_event_handler (C_TransportEvent *P_event) {

  struct sctp_assoc_change *sac;
  struct sctp_send_failed  *ssf;
  struct sctp_paddr_change *spc;
  struct sctp_remote_error *sre;
  union  sctp_notification *snp;
  //  char   addrbuf[INET6_ADDRSTRLEN];
  //  const  char *ap;
  struct sockaddr_in *sin;
  struct sockaddr_in6 *sin6;

  snp = (union sctp_notification *)m_recv_buf;

  switch (snp->sn_header.sn_type) {

    case SCTP_ASSOC_CHANGE:
      sac = &snp->sn_assoc_change;
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler()" <<
		   "EVENT assoc_change: state="
		   << sac->sac_state
		   <<", error="
		   << sac->sac_error
		   <<", instr="
		   << sac->sac_inbound_streams
		   <<", outstr="
		   << sac->sac_outbound_streams);

      switch(sac->sac_state) {
      case SCTP_COMM_UP:
	SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		     "ASSOC EVENT: COMMUNICATION UP");

	switch (m_state) {
	case E_SOCKET_STATE_INPROGESS:
	  m_state = E_SOCKET_STATE_READY ;
	  P_event->m_type = C_TransportEvent::E_TRANS_OPEN ;
	  P_event->m_channel_id = m_channel_id ;
	  P_event->m_id = m_socket_id ;
	  break ;
	default:
	  break ;
	}
	break;

      case SCTP_COMM_LOST:
	SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		     "ASSOC EVENT: COMMUNICATION LOST");
	switch (m_state) {

	case E_SOCKET_STATE_READY:
	  P_event->m_type = C_TransportEvent::E_TRANS_CLOSED ;
	  P_event->m_channel_id = m_channel_id ;
	  P_event->m_id = m_socket_id ;
	  break ;

	case E_SOCKET_STATE_INPROGESS:
	  P_event->m_type = C_TransportEvent::E_TRANS_OPEN_FAILED ;
	  P_event->m_channel_id = m_channel_id ;
	  P_event->m_id = m_socket_id ;
	  break;

	default:
	  break ;
	}
	break ;
      case SCTP_RESTART:
	SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		     "ASSOC EVENT: RESTART");
	break;
      case SCTP_SHUTDOWN_COMP:
	SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		     "ASSOC EVENT: SHUTDOWN COMPLETE");
	break;
      case SCTP_CANT_STR_ASSOC:
	SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		     "ASSOC EVENT: CANT START ASSOC "
		     << (uint32_t)sac->sac_assoc_id);
	exit(0);
	break;
      default:
	SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		     "ASSOC EVENT: UNKNOWN");
	break ;
      } 

      break;

  case SCTP_SEND_FAILED:
    ssf = &snp->sn_send_failed;
    SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " << 
		 "EVENT sendfailed: len="
		 << ssf->ssf_length
		 << " err=" << ssf->ssf_error);
    break;

  case SCTP_PEER_ADDR_CHANGE:
    spc = &snp->sn_paddr_change;

    switch(spc->spc_state) {
    case SCTP_ADDR_REACHABLE:
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " << 
		   "ASSOC PEER: ADDRESS REACHABLE");
      break;
    case SCTP_ADDR_UNREACHABLE:
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		   "ASSOC PEER: ADDRESS UNAVAILABLE");
      break;
    case SCTP_ADDR_REMOVED:
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		   "ASSOC PEER: ADDRESS REMOVED");
      break;
    case SCTP_ADDR_ADDED:
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		   "ASSOC PEER: ADDRESS ADDED");
      break;
    case SCTP_ADDR_MADE_PRIM:
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		   "ASSOC PEER: ADDRESS MADE PRIMARY");
      break;
    case SCTP_ADDR_CONFIRMED:
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		   "ASSOC PEER: ADDRESS CONFIRMED");
      break;
    default:
      SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		   "ASSOC PEER: UNKNOWN");
      break ;
    } /* end switch */
    
    if (spc->spc_aaddr.ss_family == AF_INET) {
      sin = (struct sockaddr_in *)&spc->spc_aaddr;
      //    ap = inet_ntop(AF_INET, &sin->sin_addr, addrbuf, INET6_ADDRSTRLEN);
    } else {
      sin6 = (struct sockaddr_in6 *)&spc->spc_aaddr;
      //      ap = inet_ntop(AF_INET6, &sin6->sin6_addr, addrbuf, INET6_ADDRSTRLEN);
    }
    //    printf("EVENT intf_change: %s state=%d, error=%d\n", ap, spc->spc_state, spc->spc_error);
    //    printf("EVENT intf_change: state=%d, error=%d\n", spc->spc_state, spc->spc_error);
    break;

  case SCTP_REMOTE_ERROR:
    sre = &snp->sn_remote_error;
    //    printf("EVENT: remote_error: err=%hu len=%hu\n", ntohs(sre->sre_error), ntohs(sre->sre_length));
    break;

  case SCTP_ADAPTION_INDICATION:
    SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
		 "EVENT: ADAPTATION INDICATION");
    break;
    
//   case SCTP_PARTIAL_DELIVERY_EVENT: {
//     printf("EVENT: PARTIAL DELIVERY\n");
//     struct sctp_pdapi_event *pdapi;
//     pdapi = &snp->sn_pdapi_event;
//     printf("SCTP_PD-API event:%u\n",
// 	   pdapi->pdapi_indication);
//     if(pdapi->pdapi_indication == SCTP_PARTIAL_DELIVERY_ABORTED){
//       printf("PDI- Aborted\n");
//     }
//   }
//     break;

  case SCTP_SHUTDOWN_EVENT:
    SOCKET_DEBUG(0, 
		 "C_SocketSCTPWithData::sctp_event_handler() EVENT shutdown");
    break;
    
  default:
    SOCKET_DEBUG(0,
		 "C_SocketSCTPWithData::sctp_event_handler() EVENT unknown "
		 << snp->sn_header.sn_type);
    break;
  }

}

bool C_SocketSCTPWithData::sctp_recv_msg (struct msghdr *msg, 
					  ssize_t *nrp, size_t cmsglen) {

  ssize_t nr = 0, nnr = 0;
  struct iovec iov[1];

  iov->iov_base = m_recv_buf;
  iov->iov_len = m_recv_buflen ;
  msg->msg_iov = iov;
  msg->msg_iovlen = 1;

  while(true) {

    msg->msg_flags = MSGFLAG ; 
    msg->msg_controllen = cmsglen;
    
    nnr = call_recvmsg(m_socket_id, msg, 0);

    if (nnr <= 0) {
      /* EOF or error */
      *nrp = nr;
      return (false);
    }
    
    nr += nnr;

    if ((msg->msg_flags & MSG_EOR) != 0) {
      *nrp = nr;
      return (true);
    }

    /* Realloc the buffer? */
    if (m_recv_buflen == (size_t)nr) {
      m_recv_buflen *= 2;
      REALLOC_VAR(m_recv_buf, char*, m_recv_buflen);
    }

    /* Set the next read offset */
    iov->iov_base = m_recv_buf + nr;
    iov->iov_len = m_recv_buflen - nr;
  }
}

void C_SocketSCTPWithData::sctp_dispatch_msg (C_TransportEvent *P_event) {

  T_pDataRcv L_data ;

  ssize_t nr;
  struct sctp_sndrcvinfo *sri;
  struct msghdr msg[1];
  struct cmsghdr *cmsg;
  char cbuf[sizeof (*cmsg) + sizeof (*sri)];
  struct iovec iov[1];
  size_t cmsglen = sizeof (*cmsg) + sizeof (*sri);

  m_data_recv = false ;

  nr = 0 ;

  /* Set up the msghdr structure for receiving */
  memset(msg, 0, sizeof (*msg));
  msg->msg_control = cbuf;
  msg->msg_controllen = cmsglen;
  msg->msg_flags = 0;
  cmsg = (struct cmsghdr *)cbuf;
  sri = (struct sctp_sndrcvinfo *)(cmsg + 1);

  /* check if something is received */
  while (sctp_recv_msg(msg, &nr, cmsglen)) {

    /* Intercept notifications here */
    if (msg->msg_flags & MSG_NOTIFICATION) {
      sctp_event_handler (P_event);
      continue;
    }
  
    iov->iov_base = m_recv_buf;
    iov->iov_len = nr;
    msg->msg_iov = iov;
    msg->msg_iovlen = 1;
  
    if (nr) {
      // data recv
      // write data on buffer
      if (m_data_recv == false) {
	SOCKET_DEBUG(0, "C_SocketSCTPWithData::process_fd_set() RCV " << nr);
	P_event->m_type = C_TransportEvent::E_TRANS_RECEIVED ;
	P_event->m_channel_id = m_channel_id ;
	P_event->m_id = m_socket_id ;
	m_data_recv = true ;
      }

      ALLOC_VAR(L_data, T_pDataRcv, sizeof(T_DataRcv));
      L_data->m_size = nr ;
      ALLOC_TABLE(L_data->m_data, 
		  unsigned char*, 
		  sizeof(unsigned char*), 
		  nr);
      memcpy(L_data->m_data, m_recv_buf, nr);
      m_data_queue.push_back(L_data);

    } else { break ; }
  
  }

  if (nr < 0) {
    SOCKET_DEBUG(0, "C_SocketSCTPWithData::process_fd_set() CLOSED");
    P_event->m_type = C_TransportEvent::E_TRANS_CLOSED ;
    P_event->m_channel_id = m_channel_id ;
    P_event->m_id = m_socket_id ;
  }
}


C_SocketSCTP* C_SocketSCTPWithData::process_fd_set (fd_set           *P_rSet,
						    fd_set           *P_wSet,
  						    C_TransportEvent *P_event) {

  SOCKET_DEBUG(0, "C_SocketSCTPWithData::process_fd_set() id=" << m_socket_id);

  switch (m_state) {

  case E_SOCKET_STATE_READY:
  case E_SOCKET_STATE_INPROGESS:

    if (FD_ISSET(m_socket_id, P_rSet)) {
      sctp_dispatch_msg (P_event) ;
    }
    break ;

  case E_SOCKET_STATE_NOT_READY:
    break ;
    
  }

  return (NULL);
}

size_t C_SocketSCTPWithData::received_buffer(unsigned char *P_data, 
					     size_t         P_size_buf,
					     struct timeval *P_time) {

  size_t                  L_size = 0 ;
  T_DataRcvList::iterator L_it ;
  T_pDataRcv              L_data ;
  
  SOCKET_DEBUG(0, "C_SocketSCTPWithData::received_buffer() id=" << m_socket_id);

  if (!m_data_queue.empty()) {
    L_it = m_data_queue.begin();
    L_data = *L_it ;
    m_data_queue.erase(L_it);
    L_size = L_data->m_size ;
    SOCKET_DEBUG(0, "C_SocketSCTPWithData::received_buffer() size=" << L_size);
    memcpy(P_data, L_data->m_data, L_size);
    SOCKET_DEBUG(0, "C_SocketSCTPWithData::received_buffer() L_buf=" 
		 << L_data->m_data);
    *P_time = L_data->m_time ;
    FREE_TABLE(L_data->m_data);
    FREE_VAR(L_data) ;
  }

  SOCKET_DEBUG(0, "C_SocketSCTPWithData::received_buffer() return=" 
	       << L_size);
  return (L_size);
}

