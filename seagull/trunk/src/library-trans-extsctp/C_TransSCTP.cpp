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
#include "C_SocketSCTP.hpp"
#include "socketSCTP_t.hpp"
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


int C_TransSCTP::config (T_pConfigValueList P_config_param_list) {
  GEN_DEBUG(1, "C_TransIP::config  ()");
  m_logInfo = NULL ;
  m_logError = NULL ;

  int                           L_ret = 0       ;
  T_ConfigValueList::iterator   L_config_it     ;

  if (!P_config_param_list->empty()) {
    for (L_config_it = P_config_param_list->begin() ;
      L_config_it != P_config_param_list->end();
      L_config_it++) {
      L_ret = analyze_config(*L_config_it) ;
      if (L_ret != 0) break ;
    }
  }

  return (L_ret);
}

int C_TransSCTP::analyze_config(T_ConfigValue& P_config) {
  int                           L_ret = 0       ;
  return (L_ret);
}


C_TransSCTP::C_TransSCTP() : C_TransIP()  {
  sctp_initLibrary();
}

C_TransSCTP::~C_TransSCTP() {
}

T_SelectDef C_TransSCTP::select_definition() {
  GEN_DEBUG(0, "C_TransSCTP::select_definition () ");
  return (&ext_select);
}

C_Socket* C_TransSCTP::open (int              P_channel_id, 
                             T_pIpAddr        P_Addr,
                             T_pOpenStatus    P_status,
                             C_ProtocolBinaryFrame *P_protocol) {


  int                L_rc ;
  C_Socket          *L_socket_created = NULL ;

 
    
    GEN_DEBUG(1, "C_TransIPTLS::open ()");
    
    switch (P_Addr->m_umode) {
    case E_IP_USAGE_MODE_SERVER: {
    
    if (m_trans_type == E_SOCKET_TCP_MODE) {      
      C_SocketSCTPListen *L_Socket ;
    
      NEW_VAR(L_Socket, C_SocketSCTPListen(m_trans_type, 
                                           P_Addr, 
                                           P_channel_id, 
                                           m_read_buffer_size, 
                                           m_decode_buffer_size));


      L_rc = L_Socket->_open(m_buffer_size, P_protocol) ;
      if (L_rc == 0) {
        L_socket_created = L_Socket ;
        *P_status = E_OPEN_OK ;
      } else {
        DELETE_VAR(L_Socket) ;
        *P_status = E_OPEN_FAILED ;
      }
    } else {
      C_SocketSCTPServer *L_Socket ;
      
      NEW_VAR(L_Socket, C_SocketSCTPServer(m_trans_type, 
                                           P_Addr, 
                                           P_channel_id, 
                                           m_read_buffer_size, 
                                           m_decode_buffer_size));
      
      L_rc = L_Socket->_open_udp(m_buffer_size, P_protocol) ;
      if (L_rc == 0) {
        L_socket_created = L_Socket ;
        *P_status = E_OPEN_OK ;
      } else {
        DELETE_VAR(L_Socket) ;
        *P_status = E_OPEN_FAILED ;
      }
    }
    }
      break ;
    
    case E_IP_USAGE_MODE_CLIENT: {
      C_SocketSCTPClient *L_Socket ;
      
      NEW_VAR(L_Socket, C_SocketSCTPClient(m_trans_type, 
                                           P_Addr, 
                                           P_channel_id, 
                                           m_read_buffer_size, 
                                           m_decode_buffer_size));

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

  return (L_socket_created);
}
 
// External interface

T_pTransport create_cipsctpio_instance () {
  C_TransSCTP* L_inst ;
  NEW_VAR(L_inst, C_TransSCTP());
  return (L_inst);
}

void delete_cipsctpio_instance (T_ppTransport P_inst) {
  if (P_inst != NULL) {
    C_TransSCTP* L_inst = (C_TransSCTP*) *P_inst ;
    DELETE_VAR(L_inst);
    *P_inst = NULL ;
  }
}


// end of file
