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

#ifndef    _C_MESSAGEFRAME_H
#define    _C_MESSAGEFRAME_H

#include "message_check_t.h"
#include "iostream_t.hpp"

#include "list_t.hpp"
#include "ProtocolDataType.hpp"
#include "ProtocolStats.hpp"
#include "C_ContextFrame.hpp"

#include "C_RegExp.hpp"
#include "message_header_body_t.h"

class C_MessageFrame { // Message Definition Class

public:

           C_MessageFrame() {} ;
  virtual ~C_MessageFrame() {} ;

  virtual T_pValueData get_session_id (C_ContextFrame *P_ctxt) = 0 ;

  virtual bool         compare_types (C_MessageFrame *P_ref) = 0 ;
  virtual bool         update_fields (C_MessageFrame *P_ref) = 0 ;

  virtual void         update_message_stats () = 0 ;

  virtual bool         check         (C_MessageFrame  *P_ref, 
				      unsigned int     P_levelMask,
				      T_CheckBehaviour P_behave) = 0 ;
  virtual bool         check_field_presence (int              P_id,
					     T_CheckBehaviour P_behave,
                                             int P_instance,
					     int P_sub_id) = 0 ;
  
  virtual bool         check_field_value (C_MessageFrame  *P_ref,
					  int              P_id,
					  T_CheckBehaviour P_behave,
					  int P_instance,
					  int P_sub_id) = 0 ;

  virtual bool         check_field_order (int              P_id,
					  T_CheckBehaviour P_behave,
					  int              P_position) = 0 ;


  virtual T_TypeType   get_field_type  (int P_id,
					int P_sub_id)           = 0 ;

  virtual bool         get_field_value (int P_id, 
					C_RegExp * P_reg,
					T_pValueData P_value) = 0 ;


  virtual bool         get_field_value (int P_id, 
					int P_instance,
					int P_sub_id,
					T_pValueData P_value) = 0 ;

  virtual T_pValueData   get_field_value (int P_id, 
                                          int P_instance,
                                          int P_sub_id) = 0 ;


  virtual int         get_buffer (T_pValueData P_dest,
                                  T_MessagePartType P_header_body_type) = 0 ;

  virtual bool         set_field_value (T_pValueData P_value, 
					int P_id,
					int P_instance,
					int P_sub_id) = 0 ;
  virtual void         dump(iostream_output&) = 0 ;
  virtual char*        name() = 0 ;

  virtual int          get_id_message() = 0 ;



  friend iostream_output& operator<< 
  (iostream_output& P_stream, C_MessageFrame & P_msg) {P_msg.dump(P_stream); 
  return(P_stream);} ;

protected:
private:

} ;



typedef C_MessageFrame *T_pC_MessageFrame ;
typedef list_t<T_pC_MessageFrame>  T_MessageFrameList,
                                  *T_pMessageFrameList ;


#endif  // _C_MESSAGEFRAME_H

