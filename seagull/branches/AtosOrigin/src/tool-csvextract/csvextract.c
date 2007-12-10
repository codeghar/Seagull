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

#include <limits.h>
#include <math.h>
#include <time.h>

typedef enum _enum_state {
  E_INIT,
  E_FIRST_LINE_X,
  E_FIRST_LINE_Y,
  E_LINE_X,
  E_LINE_Y,
  E_END
} T_StateFile ;

void convert_date (long P_sec, char *L_buf) {

  long L_sec, L_min, L_hour, L_day ;
  long L_tmp ;

  L_day = P_sec / 86400 ;
  L_tmp = P_sec % 86400 ;
  L_hour = L_tmp / 3600 ;
  L_tmp = L_tmp % 3600 ;
  L_min = L_tmp / 60 ;
  L_sec = L_tmp % 60 ;
  
  sprintf(L_buf, "%ld %02ld:%02ld:%02ld", L_day+1, L_hour, L_min, L_sec);


}

T_StateFile next_state(T_StateFile P_state) {

  T_StateFile L_ret ;

  switch (P_state) {
  case E_INIT:
    L_ret = E_FIRST_LINE_X ;
    break ;
  case E_FIRST_LINE_X:
    L_ret = E_FIRST_LINE_Y ;
    break ;
  case E_FIRST_LINE_Y:
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

  FILE          *L_f, *L_out ;
  char          *L_name, L_dest_name[255];
  char          *L_extract ;
  char           L_char ;
  int            L_extract_field ;
  long           L_line ;
  unsigned long  L_nb_data;
  
  char           L_field[1024];
  int            L_field_size;
  fpos_t         L_pos;

  unsigned long  L_max ;
  int            L_date_format = 0 ;

  T_StateFile    L_state ;

  L_state = next_state(E_INIT) ;
  L_line = 0 ;
  L_field_size = 0 ;
  L_extract_field = -1 ;
  L_nb_data = 0UL ;

  if (argc != 3) {
    fprintf(stderr, "Syntax : %s <x|y|a|p> <in csv file>\n", argv[0]);
    fprintf(stderr, "         extract csv data into octave ascii format\n");
    fprintf(stderr, "         x for first column extraction\n");
    fprintf(stderr, "         y for second column extraction\n");
    fprintf(stderr, "         a for the two columns extraction\n");
    fprintf(stderr, "         p for the two columns extraction (time format for the first)\n");
    exit (-1);
  }

  L_extract = argv[1] ;
  if (strcmp(L_extract, "x")==0) {
    L_extract_field = 0x01 ;
  } else if (strcmp(L_extract, "y")==0) {
    L_extract_field = 0x02 ;
  } else if (strcmp(L_extract, "a")==0) {
    L_extract_field = 0x03 ;
  } else if (strcmp(L_extract, "p")==0) {
    L_extract_field = 0x03 ;
    L_date_format = 1 ;
  }

  if (L_extract_field == -1) {
    fprintf(stderr, "Error argument x or y needed\n");
    exit (-1);
  }

  L_name = argv[2] ;
  
  L_f = fopen(L_name, "r") ;
  if (L_f == NULL) {
    fprintf(stderr, "Unable to open %s", L_name);
    exit(-1);
  }

  strcpy(L_dest_name, L_name);
  strcat(L_dest_name, ".");
  strcat(L_dest_name, L_extract);
  L_out = fopen(L_dest_name, "w") ;
  if (L_out == NULL) {
    fprintf(stderr, "Unable to open %s", L_dest_name);
    exit(-1);
  } else {
    fprintf(stderr, "[creating file %s]\n", L_dest_name);
  }

  if (L_date_format == 0) {
    fprintf(L_out, "# name: data_%s\n", L_extract);
    fprintf(L_out, "# type: matrix\n");
    fprintf(L_out, "# rows: ");
    fgetpos(L_out, &L_pos);

    L_max = 0UL ;
    L_max = ~L_max ;
    sprintf(L_field, "%lu", L_max);
    make_space_string(L_field, strlen(L_field));
    fprintf(L_out, "%s\n", L_field);


    if (L_extract_field == 0x03) {
      fprintf(L_out, "# columns: 2\n");
    } else {
      fprintf(L_out, "# columns: 1\n");
    }
  }
    
  while (!feof(L_f)) { 
    
    fscanf(L_f, "%c", &L_char); 

    switch (L_char) { 
    case ';': 

      if (L_field_size >= 1024) { 
 	fprintf(stderr, "max size buffer reached (1024)"); 
 	exit(-1); 
      } 
      L_field[L_field_size]='\0'; 

      switch (L_state) { 
      case E_LINE_X: 
 	if (L_extract_field & 0x01) { 

	  if (L_date_format == 1) {
	    // modify L_field here
	    double L_value ;
	    long   L_value_long ;
	    char *L_end = NULL ;
	    L_value = strtod(L_field, &L_end);
	    L_value /= 1000.0 ;
	    L_value_long = (long) (L_value + 0.5) ;
	    convert_date(L_value_long, L_field);
	  }
	  fprintf(L_out, "%s", L_field); 
	  if (L_extract_field != 0x03) {
	    fprintf(L_out, "\n");
	  } else {
	    fprintf(L_out, " ");
	  }
 	  L_nb_data++; 
 	} 
 	break ; 
      case E_LINE_Y: 
 	if (L_extract_field & 0x02) { 
 	  fprintf(L_out, "%s\n", L_field); 
	  if (L_extract_field != 0x03) {
	    L_nb_data++; 
	  }
 	} 
	break ;
      default: 
 	break ; 
      } 

      L_field_size=0; 
      L_state = next_state(L_state); 
      break ; 


    case '\n': 
      L_line++; 
      break ; 

    default: 
      if (L_field_size >= 1024) { 
 	fprintf(stderr, "max size buffer reached (1024)"); 
 	exit(-1); 
      } 
      L_field[L_field_size]=L_char; 
      L_field_size++; 
      break ; 
    } 

  } 

  fprintf(stderr, "[data %ld]\n", L_nb_data);

  if (L_date_format == 0) {
    fsetpos(L_out, &L_pos); 
    fprintf(L_out, "%ld", L_nb_data); 
  }

  fclose(L_f);
  fclose(L_out);
  
  return (0);
}
