
#ifndef	__LG_RBTREE_H__
#define	__LG_RBTREE_H__

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct lg_rbtree_node {

	unsigned long  rb_parent_color;
	struct lg_rbtree_node *rb_right;
	struct lg_rbtree_node *rb_left;

    }lg_rbtree_node ;

    typedef struct {

	lg_rbtree_node *rb_node;

    }lg_rbtree_head;

    extern void lg_rbtree_insert_color(lg_rbtree_node *node, lg_rbtree_head *head);
    extern void lg_rbtree_link_node(lg_rbtree_node * node, lg_rbtree_node *parent, lg_rbtree_node **rb_link);
    extern void lg_rbtree_delete(lg_rbtree_head *head, lg_rbtree_node *node);

#ifdef __cplusplus
}
#endif

#define RB_ROOT	(lg_rbtree_head) {0}
#define rb_is_red(r) (!rb_color(r))
#define rb_is_black(r) rb_color(r)
#define rb_set_red(r) do { (r)->rb_parent_color &= ~1; } while (0)
#define rb_set_black(r) do { (r)->rb_parent_color |= 1; } while (0)
#define rb_parent(r) ((lg_rbtree_node *)((r)->rb_parent_color & ~3))
#define rb_color(r) ((r)->rb_parent_color & 1)
#define __offsetof(type,member) (size_t)(&((type *)0)->member)
#define rb_entry(ptr, type, member) ({const typeof(((type *)0)->member) * __mptr = (ptr);(type *)((char *)__mptr - __offsetof(type, member));})

#endif


