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

#include "C_IncCounterAction.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "C_ScenarioControl.hpp"



C_IncCounterAction::C_IncCounterAction(T_CmdAction        P_cmdAction,
                                       T_pControllers P_controllers)
  : C_CommandAction (P_cmdAction, P_controllers) {
}


C_IncCounterAction::~C_IncCounterAction() {
}


T_exeCode    C_IncCounterAction::execute(T_pCmd_scenario P_pCmd,
                                         T_pCallContext  P_callCtxt,
                                         C_MessageFrame *P_msg,
                                         C_MessageFrame *P_ref) {

  T_exeCode           L_exeCode    = E_EXE_NOERROR ;
  m_controllers.m_scenario_control->increase_counter(m_id) ;

  return (L_exeCode);
}





