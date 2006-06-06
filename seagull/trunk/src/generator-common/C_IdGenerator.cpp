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

#include "C_IdGenerator.hpp"
#include "Utils.hpp"

#include "GeneratorTrace.hpp"

#include "list_t.hpp"
#include "set_t.hpp"

typedef list_t<int> T_IdList ;
typedef set_t<int>  T_IdSet  ;

typedef struct _struct_id_gen {
  int      m_nb_id_used ;
  T_IdList *m_available_ids ;
  T_IdSet  *m_used_ids ;
} T_IdGen, *T_pIdGen ;


C_IdGenerator::C_IdGenerator() {
  T_pIdGen L_gen ;
  ALLOC_VAR(L_gen, T_pIdGen, sizeof(T_IdGen));
  m_impl = L_gen ;
  NEW_VAR(L_gen->m_available_ids, T_IdList());
  NEW_VAR(L_gen->m_used_ids, T_IdSet());
  L_gen->m_used_ids->clear();
  L_gen->m_available_ids->clear();
  L_gen->m_nb_id_used = 0 ;
}

C_IdGenerator::~C_IdGenerator() {
  T_pIdGen L_gen = (T_pIdGen) m_impl;
  
  if (!L_gen->m_used_ids->empty()) {
    L_gen->m_used_ids->erase(L_gen->m_used_ids->begin(),
			    L_gen->m_used_ids->end());
  }
  if (!L_gen->m_available_ids->empty()) {
    L_gen->m_available_ids->erase(L_gen->m_available_ids->begin(),
				 L_gen->m_available_ids->end());
  }
  L_gen->m_nb_id_used = 0;

  DELETE_VAR(L_gen->m_available_ids);
  DELETE_VAR(L_gen->m_used_ids);

  FREE_VAR(L_gen);
  m_impl = NULL ;
}

int  C_IdGenerator::new_id () {

  T_pIdGen           L_gen = (T_pIdGen) m_impl;
  T_IdList::iterator L_it ;
  int                L_id ;

  if (!L_gen->m_available_ids->empty()) {
    L_it = L_gen->m_available_ids->begin() ;
    L_id = *L_it ;
    L_gen->m_available_ids->erase(L_it);
  } else {
    L_id = L_gen->m_nb_id_used ;
    (L_gen->m_nb_id_used)++ ;
  }
  L_gen->m_used_ids->insert(L_id);

  return (L_id);
} 

int  C_IdGenerator::release_id (int P_id) {

  T_pIdGen          L_gen = (T_pIdGen) m_impl;
  int               L_ret = -1 ;
  T_IdSet::iterator L_it ;

  L_it = L_gen->m_used_ids->find(P_id) ;
  if (L_it != L_gen->m_used_ids->end()) {
    L_gen->m_used_ids->erase(L_it);
    L_gen->m_available_ids->push_back(P_id);
    L_ret = 0 ;
  }
  return (L_ret);
}
