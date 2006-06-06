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

#ifndef _C_PROTOCOLBINARY_
#define _C_PROTOCOLBINARY_

#include "Utils.hpp"
#include "C_XmlData.hpp"

#include "iostream_t.hpp"
#include "string_t.hpp"
#include "map_t.hpp"
#include "C_ProtocolStats.hpp"

#include "C_ProtocolBinaryFrame.hpp"

class C_ProtocolBinary : public C_ProtocolBinaryFrame {

public:

  typedef enum _enum_msg_id_type {
    E_MSG_ID_HEADER = 0,
    E_MSG_ID_BODY
  } T_MsgIdType, *T_pMsgIdType ;

  typedef struct _cond_value {
    unsigned long m_mask ;
    int           m_f_id ;
  } T_CondPresence, *T_pCondPresence ;

  typedef map_t<string_t, int> T_IdMap, *T_pIdMap ;
  typedef map_t<unsigned long, int> T_DecodeMap, *T_pDecodeMap ;

  typedef struct _struct_header_field {
    int             m_id   ;
    char           *m_name ;
    unsigned long   m_size ;
    T_pCondPresence m_cond_presence ;
    int             m_type_id;  
  } T_HeaderField, *T_pHeaderField ;

  typedef struct _struct_header_value {
    int           m_id     ;
    char         *m_name   ;
    int           m_nb_set ;
    bool         *m_value_setted ;
    int          *m_id_value_setted ;
    T_pValueData  m_values ;
  } T_HeaderValue, *T_pHeaderValue ;

  typedef struct _struct_header_body_value {
    int           m_id     ;
    char         *m_name   ;
    int           m_type_id;
    int           m_nb_set ;
    bool         *m_value_setted ;
    int          *m_id_value_setted ;
    T_pValueData  m_values ;
  } T_HeaderBodyValue, *T_pHeaderBodyValue ;

  typedef struct _struct_body_value {
    int                 m_id      ;
    T_Value             m_value   ;
    _struct_body_value *m_sub_val ;
  } T_BodyValue, *T_pBodyValue ;


  typedef struct _struct_body_def {
    int          m_nb_values   ;
    T_pBodyValue m_value_table ;
  } T_BodyValueDef, *T_pBodyValueDef ;


  // protocol msg id management (session-id) and (out-of-session-id)
  typedef struct _struct_management_session_id {
    int                m_msg_id_id   ;
    T_MsgIdType        m_msg_id_type ;
    T_TypeType         m_msg_id_value_type ;
  } T_ManagementSessionId, *T_pManagementSessionId;

  typedef list_t<T_ManagementSessionId> T_ManagementSessionIdList, *T_pManagementSessionIdList ;

  C_ProtocolBinary() ;
  void  construction_data( C_XmlData            *P_def, 
                           char                **P_name,
                           T_pContructorResult   P_res) ;

  ~C_ProtocolBinary() ;

  virtual C_MessageFrame*           decode_message (unsigned char *P_buffer, 
                                                    size_t        *P_size, 
                                                    C_ProtocolFrame::T_pMsgError    P_error) ;

  C_ProtocolFrame::T_MsgError encode_message (C_MessageFrame *P_msg,
                                              unsigned char  *P_buffer,
                                              size_t         *P_buffer_size);

  virtual C_MessageFrame*                   create_new_message (C_MessageFrame *P_msg);
  virtual C_MessageFrame*                   create_new_message (void                *P_xml,
                                                                T_pInstanceDataList  P_list,
                                                                int                 *P_nb_value);
  char*                             message_name();
  char*                             message_component_name() ;
  T_pNameAndIdList                  message_name_list     () ;
  T_pNameAndIdList                  message_component_name_list  () ;

  char*                             message_name(int P_headerId);

  void log_buffer (char *P_header, unsigned char *P_buf, size_t P_size);


  // field management
  int        find_field      (char *P_name);
  T_TypeType get_field_type  (int P_id,
			      int P_sub_id);
  bool           check_sub_entity_needed     (int P_id)      ;

  int        retrieve_field_id(int P_id, T_pMsgIdType P_type);

  void           update_stats (T_ProtocolStatDataType   P_type,
			       T_ProtocolStatDataAction P_action,
			       int                      P_id) ;
  




  T_MsgIdType get_msg_id_type () ;
  int         get_msg_id () ;
  T_TypeType  get_msg_id_value_type () ;

  T_MsgIdType get_out_of_session_id_type () ;
  int         get_out_of_session_id () ;


  unsigned long      decode_msg_size (unsigned char* P_buffer, 
				      unsigned long  P_size);

  int                get_header_type_id_body () ;

  int                get_header_type_id () ;
  int                get_header_length_id () ;
  int                get_header_length_index () ;
  unsigned long      get_nb_field_header () ;
  unsigned long      get_header_size () ;
  T_pHeaderField     get_header_field_description (int P_id);
  T_pHeaderField     get_header_field_description (char *P_name);

  T_pHeaderValue     get_header_value_description (int P_id) ;

  T_pTypeDef         get_type_description (int P_id);
  T_pTypeDef         get_type_description (char *P_name);

  int                get_header_body_type_id () ;
  int                get_header_body_length_id () ;
  int                get_header_body_length_index () ;
  unsigned long      get_nb_field_header_body () ;
  unsigned long      get_header_body_size () ;
  T_pHeaderField     get_header_body_field_description (int P_id);
  T_pHeaderField     get_header_body_field_description (char *P_name);

  T_pHeaderBodyValue get_header_body_value_description (int P_id);

  unsigned long      get_m_max_nb_field_header () ;


  char*              get_header_name();
  char*              get_header_body_name () ;

  int                get_header_value_id (char* P_name);
  int                get_header_body_value_id (char* P_name);

  int                set_body_value(int P_id, char* P_value, 
				    int P_nb, T_pBodyValue P_res,
				    bool        *P_del = NULL);

  int                set_body_sub_value(int P_index, 
					int P_id, 
					char* P_value, 
					T_pBodyValue P_res);
  void               set_body_value(T_pBodyValue P_res, T_pBodyValue P_val);

  void               delete_header_value(T_pValueData P_res);
  void               delete_body_value(T_pBodyValue P_res);
  void               reset_grouped_body_value(T_pBodyValue P_res);


  void               encode_header (int P_id, unsigned char*P_buf, size_t *P_size) ;

  void               encode_header (int P_id, 
				    T_pValueData  P_headerVal, 
				    unsigned char*P_buf, size_t *P_size,
                                    C_ProtocolFrame::T_pMsgError P_error); 

  virtual C_ProtocolFrame::T_MsgError       encode_body   (int            P_nbVal, 
                                                           T_pBodyValue   P_val, 
                                                           unsigned char *P_buf, 
                                                           size_t        *P_size) ;

  int                decode_header (unsigned char *P_buf, 
				    size_t         P_size, 
				    T_pValueData   P_valDec);
  virtual int        decode_body   (unsigned char *P_buf, 
				    size_t         P_size,
				    T_pBodyValue   P_valDec,
				    int           *P_nbValDec,
				    int           *P_headerId);


  void               update_length (unsigned char*P_buf, size_t P_size);

  int find_header_field_id (char *P_name) ;
  int find_header_body_id (char *P_name) ;
  int find_body_value_id (char *P_name) ;

  void get_header_values (int P_id, T_pValueData P_val);
  void set_header_values (T_pValueData P_dest, T_pValueData P_orig) ;
  void set_header_value (int P_id, T_pValueData P_dest, T_pValueData P_orig) ;
  void set_body_values (int P_nb, T_pBodyValue P_dest, T_pBodyValue P_orig);

  void reset_header_values (int P_nb, T_pValueData P_val);
  void reset_body_values (int P_nb, T_pBodyValue P_val);

  void get_body_values (int P_id, T_pBodyValue P_val, int *P_nbVal);
  virtual void get_body_value (T_pValueData P_dest, T_pBodyValue P_orig);
  virtual void set_body_value (T_pBodyValue P_dest, T_pValueData P_orig);


  virtual T_ManagementSessionId* get_manage_session_elt (int P_id);
  virtual int get_nb_management_session () ;

  virtual bool check_present_session (int P_msg_id,int P_id) ;
  virtual bool find_present_session (int P_msg_id,int P_id) ;


  iostream_output& print_header(iostream_output& P_stream, 
				int           P_headerId,
				T_pValueData  P_val) ;
  virtual iostream_output& print_body  (iostream_output&  P_stream, 
                                        int          P_nb,
                                        T_pBodyValue P_val,
                                        int          P_level = 0) ;

  T_TypeType get_body_value_type (int P_id);
  T_TypeType get_header_value_type (int P_id);

  void          reset_value_data (T_pValueData P_val) ;
  void          convert_to_string (T_pValueData P_res, 
				   T_pValueData P_val) ;

  void set_padding (unsigned long P_value);
  bool get_header_length_excluded ();

  unsigned long get_msg_length_start ();  


  bool get_complex_header_presence ();

protected:

  // header definition variables
  char          *m_header_name ;
  T_pHeaderField m_header_field_table ;
  T_pIdMap       m_header_id_map ;

  char          *m_header_type_name ;
  int            m_header_type_id_body ;

  int            m_header_type_id;
  int            m_header_length_id ;
  int            m_header_length_index ;
  unsigned long  m_nb_field_header, m_max_nb_field_header ;
  unsigned long  m_header_size ;


  unsigned long  m_msg_length_start ;
  bool           m_msg_length_start_detected ;

  // body definition variables
  char          *m_header_body_name ;
  T_pHeaderField m_header_body_field_table ;
  T_pIdMap       m_header_body_id_map ;

  int            m_header_body_type_id, m_header_body_length_id ;
  int            m_header_body_length_index;
  unsigned long  m_nb_field_header_body ;
  unsigned long  m_max_nb_field_header_body;
  unsigned long  m_header_body_size ;

  int            m_header_body_start_optional_id ;

  bool           m_header_complex_type_presence ;


  T_pNameAndIdList             m_message_name_list            ;
  T_pNameAndIdList             m_message_comp_name_list       ;


  // type management variables
  T_pTypeDef     m_type_def_table ;
  unsigned long  m_nb_types, m_max_nb_types ;
  T_pIdMap       m_type_id_map ;

  // header values management
  unsigned long      m_nb_header_values ;
  T_pHeaderValue     m_header_value_table ;
  T_pIdMap           m_header_value_id_map ;
  T_pDecodeMap       m_header_decode_map ;

  // body values defined in the header_name section (ie command for diameter)
  // header_name section is a sub section of dictionnary

  T_pBodyValueDef    m_body_value_table ;

  // body values management
  unsigned long      m_nb_header_body_values ;
  T_pHeaderBodyValue m_header_body_value_table ;
  T_pIdMap           m_header_body_value_id_map ;
  T_pDecodeMap       m_header_body_decode_map ;

  // protocol msg id management (session-id)
  T_MsgIdType        m_msg_id_type ;
  int                m_msg_id_id   ;
  T_TypeType         m_msg_id_value_type ;

  // protocol msg id management (out of session)
  T_MsgIdType        m_type_id_out_of_session ;
  int                m_id_out_of_session ;

  // padding control
  unsigned long      m_padding_value ;
  bool               m_header_length_excluded ;

  int  add_counter (char *P_name, unsigned long P_init) ;
  
  int  add_type(char *P_name, T_TypeType P_type, unsigned long P_size);
  
  int  add_header_field (char           *P_name, 
			 unsigned long   P_size, 
			 unsigned long  *P_nb_field,
			 unsigned long  *P_header_size,
			 T_pHeaderField  P_field_table,
			 T_pIdMap        P_map,
			 T_pCondPresence P_condPresence = NULL,
                         long            P_type_id = -1);

  void set_header_type_id (int P_id);
  void set_header_length_id (int P_id);

  void set_header_body_type_id (int P_id);
  void set_header_body_length_id (int P_id);
  
  int get_header_from_xml (C_XmlData *P_def);
  int get_types_from_xml (C_XmlData *P_def);

  virtual int xml_interpretor(C_XmlData *P_def);
  virtual int analyze_header_from_xml (C_XmlData *P_def);
  virtual int analyze_types_from_xml (C_XmlData *P_def);
  virtual int analyze_body_from_xml (C_XmlData *P_def);

  int analyze_dictionnary_from_xml (C_XmlData *P_def); 
  virtual    int analyze_sessions_id_from_xml (C_XmlData *P_def) ;

  
  virtual int get_header_body_values_from_xml (C_XmlData *P_def);
  virtual int get_header_values_from_xml (C_XmlData *P_def);

  int get_header_body_from_xml (C_XmlData *P_def);
  int get_header_body_optional_from_xml (C_XmlData *P_def);




  int get_counter_from_xml (C_XmlData *P_def);

  bool check_presence_needed (T_pCondPresence P_condition,
			      unsigned long  *P_values);

  int process_grouped_type(C_XmlData *P_bodyData,
	                   int P_body_grouped_val_id,
			   T_pBodyValue P_pBodyVal);
			   
  list_t<C_MessageFrame*> *m_messageList;

} ; // class C_ProtocolBinary

#endif // _C_PROTOCOLBINARY_









