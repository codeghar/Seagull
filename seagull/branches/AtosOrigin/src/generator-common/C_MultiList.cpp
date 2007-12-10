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
 * File      : C_MultiList.cpp
 * Purpose   :
 * Version   :
 * Author(s) : Xavier Bourvellec
 *
 * Creation  : 1998
 * Modif.    :
 *             - 1999 : JDH
 *             - 2000 : XBC : modified the one-big-scenario-list system for  
 *                            a less CPU consumming system : scenarios  
 *                            are now 'sorted' according their current 
 *                            state ( receiving , sending, waiting ) 
 *                            among lists representing these states. 
 *                            This allow doing loops only on scenarios states
 *                            which really need it ( namely :  SENDING,WAITING )
 *********************************************************************/

#include "C_MultiList.hpp"
#include "Utils.hpp"

#include "iostream_t.hpp"
#ifdef GEN_DEBUG_MULTILIST
#define MLIST_GEN_DEBUG(m) iostream_error << m << iostream_flush << iostream_endl
#define MLIST_GEN_FATAL(m) iostream_error << m << iostream_flush << iostream_endl
#else
#define MLIST_GEN_DEBUG(m) 
#define MLIST_GEN_FATAL(m)
#endif


#include <cassert>

/* PRIVATE *******************************************************************/

template <class Type> int C_MultiList<Type>::removeElement ( long elementIdx )
{
  int L_return = TST_OK ;
  long L_oldState ;
  long L_prev;
  long L_next;

#ifdef GEN_DEBUG_MULTILIST
 if ( ! initDone )
 {
   MLIST_GEN_FATAL("initList has NOT been called before using list");
   assert (0);
 }
  MLIST_GEN_DEBUG("BEGIN removeElement, element=" << elementIdx << ", state=" << elementList[elementIdx].currentList);
#endif /* GEN_DEBUG_MULTILIST */

  /* Get element chaining characteristics                                     */
  L_prev = elementList[elementIdx].prevElement ;
  L_next = elementList[elementIdx].nextElement ;

  /* Extract element list (which also specify the concerned list)             */
  L_oldState = elementList[elementIdx].currentList ;

  /* IF first pointer pointed on removed element                              */
  /* THEN IF last pointer pointed on removed element                          */
  /*      THEN sub-list becomes empty                                         */
  /*      ELSE next element becomes the new first pointed                     */
  /* ELSE IF last pointer pointed on removed element                          */
  /*      THEN previous element becomes the last pointed                      */

  if ( stateList[L_oldState].firstElement == elementIdx )
    {
      if ( stateList[L_oldState].lastElement == elementIdx )
      {
        /* It is the first and the last, so the alone element        */
        /* of the sub-list so makes the sub-list empty               */
        stateList[L_oldState].firstElement = NULL_INDEX_ELEMENT ;
        stateList[L_oldState].lastElement  = NULL_INDEX_ELEMENT ;
      }
      else
      {
        /* It is the first of a no empty list                        */
        /* The new first is the next one                             */
        stateList[L_oldState].firstElement = L_next ;

        /* unlink current element's 'next' from current element */
        elementList[L_next].prevElement = NULL_INDEX_ELEMENT ;
      }
    }
  else
    {
      if ( stateList[L_oldState].lastElement == elementIdx )
      {
        /* It is the last of a no empty list                         */
        /* The new last is the previous                              */
        stateList[L_oldState].lastElement = L_prev ;

        /* unlink current element's 'previous' from current element */
        elementList[L_prev].nextElement = NULL_INDEX_ELEMENT ;
      }
      else
      {
        /* It is an indifferent element of the sub-list              */
        elementList[L_next].prevElement = L_prev ;
        elementList[L_prev].nextElement = L_next ;
      }
    }

#ifdef GEN_DEBUG_MULTILIST
    MLIST_GEN_DEBUG ("END removeElement, FirstIdle=" << stateList[L_oldState].firstElement
                << ", LastIdle="  << stateList[L_oldState].lastElement);

#endif /* GEN_DEBUG_MULTILIST */

  return ( L_return );
}

template <class Type> int C_MultiList<Type>::initSingleElement( long P_initState , long elementIdx )
{
  int             L_return = TST_OK ;

  elementList[elementIdx].currentList      = P_initState ;
  /* Chain all elements */
  if ( elementIdx != 0 )
    {
      elementList[elementIdx].prevElement    = elementIdx-1 ;
    }
  else
    {
      elementList[elementIdx].prevElement    = NULL_INDEX_ELEMENT ;
    }

  if ( elementIdx != nbElements-1 )
    {
      elementList[elementIdx].nextElement    = elementIdx+1 ;
    }
  else
    {
      elementList[elementIdx].nextElement    = NULL_INDEX_ELEMENT ;
    }

  elementList[elementIdx].payLoad = NULL;

  return (L_return) ;
}

/* PUBLIC **************************************************************/

template <class Type> C_MultiList<Type>::C_MultiList ( long numberOfLists, long numberOfElements )
{

  firstListIndex = 0;
  lastListIndex = numberOfLists - 1;
  nbLists = numberOfLists;
  nbElements = numberOfElements;

  ALLOC_TABLE (elementList, struct_element*, 
               sizeof(struct_element), numberOfElements) ;
  ALLOC_TABLE (stateList, struct_listHeader*,
               sizeof(struct_listHeader), numberOfLists) ;

  initDone = 0;

  if (!elementList || !stateList )
  {
    iostream_error 
      << "C_MultiList class constructor : cannot allocate required memory (" 
      << numberOfLists  << " lists and " 
      << numberOfElements<< " elements )" << iostream_endl << iostream_flush ;
      assert (0);
  }
};

template <class Type> C_MultiList<Type>::~C_MultiList ( void )
{
#ifdef GEN_DEBUG_MULTILIST
  MLIST_GEN_DEBUG("C_MultiList destructor " << nbLists << " lists and " << nbElements << " elements ");
#endif /* GEN_DEBUG_MULTILIST */

  FREE_TABLE (elementList);
  FREE_TABLE (stateList);
};

template <class Type> int C_MultiList<Type>::initList (long P_initState)
{
  int L_return = TST_OK ;
  long  elementIdx = 0 ;
  long           listIndex = 0;

  /* test argument validity */
  if (P_initState >= firstListIndex && P_initState <= lastListIndex )
    {
      /* Initialize sub-list pointers                                          */

      for ( listIndex = 0 ; listIndex < nbLists ; listIndex ++ )
      {
        stateList[ listIndex ].firstElement  = NULL_INDEX_ELEMENT ;
        stateList[ listIndex ].lastElement   = NULL_INDEX_ELEMENT ;
        stateList[ listIndex ].nbElements    = 0;
      }

      stateList[P_initState].firstElement   = 0 ;
      stateList[P_initState].lastElement    = nbElements - 1;
      stateList[P_initState].nbElements     = nbElements;

      /* WHILE no error AND no all elements initialize DO                         */
      elementIdx = 0 ;
      while ( ( L_return == TST_OK ) && ( elementIdx < nbElements ) )
      {
        /* Initialize the <L_loop>th element                                     */
        L_return = initSingleElement( P_initState , elementIdx ) ;
        elementIdx ++ ;
      }
      /* END WHILE no error AND no all elements initialize DO                     */
      initDone = 1;
    }
  else
    {
      iostream_error << "GEN_ERROR : initList : P_initState=" 
		     <<  P_initState << iostream_endl ;
      L_return = TST_ERROR;
    }

  return (L_return) ;

};

template <class Type> int C_MultiList<Type>::moveToList (long listIdx, long elementIdx)
{
  int L_return = TST_OK ;
  long prevElemList;

#ifdef GEN_DEBUG_MULTILIST
    if ( ! initDone )
    {
      MLIST_GEN_FATAL("initList has NOT been called before using list");
      assert (0);
    }
    MLIST_GEN_DEBUG ("BEGIN C_MultiList::moveToList ( element " << elementIdx << " list " << listIdx << " )");
#endif /* GEN_DEBUG_MULTILIST */

#ifdef MLIST_ARGUMENTS_TESTING
/* test argument validity */
  if (listIdx >= firstListIndex && listIdx <= lastListIndex && elementIdx != TST_ERROR   )
    {
#endif /* MLIST_ARGUMENTS_TESTING */

      /* Get element list */
      prevElemList = elementList[elementIdx].currentList;

      /* Remove element from its current place                                    */
      L_return = removeElement ( elementIdx )  ;

      /* IF sub-list empty                                                     */
      if ( stateList[listIdx].lastElement == NULL_INDEX_ELEMENT )
      {
        /* THEN initialize sub-list                                                  */
        elementList[elementIdx].prevElement = NULL_INDEX_ELEMENT ;
        elementList[elementIdx].nextElement = NULL_INDEX_ELEMENT ;
        stateList[listIdx].firstElement     = elementIdx ;
        stateList[listIdx].lastElement      = elementIdx ;
      }
      else
      {
        /* ELSE Insert element at tail of 'listIdx' sub-list                     */
        /* - Next     field of old last  points on new element                   */
        /* - Previous field of new element  points on old last                   */
        /* - Next     field of new element  points on NULL                       */
        /* - Last  pointer               points on new element                   */
        elementList[stateList[listIdx].lastElement].nextElement = elementIdx ;
        elementList[elementIdx].prevElement = stateList[listIdx].lastElement ;
        elementList[elementIdx].nextElement = NULL_INDEX_ELEMENT ;
        stateList[listIdx].lastElement      = elementIdx ;
      }
      /* END IF sub-list empty                                                 */

      // decrement previous list counter
      stateList[prevElemList].nbElements --;

      // increment actual list counter
      stateList[listIdx].nbElements ++;

      // set element list to new one
      elementList[elementIdx].currentList = listIdx ;

#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG ("C_MultiList::moveToList : out of bounds arguments : list " << listIdx << " element " << elementIdx);
      assert (0); /* GEN_DEBUG */
      L_return = TST_ERROR;
    }
#endif /* MLIST_ARGUMENTS_TESTING */


#ifdef MLIST_INTERNAL_COHERENCE
  /* ++ GEN_DEBUG */
  long prevE = -1;
  long nextE = -1;

  prevE = elementList[elementIdx].nextElement;
  nextE = elementList[elementIdx].prevElement;

  if ( nextE != -1 )
    {
      if ( elementList[elementIdx].currentList != elementList[nextE].currentList )
      {
        MLIST_GEN_FATAL ("C_MultiList::moveToList: next element list is not the same as current elem list!!! ");
        assert (0);
      }
    }

  if ( prevE != -1 )
    {
      if ( elementList[elementIdx].currentList != elementList[prevE].currentList )
      {
        MLIST_GEN_FATAL ("C_MultiList::moveToList: prev element list is not the same as current elem list!!! ");
        assert (0);
      }
    }

  /* -- GEN_DEBUG */
#endif /* MLIST_INTERNAL_COHERENCE */
  return ( L_return );
};

template <class Type> long C_MultiList<Type>::getFirst (long listIdx)
{
#ifdef GEN_DEBUG_MULTILIST
  MLIST_GEN_DEBUG("BEGIN getFirst " << listIdx);
#endif /* GEN_DEBUG_MULTILIST */

#ifdef MLIST_ARGUMENTS_TESTING
  if (listIdx >= firstListIndex && listIdx <= lastListIndex)
    {
#  ifdef GEN_DEBUG_MULTILIST
    MLIST_GEN_DEBUG("END getFirst : returning " << stateList[listIdx].firstElement );
#  endif /* GEN_DEBUG_MULTILIST */
#endif /* MLIST_ARGUMENTS_TESTING */
      return ( stateList[listIdx].firstElement );
#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG ("END getFirst : out of bounds list index (" << listIdx << " while max index is " << lastListIndex << ")");
      assert (0); /* GEN_DEBUG */
      return (TST_ERROR);
    }
#endif /* MLIST_ARGUMENTS_TESTING */
};

template <class Type> long C_MultiList<Type>::getNext (long elementIndex)
{
#ifdef GEN_DEBUG_MULTILIST
  MLIST_GEN_DEBUG("BEGIN getNext of element " << elementIndex);
#endif /* GEN_DEBUG_MULTILIST */

#ifdef MLIST_ARGUMENTS_TESTING
  if (elementIndex >= 0 && elementIndex < nbElements)
    {
#     ifdef GEN_DEBUG_MULTILIST
     MLIST_GEN_DEBUG("END getNext : returning " << elementList[elementIndex].nextElement);
#     endif /* GEN_DEBUG_MULTILIST */
#endif /* MLIST_ARGUMENTS_TESTING */
      return ( elementList[elementIndex].nextElement );

#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG ("C_MultiList::getNext : out of bounds index ( " << elementIndex << " )");
      assert (0); /* GEN_DEBUG */
      return (TST_ERROR);
    }
#endif /* MLIST_ARGUMENTS_TESTING */
};

template <class Type> int C_MultiList<Type>::isInState (long listIdx, long elementIndex)
{
#ifdef MLIST_ARGUMENTS_TESTING
  if (listIdx >= firstListIndex && listIdx <= lastListIndex && elementIndex >= 0 && elementIndex < nbElements )
    {
#endif /* MLIST_ARGUMENTS_TESTING */
      return ( elementList[elementIndex].currentList == listIdx )
      ? 1 : 0 ;
#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG ("GEN_ERROR : isInState : out of bound indexes. Returning TST_ERROR, BEWARE.");
      return (TST_ERROR);
    }
#endif /* MLIST_ARGUMENTS_TESTING */
};

template <class Type> long C_MultiList<Type>::getCurrentList (long elementIndex)
{
#ifdef MLIST_ARGUMENTS_TESTING
  if (elementIndex >= 0 && elementIndex < nbElements )
    {
#endif /* MLIST_ARGUMENTS_TESTING */

      return ( elementList[elementIndex].currentList);

#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG("GEN_ERROR : isInState : out of bound indexes. Returning TST_ERROR, BEWARE.");
      return (TST_ERROR);
    }
#endif /* MLIST_ARGUMENTS_TESTING */
}

template <class Type> Type * C_MultiList<Type>::getElementPayload ( long elementIndex)
{

#ifdef GEN_DEBUG_MULTILIST
    if ( ! initDone )
    {
      MLIST_GEN_FATAL("GEN_ERROR : getElementPayload : initList has NOT been called before using list");
      assert (0);
    }

#endif /* GEN_DEBUG_MULTILIST */

#ifdef MLIST_ARGUMENTS_TESTING
  if (elementIndex >= 0 && elementIndex < nbElements)
    {
#endif /* MLIST_ARGUMENTS_TESTING */
      return ( elementList[elementIndex].payLoad );
#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG ("GEN_ERROR : getElementPayload : out of bounds element index ( " << elementIndex << " )");
      return (NULL);
    }
#endif /* MLIST_ARGUMENTS_TESTING */
}

template <class Type> int C_MultiList<Type>::setElementPayload ( long elementIndex, Type * payLoad )
{

#ifdef GEN_DEBUG_MULTILIST
    if ( ! initDone )
    {
      MLIST_GEN_FATAL("GEN_ERROR : setElementPayload : initList has NOT been called before using list");
      assert (0);
    }
#endif /* GEN_DEBUG_MULTILIST */

#ifdef MLIST_ARGUMENTS_TESTING
  if (elementIndex >= 0 && elementIndex < nbElements )
    {
#endif /* MLIST_ARGUMENTS_TESTING */
      elementList[elementIndex].payLoad = payLoad;
      return ( TST_OK );
#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG("GEN_ERROR : setElementPayload : out of bounds element index ( " << elementIndex << " )");
      return ( TST_ERROR );
    }
#endif /* MLIST_ARGUMENTS_TESTING */
}


template <class Type> long C_MultiList<Type>::getNbElements ( long listIdx )
{
#ifdef GEN_DEBUG_MULTILIST
  MLIST_GEN_DEBUG("BEGIN getNbElements " << listIdx);
#endif /* GEN_DEBUG_MULTILIST */

#ifdef MLIST_ARGUMENTS_TESTING
  if (listIdx >= firstListIndex && listIdx <= lastListIndex)
    {
#  ifdef GEN_DEBUG_MULTILIST
    MLIST_GEN_DEBUG("END getNbElements : returning " << stateList[listIdx].nbElements);
#  endif /* GEN_DEBUG_MULTILIST */
#endif /* MLIST_ARGUMENTS_TESTING */
      return ( stateList[listIdx].nbElements );
#ifdef MLIST_ARGUMENTS_TESTING
    }
  else
    {
      MLIST_GEN_DEBUG("END getNbElements : out of bounds list index (" << listIdx << " while max index is " << lastListIndex << ")");
      assert (0); /* GEN_DEBUG */
      return (TST_ERROR);
    }
#endif /* MLIST_ARGUMENTS_TESTING */
}

template <class Type> void C_MultiList<Type>::dump (void)
{
  int L_int, L_cpt, L_cpt2, L_loop;

#ifdef GEN_DEBUG_MULTILIST
    if ( ! initDone )
    {
      MLIST_GEN_FATAL("initList has NOT been called before using list");
      assert (0);
    }
#endif /* GEN_DEBUG_MULTILIST */

  MLIST_GEN_DEBUG ("-- DUMP -----------------------");
  for (L_loop = 0; L_loop < nbLists; L_loop++)
    {
      L_int = stateList[L_loop].firstElement;
      L_cpt = 0;

      if (L_int != NULL_INDEX_ELEMENT)
      {
        L_cpt = 1;
      }

      while (L_int != stateList[L_loop].lastElement)
      {
        L_cpt++;
        L_int = elementList[L_int].nextElement;
        if (L_cpt > nbElements)
          L_int = stateList[L_loop].lastElement;

        if ( L_loop != elementList[L_int].currentList )
        {
            MLIST_GEN_FATAL ("(1) Element " << L_int << " found in list " << L_loop << " while having " << elementList[L_int].currentList << " as current list ???" );
            assert (0 );
        }
      }

      L_int = stateList[L_loop].lastElement;
      L_cpt2 = 0;

      if (L_int != NULL_INDEX_ELEMENT)
      {
        L_cpt2 = 1;
      }

      while (L_int != stateList[L_loop].firstElement)
      {
        L_cpt2++;
        L_int = elementList[L_int].prevElement;
        if (L_cpt2 > nbElements)
          {
          L_int = stateList[L_loop].firstElement;
          }

        if ( L_loop != elementList[L_int].currentList )
        {
            MLIST_GEN_FATAL ("(2) Element " << L_int << " found in list " << L_loop << " while having " << elementList[L_int].currentList << " as current list ???" );
            assert (0 );
        }
      }
      MLIST_GEN_DEBUG ("Nb elements in list [" << setw(6) << L_loop << "] compteur 1 :" << setw(6) << L_cpt << " compteur 2: " << setw(6) << L_cpt2 << " getNbElements " << getNbElements (L_loop) << " first element: " << setw(6) << stateList[L_loop].firstElement );
    }
    MLIST_GEN_DEBUG ("-- END OF DUMP -----------------" );
};
