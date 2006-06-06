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

#ifndef _C_LOG_VALUE_H
#define _C_LOG_VALUE_H


class C_LogValue {
  public:
  C_LogValue(char *P_string);
  C_LogValue(double P_time, double P_data);
  ~C_LogValue();
  void set_string (char *P_string);
  void set_data (double P_time, double P_data);
  void get_value (char *P_buf) ;
private:
  typedef enum _enum_type_value {
    E_TYPE_STRING,
    E_TYPE_DATA
  } T_TypeValue, *T_pTypeValue ;
  typedef struct _struct_log_data {
    double m_x ;
    double m_y ;
  } T_LogData, *T_pLogData ;

  T_TypeValue  m_type    ;
  T_LogData    m_data    ;
  char         *m_string ;
} ;

#endif // _C_LOG_VALUE_H


