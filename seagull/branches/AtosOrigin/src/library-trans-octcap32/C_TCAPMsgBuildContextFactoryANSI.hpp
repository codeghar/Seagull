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

#ifndef _C_TCAPMSGBUILDCONTEXTFACTORYANSI_
#define _C_TCAPMSGBUILDCONTEXTFACTORYANSI_

#ifndef TCAP_ANSI90
#define TCAP_ANSI90
#endif 

#ifdef TCAP_WHITE
#undef TCAP_WHITE
#endif 

#ifdef C_TcapStackNoFlavour
#undef C_TcapStackNoFlavour
#endif

#ifdef C_TCAPMsgBuildContextNoFlavour
#undef C_TCAPMsgBuildContextNoFlavour
#endif

#ifdef C_TCAPMsgBuildContextFactoryNoFlavour
#undef C_TCAPMsgBuildContextFactoryNoFlavour
#endif

#ifdef create_tcap_noflavour_msg_context_factory
#undef create_tcap_noflavour_msg_context_factory
#endif

#ifdef delete_tcap_noflavour_msg_context_factory
#undef delete_tcap_noflavour_msg_context_factory
#endif

#ifdef _ITU_STANDARD
#undef _ITU_STANDARD
#endif

#ifndef _ANSI_STANDARD
#define _ANSI_STANDARD
#endif

#define C_TCAPMsgBuildContextFactoryNoFlavour C_TCAPMsgBuildContextFactoryANSI
#define C_TCAPMsgBuildContextNoFlavour C_TCAPMsgBuildContextANSI
#define C_TcapStackNoFlavour C_TcapStackANSI
#define create_tcap_noflavour_msg_context_factory create_tcap_ansi_msg_context_factory
#define delete_tcap_noflavour_msg_context_factory delete_tcap_ansi_msg_context_factory


#include "C_TCAPMsgBuildContextFactoryNoFlavour.header"

#endif // _C_TCAPMSGBUILDCONTEXTANSI_

