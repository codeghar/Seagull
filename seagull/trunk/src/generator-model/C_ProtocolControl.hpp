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

#ifndef _C_PROTOCOLCONTROL_H
#define _C_PROTOCOLCONTROL_H

#include "string_t.hpp"
#include "map_t.hpp"

#include "C_IdGenerator.hpp"
#include "C_XmlData.hpp"
#include "C_ProtocolFrame.hpp"

#include "C_TransportControl.hpp"

#include "C_GeneratorConfig.hpp"
#include "C_ProtocolStats.hpp"


#define ERROR_PROTOCOL_UNKNOWN -1
typedef map_t<string_t, int> T_ProtocolNameMap, *T_pProtocolNameMap ;

typedef struct _protocol_instance_info {
  C_ProtocolFrame *m_instance ;
  int              m_id       ;
  char            *m_name     ;
} T_ProtocolInstanceInfo, *T_pProtocolInstanceInfo ;
typedef list_t<T_pProtocolInstanceInfo> T_ProtocolInstList ;

class C_ProtocolControl {

public:
   C_ProtocolControl(C_TransportControl *P_transport_control) ;
  ~C_ProtocolControl() ;

  bool fromXml (C_XmlData *P_data,
		T_pConfigValueList P_config_value_list,
		bool               P_display_protocol_stats) ;


  C_ProtocolFrame* get_protocol      (char *P_name);
  C_ProtocolFrame* get_protocol      (int   P_id);
  char*            get_protocol_name (int   P_id);
  int              get_protocol_id   (char *P_name);

  int              get_nb_protocol ();



protected:
private:

  T_pProtocolNameMap  m_name_map       ;
  T_pC_ProtocolFrame *m_protocol_table ;
  char              **m_protocol_name_table ;
  int                 m_protocol_table_size ;
  T_pC_IdGenerator    m_id_gen         ;

  C_TransportControl *m_transport_control ;

  // internal methods
  char* get_protocol_name(C_XmlData *);
  char* get_protocol_type(C_XmlData *);


} ;

#endif







