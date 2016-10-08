//Functions for dealing with linked lists of goodies
//Functions with names starting with an "_" have companion #defines in rogue.h which take the address of the first argument and pass it on.
//list.c      1.4 (A.I. Design) 12/5/85

#include "rogue.h"

extern THING *_things;
extern int *_t_alloc;

//_detach: Takes an item out of whatever linked list it might be in
_detach(THING **list, THING *item)
{
  if (*list==item) *list = next(item);
  if (prev(item)!=NULL) item->l_prev->l_next = next(item);
  if (next(item)!=NULL) item->l_next->l_prev = prev(item);
  item->l_next = NULL;
  item->l_prev = NULL;
}

//_attach: add an item to the head of a list
_attach(THING **list, THING *item)
{
  if (*list!=NULL) {item->l_next = *list; (*list)->l_prev = item; item->l_prev = NULL;}
  else {item->l_next = NULL; item->l_prev = NULL;}
  *list = item;
}

//_free_list: Throw the whole blamed thing away
_free_list(THING **ptr)
{
  THING *item;

  while (*ptr!=NULL) {item = *ptr; *ptr = next(item); discard(item);}
}

//new_item: Get a new item with a specified size
THING *new_item()
{
  THING *item;

#ifdef DEBUG
  if ((item = (THING *)talloc())==NULL) if (me()) msg("no more things!"); else
#else
  if ((item = (THING *)talloc())!=NULL)
#endif DEBUG

  item->l_next = item->l_prev = NULL;
  return item;
}

//talloc: simple allocation of a THING
talloc()
{
  int i;

  for (i = 0; i<MAXITEMS; i++)
  {
    if (_t_alloc[i]==0)
    {
      if (++total>maxitems) maxitems = total;
      _t_alloc[i]++;
      setmem(&_things[i], sizeof(THING), 0);
      return &_things[i];
    }
  }
  return NULL;
}

//discard: Free up an item
discard(THING *item)
{
  int i;

  for (i = 0; i<MAXITEMS; i++)
  {
    if (item==&_things[i]) {--total; _t_alloc[i] = 0; return 1;}
  }
  return NULL;
}
