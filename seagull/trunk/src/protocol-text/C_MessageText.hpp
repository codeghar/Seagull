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

#ifndef _C_MESSAGE_TEXT_H
#define _C_MESSAGE_TEXT_H


#include "C_GeneratorConfig.hpp"
#include "C_MessageFrame.hpp"
#include "iostream_t.hpp"
#include "C_ProtocolTextFrame.hpp"
// #include "C_ContextFrame.hpp"
// #include "ParserFrame.hpp"
#include "TextDef.hpp"

class C_ProtocolText ;

class C_MessageText : public C_MessageFrame {
public:

  C_MessageText (C_ProtocolText *P_protocol);
  C_MessageText(C_ProtocolText *P_protocol,
                T_pValueData    P_header,
                T_pValueData    P_body,
                int            *P_id)  ;
  virtual ~C_MessageText ();

  C_MessageText(C_MessageText &P_val) ;
  

  T_pValueData get_session_id (C_ContextFrame *P_ctxt) ;
  T_pValueData get_out_of_session_id () ;
  bool         update_fields (C_MessageFrame *P_ref) ;
  bool         compare_types (C_MessageFrame *P_ref) ;

  void         update_message_stats () ;
  bool         check (C_MessageFrame    *P_ref,
		                unsigned int       P_levelMask,
		                T_CheckBehaviour   P_behave) ;
  

  bool         check_field_presence (int              P_id,
                                     T_CheckBehaviour P_behave,
                                     int P_instance,
                                     int P_sub_id) ;
  
  bool         check_field_value (C_MessageFrame  *P_ref,
                                  int              P_id,
                                  T_CheckBehaviour P_behave,
                                  int P_instance,
                                  int P_sub_id) ;

  bool         check_field_order (int              P_id,
                                  T_CheckBehaviour P_behave,
                                  int              P_position) ;


  T_TypeType   get_field_type  (int P_id,
                                int P_sub_id) ;



  bool         get_field_value (int P_id, 
                                C_RegExp *P_reg,
                                T_pValueData P_value) ;

  bool         get_field_value (int P_id, 
                                int P_instance,
                                int P_sub_id,
                                T_pValueData P_value) ;

  int          get_buffer (T_pValueData P_dest,
                           T_MessagePartType P_header_body_type) ;


  bool         set_field_value (T_pValueData P_value, 
                                int P_id,
                                int P_instance,
                                int P_sub_id) ;


  char*        name() ;

  friend iostream_output& operator<< (iostream_output&, C_MessageText &);
  void         dump(iostream_output&) ;

  int          get_id_message() ;
  void         set_id_message(int P_id) ;

  char*        get_text_value(bool P_header_body);  
  int          set_text_value(bool P_header_body, T_pValueData P_value, 
                              int P_start, int P_end);  

  T_pValueData get_data_value(bool P_header_body);  

  C_ProtocolFrame::T_MsgError encode(unsigned char *P_buf,
                                     size_t        *P_siz);
  
  unsigned long decode(unsigned char *P_buf,
                       size_t         P_siz,
                       C_ProtocolFrame::T_pMsgError P_error) ;
  

  // static void set_session_id_id (int  P_id) ;
  // static void use_open_id () ;

//    typedef C_ProtocolFrame::T_MsgError 
//    (C_MessageText::* T_BodyDecodeMethod)(int  P_index,
//                                          char *P_ptr, 
//                                          size_t *P_size);

  // ??? typedef int (C_MessageText::* T_BodyDecodeMethodInit)(void *P_param);

  C_ProtocolFrame::T_MsgError DecodeBodyWithContentLength (int P_index, 
                                                           char *P_ptr, size_t *P_size);

  C_ProtocolFrame::T_MsgError DecodeBodyWithParser (int P_index,
                                                    char *P_ptr,
                                                    size_t *P_size) ;

//    typedef C_ProtocolFrame::T_MsgError 
//    (C_MessageText::* T_EncodeMethod)(int P_index);

  // ??? typedef int (C_MessageText::* T_EncodeMethodInit)(void *P_param);



  //  static void set_names_fields (char** P_names_fields) ;

  //  static void set_number_methods (int P_nb) ;

  // static void destroy_methods () ;

//    static void set_body_decode_method (int                     P_index,
//                                        T_BodyMethodType        P_type,
//                                        T_BodyDecodeMethod      P_method,
//                                        void                   *P_param);
//    static void set_encode_method (int                 P_index,
//                                   T_BodyMethodType    P_type,
//                                   T_EncodeMethod      P_method,
//                                   void               *P_param);


  // typedef T_pValueData (C_MessageText::* T_SessionMethod)(C_ContextFrame *P_void);

  C_ProtocolFrame::T_MsgError EncodeWithContentLength (int P_index);

  C_ProtocolFrame::T_MsgError NoEncode (int P_index);


  T_pValueData getSessionFromField (C_ContextFrame *P_ctxt) ;
  T_pValueData getSessionFromOpenId (C_ContextFrame *P_ctxt) ;


protected:
private:

  C_ProtocolText            *m_protocol              ;
  T_pValueData               m_header, m_body        ;
  int                        m_id                    ;
  T_pValueData               m_session_id            ;


  // static T_SessionMethod     m_session_method        ;


//    typedef union _union_param {
//      int                 m_id ;
//      T_ParserFunction    m_parser ;
//    } T_ParamMethod, *T_pParamMethod ;

//    typedef struct _struct_body_decode_data {
//      T_BodyDecodeMethod m_decode ;
//      T_EncodeMethod     m_encode ;
//      T_ParamMethod      m_param_encode  ;
//      T_ParamMethod      m_param_decode  ;
//    } T_BodyDecodeData, *T_pBodyDecodeData ;

//    static T_BodyDecodeData **m_methods ;
//    static int                m_nb_methods ;


//    T_pValueData getSessionFromField (C_ContextFrame *P_ctxt) ;
//    T_pValueData getSessionFromOpenId (C_ContextFrame *P_ctxt) ;


} ;

typedef C_MessageText *T_pC_MessageText ;


#endif // _C_MESSAGE_TEXT_H







