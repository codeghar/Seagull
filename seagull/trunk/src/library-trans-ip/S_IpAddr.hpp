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

#ifndef _S_IP_ADDR_HPP
#define _S_IP_ADDR_HPP

#include "socket_t.hpp"
#include "list_t.hpp"

#include "map_t.hpp"

typedef enum _enum_ip_usage_mode {
  E_IP_USAGE_MODE_UNKNOWN,
  E_IP_USAGE_MODE_SERVER,
  E_IP_USAGE_MODE_CLIENT
} T_IpUsageMode, *T_pIpUsageMode ;


typedef struct _struct_ip_addr {
  T_IpUsageMode           m_umode ;
  size_t                  m_buffer_size ;
  char                   *m_open  ;
  char                   *m_value ;
  long                    m_port  ;
  char                   *m_ip    ;
  T_SockAddrStorage       m_addr  ;
} T_IpAddr, *T_pIpAddr ;

void      clear_IpAddr(T_pIpAddr P_IpAddr) ;
T_pIpAddr create_IpAddr() ;
void      delete_IpAddr(T_pIpAddr *P_IpAddr) ;

int       get_domain(T_pIpAddr P_IpAddr);

typedef list_t<T_pIpAddr> T_IpAddrList, *T_pIpAddrList ;

typedef map_t<int, T_pIpAddr> T_IpAddrMap, *T_pIpAddrMap ;

#endif


