#include <lock_table.h>


pthread_mutex_t lock_table_latch;

struct lock_t {
	/* NO PAIN, NO GAIN. */
	struct lock_t* prev_p;
	struct lock_t* next_p;
	hash_t* sent_p;
	pthread_cond_t cond;
};

typedef struct lock_t lock_t;

hash_t* hash_table[SIZE];

int init_lock_table()
{
	pthread_mutex_init(&lock_table_latch,NULL);
	return 0;
}

lock_t* lock_acquire(int table_id, int64_t key)
{
	pthread_mutex_lock(&lock_table_latch);
	hash_t* hash_entry = hash_find(table_id,key);
	if(hash_entry == NULL)
		hash_entry = hash_insert(table_id,key);

	lock_t* new_lock = (lock_t*)malloc(sizeof(lock_t));
	pthread_cond_init(&new_lock->cond,NULL);
	new_lock->sent_p = hash_entry;
	hash_entry->head->next_p->prev_p = new_lock;
	new_lock->next_p = hash_entry->head->next_p;
	hash_entry->head->next_p = new_lock;
	new_lock->prev_p = hash_entry->head;
	if(new_lock->next_p != hash_entry->tail){
		pthread_cond_wait(&new_lock->cond,&lock_table_latch);
	}
	pthread_mutex_unlock(&lock_table_latch);
	return new_lock;
}

int
lock_release(lock_t* lock_obj)
{
	pthread_mutex_lock(&lock_table_latch);
	lock_obj->prev_p->next_p = lock_obj->next_p;
	lock_obj->next_p->prev_p = lock_obj->prev_p;
	pthread_cond_signal(&lock_obj->prev_p->cond);
	free(lock_obj);
	pthread_mutex_unlock(&lock_table_latch);
	return 0;
}



int64_t hash_f(int64_t key){
	return key%SIZE;
}

hash_t* hash_insert(int table_id, int64_t key){
	int64_t hash_key = hash_f(key);
	if(hash_table[hash_key] == NULL){
		hash_table[hash_key] = (hash_t*)malloc(sizeof(hash_t));
		hash_table[hash_key]->table_id = table_id;
		hash_table[hash_key]->key = key;
		hash_table[hash_key]->next_h = NULL;
		hash_table[hash_key]->head = (lock_t*)malloc(sizeof(lock_t));
		hash_table[hash_key]->tail = (lock_t*)malloc(sizeof(lock_t));
		hash_table[hash_key]->head->prev_p = NULL;
		hash_table[hash_key]->head->next_p = hash_table[hash_key]->tail;
		hash_table[hash_key]->tail->prev_p = hash_table[hash_key]->head;
		hash_table[hash_key]->tail->next_p = NULL;
		return hash_table[hash_key];
	}
	else{
		hash_t* temp = (hash_t*)malloc(sizeof(hash_t));
		temp->table_id = table_id;
		temp->key = key;
		temp->next_h = hash_table[hash_key];
		temp->head = (lock_t*)malloc(sizeof(lock_t));
		temp->tail = (lock_t*)malloc(sizeof(lock_t));
		temp->head->prev_p = NULL;
		temp->head->next_p = temp->tail;
		temp->tail->prev_p = temp->head;
		temp->tail->next_p = NULL;
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