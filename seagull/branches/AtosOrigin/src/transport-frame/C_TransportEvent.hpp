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

#ifndef _C_TRANSPORT_EVENT_
#define _C_TRANSPORT_EVENT_

//#include <malloc.h>

class C_TransportEvent {

public:

  typedef enum _trans_event_type {
    E_TRANS_NO_EVENT = 0,
    E_TRANS_RECEIVED,
    E_TRANS_CLOSED,
    E_TRANS_CONNECTION,
    E_TRANS_OPEN,
    E_TRANS_OPEN_FAILED
  } T_TransportEvent_type ;

  T_TransportEvent_type m_type       ;
  int                   m_channel_id ;
  int                   m_id         ;

  void no_event() { m_type=E_TRANS_NO_EVENT; 
		    m_channel_id = -1; 
                    m_id = -1; } ;

   C_TransportEvent () { no_event(); } ;
  ~C_TransportEvent () {} ;

} ;

typedef C_TransportEvent  *T_pC_TransportEvent ;

#endif // _C_TRANSPORT_EVENT_
