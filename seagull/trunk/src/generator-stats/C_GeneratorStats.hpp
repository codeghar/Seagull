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

#ifndef _C_GENERATOR_STATS_H
#define _C_GENERATOR_STATS_H

#define TIME_LENGTH 32
#define DEFAULT_FILE_NAME (char*)"dumpFile"
#define DEFAULT_EXTENSION (char*)".csv"

#define MAX_REPARTITION_HEADER_LENGTH 1024
#define MAX_REPARTITION_INFO_LENGTH   1024
#define MAX_CHAR_BUFFER_SIZE          256

#include "TimeUtils.hpp"
#include "C_Mutex.hpp"

// C++ standard include
#include "fstream_t.hpp"


/*
__________________________________________________________________________

              C L A S S    C S t a t 
__________________________________________________________________________
*/

/**
 * This class provides some means to compute and display statistics.
 * This is a singleton class.
*/

class C_GeneratorStats {
public:

  /* 
   * This struct is used for repartition table
   * border max is the max value allow for this range
   * nbInThisBorder is the counter of value in this range
   */
  typedef struct _T_dynamicalRepartition
  {
    unsigned int  borderMax;
    unsigned long nbInThisBorder; 
  } T_dynamicalRepartition; 

  /**
	* Actions on counters
	*/
  enum E_Action
  {
    E_CREATE_INCOMING_CALL,
    E_CALL_FAILED, 
    E_CALL_REFUSED,
    //    E_CALL_SUCCESSFULLY_ENDED,


    E_CALL_INIT_SUCCESSFULLY_ENDED,
    E_CALL_TRAFFIC_SUCCESSFULLY_ENDED,
    E_CALL_DEFAULT_SUCCESSFULLY_ENDED,
    E_CALL_ABORT_SUCCESSFULLY_ENDED,

    E_SEND_MSG,    
    E_RECV_MSG,

    E_RESET_PD_COUNTERS,
    E_RESET_PL_COUNTERS,

    E_FAILED_UNEXPECTED_MSG,
    E_FAILED_CANNOT_SEND_MSG,
    E_FAILED_ABORTED,

    E_FAILED_TIMEOUT,

    E_CREATE_OUTGOING_CALL,
   

    // Not used 
    E_ADD_CALL_DURATION,
    E_ADD_RESPONSE_TIME_DURATION,
    E_FAILED_CALL_REJECTED,
    E_FAILED_MAX_UDP_RETRANS,
    E_FAILED_CMD_NOT_SENT,
    E_FAILED_REGEXP_DOESNT_MATCH,
    E_FAILED_REGEXP_HDR_NOT_FOUND
  };
  /**
	* Counters management
	*/
  enum E_CounterName
  {
	 // Cumulative counter
	 CPT_C_IncomingCallCreated,
         CPT_C_MsgSend,    
         CPT_C_MsgRecv,
	 CPT_C_CurrentCall,


	 CPT_C_InitSuccessfulCall,
	 CPT_C_TrafficSuccessfulCall,
	 CPT_C_DefaultSuccessfulCall,
	 CPT_C_AbortSuccessfulCall,
	 CPT_C_FailedCall,
	 CPT_C_RefusedCall,
	 CPT_C_FailedCallAborted,
	 CPT_C_FailedCallTimeout,

	 CPT_C_FailedCallCannotSendMessage,
	 CPT_C_FailedCallUnexpectedMessage,
	 CPT_C_OutgoingCallCreated,
	 CPT_C_NbOfCallUsedForAverageResponseTime,
	 CPT_C_AverageResponseTime,

	 // not used
	 CPT_C_NbOfCallUsedForAverageCallLength,
	 CPT_C_AverageCallLength,
	 CPT_C_FailedCallMaxUdpRetrans,
	 CPT_C_FailedCallCallRejected,
	 CPT_C_FailedCallCmdNotSent,
	 CPT_C_FailedCallRegexpDoesntMatch,
	 CPT_C_FailedCallRegexpHdrNotFound,

	 // Periodic Display counter
	 CPT_PD_IncomingCallCreated, // must be the first PD counter (RESET_PD_COUNTER macro)
         CPT_PD_MsgSend,    
         CPT_PD_MsgRecv,


	 CPT_PD_InitSuccessfulCall,
	 CPT_PD_TrafficSuccessfulCall,
	 CPT_PD_DefaultSuccessfulCall,
	 CPT_PD_AbortSuccessfulCall,
	 CPT_PD_FailedCall,
         CPT_PD_RefusedCall,
	 CPT_PD_FailedCallAborted,
	 CPT_PD_FailedCallTimeout,

	 CPT_PD_FailedCallCannotSendMessage,
	 CPT_PD_FailedCallUnexpectedMessage,
	 CPT_PD_OutgoingCallCreated,
	 CPT_PD_NbOfCallUsedForAverageResponseTime,
	 CPT_PD_AverageResponseTime,


	 // not used
	 CPT_PD_NbOfCallUsedForAverageCallLength,
	 CPT_PD_AverageCallLength,
	 CPT_PD_FailedCallMaxUdpRetrans,
	 CPT_PD_FailedCallCallRejected,
	 CPT_PD_FailedCallCmdNotSent,
	 CPT_PD_FailedCallRegexpDoesntMatch,
	 CPT_PD_FailedCallRegexpHdrNotFound, // must be the last PD counter (RESET_PD_COUNTER macro)

	 // Periodic logging counter
	 CPT_PL_IncomingCallCreated, // must be the first PL counter (RESET_PL_COUNTER macro)
         CPT_PL_MsgSend,    
         CPT_PL_MsgRecv,


	 CPT_PL_InitSuccessfulCall,
	 CPT_PL_TrafficSuccessfulCall,
	 CPT_PL_DefaultSuccessfulCall,
	 CPT_PL_AbortSuccessfulCall,
	 CPT_PL_FailedCall,
         CPT_PL_RefusedCall,
	 CPT_PL_FailedCallAborted,
	 CPT_PL_FailedCallTimeout,


	 CPT_PL_FailedCallCannotSendMessage,
	 CPT_PL_FailedCallUnexpectedMessage,
	 CPT_PL_OutgoingCallCreated,
 	 CPT_PL_NbOfCallUsedForAverageResponseTime,
	 CPT_PL_AverageResponseTime,


	 // not used
	 CPT_PL_NbOfCallUsedForAverageCallLength,
	 CPT_PL_AverageCallLength,
	 CPT_PL_FailedCallMaxUdpRetrans,
	 CPT_PL_FailedCallCallRejected,
	 CPT_PL_FailedCallCmdNotSent,
	 CPT_PL_FailedCallRegexpDoesntMatch,
	 CPT_PL_FailedCallRegexpHdrNotFound, // must be the last PL counter (RESET_PL_COUNTER macro)

	 E_NB_COUNTER
  };



  /*
  ** exported methods
  */
  /**
	* Get the single instance of the class.
	*
	* Only one instance of C_GeneratorStats exists in the component. This
	* instance is automatically created the first time the instance()
	* method is called.
	*
	* @return the single instance of the C_GeneratorStats class.
	*/
  static C_GeneratorStats* instance (); 

  /**
	* Delete the single instance of the class.
	*
	* Only one instance of C_GeneratorStats exists in the component. This
	* instance is deleted when the close method is called.
	*/
  void close (); 

  /**
   * ComputeStat Methode are used to modify counter value
   * It's the main interface to update counter
   *
   * @return 0 if the action is known
   *        -1 else
   */
  int computeStat (E_Action P_action);
  int computeStat (E_Action P_action, unsigned long P_value);

  int executeStatAction (E_Action P_action);
  int executeStatAction (E_Action P_action, unsigned long P_value);

  void displayScreen (int P_id, int P_sub_screen);

  void makeFirstLog ();
  void makeLog() ;

  void info_msg (char *P_msg) ;
  void err_msg (char *P_msg) ;
  
  void reset_cumul_counters () ;


  void activate_percent_traffic () ;

  /**
   * setRepartitionCallLength 
   * - set the unsigned int table passed in parameter as the repartition table for call length
   *   This is done by calling the initRepartition methode on the m_CallLengthRepartition variable
   * - set the char* list of int (must be separeted with coma as the repartition table
   *   for call length
   *   This is done by calling the createIntegerTable to transform the char* list into
   *   unsigned int list. Then the initRepartition methode is call with the created unsigned int
   *   list and the m_CallLengthRepartition variable
   *
   *setRepartitionResponseTime
   *   Same than setRepartitionCallLength with the variable m_ResponseTimeRepartition variable
       instead of m_CallLengthRepartition variable
   */
  void setRepartitionCallLength   (unsigned int* repartition, int nombre);
  void setRepartitionCallLength   (char * liste);
  void setRepartitionResponseTime (unsigned int* repartition, int nombre);
  void setRepartitionResponseTime (char * liste);

  /* define the file name to use to dump statistic in file */
  void setFileName                (char * name);
  void setFileName                (char * name, char * extension);

  /**
	* Display data periodically updated on screen.
	*/
  void displayData ();
  void displayStat();

  void displayRepartition();



  /**
	* Dump data periodically in the file m_FileName
	*/
  void dumpData ();
  

  /**
   * initialize the class variable member
   */
  int init();

  void displayScreen() ;
  // void setCurrentScreen() ;
  void makeDisplay1 (bool P_display) ;
  void makeDisplay2 () ;

  char *dumpCounters();

private:

  bool m_initDone ;
   
  /** 
	* Constructor.
	*
	* Made private because this is a singleton class.
	*/
  C_GeneratorStats ();

  /** 
	* Destructor.
	*
	* Made private because this is a singleton class.
	*/
  ~C_GeneratorStats ();

  static C_GeneratorStats*            m_instance;

  unsigned long            m_counters        [E_NB_COUNTER];
  unsigned long            m_displayCounters [E_NB_COUNTER];
  unsigned long            m_loggingCounters [E_NB_COUNTER];
  unsigned long            m_remoteCounters  [E_NB_COUNTER];

  T_dynamicalRepartition*  m_ResponseTimeRepartition;
  T_dynamicalRepartition*  m_CallLengthRepartition;
  int                      m_SizeOfResponseTimeRepartition;
  int                      m_SizeOfCallLengthRepartition;

  struct timeval           m_startTime;
  struct timeval           m_pdStartTime;
  struct timeval           m_plStartTime;

  bool                     m_headerAlreadyDisplayed;
  char                    *m_fileName;
  fstream_output          *m_outputStream;

  unsigned long long       m_C_sumCallLength;
  unsigned long long       m_C_sumResponseTime;
  unsigned long long       m_PD_sumCallLength;
  unsigned long long       m_PD_sumResponseTime;
  unsigned long long       m_PL_sumCallLength;
  unsigned long long       m_PL_sumResponseTime;

  C_Mutex                  m_mutexTable[E_NB_COUNTER];

  void makeDisplay3 (int P_sub_screen) ;

  char  *m_info_msg ;
  char  *m_err_msg ;
  size_t m_max_msg_size ;

  /**
   * initRepartition
   * This methode is used to create the repartition table with a table of unsigned int
   * the reparition is created like following, with Vi the given value in the table
   * 0    <= x <  V1  
   * V1   <= x <  V2 
   *  ...
   * Vn-1 <= x <  Vn
   *         x >= Vn
   * So the repartition table have the size n+1 if the given table has a size of n
   */
  void  initRepartition(unsigned int* repartition, int nombre,
                       T_dynamicalRepartition ** tabRepartition, int* nbTab);

  /**
   * createIntegerTable
   * this method try to create a table of unsigned int with the list of char* passed
   * in parameters
   * if it succed, it's return true (1)
   * else it's return false (0)
   */
  int  createIntegerTable(char * P_listeStr, unsigned int ** listeInteger, int * sizeOfList);

  /**
   * isWellFormed
   * this method check if the char* passed in parameter in really a list of integer 
   * separated with comma.
   * if yes, it's return true (1)
   * else, it's return false (0)
   */
  int  isWellFormed(char * P_listeStr, int * nombre);

  /**
   * updateRepartition
   * The methode look for the place to set the value passed in parameter
   * Once found, the associeted counter is incremented
   */
  void  updateRepartition( T_dynamicalRepartition* tabRepart, int sizeOfTab, unsigned long value);

  /**
   * displayRepartition
   * Display the repartition passed in parameter at the screen
   */
  void  displayRepartition(T_dynamicalRepartition * tabRepartition, int sizeOfTab);

  void displayRepartition_with_percent(T_dynamicalRepartition * tabRepartition, int sizeOfTab);
  void displayRepartition_without_percent(T_dynamicalRepartition * tabRepartition, int sizeOfTab);


  /**
   * sRepartitionHeader
   * return a string with the range description of the given repartition
   */
  char* sRepartitionHeader(T_dynamicalRepartition * tabRepartition, int sizeOfTab,
                           char* P_repartitionName);

  /**
   * sRepartitionInfo
   * return a string with the number of value in the differente range of the given repartition
   */
  char* sRepartitionInfo(T_dynamicalRepartition * tabRepartition, int sizeOfTab);

  /**
   * UpdateAverageCounter
   * This methode compute the real moyenne with the passed value on the given counter
   */
  void updateAverageCounter(E_CounterName P_AvarageCounter, E_CounterName P_NbOfCallUsed,
                            unsigned long long* P_sum, unsigned long P_value);

  /**
	* msToHHMMSS.
	*
	* This converts an unsigned long containing a number of ms
	* into a string expressing the same value in format HH:MM:SS.
	*
	* @param P_ms.
	* 
	* @return a pointer on a static string containing formated time
	*/
  char* msToHHMMSS (unsigned long P_ms);

  char* msToHHMMSSmmm (unsigned long P_ms);
  /**
	* msToHHMMSSmm.
	*
	* This converts an unsigned long containing a number of ms
	* into a string expressing the same value in format HH:MM:SS:mmm.
	*
	* @param P_ms.
	* 
	* @return a pointer on a static string containing formated time
	*/


  /**
	* formatTime.
	*
	* This method converts a struct timeval parameter into a printable string
	* in the format given in parameter.
	*
	* @param P_tv.
	* @return a pointer on a static string containing formated time
	*/
  char* formatTime (struct timeval* P_tv);
  void  formatTime (char *P_time, struct timeval* P_tv);

  // typedef void (C_GeneratorStats::* T_DisplayFct) () ;

  // int           m_current_stat_id      ;
  // int           m_nb_stat_id           ;
  // T_DisplayFct *m_current_stat_table   ;
  // T_DisplayFct  m_current_stat_display ;


  typedef void (C_GeneratorStats::* T_DisplayRepFct)(T_dynamicalRepartition * tabRepartition, 
						     int sizeOfTab) ;

  int                   m_current_display_rep_id      ;
  int                   m_nb_display_rep_id           ;
  T_DisplayRepFct      *m_current_repartition_table   ;
  T_DisplayRepFct       m_current_repartition_display ;





  /**
	* Effective C++
	*
	* To prevent public copy ctor usage: no implementation
	*/
  C_GeneratorStats (const C_GeneratorStats&);

  /**
	* Effective C++
	*
	* To prevent public operator= usage: no implementation
	*/
  C_GeneratorStats& operator=(const C_GeneratorStats&);

};

#endif // _C_GENERATOR_STATS_H
