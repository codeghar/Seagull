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

#ifndef _CPP_C_ID_GENERATOR_
#define _CPP_C_ID_GENERATOR_

class C_IdGenerator {
public:
  C_IdGenerator();
  ~C_IdGenerator();

  int  new_id     ()         ;
  int  release_id (int P_id) ;

private:
  void *m_impl ;

} ;

typedef C_IdGenerator *T_pC_IdGenerator ;

#endif // _CPP_C_ID_GENERATOR_
