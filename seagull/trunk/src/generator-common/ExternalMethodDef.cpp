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

#include "ExternalMethodDef.hpp"
#include "TextUtils.hpp"
#include "GeneratorError.h"
#include "GeneratorTrace.hpp"
#include "Utils.hpp"
#include "dlfcn_t.hpp"

T_ExternalMethod create_external_method (char *P_args) {
  
  T_ExternalMethod            L_ret          = NULL          ;
  char                       *L_lib_name     = NULL          ;
  char                       *L_fun_name     = NULL          ;
  void                       *L_library_handle               ; 
  void                       *L_function                     ;

  L_lib_name = find_text_value(P_args,(char*)"lib")  ;
  if (L_lib_name == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "no name for the library for the filter (lib=...)");
  } else {
    L_library_handle = dlopen(L_lib_name, RTLD_LAZY);
    FREE_TABLE(L_lib_name);
    if (L_library_handle == NULL) {
      GEN_ERROR(E_GEN_FATAL_ERROR, 
                "Unable to open library file [" 
                << L_lib_name
                << "] error [" << dlerror() << "]");
    }
  }

  L_fun_name = find_text_value(P_args,(char*)"function") ;
  if (L_fun_name == NULL ) {
    GEN_ERROR(E_GEN_FATAL_ERROR,
	      "no name for the function for the parser (function=...)");
  } else {
    if (L_library_handle) { 
      L_function = dlsym(L_library_handle, L_fun_name);
      FREE_TABLE(L_fun_name);
      if (L_function == NULL) {
        GEN_ERROR(E_GEN_FATAL_ERROR, "Error [" << dlerror() << "]");
      } else {
        L_ret = (T_ExternalMethod) L_function ;
      }
    }
  }

  return (L_ret) ;
}
