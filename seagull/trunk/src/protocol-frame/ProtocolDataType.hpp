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

#ifndef _PROTOCOLDATATYPE_H
#define _PROTOCOLDATATYPE_H

#include "list_t.hpp"
#include "integer_t.hpp"

typedef enum _enum_type_type {
  E_TYPE_NUMBER = 0,
  E_TYPE_SIGNED,
  //    E_TYPE_REAL,
  E_TYPE_STRING,
  E_TYPE_STRUCT,
  E_TYPE_GROUPED,
  E_TYPE_NUMBER_64,
  E_TYPE_SIGNED_64,
  E_UNSUPPORTED_TYPE
} T_TypeType, *T_pTypeType ;

typedef struct _val_binary {
  unsigned char *m_value ;
  size_t         m_size  ;
} T_ValueBinary ;

typedef struct _val_dialog_id {
  long     m_id_1  ;
  long     m_id_2  ;
} T_ValueDialogId  ;

typedef union _union_value_data {
  T_ValueBinary        m_val_binary    ;
  T_UnsignedInteger32  m_val_number    ;
  T_Integer32          m_val_signed    ;
  T_ValueDialogId      m_val_struct    ;
  T_UnsignedInteger64  m_val_number_64 ;
  T_Integer64          m_val_signed_64 ;
} T_Value, *T_pValue ;

typedef struct _struct_value {
  unsigned long m_id      ;
  T_TypeType    m_type    ;
  T_Value       m_value   ;
} T_ValueData, *T_pValueData ;

typedef list_t<T_pValueData> T_ValueDataList, 
  *T_pValueDataList ;


typedef struct _struct_multi_value {
  int           m_nb ;
  T_pValueData *m_values ;
} T_MultiValueData, *T_pMultiValueData ;

typedef struct _struct_tuple_value {
  T_pValueData  m_value ;
  T_pValueData  m_value_data ;
} T_TupleValueData, *T_pTupleValueData ;


typedef struct _struct_type_def {
  int           m_id   ;
  char         *m_name ;
  unsigned long m_size ;
  T_TypeType    m_type ;
} T_TypeDef, *T_pTypeDef ;


#endif  // _PROTOCOLDATATYPE_H







