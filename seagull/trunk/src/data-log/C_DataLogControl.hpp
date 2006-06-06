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

#ifndef _CPP_C_DATALOGCONTROL_H_
#define _CPP_C_DATALOGCONTROL_H_

#include "list_t.hpp"
#include "fstream_t.hpp"
#include <sys/time.h>

#include "C_TaskControl.hpp"
#include "C_LogValue.hpp"
#include "C_SemaphoreTimed.hpp"

class C_DataLogControl : public C_TaskControl {

public:

  C_DataLogControl(size_t         P_max_data,
		   size_t         P_max_data_log,
		   char          *P_file_name, 
		   unsigned long  P_period);
  ~C_DataLogControl();

  int data (char *P_data);
  virtual void do_log () ;
  virtual int data (double P_time, double P_data);

  int time_data (struct timeval *P_begin, struct timeval *P_end);
  

protected:

  typedef list_t<C_LogValue *> T_LogValuePList, *T_pLogValuePList ;

  T_GeneratorError TaskProcedure () ;
  T_GeneratorError InitProcedure () ;
  T_GeneratorError EndProcedure () ;
  T_GeneratorError StoppingProcedure () ;
  T_GeneratorError ForcedStoppingProcedure () ;



  C_SemaphoreTimed  *m_sem ;
  C_Semaphore       *m_available_sem, *m_values_sem ;
  size_t             m_size, m_size_log ;
  double             m_time_ref ;

  C_LogValue       **m_data_values ;

  T_pLogValuePList   m_values, m_available, m_log ;
  fstream_output    *m_output_stream ;

  bool               m_dump_end_procedure ;


} ;


#endif // _CPP_C_DATALOGCONTROL_H_






