/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.2  1996/10/02 16:36:06  digulla
    Fixed a couple of typos in TEST

    Revision 1.1  1996/10/02 16:32:58  digulla
    Remove a node from a single linked list

    Revision 1.1  1996/08/01 18:46:31  digulla
    Simple string compare function

    Desc:
    Lang:
*/
#include <aros/system.h>
#include <clib/aros_protos.h>

/*****************************************************************************

    NAME */
	#include <clib/aros_protos.h>

	APTR RemoveSList (

/*  SYNOPSIS */
	APTR * list,
	APTR   node)

/*  FUNCTION
	Remove the node from a single linked list.

    INPUTS
	list - Pointer to the pointer which contains the first element
		of the single linked list.
	node - The node which is to be removed.

    RESULT
	Returns the node if it was in the list.

    NOTES
	This function is not part of a library and may thus be called
	any time.

    EXAMPLE
	@atend

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	24-12-95    digulla created

******************************************************************************/
{
    while (*list && *list != node)
	list = (APTR *)(*list);

    if (*list)
    {
	*list = *((APTR *)node);
	*((APTR *)node) = NULL;

	return node;
    }

    return NULL;
} /* RemoveSList */

#ifdef TEST
#include <stdio.h>

/* A single linked list looks like this: */
struct SNode
{
    /* No data before this entry ! */
    struct SNode * Next;
    int data;
};

struct SNode * List;
struct SNode node1, node2;

int main (int argc, char ** argv)
{
    struct SNode * ptr;

    List = &node1;
    node1.Next = &node2;
    node2.Next = NULL;

    ptr = RemoveSList ((APTR *)&List, &node2);
    if (ptr != &node2)
	fprintf (stderr, "Error: Couldn't find node2\n");
    else
	printf ("node2 removed.\n");

    ptr = RemoveSList ((APTR *)&List, &node1);
    if (ptr != &node1)
	fprintf (stderr, "Error: Couldn't find node1\n");
    else
	printf ("node1 removed.\n");

    if (List)
	fprintf (stderr, "Error: List is not empty\n");
    else
	printf ("List is now empty.\n");

    printf ("End.\n");
}
#endif

