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

#include "C_TransOCTcap32.hpp"
#include "Utils.hpp"

#include "create_tcap_itu_stack.hpp"
#include "create_tcap_ansi_stack.hpp"

#include "C_ProtocolExternalFrame.hpp"

#include "iostream_t.hpp"
#include "string_t.hpp"
#include <regex.h>
// #include <ttlCommon.h>


#define LOG_ERROR(m) { \
 char L_err [100] ; \
 snprintf(L_err, 100, (m)) ; \
(*m_log_error)(L_err); \
}

#define LOG_ERROR_P1(m,P1) { \
 char L_err [100] ; \
 snprintf(L_err, 100, (m), (P1)) ; \
(*m_log_error)(L_err); \
}

#define LOG_ERROR_P2(m,P1,P2) { \
 char L_err [100] ; \
 snprintf(L_err, 100, (m), (P1),(P2)) ; \
(*m_log_error)(L_err); \
}


#define LOG_ALL(m) { \
 char L_msg [100] ; \
 snprintf(L_msg, 100, (m)) ; \
(*m_log_info)(L_msg); \
}

#define LOG_ALL_P1(m,P1) { \
 char L_msg [100] ; \
 snprintf(L_msg, 100, (m), (P1)) ; \
(*m_log_info)(L_msg); \
}

#define LOG_ALL_P2(m,P1,P2) { \
 char L_msg [100] ; \
 snprintf(L_msg, 100, (m), (P1),(P2)) ; \
(*m_log_info)(L_msg); \
}



C_TransOCTcap32::C_TransOCTcap32() {
  m_cnx_id = -1 ; 
  m_stack = NULL ;

  // m_cnx_info_map.clear () ;

  m_library_name = NULL ;
  m_flavour = E_FLAVOUR_UNKNOWN ;
}

C_TransOCTcap32::~C_TransOCTcap32() {


  // T_CnxInfoList::iterator L_it ;
  // T_pCnxInfo              L_cnx_info ;

  // T_CnxInfoMap::iterator  L_it_map ; 

  m_cnx_id = -1 ; 
  DELETE_VAR(m_stack);
  FREE_TABLE(m_library_name);
  m_flavour = E_FLAVOUR_UNKNOWN ;


//    if (! m_cnx_info_map.empty()) {
//      for(L_it_map = m_cnx_info_map.begin();
//  	L_it_map != m_cnx_info_map.end();
//  	L_it_map++) {
//        L_cnx_info = L_it_map->second ;
//        delete_CnxInfo(&L_cnx_info);
//      }
//      m_cnx_info_map.erase(m_cnx_info_map.begin(), 
//  			 m_cnx_info_map.end());
//    }

}


int C_TransOCTcap32::config (T_pConfigValueList P_config_param_list) {
  return(m_stack->config(P_config_param_list));
}


int C_TransOCTcap32::init (char* P_buf,
			   T_logFunction P_logError, 
			   T_logFunction P_logInfo) {

  int L_ret = 0 ;

  //  TTL_M_INIT ((char*)"SEAGULL");

  m_log_error = P_logError ;
  m_log_info = P_logInfo ;
  L_ret = analyze_init_string(P_buf) ;
  if (L_ret == 0) {
    L_ret = m_stack->init(m_log_error, m_log_info) ;
  }
  return (L_ret);
}

int C_TransOCTcap32::close () {

  // map access

//    T_pCnxInfo              L_cnx_info ;
//    int                     L_ret  = 0 ;
//    T_CnxInfoMap::iterator  L_it_map   ; 

//    if (! m_cnx_info_map.empty()) {
//      for(L_it_map = m_cnx_info_map.begin();
//  	L_it_map != m_cnx_info_map.end();
//  	L_it_map++) {
//        L_cnx_info = L_it_map->second ;
//        L_ret = m_stack->close(L_cnx_info->m_cnx_id) ;
//        if (L_ret == -1) {
//  	break ;
//        }
//      }
//    }

  // return (L_ret);


  return (m_stack->close(m_cnx_id));

}

int C_TransOCTcap32::open (int              P_channel_id,
			   char*            P_buf, 
			   T_pOpenStatus    P_status,
			   C_ProtocolFrame *P_protocol) {

  int L_ret = 0 ;
  C_ProtocolExternalFrame* L_protocol = NULL ; 
  
  T_pCnxInfo L_cnxInfo = NULL ;

  if (L_ret != -1) {
    L_protocol 
      = dynamic_cast<C_ProtocolExternalFrame*> (P_protocol);
    if (L_protocol == NULL) {
      LOG_ERROR("protocol type not compatible (external expected)");
      L_ret = -1 ;
    }
  }
  
  if (L_ret != -1) {
    L_cnxInfo = create_CnxInfo() ;
    L_ret = (analyze_open_string(P_buf, L_cnxInfo) == true) ? 
      m_stack->open(P_channel_id, L_cnxInfo->m_class_value,
		    L_cnxInfo->m_inst_id,L_cnxInfo->m_app_id,L_cnxInfo->m_ossn,
		    L_protocol) : -1 ;

    if (L_ret != -1) {    
      m_cnx_id = L_ret ;

      LOG_ALL_P2("Stack name [%s] and ossn [%d]", 
	      L_cnxInfo->m_class_value, 
	      L_cnxInfo->m_ossn);

      LOG_ALL_P2("instance id [%d] and application id [%d]", 
	      L_cnxInfo->m_inst_id, 
	      L_cnxInfo->m_app_id);

      LOG_ALL_P1("Stack opened with connection id [%d]" , m_cnx_id);

      L_cnxInfo->m_cnx_id = L_ret ;
      // m_cnx_info_map.insert(T_CnxInfoMap::value_type(L_ret, L_cnxInfo));
      delete_CnxInfo(&L_cnxInfo);
      (*P_status) =  E_OPEN_OK ;
    } else {
      delete_CnxInfo(&L_cnxInfo);
      (*P_status) =  E_OPEN_FAILED ;
    }
  }

  return (L_ret);
}

int C_TransOCTcap32::close (int P_id) {

  // map access

//    T_pCnxInfo              L_cnx_info ;
//    int                     L_ret  = 0 ;
//    T_CnxInfoMap::iterator  L_it_map   ; 


//    L_it_map = m_cnx_info_map.find(T_CnxInfoMap::key_type(P_id));
//    if (L_it_map == m_cnx_info_map.end()) {
//      LOG_ERROR_P1("Unable to find cnxId [%d]", P_id);
//      L_ret = -1 ;
//    }

//    if (L_ret != -1) {
//      L_cnx_info = L_it_map->second ;
//      L_ret = m_stack->close(L_cnx_info->m_cnx_id) ;
//    }


  // return (L_ret);


  return (m_stack->close(P_id));
}

int C_TransOCTcap32::pre_select (int                   P_n, 
				 fd_set               *P_readfds,  
				 fd_set               *P_writefds,
				 fd_set               *P_exceptfds, 
				 struct timeval       *P_timeout, 
				 int                  *P_cnx, 
				 size_t                P_nb) {

  // map access

  return (m_stack->pre_select (P_n, P_readfds,
			       P_writefds, P_exceptfds, P_timeout)) ;
}


int C_TransOCTcap32::post_select (int                  P_n, 
				  fd_set              *P_readfds,  
				  fd_set              *P_writefds,
				  fd_set              *P_exceptfds, 
				  T_pC_TransportEvent  P_eventTable,
				  size_t              *P_nb) {

  // map access
  
  return (m_stack->post_select (P_n, P_readfds, P_writefds,
				P_exceptfds, P_eventTable, P_nb));
}

bool C_TransOCTcap32::get_message(int P_id, T_pReceiveMsgContext P_ctxt) {
  return (m_stack->get_message(P_id, P_ctxt));
}

int C_TransOCTcap32::send_message(int P_id, C_MessageFrame *P_msg) {
  return (m_stack->send_message(P_id, P_msg));
}

T_SelectDef C_TransOCTcap32::select_definition () {
  return (NULL);
}

int C_TransOCTcap32::analyze_init_string (char *P_buf) {

  int        L_ret = 0 ;
  char      *L_value ;
  char      *L_path_value = NULL ;
  char      *L_lib_name_value = NULL ;

  L_value = find_value(P_buf, (char*)"flavour");
  if (L_value == NULL) {
    LOG_ERROR("Flavour not defined in your config file");
    L_ret = -1 ;
  } else {
    m_flavour = analyze_flavour (L_value);
    if (m_flavour != C_TransOCTcap32::E_FLAVOUR_UNKNOWN) {
      L_path_value = find_value(P_buf, (char*)"path");
      L_lib_name_value = find_value(P_buf, (char*)"library");
      L_ret = find_library (L_value, L_path_value, L_lib_name_value);

      if (L_ret == 0) {
	if (m_flavour == C_TransOCTcap32::E_FLAVOUR_ITU) {
	  m_stack = create_tcap_itu_stack(m_library_name);
	  //	  NEW_VAR(m_stack, C_TcapStackITU(m_library_name));
	} else {
	  m_stack = create_tcap_ansi_stack(m_library_name);
	  //	  NEW_VAR(m_stack, C_TcapStackANSI(m_library_name));
	}
      }

      FREE_TABLE(L_path_value);
      FREE_TABLE(L_lib_name_value);

    } else {
      LOG_ERROR("Flavour unknown");
      L_ret = -1 ;
    }
    FREE_TABLE(L_value);
  }

  return (L_ret);
}


bool C_TransOCTcap32::analyze_open_string (char *P_buf, 
					   T_pCnxInfo P_cnx_info) {

  char *L_ossn_value = NULL ;
  char *L_appl_value = NULL ;
  char *L_inst_value = NULL ;
  char *L_end_ptr           ;
  bool  L_ret  = true       ;


  P_cnx_info->m_class_value = find_value(P_buf, (char *)"class");
  if (P_cnx_info->m_class_value == NULL) {
    LOG_ERROR("No class defined");
    L_ret = false ;
  } 

  L_ossn_value = find_value(P_buf, (char *)"ossn");
  if (L_ossn_value == NULL) {
    LOG_ERROR("No ossn defined");
    L_ret = false ;
  } else {
    L_end_ptr = NULL ;
    P_cnx_info->m_ossn = strtol(L_ossn_value, &L_end_ptr, 10);
    if (L_end_ptr[0] != '\0') {
      LOG_ERROR("ossn bad format");

      L_ret = false ;
    }
  }

  L_appl_value = find_value(P_buf, (char *)"application");
  if (L_appl_value == NULL) {
    LOG_ERROR("No application id defined");
    L_ret = false ;
  } else {
    L_end_ptr = NULL ;
    P_cnx_info->m_app_id = strtol(L_appl_value, &L_end_ptr, 10);
    if (L_end_ptr[0] != '\0') {
      LOG_ERROR("application id bad format");
      L_ret = false ;
    }
  }

  L_inst_value = find_value(P_buf, (char *)"instance");
  if (L_inst_value == NULL) {
    LOG_ERROR("No instance id defined");
    L_ret = false ;
  } else {
    L_end_ptr = NULL ;
    P_cnx_info->m_inst_id = strtol(L_inst_value, &L_end_ptr, 10);
    if (L_end_ptr[0] != '\0') {
      LOG_ERROR("instance id bad format");
      L_ret = false ;
    }
  }
  return (L_ret) ;
}

char* C_TransOCTcap32::find_value (char *P_buf, char *P_field) {

  char *L_value = NULL ;

  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[3] ;
  size_t     L_size = 0 ;

  string_t   L_string = "" ;
  
  L_string  = "([[:blank:]]*" ;
  L_string += P_field ;
  L_string += "[[:blank:]]*=[[:blank:]]*)([^[:blank:];]+)";

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


  return (L_value);
}

C_TransOCTcap32::T_FlavourKey C_TransOCTcap32::analyze_flavour(char *P_name) {

  static const char* L_flavour_name_table[] = {"ITU", "WBB", "WAA",
					       "ANSI", "AAA", "ABB"};
  static const int   L_nb_supported = 6 ;
  C_TransOCTcap32::T_FlavourKey L_ret = C_TransOCTcap32::E_FLAVOUR_UNKNOWN ;

  int L_flavour = 0 ;
  for (L_flavour = 0; L_flavour < L_nb_supported; L_flavour++) {
    if (strcmp(P_name, L_flavour_name_table[L_flavour]) == 0) {
      L_ret = (L_flavour < 3) ? C_TransOCTcap32::E_FLAVOUR_ITU : C_TransOCTcap32::E_FLAVOUR_ANSI ;
      break ;
    }
  }
  return (L_ret);
}

int C_TransOCTcap32::find_library(char *P_name, char *P_path, char *P_lib) {
  
  int L_ret = 0 ;

  if ((P_path != NULL) && (P_lib != NULL)) {
    m_library_name = verify_path(P_path, P_lib) ;
    L_ret = (m_library_name != NULL) ? 0 : -1 ;
  } else {
    L_ret = determine_lib(P_name);
    if (L_ret == -1) {
      LOG_ERROR("library not found");
    }
  }
  return (L_ret);
}

char *C_TransOCTcap32::verify_path(char * P_libPath, char * P_libName) {

  char             *L_Path = NULL ;
  FILE             *L_fp ;

  ALLOC_TABLE(L_Path, char*, sizeof(char), strlen(P_libName)+strlen(P_libPath)+2);
  sprintf(L_Path,"%s/%s",P_libPath,P_libName);

  if((L_fp = (fopen(L_Path,"rb"))) == NULL) {
    FREE_TABLE(L_Path);
  } else {
    fclose(L_fp);
  }
  return (L_Path);
}

int C_TransOCTcap32::determine_lib(char *P_name) {

  char              L_LibName_Aux[20];
  char              L_LibName_sl[30];
  char              L_LibName_so[30];
  int               L_result = -1;
  char             *L_FlavourName = P_name ;
  int               L_i = 0 ;

  static const char* L_Lib_Path[] = {"/opt/OC/lib/hpux64", 
				     "/opt/OC/lib/hpux32",
				     "/opt/OC/lib/"};
  static const int   L_nb_path  = 3 ;

  L_LibName_Aux[0] = '\0';

  strcpy(L_LibName_Aux,"libSS7util");
  sprintf(L_LibName_Aux,"%s%s",L_LibName_Aux, L_FlavourName);

  strcpy(L_LibName_sl,L_LibName_Aux);
  sprintf(L_LibName_sl,"%s%s",L_LibName_sl, ".sl");

  strcpy(L_LibName_so,L_LibName_Aux);
  sprintf(L_LibName_so,"%s%s",L_LibName_so, ".so");

  for (L_i = 0; L_i < L_nb_path; L_i++) {
    // build LibName with .so
    m_library_name = verify_path((char *)L_Lib_Path[L_i],L_LibName_so) ;
    if (m_library_name != NULL) {
      L_result = 0 ;
      break ;
    } else {
      // build LibName with .sl
      m_library_name = verify_path((char *)L_Lib_Path[L_i],L_LibName_sl) ;
      if(m_library_name != NULL) { 
	L_result = 0 ;
	break ;
      } 
    }
  }
  return (L_result);
}

T_pTransport create_ctransoctcap_instance () {
  C_TransOCTcap32 *L_inst ;
  NEW_VAR(L_inst, C_TransOCTcap32());
  return (L_inst);
}

void delete_ctransoctcap_instance (T_ppTransport P_inst) {
  if (P_inst != NULL) {
    C_TransOCTcap32* L_inst = dynamic_cast<C_TransOCTcap32*>(*P_inst) ;
    DELETE_VAR(L_inst);
    *P_inst = NULL ;
  }
}




// end of file
