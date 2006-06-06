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

/**********************************************************************
 * File      : C_MultiList.h
 * Purpose   :
 * Version   :
 * Author(s) : Henry.Fauchery
 *
 * Creation  : 1998
 * Modif.    :
 *             - 1999
 *             - 2000 Xavier Bourvellec ( C++ convertion )
 *********************************************************************/

#ifndef _C_MULTILIST_H_
#define _C_MULTILIST_H_

#include <cstdio>
#include <cstdlib>

/**********************************************************************
Notes

Following defines may help using this class:
* defining DEBUG_MULTILIST allows tracing
  - if the list is correctly configured when used
  - print messages when entering / quitting methods

* defining MLIST_ARGUMENTS_TESTING allows testing if Mlist methods were
passed out-of-bounds argument, usefull when coredump stand around

* defining MLIST_INTERNAL_COHERENCE test if list elements are placed in
  right list ( to detect internal errors when moving from one list to another )

************************************************************************/

// Local debug flags
// #define DEBUG_MULTILIST
// #define MLIST_ARGUMENTS_TESTING
// #define MLIST_INTERNAL_COHERENCE

// !! DO NOT CHANGE THESE VALUES
#define NULL_INDEX_ELEMENT        -1
#define TST_OK                    1
#define TST_ERROR                 NULL_INDEX_ELEMENT

/************************************************************************************/
/* C_MultiList class                                                                    */
/* This class manage lists of elements, is able to move elements from one list to   */
/* another.                                                                         */
/* IT STORES POINTERS ON OBJECTS, NOT OBJECTS THEMSELVES                            */
/*  i.e. using it like this :                                                       */
/*  C_MultiList<long> testList                                                          */
/*  will create a list management object of POINTERS to LONG, thus 'long *'         */
/*                                                                                  */
/* When instanciating a multilist object, you pass to its constructor the number of */
/* lists, and the total number of elements. */
/* You MUST then use 'initList' method to place all elements in the given list.     */
/************************************************************************************/

template <class Type>
class C_MultiList
{
protected:
  /* single element structure */
  typedef struct s_element
  {
    long            currentList ;   /*                           */
    long            prevElement ;   /* index previous dial in same state   */
    Type *          payLoad;        /* payload */
    long            nextElement ;   /* index next dial in same state       */
  } struct_element ;

  /* list header structure */
  typedef struct s_listHeader
  {
    long     firstElement ; /* index of the first element in the list */
    long     lastElement ;  /* index of the last element  in the list */
    long     nbElements ;
  } struct_listHeader ;

  /* Object sizes */
  long firstListIndex;
  long lastListIndex;

  long nbLists;
  long nbElements;

  /* array of elements pointer */
  s_element *       elementList;

  /* array of list pointer */
  s_listHeader *    stateList;

  /* initDone flag */
  int initDone;

  /* private tool functions */
  inline int removeElement ( long );
  inline int initSingleElement ( long , long );

public:


// ----------------------------------------------
// NAME: C_MultiList
//
// RETURN CODE :NONE
// PARAMETERS:
//          IN : first is the number of lists
//               second is the number of elements to be stored
//
// ROLE: multi list constructor.
//

  C_MultiList                           ( long , long );
  ~C_MultiList                          ( void );

// ----------------------------------------------
// NAME: initList
//
// RETURN CODE :return TST_ERROR if something went wrong ( like giving an out-of-bounds list index )
// PARAMETERS:
//          IN : the list index in which elements must be placed at start
//
// ROLE: initialize list : place all list elements in given list.
//
  int initList          ( long );

// ----------------------------------------------
// NAME: moveToList
//
// RETURN CODE :return TST_ERROR if something went wrong ( like giving an out-of-bounds list index )
// PARAMETERS:
//          IN: first is list index, second is element index
// ROLE: move a list element to given list
//

  int moveToList        ( long , long );

// ----------------------------------------------
// NAME: getFirst
//
// RETURN CODE : element index
// PARAMETERS:
//          IN:   list index whose first elemetn must be returned
// ROLE: give index of first list element from given list.
//       THIS FUNCTION DO NOT REMOVE ELEMENT !!!
//

  long  getFirst          ( long );

// ----------------------------------------------
// NAME: getNext
//
// RETURN CODE :element index following given one, or TST_ERROR if no next.
// PARAMETERS:
//          IN:   element index whose next ( element ) must be returned
// ROLE:  return element index following given element
//

  long  getNext           ( long );

// ----------------------------------------------
// NAME: isInState
//
// RETURN CODE : TST_OK if given element is containend in given list.
// PARAMETERS:
//          IN: first is list index, second is element index.
// ROLE: verify that an element in containend in a particular list.
//

  int   isInState         ( long , long ) ;

// ----------------------------------------------
// NAME: getCurrentList
//
// RETURN CODE : list index of given element
// PARAMETERS:
//          IN: element index.
// ROLE: verify that an element in containend in a particular list.
//

  long   getCurrentList         ( long ) ;

// ----------------------------------------------// NAME: setElementPayload
//
// RETURN CODE : return object contained in element
// PARAMETERS:
//          IN:  relement index
// ROLE  : return object contained in element given in parameter
// NOTES : can be SLOW if objects stored in elements are not simple types

  Type *  getElementPayload ( long );

// ----------------------------------------------
// NAME: setElementPayload
//
// RETURN CODE :
// PARAMETERS:
//          IN: place object in list.
// ROLE:
//
  int     setElementPayload ( long , Type * );

// ----------------------------------------------
// NAME: getNbElements
//
// RETURN CODE :
// PARAMETERS:
//          IN: long list
// ROLE: return number of elements in given list
//

  long    getNbElements ( long );

// ----------------------------------------------
// NAME: dump
//
// RETURN CODE :
// PARAMETERS:
// ROLE: print list content
//
  void  dump              ( void );
}; /* class C_MultiList */

#endif /* _C_MULTILIST_H_ */
