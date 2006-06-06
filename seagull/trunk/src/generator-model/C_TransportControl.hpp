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

#ifndef _C_TRANSPORTCONTROL_H
#define _C_TRANSPORTCONTROL_H


#include "string_t.hpp"
#include "map_t.hpp"
#include "select_t.hpp"

#include "C_IdGenerator.hpp"
#include "C_XmlData.hpp"
#include "C_Transport.hpp"

#include "C_ProtocolFrame.hpp"


#define ERROR_TRANSPORT_UNKNOWN -1
typedef map_t<string_t, int> T_TransportNameMap, *T_pTransportNameMap ;

class C_TransportControl {

public:

  typedef struct _transport_context {
    int                        m_id       ;
    T_pTransport               m_instance ;
    T_TransportCreateInstance  m_ext_create ;
    T_TransportDeleteInstance  m_ext_delete ;
    void                      *m_lib_handle ;
  } T_TransportContext, *T_pTransportContext ;

   C_TransportControl(T_logFunction P_logError, 
		      T_logFunction P_logInfo) ;
  ~C_TransportControl() ;

  bool fromXml (C_XmlData *P_data) ;

  bool set_config_value(T_pConfigValueList P_config_value_list) ;

  C_Transport*        get_transport         (char *P_name);
  C_Transport*        get_transport         (int   P_id);
  T_pTransportContext get_transport_context (int   P_id);
  int                 get_transport_id      (char *P_name);

  T_SelectDef  get_call_select () ;

protected:

private:

  T_pTransportNameMap   m_name_map       ;
  T_pTransportContext  *m_transport_context_table ;
  int                   m_transport_table_size ;
  T_pC_IdGenerator      m_id_gen         ;

  T_logFunction         m_log_error, m_log_info ;

  bool                  update_context(T_pTransportContext  P_context,
				       void                *P_value);

  T_SelectDef           m_call_select ;

} ;

#endif


