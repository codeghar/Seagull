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

#ifndef    _C_PROTOCOLFRAME_H
#define    _C_PROTOCOLFRAME_H

#include "C_MessageFrame.hpp"
#include "list_t.hpp"
#include "ExternalMethod.h"

class C_ProtocolStatsFrame ;

typedef struct _struct_inst_data {
  char *m_instance_name  ;
  int   m_instance_id    ;
  int   m_id             ;
} T_InstanceData, *T_pInstanceData ;

typedef list_t<T_InstanceData> T_InstanceDataList, 
  *T_pInstanceDataList ;

typedef struct _struct_name_id {
  char *m_name;
  int   m_id  ;
} T_NameAndId, *T_pNameAndId ;
typedef list_t<T_NameAndId> T_NameAndIdList, *T_pNameAndIdList ;


class C_ProtocolFrame { // Protocol Frame Definition Class

public:

  typedef enum _protocol_type {
    E_PROTOCOL_BINARY,
    E_PROTOCOL_EXTERNAL,
    E_PROTOCOL_TEXT,
    E_PROTOCOL_UNKNOWN
  } T_ProtocolType, *T_pProtocolType ;

  typedef enum _msg_error_code {
    E_MSG_OK = 0,
    E_MSG_ERROR_DECODING,
    E_MSG_ERROR_DECODING_SIZE_LESS,
    E_MSG_ERROR_ENCODING,
    E_MSG_EXTERNAL_ERROR, // used for EXTERNAL protocol type
    E_MSG_INTERNAL_ERROR  // used for EXTERNAL protocol type
  } T_MsgError, *T_pMsgError ;


           C_ProtocolFrame() {m_type = E_PROTOCOL_UNKNOWN; m_name=NULL; m_stats = NULL;} ;
  virtual ~C_ProtocolFrame() {} ;

  T_ProtocolType get_type() { return (m_type); }


  virtual C_MessageFrame* create_new_message (C_MessageFrame *P_msg) = 0 ;

  // void * for P_Xml parameter: 
  //     Xml internal format not exported for transport  
  virtual C_MessageFrame* create_new_message (void               *P_xml,
					      T_pInstanceDataList P_list,
					      int                *P_nb_body_value) = 0 ;

  virtual char*            message_name                 () = 0 ;
  virtual char*            message_component_name       () = 0 ;
  virtual T_pNameAndIdList message_name_list            () = 0 ;
  virtual T_pNameAndIdList message_component_name_list  () = 0 ;
  virtual bool             find_present_session (int P_msg_id,int P_id) = 0 ;



  // find field id from field name (to be used in the dictionnary)
  virtual int              find_field (char *P_name)          = 0 ;
  virtual T_TypeType       get_field_type (int P_id,
					   int P_sub_id)      = 0 ;
  virtual bool             check_sub_entity_needed (int P_id) = 0 ;

  C_ProtocolStatsFrame*    get_stats () { return (m_stats); }
  void                     set_stats (C_ProtocolStatsFrame *P_stats) {
    m_stats = P_stats ;
  }

  virtual T_ExternalMethod find_method_extern(char *P_name){return NULL;} 


  char* name() { return(m_name); }

protected:

  C_ProtocolStatsFrame *m_stats ;
  T_ProtocolType        m_type ;
  char                 *m_name ;

} ;

typedef C_ProtocolFrame *T_pC_ProtocolFrame ;


#endif  // _C_PROTOCOLFRAME_H




