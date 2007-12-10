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

#ifndef _C_MESSAGEBINARYSEPARATOR_
#define _C_MESSAGEBINARYSEPARATOR_

#include "C_MessageBinary.hpp"
#include "C_ProtocolBinarySeparator.hpp"


class C_MessageBinarySeparator : public C_MessageBinary {

public:

   C_MessageBinarySeparator(C_ProtocolBinarySeparator *P_protocol);
  ~C_MessageBinarySeparator();


  T_Value  get_type() ;

  virtual bool update_fields (C_MessageFrame* P_msg) ;
  virtual bool compare_types (C_MessageFrame *P_ref) ;

  virtual void   update_message_stats  () ;


} ; // class C_MessageBinarySeparator



typedef C_MessageBinarySeparator *T_pC_MessageBinarySeparator ;

#endif // _C_MESSAGEBINARYSEPARATOR_

