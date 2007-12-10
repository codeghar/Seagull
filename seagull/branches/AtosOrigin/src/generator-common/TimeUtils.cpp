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

#include "TimeUtils.hpp"

#include <ctime>
#include <cstring>
#include <cstdio>

long ms_difftime (struct timeval* P_final, struct timeval* P_init)
{
  long L_val_sec, L_val_usec;

  L_val_sec = P_final->tv_sec - P_init->tv_sec;
  L_val_usec = P_final->tv_usec - P_init->tv_usec;
  if (L_val_usec < 0) L_val_usec += 1000000, L_val_sec--;
  return (L_val_sec*1000 + L_val_usec/1000);

}

void time_tochar (char *P_time, struct timeval* P_tv)
{
  struct tm * L_currentDate;
  
  // Get the current date and time
  L_currentDate = localtime ((const time_t *)&P_tv->tv_sec);
  
  // Format the time
  if (L_currentDate == NULL) {
    memset (P_time, 0, TIME_STRING_LENGTH);
  } else {
    sprintf(P_time, 
	    "%4.4d-%2.2d-%2.2d.%2.2d:%2.2d:%2.2d.%3.3d", 
	    L_currentDate->tm_year + 1900,
	    L_currentDate->tm_mon + 1,
	    L_currentDate->tm_mday,
	    L_currentDate->tm_hour,
	    L_currentDate->tm_min,
	    L_currentDate->tm_sec,
	    (int)((P_tv->tv_usec)/1000));
  }
} 

void time_tochar_minus (char *P_time, struct timeval* P_tv)
{
  struct tm * L_currentDate;
  
  // Get the current date and time
  L_currentDate = localtime ((const time_t *)&P_tv->tv_sec);
  
  // Format the time
  if (L_currentDate == NULL) {
    memset (P_time, 0, TIME_STRING_LENGTH);
  } else {
    sprintf(P_time, 
	    "%4.4d-%2.2d-%2.2d.%2.2d-%2.2d-%2.2d.%3.3d", 
	    L_currentDate->tm_year + 1900,
	    L_currentDate->tm_mon + 1,
	    L_currentDate->tm_mday,
	    L_currentDate->tm_hour,
	    L_currentDate->tm_min,
	    L_currentDate->tm_sec,
	    (int)((P_tv->tv_usec)/1000));
  }
} 

