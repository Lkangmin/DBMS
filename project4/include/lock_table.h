#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100

typedef struct lock_t lock_t;


typedef struct hash_t{
	int table_id;
	int64_t key;
	struct hash_t* next_h;
	lock_t* head;
	lock_t* tail;
}hash_t;

/* APIs for lock table */
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key);
int lock_release(lock_t* lock_obj);

int64_t hash_f(int64_t key);
hash_t* hash_insert(int table_id, int64_t key);
hash_t* hash_find(int table_id, int64_t key);

#endif /* __LOCK_TABLE_H__ */
