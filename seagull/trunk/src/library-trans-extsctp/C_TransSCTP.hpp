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
#ifndef _C_TRANS_SCTP_
#define _C_TRANS_SCTP_

#include "C_TransIP.hpp"
#include "C_SocketSCTP.hpp"

class C_TransSCTP : public C_TransIP {

public:
  C_TransSCTP();
  virtual ~C_TransSCTP();
  virtual int         config (T_pConfigValueList P_config_param_list) ;
  
  T_SelectDef select_definition () ;
  
protected : 
  virtual C_Socket* open (int              P_channel_id, 
			                    T_pIpAddr        P_Addr,
			                    T_pOpenStatus    P_status,
			                    C_ProtocolBinaryFrame *P_protocol) ;

  int analyze_config(T_ConfigValue& P_config) ;
protected :


} ;


extern "C" T_pTransport create_cipsctpio_instance () ;
extern "C" void         delete_cipsctpio_instance (T_ppTransport) ;

#endif // _C_TRANS_SCTP_
