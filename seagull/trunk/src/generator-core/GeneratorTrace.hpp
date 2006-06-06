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

#ifndef _GENERATORTRACE_H
#define _GENERATORTRACE_H

#include "iostream_t.hpp"
#include "fstream_t.hpp"
#include <cstring>
#include "string_t.hpp"
#include <cstdlib> // for exit definition

#ifdef DEBUG_MODE
#define GEN_DEBUG(l,m)  iostream_error << m << iostream_flush << iostream_endl
#else
#define GEN_DEBUG(l,m)
#endif

typedef enum _log_level {
 LOG_LEVEL_NO,
 LOG_LEVEL_ERR,
 LOG_LEVEL_WAR,
 LOG_LEVEL_MSG,
 LOG_LEVEL_BUF,
 LOG_LEVEL_TRAFFIC_ERR,
 LOG_LEVEL_ALL
} T_LogLevel ;

extern char *gen_log_header;

extern const unsigned int gen_mask_table [] ;
extern const char         gen_mask_char_table [] ;

extern  void init_trace (unsigned int P_level, 
			 char        *P_fileName,
			 bool         P_useTime);

extern  void init_trace (unsigned int  P_level, 
			 string_t&  P_fileName,
			 bool          P_useTime) ;


extern  void close_trace       () ;
extern  void no_time_func      (iostream_output*) ;
extern  void current_time_func (iostream_output*) ;
extern  void no_end_func       (void*);

typedef void (*T_TimeFunc) (iostream_output*) ;
typedef void (*T_TraceEndProc) (void *) ;

extern  void use_trace_end (T_TraceEndProc P_proc, void* P_arg) ;

extern  iostream_output *genTrace ;
extern  unsigned int     genTraceLevel ;
extern  T_TimeFunc       genTimeFunction ;
extern  T_TraceEndProc   genTraceEndFunction ;
extern  void            *genTraceEndArg ;

// l: level, m: message (char*), c: exit code

#define GEN_HEADER_LOG      gen_log_header
#define GEN_HEADER_LEVEL(l) "|" << gen_mask_char_table[l] << "|"
#define GEN_HEADER_NO_LEVEL "| |"

#define GEN_FATAL(c,m)  \
             (*genTraceEndFunction)(genTraceEndArg); \
             *genTrace << "Fatal: " m << iostream_endl << iostream_flush ; \
             if (genTrace != &iostream_error) { \
               iostream_error << "Fatal: " m << iostream_endl << iostream_flush ; \
             } \
             exit ((c))

#define GEN_WARNING(m)  if (genTraceLevel & gen_mask_table[LOG_LEVEL_WAR]) {\
             (*genTimeFunction)(genTrace) ; \
             *genTrace << "|" << gen_mask_char_table[LOG_LEVEL_WAR] \
                       << "|" << m << iostream_endl << iostream_flush  ; }

#define GEN_ERROR(l,m)  if (genTraceLevel & gen_mask_table[LOG_LEVEL_ERR]) {\
             (*genTimeFunction)(genTrace) ; \
             *genTrace << "|" << gen_mask_char_table[LOG_LEVEL_ERR] \
                       <<"|" << m << iostream_endl << iostream_flush  ; }

#define GEN_LOG_EVENT(l,m)  if (gen_mask_table[l] & genTraceLevel) {\
             (*genTimeFunction)(genTrace) ; \
             *genTrace << "|" << gen_mask_char_table[l] \
                       <<"|" << m << iostream_endl << iostream_flush  ; }
#define GEN_LOG_EVENT_NO_DATE(l,m)  if (gen_mask_table[l] & genTraceLevel) {\
             *genTrace << gen_log_header ; \
             *genTrace << "|" << gen_mask_char_table[l] \
                       <<"|" << m << iostream_endl << iostream_flush  ; }

#define GEN_LOG_EVENT_CONDITIONAL(l,c,m)  if (gen_mask_table[l] & genTraceLevel) {\
             if (c) {\
               (*genTimeFunction)(genTrace) ; \
               *genTrace << "|" << gen_mask_char_table[l] \
                         << "|" << m << iostream_endl << iostream_flush  ; } \
             }


#define GEN_LOG_EVENT_FORCE(m)  \
             (*genTimeFunction)(genTrace) ; \
             *genTrace << "|" << gen_mask_char_table[LOG_LEVEL_ALL] \
                       << "|" << m << iostream_endl << iostream_flush 

#endif // _GENERATORTRACE_H
