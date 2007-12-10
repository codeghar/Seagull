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

#include "C_DataDecode.hpp"
#include "Utils.hpp"

#include <cstring>

#define SOCKET_ERROR(l,m) iostream_error << m << iostream_endl << iostream_flush
#ifdef DEBUG_MODE
#define SOCKET_DEBUG(l,m) iostream_error << m << iostream_endl << iostream_flush
#else
#define SOCKET_DEBUG(l,m)
#endif

C_DataDecode::C_DataDecode(size_t P_buf_size) {
  SOCKET_DEBUG(0, "C_DataDecode::C_DataDecode (" 
	    << P_buf_size << ")");
  m_segm_size = 0 ;
  m_buf_size = P_buf_size;
  ALLOC_TABLE(m_buffer,
	      unsigned char*,
	      sizeof(unsigned char),
	      m_buf_size);
}

C_DataDecode::~C_DataDecode() {
  SOCKET_DEBUG(0, "C_DataDecode::~C_DataDecode ()");
  m_segm_size = 0 ;
  FREE_TABLE(m_buffer);
  m_buf_size = 0 ;
}

void C_DataDecode::init () {
  SOCKET_DEBUG(0, "C_DataDecode::init ()");
  m_segm_size = 0 ;
}

void C_DataDecode::reset_buffer() {
  SOCKET_DEBUG(0, "C_DataDecode::reset_buffer ()");
  m_segm_size = 0 ;
}

unsigned char *C_DataDecode::get_buffer (unsigned char* P_buf, 
					 size_t       * P_pSize) {

  size_t L_final_size = (*P_pSize) ;

  SOCKET_DEBUG(0, "C_DataDecode::get_buffer (" 
	    << (void*)&P_buf[0] << "," << *P_pSize << ")");

  SOCKET_DEBUG(0, "C_DataDecode::get_buffer() segment = " << m_segm_size);

  
  L_final_size += m_segm_size;
  if (L_final_size <= m_buf_size) {
    memcpy (m_buffer+m_segm_size, P_buf, *P_pSize);
    m_segm_size = 0 ;
    *P_pSize = L_final_size ;
  } else {
    // error
    *P_pSize = 0 ;
  }

  SOCKET_DEBUG(0, "C_DataDecode::get_buffer () return " 
	    << &m_buffer);

  return (m_buffer);
}

int           C_DataDecode::set_buffer (unsigned char* P_buf, 
					 size_t         P_size) {
  int L_ret = 0 ;
  SOCKET_DEBUG(0, "C_DataDecode::set_buffer (" 
	    << &P_buf[0] << "," << P_size << ")");

  if (P_size <= m_buf_size) {
    memmove(m_buffer, P_buf, P_size);
    m_segm_size = P_size ;
  } else {
    SOCKET_ERROR(E_GEN_FATAL_ERROR, "session maximum size buffer=" << m_buf_size << " data=" << P_size);
    L_ret = -1 ;
  }
  return (L_ret);
}
