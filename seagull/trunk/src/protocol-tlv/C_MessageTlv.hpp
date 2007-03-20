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

#ifndef _C_MESSAGETLV_
#define _C_MESSAGETLV_

#include "C_GeneratorConfig.hpp"
#include "C_MessageFrame.hpp"
#include "C_ProtocolTlv.hpp"
#include "C_ContextFrame.hpp"
#include "iostream_t.hpp"

#include "C_RegExp.hpp"


#define MAX_BODY_VALUES 65

class C_MessageTlv : public C_MessageFrame {

public:

   C_MessageTlv(C_ProtocolTlv *P_protocol);
  ~C_MessageTlv();

  // field management
  T_TypeType get_field_type  (int P_id,
			      int P_sub_id);

  bool       get_field_value(int P_id, 
                             C_RegExp *P_reg,
                             T_pValueData P_value) ;

  bool       get_field_value (int P_id, 
			      int P_instance,
			      int P_sub_id,
			      T_pValueData P_value);

  int        get_buffer (T_pValueData P_dest,
                         T_MessagePartType P_header_body_type);
  


  bool       set_field_value (T_pValueData P_value, 
			      int P_id,
			      int P_instance,
			      int P_sub_id);


  unsigned long  get_call_id () ;
  void           set_call_id (unsigned long P_id);

  

  virtual T_pValueData  get_session_id (C_ContextFrame *P_ctxt) ;



  unsigned long  get_type () ;
  
  void           set_type (unsigned long P_type) ;

  unsigned long  decode (unsigned char *P_buffer, 
                         size_t         P_size, 
                         C_ProtocolFrame::T_pMsgError    P_error) ;
  //   C_ProtocolBinaryFrame::T_pMsgError    P_error

  void   encode (unsigned char *P_buffer, 
                 size_t *P_size,
                 C_ProtocolFrame::T_pMsgError P_error) ;

  // C_ProtocolBinaryFrame::T_pMsgError P_error

  virtual bool           compare_types (C_MessageFrame *P_ref) ;

  virtual bool   update_fields (C_MessageFrame *P_ref) ;
  virtual void   update_message_stats  () ;

  void set_header_value (int P_id, unsigned long P_val);
  void set_header_value (char* P_name, unsigned long P_val);
  void set_header_value (char* P_name, char *P_val);
  void set_header_value (int P_id, T_pValueData P_val);

  void set_header_id_value (int P_id) ;
  void set_body_value (C_ProtocolTlv::T_pBodyValue P_val);

  void get_header_value (T_pValueData P_res, int P_id);

  T_pValueData   get_field_value (int P_id, 
                                  int P_instance,
                                  int P_sub_id) ;

  virtual bool get_body_value (T_pValueData P_res, int P_id);
  virtual bool set_body_value (int P_id, T_pValueData P_val);

  C_MessageTlv& operator= (C_MessageTlv & P_val);
  friend iostream_output& operator<< (iostream_output&, C_MessageTlv &);

  C_ProtocolTlv   *get_protocol();

  bool          check (C_MessageFrame    *P_ref,
		       unsigned int       P_levelMask,
		       T_CheckBehaviour   P_behave) ;

  // WARNING: check only body field presence
  bool          check_field_presence (int              P_id,
				      T_CheckBehaviour P_behave,
				      int              P_instance,
				      int              P_sub_id) ;

  bool          check_field_value (C_MessageFrame  *P_ref,
    				   int              P_id,
   				   T_CheckBehaviour P_behave,
   				   int              P_instance,
   				   int              P_sub_id) ;

  bool          check_field_order (int              P_id,
      				   T_CheckBehaviour P_behave,
   				   int              P_position);

  void dump(iostream_output& P_stream) ;


  char* name();
  

  int get_id_message() ;



protected:

  C_ProtocolTlv          *m_protocol ;
  unsigned long              m_call_id ;

  T_ValueData    m_id               ;
  T_pValueData   m_header_values    ;

  char          *m_header_body_field_separator     ;

  // temporary
  // linked with protocol
  int                              m_header_id ;
  int                              m_nb_body_values ;
  C_ProtocolTlv::T_pBodyValue   m_body_val ;
  virtual T_pValueData  get_out_of_session_id () ;
  

} ; // class C_MessageTlv



typedef C_MessageTlv *T_pC_MessageTlv ;

#endif // _C_MESSAGETLV_

