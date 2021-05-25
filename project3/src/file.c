#include"file.h"

int fd_table[11];
char file_name[11][20];
int file_num;

int file_open_table(char* pathname){
    HeaderPage head_page_p;
    int i,fd;
    for(i=1;i<=file_num;i++){
    	if(!strcmp(pathname,file_name[i]))
    		return i;
    }

    if(i > 10)  return -1;

    //new file
    fd = open(pathname, O_RDWR|O_CREAT|O_SYNC, S_IRWXG|S_IRWXU|S_IRWXO);
    
    file_num++;
   	strcpy(file_name[file_num],pathname);
   	fd_table[file_num]=fd;

    file_read_page(fd_table[file_num],0,(page_t*)&head_page_p);
    if(head_page_p.page_num==0){
    	head_page_p.free_page_num = 0;
    	head_page_p.root_page_num = 0;
    	head_page_p.page_num = 1;
    	if(pwrite(fd_table[file_num],&head_page_p,PAGE_SIZE,0) != PAGE_SIZE){
	    	printf("Fail write head\n");
	    	exit(1);
    	}
    }
   // printf("head root page : %lld",head_page_p.root_page_num);
    return file_num;
}


/*
pagenum_t file_alloc_page(int table_id){
	HeaderPage headerpage;
	FreePage freepage;
	pagenum_t pagenum;

	file_read_page(table_id,0,(page_t*)head_page_p);
	pagenum = head_page_p->free_page_num;
	if(pagenum != 0){
		file_read_page(table_id,pagenum,(page_t*)(&freepage));
		head_page_p->free_page_num = freepage.next_free_page_num;
		file_write_page(0,(page_t*)head_page_p);
	}
	else
	{
		pagenum = head_page_p->page_num;
		head_page_p->page_num++;
		file_write_page(table_id,0,(page_t*)head_page_p);
	}
	return pagenum;
}
*/
/*
void file_free_page(int table_id,pagenum_t pagenum){
	FreePage new_free;
	pagenum_t next_free_num;

	file_read_page(table_id,0,(page_t*)head_page_p);
	next_free_num = head_page_p->free_page_num;
	head_page_p->free_page_num=pagenum;
	file_write_page(table_id,0,(page_t*)head_page_p);

	new_free.next_free_page_num = next_free_num;
	file_write_page(table_id,pagenum,(page_t*)(&new_free));
}*/


void file_read_page(int table_id,pagenum_t pagenum, page_t* dest){
	pread(table_id,dest,PAGE_SIZE,PAGE_SIZE*pagenum);
}


void file_write_page(int table_id,pagenum_t pagenum,const page_t* src){
	int flag = pwrite(table_id,src,PAGE_SIZE,PAGE_SIZE*pagenum);
	if(flag!=PAGE_SIZE){
		printf("Fail write page\n");
		exit(1);
	}
}
/*
void file_print(){
	printf("file state!\n");
	for(int i=1;i<=file_num;i++){
		printf("file name: %s  file fd: %d  file id: %d\n",file_name[i],fd_table[i],i);
	}
}*/