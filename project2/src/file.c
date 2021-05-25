#include"file.h"

int fd = -1;

int open_table(char* pathname){
    
    fd = open(pathname, O_RDWR|O_CREAT|O_SYNC, S_IRWXG|S_IRWXU|S_IRWXO);
    if(!fd) return -1;

    head_page_p = (HeaderPage*)malloc(sizeof(HeaderPage));
    file_read_page(0,(page_t*)head_page_p);
    if(head_page_p->page_num==0){
    	head_page_p->free_page_num = 0;
    	head_page_p->root_page_num = 0;
    	head_page_p->page_num = 1;
    	if(pwrite(fd,head_page_p,PAGE_SIZE,0) != PAGE_SIZE){
	    	printf("Fail write head\n");
	    	exit(1);
    	}
    }
    return fd;
}



pagenum_t file_alloc_page(){
	HeaderPage headerpage;
	FreePage freepage;
	pagenum_t pagenum;

	file_read_page(0,(page_t*)head_page_p);
	pagenum = head_page_p->free_page_num;
	if(pagenum != 0){
		file_read_page(pagenum,(page_t*)(&freepage));
		head_page_p->free_page_num = freepage.next_free_page_num;
		file_write_page(0,(page_t*)head_page_p);
	}
	else
	{
		pagenum = head_page_p->page_num;
		head_page_p->page_num++;
		file_write_page(0,(page_t*)head_page_p);
	}
	return pagenum;
}


void file_free_page(pagenum_t pagenum){
	FreePage new_free;
	pagenum_t next_free_num;

	file_read_page(0,(page_t*)head_page_p);
	next_free_num = head_page_p->free_page_num;
	head_page_p->free_page_num=pagenum;
	file_write_page(0,(page_t*)head_page_p);

	new_free.next_free_page_num = next_free_num;
	file_write_page(pagenum,(page_t*)(&new_free));
}


void file_read_page(pagenum_t pagenum, page_t* dest){
	pread(fd,dest,PAGE_SIZE,PAGE_SIZE*pagenum);
}


void file_write_page(pagenum_t pagenum,const page_t* src){
	int flag = pwrite(fd,src,PAGE_SIZE,PAGE_SIZE*pagenum);
	if(flag!=PAGE_SIZE){
		printf("Fail write page\n");
		exit(1);
	}
}