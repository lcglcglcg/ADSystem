
#include "lg_db.h"

static lg_list_t db_pool = {&db_pool, &db_pool};
static lg_list_t db_pool_free = {&db_pool_free, &db_pool_free};
static lg_list_t db_node_pool = {&db_node_pool, &db_node_pool};
static lg_list_t db_node_pool_free = {&db_node_pool_free, &db_node_pool_free};

lg_db_t *lg_db_get_db()
{

    lg_list_t *p = lg_queue(&db_pool_free);
    if (p) {
	lg_list_add(&db_pool, p);
	return lg_list_entry(p, lg_db_t, pool);
    }

    lg_db_t *db = malloc(sizeof(lg_db_t));
    if (!db) return NULL;

    lg_list_add(&db_pool, &db->pool);
    return db;
}

lg_db_node_t *lg_db_get_db_node()
{

    lg_list_t *p = lg_queue(&db_node_pool_free);
    if (p) {

	//	fprintf(stdout, "[find][%p]\n", 
	//		lg_list_entry(p, lg_db_node_t, pool));

	lg_list_add(&db_node_pool, p);
	return lg_list_entry(p, lg_db_node_t, pool);
    }

    lg_db_node_t *db_node = malloc(sizeof(lg_db_node_t));
    if (!db_node) return NULL;

    // fprintf(stdout, "[new][%p]\n", db_node);
    lg_list_add(&db_node_pool, &db_node->pool);
    return db_node;
}

// db create 
lg_db_t *lg_db_create(char *db_name)
{

    lg_db_t *db = lg_db_get_db();
    if (!db) return NULL;

    db->db_name = db_name;
    db->head_tree = RB_ROOT;
    lg_list_init(&db->head_list);
    return db;
}

// add string
lg_db_node_t *lg_db_string_node_add(lg_db_t *head, char *key)
{
    if (!head || !key || strlen(key) >= 64) return NULL;

    lg_db_node_t *node = lg_db_string_find(head, key);
    if (node) return node;

    // new node
    node = lg_db_get_db_node();
    if (!node) return NULL;

    // key
    strcpy(node->key, key);
    node->tree.key = node->key;
    node->tree.index = 0;

    // add
    lg_tree_element_key_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    // 初始化 下级节点head
    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);

    // 初始化 数据段
    lg_list_init(&node->m_list);
    node->m_data = NULL;
    node->m_size = 0;
    return node;
}

// add string child
lg_db_node_t *lg_db_string_node_child_add(lg_db_node_t *head, char *key)
{
    if (!head || !key || strlen(key) >= 64) return NULL;

    lg_db_node_t *node = lg_db_string_child_find(head, key);
    if (node) return node;

    // new node
    node = lg_db_get_db_node();
    if (!node) return NULL;

    // key
    strcpy(node->key, key);
    node->tree.key = node->key;
    node->tree.index = 0;

    // add
    lg_tree_element_key_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    // 初始化 下级节点head
    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);

    // 初始化 数据段
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

void lg_db_node_free(lg_db_node_t *db, lg_db_free_fun_t *fun) 
{

    lg_list_t *p = NULL;
    lg_list_for_each(p, &db->head_list) {

	lg_db_node_t *node = lg_list_entry(p, lg_db_node_t, next);
	lg_db_node_free(node, fun); //释放子节点

	if (fun) fun(node->key, &node->m_list, node->m_data);

	lg_list_del(&node->pool);
	lg_list_add(&db_node_pool_free, &node->pool);
    }
}

void lg_db_free(lg_db_t *db, lg_db_free_fun_t *fun)
{

    if (!db) return;

    lg_list_t *p = NULL;
    lg_list_for_each(p, &db->head_list) {

	lg_db_node_t *node = lg_list_entry(p, lg_db_node_t, next);
	lg_db_node_free(node, fun); //释放子节点

	if (fun) fun(node->key, &node->m_list, node->m_data);

	lg_list_del(&node->pool);
	lg_list_add(&db_node_pool_free, &node->pool);
    }

    lg_list_del(&db->pool);
    lg_list_add(&db_pool_free, &db->pool);
}


lg_db_node_t *lg_db_number_node_add(lg_db_t *head, size_t index)
{

    if (!head || !index) return NULL;

    lg_db_node_t *node = lg_db_number_find(head, index);
    if (node) return node;

    // new node
    node = lg_db_get_db_node();
    if (!node) return NULL;

    // key
    node->tree.key = NULL;
    node->tree.index = index;

    // add
    lg_tree_element_index_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    // 初始化 下级节点head
    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);

    // 初始化 数据段
    lg_list_init(&node->m_list);
    node->m_data = NULL;
    node->m_size = 0;
    return node;
}

// add number child
lg_db_node_t *lg_db_number_node_child_add(lg_db_node_t *head, size_t index)
{

    if (!head || !index) return NULL;

    lg_db_node_t *node = lg_db_number_child_find(head, index);
    if (node) return node;

    // new node
    node = lg_db_get_db_node();
    if (!node) return NULL;

    // key
    node->tree.key = NULL;
    node->tree.index = index;

    // add
    lg_tree_element_index_add(&head->head_tree, &node->tree);
    lg_list_add(&head->head_list, &node->next);

    // 初始化 下级节点head
    node->head_tree = RB_ROOT;
    lg_list_init(&node->head_list);

    // 初始化 数据段
    lg_list_init(&node->m_list);
    node->m_data = NULL;
    node->m_size = 0;
    return node;
}

// find number
lg_db_node_t *lg_db_number_find(lg_db_t *head, size_t index)
{
    if (!head || !index) return NULL;

    lg_rbtree_node *node = lg_tree_element_index_find(&head->head_tree, index);
    if (!node) return NULL;

    return rb_entry(node, lg_db_node_t, tree);
}

// find number child
lg_db_node_t *lg_db_number_child_find(lg_db_node_t *head, size_t index)
{

    if (!head || !index) return NULL;

    lg_rbtree_node *node = lg_tree_element_index_find(&head->head_tree, index);
    if (!node) return NULL;

    return rb_entry(node, lg_db_node_t, tree);

}




