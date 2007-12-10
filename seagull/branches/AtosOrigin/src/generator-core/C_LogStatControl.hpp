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

#ifndef _C_LOGSTATCONTROL_H
#define _C_LOGSTATCONTROL_H

#include "C_SemaphoreTimed.hpp"
#include "C_GeneratorStats.hpp"
#include "C_TaskControl.hpp"
#include "string_t.hpp"


class C_LogStatControl : public C_TaskControl {

 public:

  C_LogStatControl() ;
  ~C_LogStatControl() ;

  void  init (unsigned long P_period, string_t& P_fileName);
  void  reset_cumul_counters ();
  
  void activate_percent_traffic () ;

  private :

    C_GeneratorStats *m_stat;
    C_SemaphoreTimed *m_sem ;

    T_GeneratorError TaskProcedure () ;
    T_GeneratorError InitProcedure () ;
    T_GeneratorError EndProcedure () ;
    T_GeneratorError StoppingProcedure () ;
    T_GeneratorError ForcedStoppingProcedure () ;

} ;

#endif // _C_LOGGINGCONTROLLER_H
