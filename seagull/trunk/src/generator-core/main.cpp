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

/**
  * \mainpage Seagull code documentation
  * Go back to user documentation: http://gull.sf.net/doc/
  *
  * \section sec_intro Introduction
  * The purpose of this documentation is to explain Seagull interns. 
  * It is a summary to help developers make their way through Seagull and 
  * start contributing.
  * 
  * \section sec_modules Main modules
  * - C_Generator: core of Seagull. Contains the main functions.
  * - C_TrafficModel: internal model of Seagull to control the way new scenarios 
  * are created. 
  * - C_CallContext: Management of the call context
  * - C_CallControl: Management of the calls. Pause, restart traffic, set call rate...  
  * - C_ReadControl: Takes care of reading the protocol events, keyboard events 
  * and associated scheduling.
  * - generator-scenario: C_ScenarioControl provides methods relating to the 
  * execution of the scenario. C_Scenario takes care of reading and 
  * interpreting scenarios.
  * - ProtocolData.cpp: definition of generic data types (string, number, 
  * number64, ...) used by Seagull, as well as associated operator overloads.
  * - C_ProtocolFrame: virtual definition of a protocol and its messages. It 
  * contains the only objects that will be used by the generator
  * - C_Transport: generic definition of a transport
  * - C_CommandAction: Class from which derives all classes implementing an
  * action. Start here if you want to add one.
  * - C_DisplayObject: Class that displays various screens (statistics, help,
  * traffic, ...)
  * - 
  * 
  * \section sec_other_modules Other modules
  * - common directory: contains all the .h files to adapt some system includes 
  * to the target compilation platform
  * - C_DataLogRTDistrib: deals with response time logs (csv file)
  * - exe-env directory: execution environment for each protocol
  * - C_ExternalDataControl: manage external data files, to insert data into 
  * scenario messages during the call
  * - generator-common: utils for the tool (buffer management, id, ...)
  * - C_TransIP: IP v4 & v6 transport transport; daughter class of C_Transport
  * - C_TransSCTP: SCTP transport library; daughter class of C_Transport
  * - C_TransIPTLS: TLS transport; daughter class of C_TransIP
  * - C_TransOCTcap32: OpenCall TCAP API transport; daughter class of C_Transport
  * - C_ProtocolBinary: binary protocol definition (e.g. Diameter); daughter 
  * class of C_ProtocolBinaryFrame, C_MessageFrame
  * - statistics: stats management
  * - C_XmlData and C_XmlParser to parse the xml files (dictionary, scenario 
  * and configuration 
  * file)
  *
  * \section sec_threads Threads
  * There are 5 threads in Seagull:
  *
  * - One thread that handles the whole traffic,
  * - One thread for the keyboard,
  * - One thread for the tool display,
  * - One thread for the logs,
  * - One thread for the statistics.
  * They are handled in generator-core module
  *
  * \section sec_proto_msg Protocol and messages
  *
  * - To implement a text protocol, you need to create a new implementation of 
  * C_ProtocolFrame and C_MessageFrame like C_ProtocolBinary and C_MessageBinary
  * - In those classes, you implement the management of the XML tags and the 
  * protocol definition
  * - You need to add them in C_ProtocolControl to be instantiated by the tool
  *
  * \section sec_xmlparse XML parsing
  *
  * - Handled in module xml-parser
  * - XML files are parsed by C_XmlParser
  * - XML data are represented as a tree of elements (based on stl types): 
  * (<TAG (FIELD=“VALUE”)*> <\TAG>)*
  * - Useful functions in C_XmlData allows to get the name of a tag, retrieve 
  * the value of a field or the list (stl) of element under one element.
  *
  * \section sec_channel Channel creation
  *
  * - C_TransIP (module library-trans-ip) implements the transport, based on 
  * virtual object C_Transport (module transport-frame)
  * Done/Declared in conf file.
  * - Channel = link between one protocol and one transport, with an open 
  * command for the transport
  *
  * \section sec_multichan Multi-channel policy
  *
  * - Automatic session based: possible to open several channels (using same 
  * or different protocols), but only the first channel used can be server.
  * 
  */
 
#include "Utils.hpp"
#include "C_Generator.hpp"
#include "GeneratorTrace.hpp"
#include "TimeUtils.hpp"


#include "cmd_line_t.hpp"

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
  
  cmd_line_pt      L_cmd_line   ;

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


  L_cmd_line = create_cmd_line((P_argc+2)) ;
  copy_cmd_line(L_cmd_line, P_argc, P_argv);


  //   NEW_VAR(L_Generator, C_Generator(P_argc, P_argv));
  NEW_VAR(L_Generator, C_Generator(L_cmd_line));

  L_main = L_Generator ;
 
  L_error_code = L_Generator -> init () ;
  if (L_error_code == E_GEN_NO_ERROR) {
    L_error_code = L_Generator -> run_all_once () ; 
  }
  
  // Clean Generator main object
  DELETE_VAR(L_Generator);


  destroy_cmd(L_cmd_line);
  L_cmd_line->m_nb_args += 2 ;
  DELETE_VAR(L_cmd_line);

  // Close trace
  close_trace () ;

  return (L_error_code) ;
}

int main(int P_argc, char**P_argv) {
  int L_nRet;
  
  L_nRet = main_tool(P_argc, P_argv);

  return L_nRet;
}








