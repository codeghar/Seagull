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

#ifndef    _C_PROTOCOLSTATSFRAME_H
#define    _C_PROTOCOLSTATSFRAME_H

#include "ProtocolStats.hpp"
#include "C_ProtocolFrame.hpp"
#include "C_DisplayObject.hpp"

class C_ProtocolStatsFrame {

public:


  C_ProtocolStatsFrame (C_ProtocolFrame *P_protocol,
			C_DisplayObject *P_display) {
    m_protocol = P_protocol ; 
    m_display = P_display ; } 
  virtual ~C_ProtocolStatsFrame() { 
    m_protocol = NULL; 
    m_display= NULL ; } 

  C_ProtocolFrame* get_protocol() { return(m_protocol); }
  C_DisplayObject* get_display() { return (m_display); }

  virtual void updateStats (T_ProtocolStatDataType P_type,
			    T_ProtocolStatDataAction P_action,
			    int P_id) = 0 ;


  virtual void set_file_name  (char * P_name) = 0 ; 
  virtual int  init()                         = 0 ;

  virtual void make_first_log ()              = 0 ;
  virtual void make_log       ()              = 0 ;
  virtual void do_log         ()              = 0 ;
  virtual void reset_cumul_counters ()        = 0 ;


protected:
  C_ProtocolFrame *m_protocol ;
  C_DisplayObject *m_display ;

} ;

#endif  // _C_PROTOCOLSTATSFRAME_H




