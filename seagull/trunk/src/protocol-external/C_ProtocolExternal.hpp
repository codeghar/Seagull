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

#ifndef _C_PROTOCOL_EXTERNAL_H
#define _C_PROTOCOL_EXTERNAL_H

#include "Utils.hpp"
#include "C_XmlData.hpp"

#include "C_TransportControl.hpp"
#include "C_ProtocolExternalFrame.hpp"
#include "C_MsgBuildContext.hpp"
#include "C_MsgBuildContextFactory.hpp"
#include "list_t.hpp"
#include "map_t.hpp"
#include "string_t.hpp"
#include "C_GeneratorConfig.hpp"

#include "C_ProtocolStats.hpp"

class C_MessageExternal ; // forward definition in .cpp (prevent recursive calls)

class C_ProtocolExternal : public C_ProtocolExternalFrame {
public:
  C_ProtocolExternal(C_TransportControl   *P_transport_control,
		     C_XmlData            *P_def,
		     char                **P_name,
		     T_pConfigValueList    P_config_value_list,
		     T_ConstructorResult  *P_res);
  ~C_ProtocolExternal();

  typedef struct _field_def_t  {
    int                                  m_id ;
    T_TypeType                           m_type ;
    C_MsgBuildContext::T_ContextFunction m_set;
    C_MsgBuildContext::T_ContextFunction m_get;
    C_MsgBuildContext::T_ContextFunction m_check_set;
    C_MsgBuildContext::T_ContextFunction m_check_get;

  } T_DefField, *T_pDefField ;


  C_MessageFrame* create_new_message (C_MessageFrame *P_msg) ;
  C_MessageFrame* create_new_message (void                *P_xml, 
				      T_pInstanceDataList  P_list,
				      int                 *P_nb_body_value) ;

  char*           message_name           ()                  ;
  char*           message_component_name ()                  ;
  T_pNameAndIdList message_name_list     ()                  ;
  T_pNameAndIdList message_component_name_list  ()           ;

  int             find_field         (char *P_name)          ;
  bool            find_present_session (int P_msg_id,int P_id) ;

  T_TypeType      get_field_type     (int P_id,
				      int P_sub_id)          ;

  bool           check_sub_entity_needed     (int P_id)      ;

  char*          get_string_value(int P_id, T_pValueData P_data);
  char*          get_string_value_field_body(int P_id, T_pValueData P_data);
  
  C_MsgBuildContextFactory* get_factory() ;


  C_ProtocolFrame::T_MsgError from_external 
  (C_MsgBuildContext* P_build,
   T_pReceiveMsgContext P_recvMsgCtx) ;
  C_ProtocolFrame::T_MsgError to_external   
  (C_MsgBuildContext* P_build,
   C_MessageFrame *P_msg) ;

  void log_message (char* P_header, C_MessageFrame *P_msg) ;

protected:
private:
  typedef struct _fielddef_t {
    char *m_name    ; 
    char *m_type    ;
    char *m_get     ;
    char *m_set     ;
    char *m_check_get ;
    char *m_check_set ;
    char *m_default ;
    char *m_to_string ;
    char *m_from_string ;
    char *m_config_field_name ;
  } T_FieldDef, *T_pFieldDef ;
  typedef list_t<T_FieldDef> T_FieldDefList, *T_pFieldDefList ;

  typedef struct _field_value_t {
    char          *m_name  ; 
    char          *m_value ;
  } T_FieldValue, *T_pFieldValue ;
  typedef list_t<T_FieldValue> T_FieldValueList, *T_pFieldValueList ;

  typedef list_t<char*> T_CharList, *T_pCharList ;


  typedef struct _fielddef_body_t {
    char              *m_name             ; 
    char              *m_instance         ;
    T_pFieldValueList  m_list_value       ;
    T_pCharList        m_list_not_present ;

  } T_FieldBody, *T_pFieldBody ;
  typedef list_t<T_FieldBody> T_FieldBodyList, *T_pFieldBodyList ;

  typedef struct _fielddef_header_t {
    char              *m_name                    ; 
    T_pFieldValueList  m_list_value              ;
    T_pCharList        m_list_header_not_present ;
    T_pFieldBodyList   m_list_body               ;
  } T_FieldHeader, *T_pFieldHeader               ;
  typedef list_t<T_FieldHeader> T_FieldHeaderList, *T_pFieldHeaderList ;


  typedef struct _field_desc_t {
    int                                  m_id ;
    T_TypeType                           m_type ;
    char                                *m_name ; // to allocate  
    C_MsgBuildContext::T_ContextFunction m_get;
    C_MsgBuildContext::T_ContextFunction m_set;
    C_MsgBuildContext::T_ContextFunction m_check_get;
    C_MsgBuildContext::T_ContextFunction m_check_set;

    C_MsgBuildContext::T_ContextStringFunction m_to_string ;
    C_MsgBuildContext::T_ContextStringFunction m_from_string ;
    char*                         m_config_field_name ;

  } T_FieldDesc, *T_pFieldDesc ;

  typedef struct _field_body_t {
    int          m_id     ;
    char        *m_name   ; // to allocate
    T_pValueData m_header ;
  } T_FieldBodyDesc, *T_pFieldBodyDesc ;
  typedef map_t<string_t,T_pFieldDesc> T_FieldNameMap, *T_pFieldNameMap ;
  typedef map_t<string_t,T_pFieldBodyDesc> T_FieldBodyNameMap, *T_pFieldBodyNameMap ;
  typedef map_t<string_t,C_MessageExternal*> T_MessageNameMap, *T_pMessageNameMap ;

  typedef map_t<T_ValueData, C_MessageExternal*> T_MessageDecodeMap, *T_pMessageDecodeMap ;
  typedef map_t<T_ValueData, int > T_BodyDecodeMap, *T_pBodyDecodeMap ;


  
  int                       m_nb_body_fields      ;
  int                       m_nb_header_fields    ;
  int                       m_nb_body_values      ;

  int                       m_nb_config_params    ;

  
  int                       m_start_body_index    ;
  int                       m_end_header_index    ;
  
  T_pFieldBodyNameMap       m_body_value_name_map ;
  
  void                     *m_library_handle      ;
  T_FactoryInfo             m_factory_info        ;
  C_MsgBuildContextFactory *m_factory             ;
  C_MsgBuildContext        *m_factory_context     ;
  
  char                     *m_message_name        ;
  char                     *m_body_name           ;

  int                       m_session_id          ;
  int                       m_outof_session_id    ;

  int                       m_type_id             ;
  int                       m_body_type_id        ;

  T_pFieldNameMap           m_field_body_name_map ;
  T_pFieldNameMap           m_field_name_map      ;
  int                       m_id_counter          ;

  T_pValueData              m_header_defaults     , 
                            m_body_defaults ;


  char                     *m_data_field_name ;
  T_pMessageNameMap         m_message_map ;


  T_pFieldDesc             *m_header_field_desc_table   ;
  T_pFieldDesc             *m_body_field_desc_table     ;
  T_pFieldBodyDesc         *m_body_value_table          ;

  char                    **m_names_table               ;
  char                    **m_message_names_table       ;
  int                       m_nb_names                  ;
  int                       m_nb_message_names          ;
  T_pMessageDecodeMap       m_message_decode_map        ;
  T_pBodyDecodeMap          m_body_decode_map           ;

  bool                    **m_body_not_present_table    ;

  bool                    **m_header_not_present_table  ;
  

  C_MsgBuildContext::T_ContextFunction m_get_body;
  C_MsgBuildContext::T_ContextFunction m_add_body;

  C_MsgBuildContext::T_ContextFunction m_create_body;
  C_MsgBuildContext::T_ContextFunction m_delete_body;

  C_MsgBuildContext::T_ContextFunction m_create_header;
  C_MsgBuildContext::T_ContextFunction m_delete_header;

  C_MsgBuildContext::T_ContextStringFunction *m_from_string_table ;
  int                                         m_nb_from_string ;
  C_MsgBuildContext::T_ContextStringFunction *m_to_string_table ;

  C_MsgBuildContext::T_ContextStringFunction *m_from_string_field_body_table ;
  int                                         m_nb_from_string_field_body ;
  C_MsgBuildContext::T_ContextStringFunction *m_to_string_field_body_table ;


  T_pNameAndIdList             m_message_name_list            ;
  T_pNameAndIdList             m_message_comp_name_list       ;

  T_pConfigValueList           m_config_value_list ;

  int xml_analysis (C_XmlData *P_data, char **P_name,
		    T_pConfigValueList P_config_value_list);

  int xml_fields (C_XmlData *P_data, 
		  T_FieldDefList *P_fielddef_list,
		  char **P_section_name,
		  char **P_type,
		  char **P_create,
		  char **P_delete,
		  char **P_value_field,
		  char **P_add,
		  char **P_get,
		  bool   P_ctrl_body);


  int xml_configuration_parameters (C_XmlData *P_data, 
				    T_ParamDefList *P_paramdef_list) ;
  
  int  update_config_params(T_ParamDef& P_config_param_dico,
			    T_pConfigValueList P_config_value_list);
  


  T_FieldBodyList* analyze_body_value(C_XmlData          *P_data, 
				      T_FieldBodyList    *P_fielddef_body_list,
				      bool                P_body_only,
				      int                *P_ret);

  T_FieldValueList* analyze_setfield(C_XmlData          *P_data, 
				     T_FieldValueList   *P_field_value_list,			    
				     int                *P_ret);

  T_CharList*       analyze_not_present(C_XmlData          *P_data, 
					T_CharList         *P_field_not_present_list,			    
					int                *P_ret);



  T_FieldHeaderList* analyze_header_value(C_XmlData            *P_data, 
					  T_FieldHeaderList    *P_fielddef_header_list,
					  bool                  P_header_only,
					  int                  *P_ret);
  int new_id () ;
  int get_id () ;
  
  int analyze_dictionnary (T_FieldDefList *P_header_fields,
			   T_FieldDefList *P_body_fields,
			   char *P_body_value_name,
			   char *P_session_id_name,
			   char *P_outof_session_id_name,
			   T_FieldBodyList *P_body_values,
			   T_FieldHeaderList *P_header_values,
			   char *P_header_type,
			   T_pConfigValueList P_config_value_list,
			   char *P_body_type) ;


  char* is_variable(char* P_variableString) ;
  char* find_config_value(char* P_variable) ;

  T_pFieldDesc check_field(T_FieldDef &P_field_def, 
			   char *P_field_name,
			   char **P_data_type_name, 
			   char *P_field_type,
			   char **P_field_type_found,
			   T_pConfigValueList P_config_value_list,
			   bool P_header);

  T_pFieldBodyDesc check_body(T_FieldBody &P_field_body);
  T_pFieldBodyDesc check_body_value(T_FieldBody &P_field_body);

  int              check_not_present(T_FieldBody &P_field_body);
  int              check_header_not_present(T_FieldHeader &P_field_header);


  void set_body_field_id();
  C_MessageExternal* build_message (C_MessageExternal*    P_msg, 
				    T_FieldHeader& P_header_list,
				    T_pInstanceDataList P_list,
				    int                 *P_nb_body_value) ;

  int find_field_id (char*P_name);

} ;

#endif // _C_PROTOCOL_EXTERNAL_H







