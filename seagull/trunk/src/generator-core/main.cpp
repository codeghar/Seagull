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
#include "C_Generator.hpp"
#include "GeneratorTrace.hpp"
#include "TimeUtils.hpp"

#include <sys/time.h>

// #include <sys/resource.h>
// #include <unistd.h>

#include <csignal> // sigaction definition
#include <cstring>  // for memset

static T_pC_Generator L_main ;
static struct timeval L_stop_pressed, L_previous ;

static void sig_usr1_actions (int P_notused) {
  if (L_main != NULL) {
      L_main -> stop () ;
  }
}

static void sig_int_actions (int P_notused) {
  gettimeofday(&L_stop_pressed, NULL);
  if (L_main != NULL) {
    if (ms_difftime (&L_stop_pressed, &L_previous) > 1000) {
      L_previous = L_stop_pressed ;
      L_main -> stop () ;
    }
  }
}

int main_tool(int P_argc, char**P_argv) {

  struct sigaction L_sig_stop   ;
  struct sigaction L_sig_usr1   ;
  T_pC_Generator   L_Generator  ;
  T_GeneratorError L_error_code ;

//   struct rlimit    L_rlimit     ;

  gettimeofday(&L_previous, NULL);

  memset(&L_sig_stop, 0, sizeof(struct sigaction));
  L_sig_stop.sa_handler = sig_int_actions ;
  if (sigaction(SIGINT, &L_sig_stop, NULL)) {
    GEN_FATAL(1,"SIGINT handler error");
  }

  memset(&L_sig_usr1, 0, sizeof(struct sigaction));
  L_sig_usr1.sa_handler = sig_usr1_actions ;
  if (sigaction(SIGUSR1, &L_sig_usr1, NULL)) {
    GEN_FATAL(1,"SIGINT handler error");
  }
  

//   if (getrlimit (RLIMIT_NOFILE, &L_rlimit) < 0) {
//     GEN_FATAL(1, "getrlimit failed");
//   }
//   L_rlimit.rlim_max = 10000 ;
//   L_rlimit.rlim_cur = L_rlimit.rlim_max ;
//   if (setrlimit (RLIMIT_NOFILE, &L_rlimit) < 0) {
//     GEN_FATAL(1, "RLIMIT_NOFILE modification failed");
//   }

  NEW_VAR(L_Generator, C_Generator(P_argc, P_argv));
  L_main = L_Generator ;
 
  L_error_code = L_Generator -> init () ;
  if (L_error_code == E_GEN_NO_ERROR) {
    L_error_code = L_Generator -> run_all_once () ; 
  }
  
  // Clean Generator main object
  DELETE_VAR(L_Generator);

  // Close trace
  close_trace () ;

  return (L_error_code) ;
}

int main(int P_argc, char**P_argv) {
  int L_nRet;
  
  L_nRet = main_tool(P_argc, P_argv);

  return L_nRet;
}

