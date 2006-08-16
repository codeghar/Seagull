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

#ifndef _C_PROTOCOL_TEXT_H
#define _C_PROTOCOL_TEXT_H

#include "C_ProtocolTextFrame.hpp"
#include "C_GeneratorConfig.hpp"

#include "C_RegExp.hpp"
#include "list_t.hpp"
#include "map_t.hpp"

#include "C_ProtocolStats.hpp"

#include "TextDef.hpp"

#include "C_ContextFrame.hpp"
#include "ParserFrame.hpp"
#include "FilterFrame.hpp"

class C_MessageText ;

class C_ProtocolText : public C_ProtocolTextFrame {
public:

  friend class C_MessageText ;

  typedef struct _struct_management_session_id {
    int                m_msg_id_id   ;
    T_TypeType         m_msg_id_value_type ;
  } T_ManagementSessionTextId, *T_pManagementSessionTextId;

  typedef list_t<T_ManagementSessionTextId> T_ManagementSessionTextIdList, *T_pManagementSessionTextIdList ;

  C_ProtocolText();

  void  analyze_data( C_XmlData            *P_def, 
                      char                **P_name,
                      T_pConfigValueList    P_config_value_list,
                      T_pContructorResult   P_res) ;
  ~C_ProtocolText();

  C_MessageFrame*                 decode_message (unsigned char *P_buffer, 
                                                  size_t        *P_size, 
                                                  C_ProtocolTextFrame::T_pMsgError    P_error) ;
  
  C_ProtocolTextFrame::T_MsgError encode_message (C_MessageFrame *P_msg,
                                                  unsigned char  *P_buffer,
                                                  size_t         *P_buffer_size);

  void                            log_buffer (char *P_header, 
                                              unsigned char *P_buf, 
                                              size_t P_size);

  char*                           message_name();
  char*                           message_component_name() ;
  T_pNameAndIdList                message_name_list     () ;
  T_pNameAndIdList                message_component_name_list  () ;

  char*                           message_name(int P_Id);

  int                             find_field      (char *P_name);
  T_TypeType                      get_field_type  (int P_id,
                                                   int P_sub_id);

  bool                            check_sub_entity_needed     (int P_id)      ;


  bool                            find_present_session (int P_msg_id,int P_id) ;

  C_MessageFrame*                   create_new_message (C_MessageFrame *P_msg);
  C_MessageFrame*                   create_new_message (void                *P_xml,
                                                        T_pInstanceDataList  P_list,
                                                        int                 *P_nb_value);
  char* get_field_value_to_check(C_MessageText *P_msg, 
                                 int  P_id,
                                 int *P_size);

  char* get_field_value(C_MessageText *P_msg, int P_id);

  char* get_field_value(C_MessageText *P_msg, int P_id,C_RegExp  *P_regexp_data);

  int   set_field_value(C_MessageText *P_msg, 
                        int            P_id, 
                        T_pValueData   P_value) ;
  
  int   get_message_id (char *P_name);

  T_pManagementSessionTextId get_manage_session_elt (int P_id);
  int get_nb_management_session () ;

  iostream_output& print_header_msg(iostream_output& P_stream, 
                                    int              P_id,
                                    T_pValueData     P_header) ;
  
  
  iostream_output& print_data_msg(iostream_output& P_stream, 
                                  T_pValueData     P_data) ;


  C_MessageText* create (C_ProtocolText *P_protocol,
                          char           *P_header,
                          char           *P_body);


  // typedef T_pValueData (C_MessageText::* T_SessionMethod)(C_ContextFrame *P_void);

  typedef C_ProtocolFrame::T_MsgError 
  (C_MessageText::* T_BodyDecodeMethod)(int  P_index,
                                        char *P_ptr, 
                                        size_t *P_size);


  typedef C_ProtocolFrame::T_MsgError 
  (C_MessageText::* T_EncodeMethod)(int P_index);


  typedef T_pValueData (C_MessageText::* T_SessionMethod)(C_ContextFrame *P_void);


protected:
private:

  typedef list_t<char*> T_charPlist, *T_pcharPlist ;

  typedef struct _regexp_str_t {
    char *m_name ;
    char *m_expr ;
    int   m_nb_match ; 
    int   m_sub_match; 
    int   m_line ;     
  } T_RegExpStr, *T_pRegExpStr ;
  typedef list_t<T_pRegExpStr> T_ExpStrLst, *T_pExpStrLst ;

  
  typedef struct _fielddef_t {
    char        *m_name      ;
    char        *m_format    ;
    T_ExpStrLst *m_expr_list ;
  } T_FieldDef, *T_pFieldDef ;
  typedef list_t<T_pFieldDef> T_FieldDefList, *T_pFieldDefList ;

  typedef list_t<C_RegExp*> T_RegExpLst, *T_pRegExpLst ;

  typedef struct _field_desc_t {
    int                                  m_id          ;
    bool                                 m_header_body ; // true ==> header
    char                                *m_name        ; // to allocate  
    T_RegExpLst                         *m_reg_exp_lst ;
    
    T_pValueData                         m_format      ;
    int                                  m_format_value_start ;
    int                                  m_format_value_end ;

  } T_FieldDesc, *T_pFieldDesc ;

  typedef map_t<string_t,T_pFieldDesc> T_FieldNameMap, *T_pFieldNameMap ;


  typedef struct _fielddef_header_t {
    char              *m_name                    ; 
  } T_FieldHeader, *T_pFieldHeader               ;
  typedef list_t<T_FieldHeader> T_FieldHeaderList, *T_pFieldHeaderList ;

  typedef map_t<string_t,C_MessageText*> T_MessageNameMap, *T_pMessageNameMap ;


  typedef struct _cdata_value_t {
    char          *m_value ;
  } T_CDATAValue, *T_pCDATAValue ;
  typedef list_t<T_CDATAValue> T_CDATAValueList, *T_pCDATAValueList ;


  typedef struct _defmethod_t {
    char               *m_name        ; 
    T_BodyMethodType    m_method      ;
    char               *m_param       ;
    char               *m_default     ;
  } T_DefMethod, *T_pDefMethod ;
  typedef list_t<T_DefMethod> T_DefMethodList, *T_pDefMethodList ;

  int find_field_id (char*P_name) ;

  int xml_interpretor(C_XmlData *P_def,
                      char **P_name,
                      T_pConfigValueList P_config_value_list) ;

  int set_body_method();

  int analyze_from_xml (C_XmlData *P_def, 
                        T_FieldDefList *P_fielddef_list,
                        char **P_section_name,
                        char **P_type,
                        bool P_header_body);

  int analyze_dictionnary_from_xml (C_XmlData *P_def); 
  int analyze_sessions_id_from_xml (C_XmlData *P_def) ;

  int analyze_body_method_param (int P_index, char *P_body_method_param);
  int set_filter_method (char *P_filter_method) ;
  char* find_text_value (char *P_buf, char *P_field);

  int analyze_body_method_from_xml (C_XmlData *P_data, 
				    T_DefMethodList *P_def_method_list) ;



  T_FieldHeaderList* analyze_header_value(C_XmlData            *P_data, 
					  T_FieldHeaderList    *P_fielddef_header_list,
					  int                  *P_ret);

  int   analyze_message_scen(C_XmlData            *P_data, 
                              T_pCDATAValueList     P_cdata_value_list);

  C_MessageText* build_message (char *P_header, 
                                char *P_body);

  T_pFieldDesc check_field(T_pFieldDef P_field_def, bool P_header);

  int new_id () ;


  char                            *m_header_name              ;
  char                            *m_header_type              ;
  char                            *m_body_name                ;
  char                            *m_body_type                ;

  char                           **m_names_fields             ;


  T_pFieldDefList                  m_header_fields_list        ;
  T_pFieldDefList                  m_body_fields_list          ;
  int                              m_nb_body_fields            ;
  int                              m_nb_header_fields          ;

  T_pFieldNameMap                  m_fields_name_map           ;
  int                              m_id_counter                ;


  T_pFieldHeaderList               m_header_fields_dico        ;
  int                              m_nb_message_names          ;


  typedef map_t<string_t,int>     T_DecodeMap, *T_pDecodeMap       ;

  char                           **m_message_names_table       ;
  T_pDecodeMap                     m_message_decode_map        ;
  

  T_pNameAndIdList                 m_message_name_list         ;
  

  T_pFieldDesc                    *m_fields_desc_table         ;
  int                              m_nb_fields_desc_table      ;
  
  T_pManagementSessionTextId       m_value_sessions_table      ;
  int                              m_value_sessions_table_size ;

  T_pDefMethodList                 m_def_method_list           ;

private:
  char                            *m_body_separator            ;
  size_t                           m_body_separator_size       ;

  char                            *m_field_separator           ;
  size_t                           m_field_separator_size      ;

  char                            *m_field_body_separator      ;
  size_t                           m_field_body_separator_size ;


  int                              m_message_type_field_id     ;
  int                              m_session_id_id             ;

  T_SessionMethod                  m_session_method            ;

  T_FilterFunction                 m_filter_function           ;

  typedef union _union_param {
    int                 m_id ;
    T_ParserFunction    m_parser ;
  } T_ParamMethod, *T_pParamMethod ;

  typedef struct _struct_body_decode_data {
    T_BodyDecodeMethod m_decode ;
    T_EncodeMethod     m_encode ;
    T_ParamMethod      m_param_encode  ;
    T_ParamMethod      m_param_decode  ;
  } T_BodyDecodeData, *T_pBodyDecodeData ;

  T_BodyDecodeData **m_methods ;
  int                m_nb_methods ;

  void set_body_separator (char*  P_body_separator) ;
  void set_field_separator (char*  P_field_separator) ;
  void set_message_type_field_id (int P_id) ;
  void set_session_id_id (int  P_id) ;
  void use_open_id () ;

  void set_number_methods (int P_nb) ;
  void destroy_methods () ;
  

  void set_body_decode_method (int                     P_index,
                               T_BodyMethodType        P_type,
                               T_BodyDecodeMethod      P_method,
                               void                   *P_param);

  void set_encode_method (int                 P_index,
                          T_BodyMethodType    P_type,
                          T_EncodeMethod      P_method,
                          void               *P_param);


  

} ;

#endif // _C_PROTOCOL_TEXT_H


