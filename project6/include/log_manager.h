#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__

#include"trx_manager.h"
#include"lock_manager.h"
#include"buffer_manager.h"
#include<inttypes.h>
#include<pthread.h>

#define BEGIN 0
#define UPDATE 1
#define COMMIT 2
#define ROLLBACK 3
#define COMPENSATE 4

typedef struct log_t{
	int log_size;
	int64_t LSN;
	int64_t prev_LSN;
	int trx_id;
	int type;
	int table_id;
	int64_t pagenum;
	int offset;
	int data_len;
	char old_image[120];
	char new_image[120];
	int64_t next_undo_LSN;
}log_t;


extern log_t* log_buffer;
extern int64_t cur_LSN;
extern pthread_mutex_t log_latch;
extern int total_log;
extern int log_fd;
//extern FILE* logmsg_fd;
extern int flushed_log;
extern int loser_list[10000];
extern int loser_num;

int64_t make_log_default(int trx_id,int type);
int64_t make_log_update(int trx_id,int type, int table_id, int64_t pagenum, int offset, char* old_image, char* new_image);
int64_t make_log_com(int trx_id,int type, int table_id, int64_t pagenum, int offset, char* old_image, char* new_image);
int record_offset(int index);
int record_index(int offset);
void flush();
void rollback(int trx_id);
int64_t find_next_undo(int trx_id, char* old_image);
void recovery(int flag, int log_num);
void redo(int log_num);
int have_com(int64_t LSN);
void undo(int log_num);

#endif