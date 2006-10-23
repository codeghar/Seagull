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

#ifndef _C_TRANS_IP_TLS_H
#define _C_TRANS_IP_TLS_H

#include "C_TransIP.hpp"
#include "ssl_t.hpp"



class C_TransIPTLS : public C_TransIP {

public:

  C_TransIPTLS();
  virtual ~C_TransIPTLS();

  virtual int init (char *P_buf,
                    T_logFunction P_logError,
                    T_logFunction P_logInfo) ;

  virtual int         config (T_pConfigValueList P_config_param_list) ;

  typedef SSL_METHOD* (*T_SSLMethodType)(void); 
  typedef struct _T_supported_methods {
    char *m_name ;
    T_SSLMethodType m_method ;
  } T_supported_methods ;

protected : 

  virtual C_Socket* open (int              P_channel_id, 
			  T_pIpAddr        P_Addr,
			  T_pOpenStatus    P_status,
			  C_ProtocolBinaryFrame *P_protocol) ;

  virtual int set_option (int P_Channel_Id, char *P_buf) ;

  int analyze_config(T_ConfigValue& P_config) ;
  
  virtual bool            analyze_init_string (char *P_buf) ;

  int load_crls(char *crlfile) ;


protected :

  C_Socket* make_secure (C_Socket *) ;

  T_SSLMethodType find_method(char* P_name);

  SSL_CTX        *m_ssl_ctx ;
  char           *m_passwd ;
  char           *m_cert_chain_file ;
  char           *m_crl_file ;
  char           *m_private_key_file ;
  T_SSLMethodType m_method ;

  bool            m_start_secure_mode ;

} ;

extern "C" T_pTransport create_ciptlsio_instance () ;
extern "C" void         delete_ciptlsio_instance (T_ppTransport) ;

#endif // _C_TRANS_IP_TLS_H






