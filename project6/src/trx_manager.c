#include"lock_manager.h"
#include"trx_manager.h"

pthread_mutex_t trx_manager_latch = PTHREAD_MUTEX_INITIALIZER;
int count=0;
trx_t* trx_table[SIZE];


int trx_begin(){
	pthread_mutex_lock(&trx_manager_latch);
	int new_id = ++count;
	make_log_default(new_id,BEGIN);
	int hash_key = new_id%SIZE;
	if(trx_table[hash_key]==NULL){
		trx_table[hash_key] = (trx_t*)malloc(sizeof(trx_t));
		trx_table[hash_key]->trx_id = new_id;
		trx_table[hash_key]->need_lock = NULL;
		trx_table[hash_key]->next = NULL;
	}
	else{
		trx_t* temp = (trx_t*)malloc(sizeof(trx_t));
		temp->trx_id = new_id;
		temp->need_lock = NULL;
		temp->next = trx_table[hash_key];
		trx_table[hash_key] = temp;
	}
	pthread_mutex_unlock(&trx_manager_latch); 
	return new_id;
}

trx_t* trx_find(int trx_id){
	int hash_key = trx_id%SIZE;
	if(trx_table[hash_key] == NULL){
		return NULL;
	} 
	if(trx_table[hash_key]->trx_id == trx_id){
		return trx_table[hash_key];
	}
	else{
		trx_t* temp = trx_table[hash_key];
		if(temp == NULL) return NULL;
		while(temp->next != NULL){
			if(temp->next->trx_id == trx_id)
				return temp->next;
			temp = temp->next;
		}
	}
	return NULL;
}

int trx_commit(int trx_id){
	make_log_default(trx_id,COMMIT);
	pthread_mutex_lock(&trx_manager_latch);
	trx_t* now;
	if(trx_find(trx_id) == NULL){
		pthread_mutex_unlock(&trx_manager_latch);
		return 0;	
	}
	int hash_key = trx_id%SIZE;
	if(trx_table[hash_key]->trx_id == trx_id){
		now = trx_table[hash_key];
		trx_table[hash_key] = now->next;
	}else{
		trx_t* temp = trx_table[hash_key];
		while(temp->next){
			if(temp->next->trx_id == trx_id){
				now = temp->next;
				temp->next = now->next;
				break;
			}
			temp = temp->next;
		}
	}
	pthread_mutex_unlock(&trx_manager_latch);
	lock_t* clear = now->need_lock;
	while(clear != NULL){
		lock_t* next_lock = clear->next_lock_p;
		lock_release(clear);
		clear = next_lock;
	}
	flush();
	free(now);
	return trx_id;
}

int trx_abort(int trx_id){
	pthread_mutex_lock(&trx_manager_latch);
	trx_t* now;
	if(trx_find(trx_id)== NULL){
		pthread_mutex_unlock(&trx_manager_latch);
		return 0;	
	}
	int hash_key = trx_id%SIZE;
	if(trx_table[hash_key]->trx_id == trx_id){
		now = trx_table[hash_key];
		trx_table[hash_key] = now->next;
	}else{
		trx_t* temp = trx_table[hash_key];
		while(temp->next){
			if(temp->next->trx_id == trx_id){
				now = temp->next;
				temp->next = now->next;
				break;
			}
			temp = temp->next;
		}
	}
	pthread_mutex_unlock(&trx_manager_latch);
	lock_t* clear = now->need_lock;
	while(clear){
		lock_t* temp =clear;
		clear = clear->next_lock_p;
		lock_release(temp);
	}
	rollback(trx_id);
	make_log_default(trx_id,ROLLBACK);
	free(now);
	return 0;
}








