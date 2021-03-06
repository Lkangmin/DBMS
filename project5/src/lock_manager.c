#include"lock_manager.h"

pthread_mutex_t trx_manager_latch = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_table_latch;
int count=0;
int cycle=0;
trx_t* trx_table[SIZE];
hash_t* hash_table[SIZE];

int trx_begin(){
	pthread_mutex_lock(&trx_manager_latch);
	int new_id = ++count;
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
	free(now);
	return trx_id;
}


int init_lock_table(void){
	pthread_mutex_init(&lock_table_latch,NULL);
	return 0;		
}


lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode){
	pthread_mutex_lock(&lock_table_latch);
	hash_t* hash_entry = hash_find(table_id,key);
	if(hash_entry == NULL)
		hash_entry = hash_insert(table_id,key);

	// ?????? ?????? record??? lock??? ????????????
	if(hash_entry->head->next == hash_entry->tail){
		// new lock allocate
		lock_t* new_lock = (lock_t*)malloc(sizeof(lock_t));
		pthread_cond_init(&new_lock->cond,NULL);
		new_lock->sent_p = hash_entry;
		new_lock->lock_mode = lock_mode;
		pthread_mutex_lock(&trx_manager_latch);
		trx_t* owner_trx = trx_find(trx_id);
		new_lock->next_lock_p = owner_trx->need_lock;
		owner_trx->need_lock = new_lock;
		pthread_mutex_unlock(&trx_manager_latch);
		new_lock->owner_trx_id = trx_id;
		new_lock->state = 0; // acquired
		new_lock->visit = 0;
		hash_entry->acquired_cnt++;

		//hash table ??????
		hash_entry->tail->prev->next = new_lock;
		new_lock->prev = hash_entry->tail->prev;
		new_lock->next = hash_entry->tail;
		hash_entry->tail->prev = new_lock;
		pthread_mutex_unlock(&lock_table_latch);
		return new_lock;
	}
	else{
			//same trx??? ??????
			lock_t* same_lock = hash_entry->tail->prev;
			if(same_lock->owner_trx_id==trx_id && same_lock->lock_mode==SHARED && lock_mode==EXCLUSIVE){
				same_lock->lock_mode=EXCLUSIVE;
				pthread_mutex_unlock(&lock_table_latch);
				return same_lock;
			}
			same_lock = hash_entry->head->next;
			while(same_lock != NULL && same_lock != hash_entry->tail){
				if(same_lock->owner_trx_id == trx_id){
					// SS XX
					if(same_lock->lock_mode == lock_mode){
						pthread_mutex_unlock(&lock_table_latch);
						return same_lock;
					}
					// SX
					else if(same_lock->lock_mode == EXCLUSIVE && lock_mode == SHARED){
						pthread_mutex_unlock(&lock_table_latch);
						return same_lock;	
					}else{
						pthread_mutex_unlock(&lock_table_latch);
						return NULL;
					}
				}
				same_lock = same_lock->next;
			}
			//other trx 
			lock_t* temp = hash_entry->tail->prev;
			if(temp->lock_mode == SHARED && lock_mode == SHARED){
				//acquire
				if(temp->state == 0){
					lock_t* new_lock = (lock_t*)malloc(sizeof(lock_t));
					pthread_cond_init(&new_lock->cond,NULL);
					new_lock->sent_p = hash_entry;
					new_lock->lock_mode = lock_mode;
					pthread_mutex_lock(&trx_manager_latch);
					trx_t* owner_trx = trx_find(trx_id);
					new_lock->next_lock_p = owner_trx->need_lock;
					owner_trx->need_lock = new_lock;
					pthread_mutex_unlock(&trx_manager_latch);
					new_lock->owner_trx_id = trx_id;
					new_lock->state = 0; // acquired
					new_lock->visit = 0;
					hash_entry->acquired_cnt++;

					//hash table ??????
					hash_entry->tail->prev->next = new_lock;
					new_lock->prev = hash_entry->tail->prev;
					new_lock->next = hash_entry->tail;
					hash_entry->tail->prev = new_lock;
					pthread_mutex_unlock(&lock_table_latch);
					return new_lock;
				}
				else{
					//deadlock detection    case: SS
					while(temp != NULL && temp->lock_mode != EXCLUSIVE) 
						temp = temp->prev;
					if(deadlock_detect(temp,trx_id)){
						pthread_mutex_unlock(&lock_table_latch);
						return NULL;
					}
					else{
						lock_t* new_lock = (lock_t*)malloc(sizeof(lock_t));
						pthread_cond_init(&new_lock->cond,NULL);
						new_lock->sent_p = hash_entry;
						new_lock->lock_mode = lock_mode;
						pthread_mutex_lock(&trx_manager_latch);
						trx_t* owner_trx = trx_find(trx_id);
						new_lock->next_lock_p = owner_trx->need_lock;
						owner_trx->need_lock = new_lock;
						pthread_mutex_unlock(&trx_manager_latch);
						new_lock->owner_trx_id = trx_id;
						new_lock->state = 1; // wait
						new_lock->visit = 0;

						//hash table ??????
						hash_entry->tail->prev->next = new_lock;
						new_lock->prev = hash_entry->tail->prev;
						new_lock->next = hash_entry->tail;
						hash_entry->tail->prev = new_lock;
						pthread_cond_wait(&new_lock->cond,&lock_table_latch);
						new_lock->state = 0; //wake get acquire
						hash_entry->acquired_cnt++;
						pthread_mutex_unlock(&lock_table_latch);
						return new_lock;

					}
				}
			}
			//deadlock detection    case : SX XS XX
			if(deadlock_detect(temp,trx_id)){
				pthread_mutex_unlock(&lock_table_latch);
				return NULL;
			}
			else{
				lock_t* new_lock = (lock_t*)malloc(sizeof(lock_t));
				pthread_cond_init(&new_lock->cond,NULL);
				new_lock->sent_p = hash_entry;
				new_lock->lock_mode = lock_mode;
				pthread_mutex_lock(&trx_manager_latch);
				trx_t* owner_trx = trx_find(trx_id);
				new_lock->next_lock_p = owner_trx->need_lock;
				owner_trx->need_lock = new_lock;
				pthread_mutex_unlock(&trx_manager_latch);
				new_lock->owner_trx_id = trx_id;
				new_lock->state = 1; // wait
				new_lock->visit = 0;

				//hash table ??????
				hash_entry->tail->prev->next = new_lock;
				new_lock->prev = hash_entry->tail->prev;
				new_lock->next = hash_entry->tail;
				hash_entry->tail->prev = new_lock;
				pthread_cond_wait(&new_lock->cond,&lock_table_latch);
				new_lock->state = 0; //wake get acquire
				hash_entry->acquired_cnt++;
				pthread_mutex_unlock(&lock_table_latch);
				return new_lock;
			}
	}
}

void dfs(lock_t* lock_obj, int my_trx_id){
	trx_t* temp = trx_find(lock_obj->owner_trx_id);
	if(temp == NULL) return;
	lock_t* check = temp->need_lock;
	while(check != NULL){
		if(check->state == 1){
			check->visit = 1;
			lock_t* test = check->prev;
			while(test != NULL){
				if(test->owner_trx_id == my_trx_id){
					cycle = 1;
					break;
				}
				if(test->visit == 0){
					dfs(test,my_trx_id);
				}
				test = test->prev;
			}
		}
		if(cycle) break;
		check = check->next_lock_p;
	}
}

int deadlock_detect(lock_t* lock_obj ,int my_trx_id){
	pthread_mutex_lock(&trx_manager_latch);
	while(lock_obj != NULL){
		dfs(lock_obj,my_trx_id);	
		if(cycle) break;
		lock_obj = lock_obj->prev;
	}
	for(int i=1;i<=count;i++){
		trx_t* temp = trx_find(i);
		if(temp == NULL) continue;
		lock_t* temp_l = temp->need_lock;
		while(temp_l != NULL){
			temp_l->visit=0;
			temp_l = temp_l->next_lock_p;
		}
	}
	pthread_mutex_unlock(&trx_manager_latch);
	if(cycle){
		cycle=0;
		return 1;
	}
	return 0;
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
		lock_t* temp = clear;
		if(clear->lock_mode == EXCLUSIVE){
			buffer_t* buf = buf_get(clear->sent_p->table_id,clear->pagenum);
			for(int i=0;i<buf->frame.key_num;i++){
				if(buf->frame.record[i].key == clear->sent_p->key){
					strcpy(buf->frame.record[i].value,clear->old_value);
				}
			}
			buf_put(buf);
		}
		clear = clear->next_lock_p;
		lock_release(temp);
	}
	free(now);
	return 0;
}



int lock_release(lock_t* lock_obj){
	pthread_mutex_lock(&lock_table_latch);
	if(lock_obj == NULL) return 0;
	if(lock_obj->lock_mode == SHARED){
		//  S ?????? ????????????
		if(lock_obj->sent_p->acquired_cnt == 1){
			lock_obj->prev->next = lock_obj->next;
			lock_obj->next->prev = lock_obj->prev;
			if(lock_obj->state == 0)
				lock_obj->sent_p->acquired_cnt--;
			if(lock_obj->next != NULL && lock_obj->next != lock_obj->sent_p->tail && lock_obj->state == 0){
				pthread_cond_signal(&lock_obj->next->cond);
			}
			free(lock_obj);
		}
		//  S ????????? ?????? ??????
		else{
			lock_obj->prev->next = lock_obj->next;
			lock_obj->next->prev = lock_obj->prev;
			if(lock_obj->state == 0)
				lock_obj->sent_p->acquired_cnt--;
			free(lock_obj);
		}
	}
	else{
		// case XS
		if(lock_obj->next->lock_mode == SHARED && lock_obj->next != lock_obj->sent_p->tail){
			lock_obj->prev->next = lock_obj->next;
			lock_obj->next->prev = lock_obj->prev;
			lock_t* temp = lock_obj->next;
			if(lock_obj->state == 0){
				lock_obj->sent_p->acquired_cnt--;
				while(temp != NULL  && temp->lock_mode == SHARED){
					pthread_cond_signal(&temp->cond);
					temp = temp->next;
				}			
			}
			free(lock_obj);
		}
		// case XX
		else{
			lock_obj->prev->next = lock_obj->next;
			lock_obj->next->prev = lock_obj->prev;
			if(lock_obj->state == 0)
				lock_obj->sent_p->acquired_cnt--;
			if(lock_obj->next != NULL &&  lock_obj->next != lock_obj->sent_p->tail && lock_obj->state == 0){
				pthread_cond_signal(&lock_obj->next->cond);
			}
			free(lock_obj);
		}
	}
	pthread_mutex_unlock(&lock_table_latch);
	return 0;
}



int64_t hash_f(int64_t key){
	return key%SIZE;
}

hash_t* hash_insert(int table_id, int64_t key){
	int64_t hash_key = hash_f(key);
	if(hash_table[hash_key]==NULL){
		hash_table[hash_key] = (hash_t*)malloc(sizeof(hash_t));
		hash_table[hash_key]->table_id = table_id;
		hash_table[hash_key]->key = key;
		hash_table[hash_key]->next_h = NULL;
		hash_table[hash_key]->head = (lock_t*)malloc(sizeof(lock_t));
		hash_table[hash_key]->tail = (lock_t*)malloc(sizeof(lock_t));
		hash_table[hash_key]->head->prev = NULL;
		hash_table[hash_key]->head->next = hash_table[hash_key]->tail;
		hash_table[hash_key]->tail->prev = hash_table[hash_key]->head;
		hash_table[hash_key]->tail->next = NULL;
		hash_table[hash_key]->acquired_cnt = 0;
		return hash_table[hash_key];
	}
	else{
		hash_t* temp = (hash_t*)malloc(sizeof(hash_t));
		temp->table_id = table_id;
		temp->key = key;
		temp->next_h = hash_table[hash_key];
		temp->head = (lock_t*)malloc(sizeof(lock_t));
		temp->tail = (lock_t*)malloc(sizeof(lock_t));
		temp->head->prev = NULL;
		temp->head->next = temp->tail;
		temp->tail->prev = temp->head;
		temp->tail->next = NULL;
		temp->acquired_cnt = 0;
		hash_table[hash_key] = temp;
		return temp;
	}
}


hash_t* hash_find(int table_id, int64_t key){
	int64_t hash_key = hash_f(key);
	if(hash_table[hash_key]==NULL)
		return NULL;
	if(hash_table[hash_key]->key == key && hash_table[hash_key]->table_id == table_id)
		return hash_table[hash_key];
	else{
		hash_t* temp = hash_table[hash_key];
		while(temp->next_h){
			if(temp->next_h->key == key && temp->next_h->table_id == table_id)
				return temp->next_h;
			temp = temp->next_h;
		}
	}
	return NULL;
}
