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

#include "C_TransIPTLS.hpp"
#include "C_SecureSocket.hpp"
#include "Utils.hpp"

#define CALL_BACK_USER_DATA "ksgr"

#ifdef DEBUG_MODE
#define GEN_DEBUG(l,a) iostream_error << a << iostream_endl << iostream_flush ; 
#else
#define GEN_DEBUG(l,a) 
#endif

#define GEN_ERROR(l,a) iostream_error << a << iostream_endl << iostream_flush ; 

static C_TransIPTLS::T_supported_methods m_methods [] = {
  { "SSLv23", SSLv23_method }
} ;
static int m_nb_methods = 1 ;

static char *password ;
static int passwd_call_back(char *buf,int num,
                            int rwflag,void *userdata) {
    if(num<(int)strlen(password)+1)
      return(0);

    strcpy(buf,password);
    return(strlen(password));
}

static int verify_callback(int P_ok , X509_STORE_CTX *P_store)
{
  char L_data[512];
 
  if (!P_ok) {

     X509 *L_cert = X509_STORE_CTX_get_current_cert(P_store);
     //     int   L_depth = X509_STORE_CTX_get_error_depth(P_store);
     //     int   L_err   = X509_STORE_CTX_get_error(P_store);

     X509_NAME_oneline(X509_get_issuer_name(L_cert),
                                   L_data,512);
     GEN_ERROR(1, "TLS verification error for issuer [" << L_data << "]");
     X509_NAME_oneline(X509_get_subject_name(L_cert),
                                   L_data,512);
     GEN_ERROR(1, "TLS verification error for subject [" << L_data << "]");
  }
  return P_ok;
}


int C_TransIPTLS::load_crls(char *P_crlfile)
{
  X509_STORE          *L_store;
  X509_LOOKUP         *L_lookup;

  /*  Get the X509_STORE from SSL context */
  if (!(L_store = SSL_CTX_get_cert_store(m_ssl_ctx))) {
    return (-1);
  }

  /* Add lookup file to X509_STORE */
  if (!(L_lookup = X509_STORE_add_lookup(L_store,X509_LOOKUP_file()))) {
    return (-1);
  }

  /* Add the CRLS to the lookpup object */
  if (X509_load_crl_file(L_lookup,m_crl_file,X509_FILETYPE_PEM) != 1) {
    return (-1);
  }

  /* Set the flags of the store so that CRLS's are consulted */
#if OPENSSL_VERSION_NUMBER >= 0x00907000L
  X509_STORE_set_flags( L_store,X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#else
  GEN_ERROR(1, "version OpenSSL (<0.9.7) cannot handle CRL files in capath");
#endif

  return (1);
}



C_TransIPTLS::T_SSLMethodType C_TransIPTLS::find_method(char *P_name) {

  int L_i ;
  T_SSLMethodType L_ret = NULL ;

  for (L_i = 0 ; L_i < m_nb_methods ; L_i++) {
    if (strcmp(P_name, m_methods[L_i].m_name) == 0){
      L_ret = m_methods[L_i].m_method ;
      break ;
    }
  }
  return (L_ret);
}

int C_TransIPTLS::config (T_pConfigValueList P_config_param_list) {
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

bool C_TransIPTLS::analyze_init_string (char *P_buf) {

  bool             L_ret = false  ;
  char             L_tmp  [255]   ;

  analyze_optional_init_string(P_buf);
  
  L_tmp[0] = '\0' ;
  if (analyze_string_value (P_buf,
                           (char*)"cert_chain_file=",
                           L_tmp)) {
    ALLOC_TABLE(m_cert_chain_file,
                char*,
                sizeof(char),
                strlen(L_tmp)+1);
    sprintf(m_cert_chain_file, "%s", L_tmp);
    L_ret = true ;
  } else {
    // mandatory
    GEN_ERROR(1, "[cert_chain_file] value madatory for TLS transport");
    L_ret = false ;
  }

  L_tmp[0] = '\0' ;
  if (L_ret && analyze_string_value (P_buf,
                                     (char*)"private_key_file=",
                                     L_tmp)) {
    ALLOC_TABLE(m_private_key_file,
                char*,
                sizeof(char),
                strlen(L_tmp)+1);
    sprintf(m_private_key_file, "%s", L_tmp);
    L_ret = true ;
  } else {
    // mandatory
    GEN_ERROR(1, "[private_key_file] value madatory for secure transport");
    L_ret = false ;
  } 

  L_tmp[0] = '\0' ;
  if (L_ret && analyze_string_value (P_buf,
                                     (char*)"passwd=",
                                     L_tmp)) {
    ALLOC_TABLE(m_passwd,
                char*,
                sizeof(char),
                strlen(L_tmp)+1);
    sprintf(m_passwd, "%s", L_tmp);
    password = m_passwd ;
    L_ret = true ;
  } else {
    // mandatory
    GEN_ERROR(1, "[passwd] value madatory for secure transport");
    L_ret = false ;
  }

  L_tmp[0] = '\0' ;
  if (L_ret && analyze_string_value (P_buf,
                                     (char*)"method=",
                                     L_tmp)) {

    m_method = find_method(L_tmp) ;
    if (m_method == NULL) {
      GEN_ERROR(1, "unknown [method] value for secure transport");
      L_ret = false ;
    } else {
      L_ret = true ;
    }
  } else {
    // mandatory
    GEN_ERROR(1, "[method] value madatory for secure transport");
    L_ret = false ;
  }

  L_tmp[0] = '\0' ;
  if (L_ret && analyze_string_value (P_buf,
                                     (char*)"crl_file=",
                                     L_tmp)) {
    ALLOC_TABLE(m_crl_file,
                char*,
                sizeof(char),
                strlen(L_tmp)+1);
    sprintf(m_crl_file, "%s", L_tmp);
    L_ret = true ;
  } 

  L_tmp[0] = '\0' ;
  if (L_ret && analyze_string_value (P_buf,
                                     (char*)"secure=",
                                     L_tmp)) {

    if (strcmp(L_tmp, (char*)"no") == 0){
      m_start_secure_mode = false ;
    }
    L_ret = true ;
  } 

  return (L_ret);
}


int C_TransIPTLS::analyze_config(T_ConfigValue& P_config) {
  int                           L_ret = 0       ;
  return (L_ret);
}

C_TransIPTLS::C_TransIPTLS() : C_TransIP()  {
  m_ssl_ctx = NULL ;
  m_passwd = NULL ;
  m_cert_chain_file = NULL ;
  m_private_key_file = NULL ;
  m_crl_file = NULL ;
  m_method = NULL ;
  m_start_secure_mode = true ;
}

C_TransIPTLS::~C_TransIPTLS() {
  if (m_ssl_ctx) {
    SSL_CTX_free(m_ssl_ctx);
  }
  FREE_TABLE(m_passwd);
  FREE_TABLE(m_cert_chain_file);
  FREE_TABLE(m_private_key_file);
  FREE_TABLE(m_crl_file);
}



int C_TransIPTLS::init (char *P_buf,
                        T_logFunction P_logError,
                        T_logFunction P_logInfo) {

  int L_ret = 0 ;

  L_ret = C_TransIP::init(P_buf, P_logError, P_logInfo) ;    
  if (L_ret != -1 ) {
    // init SSL library
    SSL_library_init();
    SSL_load_error_strings() ;

    if ((m_ssl_ctx = SSL_CTX_new(((*m_method)()))) == NULL ) {
      GEN_ERROR(1, "SSL_CTX_new failed");
      L_ret = -1 ;
    } else {
      m_trans_type = E_SOCKET_TCP_MODE ;
      SSL_CTX_set_default_passwd_cb_userdata(m_ssl_ctx,
                                             (void *)m_passwd);
      SSL_CTX_set_default_passwd_cb(m_ssl_ctx,
                                    passwd_call_back);
      
      if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx,
                                             m_cert_chain_file) != 1) {
        GEN_ERROR(1, "SSL_CTX_use_certificate_file failed");
        L_ret = -1 ;
      }
      
      if (L_ret != -1) {
        if (!(SSL_CTX_use_PrivateKey_file(m_ssl_ctx,
                                          m_private_key_file,
                                          SSL_FILETYPE_PEM))) {
          GEN_ERROR(1, "SSL_CTX_use_PrivateKey_file failed");
          L_ret = -1 ;
        }
      
        if (L_ret != -1) {
          
          if (!(SSL_CTX_load_verify_locations(m_ssl_ctx, m_cert_chain_file, 0))) {
            GEN_ERROR(1, "Cannot load CA");
              L_ret = -1 ;
          }

          if (L_ret != -1) {
          
            if (m_crl_file != NULL) {
              if(load_crls(m_crl_file) == -1) {
                GEN_ERROR(1, "Unable to load CRL file [" << m_crl_file << "]" );
                L_ret = -1 ;
              }
              
              /* The following call forces to process the certificates with the */
              /* initialised SSL_CTX                                            */
              SSL_CTX_set_verify(m_ssl_ctx,
                                 SSL_VERIFY_PEER |
                                 SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                                 verify_callback);
            }
          }
        }
      }
    }
  }
  
  return (L_ret);

}


C_Socket* C_TransIPTLS::open (int              P_channel_id, 
			      T_pIpAddr        P_Addr,
			      T_pOpenStatus    P_status,
			      C_ProtocolBinaryFrame *P_protocol) {

  C_Socket          *L_socket_created = NULL ;

  if (m_start_secure_mode == true) {
    
    int                L_rc ;
    
    GEN_DEBUG(1, "C_TransIPTLS::open ()");
    
    switch (P_Addr->m_umode) {
    case E_IP_USAGE_MODE_SERVER: {
      
      C_SecureSocketListen *L_Socket ;
    
      NEW_VAR(L_Socket, C_SecureSocketListen(m_ssl_ctx,
					     m_trans_type, 
					     P_Addr, 
					     P_channel_id, 
					     m_read_buffer_size, 
					     m_decode_buffer_size));
      // std::cerr << "m_trans_type " << m_trans_type << std::endl;
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
      C_SecureSocketClient *L_Socket ;
      
      NEW_VAR(L_Socket, C_SecureSocketClient(m_ssl_ctx,
					     m_trans_type, 
					     P_Addr, 
					     P_channel_id, 
					     m_read_buffer_size, 
					     m_decode_buffer_size));

      // std::cerr << "m_trans_type Client" << m_trans_type << std::endl;
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
  
  } else {
    L_socket_created = C_TransIP::open(P_channel_id, 
				       P_Addr,
				       P_status,
				       P_protocol) ;
  }

  return (L_socket_created);
}

C_Socket* C_TransIPTLS::make_secure (C_Socket *P_Socket) {
  
  C_Socket *L_new = NULL ;
  
  {
    C_SocketClient *L_socket = NULL ;

    L_socket = dynamic_cast<C_SocketClient*>(P_Socket) ;
    if (L_socket != NULL) {
      NEW_VAR(L_new,
	      C_SecureSocketClient(m_ssl_ctx,*L_socket));
    }
  }

  if (L_new == NULL) {
    C_SocketServer *L_socket = NULL ;

    L_socket = dynamic_cast<C_SocketServer*>(P_Socket) ;
    if (L_socket != NULL) {
      NEW_VAR(L_new,
	      C_SecureSocketServer(m_ssl_ctx,*L_socket));
    }
  }

  // making secure a listen socket is not possible
  
  return (L_new);
}

int C_TransIPTLS::set_option (int P_Channel_Id, char *P_buf) {

  C_Socket             *L_socket = NULL;
  T_SocketMap::iterator L_it ;
  int                   L_ret = 0 ;

  L_it = m_socket_map.find(T_SocketMap::key_type(P_Channel_Id));
  if (L_it != m_socket_map.end()) {
    L_socket = make_secure (L_it->second);
    if (L_socket != NULL) {
      m_socket_map.erase(L_it);
      m_socket_map.insert(T_SocketMap::value_type(L_socket->get_id(),
                                                  L_socket));
    } else {
      
      L_ret = -1 ;
    }
    //DELETE_VAR(L_it->second) ;
  } else {
    L_ret = -1 ;
  }

  return (L_ret);
}


// External interface

T_pTransport create_ciptlsio_instance () {
  C_TransIPTLS* L_inst ;
  NEW_VAR(L_inst, C_TransIPTLS());
  return (L_inst);
}

void delete_ciptlsio_instance (T_ppTransport P_inst) {
  if (P_inst != NULL) {
    C_TransIPTLS* L_inst = (C_TransIPTLS*) *P_inst ;
    DELETE_VAR(L_inst);
    *P_inst = NULL ;
  }
}
