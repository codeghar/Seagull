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

#ifndef _C_EXTERNALDATACONTROL_
#define _C_EXTERNALDATACONTROL_

#include "ProtocolData.hpp"
#include "fstream_t.hpp"

#include <regex.h>

class C_ExternalDataControl {

public:

   C_ExternalDataControl() ;
  virtual ~C_ExternalDataControl() ;

  bool         init (char * P_file_name);
  T_pValueData get_value (int P_line, int P_field);

  T_TypeType&   get_field_type(int P_field_id) ;

  size_t       get_nb_field();

  virtual size_t  select_line () = 0 ;


protected:

  size_t         m_number_line   ;
  size_t         m_allocted_nb_line   ;
  size_t         m_max_size_line ;
  size_t         m_nb_field      ;  

  fstream_input *m_file ;
  char          *m_file_name     ;

  T_TypeType    *m_field_type_table ;

  regex_t       *m_regExpr1, *m_regExpr2, *m_regExpr3, *m_regExpr4 ;
  
  T_pValueData **m_data_table ;

  size_t            m_line_selected ;
  size_t            m_line_selected_max ; 
   
  bool max_buffer_size () ;

  bool  analyze () ; // determine the types of fields
  int   get_line (char *P_line);
  char* get_field (char *P_line, size_t *P_size, size_t *P_next);
  bool  analyze_first_data (char*P_line);
  bool  analyze_data (char*P_line);

  bool create_regexp () ;
  void delete_regexp () ;

private:

} ;

#endif // _C_EXTERNALDATACONTROL_
