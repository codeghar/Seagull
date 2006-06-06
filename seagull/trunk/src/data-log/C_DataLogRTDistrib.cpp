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

#include "Utils.hpp"
#include "GeneratorTrace.hpp"
#include "C_DataLogRTDistrib.hpp"

#include <pthread.h>
#include <math.h>
#include "iostream_t.hpp"

#define min(a,b) ((a)<(b)) ? (a) : (b)
#define max(a,b) ((a)<(b)) ? (b) : (a)

C_DataLogRTDistrib::C_DataLogRTDistrib(size_t         P_max_data,
				       size_t         P_max_data_log,
				       char          *P_file_name, 
				       unsigned long  P_period,
				       unsigned long  P_rtdistrib) 
  : C_DataLogControl(P_max_data,
		     P_max_data_log,
		     P_file_name, 
		     P_period){

  size_t L_i ;


  GEN_DEBUG(0, "C_DataLogRTDistrib::C_DataLogRTDistrib()");
  m_rtdistrib = P_rtdistrib ;

  NEW_TABLE( m_p_rt_distrib, unsigned long, m_rtdistrib+1);
  NEW_TABLE( m_c_rt_distrib, unsigned long, m_rtdistrib+1);
  NEW_VAR(m_sem_rt, C_Semaphore()) ;

  m_sem_rt->P();
  for (L_i=0 ; L_i <= m_rtdistrib ; L_i++) {
    m_p_rt_distrib[L_i] = 0;
    m_c_rt_distrib[L_i] = 0;
  }

  m_p_min_rt_distrib = m_rtdistrib ;
  m_p_max_rt_distrib = 0 ;

  m_c_min_rt_distrib = m_rtdistrib ;
  m_c_max_rt_distrib = 0 ;
  m_counter_rt = 0 ;
  m_sem_rt->V();
}

C_DataLogRTDistrib::~C_DataLogRTDistrib() {

  GEN_DEBUG(0, "C_DataLogRTDistrib::~C_DataLogRTDistrib()");
  m_sem_rt->P();
  DELETE_TABLE(m_p_rt_distrib);
  DELETE_TABLE(m_c_rt_distrib);
  m_sem_rt->V();
  DELETE_VAR(m_sem_rt);
}


int C_DataLogRTDistrib::data (char *P_data) {
  (*m_output_stream) << P_data << iostream_endl << iostream_flush ;
  return (1);
}


int C_DataLogRTDistrib::data (double P_time, double P_value) {
  unsigned long   L_index    ;

  L_index = (unsigned long) floor(P_value) ;

  m_sem_rt->P();
  if(L_index >= m_rtdistrib) {
    L_index = m_rtdistrib ;
  }
  
  m_p_rt_distrib[L_index] ++;
  m_c_rt_distrib[L_index] ++;
  
  m_p_min_rt_distrib = min(m_p_min_rt_distrib,L_index);
  m_c_min_rt_distrib = min(m_c_min_rt_distrib,L_index);
  m_p_max_rt_distrib = max(m_p_max_rt_distrib,L_index);
  m_c_max_rt_distrib = max(m_c_max_rt_distrib,L_index);

  m_counter_rt ++;

  if (m_counter_rt >= m_size_log) { 
    m_sem->V(); 
  }
  m_sem_rt->V();
    
  return (1);
}

void C_DataLogRTDistrib::do_log () {

  size_t         L_i ;
  unsigned long *L_tab ;
  unsigned long  L_min ;
  unsigned long  L_max ;
  
  
  m_sem_rt->P();
  if (m_counter_rt) {
    if (m_dump_end_procedure == false) {
      (*m_output_stream) << "Dump Periodic;" << iostream_endl << iostream_flush ;
      L_tab = m_p_rt_distrib ;
      L_min = m_p_min_rt_distrib ;
      L_max = m_p_max_rt_distrib ;
    } else {
      (*m_output_stream) << "Dump Cumulative;" << iostream_endl << iostream_flush ;
      L_tab = m_c_rt_distrib ;
      L_min = m_c_min_rt_distrib ;
      L_max = m_c_max_rt_distrib ;
    }
    
    for(L_i = L_min ; L_i <= L_max ; L_i++) {
      
      if(L_tab[L_i] != 0) {
	(*m_output_stream) << L_i << ";"
			   << L_tab[L_i] << ";" ;
	(*m_output_stream) << iostream_endl << iostream_flush ;
      }
    }
    
    
    for (L_i=0 ; L_i <= m_rtdistrib ; L_i++) {
      m_p_rt_distrib[L_i] = 0;
    }
    m_p_min_rt_distrib = m_rtdistrib ;
    m_p_max_rt_distrib = 0 ;
    m_counter_rt = 0 ;
  }
  m_sem_rt->V();

}






