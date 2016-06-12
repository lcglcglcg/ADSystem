
#ifndef __LG_LIST_H__
#define	__LG_LIST_H__

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct lg_list_t lg_list_t;
    struct lg_list_t {

	lg_list_t *next;
	lg_list_t *prev;
    };

    static inline void lg_list_init(lg_list_t *head)
    {
	head->next = head;
	head->prev = head;
    }

    static inline void _lg_list_add(lg_list_t *node, lg_list_t *prev, lg_list_t *next)
    {
	next->prev = node;
	node->next = next;
	node->prev = prev;
	prev->next = node;
    }

    static inline void lg_list_add(lg_list_t *head, lg_list_t *node)
    {
	_lg_list_add(node, head->prev, head);
    }

    static inline void lg_list_add_tail(lg_list_t *head, lg_list_t *node)
    {
	_lg_list_add(node, head, head->next);
    }

    static inline void _lg_list_del(lg_list_t *prev, lg_list_t *next)
    {
	next->prev = prev;
	prev->next = next;
    }

    static inline void lg_list_del(lg_list_t *node)
    {
	_lg_list_del(node->prev, node->next);
    }

    static inline int lg_list_empty(lg_list_t *head)
    {
	lg_list_t *next = head->next;
	return (next == head) && (next == head->prev);
    }

    extern void lg_list_sort(void *priv, lg_list_t *head, int size,
	    int (*cmp)(void *priv, lg_list_t *a, lg_list_t *b));

#define lg_list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)
#define lg_list_for_tail(pos, head) for (pos = (head)->prev; pos != (head); pos = pos->prev)
#define lg_list_offsetof(type,member) (size_t)(&((type *)0)->member)
#define lg_list_entry(ptr, type, member) ({const typeof(((type *)0)->member) * __mptr = (ptr);(type *)((char *)__mptr - lg_list_offsetof(type, member));})

#ifdef __cplusplus
}
#endif

#endif


