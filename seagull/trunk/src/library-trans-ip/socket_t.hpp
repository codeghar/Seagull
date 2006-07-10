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

#ifndef _SOCKET_T_HPP
#define _SOCKET_T_HPP

#include <sys/types.h>
#include <sys/socket.h>

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

#define call_shutdown   shutdown
#define call_open       open
#define call_close      close
#define call_bind       bind
#define call_accept     accept
#define call_setsockopt setsockopt
#define call_getsockopt getsockopt
#define call_listen     listen
#define call_send       send
#define call_read       read
#define call_socket     socket
#define call_connect    connect
#define call_fcntl      fcntl
#define call_recvmsg    recvmsg
// UDP
#define call_recvfrom   recvfrom
#define call_sendto     sendto


#endif


