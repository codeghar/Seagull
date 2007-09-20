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

#include "C_RemoteControl.hpp"
#include "C_TransportControl.hpp"


#include "Utils.hpp"
#include "GeneratorTrace.hpp"
#include "C_MessageText.hpp"

#include "ProtocolData.hpp"
#include <regex.h>

#include "C_Generator.hpp"
#include "C_SemaphoreTimed.hpp"

#include "dlfcn_t.hpp"

#include <cerrno>  // for errno definition

#include <cstdlib>
#include <pthread.h> // for sched_yield()


void _gen_log_error_control(char *P_data) {
  GEN_ERROR(E_GEN_FATAL_ERROR, P_data);
}

void _gen_log_info_control(char *P_data) {
  GEN_LOG_EVENT_FORCE(P_data);
}


class C_RampControl : public C_TaskControl {

 public:

  C_RampControl(C_Generator *P_gen) ;
  ~C_RampControl() ;
  
  void  init (unsigned long P_duration,
              unsigned long P_current_rate,
              unsigned long P_sub_rate,
              bool          P_increase);

private :
  
  C_SemaphoreTimed *m_sem ;
  unsigned long m_duration ;
  unsigned long m_current_rate ;
  unsigned long m_sub_rate ;
  bool m_increase ;
  C_Generator *m_gen;

  T_GeneratorError TaskProcedure () ;
  T_GeneratorError InitProcedure () ;
  T_GeneratorError EndProcedure () ;
  T_GeneratorError StoppingProcedure () ;
  T_GeneratorError ForcedStoppingProcedure() ;
} ;

C_RampControl::C_RampControl(C_Generator *P_gen) {
  m_sem = NULL ;
  m_duration = 0;
  m_current_rate = 0;
  m_sub_rate = 0;
  m_increase = false;
  m_gen = P_gen;
}

C_RampControl::~C_RampControl() {
  DELETE_VAR(m_sem);
}



void C_RampControl::init(unsigned long P_duration,
                         unsigned long P_current_rate,
                         unsigned long P_sub_rate,
                         bool          P_increase) {
  m_duration = P_duration;
  m_current_rate = P_current_rate;
  m_sub_rate = P_sub_rate;
  m_increase = P_increase;
  NEW_VAR(m_sem, C_SemaphoreTimed(1));
  m_sem -> P();
  C_TaskControl::init() ;
}

T_GeneratorError C_RampControl::ForcedStoppingProcedure() {
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_RampControl::EndProcedure() {

  delete this ;
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_RampControl::StoppingProcedure() {

  M_state = C_TaskControl::E_STATE_STOPPED ;
  m_sem->V();

  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_RampControl::InitProcedure() {
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_RampControl::TaskProcedure() {

  if (m_increase) {
    m_current_rate += m_sub_rate ;
  } else {
    m_current_rate -= m_sub_rate ;
  }  

  m_duration-- ;
  m_gen->change_call_rate(E_GEN_OP_SET_VALUE, m_current_rate);

  if (m_duration == 0) stop() ;

  sched_yield () ;
  m_sem -> P() ;

  return (E_GEN_NO_ERROR);
}


C_RemoteControl::C_RemoteControl(C_Generator      * P_gen,
                                 C_ProtocolControl* P_protocol_control,
                                 char              *P_address):C_TaskControl(){
  m_gen = P_gen ;
  m_call_select = &select ;
  m_events = NULL ;
  m_protocol_ctrl  = P_protocol_control ;
  m_transport = NULL ;


  ALLOC_TABLE(m_address,
	      char*,
	      sizeof(char),
	      strlen(P_address)+1);
  strcpy(m_address, P_address);

  NEW_VAR(m_msg_remote_list, T_RcvMsgCtxtList());
  m_protocol_frame = NULL ;
  m_protocol_text  = NULL ;
  m_stat = NULL ;

}

C_RemoteControl::~C_RemoteControl(){
  DELETE_TABLE(m_events);
  m_protocol_ctrl = NULL ;
  DELETE_VAR(m_msg_remote_list);
  FREE_TABLE(m_address);
  m_protocol_frame = NULL ;
  m_protocol_text  = NULL ;
  m_stat = NULL ;

}

void  C_RemoteControl::init() {
  GEN_DEBUG(0, "C_RemoteControl::init() start");
  C_TaskControl::init() ;
  m_stat = C_GeneratorStats::instance() ;

  GEN_DEBUG(0, "C_RemoteControl::init() end");
}

T_GeneratorError C_RemoteControl::InitProcedure() {
  T_GeneratorError L_error_code = E_GEN_NO_ERROR ;
  int              L_ret ;
  int              L_open_id ;
  C_TransportControl::T_TransportContext  L_context ;
  T_SelectDef              L_select ;

  T_OpenStatus   L_status  ;


  GEN_DEBUG(0, "C_RemoteControl::InitProcedure() start");
  m_max_event_nb = 10; 
  NEW_TABLE(m_events, C_TransportEvent, m_max_event_nb);
  L_ret =   m_protocol_ctrl->get_protocol_id((char*)"cmd-seagull-protocol") ;
  if (L_ret != -1) {
    m_protocol_frame = m_protocol_ctrl->get_protocol(L_ret);
    m_protocol_text  = dynamic_cast<C_ProtocolText*> (m_protocol_frame);

    m_command_id = m_protocol_text->find_field_id((char*)"type");
    m_uri_id = m_protocol_text->find_field_id((char*)"uri");

  } else {
    L_error_code= E_GEN_FATAL_ERROR;
  }
  
  // create transport dlopen transport
  
  L_context.m_lib_handle 
    = dlopen((char*)"libtrans_ip.so", RTLD_LAZY);
  if (L_context.m_lib_handle == NULL) {
    GEN_ERROR(E_GEN_FATAL_ERROR, 
	      "Unable to open library file [" 
	      << (char*)"libtrans_ip.so"
	      << "] error [" << dlerror() << "]");
    L_error_code = E_GEN_FATAL_ERROR;
  }
  
  if (L_error_code == E_GEN_NO_ERROR) {
    
    L_context.m_ext_create
      = (T_TransportCreateInstance) dlsym
      (L_context.m_lib_handle, 
       (char*)"create_cipio_instance");
    
    if (L_context.m_ext_create == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Symbol [" << dlerror() << "]");
      L_error_code = E_GEN_FATAL_ERROR;
    }
    
    if (L_error_code == E_GEN_NO_ERROR) {
      L_context.m_ext_delete
        = (T_TransportDeleteInstance) dlsym
        (L_context.m_lib_handle, 
         (char*)"delete_cipio_instance");
      
      if (L_context.m_ext_delete == NULL) {
        GEN_ERROR(E_GEN_FATAL_ERROR, "Symbol [" << dlerror() << "]");
        L_error_code = E_GEN_FATAL_ERROR;
      }
    }
    
    if (L_error_code == E_GEN_NO_ERROR) {
      L_context.m_instance = (*(L_context.m_ext_create))() ;
      m_transport = L_context.m_instance ;
      if (m_transport == NULL) {
        GEN_ERROR(E_GEN_FATAL_ERROR, "Transport instance not created");
        L_error_code = E_GEN_FATAL_ERROR ;
      } else {
        L_select = m_transport->select_definition() ;
        if (L_select != NULL) {
          m_call_select = L_select;
        }
      }
    }
  }

  if (L_error_code == E_GEN_NO_ERROR) {
    if (m_transport->init((char*)"type=tcp", 
                          _gen_log_error_control, 
                          _gen_log_info_control) != 0) {
      L_error_code = E_GEN_FATAL_ERROR ;
    }

    L_open_id = m_transport->open(10,m_address,
                                  &L_status,
                                  m_protocol_frame) ;
    if (L_status != E_OPEN_OK) {
      L_error_code = E_GEN_FATAL_ERROR ;
    }
  }

 GEN_DEBUG(0, "C_RemoteControl::doInit() end");
  return (L_error_code);
}

T_GeneratorError C_RemoteControl::TaskProcedure() {
  T_GeneratorError L_error_code ;
  
  while (M_state != C_TaskControl::E_STATE_STOPPED ) {
    L_error_code = receiveControl() ;
    if (L_error_code != E_GEN_NO_ERROR) {
      M_state = C_TaskControl::E_STATE_STOPPED ;
    } else {
      messageReceivedControl ();
    }
  }
  return (L_error_code);
}

T_GeneratorError C_RemoteControl::EndProcedure() {

  // close the instances of transport 
  if (!m_msg_remote_list->empty()) {
    m_msg_remote_list->erase(m_msg_remote_list->begin(),
			       m_msg_remote_list->end());
  }

  m_transport->close() ;

  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_RemoteControl::StoppingProcedure() {
  M_state = C_TaskControl::E_STATE_STOPPED ;
  return (E_GEN_NO_ERROR);
}

T_GeneratorError C_RemoteControl::ForcedStoppingProcedure() {
  return (E_GEN_NO_ERROR);
}


T_GeneratorError C_RemoteControl::receiveControl () {

  fd_set                    L_ReadMask             ;
  fd_set                    L_WriteMask            ;
  fd_set                    L_ExceptionMask        ;
  int                       L_n = 0                ;

  /* Number of Fd popped in the select */
  int                       L_MaxFd = 0            ; 
  int                       L_error = 0            ;

  struct timeval            L_TimeOut              ;
  struct timeval           *L_pTimeOut=&L_TimeOut  ;

  T_ReceiveMsgContext       L_currentRcvCtxt       ;

  size_t                    L_nb_event             ;
  size_t                    L_i                    ;


  T_pC_TransportEvent       L_event_occured        ;
  int                       L_event_id             ;

  GEN_DEBUG(1, "C_RemoteControl::receiveControl() start");
  
  L_TimeOut.tv_sec = 1;
  L_TimeOut.tv_usec = 0;
  
  /* Init masks for the select */
  FD_ZERO(&L_ReadMask);
  FD_ZERO(&L_WriteMask);
  FD_ZERO(&L_ExceptionMask);
  
  L_MaxFd = 0 ;
  
  L_MaxFd = m_transport->pre_select 
    ( L_MaxFd, 
      &L_ReadMask, 
      &L_WriteMask, 
      &L_ExceptionMask, 
      L_pTimeOut,
      NULL,
      0) ;
  
  
  if (L_MaxFd >= 1) {
    
    // select system call
    L_n = (*m_call_select) (L_MaxFd+1,
			    &L_ReadMask,
			    &L_WriteMask,
			    &L_ExceptionMask,
			    L_pTimeOut);
    
  }
  
  // select error cases
  if (L_n < 0) {
    switch (errno) {
    case EINTR :
      /*
       * NOTHING TO DO, AND MUST DO
       * SELECT HAS BEEN INTERRUPTED (EINTR) BY A SIGNAL
       */
      break;
    default :
      
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "select failed " << strerror(errno));
      break;
    }
  }

  // if L_n == 0 => nothing to do => no event => just timeout 
  GEN_DEBUG(1, "select return = " << L_n);

  // select popped with events
  if (L_n > 0) {
    L_nb_event = m_max_event_nb ; 
    L_error = m_transport->post_select
      (L_n, &L_ReadMask,&L_WriteMask, &L_ExceptionMask,
       m_events,
       (size_t*)&L_nb_event);
    
    if (L_error <0) {
      GEN_ERROR(E_GEN_FATAL_ERROR, "Post select (" << L_error << ")");
    } else { // L_error >= 0
      
      GEN_DEBUG(1, "C_RemoteControl::receiveControl() nb events = " << L_nb_event);
      
      for (L_i = 0 ; L_i < L_nb_event ; L_i++) {
        L_event_occured = &m_events [L_i];
        L_event_id      = L_event_occured->m_id ;
        
        switch (L_event_occured->m_type) {
          
        case C_TransportEvent::E_TRANS_RECEIVED: {
          
          GEN_DEBUG(1, "C_RemoteControl::receiveControl() event");
          GEN_DEBUG(1, 
                    "C_RemoteControl::receiveControl() E_TRANS_RECEIVED id ["
                    << L_event_id << "]");
          while ((m_transport
                  ->get_message(L_event_occured->m_id, &L_currentRcvCtxt)) == true) {
            m_msg_remote_list -> push_back (L_currentRcvCtxt) ;
          }
          break ;
        }
        
        case C_TransportEvent::E_TRANS_CLOSED: {
          GEN_DEBUG(1, "C_RemoteControl::receiveControl() E_TRANS_CLOSED id["
                    << L_event_id << "]");
          break ;
        }
        
        case C_TransportEvent::E_TRANS_CONNECTION: {
          GEN_DEBUG(1, 
                    "C_ReadControl::receiveControl() E_TRANS_CONNECTION id["
                    << L_event_id << "]");
          break ;
        }
        
        case C_TransportEvent::E_TRANS_OPEN: {
          GEN_DEBUG(1, 
                    "C_ReadControl::receiveControl() E_TRANS_OPEN id ["
                    << L_event_id << "]");
        }
        
        default :
          break;
        } // switch L_event_occured
      } // for L_i
    } // if L_error
  } // if L_n > 0

  GEN_DEBUG(1, "C_RemoteControl::receiveControl() end");
  return (E_GEN_NO_ERROR);
  
}


void C_RemoteControl::messageReceivedControl () {
  

  int                 L_nbMsg, L_i ;
  
  C_MessageFrame     *L_msg = NULL ; 
  T_ReceiveMsgContext L_rcvCtxt ;

  C_MessageFrame     *L_msg_send  = NULL  ;

  L_nbMsg = m_msg_remote_list -> size();

  for(L_i=0; L_i < L_nbMsg; L_i++) {

    L_rcvCtxt = *(m_msg_remote_list -> begin()) ;
    m_msg_remote_list -> erase (m_msg_remote_list->begin()) ;

    L_msg = L_rcvCtxt.m_msg ;
    L_msg_send = analyze_command(L_msg);

    if (L_msg_send != NULL) {
      m_transport->send_message(L_rcvCtxt.m_response, L_msg_send);
      DELETE_VAR(L_msg_send);
    }
    
    DELETE_VAR(L_msg);
  }
}

char *C_RemoteControl::resultOK() {
  char *L_result = NULL ;
  ALLOC_TABLE(L_result,
              char*,
              sizeof(char),
              50);
  sprintf(L_result, "%s", "HTTP/1.1 200 OK\r\n");
  return (L_result);
}

char *C_RemoteControl::resultKO() {
  char *L_result = NULL ;
  ALLOC_TABLE(L_result,
              char*,
              sizeof(char),
              50);
  sprintf(L_result, "%s", "HTTP/1.1 400 Bad Request\r\n");
  return (L_result);
}


void C_RemoteControl::rate(unsigned long P_value) {
  m_gen->change_call_rate(E_GEN_OP_SET_VALUE, P_value);
}

void C_RemoteControl::createRampThread(unsigned long P_duration,
                                       unsigned long P_current_rate,
                                       unsigned long P_sub_rate,
                                       bool          P_increase) {
  // create thread
  pthread_t              *L_rampThread = NULL;
  C_RampControl          *L_rampCtrl ;

  NEW_VAR(L_rampCtrl, C_RampControl(m_gen));
  L_rampCtrl->init(P_duration,
                   P_current_rate,
                   P_sub_rate,
                   P_increase);
  
  L_rampThread = start_thread_control(L_rampCtrl);
}

void C_RemoteControl::quit() {
  m_gen->stop() ;
}

void C_RemoteControl::pause() {
  m_gen->pause_traffic() ;
}

void C_RemoteControl::resume() {
  m_gen->restart_traffic() ;
}

void C_RemoteControl::burst() {
  m_gen->burst_traffic() ;
}

void C_RemoteControl::ramp(unsigned long P_value, unsigned long P_duration) {

  unsigned long L_current_rate ;
  unsigned long L_diff_rate ;
  unsigned long L_sub_rate ;
  bool          L_increase = true ;

  if (P_duration == 0) {
    rate(P_value);
  } else {

    L_current_rate = m_gen->get_call_rate() ;
    
    if (P_value > L_current_rate) {
      L_diff_rate = P_value - L_current_rate ;
      L_sub_rate = L_diff_rate / P_duration ;
    } else {
      L_increase = false ;
      L_diff_rate = L_current_rate - P_value ;
      L_sub_rate = L_diff_rate / P_duration ;
    }

    if (P_value != L_current_rate) {
      createRampThread(P_duration,L_current_rate,L_sub_rate,L_increase);
    }
  }
}

char *C_RemoteControl::find_directory(char *P_buf,char *P_dir) {

  char *L_value = NULL ;

  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[3] ;

  string_t   L_string = "" ;
  
  L_string  = "([[:blank:]]*" ;
  L_string += P_dir ;
  L_string += "[[:blank:]]*)";

  L_status = regcomp (&L_reg_expr, 
		      L_string.c_str(),
		      REG_EXTENDED) ;

  if (L_status != 0) {
    regerror(L_status, &L_reg_expr, L_buffer, 100);
    regfree (&L_reg_expr) ;

  } else {
    L_status = regexec (&L_reg_expr, P_buf, 2, L_pmatch, 0) ;
    regfree (&L_reg_expr) ;
    if (L_status == 0) {
      L_value =  &P_buf[L_pmatch[1].rm_eo] ;
    } 
  }

  // position pas forcement /
  return (L_value);
}

char *C_RemoteControl::find_file(char *P_buf,char *P_dir) {
  char *L_value = NULL ;

  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[3] ;
  //  size_t     L_size = 0 ;

  string_t   L_string = "" ;
  string_t   L_string_end = "" ;
  string_t   L_string_tmp = "" ;
  
  L_string  = "([[:blank:]]*" ;
  L_string += P_dir ;
  L_string += "[[:blank:]]*";

  L_string_tmp = L_string + "\?)";
  L_string_end = L_string + "$)";

  // case tmp
  L_status = regcomp (&L_reg_expr, 
		      L_string_tmp.c_str(),
		      REG_EXTENDED) ;

  if (L_status != 0) {
    regerror(L_status, &L_reg_expr, L_buffer, 100);
    regfree (&L_reg_expr) ;
  } else {
    L_status = regexec (&L_reg_expr, P_buf, 2, L_pmatch, 0) ;
    regfree (&L_reg_expr) ;
    if (L_status == 0) {
      L_value =  &P_buf[L_pmatch[1].rm_eo] ;
    } else {
      // case end 
      L_status = regcomp (&L_reg_expr, 
                          L_string_end.c_str(),
                          REG_EXTENDED) ;
      
      if (L_status != 0) {
        regerror(L_status, &L_reg_expr, L_buffer, 100);
        regfree (&L_reg_expr) ;
      } else {
        L_status = regexec (&L_reg_expr, P_buf, 2, L_pmatch, 0) ;
        regfree (&L_reg_expr) ;
        if (L_status == 0) {
          L_value =  &P_buf[L_pmatch[1].rm_eo] ;
        }
      }
    }
  }
  return(L_value);
}

char* C_RemoteControl::find_value (char *P_buf, char *P_dir) {

  char *L_value = NULL ;

  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[3] ;
  size_t     L_size = 0 ;

  string_t   L_string = "" ;


  L_string  = "([[:blank:]&]*" ;
  L_string += P_dir ;
  L_string += "[^[:blank:]=]*[[:blank:]]*=[[:blank:]]*)([^&]*)";
  
  if ((P_buf[0] == '?') && (P_buf+1)) { 
    P_buf++;
    
    L_status = regcomp (&L_reg_expr, 
                        L_string.c_str(),
                        REG_EXTENDED) ;
    
    if (L_status != 0) {
      regerror(L_status, &L_reg_expr, L_buffer, 100);
      regfree (&L_reg_expr) ;
      
    } else {
      
      L_status = regexec (&L_reg_expr, P_buf, 3, L_pmatch, 0) ;
      regfree (&L_reg_expr) ;
      
      if (L_status == 0) {
        
        L_size = L_pmatch[2].rm_eo - L_pmatch[2].rm_so ;
        ALLOC_TABLE(L_value, char*, sizeof(char), L_size+1);
        memcpy(L_value, &(P_buf[L_pmatch[2].rm_so]), L_size);
        L_value[L_size]='\0' ;
      } 
    }
  }

  return (L_value);
}

char* C_RemoteControl::decode_put_uri(char *P_uri) {

  char *L_result = NULL ;
  char *L_ptr = P_uri ;
  
  L_ptr = find_directory(L_ptr,(char*)"seagull");
  if (L_ptr) {
    L_ptr = find_directory(L_ptr,(char*)"command");
    if (L_ptr) {
      L_result = decode_uri(L_ptr);
    } 
  }
  return (L_result);
}

char* C_RemoteControl::decode_uri(char *P_uri) {

  char *L_result = NULL ;
  char *L_ptr = P_uri ;
  char *L_ptr_value = NULL ;

  unsigned long L_value = 0 ;
  unsigned long L_duration = 0 ;
  char *L_end_str = NULL ;
  char *L_file ;  


  L_file = find_file(L_ptr,(char*)"rate");
  if (L_file) {
    L_ptr_value = find_value(L_file,(char*)"value");
    if (L_ptr_value) {
      L_value = strtoul_f(L_ptr_value, &L_end_str,10) ;
      if (L_end_str[0] != '\0') { // not a number
      } else {
        rate(L_value);
        L_result = resultOK() ;
      }
      FREE_TABLE(L_ptr_value);
    }
    return (L_result);
  }  
    
  L_file = find_file(L_ptr, (char*)"ramp");
  if (L_file) {
    L_ptr_value = find_value(L_file, (char*)"value");
    if (L_ptr_value) {
      L_value = strtoul_f(L_ptr_value, &L_end_str,10) ;
      if (L_end_str[0] != '\0') { // not a number
        FREE_TABLE(L_ptr_value);
      } else {
        FREE_TABLE(L_ptr_value);
        L_ptr_value = find_value(L_file, (char*)"duration");
        if (L_ptr_value) {
          L_duration = strtoul_f(L_ptr_value, &L_end_str,10) ;
          if (L_end_str[0] != '\0') { // not a number
            FREE_TABLE(L_ptr_value);
          } else {
            ramp(L_value, L_duration);
            FREE_TABLE(L_ptr_value);
            L_result = resultOK() ;
          }
        }
      }
    } // find value
    return (L_result);
  } // find_file for ramp 
  
  L_file = find_file(L_ptr,(char*)"stop");
  if (L_file) {
    L_result = resultOK() ;
    quit();
    return (L_result);
  }  

  L_file = find_file(L_ptr,(char*)"pause");
  if (L_file) {
    L_result = resultOK() ;
    pause();
    return (L_result);
  }  

  L_file = find_file(L_ptr,(char*)"burst");
  if (L_file) {
    L_result = resultOK() ;
    burst();
    return (L_result);
  } 

  return (L_result);
}


char* C_RemoteControl::decode_get_uri(char *P_uri, char **P_result_data) {
  char *L_result        = NULL  ;
  char *L_ptr           = P_uri ;
  char *L_result_ptr    = NULL  ;

  L_ptr = find_directory(L_ptr,(char*)"seagull");
  if (L_ptr) {
    L_result_ptr = find_directory(L_ptr,(char*)"counters");
    if (L_result_ptr) {
      L_ptr = L_result_ptr ;
      L_result_ptr = find_file(L_ptr,(char*)"all");
      if (L_result_ptr) {
        (*P_result_data) = m_stat->dumpCounters();
        L_result = resultOK();
      }
    } else {
      L_result_ptr = find_directory(L_ptr,(char*)"command");
      if (L_result_ptr) {
        L_result = decode_uri(L_result_ptr);
      }
    }
  }
  return (L_result);
}


char * C_RemoteControl::execute_command(char *P_cmd, char *P_uri, 
                                        char **P_result_data) {

  char *L_result = NULL ;
  
  if (P_cmd != NULL) {

    if (strcmp(P_cmd, (char*)"PUT")==0) {
      L_result = decode_put_uri (P_uri) ;
    } else if (strcmp(P_cmd, (char*)"GET")==0) {
      L_result = decode_get_uri (P_uri,P_result_data) ;
    } 

  }
  if (L_result == NULL) {
    L_result = resultKO() ;
  }
  
  return (L_result);
}

C_MessageFrame* C_RemoteControl::analyze_command(C_MessageFrame *P_msg) {

  char          *L_result = NULL ;
  char          *L_result_data = NULL ;
  C_MessageText *L_msg_send  = NULL  ;
  char          *L_uri = NULL ;
  char          *L_cmd = NULL ;
  bool           L_continue = false ;

  T_ValueData   L_cmd_data, L_uri_data ;

  L_cmd_data.m_type = E_TYPE_NUMBER ;
  L_uri_data.m_type = E_TYPE_NUMBER ;

  L_continue = P_msg->get_field_value(m_command_id,
                                      0,0,&L_cmd_data); 
  if (L_continue) {
    L_cmd = create_string(L_cmd_data) ;
    resetMemory(L_cmd_data);
    L_continue = P_msg->get_field_value(m_uri_id,
                                        0,0,&L_uri_data); 
  }

  if (L_continue) {
    L_uri = create_string(L_uri_data) ;
    resetMemory(L_uri_data);
    L_result = execute_command(L_cmd, L_uri, &L_result_data);
    FREE_TABLE(L_cmd);
    FREE_TABLE(L_uri);
  }
  if (L_result != NULL) {
    L_msg_send = m_protocol_text -> create (m_protocol_text,
                                            L_result,
                                            (L_result_data == NULL) ? 
                                            (char*)"Command Success" :
                                            L_result_data) ;
  }

  return (L_msg_send);

}




// end of file




