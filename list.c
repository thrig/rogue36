/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	3.3 (Berkeley) 6/15/81
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the LICENSE file for full copyright and licensing information.
 */

#include <stdlib.h>
#include <string.h>

#include "rogue.h"

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */

void _detach(struct linked_list **list, struct linked_list *item)
{
    if (*list == item)
        *list = next(item);
    if (prev(item) != NULL)
        item->l_prev->l_next = next(item);
    if (next(item) != NULL)
        item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */

void _attach(struct linked_list **list, struct linked_list *item)
{
    if (*list != NULL) {
        item->l_next = *list;
        (*list)->l_prev = item;
        item->l_prev = NULL;
    } else {
        item->l_next = NULL;
        item->l_prev = NULL;
    }

    *list = item;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */

void _free_list(struct linked_list **ptr)
{
    struct linked_list *item;

    while (*ptr != NULL) {
        item = *ptr;
        *ptr = next(item);
        discard(item);
    }
}

/*
 * discard:
 *	free up an item
 */

void discard(struct linked_list *item)
{
    free(item->l_data);
    free(item);
}

/*
 * new_item
 *	get a new item with a specified size
 */

struct linked_list *new_item(size_t size)
{
    struct linked_list *item;

    item = (struct linked_list *) new(sizeof *item);
    item->l_data = new(size);
    item->l_next = item->l_prev = NULL;
    memset(item->l_data, 0, size);
    return item;
}

char *new(size_t size)
{
    char *space = malloc(size);

    /* the xterm(1) running rogue is bigger than rogue, these days... */
    if (space == NULL)
        fatal("Rogue ran out of memory.  Fatal error!");
    return space;
}
