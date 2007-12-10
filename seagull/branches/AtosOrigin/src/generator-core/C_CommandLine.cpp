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

#include <cctype>
#include <cstring>

#include "C_CommandLine.hpp"


#define UNRECOGNIZED_OPTION -1

template <class T_TIndexCommand> C_CommandLine<T_TIndexCommand>::C_CommandLine() {
  f_programName         = NULL ;
  f_command_description = NULL ;
  f_option_flags        = NULL ;
  f_noHelpIndex         = true ;
  f_internalHelpCode    = -1   ;
}

template <class T_TIndexCommand> C_CommandLine<T_TIndexCommand>::~C_CommandLine() {
  f_command_description = NULL ;
  f_noHelpIndex         = true ;
  f_internalHelpCode    = -1   ;
  FREE_VAR (f_programName);
  FREE_VAR (f_option_flags);
}

/*
 * function : usage
 *            command syntaxe display 
 */

template <class T_TIndexCommand> void C_CommandLine<T_TIndexCommand>::usage (void) {
  
  int  i ;
  char tmpstr[255] ;

  iostream_error << f_programName << iostream_endl ;

#ifdef VERSION
  iostream_error << " Version tool   : " << (char*)VERSION << iostream_endl ;
#endif
  iostream_error << " Command syntax : " << iostream_endl ;

  for(i=0; i<(f_command_description->nb_options); i++) {
    switch (f_command_description->command_options[i].option_type) {
    case E_OT_MANDATORY:
      sprintf(tmpstr, " -%s %s %s\n",
	      f_command_description->command_options[i].name,
	      f_command_description->command_options[i].param_comment,
	      f_command_description->command_options[i].comments);
      break ;
    case E_OT_OPTIONAL:
      sprintf(tmpstr, "[ -%s %s ] %s \n",
	      f_command_description->command_options[i].name,
	      f_command_description->command_options[i].param_comment,
	      f_command_description->command_options[i].comments);      
      break ;
    default :
      iostream_error << "Unsupported option type" << iostream_endl ;
      usage () ;
      exit (-1) ;
    }
    iostream_error << tmpstr ;
  }
} /* usage */

template <class T_TIndexCommand>
int C_CommandLine<T_TIndexCommand>::integer_ok (char *name) {
  unsigned int i ;
  for(i=0; i<strlen(name); i++) {
    if (!isdigit(name[i])) { return -1; }
  }
  return 0 ;
}

template <class T_TIndexCommand>
int C_CommandLine<T_TIndexCommand>::octet_hexa_ok (char *name) {
  if (strlen(name) == 4) {
    if ((name[0] != '0') 
	|| ((name[1] != 'x') && (name[1] != 'X')) 
	|| (!isxdigit(name[2])) 
	|| (!isxdigit(name[3]))) {
      return -1 ;
    }
  } else {
    return -1 ;
  }
  return 0 ;
}

template <class T_TIndexCommand> bool C_CommandLine<T_TIndexCommand>::VerifyOptionValues (int opt, int idx, char **argv) {
  
  int nb_values_to_verify ;
  int valueIdx ;
  int currentIdx = 0 ;
  char *currentValue ;

  nb_values_to_verify = f_command_description->command_options[opt].nb_values ;
  valueIdx = idx ;

  while (nb_values_to_verify > 0) {
    currentValue = argv[valueIdx] ;
    switch (f_command_description->command_options[opt].values_types[currentIdx]) {
    case E_VT_INTEGER:
      if (integer_ok (currentValue) == -1) {
	return false;
      }
      break ;
    case E_VT_OCTET_HEXA:
      if (octet_hexa_ok (currentValue) == -1) {
	return false;
      }
      break ;
    case E_VT_STRING:
      break;
    default:
      iostream_error << "not supported argument value type" << iostream_endl ;
      usage () ;
      exit (-1);
    }
    valueIdx ++ ;
    currentIdx ++ ;
    nb_values_to_verify -- ;
  }

  return true ;
  
}

template <class T_TIndexCommand> int C_CommandLine<T_TIndexCommand>::SearchForOption (char *pattern) {
  int optionIdx ;
  for (optionIdx=0; optionIdx < f_command_description->nb_options; optionIdx++) {
    if (strcmp (pattern, f_command_description->command_options[optionIdx].name) == 0) {
      return (optionIdx) ;
    } /* if */
  } /* for */
  return (-1) ;
}

template <class T_TIndexCommand> int C_CommandLine<T_TIndexCommand>::SearchForOption (T_TIndexCommand idxCmd) {
  int optionIdx ;
  for (optionIdx=0; optionIdx < f_command_description->nb_options; optionIdx++) {
    if (idxCmd == f_command_description->command_options[optionIdx].index) {
      return (optionIdx) ;
    } /* if */
  } /* for */
  return (-1) ;
}


template <class T_TIndexCommand> bool C_CommandLine<T_TIndexCommand>::VerifyMandatoryArgs () {
  int optionIdx ;
  for (optionIdx=0; optionIdx < f_command_description->nb_options; optionIdx++) {
    if ((f_option_flags[optionIdx] == false) 
	&& f_command_description->command_options[optionIdx].option_type == E_OT_MANDATORY) {
      iostream_error << "option -" << f_command_description->command_options[optionIdx].name << " is mandatory" << iostream_endl ;
      return false ;
    }
  } /* for */
  return true ;
}

template <class T_TIndexCommand> void C_CommandLine<T_TIndexCommand>::AnalyzeCommandLine (int argc, char **argv) {
  
  char option_first_char = '-' ;   // first character for option 
                                         // description of a command line

  int   currentArgIdx = 1 ;              // current command line argument index
  char *currentArg        ;              // current command line argument string

  int   optionCode        ;              // current option code 
  int   nextCmdshift      ;              // next command option position 
  
  // init command line with program name
  ALLOC_VAR(this->f_programName, char*, strlen(argv[0])+1);
  strcpy (this->f_programName, argv[0]) ;

  while ( currentArgIdx < argc ) { 

    currentArg = argv[currentArgIdx] ; 

    if (currentArg[0] != option_first_char) { // first character option verification
      iostream_error << "unrecognized option format " << currentArg << " in command line" << iostream_endl ;
      usage () ;
      exit (-1) ;
    }

    currentArg++ ; // shift to the second character (after minus)
    optionCode = SearchForOption (currentArg) ; // search for option description

    if (optionCode == UNRECOGNIZED_OPTION) { // unrecognized option
      iostream_error << "unrecognized option -" << currentArg << " in command line" << iostream_endl ;
      usage () ;
      exit (-1) ;
    }
    
    // the option has been recognized
    f_option_flags[optionCode] = true ; 
    
    nextCmdshift = f_command_description->command_options[optionCode].nb_values ;
    // ^ the next command option will be at position currentArgIdx + nextCmdshift ;

    // test : number of values for the option
    if ((currentArgIdx + 1 + nextCmdshift) > argc) {
      iostream_error << "option -" << currentArg << " has " << nextCmdshift << " value(s)" << iostream_endl ;
      usage () ;
      exit (-1);
    }
    currentArgIdx++ ; // index at the first value of the option

    // option values
    if (VerifyOptionValues (optionCode, currentArgIdx, argv) == false) {
      iostream_error << "bad value format for option -" << currentArg << iostream_endl ;
      usage () ;
      exit (-1) ;
    };

    if (set_data (f_command_description->command_options[optionCode].index, 
		  &argv[currentArgIdx]) == false) {
      iostream_error << "bad value format for option -" 
		     << currentArg << iostream_endl ;
      usage () ;
      exit (-1);
    } 
		     //    SearchForValue (optionCode, currentArgIdx, argv);
    
    currentArgIdx += nextCmdshift ;
  } /* while */

  if (f_noHelpIndex == false ) {
    if (f_option_flags[f_internalHelpCode] == true) { // help option has been used
      //    if (f_option_flags[f_command_description->help_index] == true) { 
      usage() ;
      exit (0) ;
    }
  }

  if (VerifyMandatoryArgs() == false) { // all mandatory parameters must be present
    usage () ;
    exit (-1) ;
  }

} /* AnalyzeCommandLine */

template <class T_TIndexCommand> int C_CommandLine<T_TIndexCommand>::startAnalyzeCommandLine (int argc, char **argv, 
			  T_command_line *command_descr) {
  
  if (command_descr != NULL) {
     f_command_description = command_descr ;
  } else {
    return ( -1 ) ;
  }

  ALLOC_VAR(this->f_option_flags, bool*, f_command_description->nb_options);

  for(int i=0; i<f_command_description->nb_options; i++) {
    this->f_option_flags[i] = false ;
  }

  if (f_command_description->help_index != -1) {
    f_noHelpIndex = false ;
    f_internalHelpCode = SearchForOption (f_command_description->help_index) ;
    if (f_internalHelpCode == -1) { 
      f_noHelpIndex = true ; 
    }
  }

  AnalyzeCommandLine(argc, argv) ;

  return 0 ;
} /* startAnalyzeCommandLine */








