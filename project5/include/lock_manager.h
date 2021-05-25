#ifndef __LOCK_MANAGER_H__
#define __LOCK_MANAGER_H__

#include"buffer_manager.h"
#include"file.h"
#include<pthread.h>
#define SIZE 10000
#define SHARED 0
#define EXCLUSIVE 1

typedef struct lock_t lock_t;

typedef struct hash_t{
	int table_id;
	int64_t key;
	struct hash_t* next_h;
	lock_t* head;
	lock_t* tail;
	int acquired_cnt;
}hash_t;


struct lock_t{
	lock_t* prev;
	lock_t* next;
	hash_t* sent_p;
	pthread_cond_t cond;
	int lock_mode;
	lock_t* next_lock_p;
	int owner_trx_id;
	int state; // 0:acquired 1:wait
	pagenum_t pagenum;
	char old_value[120];
	int visit;
};

typedef struct trx_t{
	int trx_id;
	lock_t* need_lock;
	struct trx_t* next;
}trx_t;


extern pthread_mutex_t trx_manager_latch;
extern pthread_mutex_t lock_table_latch;

trx_t* trx_table[SIZE];
hash_t* hash_table[SIZE];
int count;

int trx_begin();
int trx_commit(int trx_id);
trx_t* trx_find(int trx_id);
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode);
void dfs(lock_t* lock_obj, int my_trx_id);
int deadlock_detect(lock_t* lock_obj ,int my_trx_id);
int trx_abort(int trx_id);
int lock_release(lock_t* lock_obj);
int64_t hash_f(int64_t key);
hash_t* hash_insert(int table_id, int64_t key);
hash_t* hash_find(int table_id, int64_t key);


#endif