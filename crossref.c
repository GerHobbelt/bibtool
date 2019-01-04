/*** crossref.c ***************************************************************
** 
** This file is part of BibTool.
** It is distributed under the GNU General Public License.
** See the file COPYING for details.
**
** (c) 2007-2019 Gerd Neugebauer
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
**-----------------------------------------------------------------------------
** Description:
**	This module contains functions to expand crossref entries.
**
******************************************************************************/

#include <bibtool/general.h>
#include <bibtool/error.h>
#include <bibtool/crossref.h>
#include <bibtool/symbols.h>
#include <bibtool/s_parse.h>
#include <bibtool/database.h>
#include <bibtool/sbuffer.h>
#include <bibtool/rsc.h>
#include <bibtool/entry.h>

/*****************************************************************************/
/* Internal Programs                                                         */
/*===========================================================================*/

/*****************************************************************************/
/* External Programs                                                         */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

 typedef struct mAP				   /*                        */
 { rec_type src_rec;				   /*source record type index*/
   Symbol src_field;				   /*                        */
   rec_type dest_rec;				   /*target record type index*/
   Symbol dest_field;				   /*                        */
   struct mAP * next_map;			   /*                        */
 } *Map, SMap;					   /*                        */

#define SourceRecord(M)      ((M)->src_rec)
#define SourceField(M)       ((M)->src_field)
#define DestinationRecord(M) ((M)->dest_rec)
#define DestinationField(M)  ((M)->dest_field)
#define NextMap(M)	     ((M)->next_map)

/*-----------------------------------------------------------------------------
** Function*:	new_map()
** Type:	Map
** Purpose:	allocate a new Map structure
** Arguments:
**	sr	source record type index
**	sf	source field name
**	dr	destination record type index
**	df	destination field name
** Returns:	the new map
**___________________________________________________			     */
static Map new_map(sr,sf,dr,df)			   /*                        */
  rec_type sr;					   /*                        */
  Symbol sf;					   /*                        */
  rec_type dr;					   /*                        */
  Symbol df;					   /*                        */
{ Map m;					   /*                        */
  if ( (m = (Map)malloc(sizeof(SMap))) == (Map)NULL)/*                       */
  { OUT_OF_MEMORY("map"); }		   	   /*                        */
 						   /*                        */
  SourceRecord(m)      = sr;			   /*                        */
  SourceField(m)       = sf;			   /*                        */
  DestinationRecord(m) = dr;			   /*                        */
  DestinationField(m)  = df;			   /*                        */
  NextMap(m)           = NULL;			   /*                        */
  return m;					   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function*:	free_map()
** Type:	void
** Purpose:	Free a chain of map structures
** Arguments:
**	m	the map
** Returns:	nothing
**___________________________________________________			     */
static void free_map(m)				   /*                        */
  Map m;					   /*                        */
{ Map nxt;					   /*                        */
  while (m)					   /*                        */
  { nxt = NextMap(m);				   /*                        */
    free(m);				   	   /*                        */
    m = nxt;					   /*                        */
  }						   /*                        */
}						   /*------------------------*/

#define MAP_SIZE 101

 static Map map[MAP_SIZE];			   /*                        */

#define MAP_INDEX(SR,SF,DR)				\
  (int)((((SR) % 73) +					\
	 ((long)(SF) & 0xfff) +				\
	 ((DR) % 1023)) % MAP_SIZE);

/*-----------------------------------------------------------------------------
** Function:	clear_map()
** Type:	void
** Purpose:	Reset the map to it's initial state where no elements are
**		contained.
** Arguments:	none
** Returns:	nothing
**___________________________________________________			     */
void clear_map()				   /*                        */
{ register int i;				   /*                        */
  						   /*                        */
  for (i = 0; i < MAP_SIZE; i++)		   /*                        */
  { free_map(map[i]);				   /*                        */
    map[i] = NULL;				   /*                        */
  }				   		   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	map_add()
** Type:	int
** Purpose:	Add or overwrite a filed name mapping.
**		
** Arguments:
**	s_rec	the index of the source record type
**	s_fld	the source field name
**	d_rec	the index of the destination record type
**	d_fld	the destination field name
** Returns:	nothing
**___________________________________________________			     */
void map_add(s_rec,s_fld,d_rec,d_fld)		   /*                        */
  rec_type s_rec;				   /*                        */
  Symbol s_fld;					   /*                        */
  rec_type d_rec;				   /*                        */
  Symbol d_fld;					   /*                        */
{ int idx;					   /*                        */
  Map m;					   /*                        */
 						   /*                        */
  idx = MAP_INDEX(s_rec, s_fld, d_rec);		   /*                        */
  m = map[idx];					   /*                        */
  if (m == NULL )				   /*                        */
  { map[idx] = new_map(s_rec, s_fld, d_rec, d_fld);/*                        */
    return;					   /*                        */
  }						   /*                        */
 						   /*                        */
  for (;;)					   /*                        */
  { if (SourceRecord(m)      == s_rec &&	   /*                        */
	SourceField(m)       == s_fld &&	   /*                        */
	DestinationRecord(m) == d_rec )	   	   /*                        */
    { DestinationField(m) = d_fld;		   /*                        */
      return;					   /*                        */
    }		   				   /*                        */
    if (NextMap(m) == NULL )			   /*                        */
    { NextMap(m) = new_map(s_rec, s_fld,	   /*                        */
			   d_rec, d_fld);	   /*                        */
      return;					   /*                        */
    }						   /*                        */
    m = NextMap(m);				   /*                        */
  }						   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	map_get()
** Type:	Symbol
** Purpose:	Getter for a map element.
** Arguments:
**	s_rec	the index of the source entry type
**	s_fld	the name of the source field
**	d_rec	the index of the destination entry type
** Returns:	the new field name or |NO_SYMBOL|
**___________________________________________________			     */
Symbol map_get(s_rec, s_fld, d_rec)		   /*                        */
  rec_type s_rec;				   /*                        */
  Symbol s_fld;					   /*                        */
  rec_type d_rec;				   /*                        */
{ int i = MAP_INDEX(s_rec, s_fld, d_rec);	   /*                        */
  Map m	= map[i];	   			   /*                        */
						   /*                        */
  for (; m; m = NextMap(m))			   /*                        */
  { if (SourceRecord(m)      == s_rec &&	   /*                        */
	SourceField(m)       == s_fld &&	   /*                        */
	DestinationRecord(m) == d_rec )	   	   /*                        */
    { return DestinationField(m); }		   /*                        */
  }						   /*                        */
  return NO_SYMBOL;				   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	crossref_map()
** Type:	int
** Purpose:	
**		
** Arguments:
**	spec	the argument
** Returns:	nothing
**___________________________________________________			     */
void crossref_map(spec)				   /*                        */
  String spec;				   	   /*			     */
{ Symbol *src, *dest;				   /*                        */
  Symbol src_field, dest_field;			   /*                        */
  Symbol *sp, *dp;				   /*                        */
  rec_type s_rec, d_rec;			   /*                        */
 						   /*                        */
  sp_open(spec);				   /*                        */
  if ((src = sp_symbols(&spec)) == NULL) return;   /*                        */
  sp_skip(&spec);				   /*                        */
  if ((src_field = SParseSymbol(&spec)) == NULL) return;/*                   */
  sp_skip(&spec);				   /*                        */
  if ((dest = sp_symbols(&spec)) == NULL) return;  /*                        */
  sp_skip(&spec);				   /*                        */
  if ((dest_field = SParseSymbol(&spec)) == NULL) return;/*                  */
  sp_eos(&spec);			   	   /*                        */
  sp_close();					   /*                        */
 						   /*                        */
  for (sp = src; *sp; sp++)			   /*                        */
  { s_rec = find_entry_type(SymbolValue(*sp));	   /*                        */
    if (s_rec == BIB_NOOP)			   /*                        */
    { WARNING3("Unknown source entry type `",	   /*                        */
	       SymbolValue(*sp), 		   /*                        */
	       "'. Mapping ignored.");		   /*                        */
      continue;					   /*                        */
    }						   /*                        */
  						   /*                        */
    for (dp = dest; *dp; dp++)			   /*                        */
    { d_rec = find_entry_type(SymbolValue(*dp));   /*                        */
      if (d_rec == BIB_NOOP)			   /*                        */
      { WARNING3("Unknown destination entry type `",/*                       */
		 SymbolValue(*dp),		   /*                        */
		 "'. Mapping ignored.");	   /*                        */
	continue;				   /*                        */
      }						   /*                        */
 						   /*                        */
      map_add(s_rec, src_field, d_rec, dest_field);/*                        */
    }  						   /*                        */
  }						   /*                        */
 						   /*                        */
#ifndef COMPLEX_SYMBOL
  free(src);				   	   /*                        */
  free(dest);				   	   /*                        */
#endif
}						   /*------------------------*/

 static Symbol NONE = (Symbol)"x";		   /* TODO                   */

/*-----------------------------------------------------------------------------
** Function*:	insert_record()
** Type:	bool
** Purpose:	Find a record and insert some new entryies.
** Arguments:
**	db	the database
**	rec	the record
**	hp	the heap pointer
**	s	the key of the entry
**	msg	the message prefix for an unknown entry message
** Returns:	|true| iff the record could be inserted
**___________________________________________________			     */
static bool insert_record(db, rec, hp, s, msg)	   /*                        */
  DB     db;					   /*                        */
  Record rec;					   /*                        */
  register Symbol *hp;				   /*			     */
  Symbol s;					   /*                        */
  String msg;					   /*                        */
{ Record r;					   /*                        */
  Symbol t, ms;					   /*                        */
  int    i;					   /*                        */
 						   /*                        */
  if ((r = db_find(db, s)) == RecordNULL)	   /*			     */
  { ERROR3(msg," entry not found: ",		   /*                        */
	   (char*)SymbolValue(s)); 	   	   /*		             */
    return false;				   /*			     */
  }						   /*			     */
    						   /*                        */
  for (i = RecordFree(r), hp = RecordHeap(r);  	   /* visit all fields       */
       i > 0;				   	   /*			     */
       i -= 2)			   	   	   /*			     */
  { s = *hp++;			   	   	   /*                        */
    t = *hp++;			   	   	   /*                        */
    if (t != NO_SYMBOL)			   	   /*                        */
    { ms = map_get(RecordType(r), s,	   	   /*                        */
		   RecordType(rec));	   	   /*                        */
      provide_to_record(rec, ms ? ms : s, t);  	   /*                        */
    }	   				   	   /*                        */
  }						   /*                        */
  return true;					   /*                        */
}						   /*------------------------*/

/*-----------------------------------------------------------------------------
** Function:	expand_crossref()
** Purpose:	Expand all items inherited via a crossref.
** Arguments:
**	db	Database containing the entries.
**	rec	The record to expand
** Returns:	|false| iff an error has occured
**___________________________________________________			     */
bool expand_crossref(db, rec)		   	   /*                        */
  DB     db;					   /*                        */
  Record rec;					   /*                        */
{ register Symbol *hp;				   /*			     */
  register int    i;				   /*                        */
  String	  x;				   /*                        */
  Symbol	  s;			   	   /*			     */
  Record          r	= rec;			   /*                        */
  int             limit	= rsc_xref_limit;	   /*                        */
  Symbol	  xdata    = (rsc_expand_xdata	   /*                        */
			      ? sym_xdata 	   /*                        */
			      : NONE);		   /*                        */
  Symbol	  crossref = (rsc_expand_crossref  /*                        */
			      ? sym_crossref	   /*                        */
			      : NONE);		   /*                        */
 						   /*                        */
  DebugPrint1("expand_crossref");		   /*                        */
 						   /*                        */
  while	(RecordIsXREF(r) && limit-- >= 0)	   /*                        */
  {						   /*                        */
    for (i = RecordFree(r), hp = RecordHeap(r);    /* search crossref field  */
	 i > 0					   /*                        */
	   && (*hp != crossref && *hp != xdata);   /*			     */
	 i -= 2, hp += 2)			   /*			     */
    { }					   	   /*			     */
 						   /*                        */
    if (i <= 0)					   /*                        */
    { DebugPrint1("*** No crossref found.");	   /*			     */
      return false;				   /*			     */
    }						   /*                        */
    else if (*hp == sym_crossref)		   /* ---------------------- */
    {						   /*                        */
      if (rec == r) { *hp = NULL; }		   /* Delete the first xref  */
      x = SymbolValue(*++hp);			   /*                        */
      x++;				   	   /*			     */
      sp_open(x);		   		   /* Try to extract	     */
      if ((s = SParseSymbol(&x)) ==  NO_SYMBOL)	   /*  the crossref as symbol*/
      { return false; }				   /*			     */
						   /*			     */
      insert_record(db, rec, hp, s, "Crossref");   /*                        */
    }						   /*			     */
    else if (*hp == sym_xdata)			   /* ---------------------- */
    {						   /*                        */
      *hp = NULL;		   		   /* Delete the first xref  */
      x = SymbolValue(*++hp);			   /*                        */
      x++;				   	   /*			     */
      sp_open(x);		   		   /* Try to extract	     */
 						   /*                        */
      if (sp_expect(&x,	(String)"}", 0) )	   /*                        */
	return false;				   /*                        */
      for (;;)					   /*                        */
      { if ((s = SParseSymbol(&x)) == NO_SYMBOL)   /*                        */
	{ return true; }			   /*                        */
 						   /*                        */
	insert_record(db, rec, hp, s, "XData");	   /*                        */
 						   /*                        */
	if (sp_expect(&x, (String)"}", 0) ) break; /*                        */
	sp_expect(&x, (String)",", 1);		   /*                        */
      }						   /*                        */
    }						   /*                        */
  }						   /*                        */
  return false;	   				   /*			     */
}						   /*------------------------*/
