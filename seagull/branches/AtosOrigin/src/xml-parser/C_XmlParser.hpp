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

#ifndef _C_XML_PARSER_H_
#define _C_XML_PARSER_H_

#include "C_XmlData.hpp"

#include "string_t.hpp"
#include "vector_t.hpp"
#include <cstdio>

class C_XmlParser {

public:
   C_XmlParser();
  ~C_XmlParser();

  // lexical related actions
  int   lex_string_value_update (char) ;
  void  lex_string_value_reset () ;
  char *get_lex_string_value () ;

  void  lex_integer_update (char*) ;
  int   get_lex_integer ();

  // syntaxic related actions
  void  new_xml_data        (char *P_name) ;
  void  new_xml_field       (char *P_name) ;
  void  add_xml_field       (char *P_value) ;
  void  save_xml_data       () ;
  void  retrieve_xml_data   () ;
  void  add_sub_xml_data    () ;
  bool  check_xml_data_name (char *P_name, int P_line) ;

  C_XmlData * parse (int P_nb, string_t *P_fileName, int *P_error);

  string_t&   get_current_file_name() ;
  FILE*       next_file () ;

private:
  // lexical information related data
  char*  m_lex_string_value ;
  int    m_lex_string_index ;
  int    m_lex_max_string_size ;

  int    m_lex_integer ;

  // syntaxic/action related data
  C_XmlData                                 *m_pCurrentXmlData, *m_pXmldata  ;
  vector_t<C_XmlData*>                       m_vector ; 
  vector_t<C_XmlData*>::reverse_iterator     m_vector_rit ;
  C_XmlData::C_XmlField                     *m_pXmlfield ;  


  // related file information
  string_t *m_file_name_table ;
  int    m_current_file    ;
  int    m_nb_file         ;

  void init () ;


} ;

#endif //  _C_XML_PARSER_H
