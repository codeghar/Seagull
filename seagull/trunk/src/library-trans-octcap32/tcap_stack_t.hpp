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

#ifndef _TCAP_STACK_T_
#define _TCAP_STACK_T_

#include <TCAP_ext.h>
#include <TCAP_errors.h>

typedef tcx_cnxid (*T_TCAP_stack_open) (
		    /* SS7 class name */
		    char*               SS7_name,
		    /* id of the requested TCAP service */
		    tcx_tcap_service       tcap_service_id,
		    /* subsystem number */
		    int                    ssn,
		    /*  enable component layer */
		    TC_BOOL                enable_comp_layer, 
		    /* TC_YES means that dialogues are accepted */
		    TC_BOOL                active, 
		    /* must be null if no application info is provided */
		    tcx_application_info*  appli_info, 
		    /* kind of sccp service requested */
		    tcx_sccp_service       sccp_service_kind,
		    /* connection timeout */
		    struct timeval        *timeout
		    );

typedef int (*T_TCAP_stack_close) (tcx_cnxid cnx_id);

typedef int (*T_TCAP_stack_send) (tcx_cnxid       cnx_id,
				  struct timeval* time,
				  tcx_primitive*  primitive,
				  tcx_component*  comp_list);

typedef int (*T_TCAP_stack_recv) (tcx_cnxid         cnx_id,
				  struct timeval*   time,
				  tcx_primitive*    primitive,
				  tcx_component**   comp_listp, 
				  int*              more_message);


typedef int (*T_TCAP_stack_control) (int                 connection_id, 
				     void              * context,
				     tc_control_c        command,
				     tc_control_parms 	* parameter);

typedef int (*T_TCAP_stack_cnx_handler) (int     num_fd,
					 fd_set  * read_mask,
					 fd_set  * write_mask,
					 fd_set  * exception_mask,
					 int     * num_cnx,
					 int     * cnx_ids );

typedef int (*T_TCAP_stack_select_mask) (int             maxFd,
					 fd_set          * read_mask,
					 fd_set          * write_mask,
					 fd_set          * exception_mask,
					 struct timeval  ** time);


typedef int (*T_TCAP_stack_alloc_buffer)(tcx_buffer** buffer_listp,
					 unsigned int size);

typedef int (*T_TCAP_stack_free_buffer)    (tcx_buffer* bufferp);
typedef int (*T_TCAP_stack_alloc_component)(tcx_component** comp_listp,
					    unsigned int    nb_of_components);
typedef int (*T_TCAP_stack_free_components)(tcx_component* comp_list);
typedef int (*T_TCAP_stack_flush_components)(tcx_cnxid cnx_id, int uid, int pid);
typedef int (*T_TCAP_stack_put_component) (tcx_cnxid      cnx_id,
				     int            user_dialogue_id,
				     int            provider_dialogue_id,
				     tcx_component* componentp);
typedef int (*T_TCAP_stack_get_component) (tcx_cnxid       cnx_id,
				     unsigned int    user_dialogue_id,
				     unsigned int    provider_dialogue_id,
				     tcx_component** componentp);

typedef tc_error_code* T_TCAP_stack_errno ;
typedef char **T_TCAP_stack_error_text   ;

typedef struct _tcap_stack_fct {
  T_TCAP_stack_open             m_open;
  T_TCAP_stack_close            m_close;
  T_TCAP_stack_send             m_send;
  T_TCAP_stack_recv             m_recv;
  T_TCAP_stack_control          m_control;
  T_TCAP_stack_cnx_handler      m_cnx_handler;
  T_TCAP_stack_select_mask      m_select_mask;
  T_TCAP_stack_errno            m_errno;
  T_TCAP_stack_error_text       m_error_text ;
  T_TCAP_stack_alloc_buffer     m_alloc_buffer;
  T_TCAP_stack_free_buffer      m_free_buffer;
  T_TCAP_stack_alloc_component  m_alloc_component;
  T_TCAP_stack_free_components  m_free_components;
  T_TCAP_stack_flush_components m_flush_components;
  T_TCAP_stack_put_component    m_put_component;
  T_TCAP_stack_get_component    m_get_component;
} T_TCAP_stack_fct, *T_pTCAP_stack_fct ;


#endif // _TCAP_STACK_T_
