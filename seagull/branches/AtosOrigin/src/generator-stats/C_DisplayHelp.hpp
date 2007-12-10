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

#ifndef _C_DISPLAYHELP_H
#define _C_DISPLAYHELP_H

#include "C_DisplayObject.hpp"

#include "C_DisplayControl.hpp"

class C_DisplayHelp : public C_DisplayObject {
public:

  C_DisplayHelp();
  ~C_DisplayHelp();
  
  void set_display_control (C_DisplayControl *P_display_ctrl);
  void displayScreen() ;
  void setCurrentScreen(bool P_first) ;

private:
  C_DisplayControl *m_display_ctrl ;


  int               m_counter ;
  int               m_ret     ;

} ;


#endif // _C_DISPLAYHELP_H


