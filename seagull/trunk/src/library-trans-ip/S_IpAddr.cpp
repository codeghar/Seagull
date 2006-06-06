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

#include "S_IpAddr.hpp"

#include "Utils.hpp"

T_pIpAddr create_IpAddr () {
  T_pIpAddr L_ret = NULL ;
  ALLOC_VAR(L_ret, T_pIpAddr, sizeof(T_IpAddr));
  clear_IpAddr(L_ret);
  return (L_ret);
}

void  clear_IpAddr(T_pIpAddr P_IpAddr) {
  if (P_IpAddr != NULL) {
    P_IpAddr->m_umode = E_IP_USAGE_MODE_UNKNOWN ;
    P_IpAddr->m_buffer_size = DEFAULT_IP_BUFFER_SIZE ;
    P_IpAddr->m_open = NULL ;
    P_IpAddr->m_value = NULL ;
    P_IpAddr->m_port = -1 ;
    P_IpAddr->m_ip = NULL ;
    memset(&(P_IpAddr->m_addr), 0, sizeof(T_SockAddrStorage));  ;
  }
}

void  delete_IpAddr(T_pIpAddr *P_IpAddr) {

  T_pIpAddr L_IpAddr = NULL ;
  if (P_IpAddr != NULL) {
    L_IpAddr = *P_IpAddr ;

    FREE_TABLE(L_IpAddr->m_open);
    FREE_TABLE(L_IpAddr->m_value);
    FREE_TABLE(L_IpAddr->m_ip);
    FREE_VAR(L_IpAddr);

    *P_IpAddr = NULL ;
  }
  
}

int  get_domain(T_pIpAddr P_IpAddr) {
  int L_ret = -1 ;
  if (P_IpAddr != NULL) {
    switch ((P_IpAddr->m_addr).SOCKADDR_FAMILY) {
    case AF_INET:
      L_ret = PF_INET ;
      break ;
#ifndef USE_IPV4_ONLY
    case AF_INET6:
      L_ret = PF_INET6 ;
      break ;
#endif
    default:
      break ;
    }
  }
  return (L_ret);
}
