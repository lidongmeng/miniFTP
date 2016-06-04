/*************************************************************************
	> File Name: hash.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Fri 03 Jun 2016 10:19:39 PM EDT
 ************************************************************************/

#include "hash.h"


hash_t * hash_alloc(unsigned int number, hash_func_t func) {
	hash_t * hash = (hash_t*)malloc(sizeof(hash_t));
	hash->bucketsNumber = number;
	hash->hashFunc = func;

	int len = number * sizeof(hash_node_t*);
	hash->nodes = (hash_node_t**)malloc(len);
	memset(hash->nodes, 0, len);
	return hash;
}

hash_node_t ** get_entry_bucket(hash_t * hash, void * key) {
	unsigned int bucketIndex = hash->hashFunc(hash->bucketsNumber, key);
	if (bucketIndex < 0 || bucketIndex >= hash->bucketsNumber) return NULL;
    return &(hash->nodes[bucketIndex]);
}

hash_node_t * get_entry_by_key(hash_t * hash, void * key, int key_size) {
	hash_node_t * node = *get_entry_bucket(hash, key);
	while (node && memcmp(node->key, key, key_size) != 0) {
		node = node->next;
	}
	return node;
}


void add_entry(hash_t * hash, void * key, unsigned int key_size, void * value, unsigned int value_size) {
	if (lookup_entry(hash, key, key_size) != NULL) {
		printf("dulplicate hash key\n");
	}

	hash_node_t * node = (hash_node_t*)malloc(sizeof(hash_node_t));
	node->prev = NULL;
	node->next = NULL;

	node->key = malloc(sizeof(key_size));
	memcpy(node->key, key, key_size);
	node->value = malloc(sizeof(value_size));
	memcpy(node->value, value,value_size);

	hash_node_t ** buckets = get_entry_bucket(hash, key);
	if (*buckets == NULL) {
		*buckets = node;
	} else {
		node->next = *buckets;
		(*buckets)->prev = node;
		*buckets = node;
	}
}

void delete_entry(hash_t * hash, void * key, unsigned int key_size) {
	hash_node_t * node = get_entry_by_key(hash, key, key_size);
	if (node == NULL) return ;
	free(node->key);
	free(node->value);
	if (node->prev) {
		node->prev->next = node->next;
	} else {
		hash_node_t **bucket = get_entry_bucket(hash, key);
		*bucket = node->next;
	}
	if (node->next) node->next->prev = node->prev;
	free(node);
}

void * lookup_entry(hash_t * hash, void * key, unsigned int key_size) {
	hash_node_t * node = get_entry_by_key(hash, key, key_size);
	if (node == NULL) return NULL;
	else return node->value;
}

