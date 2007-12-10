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

// C_Semaphore class definition

#ifndef _C_SEMAPHORE
#define _C_SEMAPHORE

#define T_CounterValue     int
#define COUNTER_INIT_VALUE 1
#define COUNTER_COMP_VALUE 0

class C_Semaphore {

 public:
  T_CounterValue P(); // P: blocks while <=0 (signaled) and then -- 
  T_CounterValue V(); // V: ++ and signal the change
  C_Semaphore() ;
  ~C_Semaphore() ;

 protected:
  void init();        // internal init of the semaphore => system calls
  void destroy();     // internal destroy of the semaphore => system calls
  void *m_impl;       // semaphore data structure

} ;

#endif // _C_SEMAPHORE
