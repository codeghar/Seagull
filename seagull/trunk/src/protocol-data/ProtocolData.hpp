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

#ifndef _PROTOCOLDATA_H
#define _PROTOCOLDATA_H

#include "iostream_t.hpp"
#include "ProtocolDataType.hpp"

extern const char* type_type_table [] ;

T_TypeType typeFromString (char *P_type);
T_ValueData valueFromString (char *P_value, T_TypeType P_type, int &P_result);
const char* defaultStringValue(T_TypeType P_type);
void resetMemory(T_ValueData &P_value);

void resetValue(T_Value& P_value);

void copyValue(T_ValueData& P_dest, T_ValueData& P_Source, bool P_reset);
bool copyBinaryVal(T_ValueData& P_dest, int  P_begin, int P_size,
		   T_ValueData& P_source);
bool copyBinaryVal(T_Value& P_dest, int P_begin, int P_size,
		   T_Value& P_source);

bool extractBinaryVal(T_ValueData& P_dest, int  P_begin, int P_size,
		      T_ValueData& P_source);
bool extractBinaryVal(T_Value& P_dest, int  P_begin, int P_size,
		      T_Value& P_source);

bool compare_value(T_Value& P_left, 
                   T_Value& P_rigth);


bool operator< (const T_ValueData&, 
		const T_ValueData&);

bool operator== (const T_ValueData&, 
		const T_ValueData&);



iostream_output& operator<< (iostream_output& P_ostream, 
			     T_TypeType& P_value);

iostream_output& operator<< (iostream_output& P_ostream, 
			     T_ValueData&     P_valueData);

typedef struct _internal_value_data : public T_ValueData {
  struct _internal_value_data* m_next_instance ;
  struct _internal_value_data* m_next ;
  struct _internal_value_data* m_header ;
} T_InternalValueData, *T_pInternalValueData ;


#endif  // _PROTOCOLDATA_H










