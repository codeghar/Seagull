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

#ifndef _C_KEYBOARDCONTROL_H
#define _C_KEYBOARDCONTROL_H

#include "C_SemaphoreTimed.hpp"
#include "C_GeneratorStats.hpp"
#include "C_TaskControl.hpp"

#include <termios.h>

class C_Generator ; // avoid circular reference

class C_KeyboardControl : public C_TaskControl {

public:
  
  C_KeyboardControl() ;
  ~C_KeyboardControl() ;
  
  void init(C_Generator *P_gen) ;
  void force_end_procedure () ;

private :

  struct termios    m_saved_conf      ;
  struct termios    m_controller_conf ;
  C_Generator      *m_gen ;

  T_GeneratorError TaskProcedure () ;
  T_GeneratorError InitProcedure () ;
  T_GeneratorError EndProcedure () ;
  T_GeneratorError StoppingProcedure () ;
  T_GeneratorError ForcedStoppingProcedure () ;

  void execute_cmd (char P_char) ;
  void analyze_cmd (char *P_cmd);

  void init_controller_configuration () ;
  void controller_configuration () ;
  void system_configuration () ;
  
} ;

#endif // _C_KEYBOARDCONTROL_H






