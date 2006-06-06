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

#include "C_DisplayHelp.hpp"


C_DisplayHelp::C_DisplayHelp(){
  m_display_ctrl = NULL ;
  m_counter = 0 ;
  m_ret = 0 ;
}

C_DisplayHelp::~C_DisplayHelp(){
  m_display_ctrl = NULL ;
}

void C_DisplayHelp::set_display_control(C_DisplayControl *P_display_ctrl) {
  m_display_ctrl = P_display_ctrl ;
}

void C_DisplayHelp::displayScreen() {
  if (m_display_ctrl) {
    // m_display_ctrl->help_all_screens();
    m_ret = m_display_ctrl->help_all_screens(m_counter);
  }
}

void C_DisplayHelp::setCurrentScreen(bool P_first) {
  if (P_first == true) {
    m_counter = 0 ;
    m_ret = 0 ;
  } else {
    if (m_ret) {
      if (m_ret < 20) {
	m_ret = 0 ;
	m_counter = 0 ; 
      } else {
	m_counter += m_ret ;
      }
    }
  }
}

