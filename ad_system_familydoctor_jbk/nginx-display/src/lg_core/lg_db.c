
#include "lg_db.h"

// db create 
lg_db_t *lg_db_create(char *db_name)
{

    // 创建新的db
    lg_db_t *db = malloc(sizeof(lg_db_t));
    if (!db) return NULL;

    db->db_name = db_name;
    db->head_tree = RB_ROOT;
    lg_list_init(&db->head_list);
    return db;
}

// add number
lg_db_node_t *lg_db_number_node_add(lg_db_t *head, size_t index)
{

    lg_db_node_t *node = lg_db_number_find(head, index);
    if (node) return node;

    node = malloc(sizeof(lg_db_node_t));
    if (!node) return NULL;

    node->tree.key = NULL;
    node->tree.index = index;

    lg_tree_element_index_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);


    lg_list_init(&node->m_list);
    node->m_data = NULL;
    node->m_size = 0;
    return node;
}

// add number child
lg_db_node_t *lg_db_number_node_child_add(lg_db_node_t *head, size_t index)
{

    lg_db_node_t *node = lg_db_number_child_find(head, index);
    if (node) return node;

    node = malloc(sizeof(lg_db_node_t));
    if (!node) return NULL;

    node->tree.key = NULL;
    node->tree.index = index;

    lg_tree_element_index_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);

    lg_list_init(&node->m_list);
    node->m_data = NULL;
    node->m_size = 0;
    return node;
}

// find number
lg_db_node_t *lg_db_number_find(lg_db_t *head, size_t index)
{
    if (!head) return NULL;

    lg_rbtree_node *node = lg_tree_element_index_find(&head->head_tree, index);
    if (!node) return NULL;

    return rb_entry(node, lg_db_node_t, tree);
}

// find number child
lg_db_node_t *lg_db_number_child_find(lg_db_node_t *head, size_t index)
{
    if (!head) return NULL;

    lg_rbtree_node *node = lg_tree_element_index_find(&head->head_tree, index);
    if (!node) return NULL;

    return rb_entry(node, lg_db_node_t, tree);
}

// add string
lg_db_node_t *lg_db_string_node_add(lg_db_t *head, char *key)
{
    if (!key) return NULL;

    lg_db_node_t *node = lg_db_string_find(head, key);
    if (node) return node;

    node = malloc(sizeof(lg_db_node_t));
    if (!node) return NULL;

    node->tree.index = 0;
    node->tree.key = malloc(strlen(key) + 1);
    if (!node->tree.key) {

	free(node);
	return NULL;
    }

    strcpy(node->tree.key, key);
    lg_tree_element_key_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);

    lg_list_init(&node->m_list);
    node->m_data = NULL;
    node->m_size = 0;
    return node;
}

// add string child
lg_db_node_t *lg_db_string_node_child_add(lg_db_node_t *head, char *key)
{
    if (!key) return NULL;

    lg_db_node_t *node = lg_db_string_child_find(head, key);
    if (node) return node;

    node = malloc(sizeof(lg_db_node_t));
    if (!node) return NULL;

    node->tree.index = 0;
    node->tree.key = malloc(strlen(key) + 1);
    if (!node->tree.key) {

	free(node);
	return NULL;
    }

    strcpy(node->tree.key, key);
    lg_tree_element_key_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);

    lg_list_init(&node->m_list);
    node->m_data = NULL;
    node->m_size = 0;
    return node;
}

// find string
lg_db_node_t *lg_db_string_find(lg_db_t *head, char *key)
{

    if (!head || !key) return NULL;

    lg_rbtree_node *node = lg_tree_element_key_find(&head->head_tree, key);
    if (!node) return NULL;

    return rb_entry(node, lg_db_node_t, tree);
}

// find string child
lg_db_node_t *lg_db_string_child_find(lg_db_node_t *head, char *key)
{
    if (!head || !key) return NULL;

    lg_rbtree_node *node = lg_tree_element_key_find(&head->head_tree, key);
    if (!node) return NULL;

    return rb_entry(node, lg_db_node_t, tree);
}

// add tree->list
lg_db_list_node_t *lg_db_tree_list_add(lg_list_t *head, void *data)
{

    lg_db_list_node_t *node = malloc(sizeof(lg_db_list_node_t));
    if (!node) return NULL;

    node->m_data = data;
    return node;
}

void lg_db_node_list_free(lg_list_t *m_list)
{

    lg_list_t *p = m_list->next;
    while(p != m_list) {

	lg_list_t *node = p;
	p = p->next;
	free(node); //free lg_db_list_node_t
    }
}

void lg_db_node_free(lg_db_node_t *head, lg_db_free_fun_t *fun) 
{

    lg_list_t *list_head = &head->head_list;
    lg_list_t *p = list_head->next;
    while(p != list_head) {

	lg_db_node_t *node = lg_list_entry(p, lg_db_node_t, next);
	p = p->next;

	if (fun) fun(&node->m_list, node->m_data);
	lg_db_node_list_free(&node->m_list); // lg_db_list_node_t

	lg_db_node_free(node, fun); //释放子节点

	if (node->tree.key) free(node->tree.key);
	free(node);
    }
}

void lg_db_free(lg_db_t *head, lg_db_free_fun_t *fun)
{

    if (!head) return;

    lg_list_t *list_head = &head->head_list;
    lg_list_t *p = list_head->next;
    while(p != list_head) {

	lg_db_node_t *node = lg_list_entry(p, lg_db_node_t, next);
	p = p->next;

	if (fun) fun(&node->m_list, node->m_data);
	lg_db_node_list_free(&node->m_list); //lg_db_list_node_t

	lg_db_node_free(node, fun); //释放子节点

	if (node->tree.key) free(node->tree.key);
	free(node);
    }

    free(head);
}



