/*
    Copyright © 2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Code for CONU_CHARMAP console units.
    Lang: english
*/

#include "charmap.h"
#include <proto/exec.h>
#include <string.h>

struct charmap_line * charmap_dispose_line(struct charmap_line * line)
{
  struct charmap_line * next = line->next;
  if (line->size) {
	if (line->text) FreeMem(line->text,line->size);
	if (line->fgpen) FreeMem(line->fgpen, line->size);
	if (line->bgpen) FreeMem(line->bgpen, line->size);
  }
  FreeMem(line,sizeof(struct charmap_line));
  return next;
}

VOID charmap_dispose_lines(struct charmap_line * line)
{
  while (line = charmap_dispose_line(line));
}

struct charmap_line * charmap_newline(struct charmap_line * next, struct charmap_line * prev)
{
  struct charmap_line * newline = (struct charmap_line *)AllocMem(sizeof(struct charmap_line),MEMF_ANY);
  newline->next = next;
  newline->prev = prev;
  newline->text = 0;
  newline->fgpen = 0;
  newline->bgpen = 0;
  newline->size = 0;
  return newline;
}


// FIXME: **much** faster if add capacity to charmap_line and allocate
// a few characters at a time
VOID charmap_extend(struct charmap_line * line, ULONG newsize)
{
  char * text = line->text;
  BYTE * fgpen = line->fgpen;
  BYTE * bgpen = line->bgpen;
  ULONG size = line->size;

  if (newsize) {
	line->text = (char *)AllocMem(newsize,MEMF_ANY);
	if (line->text) memset(line->text,0,newsize);
	line->fgpen = (BYTE *)AllocMem(newsize,MEMF_ANY);
	if (line->fgpen) memset(line->fgpen,0,newsize);
	line->bgpen = (BYTE *)AllocMem(newsize,MEMF_ANY);
	if (line->bgpen) memset(line->bgpen,0,newsize);
	line->size = newsize;
  }

  if (text && line->text) memcpy(line->text, text, size);
  if (fgpen && line->fgpen) memcpy(line->fgpen, fgpen,size);
  if (bgpen && line->bgpen) memcpy(line->bgpen, bgpen,size);

  if (text) FreeMem(text, size);
  if (fgpen) FreeMem(fgpen, size);
  if (bgpen) FreeMem(bgpen, size);
}

