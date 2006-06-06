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

#ifndef _C_PROTOCOLBINARYBODYNOTINTERPRETED_H_
#define _C_PROTOCOLBINARYBODYNOTINTERPRETED_H_
#include "C_ProtocolBinary.hpp"

#include "map_t.hpp"


class C_ProtocolBinaryBodyNotInterpreted : public C_ProtocolBinary {

public:

  C_ProtocolBinaryBodyNotInterpreted() ;
  
  virtual ~C_ProtocolBinaryBodyNotInterpreted();


  virtual C_MessageFrame*     create_new_message (void                *P_xml,
                                                  T_pInstanceDataList  P_list,
                                                  int                 *P_nb_value);
  
  virtual C_MessageFrame*     create_new_message(C_MessageFrame *P_msg) ;


  virtual int   decode_body   (unsigned char *P_buf, 
                               size_t         P_size,
                               T_pBodyValue   P_valDec,
                               int           *P_nbValDec,
                               int           *P_headerId);

  virtual C_ProtocolFrame::T_MsgError  encode_body (int            P_nbVal, 
                                                    T_pBodyValue   P_val,
                                                    unsigned char *P_buf, 
                                                    size_t        *P_size) ;

  virtual T_ManagementSessionId* get_manage_session_elt (int P_id);
  virtual int get_nb_management_session () ;
  virtual bool check_present_session (int P_msg_id,int P_id) ;
  virtual bool find_present_session (int P_msg_id,int P_id) ;

  virtual iostream_output& print_body  (iostream_output&  P_stream, 
                                        int          P_nb,
                                        T_pBodyValue P_val,
                                        int          P_level = 0) ;

  typedef struct _struct_header_body_size_pos {
    size_t m_position;
    size_t m_size;
  } T_HeaderBodyPositionSize, 
    *T_pHeaderBodyPositionSize;

  typedef map_t<int,T_HeaderBodyPositionSize> T_HeaderBodyPositionSizeMap, 
    *T_pHeaderBodyPositionSizeMap ;

  void get_field_position (T_pHeaderBodyPositionSize P_pos, int P_id) ;

protected:

  bool                       **m_field_present_table         ;

  virtual int xml_interpretor(C_XmlData *P_def);
  virtual int analyze_body_from_xml (C_XmlData *P_def);
  virtual int get_header_body_values_from_xml (C_XmlData *P_def) ;
  virtual int get_header_values_from_xml (C_XmlData *P_def) ;

  virtual int analyze_sessions_id_from_xml (C_XmlData *P_def) ;

  T_pHeaderBodyPositionSizeMap     m_header_body_position_size_map ;

  T_pManagementSessionId           m_value_sessions_table      ;
  int                              m_value_sessions_table_size ;

} ;

#endif // _C_PROTOCOLBINARYBODYNOTINTERPRETED_H_




