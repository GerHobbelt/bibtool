/*** macros.c ******************************************************************
** 
** This file is part of BibTool.
** It is distributed under the GNU General Public License.
** See the file COPYING for details.
** 
** (c) 1996-2018 Gerd Neugebauer
** 
** Net: gene@gerd-neugebauer.de
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2, or (at your option)
** any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************/

#include <bibtool/general.h>
#include <bibtool/symbols.h>
#include <bibtool/error.h>
#include <bibtool/macros.h>
#include <bibtool/sbuffer.h>
#include "config.h"

/*****************************************************************************/
/* Internal Programs                                                         */
/*===========================================================================*/

/*****************************************************************************/
/* External Programs                                                         */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

 static Macro macros = MacroNULL;		   /*                        */

/*-----------------------------------------------------------------------------
** Function:	new_macro()
** Purpose:	Allocate a new macro structure and fill it with initial values.
**		Upon failure an error is raised and |exit()| is called.
** Arguments:
**	name	Name of the macro. This must be a symbol.
**	value	The value of the macro. This must be a symbol.
**	next	The next pointer of the |Macro| structure.
**	count	The initial reference count.
** Returns:	The new |Macro|.
**___________________________________________________			     */
Macro new_macro(name, value, next, count)	   /*                        */
  Symbol	 name;				   /*                        */
  Symbol	 value;				   /*                        */
  int		 count;				   /*                        */
  Macro          next;				   /*                        */
{ register Macro macro;				   /*                        */
 						   /*                        */
  if ( (macro=(Macro)malloc(sizeof(SMacro))) == MacroNULL )/*                */
  { OUT_OF_MEMORY("Macro"); }      		   /*                        */
 						   /*                        */
  MacroName(macro)  = name;			   /*                        */
  MacroValue(macro) = value ;			   /*                        */
  MacroCount(macro) = count;			   /*                        */
  NextMacro(macro)  = next;			   /*                        */
  return(macro);				   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	free_macro()
** Purpose:	Free a list of macros. The memory allocated for the
**		|Macro| given as argument and all structures
**		reachable via the |NextMacro| pointer are released.
** Arguments:
**	mac	First Macro to release.
** Returns:	nothing
**___________________________________________________			     */
void free_macro(mac)				   /*                        */
  Macro mac;					   /*                        */
{ Macro next;					   /*                        */
 						   /*                        */
  while (mac)					   /*                        */
  { next = NextMacro(mac);			   /*                        */
    UnlinkSymbol(MacroName(mac));		   /*                        */
    UnlinkSymbol(MacroValue(mac));		   /*                        */
    free(mac);					   /*                        */
    mac = next;					   /*                        */
  }						   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	def_macro()
** Purpose:	Define or undefine a macro.
** Arguments:
**	name	name of the macro.
**	val	NULL or the value of the new macro
**	count	initial count for the macro.
** Returns:	nothing
**___________________________________________________			     */
int def_macro(name, val, count)			   /*                        */
  Symbol	 name;			   	   /*                        */
  Symbol	 val;				   /*                        */
  int		 count;				   /*                        */
{ register Macro *mp;				   /*                        */
 						   /*                        */
  for (mp   = &macros;				   /*                        */
       *mp != MacroNULL;			   /*                        */
       mp   = &NextMacro(*mp))			   /*                        */
  { if (MacroName(*mp) == name)			   /*                        */
    { if (val)					   /*                        */
      { MacroValue(*mp) = val; }		   /*                        */
      else					   /*                        */
      { Macro mac = *mp;			   /*                        */
        *mp = NextMacro(*mp);			   /*                        */
	free(mac);			   	   /*                        */
      }						   /*                        */
      return 1;					   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  if (val)					   /*                        */
  { macros = new_macro(name, val, macros, count); }/*                        */
  return 0;					   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	look_macro()
** Purpose:	Return the value of a macro if it is defined. This
**		value is a symbol. 
**		If the macro is undefined then |NULL| is returned.  In
**		this case the value of |add| determines whether or not
**		the macro should be defined. If it is less than 0
**		then no new macros is defined. Otherwise a new macro
**		is defined. The value is the empty string and the
**		initial reference count is |add|.
** Arguments:
**	name	The name of the macros to find. This needs not to be a symbol.
**	add	Initial reference count or indicator that no new macro
**		is required.
** Returns:	The value or |NULL|.
**___________________________________________________			     */
Symbol look_macro(name, add)			   /*                        */
  Symbol	 name;			   	   /*                        */
  int		 add;				   /*                        */
{ register Macro *mp;				   /*                        */
						   /*                        */
  for (mp   = &macros;				   /*                        */
       *mp != MacroNULL;			   /*                        */
       mp   = &NextMacro(*mp))			   /*                        */
  { if (MacroName(*mp) == name)			   /*                        */
    { if (MacroCount(*mp) >= 0)			   /*                        */
	MacroCount(*mp) += add;			   /*                        */
      return(MacroValue(*mp));			   /*                        */
    }						   /*                        */
  }						   /*                        */
  if (add >= 0)					   /*                        */
  { def_macro(name, sym_empty, add);		   /*                        */
    WARNING3("Macro '",name,"' is undefined.");	   /*                        */
  }						   /*                        */
  return NULL;					   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	foreach_macro()
** Purpose:	Apply a function to each macro in turn. The function
**		is called with the name and the value of the macro. If
**		it returns |false| then the processing of further
**		macros is suppressed.
**
**		The function given as argument is called with two
**		string arguments. The first is the name of the macro
**		and the second is its value. Both are symbols and must
**		not be modified in any way.
**
**		The order of the enumeration of the macros is
**		determined by the implementation. No specific
**		assumptions should be made about this order.
** Arguments:
**	fct	Function to apply to each macro.
** Returns:	nothing
**___________________________________________________			     */
void foreach_macro(fct)				   /*                        */
  bool (*fct) _ARG((Symbol ,Symbol ));		   /*                        */
{ Macro mac;					   /*                        */
  for (mac = macros; 				   /*                        */
       mac != MacroNULL; 			   /*                        */
       mac = NextMacro(mac))			   /*                        */
  { if (MacroValue(mac) &&			   /*                        */
	! (*fct)(MacroName(mac), MacroValue(mac))) /*                        */
      return;					   /*                        */
  }						   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	each_macro()
** Type:	int
** Purpose:	Iterate over a linked list of macros. A function is applied	
**		to each macro found. The loop terminates if the
**		function returns |false|.
** Arguments:
**	m	the macro to start with
**	fct	the function to apply
** Returns:	|true| if the function has terminated the loop and
**		|false| in case the end of the list has been reached
**___________________________________________________			     */
bool each_macro(m, fct)				   /*                        */
  Macro m;					   /*                        */
  bool (*fct) _ARG((Symbol ,Symbol));		   /*                        */
{						   /*                        */
  for ( ; m != MacroNULL; m = NextMacro(m))	   /*                        */
  { if (! (*fct)(MacroName(m), MacroValue(m))) 	   /*                        */
      return true;				   /*                        */
  }						   /*                        */
  return false;					   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	dump_mac()
** Purpose:	Write macros to a file.
** Arguments:
**	fname	File name of the target file.
**	allp	if == 0 only the used macros are written.
** Returns:	nothing
**___________________________________________________			     */
void dump_mac(fname,allp)			   /*                        */
  char           *fname;			   /*                        */
  int            allp;				   /*                        */
{ FILE           *file;				   /*                        */
  register Macro mac;				   /*                        */
 						   /*                        */
  if ( *fname == '-' && *(fname+1) == '\0' ) 	   /*                        */
  { file = stdout; }				   /*                        */
  else if ( (file=fopen(fname,"w")) == NULL )      /*                        */
  { ERROR2("File could not be opened: ", fname);   /*                        */
    return;					   /*                        */
  }					   	   /*                        */
 						   /*                        */
  for (mac = macros; 				   /*                        */
       mac != MacroNULL; 			   /*                        */
       mac = NextMacro(mac))			   /*                        */
  { if (MacroCount(mac) > 0  ||			   /*                        */
        (MacroCount(mac) >= 0 && allp))		   /*                        */
    { if (MacroName(mac) == MacroValue(mac))	   /*                        */
      { (void)fprintf(file,			   /*                        */
		      "_string{ %s = %s } used: %d\n",/*                     */
		      SymbolValue(MacroName(mac)), /*                        */
		      SymbolValue(MacroValue(mac)),/*                        */
		      MacroCount(mac));		   /*                        */
      }						   /*                        */
      else					   /*                        */
      { (void)fprintf(file,			   /*                        */
		      "@STRING{ %s = %s } used: %d\n",/*                     */
		      SymbolValue(MacroName(mac)), /*                        */
		      SymbolValue(MacroValue(mac)),/*                        */
		      MacroCount(mac));		   /*                        */
      }						   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  if ( file != stdout ) (void)fclose(file);	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	init_macros()
** Purpose:	Initialize some macros from a table defined in the
**		configuration file or given as define to the C
**		compiler. This function has to be called to initialize
**		the global macros.
**
**		Note that this function is for internal purposes
**		only. The normal user should call |init_bibtool()|
**		instead.
** Arguments:	none
** Returns:	nothing
**___________________________________________________			     */
void init_macros()				   /*                        */
{ 						   /*                        */
#ifdef INITIALIZE_MACROS	
  register Symbol name;				   /*                        */
  register char**wp;				   /*			     */
  static char *word_list[] =			   /*                        */
  { INITIALIZE_MACROS, NULL };			   /*                        */
 						   /*                        */
  for (wp = word_list; *wp != NULL; ++wp)	   /*                        */
  { name = symbol((String)*wp);			   /*                        */
    def_macro(name, name, -1);			   /*                        */
  }		   				   /*			     */
#endif
}						   /*------------------------*/


/*****************************************************************************/
/***									   ***/
/*****************************************************************************/

 static Macro items = MacroNULL;		

/*-----------------------------------------------------------------------------
** Function*:	def_item()
** Purpose:	Define a macro. The arguments have to be symbols.
** Arguments:
**	name	the name of the item
**	value	the value of the item
** Returns:	nothing
**___________________________________________________			     */
static void def_item(name, value)		   /*                        */
  register Symbol name;			   	   /*                        */
  register Symbol value;			   /*                        */
{						   /*                        */
  items = new_macro(name, value, items, 0);	   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	def_field_type()
** Purpose:	This function adds a printing representation for a
**		field name to the used list. The argument is an
**		equation of the following form
**		
**			\textit{type = value}
**		
**		\textit{type} is translated to lower case and compared
**		against the internal representation. \textit{value} is
**		printed at the appropriate places instead.
** Arguments:
**	s	String containing an equation. This string is modified
**		during the process.
** Returns:	nothing
**___________________________________________________			     */
void def_field_type(s)				   /*                        */
  String s;					   /*                        */
{ Symbol name;					   /*                        */
 						   /*                        */
  while (*s && !is_allowed(*s)) ++s;		   /*                        */
  if (*s == '\0') return;			   /*                        */
 						   /*                        */
  name = sym_extract(&s, true);			   /*                        */
  						   /*                        */
  while (*s && !is_allowed(*s)) ++s;		   /*                        */
  if (*s == '\0') return;			   /*                        */
 						   /*                        */
  def_item(name, sym_extract(&s, false));	   /*                        */
}						   /*------------------------*/


/*****************************************************************************/
/***									   ***/
/*****************************************************************************/

/*-----------------------------------------------------------------------------
** Function*:	get_mapped_or_cased()
** Purpose:	Query a mapping in a macro list or return the
**		translated name.
** Arguments:
**	name	the lower-case name of the macro to get
**	mac	the list of macros to query
**	type	the type of the fall-back from |SYMBOL_TYPE_LOWER|,
**		|SYMBOL_TYPE_UPPER|, or |SYMBOL_TYPE_CASED|.
** Returns:	the requested value
**___________________________________________________			     */
static Symbol get_mapped_or_cased(name, mac, type) /*                        */
  Symbol	 name;				   /*                        */
  int            type;				   /*                        */
  register Macro mac;				   /*                        */
{ static StringBuffer* sb = (StringBuffer*)NULL;   /*                        */
  register String s;	   			   /*                        */
 						   /*                        */
  for ( ; mac != MacroNULL; mac = NextMacro(mac) ) /*                        */
  { if (name == MacroName(mac))		   	   /*                        */
    {						   /*                        */
      LinkSymbol(MacroValue(mac));		   /*                        */
      return MacroValue(mac);			   /*                        */
    }						   /*                        */
  }						   /*                        */
 						   /*                        */
  if (sb == NULL)				   /*                        */
  { if ((sb=sbopen()) == NULL)	   		   /*                        */
    { OUT_OF_MEMORY("get_item()"); } 		   /*                        */
  } else 					   /*                        */
  { sbrewind(sb); }				   /*                        */
 						   /*                        */
  switch (type)			   	   	   /*                        */
  { case SYMBOL_TYPE_LOWER:			   /*                        */
      return name;				   /*                        */
 						   /*                        */
    case SYMBOL_TYPE_CASED:			   /*                        */
      for (s = SymbolValue(name); *s; )		   /*                        */
      { if (is_alpha(*s))			   /*                        */
	{ (void)sbputc(ToUpper(*s), sb);	   /*                        */
	  for (++s; is_alpha(*s); ++s)  	   /*                        */
	  { (void)sbputc(*s, sb); }	   	   /*                        */
	}					   /*                        */
	else					   /*                        */
	{ (void)sbputc(*s, sb);		   	   /*                        */
	  ++s;				   	   /*                        */
	}					   /*                        */
      }					   	   /*                        */
      break;					   /*                        */
 						   /*                        */
    case SYMBOL_TYPE_UPPER:			   /*                        */
      for (s = SymbolValue(name); *s; ++s)	   /*                        */
      { (void)sbputc(ToUpper(*s), sb); }	   /*                        */
      break;					   /*                        */
  }						   /*                        */
  return symbol((String)sbflush(sb));		   /*                        */
}						   /*------------------------*/



/*****************************************************************************/
/***									   ***/
/*****************************************************************************/

/*-----------------------------------------------------------------------------
** Function:	get_item()
** Purpose:	Return the print representation of a \BibTeX{} string.
**		The appearance is determined by the |items| mapping.
**		If no appropriate entry is found then |type| is used to
**		decide whether the item should be returned as
**		upper-case, lower-case or first upper only.
** Arguments:
**	name	Symbol to get the print representation for.
**	type	One of the values |SYMBOL_TYPE_UPPER|, |SYMBOL_TYPE_LOWER|, or
**		|SYMBOL_TYPE_CASED| as they are defined in |type.h|.
** Returns:	A pointer to a static string. This location  is reused
**		upon the next invocation of this function.
**___________________________________________________			     */
Symbol get_item(name, type)			   /*                        */
  Symbol name;				   	   /*                        */
  int    type;				   	   /*                        */
{ return get_mapped_or_cased(name, items, type);   /*                        */
}						   /*------------------------*/


/*****************************************************************************/
/***									   ***/
/*****************************************************************************/

 static Macro keys = MacroNULL;			   /*                        */

/*-----------------------------------------------------------------------------
** Function:	save_key()
** Purpose:	Save a mapping of a lower-case key to a printed
**		representation.
** Arguments:
**	name	the name of the key in lower
**	key	the key as printed
** Returns:	nothing
**___________________________________________________			     */
void save_key(name, key)			   /*                        */
  Symbol name;				   	   /*                        */
  Symbol key;					   /*                        */
{ keys = new_macro(name, key, keys, 1);		   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	get_key()
** Purpose:	Get the printable representation of a key. If a special
**		representation has been registered then this
**		representation is returned. Otherwise the (lower-case)
**		key is returned.
** Arguments:
**	name	the name of the key to find. This must be in lower-case
** Returns:	the requested representation
**___________________________________________________			     */
Symbol get_key(name)			   	   /*                        */
  Symbol name;			   	   	   /*                        */
{ return get_mapped_or_cased(name,	   	   /*                        */
			     keys,	   	   /*                        */
			     SYMBOL_TYPE_LOWER);   /*                        */
}						   /*------------------------*/
