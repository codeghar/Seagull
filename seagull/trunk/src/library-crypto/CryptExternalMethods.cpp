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

#include "CryptExternalMethods.hpp"
#include "Utils.hpp"
#include "string_t.hpp"
#include <regex.h>

#define GEN_ERROR(l,a) iostream_error << a << iostream_endl << iostream_flush ; 



extern char *stristr (const char *s1, const char *s2) ;
extern int createAuthHeaderMD5(char * user, char * password, char * method,
                               char * uri, char * msgbody, char * auth, 
                               char * algo, char * result);
extern int createAuthHeaderAKAv1MD5(char * user, char * OP,
                                    char * AMF,
                                    char * K,
                                    char * method,
                                    char * uri, char * msgbody, char * auth, char *algo,
                                    char * result);


char* external_find_text_value (char *P_buf, char *P_field) {

  char *L_value = NULL ;

  regex_t    L_reg_expr ;
  int        L_status ;
  char       L_buffer[100];
  regmatch_t L_pmatch[3] ;
  size_t     L_size = 0 ;

  string_t   L_string = "" ;
  
  L_string  = "([[:blank:]]*" ;
  L_string += P_field ;
  L_string += "[[:blank:]]*=[[:blank:]]*)([^;]+)";

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

typedef struct _crypto_args_string {
  char * m_user; 
  char * m_password; 
  char * m_method;
  char * m_uri; 
  char * m_auth; 
  int    m_algo_id;
  char * m_algo ;
  char * m_aka_k ;
  char * m_aka_op ;
  char * m_aka_amf ;
  char * m_shared_secret ;
} T_CryptoArgsStr, *T_pCryptoArgsStr ;


static const T_CryptoArgsStr Crypto_Args_Str_init = {
  NULL,
  NULL , 
  NULL,
  NULL,
  NULL, 
  -1,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL 
} ;


int check_algorithm(char * auth) {
  
  char algo[32]="MD5";
  char *start, *end;
  
  if ((start = stristr(auth, "Digest")) == NULL) {
    return (-1);
  }
  
  if ((start = stristr(auth, "algorithm=")) != NULL) {
    start = start + strlen("algorithm=");
    if (*start == '"') { start++; }
    end = start + strcspn(start, " ,\"\r\n");
    strncpy(algo, start, end - start);
    algo[end - start] ='\0';
  }
  
  if (strncasecmp(algo, "MD5", 3)==0) {
    return (0);
  } else if (strncasecmp(algo, "AKAv1-MD5", 9)==0) {
    return (1);
  } else {
    return (-1) ;
  }
}

int crypto_args_analysis (T_pValueData  P_args, T_pCryptoArgsStr P_result) {

  int             L_ret = 0 ;

  *P_result = Crypto_Args_Str_init ;
  P_result->m_user = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                             (char*)"username")  ;
  if (P_result->m_user == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "user no defined in format of the action: set-value format=\"username=.. ");
    L_ret = -1;
    return (L_ret);
  }
  
  P_result->m_method = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                                (char*)"method")  ;
  if (P_result->m_method == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "method no defined in format of the action: set-value format=\"method=.. ");
    L_ret = -1;
    return (L_ret);
  }
  
  P_result->m_uri = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                        (char*)"uri")  ;
  if (P_result->m_uri == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "uri no defined in format of the action: set-value format=\"uri=.. ");
    L_ret = -1;
    return (L_ret);
  }

  P_result->m_auth = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                              (char*)"auth")  ;
  if (P_result->m_auth == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "auth no defined in format of the action: set-value format=\"auth=.. ");
    L_ret = -1;
    return (L_ret);
  }

  P_result->m_algo_id = check_algorithm(P_result->m_auth);
  if (P_result->m_algo_id == -1 ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
              "algorithm not defined (MD5 or AKA)");
    L_ret = -1;
    return (L_ret);
  }

  // MD5 only
  if (P_result->m_algo_id == 0) { // MD5 

    ALLOC_TABLE(P_result->m_algo, char*, sizeof(char), 4);
    strcpy(P_result->m_algo, (char*)"MD5");

    P_result->m_password = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                                    (char*)"password")  ;
    if (P_result->m_password == NULL ) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
                "password no defined in format of the action: set-value format=\"password=...");
      L_ret = -1;
      return (L_ret);
    }

    
  } else {

    ALLOC_TABLE(P_result->m_algo, char*, sizeof(char), 10);
    strcpy(P_result->m_algo, (char*)"AKAv1-MD5");

    // AKA only
    P_result->m_aka_op = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                                  (char*)"aka_op")  ;
    if (P_result->m_aka_op == NULL ) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
                "aka_op no defined in format of the action: set-value format=\"aka_op=...");
      L_ret = -1;
      return (L_ret);
    }
    
    P_result->m_aka_amf = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                                   (char*)"aka_amf")  ;
    if (P_result->m_aka_amf == NULL ) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
                "aka_amf no defined in format of the action: set-value format=\"aka_amf=...");
      L_ret = -1;
      return (L_ret);
    }
    
    P_result->m_aka_k = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                        (char*)"aka_k")  ;
    if (P_result->m_aka_k == NULL ) {
      GEN_ERROR(E_GEN_FATAL_ERROR,
                "aka_k no defined in format of the action: set-value format=\"aka_k=...");
      L_ret = -1;
      return (L_ret);
    }

  }
  return (L_ret);
}


int crypto_method (T_pValueData  P_msgPart,
                   T_pValueData  P_args,
                   T_pValueData  P_result) {
  
  int             L_ret    = 0    ;
  T_CryptoArgsStr L_crypto ;
  char            L_result [2049] ;

  L_ret = crypto_args_analysis(P_args, &L_crypto);
  if (L_ret != -1) {
    if (L_crypto.m_algo_id == 0) {
      L_ret = createAuthHeaderMD5(L_crypto.m_user,
                                  L_crypto.m_password,
                                  L_crypto.m_method,
                                  L_crypto.m_uri,
                                  (char*)P_msgPart->m_value.m_val_binary.m_value,
                                  L_crypto.m_auth,
                                  L_crypto.m_algo,
                                  L_result);
    } else {
      L_ret = createAuthHeaderAKAv1MD5(L_crypto.m_user, 
                                       L_crypto.m_aka_op,
                                       L_crypto.m_aka_amf,
                                       L_crypto.m_aka_k,
                                       L_crypto.m_method,
                                       L_crypto.m_uri,
                                       (char*)P_msgPart->m_value.m_val_binary.m_value,
                                       L_crypto.m_auth,
                                       L_crypto.m_algo,
                                       L_result);
    }
    if (L_ret == 1) {
      P_result->m_type = E_TYPE_STRING ;
      ALLOC_TABLE(P_result->m_value.m_val_binary.m_value,
                  unsigned char*,
                  sizeof(unsigned char),
                  strlen(L_result));      
      P_result->m_value.m_val_binary.m_size = strlen(L_result);
      memcpy(P_result->m_value.m_val_binary.m_value, L_result, strlen(L_result));
    } else {
      L_ret = -1 ;
    }
  }

  FREE_TABLE(L_crypto.m_user); 
  FREE_TABLE(L_crypto.m_password); 
  FREE_TABLE(L_crypto.m_method);
  FREE_TABLE(L_crypto.m_uri); 
  FREE_TABLE(L_crypto.m_auth); 
  FREE_TABLE(L_crypto.m_algo );
  FREE_TABLE(L_crypto.m_aka_k );
  FREE_TABLE(L_crypto.m_aka_op );
  FREE_TABLE(L_crypto.m_aka_amf );
  FREE_TABLE(L_crypto.m_shared_secret );

  return (L_ret);
}

/** Analyze arguments for radius protocol 
  * \param P_args uses to determine the shared secret 
  * \param P_result contains the shared secret
  * \return 0 if OK
  */
int crypto_args_analysis_radius (T_pValueData  P_args, T_pCryptoArgsStr P_result) {

  int             L_ret = 0 ;

  *P_result = Crypto_Args_Str_init ;
  if (P_args->m_value.m_val_binary.m_size > 0) {
    P_result->m_shared_secret = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                                         (char*)"shared_secret")  ;
  }
  return (L_ret);
}


/** Authentication algorithm for radius protocol 
  * \param P_msgPart uses to calculate the key   
  * \param P_args contains the shared secret
  * \param P_result contains the result of this algorithm
  * \return 0 if OK
  */
int create_algo_MD5_radius(char          *  P_msg,
                           int              P_msg_size,
                           char          *  P_shared_secret,
                           unsigned char *  P_result) {
  int        L_ret         = 0 ;
  int        L_size_shared = 0 ;
  
  MD5_CTX    L_Md5Ctx ;

  if (P_shared_secret != NULL) {
    L_size_shared = strlen(P_shared_secret);
  }

  MD5_Init(&L_Md5Ctx);
  if (L_size_shared > 0) {
    MD5_Update(&L_Md5Ctx, P_shared_secret, L_size_shared);
  }
  MD5_Update(&L_Md5Ctx, P_msg, P_msg_size);
  MD5_Final(P_result, &L_Md5Ctx);

  return (L_ret);
}

/** Authentication method for radius protocol 
  * \param P_msgPart uses to calculate the key   
  * \param P_args contains the shared secret
  * \param P_result contains the result of this method
  * \return 0 if OK
  */
int crypto_method_radius (T_pValueData  P_msgPart,
                          T_pValueData  P_args,
                          T_pValueData  P_result) {
  
  int             L_ret    = 0    ;
  T_CryptoArgsStr L_crypto        ;
  unsigned char   L_result [16]   ;


  L_ret = crypto_args_analysis_radius(P_args, &L_crypto);
  if (L_ret != -1) {
    L_ret =  create_algo_MD5_radius((char*)P_msgPart->m_value.m_val_binary.m_value,
                                    P_msgPart->m_value.m_val_binary.m_size,
                                    L_crypto.m_shared_secret,
                                    L_result);
    if (L_ret != -1) {
      P_result->m_type = E_TYPE_STRING ;
      ALLOC_TABLE(P_result->m_value.m_val_binary.m_value,
                  unsigned char*,
                  sizeof(unsigned char),
                  16);      
      P_result->m_value.m_val_binary.m_size = 16;
      memcpy(P_result->m_value.m_val_binary.m_value, L_result, 16);
    } 
  }

  FREE_TABLE(L_crypto.m_shared_secret );
  return (L_ret);
}
