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

#ifndef _C_TRANSOCTCAP32_
#define _C_TRANSOCTCAP32_

#include "C_Transport.hpp"
#include "C_TcapStack.hpp"

#include "C_TCAPCnxInfo.hpp"


class C_TransOCTcap32 : public C_Transport {

public:
  C_TransOCTcap32() ;
  ~C_TransOCTcap32() ;

  int init (char* P_buf, 
	    T_logFunction P_logError, 
	    T_logFunction P_logInfo )  ;

  int config (T_pConfigValueList P_config_param_list) ;
  int close ()  ;

  int open (int              P_channel_id,
	    char*            P_buf, 
	    T_pOpenStatus    P_status,
	    C_ProtocolFrame *P_protocol)  ;

  int close (int P_id)  ;

  int pre_select (int                   P_n, 
		  fd_set               *P_readfds,  
		  fd_set               *P_writefds,
		  fd_set               *P_exceptfds, 
		  struct timeval       *P_timeout, 
		  int                  *P_cnx, 
		  size_t                P_nb)  ;


  int             post_select     (int                  P_n, 
				   fd_set              *P_readfds,  
				   fd_set              *P_writefds,
				   fd_set              *P_exceptfds, 
				   T_pC_TransportEvent  P_eventTable,
				   size_t              *P_nb)  ;
  
  bool            get_message(int P_id, T_pReceiveMsgContext P_ctxt)  ;
  int             send_message(int P_id, C_MessageFrame *P_msg)  ;

  T_SelectDef     select_definition () ;


protected:
private:

  typedef enum _flavour_t {
    E_FLAVOUR_ITU,
    E_FLAVOUR_ANSI,
    E_FLAVOUR_UNKNOWN
  } T_FlavourKey, *T_pFlavourKey ;

  T_FlavourKey  m_flavour ;
  char         *m_library_name ;

  C_TcapStack  *m_stack ;
  int           m_cnx_id ;


  // T_CnxInfoMap  m_cnx_info_map  ;

  T_logFunction m_log_error, m_log_info ;


  int          analyze_init_string (char *P_buf);
  bool         analyze_open_string (char *P_buf, T_pCnxInfo P_cnx_info) ;
  
  char*        find_value(char *P_buf, char *P_field);
  
  T_FlavourKey analyze_flavour(char *P_name);
  int          find_library(char *P_name, char *P_path, char *P_lib);
  
  int          determine_lib (char *P_name);
  char*        verify_path (char * P_libPath, char * P_libName) ;




} ;

extern "C" T_pTransport create_ctransoctcap_instance () ;
extern "C" void         delete_ctransoctcap_instance (T_ppTransport) ;


#endif











