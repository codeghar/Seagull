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
#include "C_TransSCTP.hpp"
#include "Utils.hpp"

#include <cerrno>
#include <unistd.h>

#include <regex.h>

#include "iostream_t.hpp"
#include "integer_t.hpp"

#define GEN_ERROR(l,a) iostream_error << a << iostream_endl << iostream_flush ; 
#ifdef DEBUG_MODE
#define GEN_DEBUG(l,a) iostream_error << a << iostream_endl << iostream_flush ; 
#else
#define GEN_DEBUG(l,a) 
#endif

#define DEFAULT_BUFFER_SIZE 65535

C_TransSCTP::C_TransSCTP() {
  GEN_DEBUG(0, "C_TransSCTP::C_TransSCTP()") ;
  m_socket_map.clear () ;
  m_max_fd = 0 ;
  m_buffer_size = DEFAULT_BUFFER_SIZE ;
  sctp_initLibrary();
}

C_TransSCTP::~C_TransSCTP() {

  T_IpAddrList::iterator L_it ;
  T_pIpAddr              L_addr ;
  GEN_DEBUG(0, "C_TransSCTP::~C_TransSCTP()") ;
  if (!m_ip_addr_list.empty()) {
    for(L_it=m_ip_addr_list.begin();
	L_it != m_ip_addr_list.end();
	L_it++) {
      L_addr = *L_it ;
      delete_IpAddr(&L_addr);
    }
    m_ip_addr_list.erase(m_ip_addr_list.begin(), m_ip_addr_list.end());
  }
  close() ;
  m_max_fd = 0 ;
}

int C_TransSCTP::init (char *P_buf,
                       T_logFunction P_logError,
                       T_logFunction P_logInfo) {
  GEN_DEBUG(1, "C_TransSCTP::init  ("<< P_buf << ")");
  return ((analyze_init_string(P_buf) == true) ? 0 : -1);
}

int C_TransSCTP::config (T_pConfigValueList P_config_param_list) {
  GEN_DEBUG(1, "C_TransSCTP::config  ()");
  return(0);
}

int C_TransSCTP::open (int              P_channel_id,
		       char            *P_buf, 
		       T_pOpenStatus    P_status,
		       C_ProtocolFrame *P_protocol) {

  int       L_ret = 0 ;
  T_pIpAddr L_openAddr = NULL ;

  GEN_DEBUG(1, "C_TransSCTP::open(protocol=" << P_protocol << ")");

  // check protocol compatibility
  if (P_protocol->get_type() != C_ProtocolFrame::E_PROTOCOL_BINARY) {
    GEN_ERROR(1, "Protocol not compatible with transport (must be binary)");
    return (-1) ;
  }

  *P_status = E_OPEN_FAILED ;
  L_openAddr = create_IpAddr() ;
  
  L_ret = (analyze_open_string(P_buf, L_openAddr) == true) ? 
    open(P_channel_id, L_openAddr, P_status, (C_ProtocolBinaryFrame*)P_protocol) : -1 ;
  
  if (L_ret != -1) {
    m_ip_addr_list.push_back(L_openAddr) ;
  } else {
    delete_IpAddr(&L_openAddr);
  }
  return(L_ret) ;
}

int C_TransSCTP::set_channel_id (int   P_id,
				 int   P_channel_id) {
  int L_ret ;
  T_SocketMap::iterator L_it ;
  
  L_it = m_socket_map.find(T_SocketMap::key_type(P_id));
  if (L_it != m_socket_map.end()) {
    (L_it->second)->set_channel_id(P_channel_id);
    L_ret = 0 ;
  } else { 
    L_ret = -1 ;
  }
  return (L_ret);
}

int C_TransSCTP::pre_select (int             P_n, 
			   fd_set         *P_readfds,  
			   fd_set         *P_writefds,
			   fd_set         *P_exceptfds, 
			   struct timeval *P_timeout, 
			   int            *P_cnx, 
			   size_t          P_nb) {

  T_SocketMap::iterator L_it ;

  for(L_it = m_socket_map.begin();
      L_it != m_socket_map.end ();
      L_it ++) {
    (L_it->second) -> set_fd_set (P_readfds, P_writefds);
  }
  
  return ((m_max_fd > P_n) ? m_max_fd : P_n) ;
}

int C_TransSCTP::post_select (int                 P_n, 
	                    fd_set             *P_readfds,  
			    fd_set             *P_writefds,
			    fd_set             *P_exceptfds, 
			    T_pC_TransportEvent P_eventTable,
			    size_t             *P_nb) {

  T_SocketMap::iterator L_it ;
  int L_n = P_n ;
  C_SocketSCTP *L_newSocket, *L_socket ;
  C_TransportEvent L_event ;
  int L_id ;

  int L_nb_event = 0 ;

  list_t<T_SocketMap::iterator>::iterator L_del_it ;
  list_t<C_SocketSCTP*>::iterator         L_ins_it ;
  
  bool L_max_update = false ;

  GEN_DEBUG(0, "C_TransSCTP::post_select()");

  for (L_it = m_socket_map.begin();
       L_it != m_socket_map.end ();
       L_it++) {

    if (L_n) {

      L_socket = L_it->second ;
      L_event.no_event();
      L_newSocket = L_socket->process_fd_set(P_readfds,P_writefds, 
					     &L_event);
      switch (L_event.m_type) {
	    
      case C_TransportEvent::E_TRANS_NO_EVENT:
	GEN_DEBUG(0, "C_TransSCTP::post_select() E_TRANS_NO_EVENT");
	break ;
	    
      case C_TransportEvent::E_TRANS_RECEIVED:
	GEN_DEBUG(0, "C_TransSCTP::post_select() E_TRANS_RECEIVED");
	P_eventTable[L_nb_event] = L_event ;

	// insert decode process
	decode_from_protocol (L_socket);

	L_nb_event++ ;
	L_n--;
	break ;
	    
      case C_TransportEvent::E_TRANS_CLOSED:
	GEN_DEBUG(0, "C_TransSCTP::post_select() E_TRANS_CLOSED");
	L_id = (L_socket)->get_id() ;
	(L_socket)->_close() ;
	m_delete_list.push_back(L_it) ;
	DELETE_VAR(L_socket);
	if (L_id == m_max_fd) { L_max_update = true ; }
	P_eventTable[L_nb_event] = L_event ;
	L_nb_event++ ;
	L_n-- ;
	break ;
	    
      case C_TransportEvent::E_TRANS_CONNECTION:
	L_id = L_event.m_id ;
	GEN_DEBUG(0, "C_TransSCTP::post_select() E_TRANS_CONNECTION id=" 
		  << L_id);
	if (L_id > m_max_fd) { L_max_update = true ; }
	m_insert_list.push_back(L_newSocket);
	P_eventTable[L_nb_event] = L_event ;
	L_nb_event++ ;
	L_n--;
	break ;
	    
      case C_TransportEvent::E_TRANS_OPEN:
	P_eventTable[L_nb_event] = L_event ;
	L_nb_event++ ;
	L_n--;
	break ;
	
      case C_TransportEvent::E_TRANS_OPEN_FAILED:
	L_id = (L_socket)->get_id() ;
	(L_socket)->_close() ;
	m_delete_list.push_back(L_it) ;
	DELETE_VAR(L_socket);
	if (L_id == m_max_fd) { L_max_update = true ; }
	P_eventTable[L_nb_event] = L_event ;
	L_nb_event++ ;
	L_n--;
	break ;
	
      } // switch
    }
  }
    
  if (! m_delete_list.empty() ) {
    for(L_del_it = m_delete_list.begin();
	L_del_it != m_delete_list.end();
	L_del_it++) {
      m_socket_map.erase(*L_del_it);
    }
    m_delete_list.erase(m_delete_list.begin(), m_delete_list.end());
  }

  if (! m_insert_list.empty()) {
    for(L_ins_it = m_insert_list.begin();
	L_ins_it != m_insert_list.end();
	L_ins_it++) {
      L_newSocket = *L_ins_it ;
      L_id = L_newSocket->get_id();
      m_socket_map.insert(T_SocketMap::value_type(L_id,L_newSocket));
    }
    m_insert_list.erase(m_insert_list.begin(), m_insert_list.end());
  }

  if (L_max_update == true) {
    m_max_fd = (m_socket_map.empty()) ? 
      0 : (m_socket_map.rbegin())->first ;
    GEN_DEBUG(0, "C_TransSCTP::post_select() max fd [" << m_max_fd << "]");
  }

  *P_nb = L_nb_event ;
  GEN_DEBUG(0, "C_TransSCTP::post_select() L_nb_event = " << L_nb_event);
  GEN_DEBUG(0, "C_TransSCTP::post_select() return  = " << L_n);

  return (L_n);
}

int C_TransSCTP::send_message (int             P_id,
			       C_MessageFrame *P_msg) {

  T_SocketMap::iterator L_it         ;
  size_t                L_size = 0   ;
  int                   L_ret = -1   ;
  C_SocketSCTP         *L_socket     ;
  static  unsigned char L_data[1024] ;
  
  C_ProtocolBinaryFrame *L_protocol ; 
  C_ProtocolBinaryFrame::T_MsgError L_error ;

  L_it = m_socket_map.find (T_SocketMap::key_type(P_id));
  if (L_it != m_socket_map.end()) {
    L_socket = L_it->second ;
    L_protocol = L_socket -> get_protocol() ;
    L_error = L_protocol->encode_message(P_msg, L_data, &L_size);
    L_protocol->log_buffer((char*)"sent", L_data, L_size);
    L_ret = send_buffer2(P_id, L_data, L_size) ;
  }
  return (0) ;
}


size_t C_TransSCTP::send_buffer2 (int             P_id, 
				unsigned char  *P_data, 
				size_t          P_size) {

  size_t L_size = 0 ;
  int    L_rc       ;

  GEN_DEBUG(0, "C_TransSCTP::send_buffer2(" 
	    << P_id << "," << P_data << "," << P_size << ")");

  GEN_DEBUG(0, "C_TransSCTP::send_buffer() id OK");

  if ((L_rc = call_send(P_id, P_data, P_size, 0) < 0)) {
    GEN_ERROR(0, "send failed [" << L_rc << "] [" << strerror(errno) << "]");
    switch (errno) {
    case EAGAIN:
      GEN_ERROR(0, "Flow control not implemented");
      break ;
    case ECONNRESET:
      break ;
    default:
      GEN_ERROR(0, "process error [" << errno << "] not implemented");
      break ;
    }
  } else {
    L_size = P_size ;
  }
    
  GEN_DEBUG(0, "C_TransSCTP::send_buffer2() return " << L_size);
  return (L_size);
}

size_t C_TransSCTP::send_buffer (int             P_id, 
			       unsigned char  *P_data, 
			       size_t          P_size) {

  T_SocketMap::iterator L_it ;
  size_t L_size = 0 ;
  int    L_rc       ;

  GEN_DEBUG(0, "C_TransSCTP::send_buffer(" 
	    << P_id << "," << P_data << "," << P_size << ")");

  L_it = m_socket_map.find (T_SocketMap::key_type(P_id));
  if (L_it != m_socket_map.end()) {

    GEN_DEBUG(0, "C_TransSCTP::send_buffer() id OK");
    if ((L_rc = call_send(P_id, P_data, P_size, 0) < 0)) {
      GEN_ERROR(0, "send failed [" << L_rc << "] [" << strerror(errno) << "]");
    } else {
      L_size = P_size ;
    }
  } else {
    GEN_ERROR(0, "transport id [" << P_id << "] incorrect");
  }

  GEN_DEBUG(0, "C_TransSCTP::send_buffer() return " << L_size);
  return (L_size);
}

size_t C_TransSCTP::received_buffer (int            P_id, 
				   unsigned char *P_data, 
				   size_t         P_size_buf) {

  T_SocketMap::iterator L_it ;
  size_t                L_ret = 0 ;

  GEN_DEBUG(0, "C_TransSCTP::received_buffer(" << P_id << ")");
  L_it = m_socket_map.find(T_SocketMap::key_type(P_id));
  if (L_it != m_socket_map.end()) {
    // L_ret = (L_it->second) -> received_buffer (P_data, P_size_buf); CONFIRM ?
  } else {
    GEN_ERROR(0, "transport id [" << P_id << "] incorrect");
  }
  return (L_ret);
}

T_SelectDef C_TransSCTP::select_definition() {
  GEN_DEBUG(0, "C_TransSCTP::select_definition () ");
  return (&call_select);
}
  
int C_TransSCTP::close () {

  GEN_DEBUG(0, "C_TransSCTP::close ()");
  T_SocketMap::iterator L_it ;
  C_SocketSCTP *L_socket ;
  
  if (!m_socket_map.empty()) {
    for (L_it = m_socket_map.begin();
	 L_it != m_socket_map.end ();
	 L_it++) {
      L_socket = L_it->second ;
      L_socket -> _close();
      DELETE_VAR(L_socket);
    }
    m_socket_map.erase(m_socket_map.begin(), m_socket_map.end());
  }
  return (0);
}

int C_TransSCTP::close (int P_id) {

  T_SocketMap::iterator L_it ;
  C_SocketSCTP *L_socket ;
  int       L_ret = 0 ;

  GEN_DEBUG(0, "C_TransSCTP::close (" << P_id << ")");
  if (!m_socket_map.empty()) {
    L_it = m_socket_map.find (T_SocketMap::key_type(P_id));
    if (L_it != m_socket_map.end()) {
      L_socket = L_it->second ;
      L_socket -> _close();
      DELETE_VAR(L_socket);
      m_socket_map.erase(L_it);
    } else {
      L_ret = -1 ; 
    }
  } else {
    L_ret = -1 ;
  }

  return (L_ret);
}

// Internal methods

int C_TransSCTP::open (int              P_channel_id, 
		       T_pIpAddr        P_Addr,
		       T_pOpenStatus    P_status,
		       C_ProtocolBinaryFrame *P_protocol) {
  
  // Temporary for source/destination address
  //  char              *L_ptr ;
  char              *L_server_name   ;
  //  int                L_port ;
  //  int                L_server_port ;
  int                L_id = -1 ;
  int                L_rc ;
  C_SocketSCTP          *L_socket_created = NULL ;
  //  T_IpAddr           L_addr ;

  GEN_DEBUG(1, "C_TransSCTP::open ()");

  L_server_name = NULL ;

  if (P_Addr->m_open != NULL) {
    extract_ip_addr(P_Addr);
    resolve_addr(P_Addr);
  }

  switch (P_Addr->m_umode) {
  case E_IP_USAGE_MODE_SERVER: {
    C_SocketSCTPListen *L_Socket ;
    NEW_VAR(L_Socket, C_SocketSCTPListen(m_trans_type, P_Addr, P_channel_id));
    L_rc = L_Socket->_open(m_buffer_size, P_protocol) ; 
    if (L_rc == 0) {
      L_socket_created = L_Socket ;
      *P_status = E_OPEN_OK ;
    } else {
      DELETE_VAR(L_Socket) ;
      *P_status = E_OPEN_FAILED ;
    }
  }
    break ;
    
  case E_IP_USAGE_MODE_CLIENT: {
    C_SocketSCTPClient *L_Socket ;
    NEW_VAR(L_Socket, C_SocketSCTPClient(m_trans_type, P_Addr, P_channel_id));
    L_rc = L_Socket->_open(P_status, m_buffer_size, P_protocol) ;
    if (L_rc == 0) {
      L_socket_created = L_Socket ;
    } else {
      DELETE_VAR(L_Socket) ;
      *P_status = E_OPEN_FAILED ;
    }
  }

    break ;

  case E_IP_USAGE_MODE_UNKNOWN:

    GEN_ERROR(1, "OPEN failed: Unsupported mode");
    *P_status = E_OPEN_FAILED ;
    break ;
  }
  
  if (L_socket_created != NULL) {
    L_id = L_socket_created -> get_id () ;
    m_socket_map.insert (T_SocketMap::value_type(L_id,L_socket_created));
    if (L_id > m_max_fd) { m_max_fd = L_id;  } ;
  }

  FREE_VAR(L_server_name);
  
  return (L_id);
}

bool C_TransSCTP::analyze_init_string(char *P_buf) {

  bool             L_ret = false ;
  char             L_type [255] ;
  char            *L_ptr ;

  GEN_DEBUG(1, "C_TransSCTP::analyze_init_string  ("<< P_buf << ")");
  L_ptr = strstr(P_buf,"type=") ;
  if (L_ptr != NULL) {
    sscanf(L_ptr, "type=%[^;]*s", L_type);
    if (!strcmp(L_type,"tcp")) {
      m_trans_type = E_SOCKET_TCP_MODE ;
      L_ret = true ;
    } else if (!strcmp(L_type,"udp")) {
      m_trans_type = E_SOCKET_UDP_MODE ;
      L_ret = true ;
    }
    GEN_DEBUG(1, "C_TransSCTP::analyze_init_string() type [" << L_type << "]");
  }
  return (L_ret);
  
}

bool C_TransSCTP::analyze_open_string (char *P_buf, T_pIpAddr P_addr) {

  char            L_tmp  [255] ;
  char           *L_buf, *L_ptr ;

  GEN_DEBUG(1, "C_TransSCTP::analyze_open_string (args=" << P_buf << ")");

  L_buf = P_buf ;
  L_ptr = strstr(L_buf, "mode=");
  if (L_ptr != NULL) {
    sscanf(L_ptr+5, "%[^;]*s", L_tmp);
    GEN_DEBUG(1, "m [" << L_tmp << "]");
    if (!strcmp(L_tmp,"server")) {
      P_addr->m_umode = E_IP_USAGE_MODE_SERVER ;
    } else if (!strcmp(L_tmp,"client")) {
      P_addr->m_umode = E_IP_USAGE_MODE_CLIENT ;
    }
  }
  L_buf = P_buf ;
  L_ptr = strstr(L_buf, "source=");
  if (L_ptr != NULL) {
    sscanf(L_ptr+7, "%[^;]*s", L_tmp);
    GEN_DEBUG(1, "C_TransSCTP::analyze_open_string() s [" << L_tmp << "]");
    if (strlen(L_tmp)>0) {
      ALLOC_TABLE(P_addr->m_open,
		  char*,sizeof(char),
		  strlen(L_tmp)+1);
      strcpy(P_addr->m_open, L_tmp);
    } 
  }
  L_buf = P_buf ;
  L_ptr = strstr(L_buf, "dest=");
  if (L_ptr != NULL) {
    sscanf(L_ptr+5, "%[^;]*s", L_tmp);
    GEN_DEBUG(1, "C_TransSCTP::analyze_open_string() d [" << L_tmp << "]");
    if (strlen(L_tmp)>0) {
      ALLOC_TABLE(P_addr->m_open,
		  char*,sizeof(char),
		  strlen(L_tmp)+1);
      strcpy(P_addr->m_open, L_tmp);
    }
  }

  L_buf = P_buf ;
  L_ptr = strstr(L_buf, "buffer=");
  if (L_ptr != NULL) {
    sscanf(L_ptr+7, "%[^;]*s", L_tmp);
    GEN_DEBUG(1, "C_TransSCTP::analyze_open_string() buffer size [" 
	      << L_tmp << "]");
    if (strlen(L_tmp)>0) {
      char *L_end_ptr ;
      unsigned long L_value ;
      L_value = strtoul_f (L_tmp, &L_end_ptr, 10);
      if (L_end_ptr[0] == '\0') { // good format
	m_buffer_size = L_value ;
	P_addr->m_buffer_size = L_value ;
      }
    }
  }

  return (true) ;
}

int C_TransSCTP::extract_ip_addr(T_pIpAddr P_pIpAddr) {
  
  char      *L_search = NULL ;

  regex_t    L_regExpr ;
  regmatch_t L_pmatch ;
  int        L_status ;

  char       L_buffer[1024] ;
  size_t     L_matchSize ;

  GEN_DEBUG(0, "C_TransSCTP::extract_ip_addr()");

  if (P_pIpAddr == NULL) { return (-1) ; }
  if (P_pIpAddr->m_open == NULL) { return (0) ; }

  L_search = P_pIpAddr->m_open ;
  P_pIpAddr -> m_value = NULL ;
  P_pIpAddr -> m_port = -1 ;
  memset(&(P_pIpAddr->m_addr), 0, sizeof(T_SockAddrStorage));

  // skip blank 
  L_status = regcomp (&L_regExpr, "^[:blank:]*\\[", REG_EXTENDED) ;
  if (L_status != 0) {
    regerror(L_status, &L_regExpr, L_buffer, 1024);
    regfree (&L_regExpr) ;
    GEN_ERROR(0, "regcomp error: [" << L_buffer << "]");
    return (-1);
  }
  
  L_status = regexec (&L_regExpr, L_search, 1, &L_pmatch, 0) ;
  regfree (&L_regExpr) ;

  if (L_status == 0) { // IP V6

    L_search += L_pmatch.rm_eo ;

    // find end of address
    L_status = regcomp(&L_regExpr, "[^]]*", REG_EXTENDED) ;
    if (L_status != 0) {
      regerror(L_status, &L_regExpr, L_buffer, 1024);
      regfree (&L_regExpr) ;
      GEN_ERROR(0, "regcomp error: [" << L_buffer << "]");
      return (-1);
    }

    L_status = regexec (&L_regExpr, L_search, 1, &L_pmatch, 0) ;
    regfree (&L_regExpr) ;
    if (L_status == 0) { // end of IP addr found

      L_matchSize = L_pmatch.rm_eo - L_pmatch.rm_so ;
      if (L_matchSize) { memcpy(L_buffer, L_search, L_matchSize); }
      L_buffer[L_matchSize] = 0 ;
      L_search += L_matchSize ;
      L_search = strstr(L_search, ":");

      GEN_DEBUG(1, "C_TransSCTP::extract_ip_addr() IPV6 addr [" << L_buffer << "]");
      
    } else {
      GEN_ERROR(0, "regexec error character [" << ']' << "] not found" );
      return (-1);
    }

  } else { // IP V4 or hostname

    
    L_search = strstr(P_pIpAddr->m_open, ":") ;

    L_matchSize = (L_search != NULL) 
      ? (L_search - (P_pIpAddr->m_open)) : strlen(P_pIpAddr->m_open) ;
    if (L_matchSize) { memcpy(L_buffer, P_pIpAddr->m_open, L_matchSize) ; }
    L_buffer[L_matchSize] = 0 ;

      
    GEN_DEBUG(0, "C_TransSCTP::extract_ip_addr() IPV4 addr or hostname = [" 
	      << L_buffer << "]");
  }

  if (strlen(L_buffer) != 0) {
    ALLOC_TABLE(P_pIpAddr -> m_value, char*, 
		sizeof(char), strlen(L_buffer)+1) ;
    strcpy(P_pIpAddr->m_value, L_buffer);
  }

  if (L_search != NULL) {
    char *L_end_ptr = NULL ;
    P_pIpAddr -> m_port = (long)strtoul_f(L_search+1, &L_end_ptr, 10);
    if (L_end_ptr[0] != '\0') { 
      P_pIpAddr->m_port = -1 ;
      GEN_DEBUG (0, "C_TransSCTP::extract_ip_addr() port not defined");
    }
  } else {
    GEN_DEBUG (0, "C_TransSCTP::extract_ip_addr() port not defined");
  }

  GEN_DEBUG(1, "C_TransSCTP::extract_ip_addr() Port [" << P_pIpAddr->m_port << "]");

#ifdef DEBUG_MODE
  {
    const char *L_novalue = "no value" ;
    char *L_value = (P_pIpAddr->m_value == NULL) ?
      (char*)L_novalue : P_pIpAddr->m_value ;
    GEN_DEBUG(1, "C_TransSCTP::extract_ip_addr() Addr value [" 
	      << L_value << "]") ;
  }
#endif

  return (0);
}

int C_TransSCTP::resolve_addr(T_pIpAddr P_pIpAddr) {

  char              *L_host              = P_pIpAddr->m_value ;
  long               L_port              = P_pIpAddr->m_port  ;
  T_SockAddrStorage *L_resolved_sockaddr = &P_pIpAddr->m_addr ;
  char               L_local_host[255]                        ;

#ifndef USE_IPV4_ONLY  
  struct addrinfo   L_hints      ;
  struct addrinfo  *L_local_addr ;
#else
  struct hostent   *L_local_addr ;
#endif

#ifdef DEBUG_MODE

  const char *L_cNoHost = "none" ;
  char *L_hostDebug = (L_host == NULL) ? (char*)L_cNoHost : L_host ;

  GEN_DEBUG(1, "C_TransSCTP::resolve_addr() [" 
	    << L_hostDebug
	    << "]:[" << L_port << "]");
#endif

#ifndef USE_IPV4_ONLY
  memset((char*)&L_hints, 0, sizeof(L_hints));
  L_hints.ai_flags  = AI_PASSIVE;
  L_hints.ai_family = PF_UNSPEC;
#endif

  if (L_host == NULL) {
    if (gethostname(L_local_host, 255)) {
      GEN_ERROR(1, "Unable to get local IP address");
      return(-1);
    } else {
      ALLOC_TABLE(L_host, char*, sizeof(char),
		  strlen(L_local_host)+1);
      strcpy(L_host, L_local_host) ;
      P_pIpAddr->m_value = L_host  ;
    }
  }
  memset(L_resolved_sockaddr, 0, sizeof(T_SockAddrStorage));

#ifndef USE_IPV4_ONLY
  if (getaddrinfo(L_host,
		  NULL,
		  &L_hints,
		  &L_local_addr) != 0) {
    GEN_ERROR(1, "Unknown host [" << L_host << "]");
    return (-1);
  }

  L_resolved_sockaddr->SOCKADDR_FAMILY = L_local_addr->ai_addr->sa_family;
  memcpy(L_resolved_sockaddr,
	 L_local_addr->ai_addr,
	 SOCKADDR_IN_SIZE
	 (RICAST(T_SockAddrStorage *,L_local_addr->ai_addr)));
#else
  L_local_addr = gethostbyname(L_host);
  if (L_local_addr != NULL) {
    L_resolved_sockaddr->SOCKADDR_FAMILY = L_local_addr->h_addrtype;
    memcpy((void*)L_resolved_sockaddr->sin_addr.s_addr,
	   L_local_addr->h_addr_list[0],
	   L_local_addr->h_length);
  } else {
    GEN_ERROR(1, "Unknown host [" << L_host << "]");
    return (-1);
  }
#endif

  switch (L_resolved_sockaddr->SOCKADDR_FAMILY) {
  case AF_INET:
    (RICAST(struct sockaddr_in *, L_resolved_sockaddr))->sin_port =
      htons((short)L_port);
    break ;
#ifndef USE_IPV4_ONLY
  case AF_INET6:
    (RICAST(struct sockaddr_in6 *, L_resolved_sockaddr))->sin6_port =
      htons((short)L_port);
    break ;
#endif
  default:
    GEN_ERROR(1, "Unsupported network");
    return(-1);
  }
  if (inet_addr((char**)&(P_pIpAddr->m_ip), L_resolved_sockaddr) == -1) {
    GEN_ERROR(1, "Address not supported");
  }

  GEN_DEBUG(0, "C_TransSCTP::inet_addr() inet_addr [" << P_pIpAddr->m_ip << "]");

  return(0);
}

int C_TransSCTP::inet_addr(char **P_Addr, T_SockAddrStorage * P_AddrS) {

#ifndef USE_IPV4_ONLY
  char * L_ipAddr = NULL;

  GEN_DEBUG(0, "C_TransSCTP::inet_addr()");

  ALLOC_TABLE(L_ipAddr, char*, sizeof(char), 1024);

  if (getnameinfo(RICAST(struct sockaddr *, P_AddrS),
                  sizeof(struct sockaddr_storage),
                  L_ipAddr,
                  1024,
                  NULL,
                  0,
                  NI_NUMERICHOST) != 0) {
    FREE_TABLE(L_ipAddr);
    return(-1);
  }

  *P_Addr = L_ipAddr ;
#endif 
  return (0) ;
  
}

void C_TransSCTP::decode_from_protocol (C_SocketSCTP *P_socket) {

  // previous data remaining buffer
  size_t                            L_data_size, L_tmp_size ;
  static unsigned char              L_data [4096] ;
  size_t                            L_buf_size = 4096 ;
  unsigned char                    *L_session_buf, *L_current_data ;
  C_pSCTPDataDecode                 L_decode = P_socket->get_decode() ;
  C_ProtocolBinaryFrame            *L_protocol = P_socket->get_protocol() ;
  T_pRcvMsgCtxtList                 L_msg_list = P_socket->get_list() ;
  T_ReceiveMsgContext               L_msg_ctxt ;
  
  C_MessageFrame                   *L_decoded_msg ;
  C_ProtocolBinaryFrame::T_MsgError L_decode_result        ;
  size_t                            L_ret ;
  
  GEN_DEBUG(0, "C_TransSCTP::decode_from_protocol() start");

  L_msg_ctxt.m_channel = P_socket->get_channel_id() ;
  L_msg_ctxt.m_response = P_socket->get_id();

  while ((L_data_size 
	  = P_socket->received_buffer(L_data, 
				      L_buf_size, 
				      &L_msg_ctxt.m_time)) != 0) {

    L_protocol->log_buffer((char*)"received", L_data, L_data_size);

    L_session_buf = L_decode -> get_buffer(L_data, &L_data_size);

    L_current_data = L_session_buf ;
    L_tmp_size = L_data_size ;
    L_ret = L_data_size ;
    
    if (L_tmp_size) {

      do {

	// start decode engine
	L_decoded_msg 
	  = L_protocol->decode_message (L_current_data, 
					&L_ret, 
					&L_decode_result) ;
	
	switch (L_decode_result) {

	case C_ProtocolBinaryFrame::E_MSG_OK:
	  // Store new message received
	  // for the call controller

	  L_msg_ctxt.m_msg = L_decoded_msg ;
	  L_msg_list->push_back(L_msg_ctxt);
	  
	  // prepare for next message
	  L_tmp_size = L_data_size - L_ret ;
	  if (L_ret != 0) { // not the last of the buffer
	    L_current_data += L_tmp_size ;
	    L_data_size -= L_tmp_size ;
	  }
	  break ;

	case C_ProtocolBinaryFrame::E_MSG_ERROR_DECODING_SIZE_LESS:
	  // segmentation
	  // => put L_current_data and L_data_size in 
	  // => remaining data_buffer

	  // save buffer for next receive
	  if (L_decode->set_buffer (L_current_data, 
				    L_data_size) != 0) {
	    L_decode->reset_buffer() ;
	  } 
	  L_ret = 0 ;
	  break ;
	default:
	  // TO BE DONE : 
	  // more complex synchronization algorithm !!!!!
	  GEN_ERROR(E_GEN_FATAL_ERROR, 
		    "Unrecognized message received") ;
	  L_decode->reset_buffer() ;
	  L_data_size = 0 ;
	  L_ret = 0 ;
	  break ;
	}
      
      } while (L_ret != 0) ;

    }

  }

  GEN_DEBUG(0, "C_TransSCTP::decode_from_protocol() end");
}

bool C_TransSCTP::get_message (int P_id, T_pReceiveMsgContext P_ctxt) {

  T_SocketMap::iterator  L_it          ;
  bool                   L_ret = false ;
  T_pRcvMsgCtxtList      L_list        ;

  GEN_DEBUG(0, "C_TransSCTP::get_message(" << P_id << ")");

  L_it = m_socket_map.find(T_SocketMap::key_type(P_id));
  if (L_it != m_socket_map.end()) {
    L_list = (L_it->second)->get_list() ;
    if (!L_list->empty()) {
      *P_ctxt = *L_list->begin() ;
      L_ret = true ;
      L_list->erase(L_list->begin()) ;
    }
  } else {
    GEN_ERROR(0, "transport id [" << P_id << "] incorrect");
  }

  GEN_DEBUG(0, "C_TransSCTP::get_message() end with " << L_ret);
  return (L_ret);
}

// External interface

T_pTransport create_cipio_instance () {
  T_pC_TransSCTP L_inst ;
  NEW_VAR(L_inst, C_TransSCTP());
  return (L_inst);
}

void delete_cipio_instance (T_ppTransport P_inst) {
  if (P_inst != NULL) {
    T_pC_TransSCTP L_inst = (T_pC_TransSCTP) *P_inst ;
    DELETE_VAR(L_inst);
    *P_inst = NULL ;
  }
}

// end of file
