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
#ifndef _SOCKETSCTP_T_HPP
#define _SOCKETSCTP_T_HPP

#include <sctp.h>
#include <ext_socket.h>

#ifndef AF_INET6
#define USE_IPV4_ONLY
#endif

#if defined(__hpux)

#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#else 
#include <netinet/in.h>
#endif

#else
#include <netinet/in.h>
#endif

#define DEFAULT_IP_BUFFER_SIZE 65535

#if defined (__hpux) 
#ifdef _XOPEN_SOURCE_EXTENDED
#define tool_socklen_t  socklen_t
#else
#define tool_socklen_t  int
#endif
#else
#define tool_socklen_t  socklen_t
#endif

#if defined(__cplusplus) && defined (__hpux)
#define RICAST(type, val) (reinterpret_cast<type> (val))
#else
#define RICAST(type, val) ((type)(val))
#endif

#ifdef USE_IPV4_ONLY

typedef struct sockaddr_in      T_SockAddrStorage ;
#define SOCKADDR_IN_SIZE(a)     sizeof(struct sockaddr_in)
#define SOCKADDR_FAMILY         sin_family
#else

typedef struct sockaddr_storage T_SockAddrStorage ;
#define SOCKADDR_IN_SIZE(a) \
  (((a)->ss_family == AF_INET) ? sizeof(struct sockaddr_in) \
                               : sizeof(struct sockaddr_in6))
#define SOCKADDR_FAMILY        ss_family
#endif


// system call 

#define call_shutdown   ext_shutdown
#define call_open       ext_open
#define call_close      ext_close
#define call_bind       ext_bind
#define call_accept     ext_accept
#define call_setsockopt ext_setsockopt
#define call_getsockopt ext_getsockopt
#define call_listen     ext_listen
#define call_send       ext_send
#define call_read       ext_read
#define call_socket     ext_socket
#define call_connect    ext_connect
#define call_fcntl      ext_fcntl
#define call_select     ext_select
#define call_recvmsg    ext_recvmsg

#endif


