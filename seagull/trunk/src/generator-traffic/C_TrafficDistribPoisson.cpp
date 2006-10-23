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

#include "C_TrafficDistribPoisson.hpp"
#include "GeneratorTrace.hpp"
#include "Utils.hpp" 

#include <cmath>

C_TrafficDistribPoisson::C_TrafficDistribPoisson () : C_TrafficModel() { 
}


C_TrafficDistribPoisson::~C_TrafficDistribPoisson() {
}



int C_TrafficDistribPoisson::authorize_new_call ()
{
  return (0);
} /* end of authorize_new_call */


