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

#ifndef _C_COMMANDLINE_H_
#define _C_COMMANDLINE_H_

#include "Utils.hpp"

template <class T_TIndexCommand> 
class C_CommandLine {

public:
  // public types used for command line description
  enum T_option_type { // option type
    E_OT_MANDATORY,    // option is mandatory
    E_OT_OPTIONAL } ;  // or a default value exists 
  
  enum T_value_type { // option value type
    E_VT_INTEGER,     // an integer is expected for a value of the option
    E_VT_OCTET_HEXA,  // an octet in hexa
    E_VT_STRING } ;   // everything
  
  typedef struct _command_option {      // option description
    T_TIndexCommand    index         ;
    char              *name          ; // name of the option
    T_option_type      option_type   ; // type of the option
    int                nb_values     ; // number of values expected for the option
    T_value_type      *values_types  ; // types of the value(s) of the option
    char              *param_comment ; // parameters description
    char              *comments      ; // general comments
  } T_command_option ;
  
  typedef struct _command_line {              // command line description
    int                    nb_options       ; // number of options 
    T_TIndexCommand        help_index       ; // index of the help option
    T_command_option      *command_options  ; // description of all the options
  } T_command_line ;

public:
  // public methods
  int  startAnalyzeCommandLine (int argc, 
		                char **argv, 
		                T_command_line *) ; // starts analyzing command line
  void usage (void) ;                         // write command syntax to stderr and exit program

  C_CommandLine () ;
  virtual ~C_CommandLine () ;

  virtual bool set_data (T_TIndexCommand, char**) = 0 ;


private:
  // private fields
  char                 *f_programName         ; // executing process name (first argument)
  T_command_line       *f_command_description ; // command line description
  bool              *f_option_flags        ; // table of the detected options
  bool               f_noHelpIndex         ; // no help option defined
  int                   f_internalHelpCode    ;

  // private methods
  void    AnalyzeCommandLine  (int argc, char **argv);         // starts analyzing command line
  int     SearchForOption     (char *pattern) ;                // options pattern matching
  bool VerifyMandatoryArgs () ;                             // command line arguments verification
  bool VerifyOptionValues  (int opt, int idx, char **argv); // option values verification
  int     SearchForOption     (T_TIndexCommand idxCmd) ;       // options code matching

  int     integer_ok          (char *name);                    // option format integer verif
  int     octet_hexa_ok       (char *name);                    // option format hexa verif



} ; /* C_CommandLine */

#endif // _C_COMMANDLINE_H_











