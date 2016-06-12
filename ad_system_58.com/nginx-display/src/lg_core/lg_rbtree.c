
#include "lg_rbtree.h"

void lg_rbtree_set_parent(lg_rbtree_node *rb, lg_rbtree_node *p)
{
    rb->rb_parent_color = (rb->rb_parent_color & 3) | (unsigned long)p;
}

void lg_rbtree_set_color(lg_rbtree_node *rb, int color)
{
    rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

void lg_rbtree_link_node(lg_rbtree_node *node, lg_rbtree_node *parent, lg_rbtree_node **rb_link)
{

    node->rb_parent_color = (unsigned long )parent;
    node->rb_left = node->rb_right = NULL;
    *rb_link = node;
}

void lg_rbtree_rotate_left(lg_rbtree_node *node, lg_rbtree_head *head)
{
    lg_rbtree_node *right = node->rb_right;
    lg_rbtree_node *parent = rb_parent(node);

    if ((node->rb_right = right->rb_left))
	lg_rbtree_set_parent(right->rb_left, node);

    right->rb_left = node;
    lg_rbtree_set_parent(right, parent);

    if (parent) {

	if (node == parent->rb_left)
	    parent->rb_left = right;
	else
	    parent->rb_right = right;

    } else {

	head->rb_node = right;
    }

    lg_rbtree_set_parent(node, right);
}

void lg_rbtree_rotate_right(lg_rbtree_node *node, lg_rbtree_head *head)
{
    lg_rbtree_node *left = node->rb_left;
    lg_rbtree_node *parent = rb_parent(node);

    if ((node->rb_left = left->rb_right))
	lg_rbtree_set_parent(left->rb_right, node);

    left->rb_right = node;
    lg_rbtree_set_parent(left, parent);

    if (parent) {

	if (node == parent->rb_right)
	    parent->rb_right = left;
	else
	    parent->rb_left = left;

    } else {

	head->rb_node = left;
    }

    lg_rbtree_set_parent(node, left);
}

void lg_rbtree_insert_color(lg_rbtree_node *node, lg_rbtree_head *head)
{

    lg_rbtree_node *parent, *gparent;

    while ((parent = rb_parent(node)) && rb_is_red(parent)) {

	gparent = rb_parent(parent);

	if (parent == gparent->rb_left) {

	    register lg_rbtree_node *uncle = gparent->rb_right;
	    if (uncle && rb_is_red(uncle)) {

		rb_set_black(uncle);
		rb_set_black(parent);
		rb_set_red(gparent);
		node = gparent;
		continue;
	    }

	    if (parent->rb_right == node) {

		register lg_rbtree_node *tmp;
		lg_rbtree_rotate_left(parent, head);
		tmp = parent;
		parent = node;
		node = tmp;
	    }

	    rb_set_black(parent);
	    rb_set_red(gparent);
	    lg_rbtree_rotate_right(gparent, head);

	} else {

	    register lg_rbtree_node *uncle = gparent->rb_left;
	    if (uncle && rb_is_red(uncle)) {

		rb_set_black(uncle);
		rb_set_black(parent);
		rb_set_red(gparent);
		node = gparent;
		continue;
	    }

	    if (parent->rb_left == node) {

		register lg_rbtree_node *tmp;
		lg_rbtree_rotate_right(parent, head);
		tmp = parent;
		parent = node;
		node = tmp;
	    }

	    rb_set_black(parent);
	    rb_set_red(gparent);
	    lg_rbtree_rotate_left(gparent, head);
	}
    }

    rb_set_black(head->rb_node);
}

void lg_rbtree_delete_color(lg_rbtree_node *node, lg_rbtree_node *parent, lg_rbtree_head *head)
{

    lg_rbtree_node *other = NULL;

    while ((!node || rb_is_black(node)) && node != head->rb_node) {

	if (parent->rb_left == node) {

	    other = parent->rb_right;

	    if (rb_is_red(other)) {

		rb_set_black(other);
		rb_set_red(parent);
		lg_rbtree_rotate_left(parent, head);
		other = parent->rb_right;
	    }

	    if ((!other->rb_left || rb_is_black(other->rb_left)) &&
		    (!other->rb_right || rb_is_black(other->rb_right))) {

		rb_set_red(other);
		node = parent;
		parent = rb_parent(node);

	    } else {

		if (!other->rb_right || rb_is_black(other->rb_right)) {

		    rb_set_black(other->rb_left);
		    rb_set_red(other);
		    lg_rbtree_rotate_right(other, head);
		    other = parent->rb_right;
		}

		lg_rbtree_set_color(other, rb_color(parent));
		rb_set_black(parent);
		rb_set_black(other->rb_right);
		lg_rbtree_rotate_left(parent, head);
		node = head->rb_node;
		break;
	    }

	} else {

	    other = parent->rb_left;

	    if (rb_is_red(other)) {

		rb_set_black(other);
		rb_set_red(parent);
		lg_rbtree_rotate_right(parent, head);
		other = parent->rb_left;
	    }

	    if ((!other->rb_left || rb_is_black(other->rb_left)) &&
		    (!other->rb_right || rb_is_black(other->rb_right))) {

		rb_set_red(other);
		node = parent;
		parent = rb_parent(node);

	    } else {

		if (!other->rb_left || rb_is_black(other->rb_left)) {

		    rb_set_black(other->rb_right);
		    rb_set_red(other);
		    lg_rbtree_rotate_left(other, head);
		    other = parent->rb_left;
		}

		lg_rbtree_set_color(other, rb_color(parent));
		rb_set_black(parent);
		rb_set_black(other->rb_left);
		lg_rbtree_rotate_right(parent, head);
		node = head->rb_node;
		break;
	    }
	}
    }

    if (node)rb_set_black(node);
}

void lg_rbtree_delete(lg_rbtree_head *head, lg_rbtree_node *node)
{

    int color;
    lg_rbtree_node *child, *parent;

    if (!node->rb_left) {

	child = node->rb_right;

    } else if (!node->rb_right) {

	child = node->rb_left;

    } else {

	lg_rbtree_node *old = node, *left;

	node = node->rb_right;
	while ((left = node->rb_left) != NULL)
	    node = left;

	if (rb_parent(old)) {

	    if (rb_parent(old)->rb_left == old)
		rb_parent(old)->rb_left = node;
	    else
		rb_parent(old)->rb_right = node;

	} else {

	    head->rb_node = node;
	}

	child = node->rb_right;
	parent = rb_parent(node);
	color = rb_color(node);

	if (parent == old) {

	    parent = node;

	} else {

	    if (child)lg_rbtree_set_parent(child, parent);
	    parent->rb_left = child;

	    node->rb_right = old->rb_right;
	    lg_rbtree_set_parent(old->rb_right, node);
	}

	node->rb_parent_color = old->rb_parent_color;
	node->rb_left = old->rb_left;
	lg_rbtree_set_parent(old->rb_left, node);

	if (color == 1)lg_rbtree_delete_color(child, parent, head);
	return;
    }

    parent = rb_parent(node);
    color = rb_color(node);

    if (child)lg_rbtree_set_parent(child, parent);

    if (parent) {

	if (parent->rb_left == node)
	    parent->rb_left = child;
	else
	    parent->rb_right = child;

    } else {

	head->rb_node = child;
    }

    if (color == 1)lg_rbtree_delete_color(child, parent, head);
}

/* new api */

lg_rbtree_node *lg_tree_element_key_add(lg_rbtree_head *head, lg_rbtree_node *new_node)
{

    lg_rbtree_node *parent = NULL;
    lg_rbtree_node **p = &head->rb_node;
    lg_rbtree_node *node = NULL;

    if (new_node->key == NULL)
	return NULL;

    while (*p) {

	parent = *p;
	node = (lg_rbtree_node *)parent;
	int retval = strcmp(new_node->key, node->key);

	if (retval < 0) {

	    p = &(*p)->rb_left;

	} else if (retval > 0) {

	    p = &(*p)->rb_right;

	} else {

	    return NULL;
	}
    }

    lg_rbtree_link_node(new_node, parent, p);
    lg_rbtree_insert_color(new_node, head);
    return node;
}

lg_rbtree_node *lg_tree_element_key_find(lg_rbtree_head *head, const char *key)
{

    lg_rbtree_node *p = head->rb_node;
    lg_rbtree_node *node = NULL;

    while (p) {

	node = (lg_rbtree_node *)p;

	if (key == NULL || node->key == NULL)
	    return NULL;
	
	int retval = strcmp(key, node->key);
	if (retval < 0) {

	    p = p->rb_left;

	} else if (retval > 0) {

	    p = p->rb_right;

	} else {

	    return node;
	}
    }

    return NULL;
}

lg_rbtree_node *lg_tree_element_index_add(lg_rbtree_head *head, lg_rbtree_node *new_node)
{

    lg_rbtree_node *parent = NULL;
    lg_rbtree_node **p = &head->rb_node;
    lg_rbtree_node *node = NULL;

    while (*p) {

	parent = *p;
	node = (lg_rbtree_node *)parent;

	if (new_node->index < node->index) {

	    p = &(*p)->rb_left;

	} else if (new_node->index > node->index) {

	    p = &(*p)->rb_right;

	} else {

	    return NULL;
	}
    }

    lg_rbtree_link_node(new_node, parent, p);
    lg_rbtree_insert_color(new_node, head);
    return node;
}

lg_rbtree_node *lg_tree_element_index_find(lg_rbtree_head *head, size_t index)
{

    lg_rbtree_node *p = head->rb_node;
    lg_rbtree_node *node = NULL;

    while (p) {

	node = (lg_rbtree_node *)p;

	if (index < node->index) {

	    p = p->rb_left;

	} else if (index > node->index) {

	    p = p->rb_right;

	} else {

	    return node;
	}
    }

    return NULL;
}

void _lg_tree_element_print(lg_rbtree_node *p)
{

    lg_rbtree_node *node = (lg_rbtree_node *)p;
    fprintf(stdout, "[list][%zd][%s]\n", node->index, node->key);

    if (p->rb_left) _lg_tree_element_print(p->rb_left);
    if (p->rb_right)_lg_tree_element_print(p->rb_right);
}

void lg_tree_element_print(lg_rbtree_head *head)
{

    if (!head->rb_node)
	return;

    _lg_tree_element_print(head->rb_node);
}


