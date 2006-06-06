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

#include "C_GeneratorStats.hpp"
#include "GeneratorTrace.hpp"
#include "GeneratorError.h"
#include "StatMacros.hpp"
#include "Utils.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring> // for memset
#include <ctime> 

#include "iostream_t.hpp"


C_GeneratorStats* C_GeneratorStats::instance()
{
   if (m_instance == NULL) {
     NEW_VAR(m_instance, C_GeneratorStats()) ;
   }
   return m_instance;
}


void C_GeneratorStats::close ()
{

  FREE_TABLE(m_ResponseTimeRepartition);

  FREE_TABLE(m_CallLengthRepartition) ;

  if(m_outputStream != NULL)
  {
    m_outputStream->close();
    DELETE_VAR(m_outputStream);
  }

  FREE_TABLE(m_fileName);

  FREE_TABLE(m_current_repartition_table);


  m_SizeOfResponseTimeRepartition = 0;
  m_SizeOfCallLengthRepartition   = 0;
  m_CallLengthRepartition         = NULL;
  m_fileName                      = NULL;
  m_ResponseTimeRepartition       = NULL;
  m_outputStream                  = NULL;

  DELETE_VAR(m_instance);
  m_instance                      = NULL;

}


int C_GeneratorStats::init () 
{
  if (m_initDone == false) {
    // reset of all counter
    RESET_COUNTERS(m_counters);
    GET_TIME (&m_startTime);
    memcpy   (&m_pdStartTime, &m_startTime, sizeof (struct timeval));
    memcpy   (&m_plStartTime, &m_startTime, sizeof (struct timeval));
    m_outputStream = NULL;
    m_headerAlreadyDisplayed = false;
    m_initDone = true ;

    m_current_repartition_table = NULL ;
    m_current_display_rep_id = 0 ;
    m_nb_display_rep_id = 2 ;
    ALLOC_TABLE(m_current_repartition_table, 
		T_DisplayRepFct*, 
		sizeof(T_DisplayRepFct), 2);

    m_current_repartition_table[0] = &C_GeneratorStats::displayRepartition_without_percent ;
    m_current_repartition_table[1] = &C_GeneratorStats::displayRepartition_with_percent ;
    m_current_repartition_display = m_current_repartition_table[0];
  }
  return(1);
}


int C_GeneratorStats::isWellFormed(char * P_listeStr, int * nombre)
{
  char * ptr = P_listeStr;
  int sizeOf;
  bool isANumber;

  (*nombre) = 0; 
  sizeOf = strlen(P_listeStr);
  // getting the number 
  if(sizeOf > 0)
  {
    // is the string well formed ? [0-9] [,]
    isANumber = false;
    for(int i=0; i<=sizeOf; i++)
    {
      switch(ptr[i])
      {
        case ',':
                if(isANumber == false)
                {   
                  return(0);
                }
                else
                {
                  (*nombre)++;             
                } 
  		          isANumber = false;
       	       break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
                 isANumber = true;
                 break;
        case '\t':
        case ' ' :
                 break;
        case '\0':
                if(isANumber == false)
                {   
                  return(0);
                }
                else
                {
                  (*nombre)++;
                } 
          break;
		  default:
          return(0);
      }
    } // enf for
  }
  return(1);
}


int C_GeneratorStats::createIntegerTable(char * P_listeStr, unsigned int ** listeInteger, int * sizeOfList)
{
  int nb=0;
  char * ptr = P_listeStr;
  char * ptr_prev = P_listeStr;
  unsigned int current_int;
 
  if(isWellFormed(P_listeStr, sizeOfList) == 1)
  {

    ALLOC_TABLE(*listeInteger, unsigned int*,
	      sizeof(unsigned int), *sizeOfList);

     while((*ptr) != ('\0'))
     {
       if((*ptr) == ',')
       {
         sscanf(ptr_prev, "%u", &current_int);
         if (nb<(*sizeOfList))
           (*listeInteger)[nb] = current_int;
	  	   nb++;
         ptr_prev = ptr+1;
       }
       ptr++;
     }
     // on lit le dernier
     sscanf(ptr_prev, "%u", &current_int); 
     if (nb<(*sizeOfList))
       (*listeInteger)[nb] = current_int;
     nb++;
    return(1);
   }
  return(0);
}


void C_GeneratorStats::setFileName(char * P_name, char * P_extension)
{
  int sizeOf, sizeOfExtension; 

  if(P_name != NULL) 
  {
    sizeOf = strlen(P_name);
    if (sizeOf > 0) {
      if(P_extension != NULL) { 
        sizeOfExtension = strlen(P_extension); 
        if (sizeOfExtension > 0) {
	  FREE_TABLE(m_fileName);
          sizeOf += sizeOfExtension;
	  ALLOC_TABLE(m_fileName, char*,
		    sizeof(char), sizeOf+1);
          strcpy(m_fileName, P_name);
          strcat(m_fileName, P_extension);
	} else {
	  FREE_TABLE(m_fileName);
          sizeOf += strlen(DEFAULT_EXTENSION);
	  ALLOC_TABLE(m_fileName, char*,
		    sizeof(char), sizeOf+1);
          strcpy(m_fileName, P_name);
          strcat(m_fileName, DEFAULT_EXTENSION);
	}
      } else {
	FREE_TABLE(m_fileName);
        sizeOf += strlen(DEFAULT_EXTENSION);
	ALLOC_TABLE(m_fileName, char*,
		    sizeof(char), sizeOf+1);
        strcpy(m_fileName, P_name);
        strcat(m_fileName, DEFAULT_EXTENSION);
      }
    } else {
      iostream_error << "new file name length is null - keeping the default filename : "
                << DEFAULT_FILE_NAME << iostream_endl;
    }
  } else {
    iostream_error << "new file name is NULL ! - keeping the default filename : " 
	      << DEFAULT_FILE_NAME << iostream_endl;
  }
}


void C_GeneratorStats::setFileName(char * P_name)
{
  int sizeOf; 


  if(P_name != NULL) { 
    sizeOf = strlen(P_name);
    if(sizeOf > 0) {
      FREE_TABLE(m_fileName);
      ALLOC_TABLE(m_fileName, char*,
                  sizeof(char), sizeOf+1);
      strcpy(m_fileName, P_name);
    } else {
      iostream_error << "new file name length is null - keeping the default filename : "
                << DEFAULT_FILE_NAME << iostream_endl;
    }
  } else {
    iostream_error << 
      "new file name is NULL ! - keeping the default filename : " 
	      << DEFAULT_FILE_NAME << iostream_endl;
  }
}



void C_GeneratorStats::setRepartitionCallLength(char * P_listeStr)
{
  unsigned int * listeInteger;
  int sizeOfListe;

  if(createIntegerTable(P_listeStr, &listeInteger, &sizeOfListe) == 1)
    initRepartition(listeInteger, sizeOfListe, &m_CallLengthRepartition, &m_SizeOfCallLengthRepartition);
  else
  {
    m_CallLengthRepartition         = NULL;
    m_SizeOfCallLengthRepartition   = 0;
  }
  FREE_TABLE(listeInteger);
}

void C_GeneratorStats::setRepartitionResponseTime (char * P_listeStr)
{
  unsigned int * listeInteger;
  int sizeOfListe;

  if(createIntegerTable(P_listeStr, &listeInteger, &sizeOfListe) == 1)
    initRepartition(listeInteger, sizeOfListe, &m_ResponseTimeRepartition, &m_SizeOfResponseTimeRepartition);
  else
  {
    m_CallLengthRepartition         = NULL;
    m_SizeOfCallLengthRepartition   = 0;
  }
  FREE_TABLE(listeInteger);
} 


void C_GeneratorStats::setRepartitionCallLength(unsigned int* repartition, int nombre)
{
  initRepartition(repartition, 
		  nombre, 
		  &m_CallLengthRepartition, 
		  &m_SizeOfCallLengthRepartition);
} 

void C_GeneratorStats::setRepartitionResponseTime(unsigned int* repartition, int nombre)
{
  initRepartition(repartition, 
		  nombre, 
		  &m_ResponseTimeRepartition, 
		  &m_SizeOfResponseTimeRepartition);
}


void C_GeneratorStats::initRepartition(unsigned int* repartition, 
			     int nombre, 
			     T_dynamicalRepartition ** tabRepartition, 
			     int* tabNb) 
{
  bool sortDone;
  int i;
  unsigned int swap;

  if((nombre <= 0) || (repartition == NULL) )
  {
    (*tabNb)          = 0;
    (*tabRepartition) = NULL;
    return;
  }

  (*tabNb)          = nombre + 1;
  ALLOC_TABLE(*tabRepartition, T_dynamicalRepartition*,
	    sizeof(T_dynamicalRepartition), *tabNb);
 
  // copying the repartition table in the local table 
  for(i=0; i<nombre; i++)
  { 
    (*tabRepartition)[i].borderMax      = repartition[i];
    (*tabRepartition)[i].nbInThisBorder = 0;
  } 
  
  // sorting the repartition table
  sortDone = false;
  while(!sortDone)
  { 
    sortDone = true;
    for(i=0; i<(nombre-1); i++)
    { 
      if((*tabRepartition)[i].borderMax > (*tabRepartition)[i+1].borderMax)
      {  
        // swapping this two value and setting sortDone to false
        swap = (*tabRepartition)[i].borderMax;
        (*tabRepartition)[i].borderMax   = (*tabRepartition)[i+1].borderMax;
        (*tabRepartition)[i+1].borderMax = swap;
       sortDone = false;
      } 
    } 
  }
  // setting the range for max <= value < infinity
  (*tabRepartition)[nombre].borderMax      
    = (*tabRepartition)[nombre-1].borderMax;
  (*tabRepartition)[nombre].nbInThisBorder = 0;
}


int C_GeneratorStats::computeStat (E_Action P_action)
{
  switch (P_action) {

  case E_CREATE_INCOMING_CALL :

    m_mutexTable[CPT_C_IncomingCallCreated].lock() ;
    m_counters [CPT_C_IncomingCallCreated]++;
    m_counters [CPT_PD_IncomingCallCreated]++;
    m_counters [CPT_PL_IncomingCallCreated]++;
    m_mutexTable[CPT_C_IncomingCallCreated].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]++;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_SEND_MSG :
    m_mutexTable[CPT_C_MsgSend].lock() ;
    m_counters [CPT_C_MsgSend]++;
    m_counters [CPT_PD_MsgSend]++;
    m_counters [CPT_PL_MsgSend]++;
    m_mutexTable[CPT_C_MsgSend].unlock() ;
    break;
    
  case E_RECV_MSG :
    m_mutexTable[CPT_C_MsgRecv].lock() ;
    m_counters [CPT_C_MsgRecv]++;
    m_counters [CPT_PD_MsgRecv]++;
    m_counters [CPT_PL_MsgRecv]++;
    m_mutexTable[CPT_C_MsgRecv].unlock() ;
    break;
    
  case E_CALL_FAILED :
    m_mutexTable[CPT_C_FailedCall].lock() ;
    m_counters [CPT_C_FailedCall]++;
    m_counters [CPT_PD_FailedCall]++;
    m_counters [CPT_PL_FailedCall]++;
    m_mutexTable[CPT_C_FailedCall].unlock() ;
    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;
      
  case E_CALL_INIT_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_InitSuccessfulCall].lock() ;
    m_counters [CPT_C_InitSuccessfulCall]++;
    m_counters [CPT_PD_InitSuccessfulCall]++;
    m_counters [CPT_PL_InitSuccessfulCall]++;
    m_mutexTable[CPT_C_InitSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_CALL_TRAFFIC_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_TrafficSuccessfulCall].lock() ;
    m_counters [CPT_C_TrafficSuccessfulCall]++;
    m_counters [CPT_PD_TrafficSuccessfulCall]++;
    m_counters [CPT_PL_TrafficSuccessfulCall]++;
    m_mutexTable[CPT_C_TrafficSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_CALL_DEFAULT_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_DefaultSuccessfulCall].lock() ;
    m_counters [CPT_C_DefaultSuccessfulCall]++;
    m_counters [CPT_PD_DefaultSuccessfulCall]++;
    m_counters [CPT_PL_DefaultSuccessfulCall]++;
    m_mutexTable[CPT_C_DefaultSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_CALL_ABORT_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_AbortSuccessfulCall].lock() ;
    m_counters [CPT_C_AbortSuccessfulCall]++;
    m_counters [CPT_PD_AbortSuccessfulCall]++;
    m_counters [CPT_PL_AbortSuccessfulCall]++;
    m_mutexTable[CPT_C_AbortSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_FAILED_CANNOT_SEND_MSG :
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].lock() ;
    m_counters [CPT_C_FailedCallCannotSendMessage]++;
    m_counters [CPT_PD_FailedCallCannotSendMessage]++;
    m_counters [CPT_PL_FailedCallCannotSendMessage]++;
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].unlock() ;
    break;

  case E_FAILED_UNEXPECTED_MSG :
    m_mutexTable[CPT_C_FailedCallUnexpectedMessage].lock() ;
    m_counters [CPT_C_FailedCallUnexpectedMessage]++;
    m_counters [CPT_PD_FailedCallUnexpectedMessage]++;
    m_counters [CPT_PL_FailedCallUnexpectedMessage]++;
    m_mutexTable[CPT_C_FailedCallUnexpectedMessage].unlock() ;
    break;
    
  case E_RESET_PD_COUNTERS :
    m_mutexTable[CPT_PD_IncomingCallCreated].lock();
    m_counters[CPT_PD_IncomingCallCreated] = (unsigned long) 0;
    m_counters[CPT_PD_OutgoingCallCreated] = (unsigned long) 0;
    m_counters[CPT_PD_MsgRecv] = (unsigned long) 0;
    m_counters[CPT_PD_MsgSend] = (unsigned long) 0;

    m_counters[CPT_PD_InitSuccessfulCall] = (unsigned long) 0;
    m_counters[CPT_PD_TrafficSuccessfulCall] = (unsigned long) 0;
    m_counters[CPT_PD_DefaultSuccessfulCall] = (unsigned long) 0;
    m_counters[CPT_PD_AbortSuccessfulCall] = (unsigned long) 0;

    m_counters[CPT_PD_FailedCall] = (unsigned long) 0;

    GET_TIME (&m_pdStartTime);
    m_mutexTable[CPT_PD_IncomingCallCreated].unlock();
    break;

  case E_RESET_PL_COUNTERS :
    RESET_PL_COUNTERS (m_counters);
    GET_TIME (&m_plStartTime);
    break;
    
  case E_CREATE_OUTGOING_CALL :

    m_mutexTable [CPT_C_OutgoingCallCreated].lock() ;
    m_counters [CPT_C_OutgoingCallCreated]++;
    m_counters [CPT_PD_OutgoingCallCreated]++;
    m_counters [CPT_PL_OutgoingCallCreated]++;
    m_mutexTable [CPT_C_OutgoingCallCreated].unlock() ;

    m_mutexTable [CPT_C_CurrentCall].lock() ;
    m_counters [CPT_C_CurrentCall]++;
    m_mutexTable [CPT_C_CurrentCall].unlock() ;
    break;


    // NOT USED
  case E_FAILED_MAX_UDP_RETRANS :
    m_counters [CPT_C_FailedCallMaxUdpRetrans]++;
    m_counters [CPT_PD_FailedCallMaxUdpRetrans]++;
    m_counters [CPT_PL_FailedCallMaxUdpRetrans]++;
    break;

  case E_FAILED_CALL_REJECTED :
    m_counters [CPT_C_FailedCallCallRejected]++;
    m_counters [CPT_PD_FailedCallCallRejected]++;
    m_counters [CPT_PL_FailedCallCallRejected]++;
    break;
    
  case E_FAILED_CMD_NOT_SENT :
    m_counters [CPT_C_FailedCallCmdNotSent]++;
    m_counters [CPT_PD_FailedCallCmdNotSent]++;
    m_counters [CPT_PL_FailedCallCmdNotSent]++;
    break;

  case E_FAILED_REGEXP_DOESNT_MATCH :
    m_counters [CPT_C_FailedCallRegexpDoesntMatch]++;
    m_counters [CPT_PD_FailedCallRegexpDoesntMatch]++;
    m_counters [CPT_PL_FailedCallRegexpDoesntMatch]++;
    break;
    
  case E_FAILED_REGEXP_HDR_NOT_FOUND :
    m_counters [CPT_C_FailedCallRegexpHdrNotFound]++;
    m_counters [CPT_PD_FailedCallRegexpHdrNotFound]++;
    m_counters [CPT_PL_FailedCallRegexpHdrNotFound]++;
    break;
    

  default :
    GEN_FATAL(0,"C_GeneratorStats::ComputeStat() - Unrecognized Action " <<  P_action);
    break ;
  } /* end switch */

  return (0);
}



int C_GeneratorStats::executeStatAction (E_Action P_action) {
  
  switch (P_action) {

  case E_CREATE_INCOMING_CALL :

    m_mutexTable[CPT_C_IncomingCallCreated].lock() ;
    m_counters [CPT_C_IncomingCallCreated]++;
    m_counters [CPT_PD_IncomingCallCreated]++;
    m_counters [CPT_PL_IncomingCallCreated]++;
    m_mutexTable[CPT_C_IncomingCallCreated].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]++;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_SEND_MSG :
    m_mutexTable[CPT_C_MsgSend].lock() ;
    m_counters [CPT_C_MsgSend]++;
    m_counters [CPT_PD_MsgSend]++;
    m_counters [CPT_PL_MsgSend]++;
    m_mutexTable[CPT_C_MsgSend].unlock() ;
    break;

  case E_RECV_MSG :
    m_mutexTable[CPT_C_MsgRecv].lock() ;
    m_counters [CPT_C_MsgRecv]++;
    m_counters [CPT_PD_MsgRecv]++;
    m_counters [CPT_PL_MsgRecv]++;
    m_mutexTable[CPT_C_MsgRecv].unlock() ;
    break;
  
  case E_CALL_FAILED :
    m_mutexTable[CPT_C_FailedCall].lock() ;
    m_counters [CPT_C_FailedCall]++;
    m_counters [CPT_PD_FailedCall]++;
    m_counters [CPT_PL_FailedCall]++;
    m_mutexTable[CPT_C_FailedCall].unlock() ;
    
    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;
    
  case E_CALL_INIT_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_InitSuccessfulCall].lock() ;
    m_counters [CPT_C_InitSuccessfulCall]++;
    m_counters [CPT_PD_InitSuccessfulCall]++;
    m_counters [CPT_PL_InitSuccessfulCall]++;
    m_mutexTable[CPT_C_InitSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_CALL_TRAFFIC_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_TrafficSuccessfulCall].lock() ;
    m_counters [CPT_C_TrafficSuccessfulCall]++;
    m_counters [CPT_PD_TrafficSuccessfulCall]++;
    m_counters [CPT_PL_TrafficSuccessfulCall]++;
    m_mutexTable[CPT_C_TrafficSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_CALL_DEFAULT_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_DefaultSuccessfulCall].lock() ;
    m_counters [CPT_C_DefaultSuccessfulCall]++;
    m_counters [CPT_PD_DefaultSuccessfulCall]++;
    m_counters [CPT_PL_DefaultSuccessfulCall]++;
    m_mutexTable[CPT_C_DefaultSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_CALL_ABORT_SUCCESSFULLY_ENDED :
    m_mutexTable[CPT_C_AbortSuccessfulCall].lock() ;
    m_counters [CPT_C_AbortSuccessfulCall]++;
    m_counters [CPT_PD_AbortSuccessfulCall]++;
    m_counters [CPT_PL_AbortSuccessfulCall]++;
    m_mutexTable[CPT_C_AbortSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]--;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;

  case E_FAILED_CANNOT_SEND_MSG :
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].lock() ;
    m_counters [CPT_C_FailedCallCannotSendMessage]++;
    m_counters [CPT_PD_FailedCallCannotSendMessage]++;
    m_counters [CPT_PL_FailedCallCannotSendMessage]++;
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].unlock() ;
    break;

  case E_FAILED_UNEXPECTED_MSG :
    m_mutexTable [CPT_C_FailedCallUnexpectedMessage].lock() ;
    m_counters   [CPT_C_FailedCallUnexpectedMessage]++;
    m_counters   [CPT_PD_FailedCallUnexpectedMessage]++;
    m_counters   [CPT_PL_FailedCallUnexpectedMessage]++;
    m_mutexTable [CPT_C_FailedCallUnexpectedMessage].unlock() ;
    break;

  case E_CALL_REFUSED :
    m_mutexTable[CPT_C_RefusedCall].lock() ;
    m_counters [CPT_C_RefusedCall]++;
    m_counters [CPT_PD_RefusedCall]++;
    m_counters [CPT_PL_RefusedCall]++;
    m_mutexTable[CPT_C_RefusedCall].unlock() ;
    break;


  case E_FAILED_ABORTED:
    m_mutexTable[CPT_C_FailedCallAborted].lock() ;
    m_counters [CPT_C_FailedCallAborted] ++ ;
    m_counters [CPT_PD_FailedCallAborted] ++ ;
    m_counters [CPT_PL_FailedCallAborted] ++ ;
    m_mutexTable[CPT_C_FailedCallAborted].unlock() ;
    break ;

  case E_FAILED_TIMEOUT:
    m_mutexTable[CPT_C_FailedCallTimeout].lock() ;
    m_counters [CPT_C_FailedCallTimeout] ++ ;
    m_counters [CPT_PL_FailedCallTimeout] ++ ;
    m_counters [CPT_PD_FailedCallTimeout] ++ ;
    m_mutexTable[CPT_C_FailedCallTimeout].unlock() ;
    break ;
  

  case E_RESET_PD_COUNTERS :

    m_mutexTable[CPT_C_IncomingCallCreated].lock() ;
    m_counters [CPT_PD_IncomingCallCreated] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_IncomingCallCreated].unlock() ;

    m_mutexTable[CPT_C_MsgSend].lock() ;
    m_counters [CPT_PD_MsgSend] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_MsgSend].unlock() ;

    m_mutexTable[CPT_C_MsgRecv].lock() ;
    m_counters [CPT_PD_MsgRecv] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_MsgRecv].unlock() ;

    m_mutexTable[CPT_C_FailedCall].lock() ;
    m_counters [CPT_PD_FailedCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCall].unlock() ;
    
    m_mutexTable[CPT_C_InitSuccessfulCall].lock() ;
    m_counters [CPT_PD_InitSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_InitSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_TrafficSuccessfulCall].lock() ;
    m_counters [CPT_PD_TrafficSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_TrafficSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_AbortSuccessfulCall].lock() ;
    m_counters [CPT_PD_AbortSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_AbortSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_DefaultSuccessfulCall].lock() ;
    m_counters [CPT_PD_DefaultSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_DefaultSuccessfulCall].unlock() ;
    
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].lock() ;
    m_counters [CPT_PD_FailedCallCannotSendMessage] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].unlock() ;

    m_mutexTable[CPT_C_FailedCallUnexpectedMessage].lock() ;
    m_counters [CPT_PD_FailedCallUnexpectedMessage] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallUnexpectedMessage].unlock() ;

    m_mutexTable[CPT_C_RefusedCall].lock() ;
    m_counters [CPT_PD_RefusedCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_RefusedCall].unlock() ;

    m_mutexTable[CPT_C_FailedCallAborted].lock() ;
    m_counters [CPT_PD_FailedCallAborted] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallAborted].unlock() ;

    m_mutexTable[CPT_C_OutgoingCallCreated].lock() ;
    m_counters [CPT_PD_OutgoingCallCreated] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_OutgoingCallCreated].unlock() ;

    m_mutexTable[CPT_C_FailedCallTimeout].lock() ;
    m_counters [CPT_PD_FailedCallTimeout] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallTimeout].unlock() ;


    m_mutexTable[CPT_PD_NbOfCallUsedForAverageResponseTime].lock() ;
    m_counters [CPT_PD_NbOfCallUsedForAverageResponseTime] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_PD_NbOfCallUsedForAverageResponseTime].unlock() ;

    m_mutexTable[CPT_PD_AverageResponseTime].lock() ;
    m_counters [CPT_PD_AverageResponseTime] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_PD_AverageResponseTime].unlock() ;

    m_PD_sumResponseTime = 0;


    GET_TIME (&m_pdStartTime);
    break;
    
  case E_RESET_PL_COUNTERS :

    m_mutexTable[CPT_C_IncomingCallCreated].lock() ;
    m_counters [CPT_PL_IncomingCallCreated] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_IncomingCallCreated].unlock() ;

    m_mutexTable[CPT_C_MsgSend].lock() ;
    m_counters [CPT_PL_MsgSend] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_MsgSend].unlock() ;

    m_mutexTable[CPT_C_MsgRecv].lock() ;
    m_counters [CPT_PL_MsgRecv] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_MsgRecv].unlock() ;

    m_mutexTable[CPT_C_FailedCall].lock() ;
    m_counters [CPT_PL_FailedCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCall].unlock() ;
    
    m_mutexTable[CPT_C_InitSuccessfulCall].lock() ;
    m_counters [CPT_PL_InitSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_InitSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_TrafficSuccessfulCall].lock() ;
    m_counters [CPT_PL_TrafficSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_TrafficSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_AbortSuccessfulCall].lock() ;
    m_counters [CPT_PL_AbortSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_AbortSuccessfulCall].unlock() ;

    m_mutexTable[CPT_C_DefaultSuccessfulCall].lock() ;
    m_counters [CPT_PL_DefaultSuccessfulCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_DefaultSuccessfulCall].unlock() ;
    
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].lock() ;
    m_counters [CPT_PL_FailedCallCannotSendMessage] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallCannotSendMessage].unlock() ;

    m_mutexTable[CPT_C_FailedCallUnexpectedMessage].lock() ;
    m_counters [CPT_PL_FailedCallUnexpectedMessage] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallUnexpectedMessage].unlock() ;

    m_mutexTable[CPT_C_RefusedCall].lock() ;
    m_counters [CPT_PL_RefusedCall] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_RefusedCall].unlock() ;

    m_mutexTable[CPT_C_FailedCallAborted].lock() ;
    m_counters [CPT_PL_FailedCallAborted] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallAborted].unlock() ;

    m_mutexTable[CPT_C_OutgoingCallCreated].lock() ;
    m_counters [CPT_PL_OutgoingCallCreated] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_OutgoingCallCreated].unlock() ;

    m_mutexTable[CPT_C_FailedCallTimeout].lock() ;
    m_counters [CPT_PL_FailedCallTimeout] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_C_FailedCallTimeout].unlock() ;


    m_mutexTable[CPT_PL_NbOfCallUsedForAverageResponseTime].lock() ;
    m_counters [CPT_PL_NbOfCallUsedForAverageResponseTime] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_PL_NbOfCallUsedForAverageResponseTime].unlock() ;


    m_mutexTable[CPT_PL_AverageResponseTime].lock() ;
    m_counters [CPT_PL_AverageResponseTime] = ZERO_COUNTER_VALUE ;
    m_mutexTable[CPT_PL_AverageResponseTime].unlock() ;
    m_PL_sumResponseTime = 0;


    GET_TIME (&m_plStartTime);
    break;

  case E_CREATE_OUTGOING_CALL :

    m_mutexTable[CPT_C_OutgoingCallCreated].lock() ;
    m_counters [CPT_C_OutgoingCallCreated]++;
    m_counters [CPT_PD_OutgoingCallCreated]++;
    m_counters [CPT_PL_OutgoingCallCreated]++;
    m_mutexTable[CPT_C_OutgoingCallCreated].unlock() ;

    m_mutexTable[CPT_C_CurrentCall].lock();
    m_counters [CPT_C_CurrentCall]++;
    m_mutexTable[CPT_C_CurrentCall].unlock();
    break;
      

  default :
    GEN_FATAL(0,"C_GeneratorStats::executeStatAction() - Unrecognized Action " <<  P_action);
    break ;

  } /* end switch */

  return (0);
}




void C_GeneratorStats::updateAverageCounter(E_CounterName P_AvarageCounter, 
				  E_CounterName P_NbOfCallUsed,
				  unsigned long long* P_sum, 
				  unsigned long P_value)
{
  if (m_counters [P_NbOfCallUsed] <= 0)
  {
    m_counters [P_NbOfCallUsed] ++;
	 m_counters [P_AvarageCounter] = P_value;
	 (*P_sum) = P_value;
  }
  else
  {
    (*P_sum) = P_value + (*P_sum);
    m_counters [P_AvarageCounter] = (*P_sum) /
	 (m_counters [P_NbOfCallUsed] + 1);
    
    m_counters [P_NbOfCallUsed] ++;
  }
}

int C_GeneratorStats::computeStat (E_Action P_action, unsigned long P_value)
{
  switch (P_action)
  {
    case E_ADD_CALL_DURATION :
      // Updating Cumulative Counter
      updateAverageCounter(CPT_C_AverageCallLength, CPT_C_NbOfCallUsedForAverageCallLength,
                           &m_C_sumCallLength, P_value); 
      updateRepartition(m_CallLengthRepartition, m_SizeOfCallLengthRepartition, P_value);
      // Updating Periodical Diplayed counter
      updateAverageCounter(CPT_PD_AverageCallLength, CPT_PD_NbOfCallUsedForAverageCallLength,
                           &m_PD_sumCallLength, P_value); 
      // Updating Periodical Logging counter
      updateAverageCounter(CPT_PL_AverageCallLength, CPT_PL_NbOfCallUsedForAverageCallLength,
                           &m_PL_sumCallLength, P_value); 
      break;



    case E_ADD_RESPONSE_TIME_DURATION :
      // Updating Cumulative Counter

      updateAverageCounter(CPT_C_AverageResponseTime, 
			   CPT_C_NbOfCallUsedForAverageResponseTime,
                           &m_C_sumResponseTime, P_value); 
      updateRepartition(m_ResponseTimeRepartition, 
			m_SizeOfResponseTimeRepartition, P_value);
      // Updating Periodical Diplayed counter
      updateAverageCounter(CPT_PD_AverageResponseTime, 
			   CPT_PD_NbOfCallUsedForAverageResponseTime,
                           &m_PD_sumResponseTime, 
			   P_value); 
      // Updating Periodical Logging counter
      updateAverageCounter(CPT_PL_AverageResponseTime, 
			   CPT_PL_NbOfCallUsedForAverageResponseTime,
                           &m_PL_sumResponseTime, 
			   P_value); 
      break;

    default :
     GEN_FATAL(0,"C_GeneratorStats::ComputeStat() - Unrecognized Action " <<  P_action);
	 } /* end switch */
  return (0);
}


void C_GeneratorStats::updateRepartition( T_dynamicalRepartition* P_tabReport, int P_sizeOfTab, unsigned long P_value)
{
  bool found;
  int i;

  if(P_tabReport != NULL)
  {
    i = P_sizeOfTab-2;
    found = false;
	 while((found == false) && (i>=1))
    {
      if( (P_value < P_tabReport[i].borderMax) &&
          (P_tabReport[i-1].borderMax <= P_value) )
      {
        found = true;
        P_tabReport[i].nbInThisBorder ++;
      }
      i--;
    }
    
    if(!found)
    {
      if(P_value < P_tabReport[0].borderMax)
      {
        P_tabReport[0].nbInThisBorder ++;
      }
      else if(P_value >= P_tabReport[P_sizeOfTab-1].borderMax)
      {
        P_tabReport[P_sizeOfTab-1].nbInThisBorder ++;
      }
      else
      {
        // GEN_ERROR !!!!
        GEN_ERROR(0,"Unable to sort this Value in the repartition table! " << P_value);
      }
    }
  }
}


C_GeneratorStats::C_GeneratorStats ()
{
  size_t L_size = 0;

  

  L_size += strlen(DEFAULT_FILE_NAME) ;
  L_size += strlen(DEFAULT_EXTENSION) ;
  L_size += 1 ;
  ALLOC_TABLE(m_fileName, char*,
	      sizeof(char), L_size);
  strcpy(m_fileName, DEFAULT_FILE_NAME);
  strcat(m_fileName, DEFAULT_EXTENSION);
  m_ResponseTimeRepartition = NULL;
  m_CallLengthRepartition   = NULL;
  m_SizeOfResponseTimeRepartition = 0;
  m_SizeOfCallLengthRepartition   = 0;
  m_initDone = false ;

  m_max_msg_size = 50 ;
  ALLOC_TABLE(m_info_msg, char*, sizeof(char), m_max_msg_size+1);
  ALLOC_TABLE(m_err_msg, char*, sizeof(char), m_max_msg_size+1);
  m_info_msg[0] = '\0' ;
  strcpy(m_err_msg, "No error");
}


C_GeneratorStats::~C_GeneratorStats ()
{
  m_initDone = false ;
  FREE_TABLE(m_info_msg);
  FREE_TABLE(m_err_msg);
}

char* C_GeneratorStats::sRepartitionHeader(T_dynamicalRepartition * tabRepartition, 
				 int sizeOfTab, 
				 char * P_repartitionName)
{
  static char  repartitionHeader[MAX_REPARTITION_HEADER_LENGTH];
  char buffer[MAX_CHAR_BUFFER_SIZE];

  if(tabRepartition != NULL)
  {
    sprintf(repartitionHeader, "%s;", P_repartitionName);
    for(int i=0; i<(sizeOfTab-1); i++) {   
      sprintf(buffer, "<%d;", tabRepartition[i].borderMax);
      strcat(repartitionHeader, buffer);
    }
    sprintf(buffer, ">=%d;", tabRepartition[sizeOfTab-1].borderMax);
    strcat(repartitionHeader, buffer);
  } else {
    sprintf(repartitionHeader, "%s", "");
  }
  
  return(repartitionHeader);
}

char* C_GeneratorStats::sRepartitionInfo(T_dynamicalRepartition * tabRepartition, 
			       int sizeOfTab)
{
  static char repartitionInfo[MAX_REPARTITION_INFO_LENGTH];
  char buffer[MAX_CHAR_BUFFER_SIZE];

  if(tabRepartition != NULL)
  {
    // if a repartition is present, this field match the repartition name
    sprintf(repartitionInfo, ";");
    for(int i=0; i<(sizeOfTab-1); i++)
    {   
       sprintf(buffer, "%ld;", tabRepartition[i].nbInThisBorder);
       strcat(repartitionInfo, buffer);
    }
    sprintf(buffer, "%ld;", tabRepartition[sizeOfTab-1].nbInThisBorder);
    strcat(repartitionInfo, buffer);
  }
  else
  {
    sprintf(repartitionInfo, "%s", "");
  }

  return(repartitionInfo);
}


void C_GeneratorStats::displayRepartition_with_percent(T_dynamicalRepartition * tabRepartition, 
				int sizeOfTab)
{

  int            L_total_nb_border = 0 ;
  int            L_i                   ;
  float          L_percent_traffic     ;
  

  if(tabRepartition != NULL) {
    L_total_nb_border = 0 ;
    for(L_i = 0 ; L_i < sizeOfTab ; L_i++) {   
      L_total_nb_border += tabRepartition[L_i].nbInThisBorder ;
    }
    
    
    L_percent_traffic = (L_total_nb_border > 0 ?
			(100.0*tabRepartition[0].nbInThisBorder)/ (float)L_total_nb_border :
			0.0);

    DISPLAY_REPART_F (0, 
		      tabRepartition[0].borderMax, 
		      tabRepartition[0].nbInThisBorder,
		      L_percent_traffic);

    for(L_i = 1 ; L_i < (sizeOfTab-1); L_i++) {   
      
      L_percent_traffic = (L_total_nb_border > 0 ?
			  (100.0*tabRepartition[L_i].nbInThisBorder)/ (float)L_total_nb_border :
			  0.0);
      
      DISPLAY_REPART_F (tabRepartition[L_i-1].borderMax, 
		      tabRepartition[L_i].borderMax, 
		      tabRepartition[L_i].nbInThisBorder,
		      L_percent_traffic);
    }

    L_percent_traffic = (L_total_nb_border > 0 ?
			(100.0*tabRepartition[sizeOfTab-1].nbInThisBorder)/ (float)L_total_nb_border :
			0.0);


    DISPLAY_LAST_REPART_F (tabRepartition[sizeOfTab-1].borderMax, 
			   tabRepartition[sizeOfTab-1].nbInThisBorder,
			   L_percent_traffic);
  }
  else
  {
    DISPLAY_INFO ("  <No repartion defined>");
  }
}


void C_GeneratorStats::displayRepartition(T_dynamicalRepartition * tabRepartition, 
					  int sizeOfTab)
{
  int  L_i ;

  if(tabRepartition != NULL) {
    
    DISPLAY_REPART (0, 
		    tabRepartition[0].borderMax, 
		    tabRepartition[0].nbInThisBorder);
    
    
    for(L_i = 1 ; L_i < (sizeOfTab-1); L_i++) {   
      
      
      DISPLAY_REPART (tabRepartition[L_i-1].borderMax, 
  		      tabRepartition[L_i].borderMax, 
  		      tabRepartition[L_i].nbInThisBorder);
    }
    
    
    DISPLAY_LAST_REPART (tabRepartition[sizeOfTab-1].borderMax, 
			 tabRepartition[sizeOfTab-1].nbInThisBorder);
  }
  else
    {
      DISPLAY_INFO ("  <No repartion defined>");
    }
}


void C_GeneratorStats::displayRepartition_without_percent(T_dynamicalRepartition * tabRepartition, 
							  int sizeOfTab)
{
  int  L_i ;
  
  if(tabRepartition != NULL) {
    
    DISPLAY_REPART (0, 
		    tabRepartition[0].borderMax, 
		    tabRepartition[0].nbInThisBorder);
    
    
    for(L_i = 1 ; L_i < (sizeOfTab-1); L_i++) {   
      
      
      DISPLAY_REPART (tabRepartition[L_i-1].borderMax, 
  		      tabRepartition[L_i].borderMax, 
  		      tabRepartition[L_i].nbInThisBorder);
    }
    
    
    DISPLAY_LAST_REPART (tabRepartition[sizeOfTab-1].borderMax, 
			 tabRepartition[sizeOfTab-1].nbInThisBorder);
  }
  else
    {
      DISPLAY_INFO ("  <No repartion defined>");
    }
}


void C_GeneratorStats::displayData ()
{
  long   localElapsedTime, globalElapsedTime ;
  struct timeval currentTime;
  float  averageCallRate;
  float  realInstantCallRate;
  unsigned long numberOfCall;

  GET_TIME (&currentTime);
  // computing the real call rate
  globalElapsedTime   = ms_difftime (&currentTime, &m_startTime);
  localElapsedTime    = ms_difftime (&currentTime, &m_pdStartTime);
  // the call rate is for all the call : incoming and outgoing
  numberOfCall        = m_counters[CPT_C_IncomingCallCreated] + m_counters[CPT_C_OutgoingCallCreated];
  averageCallRate     = (globalElapsedTime > 0 ? 1000*(float)numberOfCall/(float)globalElapsedTime : 0.0);
  numberOfCall        = m_counters[CPT_PD_IncomingCallCreated] + m_counters[CPT_PD_OutgoingCallCreated];
  realInstantCallRate = (localElapsedTime  > 0 ? 
                        1000*(float)numberOfCall / (float)localElapsedTime :
                        0.0);

  // display info
  DISPLAY_DLINE ();
  // build and display header info
  DISPLAY_TXT ("Start Time  ", formatTime(&m_startTime));
  DISPLAY_TXT ("Last Reset Time", formatTime(&m_pdStartTime));
  DISPLAY_TXT ("Current Time", formatTime(&currentTime));

  // printing the header in the middle
  DISPLAY_CROSS_LINE();
  DISPLAY_HEADER();
  DISPLAY_CROSS_LINE();

  DISPLAY_TXT_COL ("Elapsed Time", msToHHMMSSmmm(localElapsedTime),   msToHHMMSSmmm(globalElapsedTime));

  DISPLAY_VAL_RATEF_COL ("Call Rate",  realInstantCallRate, averageCallRate);
  DISPLAY_CROSS_LINE ();
 
  DISPLAY_2VAL  ("Incoming call created", m_counters[CPT_PD_IncomingCallCreated],
                                          m_counters[CPT_C_IncomingCallCreated]);

  DISPLAY_2VAL  ("OutGoing call created", m_counters[CPT_PD_OutgoingCallCreated],
                                          m_counters[CPT_C_OutgoingCallCreated]);
  DISPLAY_CUMUL ("Total Call created", m_counters[CPT_C_IncomingCallCreated] +
                                       m_counters[CPT_C_OutgoingCallCreated]);
  DISPLAY_PERIO ("Current Calls",       m_counters[CPT_C_CurrentCall]);
  DISPLAY_CROSS_LINE ();


  DISPLAY_2VAL  ("Successful init call", m_counters[CPT_PD_InitSuccessfulCall], 
		 m_counters[CPT_C_InitSuccessfulCall]);

  DISPLAY_2VAL  ("Successful traffic call", m_counters[CPT_PD_TrafficSuccessfulCall], 
		 m_counters[CPT_C_TrafficSuccessfulCall]);

  DISPLAY_2VAL  ("Successful default call", m_counters[CPT_PD_DefaultSuccessfulCall], 
		 m_counters[CPT_C_DefaultSuccessfulCall]);

  DISPLAY_2VAL  ("Successful abort call", m_counters[CPT_PD_AbortSuccessfulCall], 
		 m_counters[CPT_C_AbortSuccessfulCall]);


  DISPLAY_2VAL  ("Failed call",      m_counters[CPT_PD_FailedCall], m_counters[CPT_C_FailedCall]);


  DISPLAY_CROSS_LINE ();
  DISPLAY_TXT_COL ("Response Time", msToHHMMSSmmm( m_counters [CPT_PD_AverageResponseTime] ), 
                                    msToHHMMSSmmm( m_counters [CPT_C_AverageResponseTime] ));
  DISPLAY_TXT_COL ("Call Length", msToHHMMSSmmm( m_counters [CPT_PD_AverageCallLength] ), 
                                  msToHHMMSSmmm( m_counters [CPT_C_AverageCallLength] ));
  DISPLAY_CROSS_LINE ();

  DISPLAY_INFO("Average Response Time Repartition");
  displayRepartition(m_ResponseTimeRepartition, m_SizeOfResponseTimeRepartition);
  DISPLAY_INFO("Average Call Length Repartition");
  displayRepartition(m_CallLengthRepartition, m_SizeOfCallLengthRepartition);


  DISPLAY_DLINE ();
} /* end of displayData () */


void C_GeneratorStats::displayStat ()
{
  long   localElapsedTime, globalElapsedTime ;
  struct timeval currentTime;

  float   averageTPS;
  float   realInstantTPS;
  unsigned long numberOfTPS;

  GET_TIME (&currentTime);
  // computing the real call rate
  globalElapsedTime   = ms_difftime (&currentTime, &m_startTime);
  localElapsedTime    = ms_difftime (&currentTime, &m_pdStartTime);

  numberOfTPS        = m_counters[CPT_C_MsgRecv] + m_counters[CPT_C_MsgSend];

  averageTPS     = (globalElapsedTime > 0 ? 1000*((float)numberOfTPS/2)/(float)globalElapsedTime : 0.0);

  numberOfTPS        = m_counters[CPT_PD_MsgRecv] + m_counters[CPT_PD_MsgSend];

  // call rate and not tps
  realInstantTPS = (localElapsedTime  > 0 ? 
                     1000*((float)numberOfTPS) / (float)localElapsedTime :
                      0.0);


  // build and display header info
  DISPLAY_TXT ("Start Time  ", formatTime(&m_startTime));
  DISPLAY_TXT ("Last Reset Time", formatTime(&m_pdStartTime));
  DISPLAY_TXT ("Current Time", formatTime(&currentTime));

  // printing the header in the middle
  DISPLAY_CROSS_LINE();
  DISPLAY_HEADER();
  DISPLAY_CROSS_LINE();

  DISPLAY_TXT_COL ("Elapsed Time", msToHHMMSSmmm(localElapsedTime),   msToHHMMSSmmm(globalElapsedTime));


  DISPLAY_VAL_RATEF_TPS ("Call Rate",  realInstantTPS, averageTPS);
  DISPLAY_CROSS_LINE ();

  DISPLAY_2VAL  ("Incoming calls", 
		 m_counters[CPT_PD_IncomingCallCreated],
		 m_counters[CPT_C_IncomingCallCreated]);

  DISPLAY_2VAL  ("Outgoing calls", 
		 m_counters[CPT_PD_OutgoingCallCreated],
		 m_counters[CPT_C_OutgoingCallCreated]);

  
  DISPLAY_2VAL  ("Message Received", m_counters[CPT_PD_MsgRecv],
                                     m_counters[CPT_C_MsgRecv]);

  DISPLAY_2VAL  ("Message Send", m_counters[CPT_PD_MsgSend],
                                     m_counters[CPT_C_MsgSend]);


  DISPLAY_PERIO ("Current Calls",       m_counters[CPT_C_CurrentCall]);
  DISPLAY_CROSS_LINE ();

  DISPLAY_2VAL  ("Successful init call", m_counters[CPT_PD_InitSuccessfulCall], 
		 m_counters[CPT_C_InitSuccessfulCall]);

  DISPLAY_2VAL  ("Successful traffic call", m_counters[CPT_PD_TrafficSuccessfulCall], 
		 m_counters[CPT_C_TrafficSuccessfulCall]);

  DISPLAY_2VAL  ("Successful default call", m_counters[CPT_PD_DefaultSuccessfulCall], 
		 m_counters[CPT_C_DefaultSuccessfulCall]);

  DISPLAY_2VAL  ("Successful abort call", m_counters[CPT_PD_AbortSuccessfulCall], 
		 m_counters[CPT_C_AbortSuccessfulCall]);



  DISPLAY_2VAL  ("Failed call",      m_counters[CPT_PD_FailedCall], m_counters[CPT_C_FailedCall]);

  DISPLAY_CROSS_LINE ();

}



void C_GeneratorStats::activate_percent_traffic () {

  m_current_display_rep_id ++ ;
  
  if (m_current_display_rep_id == m_nb_display_rep_id) {
    m_current_display_rep_id = 0 ;
  }
  m_current_repartition_display 
    = m_current_repartition_table[m_current_display_rep_id];

}

void C_GeneratorStats::reset_cumul_counters () {

  int L_i ;
  
  for (L_i = C_GeneratorStats::CPT_C_InitSuccessfulCall;
       L_i <= C_GeneratorStats::CPT_C_FailedCallTimeout;
       L_i++) {
      m_mutexTable[L_i].lock();
      m_counters[L_i] = (unsigned long) 0;
      m_mutexTable[L_i].unlock();
  }


  if(m_ResponseTimeRepartition != NULL) {
    for(L_i=0; L_i < m_SizeOfResponseTimeRepartition; L_i++) { 
      m_ResponseTimeRepartition[L_i].nbInThisBorder = 0;
    } 
  }

}

void C_GeneratorStats::makeDisplay1 (bool P_display) {

  long           localElapsedTime  ;
  long           globalElapsedTime ;
  struct timeval currentTime       ;

  float          MsgSendPerS;
  float          MsgRecvPerS;
  float          AverageMsgRecvPerS;
  float          AverageMsgSendPerS;
  float          AverageCurrentCallPerS ;

  unsigned long  numberOfCall;
  float          realInstantCallRate ;
  float          averageCallRate ;

  int            L_i             ;
  unsigned long  L_PD_successful_call ;
  unsigned long  L_C_successful_call ;

  


  // CRITICAL SECTION:
  // First of call: copy all accessible counters

  m_mutexTable[CPT_C_IncomingCallCreated].lock() ;
  m_displayCounters[CPT_C_IncomingCallCreated] 
    = m_counters [CPT_C_IncomingCallCreated];
  m_displayCounters[CPT_PD_IncomingCallCreated] 
    = m_counters [CPT_PD_IncomingCallCreated];
  m_mutexTable[CPT_C_IncomingCallCreated].unlock() ;

  m_mutexTable[CPT_C_CurrentCall].lock();
  m_displayCounters[CPT_C_CurrentCall] 
    = m_counters [CPT_C_CurrentCall];
  m_mutexTable[CPT_C_CurrentCall].unlock();

  m_mutexTable[CPT_C_MsgSend].lock() ;
  m_displayCounters[CPT_C_MsgSend] 
    = m_counters [CPT_C_MsgSend];
  m_displayCounters[CPT_PD_MsgSend] 
    = m_counters [CPT_PD_MsgSend];
  m_mutexTable[CPT_C_MsgSend].unlock() ;

  m_mutexTable[CPT_C_MsgRecv].lock() ;
  m_displayCounters[CPT_C_MsgRecv] 
    = m_counters [CPT_C_MsgRecv];
  m_displayCounters[CPT_PD_MsgRecv] 
    = m_counters [CPT_PD_MsgRecv];
  m_mutexTable[CPT_C_MsgRecv].unlock() ;
  
  m_mutexTable[CPT_C_FailedCall].lock() ;
  m_displayCounters[CPT_C_FailedCall] 
    = m_counters [CPT_C_FailedCall];
  m_displayCounters[CPT_PD_FailedCall] 
    = m_counters [CPT_PD_FailedCall];
  m_mutexTable[CPT_C_FailedCall].unlock() ;
    
  m_mutexTable[CPT_C_InitSuccessfulCall].lock() ;
  m_displayCounters[CPT_C_InitSuccessfulCall] 
    = m_counters [CPT_C_InitSuccessfulCall];
  m_displayCounters[CPT_PD_InitSuccessfulCall] 
    = m_counters [CPT_PD_InitSuccessfulCall];
  m_mutexTable[CPT_C_InitSuccessfulCall].unlock() ;

  m_mutexTable[CPT_C_TrafficSuccessfulCall].lock() ;
  m_displayCounters[CPT_C_TrafficSuccessfulCall] 
    = m_counters [CPT_C_TrafficSuccessfulCall];
  m_displayCounters[CPT_PD_TrafficSuccessfulCall] 
    = m_counters [CPT_PD_TrafficSuccessfulCall];
  m_mutexTable[CPT_C_TrafficSuccessfulCall].unlock() ;

  m_mutexTable[CPT_C_DefaultSuccessfulCall].lock() ;
  m_displayCounters[CPT_C_DefaultSuccessfulCall] 
    = m_counters [CPT_C_DefaultSuccessfulCall];
  m_displayCounters[CPT_PD_DefaultSuccessfulCall] 
    = m_counters [CPT_PD_DefaultSuccessfulCall];
  m_mutexTable[CPT_C_DefaultSuccessfulCall].unlock() ;

  m_mutexTable[CPT_C_AbortSuccessfulCall].lock() ;
  m_displayCounters[CPT_C_AbortSuccessfulCall] 
    = m_counters [CPT_C_AbortSuccessfulCall];
  m_displayCounters[CPT_PD_AbortSuccessfulCall] 
    = m_counters [CPT_PD_AbortSuccessfulCall];
  m_mutexTable[CPT_C_AbortSuccessfulCall].unlock() ;

  m_mutexTable[CPT_C_FailedCallCannotSendMessage].lock() ;
  m_displayCounters[CPT_C_FailedCallCannotSendMessage] 
    = m_counters [CPT_C_FailedCallCannotSendMessage];
  m_displayCounters[CPT_PD_FailedCallCannotSendMessage] 
    = m_counters [CPT_PD_FailedCallCannotSendMessage];
  m_mutexTable[CPT_C_FailedCallCannotSendMessage].unlock() ;

  m_mutexTable[CPT_C_FailedCallUnexpectedMessage].lock() ;
  m_displayCounters[CPT_C_FailedCallUnexpectedMessage] 
    = m_counters [CPT_C_FailedCallUnexpectedMessage];
  m_displayCounters[CPT_PD_FailedCallUnexpectedMessage] 
    = m_counters [CPT_PD_FailedCallUnexpectedMessage];
  m_mutexTable[CPT_C_FailedCallUnexpectedMessage].unlock() ;

  m_mutexTable[CPT_C_RefusedCall].lock() ;
  m_displayCounters[CPT_C_RefusedCall] 
    = m_counters [CPT_C_RefusedCall];
  m_displayCounters[CPT_PD_RefusedCall] 
    = m_counters [CPT_PD_RefusedCall];
  m_mutexTable[CPT_C_RefusedCall].unlock() ;

  m_mutexTable[CPT_C_FailedCallAborted].lock() ;
  m_displayCounters[CPT_C_FailedCallAborted] 
    = m_counters [CPT_C_FailedCallAborted];
  m_displayCounters[CPT_PD_FailedCallAborted] 
    = m_counters [CPT_PD_FailedCallAborted];
  m_mutexTable[CPT_C_FailedCallAborted].unlock() ;

  m_mutexTable[CPT_C_FailedCallTimeout].lock() ;
  m_displayCounters[CPT_C_FailedCallTimeout] 
    = m_counters [CPT_C_FailedCallTimeout];
  m_displayCounters[CPT_PD_FailedCallTimeout] 
    = m_counters [CPT_PD_FailedCallTimeout];
  m_mutexTable[CPT_C_FailedCallTimeout].unlock() ;

  m_mutexTable[CPT_C_OutgoingCallCreated].lock() ;
  m_displayCounters[CPT_C_OutgoingCallCreated] 
    = m_counters [CPT_C_OutgoingCallCreated];
  m_displayCounters[CPT_PD_OutgoingCallCreated] 
    = m_counters [CPT_PD_OutgoingCallCreated];
  m_mutexTable[CPT_C_OutgoingCallCreated].unlock() ;

  // END CRITICAL SECTION

  GET_TIME (&currentTime);
  // computing the real call rate
  globalElapsedTime   = ms_difftime (&currentTime, &m_startTime);
  localElapsedTime    = ms_difftime (&currentTime, &m_pdStartTime);


  numberOfCall        
    = m_displayCounters[CPT_C_IncomingCallCreated] 
    + m_displayCounters[CPT_C_OutgoingCallCreated];

  averageCallRate     
    = (globalElapsedTime > 0 ? 
       1000*(float)numberOfCall/(float)globalElapsedTime : 0.0);

  numberOfCall        
    = m_displayCounters[CPT_PD_IncomingCallCreated] 
    + m_displayCounters[CPT_PD_OutgoingCallCreated];

  realInstantCallRate 
    = (localElapsedTime  > 0 ? 
       1000*(float)numberOfCall / (float)localElapsedTime :
       0.0);

  MsgRecvPerS = (localElapsedTime  > 0 ? 
		    1000*((float)m_displayCounters[CPT_PD_MsgRecv]) / (float)localElapsedTime :
		    0.0);

  MsgSendPerS = (localElapsedTime  > 0 ? 
		    1000*((float)m_displayCounters[CPT_PD_MsgSend]) / (float)localElapsedTime :
		    0.0);

  AverageMsgRecvPerS = (globalElapsedTime  > 0 ? 
		    1000*((float)m_displayCounters[CPT_C_MsgRecv]) / (float)globalElapsedTime :
		    0.0);

  AverageMsgSendPerS = (globalElapsedTime  > 0 ? 
		    1000*((float)m_displayCounters[CPT_C_MsgSend]) / (float)globalElapsedTime :
		    0.0);

  AverageCurrentCallPerS = (globalElapsedTime  > 0 ? 
			    1000*((float)m_displayCounters[CPT_C_CurrentCall]) / (float)globalElapsedTime :
			    0.0);

  
  L_PD_successful_call = 
    m_displayCounters[CPT_PD_InitSuccessfulCall] +
    m_displayCounters[CPT_PD_TrafficSuccessfulCall] +
    m_displayCounters[CPT_PD_DefaultSuccessfulCall] +
    m_displayCounters[CPT_PD_AbortSuccessfulCall] ;

  L_C_successful_call = 
    m_displayCounters[CPT_C_InitSuccessfulCall] +
    m_displayCounters[CPT_C_TrafficSuccessfulCall] +
    m_displayCounters[CPT_C_DefaultSuccessfulCall] +
    m_displayCounters[CPT_C_AbortSuccessfulCall] ;


    

  if (P_display) {
    DISPLAY_CROSS_LINE ();
    
    {
      char L_start_time[TIME_LENGTH];
      char L_current_time[TIME_LENGTH];
      formatTime(L_start_time, &m_startTime);
      formatTime(L_current_time, &currentTime);
      DISPLAY_3TXT ("Start/Current Time", 
		    L_start_time, 
		    L_current_time);
    }

    // printing the header in the middle
    DISPLAY_CROSS_LINE();
    DISPLAY_HEADER();
    
    DISPLAY_CROSS_LINE();
    
    DISPLAY_TXT_COL ("Elapsed Time", msToHHMMSSmmm(localElapsedTime),   msToHHMMSSmmm(globalElapsedTime));
    
    DISPLAY_VAL_RATEF_TPS ("Call rate (/s)",  realInstantCallRate, averageCallRate);
    DISPLAY_CROSS_LINE ();
    
    DISPLAY_2VAL  ("Incoming calls", 
		   m_displayCounters[CPT_PD_IncomingCallCreated],
		   m_displayCounters[CPT_C_IncomingCallCreated]);
    
    DISPLAY_2VAL  ("Outgoing calls", 
		   m_displayCounters[CPT_PD_OutgoingCallCreated],
		   m_displayCounters[CPT_C_OutgoingCallCreated]);
    
    
    DISPLAY_2VAL_RATEF ( "Msg Recv/s" ,
			 MsgRecvPerS,
			 AverageMsgRecvPerS);
    
    DISPLAY_2VAL_RATEF ( "Msg Sent/s" ,
			 MsgSendPerS,
			 AverageMsgSendPerS);
    
    DISPLAY_2VAL  ("Unexpected msg",      
		   m_displayCounters[CPT_PD_FailedCallUnexpectedMessage], 
		   m_displayCounters[CPT_C_FailedCallUnexpectedMessage]);
    
    DISPLAY_2VAL_CURRENTF ("Current calls",       
			   m_displayCounters[CPT_C_CurrentCall],
			   AverageCurrentCallPerS);
    
    DISPLAY_CROSS_LINE ();
    

    //    DISPLAY_2VAL  ("Successful calls", 
    //  		 m_displayCounters[CPT_PD_SuccessfulCall], 
    //  		 m_displayCounters[CPT_C_SuccessfulCall]);
    
    //    char L_values_periodic[100], L_values_cumulated[100];
    
    //    sprintf(L_values_periodic, "%ld/%ld/%ld/%ld", 
    //  	  m_displayCounters[CPT_PD_InitSuccessfulCall],
    //  	  m_displayCounters[CPT_PD_TrafficSuccessfulCall],
    //  	  m_displayCounters[CPT_PD_DefaultSuccessfulCall],
    //  	  m_displayCounters[CPT_PD_AbortSuccessfulCall]);
    //    sprintf(L_values_cumulated, "%ld/%ld/%ld/%ld", 
    //  	  m_displayCounters[CPT_C_InitSuccessfulCall],
    //  	  m_displayCounters[CPT_C_TrafficSuccessfulCall],
    //  	  m_displayCounters[CPT_C_DefaultSuccessfulCall],
    //  	  m_displayCounters[CPT_C_AbortSuccessfulCall]);
    
    //    DISPLAY_3TXT  ("Success calls(i/t/d/a)", 
    //  		 L_values_periodic,
    //  		 L_values_cumulated);
    

    DISPLAY_2VAL  ("Successful calls", 
		   L_PD_successful_call,
		   L_C_successful_call);

    
    DISPLAY_2VAL  ("Failed calls",      
		   m_displayCounters[CPT_PD_FailedCall], 
		   m_displayCounters[CPT_C_FailedCall]);
    
    DISPLAY_2VAL  ("Refused calls",      
		   m_displayCounters[CPT_PD_RefusedCall], 
		   m_displayCounters[CPT_C_RefusedCall]);
    
    DISPLAY_2VAL  ("Aborted calls",      
		   m_displayCounters[CPT_PD_FailedCallAborted], 
		   m_displayCounters[CPT_C_FailedCallAborted]);
    
    DISPLAY_2VAL  ("Timeout calls",      
		   m_displayCounters[CPT_PD_FailedCallTimeout], 
		   m_displayCounters[CPT_C_FailedCallTimeout]);
    
    DISPLAY_CROSS_LINE ();
    DISPLAY_TXT("Last Info", m_info_msg);
    DISPLAY_TXT("Last Error", m_err_msg);
    
    printf("|--- Next screen : Press key 1 ----------------------- [h]: Display help ------|\r\n");
    
  } else {
    DISPLAY_CROSS_LINE ();

    //    DISPLAY_CUMUL ("Total Call created", 
    //	   m_counters[CPT_C_IncomingCallCreated] +
    //	   m_counters[CPT_C_OutgoingCallCreated]);

    DISPLAY_2VAL  ("Success init calls", 
		   m_displayCounters[CPT_PD_InitSuccessfulCall], 
		   m_displayCounters[CPT_C_InitSuccessfulCall]);
    
    
    DISPLAY_2VAL  ("Success traffic calls", 
		   m_displayCounters[CPT_PD_TrafficSuccessfulCall], 
		   m_displayCounters[CPT_C_TrafficSuccessfulCall]);
    
    DISPLAY_2VAL  ("Success default calls", 
		   m_displayCounters[CPT_PD_DefaultSuccessfulCall], 
		   m_displayCounters[CPT_C_DefaultSuccessfulCall]);
    
    DISPLAY_2VAL  ("Success abort calls", 
		   m_displayCounters[CPT_PD_AbortSuccessfulCall], 
		   m_displayCounters[CPT_C_AbortSuccessfulCall]);

    
    
    DISPLAY_CROSS_LINE ();
    
    for (L_i = 0 ; L_i < 17 ; L_i ++) {
      DISPLAY_INFO((char*)"") ;
    }
    printf("|--- Next screen : Press key 1 ----------------------- [h]: Display help ------|\r\n");

  }
}

void C_GeneratorStats::makeDisplay2 () {

  int L_i ;

  m_mutexTable[CPT_C_AverageResponseTime].lock() ;
  m_displayCounters[CPT_C_AverageResponseTime]
    = m_counters[CPT_C_AverageResponseTime];
  m_displayCounters[CPT_PD_AverageResponseTime]
    = m_counters[CPT_PD_AverageResponseTime];
  m_mutexTable[CPT_C_AverageResponseTime].unlock() ;

  DISPLAY_CROSS_LINE ();
  DISPLAY_INFO("Start/Stop timer repartition");
  DISPLAY_CROSS_LINE ();
  DISPLAY_TXT_COL ("Time (Periodic/Average)", 
		   msToHHMMSSmmm( m_displayCounters [CPT_PD_AverageResponseTime] ), 
		   msToHHMMSSmmm( m_displayCounters [CPT_C_AverageResponseTime] ) );
  DISPLAY_CROSS_LINE ();
  m_mutexTable[CPT_C_AverageResponseTime].lock() ;
  // displayRepartition_with_percent(m_ResponseTimeRepartition, m_SizeOfResponseTimeRepartition);

  ((this)->*(m_current_repartition_display))
    (m_ResponseTimeRepartition, m_SizeOfResponseTimeRepartition);


  m_mutexTable[CPT_C_AverageResponseTime].unlock() ;
  DISPLAY_CROSS_LINE ();
  for (L_i = 0 ; L_i < (17 - m_SizeOfResponseTimeRepartition); L_i++) {
    DISPLAY_INFO((char*)"") ;
  }
  DISPLAY_CROSS_LINE ();

}

void C_GeneratorStats::displayRepartition ()
{
  DISPLAY_INFO("Average Response Time Repartition");
  displayRepartition(m_ResponseTimeRepartition, m_SizeOfResponseTimeRepartition);
  DISPLAY_INFO("Average Call Length Repartition");
  displayRepartition(m_CallLengthRepartition, m_SizeOfCallLengthRepartition);
}



void C_GeneratorStats::dumpData ()
{
  long   localElapsedTime, globalElapsedTime ;
  struct timeval currentTime;
  float  averageCallRate;
  float  realInstantCallRate;
  unsigned long numberOfCall;

  // computing the real call rate
  GET_TIME (&currentTime);
  globalElapsedTime   = ms_difftime (&currentTime, &m_startTime);
  localElapsedTime    = ms_difftime (&currentTime, &m_plStartTime);
  // the call rate is for all the call : incoming and outgoing
  numberOfCall        = m_counters[CPT_C_IncomingCallCreated] + m_counters[CPT_C_OutgoingCallCreated];
  averageCallRate     = (globalElapsedTime > 0 ? 1000*(float)numberOfCall/(float)globalElapsedTime : 0.0);
  numberOfCall        = m_counters[CPT_PL_IncomingCallCreated] + m_counters[CPT_PL_OutgoingCallCreated];
  realInstantCallRate = (localElapsedTime  > 0 ? 
                        1000*(float)numberOfCall / (float)localElapsedTime :
                        0.0);


  if(m_outputStream == NULL)
  {
    // if the file is still not opened, we opened it now
    NEW_VAR(m_outputStream,
	    fstream_output(m_fileName));
    m_headerAlreadyDisplayed = false;
    
    if(m_outputStream == NULL) {
      iostream_error << "Unable to open log file !" << iostream_endl;
      exit(-1);
    }
 
    if(!m_outputStream->good()) {
      iostream_error << "Unable to open log file !" << iostream_endl;
      exit(-1);
	 }
  }

  if(m_headerAlreadyDisplayed == false)
  {
    // header - it's dump in file only one time at the beginning of the file
    (*m_outputStream) << "StartTime;"
                      << "LastResetTime;"
                      << "CurrentTime;" 
                      << "ElapsedTime(P);"
                      << "ElapsedTime(C);"
                      << "CallRate(P);"
                      << "CallRate(C);"
                      << "IncomingCall(P);"
                      << "IncomingCall(C);"
                      << "OutgoingCall(P);"
                      << "OutgoingCall(C);"
                      << "TotalCallCreated;"
                      << "CurrentCall;"
                      << "InitSuccessfulCall(P);"
                      << "InitSuccessfulCall(C);"
                      << "TrafficSuccessfulCall(P);"
                      << "TrafficSuccessfulCall(C);"
                      << "DefaultSuccessfulCall(P);"
                      << "DefaultSuccessfulCall(C);"
                      << "AbortSuccessfulCall(P);"
                      << "AbortSuccessfulCall(C);"
                      << "FailedCall(P);"
                      << "FailedCall(C);"
                      << "FailedCannotSendMessage(P);"
                      << "FailedCannotSendMessage(C);"
                      << "FailedMaxUDPRetrans(P);"
                      << "FailedMaxUDPRetrans(C);"
                      << "FailedUnexpectedMessage(P);"
                      << "FailedUnexpectedMessage(C);"
                      << "FailedCallRejected(P);"
                      << "FailedCallRejected(C);"
                      << "FailedCmdNotSent(P);"
                      << "FailedCmdNotSent(C);"
                      << "FailedRegexpDoesntMatch(P);"
                      << "FailedRegexpDoesntMatch(C);"
                      << "FailedRegexpHdrNotFound(P);"
                      << "FailedRegexpHdrNotFound(C);"
                      << "ResponseTime(P);"
                      << "ResponseTime(C);"
                      << "CallLength(P);"
                      << "CallLength(C);";
    (*m_outputStream) << sRepartitionHeader(m_ResponseTimeRepartition, m_SizeOfResponseTimeRepartition,
                                            (char*) "ResponseTimeRepartition");
    (*m_outputStream) << sRepartitionHeader(m_CallLengthRepartition, m_SizeOfCallLengthRepartition,
                                            (char*) "CallLengthRepartition");
    (*m_outputStream) << iostream_endl;
    m_headerAlreadyDisplayed = true;
  }

  // content
  (*m_outputStream) << formatTime(&m_startTime)               << ";";
  (*m_outputStream) << formatTime(&m_plStartTime)             << ";";
  (*m_outputStream) << formatTime(&currentTime)               << ";"
                    << msToHHMMSS(localElapsedTime)           << ";";
  (*m_outputStream) << msToHHMMSS(globalElapsedTime)          << ";"
                    << realInstantCallRate                    << ";"
                    << averageCallRate                        << ";"
                    << m_counters[CPT_PL_IncomingCallCreated] << ";"
                    << m_counters[CPT_C_IncomingCallCreated]  << ";"
                    << m_counters[CPT_PL_OutgoingCallCreated] << ";"
                    << m_counters[CPT_C_OutgoingCallCreated]  << ";"
                    << m_counters[CPT_C_IncomingCallCreated]+ 
                       m_counters[CPT_C_OutgoingCallCreated]  << ";"
                    << m_counters[CPT_C_CurrentCall]          << ";"
                    << m_counters[CPT_PL_InitSuccessfulCall]     << ";"
                    << m_counters[CPT_C_InitSuccessfulCall]      << ";"
                    << m_counters[CPT_PL_TrafficSuccessfulCall]     << ";"
                    << m_counters[CPT_C_TrafficSuccessfulCall]      << ";"
                    << m_counters[CPT_PL_DefaultSuccessfulCall]     << ";"
                    << m_counters[CPT_C_DefaultSuccessfulCall]      << ";"
                    << m_counters[CPT_PL_AbortSuccessfulCall]     << ";"
                    << m_counters[CPT_C_AbortSuccessfulCall]      << ";"
                    << m_counters[CPT_PL_FailedCall]          << ";"
                    << m_counters[CPT_C_FailedCall]           << ";"
                    << m_counters[CPT_PL_FailedCallCannotSendMessage]   << ";"
                    << m_counters[CPT_C_FailedCallCannotSendMessage]   << ";"
                    << m_counters[CPT_PL_FailedCallMaxUdpRetrans]   << ";"
                    << m_counters[CPT_C_FailedCallMaxUdpRetrans]   << ";"
                    << m_counters[CPT_PL_FailedCallUnexpectedMessage]   << ";"
                    << m_counters[CPT_C_FailedCallUnexpectedMessage]   << ";"
                    << m_counters[CPT_PL_FailedCallCallRejected]   << ";"
                    << m_counters[CPT_C_FailedCallCallRejected]   << ";"
                    << m_counters[CPT_PL_FailedCallCmdNotSent]   << ";"
                    << m_counters[CPT_C_FailedCallCmdNotSent]   << ";"
                    << m_counters[CPT_PL_FailedCallRegexpDoesntMatch]   << ";"
                    << m_counters[CPT_C_FailedCallRegexpDoesntMatch]   << ";"
                    << m_counters[CPT_PL_FailedCallRegexpHdrNotFound]   << ";"
                    << m_counters[CPT_C_FailedCallRegexpHdrNotFound]   << ";";
                    // SF917289 << m_counters[CPT_C_UnexpectedMessage]    << ";";
  (*m_outputStream) << msToHHMMSSmmm( m_counters [CPT_PL_AverageResponseTime] ) << ";";
  (*m_outputStream) << msToHHMMSSmmm( m_counters [CPT_C_AverageResponseTime ] ) << ";";
  (*m_outputStream) << msToHHMMSSmmm( m_counters [CPT_PL_AverageCallLength  ] ) << ";";
  (*m_outputStream) << msToHHMMSSmmm( m_counters [CPT_C_AverageCallLength   ] ) << ";";
  (*m_outputStream) << sRepartitionInfo(m_ResponseTimeRepartition, m_SizeOfResponseTimeRepartition);
  (*m_outputStream) << sRepartitionInfo(m_CallLengthRepartition, m_SizeOfCallLengthRepartition);
  (*m_outputStream) << iostream_endl;

  // flushing the output file to let the tail -f working !
  (*m_outputStream).flush();

} /* end of logData () */





void C_GeneratorStats::makeLog ()
{

  long           localElapsedTime  ;
  long           globalElapsedTime ;
  struct timeval currentTime       ;

  float          MsgSendPerS;
  float          MsgRecvPerS;
  float          AverageMsgRecvPerS;
  float          AverageMsgSendPerS;

  unsigned long  numberOfCall;
  float          realInstantCallRate ;
  float          averageCallRate ;



  // CRITICAL SECTION:
  // First of call: copy all accessible counters

  m_mutexTable[CPT_C_IncomingCallCreated].lock() ;
  m_loggingCounters[CPT_C_IncomingCallCreated] 
    = m_counters [CPT_C_IncomingCallCreated];
  m_loggingCounters[CPT_PL_IncomingCallCreated] 
    = m_counters [CPT_PL_IncomingCallCreated];
  m_mutexTable[CPT_C_IncomingCallCreated].unlock() ;

  m_mutexTable[CPT_C_CurrentCall].lock();
  m_loggingCounters[CPT_C_CurrentCall] 
    = m_counters [CPT_C_CurrentCall];
  m_mutexTable[CPT_C_CurrentCall].unlock();

  m_mutexTable[CPT_C_MsgSend].lock() ;
  m_loggingCounters[CPT_C_MsgSend] 
    = m_counters [CPT_C_MsgSend];
  m_loggingCounters[CPT_PL_MsgSend] 
    = m_counters [CPT_PL_MsgSend];
  m_mutexTable[CPT_C_MsgSend].unlock() ;

  m_mutexTable[CPT_C_MsgRecv].lock() ;
  m_loggingCounters[CPT_C_MsgRecv] 
    = m_counters [CPT_C_MsgRecv];
  m_loggingCounters[CPT_PL_MsgRecv] 
    = m_counters [CPT_PL_MsgRecv];
  m_mutexTable[CPT_C_MsgRecv].unlock() ;
  
  m_mutexTable[CPT_C_FailedCall].lock() ;
  m_loggingCounters[CPT_C_FailedCall] 
    = m_counters [CPT_C_FailedCall];
  m_loggingCounters[CPT_PL_FailedCall] 
    = m_counters [CPT_PL_FailedCall];
  m_mutexTable[CPT_C_FailedCall].unlock() ;

  m_mutexTable[CPT_C_InitSuccessfulCall].lock() ;
  m_loggingCounters[CPT_C_InitSuccessfulCall] 
    = m_counters [CPT_C_InitSuccessfulCall];
  m_loggingCounters[CPT_PL_InitSuccessfulCall] 
    = m_counters [CPT_PL_InitSuccessfulCall];
  m_mutexTable[CPT_C_InitSuccessfulCall].unlock() ;

  m_mutexTable[CPT_C_TrafficSuccessfulCall].lock() ;
  m_loggingCounters[CPT_C_TrafficSuccessfulCall] 
    = m_counters [CPT_C_TrafficSuccessfulCall];
  m_loggingCounters[CPT_PL_TrafficSuccessfulCall] 
    = m_counters [CPT_PL_TrafficSuccessfulCall];
  m_mutexTable[CPT_C_TrafficSuccessfulCall].unlock() ;

  m_mutexTable[CPT_C_DefaultSuccessfulCall].lock() ;
  m_loggingCounters[CPT_C_DefaultSuccessfulCall] 
    = m_counters [CPT_C_DefaultSuccessfulCall];
  m_loggingCounters[CPT_PL_DefaultSuccessfulCall] 
    = m_counters [CPT_PL_DefaultSuccessfulCall];
  m_mutexTable[CPT_C_DefaultSuccessfulCall].unlock() ;

  m_mutexTable[CPT_C_AbortSuccessfulCall].lock() ;
  m_loggingCounters[CPT_C_AbortSuccessfulCall] 
    = m_counters [CPT_C_AbortSuccessfulCall];
  m_loggingCounters[CPT_PL_AbortSuccessfulCall] 
    = m_counters [CPT_PL_AbortSuccessfulCall];
  m_mutexTable[CPT_C_AbortSuccessfulCall].unlock() ;
    
  m_mutexTable[CPT_C_FailedCallCannotSendMessage].lock() ;
  m_loggingCounters[CPT_C_FailedCallCannotSendMessage] 
    = m_counters [CPT_C_FailedCallCannotSendMessage];
  m_loggingCounters[CPT_PL_FailedCallCannotSendMessage] 
    = m_counters [CPT_PL_FailedCallCannotSendMessage];
  m_mutexTable[CPT_C_FailedCallCannotSendMessage].unlock() ;

  m_mutexTable[CPT_C_FailedCallUnexpectedMessage].lock() ;
  m_loggingCounters[CPT_C_FailedCallUnexpectedMessage] 
    = m_counters [CPT_C_FailedCallUnexpectedMessage];
  m_loggingCounters[CPT_PL_FailedCallUnexpectedMessage] 
    = m_counters [CPT_PL_FailedCallUnexpectedMessage];
  m_mutexTable[CPT_C_FailedCallUnexpectedMessage].unlock() ;

  m_mutexTable[CPT_C_RefusedCall].lock() ;
  m_loggingCounters[CPT_C_RefusedCall] 
    = m_counters [CPT_C_RefusedCall];
  m_loggingCounters[CPT_PL_RefusedCall] 
    = m_counters [CPT_PL_RefusedCall];
  m_mutexTable[CPT_C_RefusedCall].unlock() ;

  m_mutexTable[CPT_C_FailedCallAborted].lock() ;
  m_loggingCounters[CPT_C_FailedCallAborted] 
    = m_counters [CPT_C_FailedCallAborted];
  m_loggingCounters[CPT_PL_FailedCallAborted] 
    = m_counters [CPT_PL_FailedCallAborted];
  m_mutexTable[CPT_C_FailedCallAborted].unlock() ;

  m_mutexTable[CPT_C_FailedCallTimeout].lock() ;
  m_loggingCounters[CPT_C_FailedCallTimeout] 
    = m_counters [CPT_C_FailedCallTimeout];
  m_loggingCounters[CPT_PL_FailedCallTimeout] 
    = m_counters [CPT_PL_FailedCallTimeout];
  m_mutexTable[CPT_C_FailedCallTimeout].unlock() ;

  m_mutexTable[CPT_C_OutgoingCallCreated].lock() ;
  m_loggingCounters[CPT_C_OutgoingCallCreated] 
    = m_counters [CPT_C_OutgoingCallCreated];
  m_loggingCounters[CPT_PL_OutgoingCallCreated] 
    = m_counters [CPT_PL_OutgoingCallCreated];
  m_mutexTable[CPT_C_OutgoingCallCreated].unlock() ;

  // END CRITICAL SECTION

  GET_TIME (&currentTime);
  // computing the real call rate
  globalElapsedTime   = ms_difftime (&currentTime, &m_startTime);
  localElapsedTime    = ms_difftime (&currentTime, &m_plStartTime);


  numberOfCall        
    = m_loggingCounters[CPT_C_IncomingCallCreated] 
    + m_loggingCounters[CPT_C_OutgoingCallCreated];

  averageCallRate     
    = (globalElapsedTime > 0 ? 
       1000*(float)numberOfCall/(float)globalElapsedTime : 0.0);

  numberOfCall        
    = m_loggingCounters[CPT_PL_IncomingCallCreated] 
    + m_loggingCounters[CPT_PL_OutgoingCallCreated];

  realInstantCallRate 
    = (localElapsedTime  > 0 ? 
       1000*(float)numberOfCall / (float)localElapsedTime :
       0.0);

  MsgRecvPerS = (localElapsedTime  > 0 ? 
		    1000*((float)m_loggingCounters[CPT_PL_MsgRecv]) / (float)localElapsedTime :
		    0.0);

  MsgSendPerS = (localElapsedTime  > 0 ? 
		    1000*((float)m_loggingCounters[CPT_PL_MsgSend]) / (float)localElapsedTime :
		    0.0);

  AverageMsgRecvPerS = (globalElapsedTime  > 0 ? 
		    1000*((float)m_loggingCounters[CPT_C_MsgRecv]) / (float)globalElapsedTime :
		    0.0);

  AverageMsgSendPerS = (globalElapsedTime  > 0 ? 
		    1000*((float)m_loggingCounters[CPT_C_MsgSend]) / (float)globalElapsedTime :
		    0.0);




  // content
  (*m_outputStream) << formatTime(&m_startTime)                                << ";";
  (*m_outputStream) << formatTime(&m_pdStartTime)                              << ";";
  (*m_outputStream) << formatTime(&currentTime)                                << ";"
                    << msToHHMMSS(localElapsedTime)                            << ";";
  (*m_outputStream) << msToHHMMSS(globalElapsedTime)                           << ";"
                    << realInstantCallRate                                     << ";"
                    << averageCallRate                                         << ";"
                    << m_loggingCounters[CPT_PL_IncomingCallCreated]           << ";"
                    << m_loggingCounters[CPT_C_IncomingCallCreated]            << ";"
                    << m_loggingCounters[CPT_PL_OutgoingCallCreated]           << ";"
                    << m_loggingCounters[CPT_C_OutgoingCallCreated]            << ";"
		    << MsgRecvPerS                                             << ";"
		    << AverageMsgRecvPerS                                      << ";"
		    << MsgSendPerS                                             << ";"
		    << AverageMsgSendPerS                                      << ";"
                    << m_loggingCounters[CPT_PL_FailedCallUnexpectedMessage]   << ";"
                    << m_loggingCounters[CPT_C_FailedCallUnexpectedMessage]    << ";"
		    << m_loggingCounters[CPT_C_CurrentCall]                    << ";"
                    << m_loggingCounters[CPT_PL_InitSuccessfulCall]                << ";"
                    << m_loggingCounters[CPT_C_InitSuccessfulCall]                 << ";"
                    << m_loggingCounters[CPT_PL_TrafficSuccessfulCall]                << ";"
                    << m_loggingCounters[CPT_C_TrafficSuccessfulCall]                 << ";"
                    << m_loggingCounters[CPT_PL_DefaultSuccessfulCall]                << ";"
                    << m_loggingCounters[CPT_C_DefaultSuccessfulCall]                 << ";"
                    << m_loggingCounters[CPT_PL_AbortSuccessfulCall]                << ";"
                    << m_loggingCounters[CPT_C_AbortSuccessfulCall]                 << ";"
                    << m_loggingCounters[CPT_PL_FailedCall]                    << ";"
                    << m_loggingCounters[CPT_C_FailedCall]                     << ";"
                    << m_loggingCounters[CPT_PL_RefusedCall]                   << ";"
                    << m_loggingCounters[CPT_C_RefusedCall]                    << ";"
                    << m_loggingCounters[CPT_PL_FailedCallAborted]             << ";"
                    << m_loggingCounters[CPT_C_FailedCallAborted]              << ";"
                    << m_loggingCounters[CPT_PL_FailedCallTimeout]             << ";"
                    << m_loggingCounters[CPT_C_FailedCallTimeout]              << ";" ;
  m_mutexTable[CPT_C_AverageResponseTime].lock() ;
  (*m_outputStream) << msToHHMMSSmmm( m_counters [CPT_PL_AverageResponseTime] ) << ";" ;
  (*m_outputStream) << msToHHMMSSmmm( m_counters [CPT_C_AverageResponseTime ] ) << ";" ;
  (*m_outputStream) << sRepartitionInfo(m_ResponseTimeRepartition, m_SizeOfResponseTimeRepartition);
  m_mutexTable[CPT_C_AverageResponseTime].unlock() ;


  (*m_outputStream) << iostream_endl;

  // flushing the output file to let the tail -f working !
  (*m_outputStream).flush();

} /* end of logData () */



void C_GeneratorStats::makeFirstLog ()
{
 
  if(m_outputStream == NULL)
  {
    // if the file is still not opened, we opened it now
    NEW_VAR(m_outputStream,
	    fstream_output(m_fileName));
    m_headerAlreadyDisplayed = false;
    
    if(m_outputStream == NULL) {
      iostream_error << "Unable to open log file !" << iostream_endl;
      exit(-1);
    }
 
    if(!m_outputStream->good()) {
      iostream_error << "Unable to open log file !" << iostream_endl;
      exit(-1);
    }
  }

  // header - it's dump in file only one time at the beginning of the file
  (*m_outputStream) << "StartTime;"
		    << "LastResetTime;"
		    << "CurrentTime;" 
		    << "ElapsedTime(P);"
		    << "ElapsedTime(C);"
		    << "Rate(P);"
		    << "Rate(C);"
		    << "IncomingCall(P);"
		    << "IncomingCall(C);"
		    << "OutgoingCall(P);"
		    << "OutgoingCall(C);"
		    << "MsgRecvPerS(P);"
		    << "MsgRecvPerS(C);"
		    << "MsgSendPerS(P);"
		    << "MsgSendPerS(C);"
		    << "UnexpectedMsg(P);"
		    << "UnexpectedMsg(C);"
		    << "CurrentCall;"
		    << "InitSuccessfulCall(P);"
		    << "InitSuccessfulCall(C);"
		    << "TrafficSuccessfulCall(P);"
		    << "TrafficSuccessfulCall(C);"
		    << "DefaultSuccessfulCall(P);"
		    << "DefaultSuccessfulCall(C);"
		    << "AbortSuccessfulCall(P);"
		    << "AbortSuccessfulCall(C);"
		    << "FailedCall(P);"
		    << "FailedCall(C);"
		    << "FailedRefused(P);"
		    << "FailedRefused(C);"
		    << "FailedAborted(P);"
		    << "FailedAborted(C);"
		    << "FailedTimeout(P);"
		    << "FailedTimeout(C);"
		    << "ResponseTime(P);"
		    << "ResponseTime(C);" ;
  (*m_outputStream) << sRepartitionHeader(m_ResponseTimeRepartition, 
					  m_SizeOfResponseTimeRepartition,
					  (char*) "ResponseTimeRepartition");
  (*m_outputStream) << iostream_endl;
  // flushing the output file to let the tail -f working !
  (*m_outputStream).flush();

}





/* Time Gestion */
char* C_GeneratorStats::msToHHMMSS (unsigned long P_ms)
{
	static char L_time [TIME_LENGTH];
	unsigned long hh, mm, ss;
	
	P_ms = P_ms / 1000;
	hh = P_ms / 3600;
	mm = (P_ms - hh * 3600) / 60;
	ss = P_ms - (hh * 3600) - (mm * 60);
	sprintf (L_time, "%2.2ld:%2.2ld:%2.2ld", hh, mm, ss);
	return (L_time);
} /* end of msToHHMMSS */

char* C_GeneratorStats::msToHHMMSSmmm (unsigned long P_ms)
{
	static char L_time [TIME_LENGTH];
	unsigned long sec, hh, mm, ss, mmm;
	
	sec  = P_ms / 1000;
	hh   = sec / 3600;
	mm   = (sec - hh * 3600) / 60;
	ss   = sec - (hh * 3600) - (mm * 60);
   mmm  = P_ms - (hh * 3600000) - (mm * 60000) - (ss*1000);
	sprintf (L_time, "%2.2ld:%2.2ld:%2.2ld:%3.3ld", hh, mm, ss, mmm);
	return (L_time);
} /* end of msToHHMMSS */


void C_GeneratorStats::formatTime (char *P_time, struct timeval* P_tv)
{
	struct tm * L_currentDate;

	// Get the current date and time
	L_currentDate = localtime ((const time_t *)&P_tv->tv_sec);

	// Format the time
	if (L_currentDate == NULL)
	{
		memset (P_time, 0, TIME_LENGTH);
	} 
   else
	{
		sprintf(P_time, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", 
			L_currentDate->tm_year + 1900,
			L_currentDate->tm_mon + 1,
			L_currentDate->tm_mday,
			L_currentDate->tm_hour,
			L_currentDate->tm_min,
			L_currentDate->tm_sec);

	}
} /* end of formatTime */


char* C_GeneratorStats::formatTime (struct timeval* P_tv)
{
	static char L_time [TIME_LENGTH];
	struct tm * L_currentDate;

	// Get the current date and time
	L_currentDate = localtime ((const time_t *)&P_tv->tv_sec);

	// Format the time
	if (L_currentDate == NULL)
	{
		memset (L_time, 0, TIME_LENGTH);
	} 
   else
	{
		sprintf(L_time, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", 
			L_currentDate->tm_year + 1900,
			L_currentDate->tm_mon + 1,
			L_currentDate->tm_mday,
			L_currentDate->tm_hour,
			L_currentDate->tm_min,
			L_currentDate->tm_sec);
			// SF917230 (int) (P_tv->tv_usec)/1000);
	}
	return (L_time);
} /* end of formatTime */

int C_GeneratorStats::executeStatAction (E_Action P_action, unsigned long P_value)
{
  switch (P_action)
  {
    case E_ADD_RESPONSE_TIME_DURATION :
      m_mutexTable[CPT_C_AverageResponseTime].lock() ;
      // Updating Cumulative Counter
      updateAverageCounter(CPT_C_AverageResponseTime, 
			   CPT_C_NbOfCallUsedForAverageResponseTime,
                           &m_C_sumResponseTime, P_value); 
      updateRepartition(m_ResponseTimeRepartition, 
			m_SizeOfResponseTimeRepartition, P_value);
      // Updating Periodical Diplayed counter
      updateAverageCounter(CPT_PD_AverageResponseTime, 
			   CPT_PD_NbOfCallUsedForAverageResponseTime,
                           &m_PD_sumResponseTime, 
			   P_value); 
      // Updating Periodical Logging counter
      updateAverageCounter(CPT_PL_AverageResponseTime, 
			   CPT_PL_NbOfCallUsedForAverageResponseTime,
                           &m_PL_sumResponseTime, 
			   P_value); 
      m_mutexTable[CPT_C_AverageResponseTime].unlock() ;
      break;

    default :
     GEN_FATAL(0,"C_GeneratorStats::executeStatAction() - Unrecognized Action " 
	   <<  P_action << " " << P_value);
  } /* end switch */
  return (0);
}


void C_GeneratorStats::info_msg (char *P_msg) {
  if (strlen(P_msg) < m_max_msg_size) {
    strcpy(m_info_msg, P_msg);
  } else {
    strncpy(m_info_msg, P_msg, m_max_msg_size);
    m_info_msg[m_max_msg_size]='\0' ;
  }
}

void C_GeneratorStats::err_msg (char *P_msg) {
  if (strlen(P_msg) < m_max_msg_size) {
    strcpy(m_err_msg, P_msg);
  } else {
    strncpy(m_err_msg, P_msg, m_max_msg_size);
    m_err_msg[m_max_msg_size]='\0' ;
  }
}



C_GeneratorStats* C_GeneratorStats::m_instance = NULL;

