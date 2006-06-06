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

#ifndef _C_DISPLAYCONTROL_H
#define _C_DISPLAYCONTROL_H

#include "C_Semaphore.hpp"
#include "C_SemaphoreTimed.hpp"
#include "C_GeneratorStats.hpp"
#include "C_TaskControl.hpp"
#include "C_DisplayObject.hpp"
#include "list_t.hpp"


class C_DisplayControl : public C_TaskControl {

 public:

  C_DisplayControl() ;
  ~C_DisplayControl() ;
  
  void  init (long P_period);
  void  init_screens () ;
  void  set_screen (char P_key) ;
  void  set_sub_screen (int P_increment) ;
  void  set_max_sub_screen (int P_id, int P_max_sub_screen) ;
  void  pause () ;

  void  add_screen (C_DisplayObject *P_display, 
		    char P_key,
		    char *P_comment);
  void  add_help (char P_key,
		  char *P_comment);
  int  help_all_screens (int P_counter) ;

  void change_display_period (long P_period);

private :
  
  C_GeneratorStats *m_stat;
  C_SemaphoreTimed *m_sem ;

  typedef struct _t_screen_data {
    C_DisplayObject *m_object ;
    char             m_key    ;
    char            *m_comment;
  } T_ScreenData, *T_pScreenData ;
  typedef list_t<T_ScreenData> T_ScreenDataList, *T_pScreenDataList ;


  typedef struct _t_screen_by_key {
    int             m_nb_screen       ;
    int             m_current_screen  ;
    C_DisplayObject **m_screen_object ;
    char            *m_comment        ;
  } T_ScreenByKey, *T_pScreenByKey ;

  T_pScreenByKey    m_screen_table ;
  C_DisplayObject  *m_current_display ;

  typedef struct _help_info {
    char m_key      ;
    char *m_comment ;
  } T_HelpInfo, *T_pHelpInfo ;
  T_pHelpInfo m_help_table      ;
  int         m_help_table_size ;
  
  typedef list_t<T_HelpInfo> T_HelpInfoList, 
    *T_pHelpInfoList ;          

  T_pHelpInfoList m_help_list ;
  

     
  T_pScreenDataList  m_screen_list ;

  int                m_current_screen  ;

  int               m_screen_id  ;
  int               m_previous_screen_id ;
  C_Semaphore      *m_sem_scr_id ; 

  bool              m_pause ;
  C_Semaphore      *m_sem_pause ;
  
  T_GeneratorError TaskProcedure () ;
  T_GeneratorError InitProcedure () ;
  T_GeneratorError EndProcedure () ;
  T_GeneratorError StoppingProcedure () ;
  T_GeneratorError ForcedStoppingProcedure () ;
  
} ;

#endif // _C_DISPLAYCONTROL_H
