%{
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
%}

letter           [a-zA-Z]
lettersymb       [a-zA-Z0-9_-]
digit            [0-9]
digitp           [0-9.]
crog             [\[]
crod             [\]]
not_crod_bslash  [^\]\\]
equal            [=]
esclam           [!]
virg             [,]
ptvirg           [;]
inf              [<]
sup              [>]
minus            [-]
inter            [?]
guil             [\"]
not_guil         [^\"]
not_guil_bslash  [^\"\\]
bslash           [\\]
slash            [\/]
space            [\t\n ]

%{

#include "GeneratorError.h"
#include "GeneratorTrace.hpp"

#include "y.tab.h"
#include "C_XmlParser.hpp"

extern C_XmlParser* XmlParserClass ;

%}

%option yylineno
%o      500
%x STRING_MODE           SPECIAL_SECTION 
%x START_COMMENT_SECTION
%x START_DATA_SECTION    STOP_DATA_SECTION
%x DATA_DEFINITION

%%

<INITIAL>{space}+ 		    {/* skip blanck*/ }
<INITIAL>{digit}+ 		    { XmlParserClass->lex_integer_update((char*)yytext);
                                      return T_INTEGER; }
<INITIAL>{inf}                      { return T_INF ; }
<INITIAL>{inf}{space}*{slash}       { return T_INF_SLASH ;  }
<INITIAL>{inf}{space}*{inter}       { return T_INF_INTER ;  }
<INITIAL>{inf}{space}*{esclam}      { BEGIN SPECIAL_SECTION ;  return T_INF_ESCLAM; }
<INITIAL>{inter}{space}*{sup}       { return T_INTER_SUP ; }
<INITIAL>{sup}                      { return T_SUP ;  }
<INITIAL>{slash}                    { return T_SLASH ; }
<INITIAL>{crod}          	    { return T_CROD ; }
<INITIAL>{crog}          	    { return T_CROG ; }
<INITIAL>{virg}          	    { return T_VIRG ; }
<INITIAL>{ptvirg}          	    { return T_PTVIRG ; }
<INITIAL>{equal} 		    { return T_EQUAL ; }
<INITIAL>{letter}{lettersymb}*      { return T_IDENT ;  }
<INITIAL>{guil}                     { XmlParserClass->lex_string_value_reset() ; 
                                      BEGIN STRING_MODE ;  }
<SPECIAL_SECTION>{crog}             { BEGIN START_DATA_SECTION; return T_CROG; }
<SPECIAL_SECTION>"--"               { BEGIN START_COMMENT_SECTION ;  return T_MIN_MIN; }
<SPECIAL_SECTION>{space}*           { /* skipp blanck */ }

<START_COMMENT_SECTION>.            { /* skipp comment */ }
<START_COMMENT_SECTION>{space}*     { /* skipp comment */ }
<START_COMMENT_SECTION>"--"{space}*{sup}    { BEGIN INITIAL ;  return T_MIN_MIN_SUP; }

<START_DATA_SECTION>{space}+        { /* skipp blanck */ }
<START_DATA_SECTION>"CDATA"         { return T_CDATA ; }
<START_DATA_SECTION>{crog}          { XmlParserClass->lex_string_value_reset() ; 
                                      BEGIN DATA_DEFINITION ; return T_CROG; }
<DATA_DEFINITION>{bslash}           { XmlParserClass->lex_string_value_update('\\') ; }
<DATA_DEFINITION>{bslash}{space}+   {  }
<DATA_DEFINITION>{crod}             { BEGIN STOP_DATA_SECTION ; 
                                      XmlParserClass->lex_string_value_update('\0'); 
                                      return T_DATA ;  }
<DATA_DEFINITION>{bslash}"n"        { XmlParserClass->lex_string_value_update('\n'); }
<DATA_DEFINITION>{bslash}"r"        { XmlParserClass->lex_string_value_update('\r'); }
<DATA_DEFINITION>{bslash}"t"        { XmlParserClass->lex_string_value_update('\t'); }
<DATA_DEFINITION>{bslash}{bslash}   { XmlParserClass->lex_string_value_update('\\') ; }
<DATA_DEFINITION>{bslash}{crod}     { XmlParserClass->lex_string_value_update(']') ; }
<DATA_DEFINITION>.                  { XmlParserClass->lex_string_value_update(*yytext); }
<DATA_DEFINITION>{space}            { XmlParserClass->lex_string_value_update(*yytext); }
<STOP_DATA_SECTION>{space}+         { /* skipp comment */ }
<STOP_DATA_SECTION>{crod}           { return T_CROD ; }
<STOP_DATA_SECTION>{sup}            { BEGIN INITIAL ; return T_SUP; }
<STRING_MODE>{not_guil_bslash}      { XmlParserClass->lex_string_value_update(*yytext); }
<STRING_MODE>{guil}                 { BEGIN INITIAL ; 
                                      XmlParserClass->lex_string_value_update('\0') ; 
                                      return T_STRING ; }
<STRING_MODE>{bslash}               { XmlParserClass->lex_string_value_update('\\') ; }
<STRING_MODE>{bslash}{bslash}       { XmlParserClass->lex_string_value_update('\\') ; }
<STRING_MODE>{bslash}{guil}         { XmlParserClass->lex_string_value_update('\"') ; }
<STRING_MODE>{bslash}"n"            { XmlParserClass->lex_string_value_update('\n'); }
<STRING_MODE>{bslash}"r"            { XmlParserClass->lex_string_value_update('\r'); }
<STRING_MODE>{bslash}"t"            { XmlParserClass->lex_string_value_update('\t'); }
.                        	    { GEN_ERROR(E_GEN_FATAL_ERROR, 
		      		      "lexical error : unknown token [" 
         			      << yytext << "]"); return T_ERROR; }

%%

int yywrap(void) {

  FILE *L_nextFile ;
  int   L_ret ;

  L_nextFile = XmlParserClass->next_file() ;
  if (L_nextFile == NULL) {
    L_ret = 1 ;
  } else {
    yyin = L_nextFile ;
    yylineno = 0 ;
    L_ret = 0 ;
  }
  
  return (L_ret);
}



