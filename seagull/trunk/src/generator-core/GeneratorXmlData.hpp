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

#ifndef _GENERATOR_XML_DATA_H
#define _GENERATOR_XML_DATA_H

#include "string_t.hpp"
#include "list_t.hpp"
#include "map_t.hpp"

typedef struct _xml_trans_def {
  char* m_name ;
  char* m_init_args ;
  char* m_library_file_name ;
  char* m_instance_create_fct_name ;
  char* m_instance_delete_fct_name ;
  char* m_protocol_name ;
}  T_xml_transport_definition, 
  *T_pXml_transport_definition ;

typedef list_t<T_pXml_transport_definition> T_Xml_trans_def_list ;

typedef struct _xml_cmd_def {
  char *m_name ;
  char *m_entity_concerned ;
  char *m_args ;
}  T_xml_cmd_definiton,
  *T_pXml_cmd_definition ;

typedef list_t<T_pXml_cmd_definition>       T_Xml_cmd_def_list ;

typedef map_t<string_t, int>             T_Xml_trans_name_map, 
                                              *T_pXml_trans_name_map;

typedef struct _xml_channel_def {
  char *m_name ;
  char *m_transport ;
}  T_Xml_channel_definition, 
  *T_pXml_channel_definition ;

typedef list_t<T_pXml_channel_definition> T_Xml_channel_def_list ;

typedef map_t<string_t, int>             T_Xml_channel_name_map,
                                              *T_pXml_channel_name_map;

#endif // _GENERATOR_XML_DATA_H
