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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

typedef enum _enum_state {
  E_INIT,
  E_LINE_X,
  E_LINE_Y,
  E_END
} T_StateFile ;

T_StateFile next_state(T_StateFile P_state) {

  T_StateFile L_ret ;

  switch (P_state) {
  case E_INIT:
    L_ret = E_LINE_X ;
    break ;
  case E_LINE_X:
    L_ret = E_LINE_Y ;
    break ;
  case E_LINE_Y:
    L_ret = E_LINE_X ;
    break ;
  case E_END:
    L_ret = E_END ;
    break ;
  }
  
  return (L_ret);
}

void make_space_string(char *P_string, size_t P_size) {
  size_t L_i ;
  for(L_i = 0; L_i < P_size; L_i++) {
    P_string[L_i]=' ';
  }
  P_string[L_i]='\0';
}

int main(int argc, char**argv) {

  FILE          *L_in, *L_out ;
  long           L_line ;
  unsigned long  L_nb_data;
  
  char           L_field[1024];
  int            L_field_size;
  
  char           L_char ;


  int            L_nb_args = argc       ;
  char*          L_file_tab[2]          ;
  char          *L_in_file, *L_out_file ;
  int            L_nb_file = 0          ;
  char*          L_arg_value            ;

  int            L_skip = 0             ;
  int            L_ratio = 10           ;
  int            L_select = 0           ;

  T_StateFile    L_state ;

  L_state = next_state(E_INIT) ;
  L_line = 0 ;
  L_field_size = 0 ;
  L_nb_data = 0UL ;

  if (argc < 3) {
    fprintf(stderr, "Syntax : %s <in csv file> <out csv file>\n", argv[0]);
    fprintf(stderr, "         [-skip n] skip the n first values (default 0)\n");
    fprintf(stderr, "         [-ratio r] let 1 out of r value (default 10)\n");
    exit (-1);
  }

  L_nb_args--;
  while (L_nb_args) {
    L_arg_value = argv[argc-L_nb_args];
    if (strcmp(L_arg_value, "-skip") == 0) {
      if (L_nb_args) {
	L_nb_args--;
	L_arg_value=argv[argc-L_nb_args];
	L_skip = atoi(L_arg_value);
      } else {
	fprintf(stderr, "argument expected for -skip\n");
	exit (-1);
      }
    } else if (strcmp(L_arg_value, "-ratio") == 0) {
      if (L_nb_args) {
	L_nb_args--;
	L_arg_value=argv[argc-L_nb_args];
	L_ratio = atoi(L_arg_value);
      } else {
	fprintf(stderr, "argument expected for -ratio\n");
	exit (-1);
      }
    } else {
      if (L_nb_file < 2) {
	L_file_tab[L_nb_file] = L_arg_value ;
	L_nb_file++;
      } else {
	fprintf(stderr, "syntax error %s unexpected\n", L_arg_value);
	exit (-1);
      }
    }
    L_nb_args-- ;
  }

  if (L_nb_file != 2) {
    fprintf(stderr, "error on file in/out arguments\n");
    exit (-1);
  } else {
    L_in_file = L_file_tab[0];
    L_out_file = L_file_tab[1] ;
  }

  fprintf(stderr, "[in: %s; out:%s; skip:%d; ratio:%d]\n", 
	  L_file_tab[0], L_file_tab[1], L_skip, L_ratio);

  L_in = fopen(L_in_file, "r") ; 
  if (L_in == NULL) { 
    fprintf(stderr, "Unable to open %s\n", L_in_file); 
    exit(-1); 
  } 

  L_out = fopen(L_out_file, "w") ; 
  if (L_out == NULL) { 
    fprintf(stderr, "Unable to open %s\n", L_out_file); 
    exit(-1); 
  } 

  while (!feof(L_in)) {

    L_char = fgetc(L_in) ;

    switch(L_char) {
    case ';':
      if (L_field_size >= 1024) {
	fprintf(stderr, "Maximum field size (1024) reached\n");
	exit (-1);
      }
      L_field[L_field_size]=L_char;
      L_field_size++;
      switch (L_state) {
      case E_LINE_X:
	break;
      case E_LINE_Y:
	if (L_field_size >= 1024) {
	  fprintf(stderr, "Maximum field size (1024) reached\n");
	  exit (-1);
	}
	L_field[L_field_size]=0;
	if (L_line == 0){
	  fprintf(L_out,"%s\n",L_field);
	  L_field_size=0;
	  break;
	}	  
	L_nb_data++;
	if (L_nb_data > L_skip) {
	  if (L_select == 0) {
	    fprintf(L_out,"%s\n",L_field);
	    L_select = L_ratio - 1;
	  } else {
	    L_select -- ;
	  }
	}
	L_field_size=0;
	break;
      default:
	break;
      }
      L_state = next_state(L_state);
      break ;
    case '\n':
      L_line++;
      break ;
    default:
      if (L_field_size >= 1024) {
	fprintf(stderr, "max buffer size (1024) reached");
	exit (-1);
      }
      L_field[L_field_size]=L_char;
      L_field_size++;
      break ;
    }

  }

  fclose(L_in) ;
  fclose(L_out) ;

  return (0);
}
