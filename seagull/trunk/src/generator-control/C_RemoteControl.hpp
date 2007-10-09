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

#ifndef _C_REMOTECONTROL_H
#define _C_REMOTECONTROL_H

#include "select_t.hpp"

#include "C_TaskControl.hpp"
#include "list_t.hpp"
#include "C_TransportControl.hpp"
#include "C_ProtocolFrame.hpp"
#include "C_GeneratorConfig.hpp"
#include "C_ProtocolControl.hpp"
#include "C_ProtocolText.hpp"
#include "C_GeneratorStats.hpp"


class C_Generator ;


class C_RemoteControl : public C_TaskControl {

 public:

  C_RemoteControl(C_Generator* P_gen,
                  C_ProtocolControl* P_protocol_control,
                  char* P_address) ;
  ~C_RemoteControl() ;
  
  void  init() ;

private :

  C_Generator         *m_gen ;

  C_Transport         *m_transport ;
  size_t               m_max_event_nb ;
  struct timeval       m_select_timeout ;
  C_GeneratorConfig   *m_config ;
  T_SelectDef          m_call_select ;
  T_pC_TransportEvent  m_events ;
  C_ProtocolControl   *m_protocol_ctrl  ;
  C_ProtocolFrame     *m_protocol_frame ;
  C_ProtocolText      *m_protocol_text  ;

  T_pRcvMsgCtxtList    m_msg_remote_list ;
  char                *m_address ;

  C_GeneratorStats    *m_stat ;

  int                  m_command_id, m_uri_id ;




  T_GeneratorError TaskProcedure () ;
  T_GeneratorError InitProcedure () ;
  T_GeneratorError EndProcedure () ;
  T_GeneratorError StoppingProcedure () ;
  T_GeneratorError ForcedStoppingProcedure () ;

  T_GeneratorError receiveControl () ;
  void             messageReceivedControl () ;

  char* execute_command(char *P_cmd, char *P_uri, char **P_result_data);
  char* decode_put_uri(char *P_uri);
  char* decode_get_uri(char *P_uri, char **P_result_data);

  char* decode_command_uri(char *P_uri);
  char* decode_data_uri(char *P_uri,char **P_result_data);


  char* find_directory(char *P_buf,char *P_dir) ;
  char* find_file(char *P_buf,char *P_dir) ;
  char* find_value(char *P_buf,char *P_dir) ;

  void rate(unsigned long P_value);
  void ramp(unsigned long P_value, unsigned long P_duration);

  void quit();
  void pause();
  void resume();
  void burst();
  void increase();
  void decrease();
  void scale(unsigned long P_value);
  void resetcumul();
  void percent();
  void forceinit();


  char* resultOK();
  char* resultKO();

  void createRampThread(unsigned long P_duration,
                        unsigned long P_current_rate,
                        unsigned long P_sub_rate,
                        bool          P_increase);

  C_MessageFrame* analyze_command(C_MessageFrame *P_msg) ;
  
} ;

#endif // _C_REMOTECONTROL_H






