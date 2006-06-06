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

#include "C_XmlParser.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

#include "Utils.hpp"

extern int parser_start (FILE *P_fd, C_XmlParser* P_parser);

C_XmlParser::C_XmlParser() {

  GEN_DEBUG(1, "C_XmlParser::C_XmlParser() start") ;

  m_lex_string_value = NULL ;
  m_lex_string_index = 0 ;
  m_lex_max_string_size = 0 ;

  m_pCurrentXmlData = NULL ; 
  m_pXmldata = NULL ;
  m_vector.clear () ; 
  m_pXmlfield = NULL ;  

  GEN_DEBUG(1, "C_XmlParser::C_XmlParser() end") ;

}

C_XmlParser::~C_XmlParser() {
  GEN_DEBUG(1, "C_XmlParser::~C_XmlParser() start") ;
  GEN_DEBUG(1, "C_XmlParser::~C_XmlParser() end") ;
}

int C_XmlParser::lex_string_value_update (char P_char) {
  int L_result = 0 ;

  if (m_lex_string_index > m_lex_max_string_size) {
    printf("error, max reached\n");
    L_result = 1 ;
  } else {
    m_lex_string_value [ m_lex_string_index ] = P_char ;
    m_lex_string_index++ ;
    L_result = 0 ;
  }
  return (L_result);

}

void  C_XmlParser::lex_integer_update (char* P_value) {
  GEN_DEBUG(1, "C_XmlParser::lex_integer_update(" << P_value << ") start");
  m_lex_integer = atoi (P_value);
  GEN_DEBUG(1, "C_XmlParser::lex_integer_update() end");
}

int   C_XmlParser::get_lex_integer () {
  GEN_DEBUG(1, "C_XmlParser::get_lex_integer() start");
  GEN_DEBUG(1, "C_XmlParser::get_lex_integer() end");
  return (m_lex_integer);
}


char* C_XmlParser::get_lex_string_value () {
  GEN_DEBUG(1, "C_XmlParser::get_lex_string_value() start");
  GEN_DEBUG(1, "C_XmlParser::get_lex_string_value() end");
  return (m_lex_string_value) ;
}

void C_XmlParser::lex_string_value_reset () {
  GEN_DEBUG(1, "C_XmlParser::lex_string_value_reset() start");
  m_lex_string_index = 0 ;
  GEN_DEBUG(1, "C_XmlParser::lex_string_value_reset() end");
}

void C_XmlParser::new_xml_data (char *P_name) {
  GEN_DEBUG(1, "C_XmlParser::new_xml_data(" << P_name << ") start");
  NEW_VAR(m_pCurrentXmlData, C_XmlData(P_name));
  GEN_DEBUG(1, "C_XmlParser::new_xml_data() end");
}

void C_XmlParser::new_xml_field (char *P_name) {
  GEN_DEBUG(1, "C_XmlParser::new_xml_field(" << P_name << ") start");
  NEW_VAR(m_pXmlfield, C_XmlData::C_XmlField(P_name));
  GEN_DEBUG(1, "C_XmlParser::new_xml_field() end");
}

void C_XmlParser::add_xml_field (char *P_value) {
  GEN_DEBUG(1, "C_XmlParser::add_xml_field(" << P_value << ") start");
  m_pXmlfield->set_value(P_value); 
  m_pCurrentXmlData->add_field(m_pXmlfield) ;
  GEN_DEBUG(1, "C_XmlParser::add_xml_field() end");
}

void C_XmlParser::save_xml_data () {
  GEN_DEBUG(1, "C_XmlParser::save_xml_data() start");
  m_vector.push_back(m_pXmldata) ;
  m_vector.push_back(m_pCurrentXmlData) ;
  m_pXmldata = m_pCurrentXmlData ;
  GEN_DEBUG(1, "C_XmlParser::save_xml_data() end");
}

void C_XmlParser::retrieve_xml_data () {
  GEN_DEBUG(1, "C_XmlParser::retrieve_xml_data() start");
  m_vector_rit = m_vector.rbegin() ;
  m_pCurrentXmlData = *m_vector_rit ;
  m_vector.pop_back() ;
  m_vector_rit = m_vector.rbegin() ;
  m_pXmldata = *m_vector_rit ; 
  m_vector.pop_back() ;
  GEN_DEBUG(1, "C_XmlParser::retrieve_xml_data() end");
}

void C_XmlParser::add_sub_xml_data () {
  GEN_DEBUG(1, "C_XmlParser::add_sub_xml_data() start");
  GEN_DEBUG(1, "C_XmlParser::add_sub_xml_data() end");
  m_pXmldata -> add_sub_data (m_pCurrentXmlData);
}


void C_XmlParser::init () {
  GEN_DEBUG(1, "C_XmlParser::init() start");
  m_lex_string_value = (char*) malloc (1024) ;
  m_lex_max_string_size = 1024 ;
  NEW_VAR(m_pXmldata, C_XmlData((char*)""));
  GEN_DEBUG(1, "C_XmlParser::init() end");
}

FILE* C_XmlParser::next_file() {
  
  FILE *L_fd = NULL ;
  GEN_DEBUG(1, "C_XmlParser::next_file() start");
  m_current_file++ ;
  if (m_current_file < m_nb_file) {

    if ((L_fd = fopen (m_file_name_table[m_current_file].c_str(), "r")) == NULL) {
      GEN_FATAL(E_GEN_FATAL_ERROR, "Unable to open file [" 
	    << m_file_name_table[m_current_file] << "]");
    } 
  } else {
    if (m_current_file) m_current_file -- ;
  }
  GEN_DEBUG(1, "C_XmlParser::next_file() end");
  return (L_fd) ;
}

string_t& C_XmlParser::get_current_file_name() {
  GEN_DEBUG(1, "C_XmlParser::get_current_file_name() start");
  GEN_DEBUG(1, "C_XmlParser::get_current_file_name() end");
  return (m_file_name_table[m_current_file]) ;
}

C_XmlData* C_XmlParser::parse (int P_nb, 
			       string_t* P_fileName, 
			       int *P_error) {

  FILE      *L_fd = NULL ;
  C_XmlData *L_pData = NULL ;

  GEN_DEBUG(1, "C_XmlParser::parse() start");

  m_nb_file = P_nb ;
  m_file_name_table = P_fileName ;
  m_current_file = 0 ;

  if ((L_fd = fopen (m_file_name_table[m_current_file].c_str(), "r")) == NULL) {
    GEN_FATAL(E_GEN_FATAL_ERROR, "Unable to open file [" 
	  << m_file_name_table[m_current_file] << "]");
    *P_error = -1 ;
    return (NULL) ;
  } 
  
  init();

  *P_error = parser_start (L_fd, this) ;
  if (*P_error == 0) {
    L_pData = m_pXmldata ;
  } 
  
  GEN_DEBUG(1, "C_XmlParser::parse() end");
  return (L_pData);
}


bool C_XmlParser::check_xml_data_name(char *P_name, int P_line) {
  bool L_ret = false ;
  GEN_DEBUG(1, "C_XmlParser::xml_data_name() start");
  if (m_pCurrentXmlData != NULL) {
    L_ret 
      = (strcmp(m_pCurrentXmlData->get_name(), P_name) == 0) ? true : false ;
    if (L_ret == false) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
	    "Keyword [" 
	    << m_pCurrentXmlData->get_name() 
	    << "] expected near line ["
	    << P_line << "]");
    }
  } 
  
  GEN_DEBUG(1, "C_XmlParser::xml_data_name() end");
  return (L_ret);
}
