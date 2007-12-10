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

#include <cstdlib>
#include <pthread.h> // for sched_yield definition

#include "Utils.hpp"
#include "GeneratorTrace.hpp"
#include "C_LogProtocolStatControl.hpp"


C_LogProtocolStatControl::C_LogProtocolStatControl(C_ProtocolStatsFrame *P_stats):C_TaskControl(){
  m_sem = NULL ;
  m_stat = P_stats ;
}

C_LogProtocolStatControl::~C_LogProtocolStatControl(){
  DELETE_VAR (m_sem) ;
  m_stat = NULL ;
}

void  C_LogProtocolStatControl::init(unsigned long P_period, string_t& P_fileName) {
  GEN_DEBUG(0, "C_LogProtocolStatControl::init() start");

  m_stat -> set_file_name ((char*)P_fileName.c_str());
  NEW_VAR(m_sem, C_SemaphoreTimed(P_period));
  m_sem -> P();
  C_TaskControl::init() ;
  GEN_DEBUG(0, "C_LogProtocolStatControl::init() end");
}


T_GeneratorError C_LogProtocolStatControl::InitProcedure() {
  GEN_DEBUG(0, "C_LogProtocolStatControl::doInit() start");
  m_stat->init();
  m_stat->make_first_log();
  GEN_DEBUG(0, "C_LogProtocolStatControl::doInit() end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_LogProtocolStatControl::TaskProcedure() {
  GEN_DEBUG(0, "C_LogProtocolStatControl::doTask() start");
  m_stat->make_log ();
//    m_stat->executeStatAction(C_GeneratorStats::E_RESET_PL_COUNTERS);
  sched_yield () ;
  m_sem -> P() ;
  GEN_DEBUG(0, "C_LogProtocolStatControl::doTask() end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_LogProtocolStatControl::EndProcedure() {
  m_stat->make_log ();
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_LogProtocolStatControl::StoppingProcedure() {
  M_state = C_TaskControl::E_STATE_STOPPED ;
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_LogProtocolStatControl::ForcedStoppingProcedure() {
  return (E_GEN_NO_ERROR);
}

void C_LogProtocolStatControl::reset_cumul_counters() {
  m_stat->reset_cumul_counters();
}



// end of file

