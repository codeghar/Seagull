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

#ifndef _UTILS_H_
#define _UTILS_H_

//-----------------------------------------------------------------------------
// Include definitions
//-----------------------------------------------------------------------------

// System Include

// New STL definition
#include <cstdio>
#include <cstdlib>

#include "iostream_t.hpp"


// Types definitions

//-----------------------------------------------------------------------------
// Constante and Macro definitions
// -----------------------------------------------------------------------------

// Compilation Flag Description
// ----------------------------
// STL_OLD_HEADERS :  Use for old STL Compatibility
//                    To define in case of new STL definition 

typedef enum _enum_cons_result {
  E_CONSTRUCTOR_OK,
  E_CONSTRUCTOR_KO
} T_ConstructorResult, *T_pContructorResult ;


// boolean general definition
#ifdef LOCAL_BOOLEAN
#ifndef boolean
typedef unsigned char boolean ;
#endif /* boolean */
#ifndef true
#define true  (boolean) ((0) == (0))
#endif /* true */
#ifndef false
#define false (boolean) ((0) == (1))
#endif /* false */
#endif /* LOCAL_BOOLEAN */

#ifndef DEBUG_NEW

//
// Memory allocation general macros

// Allocate a simple variable with malloc
#define ALLOC_VAR(v,t,s)\
	(v) = (t) malloc (s); \
	if ((v) == NULL) { \
           fprintf(stderr, "ALLOC_VAR (" #v ", " #t ", size: %lu) failure\n", (unsigned long)(s));\
	   exit(-1) ;\
	}

#define REALLOC_VAR(v,t,s)\
	(v) = (t) realloc ((void*)(v),s); \
	if ((v) == NULL) { \
           fprintf(stderr, "REALLOC_VAR (" #v ", " #t ", size: %lu) failure\n", (unsigned long)(s));\
	   exit(-1) ;\
	}

// Allocate an array variable with malloc
#define ALLOC_TABLE(v,t,s,nb)\
	(v) = (t) malloc ((s) * (nb)); \
	if ((v) == NULL) { \
           fprintf(stderr, "ALLOC_TABLE (" #v ", " #t ", size: %lu, nb: %lu) failure\n", (unsigned long)(s), (unsigned long)(nb));\
	   exit(-1) ;\
	}

// Allocate a simple variable with new
#define NEW_VAR(v,t)\
	(v) = new t ;\
	if ((v) == NULL) {\
          iostream_error << "NEW_VAR (" #v ", " #t ") failure" << iostream_endl ;\
	  exit(-1);\
	} 

// Allocate an array variable with new
#define NEW_TABLE(v,t,s)\
	(v) = new t [(s)] ;\
	if ((v) == NULL) {\
          iostream_error << "NEW_TABLE (" #v ", " #t ", nb: " << (s) << ") failure" << iostream_endl ;\
	  exit(-1);\
	} 

//
// Memory release general macros

// Free a simple variable
#define FREE_VAR(p)\
	if ((p) != NULL) {\
	   free (p) ; \
	   (p) = NULL ; \
	}

// Free an array variable
#define FREE_TABLE(p)\
	if ((p) != NULL) {\
	   free (p) ; \
	   (p) = NULL ; \
	}

// Delete a simple variable
#define DELETE_VAR(p)\
	if ((p) != NULL) {\
	   delete p ;\
	   (p) = NULL ;\
        }

// Delete an array variable
#define DELETE_TABLE(p)\
	if ((p) != NULL) {\
	   delete [] p ;\
	   (p) = NULL ;\
        }

#else

// Debug mode for allocator
#include "DebugNew.hpp"

#ifndef DEBUG_MODE
//
// Debug Trace no activated

//
// Memory allocation general macros

// Allocate a simple variable with malloc
#define ALLOC_VAR(v,t,s)\
  (v) = (t) CMemoryManager::Instance().Allocate((s), __FILE__, __LINE__, #v, false, true); \
  if ((v) == NULL) { \
    fprintf(stderr, "ALLOC_VAR (" #v ", " #t ", size: %lu) failure\n", (unsigned long)(s));\
    exit(-1) ;\
  }

//
#define REALLOC_VAR(v,t,s)\
  (v) = (t) realloc ((void*)(v),(s)); \
  if ((v) == NULL) { \
    fprintf(stderr, "REALLOC_VAR (" #v ", " #t ", size: %lu) failure\n", (unsigned long)(s));\
    exit(-1) ;\
  }

// Allocate an array variable with malloc
#define ALLOC_TABLE(v,t,s,nb)\
  (v) = (t) CMemoryManager::Instance().Allocate((s)*(nb), __FILE__, __LINE__, #v, true, true); \
  if ((v) == NULL) { \
    fprintf(stderr, "ALLOC_TABLE (" #v ", " #t ", size: %lu, nb: %lu) failure\n", (unsigned long)(s), (unsigned long)(nb));\
    exit(-1) ;\
  }

// Allocate a simple variable with new
#define NEW_VAR(v,t)\
  (v) = new(__FILE__, __LINE__, #v)  t ; \
  if ((v) == NULL) {\
    iostream_error << "NEW_VAR (" #v ", " #t ") failure" << iostream_endl ;\
    exit(-1);\
  } 

// Allocate an array variable with new
#define NEW_TABLE(v,t,s)\
  (v) = new(__FILE__, __LINE__, #v) t [(s)] () ;\
  if ((v) == NULL) {\
    iostream_error << "NEW_TABLE (" #v ", " #t ", nb: " << (s) << ") failure" << iostream_endl ;\
    exit(-1);\
  } 


//
// Memory release general macros

// Free a simple variable
#define FREE_VAR(p)\
  if ((p) != NULL) { \
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    CMemoryManager::Instance().Free(p, false, true); \
    (p) = NULL ; \
  }

// Free an array variable
#define FREE_TABLE(p)\
  if ((p) != NULL) {\
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    CMemoryManager::Instance().Free(p, true, true); \
    (p) = NULL ; \
  }

// Delete a simple variable
#define DELETE_VAR(p)\
  if ((p) != NULL) {\
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    delete p; \
    CMemoryManager::Instance().Clean(p, false, false); \
    (p) = NULL ;\
  }

// Delete an array variable
#define DELETE_TABLE(p)\
  if ((p) != NULL) {\
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    delete [] p; \
    CMemoryManager::Instance().Clean(p, true, false); \
    (p) = NULL ; \
  }

#else

//
// Debug Trace activated

//
// Memory allocation general macros

// Allocate a simple variable with malloc
#define ALLOC_VAR(v,t,s)\
  fprintf(stderr, "ALLOC_VAR (" #v ", " #t ", size: %lu) file: %s (%d)\n", (unsigned long)(s), __FILE__, __LINE__); \
  (v) = (t) CMemoryManager::Instance().Allocate((s), __FILE__, __LINE__, #v, false, true); \
   fprintf(stderr, "ALLOC_VAR (" #v ", " #t ") = 0x%08lx\n", (unsigned long)(v)); \
  if ((v) == NULL) { \
    fprintf(stderr, "ALLOC_VAR (" #v ", " #t ", size: %lu) failure\n", (unsigned long)(s));\
    exit(-1) ;\
  }

//
#define REALLOC_VAR(v,t,s)\
  fprintf(stderr, "REALLOC_VAR (" #v ", " #t ", size: %lu) file: %s (%d)\n", (unsigned long)(s), __FILE__, __LINE__); \
  (v) = (t) realloc ((void*)(v),(s)); \
  if ((v) == NULL) { \
    fprintf(stderr, "REALLOC_VAR (" #v ", " #t ", size: %lu) failure\n", (unsigned long)(s));\
    exit(-1) ;\
  }

// Allocate an array variable with malloc
#define ALLOC_TABLE(v,t,s,nb)\
  fprintf(stderr, "ALLOC_TABLE (" #v ", " #t ", size: %lu, nb: %lu) file: %s (%d)\n", (unsigned long)(s), (unsigned long)(nb), __FILE__, __LINE__); \
  (v) = (t) CMemoryManager::Instance().Allocate((s)*(nb), __FILE__, __LINE__, #v, true, true); \
  fprintf(stderr, "ALLOC_TABLE (" #v ", " #t ") = 0x%08lx\n", (unsigned long)(v)); \
  if ((v) == NULL) { \
    fprintf(stderr, "ALLOC_TABLE (" #v ", " #t ", size: %lu, nb: %lu) failure\n", (unsigned long)(s), (unsigned long)(nb));\
    exit(-1) ;\
  }

// Allocate a simple variable with new
#define NEW_VAR(v,t)\
  iostream_error << "NEW_VAR (" #v ", " #t ") " << " file: " << __FILE__ << " (" << __LINE__ << ")" << iostream_endl ; \
  (v) = new(__FILE__, __LINE__, #v)  t ; \
  iostream_error << "NEW_VAR (" #v ", " #t ") " << " = 0x" << iostream_hex << (unsigned long)(v) << iostream_dec << iostream_endl ; \
  if ((v) == NULL) {\
    iostream_error << "NEW_VAR (" #v ", " #t ") failure" << iostream_endl ;\
    exit(-1);\
  } 

// Allocate an array variable with new
#define NEW_TABLE(v,t,s)\
 iostream_error << "NEW_TABLE (" #v ", " #t ", nb: " << (s) << ")" << " file: " << __FILE__ << " (" << __LINE__ << ")" << iostream_endl ; \
  (v) = new(__FILE__, __LINE__, #v) t [(s)] () ;\
  iostream_error << "NEW_TABLE (" #v ", " #t ", nb: " << (s) << ") " << " = 0x" << iostream_hex << (unsigned long)(v) << iostream_dec << iostream_endl ; \
  if ((v) == NULL) {\
    iostream_error << "NEW_TABLE (" #v ", " #t ", nb: " << (s) << ") failure" << iostream_endl ;\
    exit(-1);\
  } 


//
// Memory release general macros

// Free a simple variable
#define FREE_VAR(p)\
  fprintf(stderr, "FREE_VAR (" #p ") = 0x%08lx file: %s (%d)\n", (unsigned long)(p), __FILE__, __LINE__); \
  if ((p) != NULL) { \
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    CMemoryManager::Instance().Free(p, false, true); \
    (p) = NULL ; \
  }

// Free an array variable
#define FREE_TABLE(p)\
  fprintf(stderr, "FREE_TABLE (" #p ") = 0x%08lx file: %s (%d)\n", (unsigned long)(p), __FILE__, __LINE__); \
  if ((p) != NULL) {\
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    CMemoryManager::Instance().Free(p, true, true); \
    (p) = NULL ; \
  }

// Delete a simple variable
#define DELETE_VAR(p)\
  iostream_error << "DELETE_VAR (" #p ") = 0x" << iostream_hex << (unsigned long)(p) << iostream_dec << " file: " << __FILE__ << " (" << __LINE__ << ")" << iostream_endl ; \
  if ((p) != NULL) {\
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    delete p; \
    CMemoryManager::Instance().Clean(p, false, false); \
    (p) = NULL ;\
  }

// Delete an array variable
#define DELETE_TABLE(p)\
  iostream_error << "DELETE_TABLE (" #p ") = 0x" << iostream_hex << (unsigned long)(p) << iostream_dec << " file: " << __FILE__ << " (" << __LINE__ << ")" << iostream_endl ; \
  if ((p) != NULL) {\
    CMemoryManager::Instance().NextDelete(__FILE__, __LINE__, #p); \
    delete [] p; \
    CMemoryManager::Instance().Clean(p, true, false); \
    (p) = NULL ; \
  }

#endif // DEBUG_MODE

#endif // DEBUG_NEW

#endif /* _UTILS_H_ */

// End of file
