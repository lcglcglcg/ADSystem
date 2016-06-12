
#include <stdio.h>
#include <string.h>
#include "lg_list.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define unlikely(x)     __builtin_expect(!!(x), 0)

/*
 * Returns a list organized in an intermediate format suited
 * to chaining of merge() calls: null-terminated, no reserved or
 * sentinel head node, "prev" links not maintained.
 */
lg_list_t *lg_list_merge(void *priv,
	int (*cmp)(void *priv, lg_list_t *a, lg_list_t *b),
	lg_list_t *a, lg_list_t *b)
{
    lg_list_t head, *tail = &head;

    while (a && b) {
	/* if equal, take 'a' -- important for sort stability */
	if ((*cmp)(priv, a, b) <= 0) {
	    tail->next = a;
	    a = a->next;
	} else {
	    tail->next = b;
	    b = b->next;
	}
	tail = tail->next;
    }
    tail->next = a?:b;
    return head.next;
}

/*
 * Combine final list merge with restoration of standard doubly-linked
 * list structure.  This approach duplicates code from merge(), but
 * runs faster than the tidier alternatives of either a separate final
 * prev-link restoration pass, or maintaining the prev links
 * throughout.
 */
void lg_list_merge_and_restore_back_links(void *priv,
	int (*cmp)(void *priv, lg_list_t *a, lg_list_t *b),
	lg_list_t *head, lg_list_t *a, lg_list_t *b)
{
    lg_list_t *tail = head;

    while (a && b) {
	/* if equal, take 'a' -- important for sort stability */
	if ((*cmp)(priv, a, b) <= 0) {
	    tail->next = a;
	    a->prev = tail;
	    a = a->next;
	} else {
	    tail->next = b;
	    b->prev = tail;
	    b = b->next;
	}
	tail = tail->next;
    }
    tail->next = a ? : b;

    do {
	/*
	 * In worst cases this loop may run many iterations.
	 * Continue callbacks to the client even though no
	 * element comparison is needed, so the client's cmp()
	 * routine can invoke cond_resched() periodically.
	 */
	(*cmp)(priv, tail->next, tail->next);

	tail->next->prev = tail;
	tail = tail->next;
    } while (tail->next);

    tail->next = head;
    head->prev = tail;
}

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * This function implements "merge sort", which has O(nlog(n))
 * complexity.
 *
 * The comparison function @cmp must return a negative value if @a
 * should sort before @b, and a positive value if @a should sort after
 * @b. If @a and @b are equivalent, and their original relative
 * ordering is to be preserved, @cmp must return 0.
 */
void lg_list_sort(void *priv, lg_list_t *head, int size,
	int (*cmp)(void *priv, lg_list_t *a, lg_list_t *b))
{

    lg_list_t *part[size];
    int lev;  /* index into part[] */
    int max_lev = 0;
    lg_list_t *list;

    if (lg_list_empty(head))
	return;

    memset(part, 0, sizeof(part));

    head->prev->next = NULL;
    list = head->next;

    while (list) {
	lg_list_t *cur = list;
	list = list->next;
	cur->next = NULL;

	for (lev = 0; part[lev]; lev++) {
	    cur = lg_list_merge(priv, cmp, part[lev], cur);
	    part[lev] = NULL;
	}
	if (lev > max_lev) {

	    if (unlikely(lev >= (int)ARRAY_SIZE(part)-1)) {

		lev--;
	    }

	    max_lev = lev;
	}

	part[lev] = cur;
    }

    for (lev = 0; lev < max_lev; lev++)
	if (part[lev])
	    list = lg_list_merge(priv, cmp, part[lev], list);

    lg_list_merge_and_restore_back_links(priv, cmp, head, part[max_lev], list);
}




