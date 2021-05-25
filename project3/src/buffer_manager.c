#include"buffer_manager.h"
#include"file.h"

int buf_size;
LRU_list lru_list;

int buf_open_table(char* pathname){
	return file_open_table(pathname);	
}

int init_db(int num_buf){

	buf_pool = (buffer_t*)malloc(sizeof(buffer_t)*num_buf);
	
	if(buf_pool == NULL) return 1;

	file_num = 0;
	buf_size = num_buf;
	lru_list.head = (buffer_t*)malloc(sizeof(buffer_t));
	lru_list.tail = (buffer_t*)malloc(sizeof(buffer_t));
	lru_list.head->is_pinned = 1;
	lru_list.tail->is_pinned = 1;
	lru_list.head->next = lru_list.tail;
	lru_list.head->prev = lru_list.tail;
	lru_list.tail->next = lru_list.head;
	lru_list.tail->prev = lru_list.head;

	for(int i=0;i<num_buf;i++)
		buf_pool[i].table_id = -1;

	for(int i=0;i<11;i++)
		fd_table[i]=-1;

	return 0;
}


buffer_t* buf_get(int table_id, pagenum_t page_num){

	int i,cnt;
	buffer_t* temp;

	//해당 buf 있는지 확인
	for(i=0;i<buf_size;i++){
		if(buf_pool[i].table_id == table_id && buf_pool[i].page_num == page_num){
			buf_pool[i].is_pinned=1;
			return &buf_pool[i];
		}
	}
	//pool full인지 확인
	for(cnt=0;cnt<buf_size;cnt++){
		if(buf_pool[cnt].table_id==-1) break;
	}

	//pool full이 아닐때
	if(cnt<buf_size){
		file_read_page(table_id,page_num,&buf_pool[cnt].frame);

		buf_pool[cnt].table_id = table_id;
		buf_pool[cnt].page_num = page_num;
		buf_pool[cnt].is_dirty = 0;
		buf_pool[cnt].is_pinned = 1;
		lru_list.head->next->prev = &buf_pool[cnt];
		buf_pool[cnt].next = lru_list.head->next;
	 	lru_list.head->next = &buf_pool[cnt];
		buf_pool[cnt].prev = lru_list.head;
		return &buf_pool[cnt];
	}
	temp = lru_list.tail->prev;
	while(1){
		if(!temp->is_pinned){

			if(temp->is_dirty)
				file_write_page(temp->table_id,temp->page_num,&temp->frame);

			file_read_page(table_id,page_num,&temp->frame);
			temp->table_id = table_id;
			temp->page_num = page_num;
			temp->is_dirty = 0;
			temp->is_pinned = 1;
			break;
		}
		temp = temp->prev;
	}
	return temp;
}

void buf_put(buffer_t* temp){
	temp->is_pinned = 0;
	temp->is_dirty = 1;
}


pagenum_t buf_alloc(int table_id){
	buffer_t *header,*temp;
	pagenum_t ret;

	header = buf_get(table_id,0);
	ret = header->head_p.free_page_num;
  
  	if(ret != 0){
  		temp = buf_get(table_id,ret);
  		header->head_p.free_page_num = temp->free_p.next_free_page_num;
  		buf_put(temp);
  	}
  	else{
  		ret = header->head_p.page_num;
  		header->head_p.page_num++;
  	}
  	buf_put(header);
  	return ret;
}

void buf_free(int table_id,pagenum_t page_num){
	buffer_t *header;
	FreePage new_free;
	
	header = buf_get(table_id,0);
	new_free.next_free_page_num = header->head_p.free_page_num;
	header->head_p.free_page_num = page_num;
	buf_put(header);
	file_write_page(table_id,page_num,(page_t*)(&new_free));
}


int close_table(int table_id){
	for(int i=0;i<buf_size;i++){
		if(buf_pool[i].table_id == fd_table[table_id]){
			if(buf_pool[i].is_dirty)
				file_write_page(fd_table[table_id],buf_pool[i].page_num,&buf_pool[i].frame);
			memset(&buf_pool[i].frame,0,sizeof(page_t));
			buf_pool[i].table_id = -1;
			buf_pool[i].page_num = -1;
			buf_pool[i].is_dirty = 0;
			buf_pool[i].is_pinned = 0;
			buf_pool[i].prev->next = buf_pool[i].next;
			buf_pool[i].next->prev = buf_pool[i].prev;
			buf_pool[i].next = NULL;
			buf_pool[i].prev = NULL;
		}
	}
	return 0;
}



int shutdown_db(){
	for(int i=1;i<=file_num;i++){
		close_table(i);
		close(fd_table[i]);
	}
	free(lru_list.head);
	free(lru_list.tail);
	free(buf_pool);
	return 0;
}
/*
void buf_print_lru(){
	buffer_t* temp;
	temp = lru_list.head->next;
	printf("Head start!\n");
	while(temp != lru_list.tail){
		printf("buf tid: %d, page: %lld, dirty:%d, pin: %d\n",temp->table_id,temp->page_num,temp->is_dirty,temp->is_pinned);
		temp = temp->next;
	}
	printf("arrive Tail\n\n");
}

void buf_print_all(){
	for(int i=0;i<buf_size;i++){
		if(buf_pool[i].table_id != -1)
			printf("buf tid: %d, page: %lld, dirty:%d, pin: %d\n",buf_pool[i].table_id,buf_pool[i].page_num,buf_pool[i].is_dirty,buf_pool[i].is_pinned);
	}
	printf("\n");
}*/