/*
** LCLint - annotation-assisted static program checker
** Copyright (C) 1994-2000 University of Virginia,
**         Massachusetts Institute of Technology
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
** 
** The GNU General Public License is available from http://www.gnu.org/ or
** the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
** MA 02111-1307, USA.
**
** For information on lclint: lclint-request@cs.virginia.edu
** To report a bug: lclint-bug@cs.virginia.edu
** For more information: http://lclint.cs.virginia.edu
*/
/*
** flagMarkerList.c
**
** based on list_template.c
**
** where T has T_equal (or change this) and T_unparse
**
** invariant: flagMarker's are listed in order
*/

# include "lclintMacros.nf"
# include "basic.h"

flagMarkerList
  flagMarkerList_new ()
{
  flagMarkerList s = (flagMarkerList) dmalloc (sizeof (*s));
  
  s->nelements = 0;
  s->nspace = flagMarkerListBASESIZE; 
  s->elements = (flagMarker *)
    dmalloc (sizeof (*s->elements) * flagMarkerListBASESIZE);

  return (s);
}

static void
flagMarkerList_grow (flagMarkerList s)
{
  int i;
  flagMarker *newelements;
  
  s->nspace += flagMarkerListBASESIZE; 

  newelements = (flagMarker *) dmalloc (sizeof (*newelements) 
					* (s->nelements + s->nspace));

  for (i = 0; i < s->nelements; i++)
    {
      newelements[i] = s->elements[i];
    }
  
  sfree (s->elements);
  s->elements = newelements;
}

void flagMarkerList_add (flagMarkerList s, flagMarker fm)
{
  int i = s->nelements - 1;

  
  if (i > 0)
    {
      flagMarker last = s->elements[i];

      
      if (flagMarker_isIgnoreCount (last))
	{
	  if (!flagMarker_isIgnoreOff (fm))
	    {
	      if (flagMarker_isLocalSet (fm))
		{
		  llforceerror 
		    (FLG_WARNFLAGS,
		     cstring_makeLiteral ("Cannot set flag inside ignore "
					  "count region."),
		     flagMarker_getLoc (fm));
		  llgenindentmsg 
		    (cstring_makeLiteral ("Ignore count region starts"),
		     flagMarker_getLoc (last));

		}
	      else 
		{
		  if (flagMarker_isIgnoreOn (fm)) 
		    {
		      llforceerror 
			(FLG_WARNFLAGS,
			 cstring_makeLiteral ("Cannot nest ignore regions."),
			 flagMarker_getLoc (fm));
		      llgenindentmsg 
			(cstring_makeLiteral ("Previous ignore region starts"),
			 flagMarker_getLoc (last));
		    }
		}

	      flagMarker_free (fm);
	      return;
	    }
	}
      else 
	{
	  if (flagMarker_isIgnoreOff (last))
	    {
	      flagMarker nlast = s->elements [i - 1];
	      
	      if (flagMarker_isIgnoreCount (nlast))
		{
		  if (fileloc_sameFile (flagMarker_getLoc (fm),
					flagMarker_getLoc (last))
		      && fileloc_notAfter (flagMarker_getLoc (fm), 
					   flagMarker_getLoc (last)))
		    {
		      if (flagMarker_isLocalSet (fm))
			{
			  llforceerror 
			    (FLG_WARNFLAGS,
			     cstring_makeLiteral ("Cannot set flag inside ignore "
						  "count region."),
			     flagMarker_getLoc (fm));
			  llgenindentmsg 
			    (cstring_makeLiteral ("Ignore count region starts"),
			     flagMarker_getLoc (nlast));
			  
			}
		      else 
			{
			  if (flagMarker_isIgnoreOn (fm)) 
			    {
			      llforceerror 
				(FLG_WARNFLAGS,
				 cstring_makeLiteral ("Cannot nest ignore regions."),
				 flagMarker_getLoc (fm));
			      llgenindentmsg 
				(cstring_makeLiteral ("Previous ignore region starts"),
				 flagMarker_getLoc (nlast));
			    }
			}
		      
		      flagMarker_free (fm);
		      return;
		    }
		}
	    }
	}
    }

  
  /*
  ** all this code is necessary to check the invariant is preserved
  */

  while (i > 0
	 && !flagMarker_sameFile (s->elements[i],
				  flagMarker_getLoc (fm))) 
    {
      i--;
    }

  /*
  ** reprocessing header file, okay to be out of order
  */

  if (i >= 0 && !fileloc_isHeader (flagMarker_getLoc (fm)))
    {
                  
      /*
      llassert (!flagMarker_beforeMarker (s->elements[i], 
					  flagMarker_getLoc (fm)));
					  */
    }

  if (s->nspace <= 0)
    {
      flagMarkerList_grow (s);
    }
  
  s->nspace--;
  s->elements[s->nelements] = fm;
  s->nelements++;
  }

void flagMarkerList_checkSuppressCounts (flagMarkerList s)
{
  int nexpected = 0;
  int nsuppressed = 0;
  fileloc loc = fileloc_undefined;
  bool inCount = FALSE;
  int i;

  
  for (i = 0; i < s->nelements; i++)
    {
      flagMarker current = s->elements[i];

      if (flagMarker_isIgnoreCount (current))
	{
	  llassert (!inCount);
	  inCount = TRUE;
	  nexpected = flagMarker_getCount (current);
	  loc = flagMarker_getLoc (current);
	  nsuppressed = 0;
	  	}
      else if (flagMarker_isIgnoreOff (current))
	{
	  if (inCount)
	    {
	      inCount = FALSE;
	      llassert (fileloc_isDefined (loc));

	      if (nexpected > 0 && nexpected != nsuppressed)
		{
		  llforceerror 
		    (FLG_SUPCOUNTS,
		     message 
		     ("Line expects to suppress %d error%p, found %d error%p",
		      nexpected, nsuppressed),
		     loc);
		}
	    }
	}
      else if (flagMarker_isSuppress (current))
	{
	  nsuppressed++;
	}
      else
	{
	  ;
	}
    }

  llassert (!inCount);
}

static void flagMarkerList_splice (flagMarkerList s, 
				   int index,
				   /*@keep@*/ flagMarker fm)
{
  fileloc loc = flagMarker_getLoc (fm);
  fileloc beforeloc, afterloc;
  int i;

  llassert (index >= 0 && (index + 1 < s->nelements));
  
  beforeloc = flagMarker_getLoc (s->elements[index]);
  afterloc = flagMarker_getLoc (s->elements[index + 1]);;
  
  llassert (fileloc_sameFile (beforeloc, loc));
  llassert (fileloc_sameFile (afterloc, loc));

  if (s->nspace <= 0)
    {
      flagMarkerList_grow (s);
    }
  
  for (i = s->nelements; i > index + 1; i--)
    {
      s->elements[i] = s->elements[i - 1];
    }

  s->elements[index + 1] = fm;
  s->nelements++;
  s->nspace--;

  }

/*@only@*/ cstring
flagMarkerList_unparse (flagMarkerList s)
{
   int i;
   cstring st = cstring_makeLiteral ("[");

   for (i = 0; i < s->nelements; i++)
     {
       if (i == 0)
	 {
	   st = message ("%q %q", st, flagMarker_unparse (s->elements[i]));
	 }
       else
	 st = message ("%q, %q", st, flagMarker_unparse (s->elements[i]));
     }
   
   st = message ("%q ]", st);
   return st;
}

void
flagMarkerList_free (flagMarkerList s)
{
  int i;
  for (i = 0; i < s->nelements; i++)
    {
      flagMarker_free (s->elements[i]);
    }
  
  sfree (s->elements); 
  sfree (s);
}

/*
** returns YES iff
**    > in ignore region (there is an ignore ON marker not followed by OFF)
**    > code is OFF (-)
**
** returns NO iff
**    > not in ignore region
**    > code is ON (+)
**
** returns MAYBE iff
**    > not in ignore region
**    > code is unset or =
**
** requires: invariant for flagMarkerList:
**    flagMarker's are sorted by line and col
*/

static int
flagMarkerList_lastBeforeLoc (flagMarkerList s, fileloc loc)
{
  int i;

  for (i = s->nelements - 1; i >= 0; i--) 
    {
      flagMarker current = s->elements[i];
      
      if (fileloc_sameFile (current->loc, loc) 
	  && (!flagMarker_beforeMarker (current, loc)))
	{
	  return i;
	}
/*
      if (flagMarker_sameFile (current, loc) 
	  && (!flagMarker_beforeMarker (current, loc)))
	{
	  return i;
	}
*/
    }

  return -1;
}
	  
ynm
flagMarkerList_suppressError (flagMarkerList s, flagcode code, fileloc loc)
{
  int i;
  bool ignoreOff = FALSE;
  bool nameChecksOff = FALSE;
  bool flagOff = FALSE;
  ynm flagSet = MAYBE;
  bool islib = FALSE;
  bool isNameChecksFlag = flagcode_isNameChecksFlag (code);

  if (fileloc_isLib (loc))
    {
      i = s->nelements - 1;
      islib = TRUE;
    }
  else
    {
      i = flagMarkerList_lastBeforeLoc (s, loc);
    }

  
  if (i < 0)
    {
      return MAYBE;
    }
  
  /*
  ** Go backwards through the remaining flagMarkers in this file.
  */

  for (; i >= 0; i--) 
    {
      flagMarker current = s->elements[i];

      
      if (!islib && !flagMarker_sameFile (current, loc))
	{
	  break;
	}

      if (flagMarker_isIgnoreOff (current))
	{
	  ignoreOff = TRUE;
	}
      else if (flagMarker_isIgnoreOn (current))
	{
	  if (!ignoreOff)
	    {
	      return YES;
	    }
	}
      else if (flagMarker_isIgnoreCount (current))
	{
	  if (!ignoreOff)
	    {
	      flagMarkerList_splice (s, i,
				     flagMarker_createSuppress (code, loc));
	      return YES;
	    }
	}
      else if (flagMarker_isLocalSet (current))
	{
	  
	  if (!flagOff && flagMarker_getCode (current) == code)
	    {
	      ynm set  = flagMarker_getSet (current);
	      
	      if (ynm_isOff (set))
		{
		  return YES;
		}
	      else
		{
		  if (ynm_isOn (set))
		    {
		      flagOff = TRUE;
		      flagSet = NO;
		    }
		  else
		    {
		      		      flagOff = TRUE;
		      flagSet = MAYBE;
		    }
		  
		  if (ignoreOff)
		    {
		      if (isNameChecksFlag && !nameChecksOff)
			{
			  ;
			}
		      else
			{
			  return flagSet;
			}
		    }
		}
	    }
	  
	  if (flagMarker_getCode (current) == FLG_NAMECHECKS
	      && !nameChecksOff && isNameChecksFlag)
	    {
	      ynm set  = flagMarker_getSet (current);
	      
	      if (ynm_isOff (set))
		{
		  return YES;
		}
	      else
		{
		  if (ynm_isOn (set))
		    {
		      nameChecksOff = TRUE;
		      flagSet = NO;
		    }
		  else
		    {
		      		      nameChecksOff = TRUE;
		      flagSet = MAYBE;
		    }
		  
		  if (ignoreOff && flagOff)
		    {
		      return flagSet;
		    }
		}
	    }
	}
      else
	{
	  llassert (flagMarker_isSuppress (current));
	}
    }
  
  return flagSet;
}

bool
flagMarkerList_inIgnore (flagMarkerList s, fileloc loc)
{
  int i;

  if (fileloc_isLib (loc))
    {
      return FALSE;
    }

  i = flagMarkerList_lastBeforeLoc (s, loc);
  
  /*
  ** Go backwards through the remaining flagMarkers in this file.
  */

  for (; i >= 0; i--) 
    {
      flagMarker current = s->elements[i];
      
      if (!flagMarker_sameFile (current, loc))
	{
	  break;
	}

      if (flagMarker_isIgnoreOff (current))
	{
	  return FALSE;;
	}
      else if (flagMarker_isIgnoreOn (current))
	{
	  return TRUE;
	}
      else if (flagMarker_isIgnoreCount (current))
	{
	  flagMarkerList_splice (s, i,
				 flagMarker_createSuppress (SKIP_FLAG, loc));
	  return TRUE;
	}
      else
	{
	  llassert (flagMarker_isLocalSet (current)
		    || flagMarker_isSuppress (current));
	}
    }
  
  return FALSE;
}

