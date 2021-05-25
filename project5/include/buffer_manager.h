#ifndef __BUFFER_MANAGER_H__
#define __BUFFER_MANAGER_H__

#include"file.h"
#include"lock_manager.h"
#include<inttypes.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct buffer_t{
	union{
		page_t frame;
		HeaderPage head_p;
		FreePage free_p;
	};
	int table_id;
	pagenum_t page_num;
	int is_dirty;
	pthread_mutex_t page_latch;
	struct buffer_t *next;
	struct buffer_t *prev;
}buffer_t;

typedef struct {
	buffer_t* head;
	buffer_t* tail;	
}LRU_list;

buffer_t* buf_get(int table_id, pagenum_t page_num);
void buf_put(buffer_t* temp);
pagenum_t buf_alloc(int table_id);
void buf_free(int table_id,pagenum_t page_num);

int buf_open_table(char* pathname);
int init_db(int num_buf);
int close_table(int table_id);
int shutdown_db();/*
void buf_print_lru();
void buf_print_all();*/

buffer_t* buf_pool;
extern int buf_size;
extern LRU_list lru_list;
extern pthread_mutex_t buf_pool_latch;

#endif