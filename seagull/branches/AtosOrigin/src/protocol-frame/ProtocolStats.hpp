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

#ifndef _PROTOCOL_STATS_H
#define _PROTOCOL_STATS_H

typedef enum _protocol_stat_data_type {
  E_MESSAGE,
  E_MESSAGE_COMPONENT
} T_ProtocolStatDataType, *T_pProtocolStateDataType ;
  
typedef enum _protocol_stat_data_action {
  E_RECEIVE,
  E_SEND,
  E_NB_ACTIONS
} T_ProtocolStatDataAction, *T_pProtocolStatDataAction ;

typedef int T_CounterType, *T_pCounterType ;

#endif // _PROTOCOL_STATS_H

