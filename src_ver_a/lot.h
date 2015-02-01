/*
 * lot.h
 * 
 * Created by Marius Utheim on 27.05.2011
 * All rights reserved
 *
 */

#ifndef LOT_H_
#define LOT_H_

/*
 * The lot is a data structure similar to lists, but each entry has
 * a 'place' associated with it. When adding an entry, its place will
 * be the smallest integer that is not used by any other entry in the
 * lot. This means the lot maps from void-pointers to integers, with
 * a guaranteed one-to-one relationship.
 */

struct lot;
typedef struct lot lot_t;
struct lot_iterator;
typedef struct lot_iterator lot_iterator_t;

lot_t *lot_create();
void lot_destroy(lot_t *lot);
int lot_count(lot_t *lot);
int lot_add(lot_t *lot, void *item);
void *lot_at(lot_t *lot, int position);
void *lot_place(lot_t *lot, int place);
void *lot_remove(lot_t *lot, void *item);
void *lot_remove_at(lot_t *lot, int position);
void *lot_remove_place(lot_t *lot, int place);

lot_iterator_t *lot_iterator_create(lot_t *lot);
void lot_iterator_destroy(lot_iterator_t *iterator);
void lot_iterator_reset(lot_iterator_t *iterator);
void *lot_next(lot_iterator_t *iterator, int *place);

#endif /* LOT_H_ */