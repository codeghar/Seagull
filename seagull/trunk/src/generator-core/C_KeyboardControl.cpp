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

#include "C_KeyboardControl.hpp"

#include "Utils.hpp"
#include "GeneratorTrace.hpp"

#include "C_Generator.hpp"

#include "integer_t.hpp" // for strtoul_f

#include <pthread.h> // for sched_yield()
#include <cstring>
#include <cctype>
#include <sys/time.h>

C_KeyboardControl::C_KeyboardControl():C_TaskControl(){
  m_gen = NULL ;
}

C_KeyboardControl::~C_KeyboardControl(){
  m_gen = NULL ;
}

void C_KeyboardControl::init_controller_configuration () {

  if (tcgetattr(0, &m_saved_conf) < 0) {
    GEN_FATAL(E_GEN_NO_ERROR, "Keyboard saved configuration failure");
  }


  m_controller_conf = m_saved_conf ;

  m_controller_conf.c_lflag &= ~(ICANON|ECHO);
  m_controller_conf.c_cc[VMIN]  = 0;
  m_controller_conf.c_cc[VTIME] = 0;

  if (tcsetattr (0, TCSANOW, &m_controller_conf) < 0) {
    GEN_FATAL(E_GEN_NO_ERROR, "Keyboard initial configuration failure");
  }
}

void C_KeyboardControl::controller_configuration () {
  if (tcsetattr (0, TCSANOW, &m_controller_conf) < 0) {
    GEN_FATAL(E_GEN_NO_ERROR, "Keyboard setting configuration fatal error");
  }
}

void C_KeyboardControl::system_configuration () {
  if (tcsetattr (0, TCSANOW, &m_saved_conf) < 0) {
    GEN_FATAL(E_GEN_NO_ERROR, "Keyboard restore configuration fatal error");
  }
}

void C_KeyboardControl::init(C_Generator *P_gen) {
  init_controller_configuration();
  m_gen = P_gen ;
  C_TaskControl::init() ;
}

T_GeneratorError C_KeyboardControl::InitProcedure() {
  GEN_DEBUG(0, "C_KeyboardControl::doInit() start");
  GEN_DEBUG(0, "C_KeyboardControl::doInit() end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_KeyboardControl::TaskProcedure() {

  fd_set              L_ReadMask ;
  int                 L_n = 0    ;
  int                 L_rc       ;
  struct timeval      L_TimeOut  ;
  char                L_char     ;

  GEN_DEBUG(0, "C_KeyboardControl::doTask() start");

  L_TimeOut.tv_sec        = 1 ;
  L_TimeOut.tv_usec       = 0 ;

  FD_ZERO(&L_ReadMask);
  FD_SET(0, &L_ReadMask); // O for stdin

  L_n = select(1,&L_ReadMask,NULL,NULL,&L_TimeOut);
  // no check on select error on purpose
  // select popped with event
  if (L_n > 0) {

    if (FD_ISSET(0, &L_ReadMask)) {
      L_rc = read(0, &L_char, 1) ;
      if (L_rc == 1) { execute_cmd(L_char); }
    }
    
  } // if L_n > 0

  GEN_DEBUG(0, "C_KeyboardControl::doTask() end");
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_KeyboardControl::EndProcedure() {
  system_configuration();
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_KeyboardControl::ForcedStoppingProcedure() {
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_KeyboardControl::StoppingProcedure() {
  M_state = C_TaskControl::E_STATE_STOPPED ;
  return (E_GEN_NO_ERROR);
}

void C_KeyboardControl::force_end_procedure () {
  (void) EndProcedure() ;
}

void C_KeyboardControl::execute_cmd(char P_char) {

  switch (P_char) {
  case 'a':
    m_gen->activate_percent_traffic();
    break ;

  case 'd':
    m_gen->reset_cumul_counters();
    break ;
  case 'q':
    m_gen->stop();
    break ;
  case 'p':
    m_gen->pause_traffic();
    break ;
  case 'b':
    m_gen->burst_traffic();
    break ;
    
  case 'f':
    m_gen->force_init();
    break ;
    
  case 'c': {
    char L_command[100] ;
    m_gen->pause_display() ;
    system_configuration() ;
    fprintf(stderr, "Command: ");
    memset(L_command, '\0', 100);
    read(0, L_command, 100);
    controller_configuration();
    m_gen->pause_display() ;
    analyze_cmd(L_command) ;
  }
  break ;
  
  case '+':
    m_gen->change_call_rate(E_GEN_OP_INCREASE, 0L);
    break ;

  case '-':
    m_gen->change_call_rate(E_GEN_OP_DECREASE, 0L);
    break ;

  default:
    m_gen->set_screen(P_char) ;
    break ;

  }
}

void C_KeyboardControl::analyze_cmd(char *P_cmd) {

  size_t  L_cmd_len, L_pos ;
  char   *L_ptr ;

  bool    L_cmd_found = false ;
  bool    L_cmd_error = false ;
  char   *L_arg1 = NULL;
  char   *L_arg2 = NULL;
  unsigned long L_value ;
  char *L_end_str = NULL ;

  L_cmd_len = strlen(P_cmd);
  L_pos = 0 ;
  while (isspace(P_cmd[L_pos]) && (L_pos < L_cmd_len)) {
    L_pos++ ;
  }
  if (L_pos == L_cmd_len) L_cmd_error = true ;

  if (L_cmd_error == false) {
    L_ptr = strstr(&P_cmd[L_pos], "set");
    if (L_ptr == &P_cmd[L_pos]) {
      L_pos += 3 ;
      if (L_pos >= L_cmd_len) L_cmd_error = true ;
    } else {
      L_cmd_error = true ;
    }
  }

  // skip space
  if (L_cmd_error == false) {
    while (isspace(P_cmd[L_pos]) && (L_pos < L_cmd_len)) {
      L_pos++ ;
    }
    if (L_pos == L_cmd_len) L_cmd_error = true ;
  }

  if (L_cmd_error == false) {
    L_arg1 = &P_cmd[L_pos];
    while ((!isspace(P_cmd[L_pos])) && (L_pos < L_cmd_len)) {
      L_pos++ ;
    }
    if (L_pos == L_cmd_len) { 
      L_cmd_error = true ; 
    } else {
      P_cmd[L_pos] = '\0' ;
      L_pos++ ;
      if (L_pos == L_cmd_len) L_cmd_error = true ;
    }
  }
   
  if (L_cmd_error == false) {
    while (isspace(P_cmd[L_pos]) && (L_pos < L_cmd_len)) {
    }
    if (L_pos == L_cmd_len) L_cmd_error = true ;
  }  
    
  if (L_cmd_error == false) {
    L_arg2 = &P_cmd[L_pos];
    while ((!isspace(P_cmd[L_pos])) && (L_pos < L_cmd_len)) {
      L_pos++ ;
    }
    if (L_pos == L_cmd_len) { 
      L_cmd_error = true ; 
    } else {
      P_cmd[L_pos] = '\0' ;
      L_cmd_found = true ;
    }
  }
    
  if (L_cmd_found == true) {
    
    if (strcmp(L_arg1, "call-rate") == 0) {
      L_value = strtoul_f(L_arg2, &L_end_str,10) ;
      if (L_end_str[0] != '\0') { // not a number
	L_cmd_error = true ;
      } else { // execute command
	m_gen->change_call_rate(E_GEN_OP_SET_VALUE, L_value);
      }
    } else if (strcmp(L_arg1, "burst-limit") == 0) {
      L_value = strtoul_f(L_arg2, &L_end_str,10) ;
      if (L_end_str[0] != '\0') { // not a number
	L_cmd_error = true ;
      } else { // execute command
	m_gen->change_burst(L_value);
      }
    } else if (strcmp(L_arg1, "call-rate-scale") == 0) {
      L_value = strtoul_f(L_arg2, &L_end_str,10) ;
      if (L_end_str[0] != '\0') { // not a number
	L_cmd_error = true ;
      } else { // execute command
	m_gen->change_rate_scale(L_value);
      }
    } else if (strcmp(L_arg1, "display-period") == 0) {
      L_value = strtoul_f(L_arg2, &L_end_str,10) ;
      if (L_end_str[0] != '\0') { // not a number
	L_cmd_error = true ;
      } else { // execute command
	m_gen->change_display_period(L_value);
      }
    } else {
      L_cmd_found = false ;
      L_cmd_error = true ;
    }
  }
    
  if (L_cmd_error == true) {

    GEN_ERROR(E_GEN_FATAL_ERROR, "Unknown command ignored");
  }

}

// end of file
