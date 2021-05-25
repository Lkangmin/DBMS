#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include"buffer_manager.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

// Default order is 4.
//#define DEFAULT_ORDER 4

// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 20

// Constants for printing part or all of the GPL license.
#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625

// TYPES.

typedef struct Node* PtrToNode;
typedef PtrToNode Queue;
struct Node {
    pagenum_t id;
    Queue next;
};
/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */
typedef struct record {
    int value;
} record;

/* Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
typedef struct node {
    void ** pointers;
    int * keys;
    struct node * parent;
    bool is_leaf;
    int num_keys;
    struct node * next; // Used for queue.
} node;

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
//extern int order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
extern bool verbose_output;


// FUNCTION PROTOTYPES.

// Output and utility.
/*
void license_notice( void );
void print_license( int licence_part );
void usage_1( void );
void usage_2( void );
void usage_3( void );*/
//void enqueue( node * new_node );
//node * dequeue( void );
/*Queue make_new_queue();//
int is_empty(Queue queue);//
void enqueue(pagenum_t pagenum, Queue queue);//
pagenum_t dequeue(Queue queue);//
int path_to_root(int table_id, pagenum_t pagenum);//
void print_tree(int table_id);//*/
//int height( node * root );
//int path_to_root( node * root, node * child );
//void print_leaves( node * root );
//void print_tree();
//void find_and_print(node * root, int key, bool verbose); 
//void find_and_print_range(node * root, int range1, int range2, bool verbose); 
//int find_range( node * root, int key_start, int key_end, bool verbose, int returned_keys[], void * returned_pointers[]); 
pagenum_t find_leaf(int table_id, pagenum_t root, int64_t key);
//record * find( node * root, int key, bool verbose );
int cut( int length );

// Insertion.

Record * make_record(int64_t key,char* value);
page_t * make_node( void );
page_t * make_leaf( void );
int get_left_index(buffer_t* parent_page, pagenum_t left);
void insert_into_leaf(int table_id, pagenum_t leaf_num, Record * record );
void insert_into_leaf_after_splitting(int table_id, pagenum_t leaf_num, buffer_t *leaf_page, Record *record);
void insert_into_node(int table_id, pagenum_t parent_num, buffer_t* parent_page, int left_index, int64_t key, pagenum_t right_num);
void insert_into_node_after_splitting(int table_id, pagenum_t old_node_num , buffer_t *old_node, int left_index, int64_t key, pagenum_t right_num);
void insert_into_parent(int table_id, pagenum_t left_num, buffer_t* left, int64_t key, pagenum_t right_num, buffer_t* right);
void insert_into_new_root(int table_id, pagenum_t left_num, buffer_t* left, int64_t key, pagenum_t right_num, buffer_t* right);
pagenum_t start_new_tree(int table_id ,int key, char* value);
//node * insert( node * root, int key, int value );

// Deletion.

int get_neighbor_index( buffer_t *page, pagenum_t num );
void adjust_root(int table_id, pagenum_t root_num, buffer_t * root);
void coalesce_nodes(int table_id, pagenum_t page_num, buffer_t* page, pagenum_t parent_num, buffer_t* parent_page, pagenum_t neighbor_num, int neighbor_index, int64_t k_prime);
void redistribute_nodes(int table_id, pagenum_t page_num, buffer_t* page, pagenum_t parent_num, buffer_t* parent_page, pagenum_t neighbor_num, int neighbor_index, int64_t k_prime);
void delete_entry(int table_id,pagenum_t page_num, int64_t key);
//node * delete( node * root, int key );
//void destroy_tree_nodes(node * root);
//node * destroy_tree(node * root);

//db api
int open_table(char* pathname);
int db_find(int table_id,int64_t key, char * ret_val);
int db_insert(int table_id,int64_t key, char * value);
int db_delete(int table_id,int64_t key);

#endif /* __BPT_H__*/
