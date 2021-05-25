#ifndef __FILE_H__
#define __FILE_H__

#include<inttypes.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>


#define PAGE_SIZE 4096
#define LEAF_ORDER 32
#define INTERNAL_ORDER 249


typedef uint64_t pagenum_t;

typedef struct{
	int64_t key;
	char value[120];
}Record;


typedef struct{
	int64_t key;
	pagenum_t page_offset;
}Entry;


typedef struct {
	pagenum_t free_page_num;
	pagenum_t root_page_num;
	pagenum_t page_num;
	char reserved[4072];
}HeaderPage;

typedef struct {
	pagenum_t next_free_page_num;
	char notused[4088];
} FreePage;


typedef struct {

	pagenum_t parent_page_num;
	int is_leaf;
	int key_num;
	char reserved[104];		

	union{
		pagenum_t right_sibling_page_num;
		pagenum_t left_most_num;
	};

	union{		
		Record record[31];
		Entry entry[248];
	};
}page_t;



extern int fd;
HeaderPage* head_page_p;

int open_table(char* pathname);
pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);
void file_read_page(pagenum_t pagenum, page_t* dest);
void file_write_page(pagenum_t pagenum,const page_t* src);

#endif
