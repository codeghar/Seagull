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

#ifndef _C_XML_DATA_
#define _C_XML_DATA_

#include "list_t.hpp"
#include "iostream_t.hpp"

class _C_XmlEntity_Basic {
public:
  _C_XmlEntity_Basic () ;
  _C_XmlEntity_Basic (char *P_name);
  ~_C_XmlEntity_Basic () ;
  char *get_name () ;
  void  set_name (char *P_name);
private:
  char * m_name ;
} ;


class C_XmlData {
public:

  class C_XmlField {
  public:
    C_XmlField();
    C_XmlField(char *P_name, char *P_value);
    C_XmlField(char *P_name);
    ~C_XmlField();
    char* get_name();
    char* get_value();
    void  set_name(char *P_name);
    void  set_value(char *P_value);
    friend iostream_output& operator<<(iostream_output&, C_XmlField&);
  private:
    char *m_name;
    char *m_value;
  } ;

  typedef C_XmlField            *T_pC_XmlField ;
  typedef list_t<C_XmlField*> T_XmlField_List, *T_pXmlField_List ;
     
   C_XmlData() ;
   C_XmlData(char * P_name) ;
  ~C_XmlData() ;

  char* get_name() ;
  void  set_name(char* P_name) ;

  void  add_sub_data (C_XmlData* P_data);
  void  add_field    (C_XmlField* P_field);
  
  char* find_value   (char* P_fieldName) ;

  list_t<C_XmlData*>*  get_sub_data () ;
  T_pXmlField_List     get_fields   () ;

  friend iostream_output& operator<<(iostream_output&, C_XmlData&);

private:
  char                 *m_name ;
  list_t <C_XmlData*>  *m_sub_data ;
  T_pXmlField_List      m_fields ;
} ;

typedef C_XmlData *T_pC_XmlData ;
typedef list_t<T_pC_XmlData>  T_XmlData_List, *T_pXmlData_List ;


#endif // _C_XML_DATA_
