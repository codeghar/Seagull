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

#include "C_XmlData.hpp"
#include "Utils.hpp"

#include <cstring>

#include "GeneratorTrace.hpp"

// class _C_XmlEntity_Basic related methods

_C_XmlEntity_Basic::_C_XmlEntity_Basic () {
  m_name = NULL ;
} 

_C_XmlEntity_Basic::_C_XmlEntity_Basic (char *P_name) {
  set_name(P_name);
}

_C_XmlEntity_Basic::~_C_XmlEntity_Basic () {
  FREE_VAR(m_name);
}

char *_C_XmlEntity_Basic::get_name () {
  return (m_name);
}

void  _C_XmlEntity_Basic::set_name (char *P_name) {
  FREE_VAR(m_name) ;
  ALLOC_VAR(m_name, char*, sizeof(char)*strlen(P_name)+1) ;
  strcpy (m_name, P_name);
}


C_XmlData::C_XmlField::C_XmlField() {
  
  m_name = NULL ;
  m_value = NULL ;
  
}


C_XmlData::C_XmlField::C_XmlField(char *P_name, char* P_value) {
  
  ALLOC_VAR(m_name, char*, sizeof(char)*strlen(P_name)+1) ;
  strcpy (m_name, P_name);
  ALLOC_VAR(m_value, char*, sizeof(char)*strlen(P_value)+1) ;
  strcpy (m_value, P_value);
  
}

C_XmlData::C_XmlField::C_XmlField(char *P_name) {
  
  ALLOC_VAR(m_name, char*, sizeof(char)*strlen(P_name)+1) ;
  strcpy (m_name, P_name);
  m_value = NULL ;
  
}

C_XmlData::C_XmlField::~C_XmlField() {

  FREE_VAR(m_name);
  FREE_VAR(m_value);

}

void C_XmlData::C_XmlField::set_name(char *P_name) {
  FREE_VAR(m_name) ;
  ALLOC_VAR(m_name, char*, sizeof(char)*strlen(P_name)+1) ;
  strcpy (m_name, P_name);
}

void C_XmlData::C_XmlField::set_value(char *P_value) {
  GEN_DEBUG(1, "C_XmlData::C_XmlField::set_value start");
  FREE_VAR(m_value) ;
  ALLOC_VAR(m_value, char*, sizeof(char)*strlen(P_value)+1) ;
  strcpy (m_value, P_value);

  GEN_DEBUG(1, "C_XmlData::C_XmlField::set_value m_value " << m_value );
  GEN_DEBUG(1, "C_XmlData::C_XmlField::set_value end");
}

char* C_XmlData::C_XmlField::get_name() {
  return (m_name);
}

char* C_XmlData::C_XmlField::get_value() {
  return (m_value);
}


C_XmlData::C_XmlData() {
  m_name = NULL;
  NEW_VAR(m_sub_data, list_t<C_XmlData*>);
  NEW_VAR(m_fields, list_t<C_XmlField*>);
}

C_XmlData::C_XmlData(char *P_name) {

  ALLOC_VAR(m_name, char*, sizeof(char)*strlen(P_name)+1) ;
  strcpy (m_name, P_name);
  NEW_VAR(m_sub_data, list_t<C_XmlData*>);
  NEW_VAR(m_fields, list_t<C_XmlField*>);

}

C_XmlData::~C_XmlData() {

  if (!m_sub_data->empty()) {
    list_t<C_XmlData*>::iterator  L_xmldata_it ;
    C_XmlData                  *L_pXmldata ;
    for (L_xmldata_it  = m_sub_data->begin() ;
         L_xmldata_it != m_sub_data->end();
         L_xmldata_it++) {
      L_pXmldata = *L_xmldata_it ;
      DELETE_VAR (L_pXmldata);
    }
  }
  m_sub_data->erase (m_sub_data->begin(), m_sub_data->end());
  DELETE_VAR(m_sub_data);

  if (!m_fields->empty()) {
    list_t<C_XmlField*>::iterator  L_field_it ;
    C_XmlField                  *L_pField ;
    for (L_field_it  = m_fields->begin() ;
         L_field_it != m_fields->end();
         L_field_it++) {
      L_pField = *L_field_it ;
      DELETE_VAR (L_pField);
    }
  }
  m_fields->erase (m_fields->begin(),  m_fields->end());
  DELETE_VAR(m_fields);

  FREE_VAR (m_name) ;

}

char* C_XmlData::get_name () {
  return (m_name);
}

void C_XmlData::set_name (char *P_name) {
  FREE_VAR(m_name) ;
  ALLOC_VAR(m_name, char*, sizeof(char)*strlen(P_name)+1) ;
  strcpy (m_name, P_name);
}

void C_XmlData::add_sub_data(C_XmlData *P_data) {
  if (P_data != NULL) {
    m_sub_data -> push_back (P_data) ;
  }
}

void C_XmlData::add_field(C_XmlField *P_field) {
  if (P_field != NULL) {
    m_fields -> push_back (P_field);
  }
}

list_t<C_XmlData*>*  C_XmlData::get_sub_data () {
  return (m_sub_data) ;
} 
list_t<C_XmlData::C_XmlField*>* C_XmlData::get_fields() {
  return (m_fields) ;
}

char* C_XmlData::find_value(char* P_fieldName) {

  char*                     L_result = NULL ;
  T_XmlField_List::iterator L_it ;
  T_pC_XmlField             L_field ;
  
  
  for(L_it = m_fields->begin();
      L_it != m_fields->end() ;
      L_it++) {
    L_field = *L_it ;
    if (strcmp(L_field->get_name(), P_fieldName)==0) {
      L_result = L_field->get_value() ;
      break ;
    }
  }

  return (L_result) ;

}

iostream_output& operator<<(iostream_output& P_ostream, 
			    C_XmlData& P_data) {
  P_ostream << "XML-DATA name = " << ((P_data.m_name == NULL) ? "no-name" : P_data.m_name) ;
  P_ostream << iostream_endl ;
  if (P_data.m_fields->empty()) {
    P_ostream << "no-fields" << iostream_endl ;
  } else {
    list_t<C_XmlData::C_XmlField*>::iterator  L_field_it ;
    C_XmlData::C_XmlField                  *L_pField ;
    for (L_field_it  = P_data.m_fields->begin() ;
	 L_field_it != P_data.m_fields->end();
	 L_field_it++) {
      L_pField = *L_field_it ;
      P_ostream << *L_pField << iostream_endl ;
    }
  }

  if (P_data.m_sub_data->empty()) {
    P_ostream << "no-sub_data" << iostream_endl ;
  } else {
    list_t<C_XmlData*>::iterator  L_xmldata_it ;
    C_XmlData                  *L_pXmldata ;
    for (L_xmldata_it  = P_data.m_sub_data->begin() ;
	 L_xmldata_it != P_data.m_sub_data->end();
	 L_xmldata_it++) {
      L_pXmldata = *L_xmldata_it ;
      P_ostream << *L_pXmldata << iostream_endl ;
    }
  }

  
  return (P_ostream) ;
}

iostream_output& operator<<(iostream_output& P_ostream, 
			    C_XmlData::C_XmlField& P_field) {
  P_ostream << "name = " 
	    << ((P_field.m_name == NULL) ? "no-name" : P_field.m_name) ;
  P_ostream << " ; value = " 
	    << ((P_field.m_value == NULL) ? "no-value" : P_field.m_value) ;
  return (P_ostream) ;
}
