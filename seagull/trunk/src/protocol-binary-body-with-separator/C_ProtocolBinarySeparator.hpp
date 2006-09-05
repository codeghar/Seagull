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

#ifndef _C_PROTOCOLBINARYSEPARATOR_
#define _C_PROTOCOLBINARYSEPARATOR_

#include "C_ProtocolBinary.hpp"

class C_ProtocolBinarySeparator : public C_ProtocolBinary {

public:

  C_ProtocolBinarySeparator() ;
  ~C_ProtocolBinarySeparator() ;

  virtual C_ProtocolFrame::T_MsgError       
  encode_body (int            P_nbVal, 
	       T_pBodyValue   P_val, 
	       unsigned char *P_buf, 
	       size_t        *P_size) ;

  virtual C_MessageFrame*    create_new_message (C_MessageFrame *P_msg);
  virtual C_MessageFrame*    create_new_message (void                *P_xml,
                                                 T_pInstanceDataList  P_list,
                                                 int                 *P_nb_value);

  virtual int 
  decode_body (unsigned char *P_buf, 
	       size_t         P_size,
	       T_pBodyValue   P_valDec,
	       int           *P_nbValDec,
	       int           *P_headerId);

  virtual void  construction_data( C_XmlData            *P_def, 
				   char                **P_name,
				   T_pContructorResult   P_res) ;

private:
  size_t  m_header_body_field_separator_size ;
  int     m_session_id ;


} ; // class C_ProtocolBinarySeparator

#endif // _C_PROTOCOLBINARYSEPARATOR_


