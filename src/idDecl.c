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
** idDecl.c
*/

# include "lclintMacros.nf"
# include "basic.h"

/*@only@*/ idDecl
  idDecl_create (/*@only@*/ cstring s, /*@only@*/ qtype t)
{
  idDecl d = (idDecl) dmalloc (sizeof (*d));

  d->id = s;
  d->typ = t;

  return (d);
}

void
idDecl_free (idDecl t)
{
  if (idDecl_isDefined (t))
    {
      cstring_free (t->id);
      qtype_free (t->typ);
      sfree (t);
    }
}

cstring
idDecl_unparse (idDecl d)
{
  if (idDecl_isDefined (d))
    {
      return (message ("%s : %q", d->id, qtype_unparse (d->typ)));
    }
  else
    {
      return (cstring_makeLiteral ("<undefined id>"));
    }
}

/*@observer@*/ cstring
idDecl_observeId (idDecl d)
{
  if (idDecl_isDefined (d))
    {
      return (d->id);
    }
  else
    {
      return cstring_undefined;
    }
}

qtype
idDecl_getTyp (idDecl d)
{
  llassert (idDecl_isDefined (d));

  return d->typ;
}

ctype
idDecl_getCtype (idDecl d)
{
  if (idDecl_isDefined (d))
    {
      return (qtype_getType (d->typ));
    }
  else
    {
      return ctype_unknown;
    }
}

qualList
idDecl_getQuals (idDecl d)
{
  if (idDecl_isDefined (d))
    {
      return (qtype_getQuals (d->typ));
    }
  else
    {
      return qualList_undefined;
    }
}

void
idDecl_addQual (idDecl d, qual q)
{
  llassert (idDecl_isDefined (d));

  (void) qtype_addQual (d->typ, q);
}

void
idDecl_setTyp (idDecl d, qtype c)
{
  llassert (idDecl_isDefined (d));

  qtype_free (d->typ);
  d->typ = c;
}

idDecl
idDecl_replaceCtype (/*@returned@*/ idDecl d, ctype c)
{
  llassert (idDecl_isDefined (d));

  qtype_setType (d->typ, c);
  return d;
}

idDecl
idDecl_fixBase (/*@returned@*/ idDecl t, qtype b)
{
  llassert (idDecl_isDefined (t));

  t->typ = qtype_newQbase (t->typ, b);
  return t;
}

idDecl
idDecl_fixParamBase (/*@returned@*/ idDecl t, qtype b)
{
  qtype q;
  ctype c;

  llassert (idDecl_isDefined (t));

  q = qtype_newQbase (t->typ, b);
  c = qtype_getType (q);

  /*
  ** For some reason, C adds an implicit pointer to function
  ** parameters.  It is "optional" syntax.
  */

  if (ctype_isFunction (c) && !ctype_isPointer (c))
    {
      qtype_setType (q, ctype_makePointer (c));
    }

  t->typ = q;
  /* LCLint thinks t->typ is kept. */
  /*@-compmempass@*/ return t; /*@=compmempass@*/
}

idDecl
idDecl_expectFunction (/*@returned@*/ idDecl d)
{
  llassert (idDecl_isDefined (d));

  qtype_setType (d->typ, ctype_expectFunction (qtype_getType (d->typ)));
  return d;
}
