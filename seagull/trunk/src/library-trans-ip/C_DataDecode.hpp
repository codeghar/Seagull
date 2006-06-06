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

#ifndef _CDATADECODE_H_
#define _CDATADECODE_H_

#include "list_t.hpp"
#include "map_t.hpp"

class C_DataDecode {
public:
  C_DataDecode(size_t P_buf_size);
  ~C_DataDecode();

  void           init () ;
  unsigned char *get_buffer (unsigned char* P_buf, 
			     size_t       * P_pSize);
  int            set_buffer (unsigned char* P_buf, size_t P_size);
  void           reset_buffer () ;

private:
  size_t         m_buf_size    ;
  unsigned char *m_buffer      ;
  size_t         m_segm_size   ;
} ;

typedef C_DataDecode *C_pDataDecode ;


#endif //_CDATADECODE_H_


