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

#ifndef _CPP_C_DATALOGRTDISTRIB_H_
#define _CPP_C_DATALOGRTDISTRIB_H_

// #include "list_t.hpp"
// #include "fstream_t.hpp"
// #include <sys/time.h>

#include "C_DataLogControl.hpp"
// #include "C_SemaphoreTimed.hpp"

class C_DataLogRTDistrib : public C_DataLogControl {

public:

  C_DataLogRTDistrib(size_t         P_max_data,
		     size_t         P_max_data_log,
		     char          *P_file_name, 
		     unsigned long  P_period,
		     unsigned long  P_rtdistrib);
  ~C_DataLogRTDistrib();

  int data (double P_time, double P_value);
  int data (char *P_data);
  void do_log () ;
  void do_log_cumul () ;


private:
  unsigned long     m_rtdistrib ; 
  unsigned long    *m_p_rt_distrib     ;
  unsigned long    *m_c_rt_distrib     ;

  unsigned long     m_p_min_rt_distrib ; 
  unsigned long     m_p_max_rt_distrib ;
  unsigned long     m_c_min_rt_distrib ;
  unsigned long     m_c_max_rt_distrib ;
  unsigned long     m_counter_rt       ;

  C_Semaphore      *m_sem_rt ; 

} ;


#endif // _CPP_C_DATALOGRTDISTRIB_H_




