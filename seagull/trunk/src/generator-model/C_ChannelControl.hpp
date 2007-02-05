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

#ifndef _C_CHANNELCONTROL_H
#define _C_CHANNELCONTROL_H

#include "string_t.hpp"
#include "map_t.hpp"

#include "C_IdGenerator.hpp"
#include "C_XmlData.hpp"

#include "C_MessageFrame.hpp"
#include "C_TransportControl.hpp"
#include "C_ProtocolControl.hpp"
#include "C_SemaphoreTimed.hpp"


#define ERROR_CHANNEL_UNKNOWN -1
typedef map_t<string_t, int> T_ChannelNameMap, *T_pChannelNameMap ;

class C_ChannelControl {

public:

  typedef enum _channel_type {
    E_CHANNEL_GLOBAL,
    E_CHANNEL_LOCAL,
    E_CHANNEL_UNKNOWN
  } T_ChannelType, *T_pChannelType ;

  typedef enum _channel_open_status {
    E_CHANNEL_CLOSED,
    E_CHANNEL_OPEN_IN_PROGESS,
    E_CHANNEL_OPENED
  } T_ChannelStatus, *T_pChannelStatus ;

  typedef struct _channel_data {

    int              m_id           ;
    T_ChannelType    m_type         ;
    int              m_protocol_id  ;
    int              m_transport_id ;
    C_ProtocolFrame *m_protocol     ;
    C_Transport     *m_transport    ;
    char            *m_open_args    ;
    //    bool             m_open_status  ;
    T_ChannelStatus  m_open_status  ;
    int              m_open_id      ;
    bool             m_reconnect    ;

    char            *m_name         ;
    // response id list ?

  } T_ChannelData,  *T_pChannelData ;
    
   C_ChannelControl() ;
  ~C_ChannelControl() ;

  bool fromXml (C_XmlData          *P_data,
		C_ProtocolControl  *P_protocol_ctrl,
		C_TransportControl *P_transport_ctrl) ;

  int            get_channel_id   (char *P_name);
  char*          get_channel_name (int P_id);
  T_pChannelData get_channel_data (char *P_name);
  T_pChannelData get_channel_data (int P_id);

  int            open_global_channel () ;
  int            check_global_channel () ;
  int            open_local_channel (int           P_id, 
                                     char         *P_args,
				     int          *P_table, 
				     T_pOpenStatus P_status);



  int            set_option_global_channel (int   P_id,
                                            char *P_args,
                                            int  *P_table) ;


  void           close_local_channel (int P_id, int*P_table);



  int            nb_channel () ;
  int            nb_global_channel() ;
  int            open_id_from_channel_id (int P_id, int *P_table);
  int            send_to_channel (int P_id, int *P_table, C_MessageFrame *P_msg);

  void           reset_channel (int *P_table);

  void           opened (int P_id, int P_open_id);
  void           open_failed (int P_id, int P_open_id);
  void           closed (int P_id, int P_open_id);
  
  int            opened () ;

  C_Transport**  get_transport_table (int *P_size) ;

  C_ProtocolFrame *get_channel_protocol (int P_id) ;

  bool            reconnect() ;

protected:
private:

  T_pChannelNameMap   m_name_map           ;
  T_pChannelData     *m_channel_table      ;
  int                 m_channel_table_size ;
  T_pC_IdGenerator    m_id_gen             ;

  int                 m_nb_global_channel     ;
  int                *m_channel_to_ctxt_table ;
  int                *m_ctxt_to_channel_table ;

  C_Transport        **m_transport_table ;
  int                  m_nb_transport    ;

  bool                 m_reconnect ;
  C_SemaphoreTimed    *m_sem_reconnect ;

  void create_context_channel () ;
  void create_transport_table (C_TransportControl *P_transport_ctrl) ;
  
} ;

#endif
