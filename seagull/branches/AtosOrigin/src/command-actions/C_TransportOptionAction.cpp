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

#include "C_TransportOptionAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"

C_TransportOptionAction::C_TransportOptionAction(T_CmdAction        P_cmdAction,
                                                 T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}


C_TransportOptionAction::~C_TransportOptionAction() {
}


T_exeCode    C_TransportOptionAction::execute(T_pCmd_scenario P_pCmd,
                                              T_pCallContext  P_callCtxt,
                                              C_MessageFrame *P_msg,
                                              C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  int                 L_ret        = 0             ;
  // P_pCmd->m_channel_id
  L_ret = m_controllers.m_channel_ctrl->set_option_global_channel(m_id,
                                                                  m_args,
                                                                  P_callCtxt->m_channel_table);
  if (L_ret == -1) {
    L_exeCode    = E_EXE_ERROR ;
  }

  return (L_exeCode);
}





