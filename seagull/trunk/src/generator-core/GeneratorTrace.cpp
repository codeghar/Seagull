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

#include "GeneratorTrace.hpp"
#include "Utils.hpp"
#include "TimeUtils.hpp"

static const char  gen_log_header_date   [] = "                       "     ;
static const char  gen_log_header_nodate [] = ""                            ;
             char *gen_log_header           = (char*) gen_log_header_nodate ;

const unsigned int gen_mask_table [] = {
  0x00000000,
  0x00000001,
  0x00000002,
  0x00000004,
  0x00000008,
  0x00000010,
  0x00000020,
  0x00000040,
  0xFFFFFFFF
} ;

const char gen_mask_char_table [] = {
  ' ',
  'E',
  'W',
  'M',
  'B',
  'T',
  'V',
  'U',
  'A'
} ;

T_TimeFunc     genTimeFunction     = no_time_func ;
iostream_output*  genTrace            = &iostream_error ;
unsigned int   genTraceLevel       = gen_mask_table[LOG_LEVEL_ALL] ;
T_TraceEndProc genTraceEndFunction = no_end_func ;
void*          genTraceEndArg      = NULL ;

void init_trace (unsigned int P_level, 
		 char        *P_fileName,
		 bool         P_useTime) {


  genTraceLevel = P_level ;

  if (P_useTime == true) {
    genTimeFunction = current_time_func ;
    gen_log_header = (char*)gen_log_header_date ;
  } else {
    genTimeFunction = no_time_func ;
  }

  if (P_level) {
    if (P_fileName != NULL) {
      fstream_output* L_stream ;
      NEW_VAR(L_stream, fstream_output(P_fileName));
      if (!L_stream->good()) {
	iostream_error <<  "Unable to open file [" << P_fileName << "]" ;
	exit (0) ;
      } else {
	genTrace = L_stream ;
	GEN_LOG_EVENT_FORCE("START log");
      }
    } 
  }
}


void init_trace (unsigned int  P_level, 
		 string_t&  P_fileName,
		 bool          P_useTime) {


  genTraceLevel = P_level ;
  if (P_useTime == true) {
    genTimeFunction = current_time_func ;
    gen_log_header = (char*)gen_log_header_date ;
  } else {
    genTimeFunction = no_time_func ;
  }

  if (P_level) {
    if (P_fileName != "") {
      fstream_output* L_stream ;
      // NEW_VAR(L_stream, fstream_output(P_fileName.data()));
      NEW_VAR(L_stream, fstream_output(P_fileName.c_str()));
      if (!L_stream->good()) {
	iostream_error <<  "Unable to open file [" << P_fileName << "]" ;
	exit (0) ;
      } else {
	genTrace = L_stream ;
	GEN_LOG_EVENT_FORCE("START log");
      }
    } 
  }
}



void close_trace () {
  
  GEN_LOG_EVENT_FORCE("STOP  log");
  if (genTrace != &iostream_error) {
    fstream_output *L_stream = dynamic_cast<fstream_output*>(genTrace);
    DELETE_VAR(L_stream);
    genTrace = &iostream_error;
  }

}

void no_time_func (iostream_output* P_stream) {
}

void no_end_func (void*) {
}


void current_time_func (iostream_output* P_stream) {
  char L_timeChar[TIME_STRING_LENGTH+1] ;
  struct timeval L_time ;

  GET_TIME(&L_time);
  time_tochar(L_timeChar, &L_time) ;
  *P_stream << L_timeChar ;
}

void use_trace_end (T_TraceEndProc P_proc, void* P_arg) {
  genTraceEndFunction = P_proc ;
  genTraceEndArg = P_arg ;
}
