/*** binding.c *****************************************************************
** 
** This file is part of BibTool.
** It is distributed under the GNU General Public License.
** See the file COPYING for details.
** 
** (c) 2015-2016 Gerd Neugebauer
** 
** Net: gene@gerd-neugebauer.de
** 
******************************************************************************/

#include <bibtool/bibtool.h>
#include <bibtool/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "binding.h"
#include "literate.h"
#include "lcore.h"

/*****************************************************************************/
/* Internal Programs                                                         */
/*===========================================================================*/

#ifdef __STDC__
#define _ARG(A) A
#else
#define _ARG(A) ()
#endif

/*****************************************************************************/
/* External Programs                                                         */
/*===========================================================================*/

extern Term meth_db _ARG((Binding binding, Term db, Term args));
extern Term meth_list _ARG((Binding binding, Term cons, Term args));
extern Term meth_number _ARG((Binding binding, Term n, Term args));
extern Term meth_string _ARG((Binding binding, Term s, Term args));

extern Binding cb_binding;
extern Binding cc_binding;
extern Binding cd_binding;
extern Binding cn_binding;
extern Binding cr_binding;
extern Binding cs_binding;

/*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
** Function:	binding()
** Type:	Binding
** Purpose:	Allocate a new instance of a Binding.
** Arguments:
**	size	the number of junks contained
** Returns:	the new binding
**___________________________________________________			     */
Binding binding(size, nextBinding)		   /*                        */
  unsigned int size;				   /*                        */
  Binding nextBinding;				   /*                        */
{						   /*                        */
  Binding b = (Binding) malloc(sizeof(SBinding));  /*                        */
  if (b == BindingNULL) OUT_OF_MEMORY("binding");  /*                        */
 						   /*                        */
  BJunks(b) = (SymDef*) calloc(size, sizeof(SymDef));/*                      */
  if (BJunks(b) == NULL) OUT_OF_MEMORY("binding"); /*                        */
 						   /*                        */
  BSize(b) = size;				   /*                        */
  NextBinding(b) = nextBinding;		   	   /*                        */
  return b;				   	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	free_binding()
** Type:	void
** Purpose:	Release the memory occupied by a binding.
**		
** Arguments:
**	binding	the binding
** Returns:	nothing
**___________________________________________________			     */
void free_binding(binding)			   /*                        */
  Binding binding;				   /*                        */
{ int i;					   /*                        */
  SymDef sym, junk;				   /*                        */
 						   /*                        */
  for (i = BSize(binding) - 1; i >= 0; i--)	   /*                        */
  { for (junk = BJunks(binding)[i];		   /*                        */
	 junk;					   /*                        */
	 junk = sym)			   	   /*                        */
    { sym = NextJunk(junk);			   /*                        */
      free_sym(junk);				   /*                        */
    }						   /*                        */
  }						   /*                        */
  free(binding);				   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	bind()
** Type:	void
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	sym	the symbol definition
** Returns:	nothing
**___________________________________________________			     */
void bind(binding, sym)		   		   /*                        */
  Binding binding;				   /*                        */
  SymDef sym;					   /*                        */
{ String key 	 = SymName(sym);		   /*                        */
  unsigned int h = SymHash(sym) % BSize(binding);  /*                        */
  SymDef junk;					   /*                        */
   						   /*                        */
  for (junk = BJunks(binding)[h];		   /*                        */
       junk;					   /*                        */
       junk = NextJunk(junk))			   /*                        */
  { if (SymName(junk) == key)			   /*                        */
    { SymTerm(junk)  = SymTerm(sym);		   /*                        */
      LinkTerm(SymTerm(junk));			   /*                        */
      SymValue(junk) = SymValue(sym);		   /*                        */
      SymGet(junk)   = SymGet(sym);		   /*                        */
      SymSet(junk)   = SymSet(sym);		   /*                        */
      free_sym(sym);				   /*                        */
      return;					   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  NextJunk(sym) = BJunks(binding)[h];		   /*                        */
  BJunks(binding)[h] = sym;			   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	gbind()
** Type:	void
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	sym	the symbol definition
** Returns:	nothing
**___________________________________________________			     */
void gbind(binding, sym)			   /*                        */
  Binding binding;				   /*                        */
  SymDef sym;					   /*                        */
{ String key = SymName(sym);		   	   /*                        */
  unsigned int h;	   			   /*                        */
  SymDef junk;					   /*                        */
   						   /*                        */
  for (;;)					   /*                        */
  { h = SymHash(sym) % BSize(binding);		   /*                        */
 						   /*                        */
    for (junk = BJunks(binding)[h];		   /*                        */
	 junk;					   /*                        */
	 junk = NextJunk(junk))			   /*                        */
    { if (SymName(junk) == key)			   /*                        */
      { SymTerm(junk)  = SymTerm(sym);		   /*                        */
	LinkTerm(SymTerm(junk));		   /*                        */
	SymValue(junk) = SymValue(sym);		   /*                        */
	SymGet(junk)   = SymGet(sym);		   /*                        */
	SymSet(junk)   = SymSet(sym);		   /*                        */
	free_sym(sym);				   /*                        */
	return;					   /*                        */
      }						   /*                        */
    }						   /*                        */
 						   /*                        */
    if (NextBinding(binding) == NULL) break;	   /*                        */
 						   /*                        */
    binding = NextBinding(binding);		   /*                        */
  }						   /*                        */
  NextJunk(sym) = BJunks(binding)[h];		   /*                        */
  BJunks(binding)[h] = sym;			   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	setq()
** Type:	Term
** Purpose:	
**		
** Arguments:
**	b	the binding
**	key	the key
**	term	the term
** Returns:	
**___________________________________________________			     */
Term setq(b, key, term)		   		   /*                        */
  Binding b;				   	   /*                        */
  String key;					   /*                        */
  Term term;					   /*                        */
{ unsigned int h = hash(key) % BSize(b);	   /*                        */
  SymDef junk;					   /*                        */
   						   /*                        */
  for (junk = BJunks(b)[h]; junk; junk = NextJunk(junk))/*                   */
  { if (SymName(junk) == key)			   /*                        */
    { UnlinkTerm(SymValue(junk));		   /*                        */
      SymValue(junk) = term;		   	   /*                        */
      LinkTerm(term);				   /*                        */
      return term;				   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  junk = symdef(key,			   	   /*                        */
		L_VAR,			   	   /*                        */
		SYM_NONE,		   	   /*                        */
		g_field,		   	   /*                        */
		g_setq);  		   	   /*                        */
  SymValue(junk) = term;		   	   /*                        */
  NextJunk(junk) = BJunks(b)[h];		   /*                        */
  BJunks(b)[h]   = junk;			   /*                        */
  LinkTerm(term);				   /*                        */
  return term;					   /*                        */
}						   /*------------------------*/

#undef DEBUG_BIND

/*-----------------------------------------------------------------------------
** Function:	get_bind()
** Type:	SymDef
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	key	the key
** Returns:	
**___________________________________________________			     */
SymDef get_bind(binding, key)			   /*                        */
  Binding binding;				   /*                        */
  String key;					   /*                        */
{ SymDef s;					   /*                        */
  unsigned int h = hash(key);	   		   /*                        */
  						   /*                        */
#ifdef DEBUG_BIND
  puts("BINDING");				   /*                        */
  dump_binding(binding, stdout);		   /*                        */
  printf("--- lookup %s at %d\n", (char*)key, h);  /*                        */
#endif
  while (binding)				   /*                        */
  {						   /*                        */
    for (s = BJunks(binding)[h % BSize(binding)];  /*                        */
	 s;					   /*                        */
	 s = NextJunk(s))			   /*                        */
    {						   /*                        */
#ifdef DEBUG_BIND
      printf("--- cmp %s\n",(char*)SymName(s));	   /*                        */
#endif
      if (cmp(SymName(s), key) == 0) {		   /*                        */
#ifdef DEBUG_BIND
	printf("--- found 0x%x\n",s ? SymOp(s):0); /*                        */
#endif
	return s; 	   			   /*                        */
      }						   /*                        */
    }						   /*                        */
    binding = NextBinding(binding);		   /*                        */
  }						   /*                        */
  return SymDefNULL; 	   			   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	bool_s_rsc()
** Type:	Term
** Purpose:	Setter for a boolean resource.
**		
** Arguments:
**	binding	the binding
**	name	the name
**	term	the term
**	rp	a pointer to the resource
** Returns:	the new value
**___________________________________________________			     */
static Term bool_s_rsc(binding, name, term, rp)	   /*                        */
  Binding binding;				   /*                        */
  char * name;				   	   /*                        */
  Term term;					   /*                        */
  int * rp;					   /*                        */
{ term = eval_bool(binding, Cadr(term));	   /*                        */
  *rp  = (IsTrue(term) ? 1 : 0 );	   	   /*                        */
  UnlinkTerm(term);				   /*                        */
  return (*rp ? term_true : term_false);	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	num_rsc()
** Type:	Term
** Purpose:	Setter for a numeric resource.
**		
** Arguments:
**	binding	the binding
**	name	the name
**	term	the term
**	rp	a pointer to the resource
** Returns:	the new value
**___________________________________________________			     */
static Term num_s_rsc(binding, name, term, rp)	   /*                        */
  Binding binding;				   /*                        */
  char * name;				   	   /*                        */
  Term term;					   /*                        */
  int * rp;					   /*                        */
{ term = eval_num(binding, Cadr(term));	   	   /*                        */
  *rp  = TNumber(term);	   		   	   /*                        */
  return term;				   	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	str_rsc()
** Type:	Term
** Purpose:	Setter for a string resource.
**		
** Arguments:
**	binding	the binding
**	name	the name
**	term	the term
**	rp	a pointer to the resource
** Returns:	the new value
**___________________________________________________			     */
static Term str_s_rsc(binding, name, term, rp)	   /*                        */
  Binding binding;				   /*                        */
  char *name;				   	   /*                        */
  Term term;					   /*                        */
  String *rp;					   /*                        */
{ term = eval_str(binding, Cadr(term));	   	   /*                        */
  *rp  = TString(term);	   		   	   /*                        */
  return term;				   	   /*                        */
}						   /*------------------------*/

#define Bind(NAME,GET,SET,FLAGS,OP)
#define BindSym(NAME,SYM)
#define BindBool(NAME,GETTER,SETTER,RSC)		\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    term = (RSC ? term_true : term_false);		\
    LinkTerm(term);					\
    return term;					\
  }							\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    return bool_s_rsc(binding, NAME, term, &RSC); }
#define BindNum(NAME,GETTER,SETTER,RSC)			\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    return NumberTerm(RSC); }				\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern int RSC;					\
    return num_s_rsc(binding, NAME, term, &RSC); }
#define BindStr(NAME,GETTER,SETTER,RSC)			\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern String RSC;					\
    return StringTerm(RSC ? RSC : (String)""); }	\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { extern String RSC;					\
    return str_s_rsc(binding, NAME, term, &RSC); }
#define BindFct(NAME,GETTER,SETTER,SETTER_FCT)		\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { ErrorNF2(NAME, " is not accessible"); return NIL; }	\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { String val;						\
    term = eval_str(binding, Cadr(term));		\
    val  = TString(term);				\
    SETTER_FCT;						\
    return term; }
#define BindFunc(NAME,GETTER,GET_FCT,SETTER,SET_FCT)	\
  static Term GETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { return GET_FCT; }					\
  static Term SETTER (binding, term)			\
    Binding binding;					\
    Term term;						\
  { String val;						\
    term = eval_str(binding, Cadr(term));		\
    val  = TString(term);				\
    SET_FCT;						\
    return GET_FCT; }
#include "builtin.h"

#undef Bind
#undef BindSym
#undef BindBool
#undef BindNum
#undef BindStr
#undef BindFct
#undef BindFunc

/*-----------------------------------------------------------------------------
** Function:	root_binding()
** Type:	Binding
** Purpose:	Create a new binding and initialize it with the BibTool
**		defaults.
**		
** Arguments:	none
** Returns:	the new binding
**___________________________________________________			     */
Binding root_binding()				   /*                        */
{ Binding b = binding(511, BindingNULL);	   /*                        */
 						   /*                        */
#define BindBool(NAME,GET,SET,R)     Bind(NAME, GET, SET, SYM_BUILTIN, L_VAR)
#define BindNum(NAME,GET,SET,R)	     Bind(NAME, GET, SET, SYM_BUILTIN, L_VAR)
#define BindStr(NAME,GET,SET,R)	     Bind(NAME, GET, SET, SYM_BUILTIN, L_VAR)
#define BindFct(NAME,GET,SET,EX)     Bind(NAME, GET, SET, SYM_BUILTIN, L_VAR)
#define BindFunc(NAME,GET,G,SET,S)   Bind(NAME, GET, SET, SYM_BUILTIN, L_VAR)
#define Bind(NAME,GET,SET,FLAGS,OP)  bind(b, symdef(symbol((String)NAME),     \
						    OP, FLAGS, GET, SET));
#define BindSym(NAME,SYM)	      bind(b, SYM);
 						   /*                        */
#include "builtin.h"
  return b;				   	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	dump_binding()
** Type:	void
** Purpose:	
**		
** Arguments:
**	binding	the binding
**	file	the target output file
** Returns:	nothing
**___________________________________________________			     */
void dump_binding(binding, file)		   /*                        */
  Binding binding;				   /*                        */
  FILE* file;					   /*                        */
{ int i;					   /*                        */
  SymDef s;					   /*                        */
 						   /*                        */
  for (; binding; binding = NextBinding(binding))  /*                        */
  {						   /*                        */
#ifdef DEBUG
    fprintf(file, "--- 0x%lx [%d] ---\n",	   /*                        */
	    (unsigned long)binding,		   /*                        */
	    BSize(binding));			   /*                        */
#endif
    for (i = 0; i < BSize(binding); i++)	   /*                        */
    { s = BJunks(binding)[i];			   /*                        */
      if (s) {					   /*                        */
	fprintf(file, "    #%d\n", i);		   /*                        */
	for (; s; s = NextJunk(s))		   /*                        */
	{ fprintf(file,				   /*                        */
		  "\t%s\n",			   /*                        */
		  SymName(s));			   /*                        */
	}					   /*                        */
      }						   /*                        */
    }						   /*                        */
  }						   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	each()
** Type:	Term
** Purpose:	
**		
** Arguments:
**	b	the binding
**	a	the assigns
**	group	the code group
** Returns:	
**___________________________________________________			     */
static Term each(b, a, group)		   	   /*                        */
  Binding b;					   /*                        */
  Term a;					   /*                        */
  Term group;					   /*                        */
{ Binding new_bind  = binding(13, b);		   /*                        */
  Term t	    = evaluate(b, Cdr(a));	   /*                        */
  Iterator iterator = get_iterator(t);		   /*                        */
 						   /*                        */
  if (iterator == NULL )			   /*                        */
    ErrorNF2("Illegal argument for each: ",	   /*                        */
	    term_type(t));		   	   /*                        */
  t = NIL;					   /*                        */
  						   /*                        */
  while (DoItHasNext(iterator))			   /*                        */
  { setq(new_bind,				   /*                        */
	 TString(a),				   /*                        */
	 evaluate(b, DoItNext(iterator)));	   /*                        */
    t = evaluate(new_bind, group);		   /*                        */
  }						   /*                        */
 						   /*                        */
  DoItFinish(iterator);			   	   /*                        */
 						   /*                        */
  LinkTerm(t);					   /*                        */
  free_binding(new_bind);			   /*                        */
  return t;					   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	funcall()
** Type:	Term
** Purpose:	
**		
** Arguments:
**	b	the binding
**	key	the name of the invoking function
**	f	the function term
**	args	the actual argument list
** Returns:	
**___________________________________________________			     */
static Term funcall(b, key, f, args)		   /*                        */
  Binding b;					   /*                        */
  String key;					   /*                        */
  Term f;					   /*                        */
  Term args;					   /*                        */
{ Binding nb = binding(127, b);			   /*                        */
  SymDef sym;					   /*                        */
  Term t;					   /*                        */
 						   /*                        */
  for (t = Car(f); t; t = Cdr(t))		   /*                        */
  { sym = symdef(TString(Car(t)), L_VAR, 0, NULL, NULL);/*                   */
    if (args)					   /*                        */
    { SymValue(sym) = evaluate(b, Car(args));	   /*                        */
      args = Cdr(args);				   /*                        */
    }						   /*                        */
    else					   /*                        */
    { SymValue(sym) = evaluate(b, Cdar(t));	   /*                        */
      LinkTerm(SymValue(sym));			   /*                        */
    }						   /*                        */
    bind(nb, sym);				   /*                        */
  }						   /*                        */
 						   /*                        */
  if (args)					   /*                        */
    ErrorNF2("Too many arguments for ", key);	   /*                        */
    						   /*                        */
  t = evaluate(nb, Cdr(f));			   /*                        */
 						   /*                        */
  LinkTerm(t);					   /*                        */
  free_binding(nb);				   /*                        */
  return t;					   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	evaluate()
** Type:	Term
** Purpose:	Evaluate a term and return the result.
** Arguments:
**	binding	the binding
**	term	the term to evaluate
** Returns:	the resulting term
**___________________________________________________			     */
Term evaluate(binding, term)			   /*                        */
  Binding binding;				   /*                        */
  Term term;					   /*                        */
{ SymDef s;					   /*                        */
  String key = NULL;				   /*                        */
 						   /*                        */
  if (term == NIL) return NIL;			   /*                        */
 						   /*                        */
  switch (TType(term))				   /*                        */
  { case L_CONS:				   /*                        */
      { Term t = NIL, *tp;			   /*                        */
	tp     = &t;				   /*                        */
	for ( ; term; term = Cdr(term))		   /*                        */
	{ *tp = Cons1(evaluate(binding,	   	   /*                        */
				Car(term)));	   /*                        */
	  tp = &Cdr(*tp);			   /*                        */
	}					   /*                        */
	return t;				   /*                        */
      }						   /*                        */
 						   /*                        */
    case L_DB:				   	   /*                        */
    case L_FALSE:				   /*                        */
    case L_NUMBER:				   /*                        */
    case L_RECORD:				   /*                        */
    case L_STRING:				   /*                        */
    case L_TRUE:				   /*                        */
      LinkTerm(term);				   /*                        */
      return term;				   /*                        */
 						   /*                        */
    case L_DEFUN:				   /*                        */
      return setq(binding,			   /*                        */
		  TString(term),		   /*                        */
		  Cdr(term));			   /*                        */
 						   /*                        */
    case L_EACH:				   /*                        */
      return each(binding, Car(term), Cdr(term));  /*                        */
 						   /*                        */
    case L_VAR	:				   /*                        */
      s = get_bind(binding, TString(term));	   /*                        */
      return s ? SymValue(s) : NIL;		   /*                        */
 						   /*                        */
    case L_FUNCALL:				   /*                        */
      key = TString(term);			   /*                        */
      s	  = get_bind(binding, key);	   	   /*                        */
      if (s == NULL)	   			   /*                        */
	ErrorNF2("Undefined function ", key);	   /*                        */
 						   /*                        */
      if (SymValue(s) &&			   /*                        */
	  TType(SymValue(s)) == L_FUNCTION)	   /*                        */
      { Term t = funcall(binding, key,		   /*                        */
			 SymValue(s),		   /*                        */
			 Cdr(term));		   /*                        */
	if (t && TType(t) == L_RETURN)		   /*                        */
	{ Term x = t;				   /*                        */
	  t = Cdr(t);		   		   /*                        */
	  Cdr(x) = NIL;				   /*                        */
	  UnlinkTerm(x);			   /*                        */
	}					   /*                        */
	return t;				   /*                        */
      }		   				   /*                        */
 						   /*                        */
      if (SymGet(s) == NULL)	   		   /*                        */
	ErrorNF2("Undefined function ", key);	   /*                        */
 						   /*                        */
      if (Cdr(term))				   /*                        */
      { if (SymSet(s) == NULL)	   		   /*                        */
	  ErrorNF2(key, " is immutable");	   /*                        */
	return (*SymSet(s))(binding, term);	   /*                        */
      }						   /*                        */
      else					   /*                        */
      { if (SymGet(s) == NULL)	   		   /*                        */
	  ErrorNF2(key, " is not readable");	   /*                        */
 						   /*                        */
	return (*SymGet(s))(binding, term);	   /*                        */
      }						   /*                        */
 						   /*                        */
    case L_FUNCTION:				   /*                        */
      LinkTerm(term);				   /*                        */
      return term;				   /*                        */
 						   /*                        */
    case L_GROUP:				   /*                        */
      { Term tt, t = NIL;			   /*                        */
	for (tt = Cdr(term); tt; tt = Cdr(tt))     /*                        */
	{ t = evaluate(binding, Car(tt));	   /*                        */
	  if (t && TType(t) == L_RETURN) return t; /*                        */
	}	   				   /*                        */
	return t;				   /*                        */
      }						   /*                        */
 						   /*                        */
    case L_IF:				   	   /*                        */
      if (IsTrue(eval_bool(binding, Car(term))))   /*                        */
      { return evaluate(binding, Cadr(term)); }    /*                        */
      						   /*                        */
      return (Cddr(term)			   /*                        */
	      ? evaluate(binding, Cddr(term))	   /*                        */
	      : NIL);				   /*                        */
 						   /*                        */
    case L_METHOD:				   /*                        */
      { Term t = evaluate(binding, Car(term));	   /*                        */
	Binding class_b;			   /*                        */
	char * clazz;				   /*                        */
 						   /*                        */
	switch (t == NIL ? L_CONS : TType(t))	   /*                        */
	{ case L_TRUE:				   /*                        */
	    class_b = cb_binding;		   /*                        */
	    clazz   = "True";			   /*                        */
	    break;				   /*                        */
	  case L_FALSE:				   /*                        */
	    class_b = cb_binding;		   /*                        */
	    clazz   = "False";			   /*                        */
	    break;				   /*                        */
	  case L_STRING:			   /*                        */
	    class_b = cs_binding;		   /*                        */
	    clazz   = "String";			   /*                        */
	    break;				   /*                        */
	  case L_NUMBER:			   /*                        */
	    class_b = cn_binding;		   /*                        */
	    clazz   = "Number";			   /*                        */
	    break;				   /*                        */
	  case L_CONS:				   /*                        */
	    class_b = cc_binding;		   /*                        */
	    clazz   = "List";			   /*                        */
	    break;				   /*                        */
	  case L_DB:				   /*                        */
	    class_b = cd_binding;		   /*                        */
	    clazz   = "DB";			   /*                        */
	    break;				   /*                        */
	  case L_RECORD:			   /*                        */
	    class_b = cr_binding;		   /*                        */
	    clazz   = "Record";			   /*                        */
	    break;				   /*                        */
	  case L_FUNCTION:			   /*                        */
	  default:				   /*                        */
	    ErrorNF3("Missing instance for method ",/*                       */
		     TString(Cdr(term)),"()");	   /*                        */
	}					   /*                        */
 						   /*                        */
	SymDef symdef = get_bind(class_b,	   /*                        */
			   TString(Cdr(term)));	   /*                        */
	if (symdef == SymDefNULL		   /*                        */
	    || SymGet(symdef) == NULL)		   /*                        */
	  ErrorNF3(clazz,			   /*                        */
		   ": Unknown method ",	   	   /*                        */
		   TString(Cdr(term)));		   /*                        */
	return (*SymGet(symdef))(binding, t, Cddr(term));/*                  */
      }						   /*                        */
    case L_RETURN:				   /*                        */
      return new_term(L_RETURN,			   /*                        */
		      NIL,			   /*                        */
		      evaluate(binding, Cdr(term)));/*                       */
 						   /*                        */
    case L_WITH:				   /*                        */
      return funcall(binding, key, term, NIL);	   /*                        */
 						   /*                        */
    case L_AND:      key = (String)"&&";     break;/*                        */
    case L_DIV:      key = (String)"/";	     break;/*                        */
    case L_EQ:       key = (String)"==";     break;/*                        */
    case L_GE:       key = (String)">=";     break;/*                        */
    case L_GT:       key = (String)">";	     break;/*                        */
    case L_ILIKE:    key = (String)"ilike";  break;/*                        */
    case L_LE:       key = (String)"<=";     break;/*                        */
    case L_LIKE:     key = (String)"like";   break;/*                        */
    case L_LT:       key = (String)"<";	     break;/*                        */
    case L_MINUS:    key = (String)"-";	     break;/*                        */
    case L_MOD:      key = (String)"mod";    break;/*                        */
    case L_NE:       key = (String)"!=";     break;/*                        */
    case L_NOT:      key = (String)"!";	     break;/*                        */
    case L_OR:       key = (String)"||";     break;/*                        */
    case L_PLUS:     key = (String)"+";	     break;/*                        */
    case L_QUOTE:    key = (String)"'";	     break;/*                        */
    case L_SET:      key = (String)"=";	     break;/*                        */
    case L_TIMES:    key = (String)"*";	     break;/*                        */
    default:					   /*                        */
      ErrorNF2("Undefined tag ", term_type(term)); /*                        */
  }						   /*                        */
 						   /*                        */
   s = get_bind(binding, key);		   	   /*                        */
   if (s == SymDefNULL) 			   /*                        */
   { ErrorNF2("Undefined function ", key); } 	   /*                        */
 						   /*                        */
   return (*SymGet(s))(binding, term);		   /*                        */
}						   /*------------------------*/

/*---------------------------------------------------------------------------*/
