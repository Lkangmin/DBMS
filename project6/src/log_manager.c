#include"log_manager.h"

log_t* log_buffer;
int64_t cur_LSN = 0;
pthread_mutex_t log_latch = PTHREAD_MUTEX_INITIALIZER;
int total_log = 0;
int log_fd;
FILE* logmsg_fd;
int flushed_log = 0;
int loser_list[10000];
int loser_num=0;

int64_t make_log_default(int trx_id,int type){
	cur_LSN += 28;
	log_buffer[total_log].LSN = cur_LSN;
	log_buffer[total_log].trx_id = trx_id;
	log_buffer[total_log].type = type;
	total_log++;
	return log_buffer[total_log].LSN;
}

int64_t make_log_update(int trx_id,int type, int table_id, int64_t pagenum, int offset, char* old_image, char* new_image){
	cur_LSN += 288;
	log_buffer[total_log].LSN = cur_LSN;
	log_buffer[total_log].trx_id = trx_id;
	log_buffer[total_log].type = type;
	log_buffer[total_log].data_len = 120;
	log_buffer[total_log].table_id = table_id;
	log_buffer[total_log].pagenum = pagenum;
	log_buffer[total_log].offset = (offset+1)*128 + 8;
	strcpy(log_buffer[total_log].old_image,old_image);
	strcpy(log_buffer[total_log].new_image,new_image);
	total_log++;
	return log_buffer[total_log].LSN;
}

int64_t make_log_com(int trx_id,int type, int table_id, int64_t pagenum, int offset, char* old_image, char* new_image){
	cur_LSN += 296;
	log_buffer[total_log].LSN = cur_LSN;
	log_buffer[total_log].trx_id = trx_id;
	log_buffer[total_log].type = type;
	log_buffer[total_log].data_len = 120;
	log_buffer[total_log].table_id = table_id;
	log_buffer[total_log].pagenum = pagenum;
	log_buffer[total_log].offset = (offset+1)*128 + 8;
	strcpy(log_buffer[total_log].old_image,old_image);
	strcpy(log_buffer[total_log].new_image,new_image);
	log_buffer[total_log].next_undo_LSN = find_next_undo(trx_id,old_image);
	total_log++;
	return log_buffer[total_log].LSN;
}

int64_t find_next_undo(int trx_id, char* old_image){
	for(int i=0;i<total_log;i++){
		if(log_buffer[i].trx_id == trx_id && log_buffer[i].type == UPDATE){
			if(!strcmp(old_image,log_buffer[i].new_image))
				return log_buffer[i].LSN;
		}
	}
	return 0;
}


int record_offset(int index){
	return (1 + index)*128 + 8;
}

int record_index(int offset){
	return (offset - 8)/128 - 1;
}

void flush(){
	log_t* buf;
	for(int i = flushed_log;i<total_log;i++){
		if(log_buffer[i].type == UPDATE){
			buf = &log_buffer[i];
			write(log_fd,buf,288);
		}
		else if(log_buffer[i].type == COMPENSATE){
			buf = &log_buffer[i];
			write(log_fd,buf,296);
		}
		else{
			buf = &log_buffer[i];
			write(log_fd,buf,28);
		}
	}
}


void rollback(int trx_id){
	for(int i = 0; i<total_log; i++){
		if(log_buffer[i].trx_id == trx_id && log_buffer[i].type == UPDATE){
			make_log_com(trx_id,COMPENSATE,log_buffer[i].table_id,log_buffer[i].pagenum,log_buffer[i].offset,log_buffer[i].new_image,log_buffer[i].old_image);
			buffer_t* temp = buf_get(log_buffer[i].table_id,log_buffer[i].pagenum);
			int record = record_index(log_buffer[i].offset);
			strcpy(temp->frame.record[record].value,log_buffer[i].old_image);
			buf_put(temp);
		}
	}
}

void recovery(int flag, int log_num){
	if(flag == 0){
		redo(-1);
		undo(-1);
	}
	else if(flag == 1){
		redo(log_num);
	}
	else if(flag ==2){
		redo(-1);
		undo(log_num);
	}
}


void redo(int log_num){
	log_t* temp = (log_t*)malloc(sizeof(log_t));
//	fprintf(logmsg_fd, "[ANALYSIS] Analysis pass start\n");
//	fprintf(logmsg_fd, "[REDO] Redo pass start\n");
	while(pread(log_fd,temp,296,cur_LSN) > 0){
		flushed_log++;
		if(temp->type == BEGIN){
			loser_list[loser_num] = temp->trx_id;
			loser_num++;
			make_log_default(temp->trx_id,BEGIN);
			//fprintf(logmsg_fd, "LSN %llu [BEGIN] Transaction id %d\n",,temp->trx_id);
		}
		else if(temp->type == UPDATE){
			char datafile[100];
			sprintf(datafile,"DATA%d",temp->table_id);
			file_open_table(datafile);
			buffer_t* buf = buf_get(temp->table_id,temp->pagenum);
			int record = record_index(temp->offset);
			make_log_update(temp->trx_id,UPDATE,temp->table_id,temp->pagenum,temp->offset,temp->old_image,temp->new_image);
			//fprintf(logmsg_fd, "LSN %llu [UPDATE] Transaction id %d\n",,temp->trx_id);
			if(buf->frame.page_LSN < temp->LSN){
				buf->frame.page_LSN = temp->LSN;
				strcpy(buf->frame.record[record].value,temp->new_image);
			}
			buf_put(buf);
		}
		else if(temp->type == COMPENSATE){
			char datafile[100];
			sprintf(datafile,"DATA%d",temp->table_id);
			file_open_table(datafile);
			buffer_t* buf = buf_get(temp->table_id,temp->pagenum);
			int record = record_index(temp->offset);
			make_log_com(temp->trx_id,COMPENSATE,temp->table_id,temp->pagenum,temp->offset,temp->new_image,temp->old_image);
			if(buf->frame.page_LSN < temp->LSN){
				buf->frame.page_LSN = temp->LSN;
				strcpy(buf->frame.record[record].value,temp->old_image);
			}
			buf_put(buf);
		}
		else if(temp->type == COMMIT || temp->type == ROLLBACK){
			for(int i = 0;i<loser_num;i++){
				if(loser_list[i] == temp->trx_id){
					loser_list[i] = loser_list[loser_num-1];
					loser_num--;
				}
			}
			make_log_default(temp->trx_id,temp->type);/*
			if(temp->type == COMMIT)
				fprintf(logmsg_fd, "LSN %llu [COMMIT] Transaction id %d\n",t_LSN,temp->trx_id);
			else
				fprintf(logmsg_fd, "LSN %llu [ROLLBACK] Transaction id %d\n",t_LSN,temp->trx_id);*/
		}
		memset(temp,0,sizeof(log_t));
		if(log_num != -1){
			if(log_num == total_log)
				break;
		}
	}
}

int have_com(int64_t LSN){
	for(int i=0;i<total_log;i++){
		if(log_buffer[i].type == COMPENSATE && log_buffer[i].next_undo_LSN == LSN){
			return 1;
		}
	}
	return 0;
}

void undo(int log_num){
	int undo_cnt=0;
//	fprintf(logmsg_fd, "[UNDO] undo pass start\n");
	for(int i = total_log-1;i>=0;i++){
		for(int j=0;j<loser_num;j++){
			if(log_buffer[i].trx_id == loser_list[j]){
				if(log_buffer[i].type == UPDATE && !have_com(log_buffer[i].LSN)){
					buffer_t* buf = buf_get(log_buffer[i].table_id,log_buffer[i].pagenum);
					int record = record_index(log_buffer[i].offset);
				//	fprintf(logmsg_fd, "LSN %llu [CLR] Undo Transaction id %d\n",make_log_com(log_buffer[i].trx_id,COMPENSATE,log_buffer[i].table_id,log_buffer[i].pagenum,log_buffer[i].offset,log_buffer[i].new_image,log_buffer[i].old_image),log_buffer[i].trx_id);
					if(buf->frame.page_LSN >= log_buffer[i].LSN){
						buf->frame.page_LSN = log_buffer[i].LSN;
						strcpy(buf->frame.record[record].value,log_buffer[i].old_image);
					}
					buf_put(buf);
					undo_cnt++;
				}
			}
		}
		if(log_num != -1){
			if(log_num == undo_cnt)
				break;
		}
	}
}

