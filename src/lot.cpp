/*
 * lot.cpp
 * 
 * Created by Marius Utheim on 27.05.2011
 * All rights reserved
 *
 */

#include <stdlib.h>
#include "lot.h"

typedef struct lotnode lotnode_t;

struct lotnode {
	lotnode_t *next;
	void *item;
	int place;
};

struct lot {
	lotnode_t *head;
	int count;
};

struct lot_iterator {
	lot_t *lot;
	lotnode_t *next;
};


lot_t *lot_create()
{
	lot_t *newlot = (lot_t*)malloc(sizeof(lot_t));
	newlot->head = NULL;
	newlot->count = 0;
	return newlot;
}

void lot_destroy(lot_t *lot)
{
	lotnode_t *next = lot->head, *current;

	while ((current = next) != NULL)
	{
		next = current->next;
		free(current);
	}

	free(lot);
}

int lot_count(lot_t *lot)
{
	return lot->count;
}

int lot_add(lot_t *lot, void *item)
{
	lotnode_t *newnode = (lotnode_t*)malloc(sizeof(lotnode_t));
	newnode->item = item;

	if (lot->count == 0 || lot->head->place > 0)
	{
		if (lot->count == 0)
			newnode->next = NULL;
		else
			newnode->next = lot->head;
		newnode->place = 0;
		lot->head = newnode;
		lot->count++;
		return 0;
	}

	int place = 0;
	lotnode_t *current, *next;
	next = lot->head;
		
	while (current = next)
	{
		place++;
		next = next->next;

		if (next == NULL)
		{
			current->next = newnode;
			newnode->next = NULL;
			newnode->place = place;
			lot->count++;
			return place;
		}
		else if (next->place > place)
		{
			newnode->next = next;
			current->next = newnode;
			newnode->place = place;
			lot->count++;
			return place;
		}
	}

	throw "Unreachable code";
}

void *lot_at(lot_t *lot, int position)
{
	if (lot->count == 0)
		return NULL;

	lotnode_t *current = lot->head;
	for (int i = 0; i < position; i++)
		if ((current = current->next) == NULL)
			return NULL;

	return current->item;
}

void *lot_place(lot_t *lot, int place)
{
	if (lot->count == 0)
		return NULL;

	lotnode_t *current = lot->head;

	current = lot->head;
	do 
	{
		if (current->place == place)
			return current->item;

		if ((current = current->next) == NULL)
			return NULL;
	}
	while (current->place <= place);

	return NULL;
}

void *lot_remove(lot_t *lot, void *item)
{
	if (lot->count == 0)
		return NULL;

	lotnode_t *target;

	if (lot->head->item == item)
	{
		target = lot->head;
		lot->head = lot->head->next;
		lot->count--;
		free(target);
		return item;
	}

	lotnode_t *current, *next;
	current = next = lot->head;

	while ((next = next->next) != NULL)
	{
		if (next->item == item)
		{
			target = next;
			current->next = next->next;
			lot->count--;
			free(next);
			return item;
		}

		current = next;
	}

	return NULL;
}

void *lot_remove_at(lot_t *lot, int position)
{
	if (lot->count == 0)
		return NULL;

	lotnode_t *current = lot->head;
	lotnode_t *target = current;
	void *item;

	if (position == 0)
	{
		target = lot->head;
		lot->head = lot->head->next;
		lot->count--;
		item = target->item;
		free(target);
		return item;
	}

	for (int i = 0; i < position; i++)
		if ((current = current->next)->next == NULL)
			return NULL;

	target = current->next;
	current = current->next->next;
	lot->count--;
	item = target->item;
	free(target);
	return item;
}

void *lot_remove_place(lot_t *lot, int place)
{
	if (lot->count == 0)
		return NULL;

	lotnode_t *target;
	lotnode_t *current = lot->head;
	lotnode_t *next = lot->head;
	void *item;

	if (lot->head->place == place)
	{
		target = lot->head;
		lot->head = lot->head->next;
		lot->count--;
		item = target->item;
		free(target);
		return item;
	}

	while ((next = next->next) != NULL)
	{
		if (next->place == place)
		{
			target = next;
			current->next = next->next;
			lot->count--;
			item = next->item;
			free(next);
			return item;
		}
		else if (next->place > place)
			return NULL;

		current = next;
	}

	return NULL;
}


lot_iterator_t *lot_iterator_create(lot_t *lot)
{
	lot_iterator_t *newiterator = (lot_iterator_t*)malloc(sizeof(lot_iterator_t));
	newiterator->lot = lot;
	newiterator->next = lot->head;
	return newiterator;
}

void lot_iterator_destroy(lot_iterator_t *iterator)
{
	free(iterator);
}

void lot_iterator_reset(lot_iterator_t *iterator)
{
	iterator->next = iterator->lot->head;
}

void *lot_next(lot_iterator_t *iterator, int *place)
{
	if (iterator->next == NULL)
		return NULL;

	void *next = iterator->next->item;
	if (place != NULL)
		*place = iterator->next->place;
	iterator->next = iterator->next->next;
	return next;
}