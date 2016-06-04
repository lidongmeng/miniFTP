/*************************************************************************
	> File Name: hash.h
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Fri 03 Jun 2016 10:11:42 PM EDT
 ************************************************************************/

#ifndef _HASH_H_
#define _HASH_H_

#include "common.h"
typedef unsigned int (*hash_func_t) (unsigned int , void *);

struct hash_node_t {
	void * key;
	void * value;
	hash_node_t * prev;
	hash_node_t * next;
};

struct hash_t {
	unsigned int bucketsNumber;
	hash_func_t hashFunc;
	hash_node_t ** nodes;
};

hash_t * hash_alloc(unsigned int number, hash_func_t func);
void add_entry(hash_t * hash, void * key, unsigned int key_size, void * value, unsigned int value_size);
void delete_entry(hash_t * hash, void * key, unsigned int key_size);
void * lookup_entry(hash_t * hash, void * key, unsigned int key_size);

#endif // _HASH_H_

