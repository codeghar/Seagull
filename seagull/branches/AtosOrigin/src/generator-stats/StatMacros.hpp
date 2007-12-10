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

#ifndef _STAT_MACROS_H
#define _STAT_MACROS_H


#define DISPLAY_LINE()\
	printf("|------------------------------------------------------------------------------|\r\n")
#define DISPLAY_DLINE()\
	printf("================================================================================\r\n")
#define DISPLAY_CROSS_LINE()\
        printf("|------------------------+---------------------------+-------------------------|\r\n")

#define DISPLAY_HEADER()\
   printf("|    Counter Name        |      Periodic value       |     Cumulative value    |\r\n")
#define DISPLAY_TXT_COL(T1, V1, V2)\
	printf("| %-22.22s | %-25.25s |", T1, V1); printf(" %-24.24s|\r\n", V2)
#define DISPLAY_VAL_RATEF_COL(T1, V1, V2)\
	printf("| %-22.22s | %8.3f cps              | %8.3f cps            |\r\n", T1, V1, V2)
#define DISPLAY_2VAL(T1, V1, V2)\
	printf("| %-22.22s | %8ld                  | %8ld                |\r\n", T1, V1, V2)
#define DISPLAY_CUMUL(T1, V1)\
	printf("| %-22.22s |                           | %8ld                |\r\n", T1, V1)
#define DISPLAY_PERIO(T1, V1)\
	printf("| %-22.22s | %8ld                  |                         |\r\n", T1, V1)
#define DISPLAY_VALF(T1, V1)\
	printf("| %-22.22s | %8.3f ms                                         |\r\n", T1, V1)
#define DISPLAY_VAL_RATEF(T1, V1)\
	printf("| %-22.22s | %8.3f cps                                        |\r\n", T1, V1)
#define DISPLAY_VAL_RATE(T1, V1)\
	printf("| %-22.22s | %8d cps                                        |\r\n", T1, V1)
#define DISPLAY_VAL(T1, V1)\
	printf("| %-22.22s : %8d                                            |\r\n", T1, V1)
#define DISPLAY_2VALF(T1, V1, T2, V2)\
	printf("| %-22.22s : %8.2f  | %-7.7s : %8.2f                      |\r\n", T1, V1, T2, V2)
#define DISPLAY_3VAL(T1, V1, T2, V2, T3, V3)\
	printf("| %-22.22s : %8d  | %-7.7s : %8d  | %-12.12s : %5d|\r\n", T1, V1, T2, V2, T3, V3)
#define DISPLAY_3VALF(T1, V1, T2, V2, T3, V3)\
	printf("| %-22.22s : %8.3f  | %-7.7s : %8.3f  | %-12.12s : %5.1f|\r\n", T1, V1, T2, V2, T3, V3)
#define DISPLAY_TXT(T1, V1)\
	printf("| %-22.22s | %-52.52s|\r\n", T1, V1)
#define DISPLAY_3TXT(T1, T2, T3)\
	printf("| %-22s | %25s | %23s |\r\n", (T1), (T2), (T3))

//#define DISPLAY_3TXT(T1, T2, T3)
//	printf("| %-22.22s | %-25.25s | %-25.25s |\r\n", (T1), (T2), (T3))


#define DISPLAY_4TXT(T1, T2, T3, T4)\
	printf("| %-10.10s | %-19.19s | %-19.19s | %-19.19s |\r\n", (T1), (T2), (T3), (T4))
#define DISPLAY_INFO(T1)\
	printf("| %-77.77s|\r\n", T1)

#define DISPLAY_REPART(T1, T2, V1)\
   printf("|   %8d ms <= n <  %8d ms : %10ld  %-29.29s|\r\n", T1, T2, V1, "")


#define DISPLAY_LAST_REPART(T1, V1)\
   printf("|   %14.14s n >= %8d ms : %10ld  %-29.29s|\r\n", "", T1, V1, "")

#define DISPLAY_VAL_RATEF_TPS(T1, V1, V2)\
	printf("| %-22.22s | %8.3f                  | %8.3f                |\r\n", T1, V1, V2)

#define DISPLAY_2VAL_RATEF(T1, V1, V2)\
	printf("| %-22.22s | %8.3f                  | %8.3f                |\r\n", T1, V1, V2)


#define DISPLAY_2VAL_CURRENTF(T1, V1, V2)\
	printf("| %-22.22s | %8ld                  | %8.3f                |\r\n", T1, V1, V2)


#define DISPLAY_REPART_F(T1, T2, V1, V2)\
   printf("|   %8d ms <= n <  %8d ms : %10ld     : %8.3f %-15.15s|\r\n", T1, T2, V1, V2, "")


#define DISPLAY_LAST_REPART_F(T1, V1, V2)\
   printf("|   %14.14s n >= %8d ms : %10ld     : %8.3f %-15.15s|\r\n", "", T1, V1, V2,"")



#define ZERO_COUNTER_VALUE      (unsigned long) 0


#define DISPLAY_HEADER_NIL()\
   printf("|                        |                           |                         |\r\n")


#define DISPLAY_SCEN_INFO(T1,T2)\
	printf("| %-22.22s : %02d                                                  |\r\n", T1, T2)

#define RESET_COUNTERS(PT)\
	memset (PT, 0, C_GeneratorStats::E_NB_COUNTER * sizeof(unsigned long))

#define RESET_PD_COUNTERS(PT)\
	{\
	   int i;\
           for (i=C_GeneratorStats::CPT_PD_IncomingCallCreated; \
                i<=C_GeneratorStats::CPT_PD_FailedCallRegexpHdrNotFound; i++)\
			PT[i] = (unsigned long) 0;\
	}

#define RESET_PL_COUNTERS(PT)\
	{\
	   int i;\
	   for (i=C_GeneratorStats::CPT_PL_IncomingCallCreated; \
                i<=C_GeneratorStats::CPT_PL_FailedCallRegexpHdrNotFound; i++)\
			PT[i] = (unsigned long) 0;\
	}


#endif // _STAT_MACROS_H
