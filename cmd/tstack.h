/*** tstack.h *****************************************************************
** 
** This file is part of BibTool.
** It is distributed under the GNU General Public License.
** See the file COPYING for details.
** 
** (c) 2015-2018 Gerd Neugebauer
** 
** Net: gene@gerd-neugebauer.de
** 
******************************************************************************/

#include "term.h"

/*-----------------------------------------------------------------------------
** Typedef:	STStack
** Purpose:	
**		
**		
**___________________________________________________			     */
typedef struct T_STACK {			   /*                        */
  int c;					   /*                        */
  Term term;					   /*                        */
  struct T_STACK * prev;			   /*                        */
} STStack, *TStack;				   /*------------------------*/

#define StackNULL	(TStack)NULL
#define StackChar(TS)	((TS)->c)
#define StackTerm(TS)	((TS)->term)
#define StackPrev(TS)	((TS)->prev)

/*---------------------------------------------------------------------------*/

extern TStack ts_pop _ARG((TStack ts));
extern TStack ts_push _ARG((TStack ts, int c, Term t));
extern void dump_tstack _ARG((FILE* file, TStack stack));

/*---------------------------------------------------------------------------*/
