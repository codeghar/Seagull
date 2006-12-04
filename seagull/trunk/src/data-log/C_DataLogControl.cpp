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
#include "C_DataLogControl.hpp"

#include <pthread.h>
#include "iostream_t.hpp"

C_DataLogControl::C_DataLogControl(size_t         P_max_data,
				   size_t         P_max_data_log,
				   char          *P_file_name, 
				   unsigned long  P_period) {

  size_t L_i ;


  GEN_DEBUG(0, "C_DataLogControl::C_DataLogControl()");

  m_size_log = P_max_data_log ;
  m_size     = P_max_data ;

  NEW_VAR(m_sem, C_SemaphoreTimed(P_period));
  NEW_VAR(m_available_sem, C_Semaphore());
  NEW_VAR(m_values_sem, C_Semaphore());

  NEW_VAR(m_values, T_LogValuePList());
  NEW_VAR(m_available, T_LogValuePList());
  NEW_VAR(m_log, T_LogValuePList());

  ALLOC_TABLE(m_data_values, C_LogValue**, sizeof(C_LogValue*), m_size);
  for(L_i = 0; L_i < m_size; L_i++) {
    NEW_VAR(m_data_values[L_i], C_LogValue((char*)""));
    // init all to available
    m_available->push_back(m_data_values[L_i]);
  }

  NEW_VAR(m_output_stream, fstream_output(P_file_name));
  if (!m_output_stream->good()) {
    GEN_FATAL(0, "Unable to open file ["
	      << P_file_name << "] for data logging");
  }
  
  m_time_ref = 0.0 ;
}

C_DataLogControl::~C_DataLogControl() {

  size_t L_i ;

  GEN_DEBUG(0, "C_DataLogControl::~C_DataLogControl()");

  DELETE_VAR(m_sem);
  DELETE_VAR(m_available_sem);
  DELETE_VAR(m_values_sem);

  if (!m_values->empty()) {
    m_values->erase(m_values->begin(), m_values->end());
  }
  if (!m_available->empty()) {
    m_available->erase(m_available->begin(), m_available->end());
  }
  if (!m_log->empty()) {
    m_log->erase(m_log->begin(), m_log->end());
  }
  DELETE_VAR(m_values);
  DELETE_VAR(m_available);
  DELETE_VAR(m_log);

  for(L_i = 0; L_i < m_size; L_i++) {
    DELETE_VAR(m_data_values[L_i]);
  }
  FREE_TABLE(m_data_values);
  DELETE_VAR(m_output_stream);


}

T_GeneratorError C_DataLogControl::InitProcedure () {
  T_GeneratorError L_err = E_GEN_NO_ERROR ;

  struct timezone L_zone ;
  struct timeval  L_time;

  GEN_DEBUG(0, "C_DataLogControl::InitProcedure()");

  gettimeofday(&L_time, &L_zone);

  m_time_ref = (double)L_time.tv_sec*1000.0 + (double)L_time.tv_usec/1000.0 ;
 
  m_sem->P() ;

  return (L_err);
}


T_GeneratorError C_DataLogControl::TaskProcedure () {
  T_GeneratorError L_err = E_GEN_NO_ERROR ;

  GEN_DEBUG(0, "C_DataLogControl::TaskProcedure()");

  sched_yield() ;
  do_log() ;
  m_sem->P() ;

  return (L_err);
}

T_GeneratorError C_DataLogControl::EndProcedure () {
  T_GeneratorError L_err = E_GEN_NO_ERROR ;

  GEN_DEBUG(0, "C_DataLogControl::EndProcedure()");
  do_log () ;
  do_log_cumul () ;
  return (L_err);
}

T_GeneratorError C_DataLogControl::StoppingProcedure () {
  T_GeneratorError L_err = E_GEN_NO_ERROR ;

  M_state = C_TaskControl::E_STATE_STOPPED ;
  m_sem->V();

  GEN_DEBUG(0, "C_DataLogControl::StoppingProcedure()");
  return (L_err);
}

T_GeneratorError C_DataLogControl::ForcedStoppingProcedure () {
  T_GeneratorError L_err = E_GEN_NO_ERROR ;
  GEN_DEBUG(0, "C_DataLogControl::ForcedStoppingProcedure()");
  return (L_err);
}


int C_DataLogControl::data (double P_time, double P_data) {

  int                   L_ret          = 0 ;
  T_LogValuePList::iterator L_it ;
  C_LogValue*               L_buf          = NULL ;
  size_t                L_waiting_data = 0 ;
  
  GEN_DEBUG(0, "C_DataLogControl::data(" << P_data << ")");
  m_available_sem->P();
  if (!m_available->empty()) {
    L_it = m_available->begin() ;
    L_buf = *L_it ;
    m_available->erase(L_it);
  }
  m_available_sem->V();

  if (L_buf != NULL) { // a buffer is available

    L_buf->set_data(P_time, P_data);
    m_values_sem->P();
    m_values->push_back(L_buf);
    L_waiting_data = m_values->size();
    GEN_DEBUG(0, "C_DataLogControl::data() waiting list size=" 
	      << L_waiting_data);
    m_values_sem->V();
    if (L_waiting_data >= m_size_log) { m_sem->V(); }
    L_ret = 1 ;
  }

  return (L_ret);
}


int C_DataLogControl::data (char* P_data) {

  int                   L_ret          = 0 ;
  T_LogValuePList::iterator L_it ;
  C_LogValue*               L_buf          = NULL ;
  size_t                L_waiting_data = 0 ;
  
    GEN_DEBUG(0, "C_DataLogControl::data(" << P_data << ")");
    m_available_sem->P();
    if (!m_available->empty()) {
      L_it = m_available->begin() ;
      L_buf = *L_it ;
      m_available->erase(L_it);
    }
    m_available_sem->V();
    
    if (L_buf != NULL) { // a buffer is available
      L_buf->set_string(P_data);
      m_values_sem->P();
      m_values->push_back(L_buf);
      L_waiting_data = m_values->size();
      GEN_DEBUG(0, "C_DataLogControl::data() waiting list size=" 
		<< L_waiting_data);
      m_values_sem->V();
      if (L_waiting_data >= m_size_log) { m_sem->V(); }
      L_ret = 1 ;
    }
    
    return (L_ret);
}

void C_DataLogControl::do_log_cumul () {
}


void C_DataLogControl::do_log () {

  T_LogValuePList::iterator L_it ;
  C_LogValue*               L_buf=NULL ;
  size_t                    L_nb_data=0  ;
  char                      L_final_string [ 100 ] ;

  GEN_DEBUG(0, "C_DataLogControl::do_log()");
  // critical section => just copy the pointers
  m_values_sem->P() ;
  GEN_DEBUG(0, "C_DataLogControl::do_log() waiting list size=" << m_values->size());
  if ((L_nb_data=m_values->size())) {
    while (L_nb_data) {
      L_it = m_values->begin() ;
      GEN_DEBUG(0, "C_DataLogControl::do_log() data=" << *L_it);
      m_log->push_back(*L_it);
      m_values->erase(L_it);
      L_nb_data -- ;
    }
  }
  m_values_sem->V() ;

  if ((L_nb_data=m_log->size())) {

    while (L_nb_data) {
      
      L_it = m_log->begin();
      L_buf = *L_it ;

      L_buf->get_value(L_final_string);
      // make sys log
      *m_output_stream << L_final_string << iostream_endl << iostream_flush ;

      m_available_sem->P();
      m_available->push_back(L_buf);
      m_available_sem->V();
      m_log->erase(L_it);

      L_nb_data-- ;
    }
  }

}

int C_DataLogControl::time_data(struct timeval *P_begin,
				struct timeval *P_end) {

  // convert data to string 

  double L_diff, L_time ;
  long L_val_sec, L_val_usec ;

  L_time = (double)P_end->tv_sec*1000.0 + (double)(P_end->tv_usec)/(double)1000.0 ;
  L_time = (double)L_time - (double)m_time_ref ;

  L_val_sec = P_end->tv_sec - P_begin->tv_sec ;
  L_val_usec = P_end->tv_usec - P_begin->tv_usec ;
  if (L_val_usec < 0) { L_val_usec += 1000000; L_val_sec --; }

  L_diff = (double) L_val_sec*1000.0 + (double)(L_val_usec)/1000.0 ;

  return (data(L_time, L_diff));

}






