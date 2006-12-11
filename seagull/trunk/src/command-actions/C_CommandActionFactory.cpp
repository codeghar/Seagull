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


#include "C_CommandActionFactory.hpp"
#include "C_StoreAction.hpp"
#include "C_ReStoreAction.hpp"
#include "C_StartTimerAction.hpp"
#include "C_StopTimerAction.hpp"
#include "C_SetValueAction.hpp"
#include "C_IncCounterAction.hpp"
#include "C_CheckPresenceAction.hpp"
#include "C_GetExternalDataAction.hpp"
#include "C_IncVarAction.hpp"
#include "C_CheckValueAction.hpp"
#include "C_CheckOrderAction.hpp"
#include "C_SetNewSessionIdAction.hpp"
#include "C_InternalInitDoneAction.hpp"
#include "C_CheckAllMsgAction.hpp"
#include "C_AddInCallMapAction.hpp"
#include "C_SelectExternalDataLineAction.hpp"
#include "C_OpenAction.hpp"
#include "C_CloseAction.hpp"
#include "C_TransportOptionAction.hpp"
#include "C_SetBitAction.hpp"
#include "C_SetValueBitAction.hpp"
#include "C_GetExternalDataActionToMem.hpp"



#include "Utils.hpp"

C_CommandActionFactory::C_CommandActionFactory(T_pControllers P_controllers) {
  m_controllers = *P_controllers ;
}

C_CommandActionFactory::~C_CommandActionFactory() {
}

C_CommandAction* C_CommandActionFactory::create(T_CmdAction P_cmdAction) {

  C_CommandAction* L_action = NULL ;
  
  switch (P_cmdAction.m_type) {

  case E_ACTION_SCEN_OPEN:
    NEW_VAR(L_action, 
            C_OpenAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_CLOSE:
    NEW_VAR(L_action, 
            C_CloseAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_MEMORY_STORE:
    NEW_VAR(L_action, 
            C_StoreAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_MEMORY_RESTORE:
    NEW_VAR(L_action, 
            C_ReStoreAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_START_TIMER:
    NEW_VAR(L_action, 
            C_StartTimerAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_STOP_TIMER:
    NEW_VAR(L_action, 
            C_StopTimerAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_SET_VALUE:
    NEW_VAR(L_action, 
            C_SetValueAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_INC_COUNTER:
    NEW_VAR(L_action, 
            C_IncCounterAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_CHECK_PRESENCE:
    NEW_VAR(L_action, 
            C_CheckPresenceAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_GET_EXTERNAL_DATA:
    NEW_VAR(L_action, 
            C_GetExternalDataAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_INC_VAR:
    NEW_VAR(L_action, 
            C_IncVarAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_CHECK_VALUE:
    NEW_VAR(L_action, 
            C_CheckValueAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_CHECK_ORDER:
    NEW_VAR(L_action, 
            C_CheckOrderAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_SET_NEW_SESSION_ID:
    NEW_VAR(L_action, 
            C_SetNewSessionIdAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_INTERNAL_INIT_DONE:
    NEW_VAR(L_action, 
            C_InternalInitDoneAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_CHECK_ALL_MSG:
    NEW_VAR(L_action, 
            C_CheckAllMsgAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_ADD_IN_CALL_MAP:
    NEW_VAR(L_action, 
            C_AddInCallMapAction(P_cmdAction,&m_controllers));
    break ;
  case E_ACTION_SCEN_SELECT_EXTERNAL_DATA_LINE:
    NEW_VAR(L_action, 
            C_SelectExternalDataLineAction(P_cmdAction,&m_controllers));
    break ;

  case E_ACTION_SCEN_TRANSPORT_OPTION:
    NEW_VAR(L_action, 
            C_TransportOptionAction(P_cmdAction,&m_controllers));
    break ;

  case E_ACTION_SCEN_SET_BIT:
    NEW_VAR(L_action, 
            C_SetBitAction(P_cmdAction,&m_controllers));
    break ;

  case E_ACTION_SCEN_SET_VALUE_BIT:
    NEW_VAR(L_action, 
            C_SetValueBitAction(P_cmdAction,&m_controllers));
    break ;

  case E_ACTION_SCEN_GET_EXTERNAL_DATA_TO_MEM:
    NEW_VAR(L_action, 
            C_GetExternalDataActionToMem(P_cmdAction,&m_controllers));
    break ;

  default :
    break ;
  }

  return(L_action);
}
