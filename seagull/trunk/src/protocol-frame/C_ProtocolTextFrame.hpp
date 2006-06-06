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

#ifndef    _C_PROTOCOL_TEXT_FRAME_H
#define    _C_PROTOCOL_TEXT_FRAME_H

// Protocol Text Frame Definition Class

#include "C_ProtocolFrame.hpp"

class C_ProtocolTextFrame : public C_ProtocolFrame { 

public:
//    typedef enum _msg_error_code {
//      E_MSG_OK = 0,
//      E_MSG_ERROR_DECODING,
//      E_MSG_ERROR_DECODING_SIZE_LESS,
//      E_MSG_ERROR_ENCODING
//    } T_MsgError, *T_pMsgError ;

           C_ProtocolTextFrame() { m_type = E_PROTOCOL_TEXT; } ;
  virtual ~C_ProtocolTextFrame() {} ;


  virtual  C_MessageFrame *decode_message (unsigned char *P_buffer, 
					   size_t        *P_size, 
					   T_pMsgError    P_error) = 0 ;

  virtual  T_MsgError      encode_message (C_MessageFrame *P_msg,
  				           unsigned char  *P_buffer,
  				           size_t         *P_buffer_size) = 0 ;

  virtual  void            log_buffer     (char          *P_header,
			                   unsigned char *P_buffer,
			                   size_t         P_buffer_size) = 0 ;
protected:
private:

} ;

typedef C_ProtocolTextFrame *T_pC_ProtocolTextFrame ;

#endif  // _C_PROTOCOL_TEXT_FRAME_H


