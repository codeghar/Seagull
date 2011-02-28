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

#include "GeneratorTrace.hpp"
#include "C_XmlParser.hpp"
#include "Utils.hpp"



#define YYMAXDEPTH 10000

#define YYINITDEPTH 20000

  C_XmlParser      *XmlParserClass ;

  // Generic parser data
  extern char  yytext[] ;
  extern FILE *yyin    ;
  extern int  yylineno ;

#if defined(__linux__) || defined(__CYGWIN__) || defined (__sun) || defined (__sun__)
   extern int  yyparse ();
   extern int  yylex () ;
   extern void yyerror (const char*) ;
#endif
%}


%token T_INTEGER
%token T_CROD
%token T_CROG
%token T_VIRG
%token T_PTVIRG
%token T_EQUAL
%token T_ERROR
%token T_INF
%token T_IDENT
%token T_SUP
%token T_SLASH
%token T_STRING
%token T_INF_SLASH
%token T_INF_INTER
%token T_INF_ESCLAM
%token T_CDATA
%token T_DATA
%token T_INTER_SUP
%token T_MIN_MIN
%token T_MIN_MIN_SUP

%start xml_definition

%%
  
xml_definition: all_xml_sections
  | T_ERROR { GEN_DEBUG(1, "yacc: no rule"); return (-1); } ;

all_xml_sections:
  | xml_sections
    all_xml_sections ;

xml_sections:
    xml_meta_section { XmlParserClass->add_sub_xml_data(); GEN_DEBUG(1, "yacc: rule xml_meta_section"); }
  | xml_special_section { GEN_DEBUG(1, "yacc: rule xml_special_section"); }
  | xml_section { XmlParserClass->add_sub_xml_data(); GEN_DEBUG(1, "yacc: rule xml_section"); } ;

xml_section:
    T_INF
    T_IDENT { XmlParserClass->new_xml_data(yytext);
              GEN_DEBUG(1,"XmlParser: start section [" << yytext << "]"); }
    xml_fields
    T_SUP { XmlParserClass->save_xml_data () ; }
    all_xml_sections { XmlParserClass->retrieve_xml_data(); }
    T_INF_SLASH
    T_IDENT { if (XmlParserClass->check_xml_data_name(yytext, yylineno) 
		  == false) {
                return (-1) ;
              }
              GEN_DEBUG(1,"XmlParser: end section [" << yytext << "]"); }
    T_SUP ;

xml_special_section:
    T_INF_ESCLAM {GEN_DEBUG(1,"yacc: xml_special_section_content"); } 
    xml_special_section_content ;

xml_special_section_content:
    xml_special_section_comment
    | xml_special_section_data { XmlParserClass->add_sub_xml_data(); } T_SUP ;

xml_special_section_comment:
    { GEN_DEBUG(1,"yacc: try xml_special_section_comment " << yytext); } 
    T_MIN_MIN { GEN_DEBUG(1,"yacc: xml_special_section_comment"); } T_MIN_MIN_SUP ;

xml_special_section_data:
    { GEN_DEBUG(1,"yacc: try xml_special_section_data"); }
    T_CROG { GEN_DEBUG(1, "yacc: rule xml_special_section_data"); }
    T_CDATA T_CROG { XmlParserClass->new_xml_data(yytext); } 
    data_buffer { GEN_DEBUG(1, "yacc: rule xml_special_section_data (end)"); }
    T_CROD { GEN_DEBUG(1, "yacc: rule xml_special_section_data (])"); } ;

data_buffer:
  | T_DATA { XmlParserClass->new_xml_field ((char*)"value");
             XmlParserClass->add_xml_field 
	       (XmlParserClass -> get_lex_string_value());
	     GEN_DEBUG(1, "XmlParser: data [" 
		   << XmlParserClass->get_lex_string_value()
		   << "]"); }
    ;

xml_fields:
  | xml_field_or_comment
    xml_fields ;

xml_field_or_comment: xml_field | xml_comment ;
  

xml_comment:
    T_INF_ESCLAM T_MIN_MIN T_MIN_MIN_SUP ;

xml_field: 
    T_IDENT { XmlParserClass->new_xml_field (yytext);
              GEN_DEBUG (1, "XmlParser: field [" << yytext << "]"); }
    T_EQUAL
    T_STRING { XmlParserClass->add_xml_field 
		 (XmlParserClass -> get_lex_string_value()); 
               GEN_DEBUG(1, "XmlParser: value [" << 
		     XmlParserClass->get_lex_string_value() << "]"); } 
    ;

xml_meta_section:
    T_INF_INTER
    T_IDENT { XmlParserClass->new_xml_data(yytext); 
    GEN_DEBUG(1, "XmlParser: meta section [" << yytext << "]"); }
    xml_fields
    T_INTER_SUP
    ;

%%

int parser_start (FILE *P_fd, C_XmlParser *P_parser) {

  int   L_parse_result ;

  XmlParserClass = P_parser ;
  yyin = P_fd ;

  L_parse_result = yyparse () ;

  if (L_parse_result != 0) {
    GEN_ERROR(E_GEN_FATAL_ERROR, "Syntax error on file [" 
	  << XmlParserClass->get_current_file_name()
	  << "] near line [" << yylineno << "]");
    return (-1) ;
  } 
  return (0) ;
}


void yyerror(const char *s)
{
  GEN_DEBUG(E_GEN_FATAL_ERROR,
            "Parser lex string value: " << XmlParserClass->get_lex_string_value());
  GEN_ERROR(E_GEN_FATAL_ERROR, 
	"Parser error [" 
	<< s << "] near [" 
	<< yytext << "] on line [" 
	<< yylineno << "]");
}

