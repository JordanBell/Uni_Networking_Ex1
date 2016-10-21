#include "types.h"

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#define LL_TYPE c_connection_data*

// --- Data structures ---
typedef struct list
{
	LL_TYPE m_iData;
	struct list* m_pNext;
} list;

// --- Functions ---

/*
 * Function to initialize a new list
 *
 * #Argument
 * * `l`    - The list to be initialized
 *
 */
void init(list * const l);

/*
 * Function to free a list when it is not needed anymore
 *
 * #Argument
 * * `l`    - The list to be destroyed
 *
 */
void list_destroy(list * const l);

/*
 * Function to get `data` of element number `index`.
 *
 * #Arguments
 * * `l`    - The list to be modified
 * * `index`   - the index of the searched element.
 *
 * #Return
 * Returns the `data` field of the element or -1 in case of errors.
 *
 */
LL_TYPE get(list * const l, unsigned int index);

int iterate_to_index(list ** l, unsigned int index);

/*
 * Function to prepend a new element to the list.
 *
 * #Arguments
 * * `l`    - The list to be modified
 * * `data`   - the integer to add to the front of the linkedlist.
 *
 * #Return
 * Returns 0 for successful termination and -1 in case of errors.
 *
 */
int prepend(list * const l, LL_TYPE data);

/*
 * Function to append a new element to the list.
 *
 * #Arguments
 * * `l`    - The list to be modified
 * * `data`   - the integer to add to the back of the linkedlist.
 *
 * #Return
 * Returns 0 for successful termination and -1 in case of errors.
 *
 */
int append(list * const l, LL_TYPE data);

/*
 * Function to insert a new element to the list.
 *
 * #Arguments
 * * `l`    - The list to be modified
 * * `index`  - the index after which the new element should be inserted.
 * * `data`   - the integer which should be stored in the new element.
 *
 * #Return
 * Returns 0 for successful termination and -1 in case of errors.
 *
 */
int insert(list * const l, unsigned int index, LL_TYPE data);

/*
 * Function to delete an existing element from the list.
 *
 * #Arguments
 * * `l`    - The list to be modified
 * * `index`   - the index of the element to remove from the linkedlist.
 *
 * #Return
 * Returns 0 for successful termination and -1 in case of errors.
 *
 */
int remove_element(list * const l, unsigned int index);

/*
 * Function to print all elements of the list.
 *
 * #Arguments
 * * `l`      - the list to print.
 */
void print_list(list const * const l);

void back(list ** l);

int size(list const* const l);

int empty(list const* const l);

#endif
