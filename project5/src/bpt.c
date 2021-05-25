/*
 *  bpt.c  
 */
#define Version "1.14"
/*
 *
 *  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
 
 *  3. Neither the name of the copyright holder nor the names of its 
 *  contributors may be used to endorse or promote products derived from this 
 *  software without specific prior written permission.
 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 
 *  Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *  Original Date:  26 June 2010
 *  Last modified: 17 June 2016
 *
 *  This implementation demonstrates the B+ tree data structure
 *  for educational purposes, includin insertion, deletion, search, and display
 *  of the search path, the leaves, or the whole tree.
 *  
 *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 *  Usage:  bpt [order]
 *  where order is an optional argument
 *  (integer MIN_ORDER <= order <= MAX_ORDER)
 *  defined as the maximal number of pointers in any node.
 *
 */
#include"file.h"
#include "bpt.h"

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


/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
node * queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
bool verbose_output = false;
//int order = DEFAULT_ORDER;

// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

/* Copyright and license notice to user at startup. 
 *//*
void license_notice( void ) {
    printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
            "http://www.amittai.com\n", Version);
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details "
            "type `show w'.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions; type `show c' for details.\n\n");
}
*/

/* Routine to print portion of GPL license to stdout.
 *//*
void print_license( int license_part ) {
    int start, end, line;
    FILE * fp;
    char buffer[0x100];

    switch(license_part) {
    case LICENSE_WARRANTEE:
        start = LICENSE_WARRANTEE_START;
        end = LICENSE_WARRANTEE_END;
        break;
    case LICENSE_CONDITIONS:
        start = LICENSE_CONDITIONS_START;
        end = LICENSE_CONDITIONS_END;
        break;
    default:
        return;
    }

    fp = fopen(LICENSE_FILE, "r");
    if (fp == NULL) {
        perror("print_license: fopen");
        exit(EXIT_FAILURE);
    }
    for (line = 0; line < start; line++)
        fgets(buffer, sizeof(buffer), fp);
    for ( ; line < end; line++) {
        fgets(buffer, sizeof(buffer), fp);
        printf("%s", buffer);
    }
    fclose(fp);
}


/* First message to the user.
 *//*
void usage_1( void ) {
    printf("B+ Tree of Order %d.\n", order);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
           "5th ed.\n\n"
           "To build a B+ tree of a different order, start again and enter "
           "the order\n"
           "as an integer argument:  bpt <order>  ");
    printf("(%d <= order <= %d).\n", MIN_ORDER, MAX_ORDER);
    printf("To start with input from a file of newline-delimited integers, \n"
           "start again and enter the order followed by the filename:\n"
           "bpt <order> <inputfile> .\n");
}


/* Second message to the user.
 *//*
void usage_2( void ) {
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tp <k> -- Print the path from the root to key k and its associated "
           "value.\n"
    "\tr <k1> <k2> -- Print the keys and values found in the range "
            "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
           "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and "
           "leaves.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");
}


/* Brief usage note.
 *//*
void usage_3( void ) {
    printf("Usage: ./bpt [<order>]\n");
    printf("\twhere %d <= order <= %d .\n", MIN_ORDER, MAX_ORDER);
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
/*
void enqueue( pagenum_t new_node ) {
    node * c;
    if (queue == NULL) {
        queue = new_node;
        queue->next = NULL;
    }
    else {
        c = queue;
        while(c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
        new_node->next = NULL;
    }
}
*/


/* Helper function for printing the
 * tree out.  See print_tree.
 */
/*
node * dequeue( void ) {
    node * n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}*//*
Queue make_new_queue() {
    Queue q = (Queue)malloc(sizeof(struct Node));
    q->next = NULL;
    return q;
}
int is_empty(Queue queue) {
    return queue->next == NULL;
}
void enqueue(pagenum_t new_pagenum, Queue queue) {
    Queue c = queue;
    Queue end_c = (Queue)malloc(sizeof(struct Node));
    while (c->next != NULL) {
        c = c->next;
    }
    end_c->id = new_pagenum;
    end_c->next = NULL;
    c->next = end_c;
}
pagenum_t dequeue(Queue queue) {
    Queue temp = queue->next;
    pagenum_t ret = temp->id;
    queue->next = temp->next;
    free(temp);
    temp = NULL;
    return ret;
}
*/
/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
/*
void print_leaves( node * root ) {
    int i;
    node * c = root;
    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    while (!c->is_leaf)
        c = c->pointers[0];
    while (true) {
        for (i = 0; i < c->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)c->pointers[i]);
            printf("%d ", c->keys[i]);
        }
        if (verbose_output)
            printf("%lx ", (unsigned long)c->pointers[order - 1]);
        if (c->pointers[order - 1] != NULL) {
            printf(" | ");
            c = c->pointers[order - 1];
        }
        else
            break;
    }
    printf("\n");
}
*/

/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 *//*
int height( node * root ) {
    int h = 0;
    node * c = root;
    while (!c->is_leaf) {
        c = c->pointers[0];
        h++;
    }
    return h;
}
*/

/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
/*
int path_to_root( node * root, node * child ) {
    int length = 0;
    node * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}
*/

/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */

/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
/*
void find_and_print(node * root, int key, bool verbose) {
    record * r = find(root, key, verbose);
    if (r == NULL)
        printf("Record not found under key %d.\n", key);
    else 
        printf("Record at %lx -- key %d, value %d.\n",
                (unsigned long)r, key, r->value);
}
*/

/* Finds and prints the keys, pointers, and values within a range
 * of keys between key_start and key_end, including both bounds.
 */
/*
void find_and_print_range( node * root, int key_start, int key_end,
        bool verbose ) {
    int i;
    int array_size = key_end - key_start + 1;
    int returned_keys[array_size];
    void * returned_pointers[array_size];
    int num_found = find_range( root, key_start, key_end, verbose,
            returned_keys, returned_pointers );
    if (!num_found)
        printf("None found.\n");
    else {
        for (i = 0; i < num_found; i++)
            printf("Key: %d   Location: %lx  Value: %d\n",
                    returned_keys[i],
                    (unsigned long)returned_pointers[i],
                    ((record *)
                     returned_pointers[i])->value);
    }
}
*/

/* Finds keys and their pointers, if present, in the range specified
 * by key_start and key_end, inclusive.  Places these in the arrays
 * returned_keys and returned_pointers, and returns the number of
 * entries found.
 */
/*
int find_range( node * root, int key_start, int key_end, bool verbose,
        int returned_keys[], void * returned_pointers[]) {
    int i, num_found;
    num_found = 0;
    node * n = find_leaf( root, key_start, verbose );
    if (n == NULL) return 0;
    for (i = 0; i < n->num_keys && n->keys[i] < key_start; i++) ;
    if (i == n->num_keys) return 0;
    while (n != NULL) {
        for ( ; i < n->num_keys && n->keys[i] <= key_end; i++) {
            returned_keys[num_found] = n->keys[i];
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }
        n = n->pointers[order - 1];
        i = 0;
    }
    return num_found;
}
*/

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
pagenum_t find_leaf(int table_id, pagenum_t root, int64_t key) {
    int i = 0;
    buffer_t* c;

    if (root == 0) 
        return root;
    
    c = buf_get(table_id,root);
    
    while (!c->frame.is_leaf) {
        i = -1;
        while (i+1 < c->frame.key_num) {
            if (key >= c->frame.entry[i+1].key) i++;
            else break;
        }
        if(i == -1) root = c->frame.left_most_num;
        else root = c->frame.entry[i].page_offset;
        buf_put(c);
        c = buf_get(table_id,root);
        
    }
    buf_put(c);
    return root;
}


/* Finds and returns the record to which
 * a key refers.
 /
record * find( node * root, int key, bool verbose ) {
    int i = 0;
    node * c = find_leaf( root, key, verbose );
    if (c == NULL) return NULL;
    for (i = 0; i < c->num_keys; i++)
        if (c->keys[i] == key) break;
    if (i == c->num_keys) 
        return NULL;
    else
        return (record *)c->pointers[i];
}*/

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}


// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
 */
Record * make_record(int64_t key, char* value) {
    Record * new_record = (Record *)malloc(sizeof(Record));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->key = key;
        strcpy(new_record->value,value);
    }
    return new_record;
}


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
page_t * make_node( void ) {
    page_t * new_node;
    new_node = (page_t*)malloc(sizeof(page_t));
    memset(new_node,0,sizeof(page_t));
    return new_node;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
page_t * make_leaf( void ) {
    page_t * leaf;
    leaf = (page_t*)malloc(sizeof(page_t));
    memset(leaf,0,sizeof(page_t));
    leaf->is_leaf = 1;
    return leaf;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index(buffer_t* parent_page, pagenum_t left) {
    int i;
    int left_index = 0;
    
    if(parent_page->frame.left_most_num == left) return 0;
    for(i=0;i<parent_page->frame.key_num;i++){
        if(parent_page->frame.entry[i].page_offset == left){
            return i + 1;
        }
    }
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
void insert_into_leaf(int table_id, pagenum_t leaf_num, Record * record ) {

    int i, insertion_point;
    buffer_t* leaf_page;
    
    leaf_page = buf_get(table_id,leaf_num);
    insertion_point = 0;
    while (insertion_point < leaf_page->frame.key_num && leaf_page->frame.record[insertion_point].key < record->key)
        insertion_point++;

    for (i = leaf_page->frame.key_num; i > insertion_point; i--) {
        leaf_page->frame.record[i].key = leaf_page->frame.record[i - 1].key;
        strcpy(leaf_page->frame.record[i].value,leaf_page->frame.record[i - 1].value);
    }
    leaf_page->frame.record[insertion_point].key = record->key;
    strcpy(leaf_page->frame.record[insertion_point].value,record->value);
    leaf_page->frame.key_num++;
    buf_put(leaf_page);
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting(int table_id, pagenum_t leaf_num, buffer_t *leaf_page, Record *record) {

    buffer_t* new_leaf;
    Record temp[33];
    int insertion_index, split, new_key, i, j;
    int64_t new_leaf_key;
    pagenum_t new_leaf_num = buf_alloc(table_id);
    new_leaf = buf_get(table_id,new_leaf_num);
    new_leaf->frame.is_leaf=1;

    if (temp == NULL) {
        perror("Temporary array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    while (insertion_index < LEAF_ORDER - 1 && leaf_page->frame.record[insertion_index].key < record->key)
        insertion_index++;

    for (i = 0, j = 0; i < leaf_page->frame.key_num; i++, j++) {
        if (j == insertion_index) j++;
        memcpy(&temp[j],&leaf_page->frame.record[i],sizeof(Record));
    }
    memcpy(&temp[insertion_index],record,sizeof(Record));

    leaf_page->frame.key_num = 0;
    split = cut(LEAF_ORDER - 1);

    for (i = 0; i < split; i++) {
        memcpy(&leaf_page->frame.record[i],&temp[i],sizeof(Record));
        leaf_page->frame.key_num++;
    }

    for (i = split, j = 0; i < LEAF_ORDER; i++, j++) {
        memcpy(&new_leaf->frame.record[j],&temp[i],sizeof(Record));
        new_leaf->frame.key_num++;
    }

    new_leaf->frame.right_sibling_page_num = leaf_page->frame.right_sibling_page_num;
    leaf_page->frame.right_sibling_page_num = new_leaf_num;
    new_leaf->frame.parent_page_num = leaf_page->frame.parent_page_num;
    new_leaf_key = new_leaf->frame.record[0].key;
    
    buf_put(leaf_page);
    buf_put(new_leaf);

    insert_into_parent(table_id,leaf_num,leaf_page, new_leaf_key, new_leaf_num, new_leaf);
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(int table_id, pagenum_t parent_num, buffer_t* parent_page, int left_index, int64_t key, pagenum_t right_num) {
    
    int i;
    for (i = parent_page->frame.key_num; i > left_index; i--) {
        memcpy(&parent_page->frame.entry[i],&parent_page->frame.entry[i-1],sizeof(Entry));
    }
    parent_page->frame.entry[left_index].key = key;
    parent_page->frame.entry[left_index].page_offset = right_num;
    parent_page->frame.key_num++;
    buf_put(parent_page);
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(int table_id, pagenum_t old_node_num , buffer_t *old_node, int left_index, int64_t key, pagenum_t right_num) {

    int i, j, split;
    int64_t k_prime;
    buffer_t * new_node, * child;
    Entry temp[250];
    pagenum_t child_num, new_node_num;


    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */
    memset(temp,0,sizeof(temp));
    for (i = 0, j = 0; i < old_node->frame.key_num; i++, j++) {
        if (j == left_index + 1) j++;
        temp[j].page_offset = old_node->frame.entry[i].page_offset;
        temp[j].key = old_node->frame.entry[i].key;
    }
    temp[left_index].page_offset = right_num;
    temp[left_index].key = key;
    
        /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(INTERNAL_ORDER);
    new_node_num = buf_alloc(table_id);
    new_node = buf_get(table_id,new_node_num);
    old_node->frame.key_num = 0;
    for (i = 0; i < split; i++) {
        old_node->frame.entry[i].page_offset =  temp[i].page_offset;
        old_node->frame.entry[i].key = temp[i].key;
        old_node->frame.key_num++;
    }
    k_prime = temp[split].key;
    new_node->frame.left_most_num = temp[split].page_offset;
    new_node->frame.key_num = 0;
    for (i=split+1, j = 0;temp[i].page_offset != 0; i++, j++) {
        new_node->frame.entry[j].page_offset = temp[i].page_offset;
        new_node->frame.entry[j].key = temp[i].key;
        new_node->frame.key_num++;
    }

    new_node->frame.parent_page_num = old_node->frame.parent_page_num;

    child_num = new_node->frame.left_most_num;
    buf_put(old_node);
    buf_put(new_node);
    child = buf_get(table_id,child_num);
    child->frame.parent_page_num = new_node_num;
    buf_put(child);

    for (i = 0; i < new_node->frame.key_num; i++) {
        child_num = new_node->frame.entry[i].page_offset;
        child = buf_get(table_id,child_num);
        child->frame.parent_page_num = new_node_num;
        buf_put(child);
    }

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    insert_into_parent(table_id,old_node_num, old_node, k_prime, new_node_num,new_node);
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(int table_id, pagenum_t left_num, buffer_t* left, int64_t key, pagenum_t right_num, buffer_t* right) {

    int left_index;

    /* Case: new root. */;
    if (left->frame.parent_page_num == 0){
        insert_into_new_root(table_id,left_num,left, key, right_num,right);
        return;
     }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */
    pagenum_t parent_num = left->frame.parent_page_num;
    buffer_t* parent_page = buf_get(table_id, parent_num);
    left_index = get_left_index(parent_page, left_num);
    buf_put(parent_page);
    /* Simple case: the new key fits into the node. 
     */

    if (parent_page->frame.key_num < INTERNAL_ORDER - 1){
        insert_into_node(table_id,parent_num,parent_page, left_index, key, right_num);
        return;
    }

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    insert_into_node_after_splitting(table_id,parent_num,parent_page, left_index, key, right_num);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
void insert_into_new_root(int table_id, pagenum_t left_num, buffer_t* left, int64_t key, pagenum_t right_num, buffer_t* right) {

    pagenum_t root_num = buf_alloc(table_id);
    buffer_t* root = buf_get(table_id,root_num);
    buffer_t* header;

    root->frame.parent_page_num = 0;
    root->frame.is_leaf = 0;
    root->frame.key_num = 1;
    root->frame.left_most_num = left_num;
    root->frame.entry[0].key = key;
    root->frame.entry[0].page_offset = right_num;

    left->frame.parent_page_num = root_num;
    right->frame.parent_page_num = root_num;
    header = buf_get(table_id,0);
    header->head_p.root_page_num = root_num;

    buf_put(header);
    buf_put(left);
    buf_put(right);
    buf_put(root);
}



/* First insertion:db_insert
 * start a new tree.
 */
 pagenum_t start_new_tree(int table_id ,int key, char* value) {
    pagenum_t new_num = buf_alloc(table_id);
    buffer_t* root = buf_get(table_id,new_num);
    buf_put(root);
    root->frame.parent_page_num = 0;
    root->frame.is_leaf = 1;
    root->frame.key_num++;
    root->frame.right_sibling_page_num = 0;
    root->frame.record[0].key = key;
    strcpy(root->frame.record[0].value,value);
    return new_num;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
/*
node * insert( node * root, int key, int value ) {

    record * pointer;
    node * leaf;

    /* The current implementation ignores
     * duplicates.
     *

    if (find(root, key, false) != NULL)
        return root;

    /* Create a new record for the
     * value.
     *
    pointer = make_record(value);


    /* Case: the tree does not exist yet.
     * Start a new tree.
     *

    if (root == NULL) 
        return start_new_tree(key, pointer);


    /* Case: the tree already exists.
     * (Rest of function body.)
     *

    leaf = find_leaf(root, key, false);

    /* Case: leaf has room for key and pointer.
     *

    if (leaf->num_keys < order - 1) {
        leaf = insert_into_leaf(leaf, key, pointer);
        return root;
    }


    /* Case:  leaf must be split.
     *

    return insert_into_leaf_after_splitting(root, leaf, key, pointer);
}
*/



// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int get_neighbor_index( buffer_t *page, pagenum_t num ) {

    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = 0; i <= page->frame.key_num; i++)
        if (page->frame.entry[i].page_offset == num)
            return i;
        
    // Error state.
    return -1;
}


void remove_entry_from_node(pagenum_t page_num, buffer_t* n, int64_t key) {

    int i, num_pointers;

    // Remove the key and shift other keys accordingly.
    i = 0;
    if(n->frame.is_leaf){
        while(n->frame.record[i].key != key)
            i++;
        for(++i; i< n->frame.key_num ;i++)
            memcpy(&n->frame.record[i-1],&n->frame.record[i],sizeof(Record));
        n->frame.key_num--;
        n->frame.record[n->frame.key_num].key = 0;
        memset(n->frame.record[n->frame.key_num].value,0,120);
    }
    else{
        while(n->frame.entry[i].key != key)
            i++;
        for(++i;i<n->frame.key_num;++i)
            memcpy(&n->frame.entry[i-1],&n->frame.entry[i],sizeof(Entry));
        n->frame.key_num--;
        n->frame.entry[n->frame.key_num].key = 0;
        n->frame.entry[n->frame.key_num].page_offset=0;
    }
}


void adjust_root(int table_id, pagenum_t root_num, buffer_t *root) {

    pagenum_t new_num;
    buffer_t *new_root;
    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (root->frame.key_num > 0){
        buf_put(root);
        return ;
    }

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.
    
    if (!root->frame.is_leaf) {
        new_num = root->frame.left_most_num;
        new_root = buf_get(table_id,new_num);
        new_root->frame.parent_page_num = 0;
        buf_put(new_root);
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.
    else{
        new_num = 0;
        buffer_t *header = buf_get(table_id,0);
        header->head_p.root_page_num = new_num;
        buf_put(header);
        buf_free(table_id,root_num);
    }
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
void coalesce_nodes(int table_id, pagenum_t page_num, buffer_t* page, pagenum_t parent_num, buffer_t* parent_page,
    pagenum_t neighbor_num, int neighbor_index, int64_t k_prime) {

    buffer_t* a;
    buffer_t* temp = buf_get(table_id,neighbor_num);
    /* Swap neighbor with node if node is on the
     *extreme left and neighbor is to its right.*/

    if(neighbor_index != -1){
        if(page->frame.is_leaf){
            temp->frame.right_sibling_page_num = page->frame.right_sibling_page_num;
            k_prime = parent_page->frame.entry[neighbor_index].key;
            buf_put(temp);
            buf_free(table_id,page_num);
        }  
        else{
            temp->frame.entry[1].key = parent_page->frame.entry[neighbor_index].key;
            temp->frame.entry[1].page_offset = page->frame.left_most_num;
            a = buf_get(table_id,page->frame.left_most_num);
            a->frame.parent_page_num = neighbor_num;
            buf_put(a);
            temp->frame.key_num++;

            k_prime = parent_page->frame.entry[0].key;

            buf_put(temp);
            buf_free(table_id,page_num);
        }
    }
    else{            
        if(page->frame.is_leaf){
            page->frame.record[0].key = temp->frame.record[0].key;
            strcpy(page->frame.record[0].value,temp->frame.record[0].value);
            page->frame.right_sibling_page_num = temp->frame.right_sibling_page_num;
            page->frame.key_num++;

            k_prime = parent_page->frame.entry[0].key;

            buf_put(page);
            buf_free(table_id,neighbor_num);
            
        }
        else{
            page->frame.entry[0].key = parent_page->frame.entry[0].key;
            page->frame.entry[0].page_offset = temp->frame.left_most_num;
            a = buf_get(table_id,temp->frame.left_most_num);
            a->frame.parent_page_num = page_num;
            buf_put(a);
            page->frame.entry[1].key = temp->frame.entry[0].key;
            page->frame.entry[1].page_offset = temp->frame.entry[0].page_offset;
            a = buf_get(table_id,page->frame.entry[1].page_offset);
            a->frame.parent_page_num = page_num;
            buf_put(a);
            page->frame.key_num +=2;

            k_prime = parent_page->frame.entry[0].key;

            buf_put(page);
            buf_free(table_id,neighbor_num);
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.*/
    delete_entry(table_id,parent_num,k_prime);
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum*/
 
void redistribute_nodes(int table_id, pagenum_t page_num, buffer_t* page, pagenum_t parent_num, buffer_t* parent_page,
    pagenum_t neighbor_num, int neighbor_index, int64_t k_prime) {  

    buffer_t* temp;
    buffer_t* neighbor_page;
    neighbor_page = buf_get(table_id,neighbor_num);
    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.*/
     
    
    if (neighbor_index != -1) {
        if (page->frame.is_leaf){
            memcpy(&page->frame.record[0],&neighbor_page->frame.record[neighbor_page->frame.key_num-1],sizeof(Record));
            page->frame.key_num++;

            neighbor_page->frame.record[neighbor_page->frame.key_num-1].key = 0;
            memset(neighbor_page->frame.record[neighbor_page->frame.key_num-1].value,0,120);
            neighbor_page->frame.key_num--;

            parent_page->frame.entry[neighbor_index].key = page->frame.record[0].key;
            buf_put(parent_page);
            buf_put(page);
            buf_put(neighbor_page);
        }
        else{
            page->frame.entry[0].page_offset = page->frame.left_most_num;
            page->frame.entry[0].key = parent_page->frame.entry[neighbor_index].key;
            parent_page->frame.entry[neighbor_index].key = neighbor_page->frame.entry[neighbor_page->frame.key_num-1].key;
            page->frame.left_most_num = neighbor_page->frame.entry[neighbor_page->frame.key_num-1].page_offset;
            
            temp = buf_get(table_id,page->frame.left_most_num);
            temp->frame.parent_page_num = page_num;
            page->frame.key_num++;

            neighbor_page->frame.entry[neighbor_page->frame.key_num-1].key=0;
            neighbor_page->frame.entry[neighbor_page->frame.key_num-1].page_offset=0;
            neighbor_page->frame.key_num--;

            buf_put(parent_page);
            buf_put(page);
            buf_put(neighbor_page);
            buf_put(temp);
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.*/
     

    else {  
        if (page->frame.is_leaf) {
            memcpy(&page->frame.record[0],&neighbor_page->frame.record[0],sizeof(Record));
            page->frame.key_num++;

            for(int i=0;i<neighbor_page->frame.key_num-1;i++)
                memcpy(&neighbor_page->frame.record[i],&neighbor_page->frame.record[i+1],sizeof(Record));

            neighbor_page->frame.record[neighbor_page->frame.key_num-1].key = 0;
            memset(neighbor_page->frame.record[neighbor_page->frame.key_num-1].value,0,120);
            neighbor_page->frame.key_num--;

            parent_page->frame.entry[0].key = neighbor_page->frame.record[0].key;
            buf_put(parent_page);
            buf_put(page);
            buf_put(neighbor_page);
        }
        else{
            page->frame.entry[0].key = parent_page->frame.entry[0].key;
            page->frame.entry[0].page_offset = neighbor_page->frame.left_most_num;
            temp = buf_get(table_id,neighbor_page->frame.left_most_num);
            temp->frame.parent_page_num = page_num;
            parent_page->frame.entry[0].key = neighbor_page->frame.entry[0].key;
            page->frame.key_num++;

            neighbor_page->frame.left_most_num = neighbor_page->frame.entry[0].page_offset;
            for(int i=0;i<neighbor_page->frame.key_num-1;i++)
                memcpy(&neighbor_page->frame.entry[i],&neighbor_page->frame.entry[i+1],sizeof(Entry));
            neighbor_page->frame.entry[neighbor_page->frame.key_num-1].key=0;
            neighbor_page->frame.entry[neighbor_page->frame.key_num-1].page_offset=0;
            neighbor_page->frame.key_num--;

            temp = buf_get(table_id,page->frame.entry[0].page_offset);
            buf_put(temp);
            buf_put(parent_page);
            buf_put(page);
            buf_put(neighbor_page);            
        }
    }
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
void delete_entry(int table_id, pagenum_t page_num, int64_t key) {

    buffer_t *page,*header;
    int neighbor_index;
    int k_prime_index;
    int64_t k_prime;

    // Remove key and pointer from node.
    page = buf_get(table_id,page_num);
    remove_entry_from_node(page_num,page,key);
    buf_put(page);
    /* Case:  deletion from the root. 
     */
    header = buf_get(table_id,0);  
    if (page_num == header->head_p.root_page_num){
        buf_put(header);
        adjust_root(table_id,page_num,page);
        return;
    }
    buf_put(header);

    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */

    

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (page->frame.key_num > 0)
        return;

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */

    pagenum_t parent_num = page->frame.parent_page_num;
    buffer_t* parent_page = buf_get(table_id,parent_num);
    neighbor_index = get_neighbor_index( parent_page, page_num );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index-1;

    k_prime =parent_page->frame.entry[k_prime_index].key;
    pagenum_t neighbor_num = k_prime_index== -1 ? parent_page->frame.left_most_num : parent_page->frame.entry[k_prime_index].page_offset;
    buffer_t* neighbor_page = buf_get(table_id,neighbor_num);
    /* Coalescence. */

    if (neighbor_page->frame.key_num < 2){
        coalesce_nodes(table_id,page_num,page,parent_num,parent_page,neighbor_num, neighbor_index, k_prime);
        buf_put(parent_page);
        buf_put(page);
        buf_put(neighbor_page);
    }

    /* Redistribution. */

    else{
        redistribute_nodes(table_id,page_num,page,parent_num,parent_page,neighbor_num, neighbor_index, k_prime);
        buf_put(parent_page);
        buf_put(page);
        buf_put(neighbor_page);
    }
}



/* Master deletion function.
 *
node * delete(node * root, int key) {

    node * key_leaf;
    record * key_record;

    key_record = find(root, key, false);
    key_leaf = find_leaf(root, key, false);
    if (key_record != NULL && key_leaf != NULL) {
        root = delete_entry(root, key_leaf, key, key_record);
        free(key_record);
    }
    return root;
}
*/
/*
void destroy_tree_nodes(node * root) {
    int i;
    if (root->is_leaf)
        for (i = 0; i < root->num_keys; i++)
            free(root->pointers[i]);
    else
        for (i = 0; i < root->num_keys + 1; i++)
            destroy_tree_nodes(root->pointers[i]);
    free(root->pointers);
    free(root->keys);
    free(root);
}


node * destroy_tree(node * root) {
    destroy_tree_nodes(root);
    return NULL;
}*/

int open_table(char* pathname){
    return buf_open_table(pathname);
}

int db_find(int table_id, int64_t key, char * ret_val, int trx_id){
    pagenum_t page_num;
    buffer_t *leaf_page,*header;
    int i;

    if(fd_table[table_id] == -1) return 1;
    lock_t* lock_obj = lock_acquire(table_id,key,trx_id,SHARED);
    if(lock_obj == NULL){
  //      printf("abort!!\n");
        trx_abort(trx_id);
        return 1;
    }
    
    header = buf_get(fd_table[table_id],0);
    buf_put(header);
    page_num = find_leaf(fd_table[table_id],header->head_p.root_page_num,key);
    

    if(page_num == 0){        
        return 1;
    }

    leaf_page = buf_get(fd_table[table_id],page_num);
     buf_put(leaf_page);
    for(i=0;i<leaf_page->frame.key_num;i++){
        if(leaf_page->frame.record[i].key == key){
            strcpy(ret_val,leaf_page->frame.record[i].value);
            return 0;
        }
    }
    return 1;
}

int db_update(int table_id, int64_t key, char * values, int trx_id){ 
    pagenum_t page_num;
    buffer_t *leaf_page,*header;
    int i;
    if(fd_table[table_id] == -1) return 1;

    lock_t* lock_obj = lock_acquire(table_id,key,trx_id,EXCLUSIVE);

    if(lock_obj == NULL){
 //       printf("abort!!\n");
        trx_abort(trx_id);
        return 1;
    }
    header = buf_get(fd_table[table_id],0);
    buf_put(header);
    page_num = find_leaf(fd_table[table_id],header->head_p.root_page_num,key);
    if(page_num == 0){        
        return 1;
    }
    leaf_page = buf_get(fd_table[table_id],page_num);
    for(i=0;i<leaf_page->frame.key_num;i++){
        if(leaf_page->frame.record[i].key == key){
            pthread_mutex_lock(&lock_table_latch);
            lock_obj->pagenum = page_num;
            strcpy(lock_obj->old_value,leaf_page->frame.record[i].value);
            pthread_mutex_unlock(&lock_table_latch);
            strcpy(leaf_page->frame.record[i].value,values);
            buf_put(leaf_page);
            return 0;
        }
    }
    return 1;
}



int db_find_old(int table_id, int64_t key, char * ret_val){
    pagenum_t page_num;
    buffer_t *leaf_page,*header;
    int i;


    if(fd_table[table_id] == -1) return 1;

    header = buf_get(fd_table[table_id],0);
    buf_put(header);
    page_num = find_leaf(fd_table[table_id],header->head_p.root_page_num,key);
    
    if(page_num == 0)         
        return 1;

    leaf_page = buf_get(fd_table[table_id],page_num);
    buf_put(leaf_page);
    for(i=0;i<leaf_page->frame.key_num;i++){
        if(leaf_page->frame.record[i].key == key){
            strcpy(ret_val,leaf_page->frame.record[i].value);
            return 0;
        }
    }    
    return 1;
}


int db_insert(int table_id, int64_t key, char * value){
    Record* record;
    pagenum_t page_num,n_root,leaf_num;
    buffer_t *leaf_page,*header;
    char ret_val[120];

    if(fd_table[table_id] == -1) return 1;

    // case 1
    if(db_find_old(table_id,key,ret_val)==0) return 1;
    //case 2    
    header = buf_get(fd_table[table_id],0);    
    buf_put(header);
    if(header->head_p.root_page_num == 0){
        n_root = start_new_tree(fd_table[table_id],key,value);
        header->head_p.root_page_num = n_root;
                return 0;
    }
      record = make_record(key,value);
      leaf_num = find_leaf(fd_table[table_id],header->head_p.root_page_num,key);
      leaf_page = buf_get(fd_table[table_id],leaf_num);
      buf_put(leaf_page);
      leaf_page->frame.is_leaf = 1;
      //case 3
      if(leaf_page->frame.key_num < LEAF_ORDER -1 ){     
        insert_into_leaf(fd_table[table_id],leaf_num,record);
        return 0;
      }
      
      //case4
      insert_into_leaf_after_splitting(fd_table[table_id],leaf_num,leaf_page,record);
      return 0;
}


int db_delete(int table_id, int64_t key){
    buffer_t *header;
    pagenum_t leaf_num;
    char ret_val[120];
 
    if(fd_table[table_id] == -1) return 1;

    if(db_find_old(table_id,key,ret_val)!=0) return 1;

    header = buf_get(fd_table[table_id],0);
    leaf_num = find_leaf(fd_table[table_id],header->head_p.root_page_num,key);
    buf_put(header);
    delete_entry(fd_table[table_id],leaf_num,key);

    return 0;
}
