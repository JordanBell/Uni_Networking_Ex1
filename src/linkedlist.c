/**
 *  Source file for a linked list in C
 *
 * @authors:   		Michael Denzel
 * @creation_date:	2016-09-05
 * @contact:		m.denzel@cs.bham.ac.uk
 */

//standard includes
#include <stdlib.h>
#include <stdio.h>

#define SUCCESS 0
#define FAILURE -1

//own includes
#include "linkedlist.h"

LL_TYPE get(list * const l, unsigned int index)
{
	list* pIndexNode = l;
	int iSuccessCode = iterate_to_index(&pIndexNode, index);
	if(iSuccessCode != SUCCESS || pIndexNode == NULL)
	{
		return 0;
	}

	return pIndexNode->m_iData;
}

int iterate_to_index(list ** l, unsigned int index)
{
	if(l == 0 || *l == 0)
	{
		return FAILURE;
	}

	int iListSize = size(*l);
	if(index >= iListSize)
	{
		return FAILURE;
	}

	// Start at the head
	*l = (*l)->m_pNext;
	int i;
	for(i = 0; i < index; ++i)
	{
		*l = (*l)->m_pNext;

		if(*l == 0)
		{
			// Error: Out of bounds
			return FAILURE;
		}
	}

	return SUCCESS;
}

int prepend(list * const l, LL_TYPE data)
{
	if(l == 0)
	{
		return FAILURE;
	}

		// Get the head (it makes no difference if this is null)
	list* pOldHead = l->m_pNext;

	// Add a new element as the head
	list* pNewHead = (list*)malloc(sizeof(list));
	if(pNewHead == 0)
	{
		return FAILURE;
	}

	init(pNewHead);
	pNewHead->m_iData = data;

	// Set the new head
	l->m_pNext = pNewHead;

	// Attach the old head to after the new one
	pNewHead->m_pNext = pOldHead;

	return SUCCESS;
}

int append(list * const l, LL_TYPE data)
{
	if(l == 0)
	{
		return FAILURE;
	}

	// Allocate memory to the back of the list
	list* pBack = l;
	back(&pBack);

	pBack->m_pNext = (list*)malloc(sizeof(list));
	if(pBack->m_pNext == 0)
	{
		return FAILURE;
	}

	// Set the data of the new node
	init(pBack->m_pNext);
	pBack->m_pNext->m_iData = data;

	return SUCCESS;
}

int remove_element(list * const l, unsigned int index)
{
	// Return failure for out-of-bounds
	if(index >= size(l))
	{
		return FAILURE;
	}

	// Get the element currently occupying the index before our target
	list* pOldIndexNodeParent = l;
	if(index > 0)
	{
		const int success_code = iterate_to_index(&pOldIndexNodeParent, index - 1);
		if(success_code != SUCCESS)
		{
			return success_code;
		}
	}


	// Get the node previously occupying this index
	list* pOldIndexNode = pOldIndexNodeParent->m_pNext;

	if(pOldIndexNode == 0)
	{
		// There is no object at the index (this is a failsafe, and should not trigger due to previous checks for out-of-bounds). This failure implies unexpected modification of node connectors.
		return FAILURE;
	}

	// Get the node after (it makes no difference if this is null)
	list* pOldIndexNodeChild = pOldIndexNode->m_pNext;

	// Destroy the node to remove
	free(pOldIndexNode);

	// Set the new connection
	pOldIndexNodeParent->m_pNext = pOldIndexNodeChild;

	return SUCCESS;
}

int insert(list * const l, unsigned int index, LL_TYPE data)
{
	// Return failure for out-of-bounds
	if(index >= size(l))
	{
		return FAILURE;
	}

	// Get the element currently occupying the index
	list* pIndexNode = l;
	int success_code = iterate_to_index(&pIndexNode, index);
	if(success_code != SUCCESS)
	{
		return success_code;
	}

	// Add a new element
	list* pNewIndexChild = (list*)malloc(sizeof(list));
	if(pNewIndexChild == 0)
	{
		return FAILURE;
	}
	init(pNewIndexChild);
	pNewIndexChild->m_iData = data;

	// Get the node after this index (it makes no difference if this is null)
	list* pOldIndexChild = pIndexNode->m_pNext;

	// Set the new connections
	pIndexNode->m_pNext = pNewIndexChild;
	pNewIndexChild->m_pNext = pOldIndexChild;

	return SUCCESS;
}

void print_list(list const* const l)
{
	#if 0
	if(empty(l))
	{
		printf("empty list\n");
		return;
	}

	list const* pIterator = l;
	while(pIterator->m_pNext != 0)
	{
		pIterator = pIterator->m_pNext;
		printf("%d ", pIterator->m_iData);
	}

	printf("\n");
	#endif
	printf("print_list :: Not implemented.");
}

void init(list * const l)
{
	l->m_pNext = 0;
}

void list_destroy(list * const l)
{
	if(empty(l))
	{
		// We do not need to destroy the head of the list.
		return;
	}

	list* pToFree;
	list* pIterator = l->m_pNext;
	while(pIterator)
	{
		// Save the pointer to this node to free
		pToFree = pIterator;

		// Loop to the next node
		pIterator = pIterator->m_pNext;

		// Free this node
		free(pToFree->m_iData);
		free(pToFree);
	}
}

void back(list ** l)
{
	while((*l)->m_pNext != 0)
	{
		*l = (*l)->m_pNext;
	}
}

int size(list const* const l)
{
	int i;
	list const* pIterator = l;
	for(i = 0; pIterator->m_pNext != 0; i++)
	{
		pIterator = pIterator->m_pNext;
	}

	return i;
}

int empty(list const* const l)
{
	return l->m_pNext == 0;
}
