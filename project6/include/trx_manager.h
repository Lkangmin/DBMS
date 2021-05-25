#ifndef __TRX_MANAGER_H__
#define __TRX_MANAGER_H__

#include"lock_manager.h"
#include"buffer_manager.h"
#include"file.h"
#include"log_manager.h"
#define SIZE 10000

typedef struct trx_t{
	int trx_id;
	struct lock_t* need_lock;
	struct trx_t* next;
}trx_t;


extern pthread_mutex_t trx_manager_latch;

trx_t* trx_table[SIZE];

int count;

int trx_begin();
int trx_commit(int trx_id);
trx_t* trx_find(int trx_id);
int trx_abort(int trx_id);


#endif