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

#include "C_LogValue.hpp"
#include "Utils.hpp"
#include "string_t.hpp"

C_LogValue::C_LogValue(char *P_string) {
  m_type = E_TYPE_STRING ;
  ALLOC_TABLE (m_string,
	       char*,
	       sizeof(char),
	       100);
  strncpy(m_string, P_string, 100);
}
C_LogValue::C_LogValue(double P_time, double P_data) {
  m_type = E_TYPE_DATA ;
  ALLOC_TABLE (m_string,
	       char*,
	       sizeof(char),
	       100);
  m_data.m_x = P_time ;
  m_data.m_y = P_data ;
}
C_LogValue::~C_LogValue() {
  FREE_TABLE (m_string)
}
void C_LogValue::set_string (char *P_string) {
  m_type = E_TYPE_STRING ;
  strncpy(m_string, P_string, 100);
}
void C_LogValue::set_data (double P_time, double P_data) {
  m_type = E_TYPE_DATA ;
  m_data.m_x = P_time ;
  m_data.m_y = P_data ;
}
void C_LogValue::get_value (char *P_buf) {
  switch (m_type) {
  case E_TYPE_STRING:
    strncpy(P_buf, m_string, 100);
    break ;
  case E_TYPE_DATA:
    snprintf(P_buf, 100, (char*)"%f;%f;", m_data.m_x, 
	     m_data.m_y);
    break ;
  }
}
