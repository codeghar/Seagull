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

#include "generic_methods.hpp"
#include "iostream_t.hpp"
#include <regex.h>
#include "string_t.hpp"
#include "Utils.hpp"
#include <time.h>
#include <string.h>
#include <stdio.h>


char* external_find_text_value (char *P_buf, char *P_field) {

  if ((P_buf == NULL) || (P_field == NULL))
    return NULL;

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


typedef struct args_string {
  char * m_startoffset;
} T_ArgsStr, *T_pArgsStr ;

static const T_ArgsStr Args_Str_init = {
  (char *)"0"
} ;


int args_analysis (T_pValueData  P_args, T_pArgsStr P_result) {

  int             L_ret = 0 ;

  *P_result = Args_Str_init ;
  P_result->m_startoffset = external_find_text_value((char*)P_args->m_value.m_val_binary.m_value,
                                             (char*)"startoffset")  ;

  return (L_ret);
}

int add (int A[], int B[], int C[], int N) {
        int     i, carry, sum;
        int BASE = 10;
        carry = 0;
        for (i=0; i<N; i++) {
          sum = A[i] + B[i] + carry;
          if (sum>=0) {
            carry = sum / BASE;
            sum  %= BASE;
          }  else { // subtraction logic also.
            carry = sum / BASE - 1;
            sum  = BASE + sum% BASE;
          }
          C[i] = sum;
        }
        if (carry) {
        C[N] = carry;
        return N;
        }
        return N-1;
}


char * add (char *A, T_Integer32 B) {

    int sign = 1;
    if (strncmp(A, "-", 1) == 0)
        sign = -1;
    int a[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int c[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int b[20] = {B,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int i = 0, length = strlen(A);
    char ch[2];
    ch[1] = '\0';
    while (i<length) {
        ch[0] = A[length-1-i];
        a[i] = sign * atoi(ch);
        i++;
    }
    length = add(a,b,c, length);
    i = length;
    char ret[20];
    while (i>=0) {
        strcat(ret, ltoa(c[i--]));
    }
    char * L_value;
    ALLOC_TABLE(L_value, char*, sizeof(char), strlen(ret)+1);
      memcpy(L_value, ret, strlen(ret));
      L_value[strlen(ret)]='\0' ;
    return  L_value;
}

int sys_time_secs (T_pValueData  P_msgPart,
                   T_pValueData  P_args,
                   T_pValueData  P_result) {

  int             L_ret    = 0    ;



   T_ArgsStr L_args;

   int l_ret = 0;

  l_ret = args_analysis (P_args, &L_args);

  if (L_args.m_startoffset == NULL) {
      P_result->m_type = E_TYPE_NUMBER ;
      P_result->m_value.m_val_number = (T_UnsignedInteger32)time(NULL) ;
  } else { // the offset can be very long, so dirty work....
      P_result->m_type = E_TYPE_STRING ;
      T_Integer32 tm = time(NULL) ;
      char *c = add(L_args.m_startoffset, tm);
      P_result->m_type = E_TYPE_STRING ;
      P_result->m_value.m_val_binary.m_value =  (unsigned char *)c;
      P_result->m_value.m_val_binary.m_size = strlen(c);

  }


  FREE_TABLE(L_args.m_startoffset);
  return (L_ret);
}

