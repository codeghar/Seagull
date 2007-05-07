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

#ifndef _C_MESSAGEEXTERNAL_H
#define _C_MESSAGEEXTERNAL_H

#include "iostream_t.hpp"
#include "C_MessageFrame.hpp"
#include "C_GeneratorConfig.hpp"
#include "C_ContextFrame.hpp"

#include "C_RegExp.hpp"

class C_ProtocolExternal ;


class C_MessageExternal : public C_MessageFrame {
public:
  C_MessageExternal ();
  C_MessageExternal (C_MessageExternal&);
  virtual ~C_MessageExternal ();

  T_pValueData get_session_id (C_ContextFrame *P_ctxt) ;
  T_pValueData get_type () ;

  bool         compare_types (C_MessageFrame *P_ref) ;
  bool         update_fields (C_MessageFrame *P_ref) ;

  void         update_message_stats ()      ;

  bool         check (C_MessageFrame  *P_ref, 
		      unsigned int     P_levelMask,
		      T_CheckBehaviour P_behave) ;
  bool         check_field_presence (int              P_id,
				     T_CheckBehaviour P_behave,
				     int              P_instance,
				     int              P_sub_id) ;


  bool         check_field_value (C_MessageFrame  *P_ref,
				  int              P_id,
				  T_CheckBehaviour P_behave,
				  int              P_instance,
				  int              P_sub_id) ;

  bool         check_field_order (int              P_id,
				  T_CheckBehaviour P_behave,
				  int              P_position);


  T_TypeType   get_field_type  (int P_id,
				int P_sub_id)                   ;

  friend class C_ProtocolExternal ;


  bool       get_field_value(int P_id, 
                             C_RegExp *P_reg,
                             T_pValueData P_value) ;


  int         get_buffer (T_pValueData P_dest,
                          T_MessagePartType P_header_body_type) ;


  T_pValueData get_field_value (int P_id, 
                                C_ContextFrame *P_ctxt,
                                int P_instance,
                                int P_sub_id) ;

  bool         get_field_value (int P_id, 
				int P_instance,
				int P_sub_id,
				T_pValueData P_value) ;

  bool         set_field_value (T_pValueData P_value, 
				int P_id,
				int P_instance,
				int P_sub_id) ;

  C_MessageExternal& operator= (C_MessageExternal & P_val);

  friend iostream_output& operator<< (iostream_output&, C_MessageExternal &);

  void dump(iostream_output&);

  char* name();

  int get_id_message() ;



  static void set_protocol_data (int P_nb_header_fields,
				 int P_nb_body_values,
				 int P_nb_body_fields,
				 int P_session_id,
				 int P_outof_session_id,
				 int P_type_id,
				 char** P_names,
				 char** P_message_names,
				 bool** P_body_not_present_table,
				 bool** P_header_not_present_table);


  static int get_nb_header_fields () ;
  static int get_nb_body_values   () ;
  static int get_nb_body_fields   () ;
  
protected:
private:

  typedef T_pValueData (C_MessageExternal::* T_GetId)(int);
  typedef T_pValueData (C_MessageExternal::* T_GetId_instance)(int, int);

  static int m_nb_header_fields         ;
  static int m_nb_body_values           ;
  static int m_nb_body_fields           ;

  static int m_session_id               ;
  static int m_outof_session_id         ;
  static int m_type_id                  ;

  static T_GetId_instance m_get_id               ;

  static T_GetId_instance  m_get_outof_session_id ;
  static T_GetId m_get_type             ;

  static char ** m_names              ;
  static char ** m_message_names      ;

  static bool ** m_body_not_present_table      ;
  static bool ** m_header_not_present_table    ;

  C_ProtocolExternal *m_protocol    ;
  T_pValueData        m_header      ;

  T_pMultiValueData   m_body       ;
  int                 m_nb_values  ;


  T_pValueData       *m_all_values ;
  int                *m_ids        ;

  int                 m_id          ;

  T_pValueData       m_correlation_session_id ;


  T_pValueData get_out_of_session_id () ;
  void internal_reset () ;

  C_MessageExternal (C_ProtocolExternal *P_protocol,
		     T_pValueData        P_header, 
		     T_pValueDataList    P_body,       
		     list_t<int>        *P_body_id,
		     int                *P_instance_body);



  T_pValueData get_id_header (int P_id, int P_instance)     ;

  T_pValueData get_id_body (int P_id, int P_instance)       ;

  T_pValueData  get_type_header (int P_id) ;

  T_pValueData  get_type_body (int P_id) ;



} ;

#endif // _C_MESSAGEEXTERNAL_H







