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

#ifndef _C_TCAPMSGBUILDCONTEXTITU_
#define _C_TCAPMSGBUILDCONTEXTITU_

#ifdef TCAP_ANSI90
#undef TCAP_ANSI90
#endif 

#ifndef TCAP_WHITE
#define TCAP_WHITE
#endif 

#ifdef C_TcapStackNoFlavour
#undef C_TcapStackNoFlavour
#endif

#ifdef C_TCAPMsgBuildContextNoFlavour
#undef C_TCAPMsgBuildContextNoFlavour
#endif

#define C_TCAPMsgBuildContextNoFlavour C_TCAPMsgBuildContextITU
#define C_TcapStackNoFlavour C_TcapStackITU

#ifndef _ITU_STANDARD
#define _ITU_STANDARD
#endif

#ifdef _ANSI_STANDARD
#undef _ANSI_STANDARD
#endif


#include "C_TCAPMsgBuildContextNoFlavour.header"

#endif // _C_TCAPMSGBUILDCONTEXTITU_

