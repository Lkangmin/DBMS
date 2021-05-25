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
int order = DEFAULT_ORDER;

// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

/* Copyright and license notice to user at startup. 
 */
void license_notice( void ) {
    printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
            "http://www.amittai.com\n", Version);
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details "
            "type `show w'.\n"
            "This is free software, and you are welcome to redistribute it\n"
            "under certain conditions; type `show c' for details.\n\n");
}


/* Routine to print portion of GPL license to stdout.
 */
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
 */
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
 */
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
 */
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
}*/

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
pagenum_t find_leaf( pagenum_t root, int64_t key, bool verbose ) {
    int i = 0;
    page_t* c;
    if (root == 0) {
        if (verbose) 
            printf("Empty tree.\n");
        return root;
    }
    c = make_node();
    file_read_page(root,c);

    while (!c->is_leaf) {
        i = -1;
        while (i+1 < c->key_num) {
            if (key >= c->entry[i+1].key) i++;
            else break;
        }
        if(i == -1) root = c->left_most_num;
        else root = c->entry[i].page_offset;
        file_read_page(root,c);
    }
    free(c);
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
int get_left_index(page_t* parent_page, pagenum_t left) {
    int i;
    int left_index = 0;
    
    if(parent_page->left_most_num == left) return 0;
    for(i=0;i<parent_page->key_num;i++){
        if(parent_page->entry[i].page_offset == left){
            return i + 1;
        }
    }
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
void insert_into_leaf( pagenum_t leaf_num, Record * record ) {

    int i, insertion_point;
    page_t leaf_page;
    
    file_read_page(leaf_num,&leaf_page);
    insertion_point = 0;
    while (insertion_point < leaf_page.key_num && leaf_page.record[insertion_point].key < record->key)
        insertion_point++;

    for (i = leaf_page.key_num; i > insertion_point; i--) {
        leaf_page.record[i].key = leaf_page.record[i - 1].key;
        strcpy(leaf_page.record[i].value,leaf_page.record[i - 1].value);
    }
    leaf_page.record[insertion_point].key = record->key;
    strcpy(leaf_page.record[insertion_point].value,record->value);
    leaf_page.key_num++;
    file_write_page(leaf_num,&leaf_page);
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting(pagenum_t leaf_num, page_t * leaf_page, Record * record) {

    page_t* new_leaf;
    Record temp[33];
    int insertion_index, split, new_key, i, j;
    int64_t new_leaf_key;
    pagenum_t new_leaf_num;

    new_leaf = make_leaf();
    
    if (temp == NULL) {
        perror("Temporary array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    while (insertion_index < LEAF_ORDER - 1 && leaf_page->record[insertion_index].key < record->key)
        insertion_index++;

    for (i = 0, j = 0; i < leaf_page->key_num; i++, j++) {
        if (j == insertion_index) j++;
        memcpy(&temp[j],&leaf_page->record[i],sizeof(Record));
    }
    memcpy(&temp[insertion_index],record,sizeof(Record));

    leaf_page->key_num = 0;
    split = cut(LEAF_ORDER - 1);

    for (i = 0; i < split; i++) {
        memcpy(&leaf_page->record[i],&temp[i],sizeof(Record));
        leaf_page->key_num++;
    }

    for (i = split, j = 0; i < LEAF_ORDER; i++, j++) {
        memcpy(&new_leaf->record[j],&temp[i],sizeof(Record));
        new_leaf->key_num++;
    }

    new_leaf_num = file_alloc_page();
    new_leaf->right_sibling_page_num = leaf_page->right_sibling_page_num;
    leaf_page->right_sibling_page_num = new_leaf_num;
    new_leaf->parent_page_num = leaf_page->parent_page_num;
    new_leaf_key = new_leaf->record[0].key;
    
    file_write_page(leaf_num,leaf_page);
    file_write_page(new_leaf_num,new_leaf);

    insert_into_parent(leaf_num,leaf_page, new_leaf_key, new_leaf_num,new_leaf);
    free(new_leaf);
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(pagenum_t parent_num,page_t* parent_page,int left_index,int64_t key,pagenum_t right_num) {
    
    int i;
    for (i = parent_page->key_num; i > left_index; i--) {
        memcpy(&parent_page->entry[i],&parent_page->entry[i-1],sizeof(Entry));
    }
    parent_page->entry[left_index].key = key;
    parent_page->entry[left_index].page_offset = right_num;
    parent_page->key_num++;

    file_write_page(parent_num,parent_page);

}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(pagenum_t old_node_num ,page_t * old_node, int left_index, int64_t key, pagenum_t right_num) {

    int i, j, split;
    int64_t k_prime;
    page_t * new_node, * child;
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

    for (i = 0, j = 0; i < old_node->key_num; i++, j++) {
        if (j == left_index + 1) j++;
        temp[j].page_offset = old_node->entry[i].page_offset;
        temp[j].key = old_node->entry[i].key;
    }
    temp[left_index].page_offset = right_num;
    temp[left_index].key = key;
    
        /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(INTERNAL_ORDER);
    new_node = make_node();
    old_node->key_num = 0;
    for (i = 0; i < split-1; i++) {
        old_node->entry[i].page_offset =  temp[i].page_offset;
        old_node->entry[i].key = temp[i].key;
        old_node->key_num++;
    }
    old_node->entry[i-1].page_offset = temp[i].page_offset;
    k_prime = temp[split - 1].key;

    for (++i, j = 0; i < INTERNAL_ORDER; i++, j++) {
        new_node->entry[j].page_offset = temp[i].page_offset;
        new_node->entry[j].key = temp[i].key;
        new_node->key_num++;
    }

    new_node->left_most_num = temp[split-1].page_offset;
    new_node->parent_page_num = old_node->parent_page_num;

    new_node_num = file_alloc_page();
    child_num = new_node->left_most_num;
    file_write_page(old_node_num,old_node);
    file_write_page(new_node_num,new_node); 
    child=make_node();
    file_read_page(child_num,child);
    child->parent_page_num = new_node_num;
    file_write_page(child_num,child);

    for (i = 0; i < new_node->key_num; i++) {
        child_num = new_node->entry[i].page_offset;
        file_read_page(child_num,child);
        child->parent_page_num = new_node_num;
        file_write_page(child_num,child);
    }

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    insert_into_parent(old_node_num, old_node, k_prime, new_node_num,new_node);
    free(child);
    free(new_node);
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(pagenum_t left_num, page_t* left, int64_t key, pagenum_t right_num, page_t* right) {

    int left_index;

    /* Case: new root. */;
    if (left->parent_page_num == 0){
        insert_into_new_root(left_num,left, key, right_num,right);
        return;
     }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */
    
    page_t* parent_page = make_node();
    pagenum_t parent_num = left->parent_page_num;
    file_read_page(parent_num,parent_page);
    left_index = get_left_index(parent_page, left_num);

    /* Simple case: the new key fits into the node. 
     */

    if (parent_page->key_num < INTERNAL_ORDER - 1){
        insert_into_node(parent_num,parent_page, left_index, key, right_num);
        free(parent_page);
        return;
    }

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    insert_into_node_after_splitting(parent_num,parent_page, left_index, key, right_num);
    free(parent_page);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
void insert_into_new_root(pagenum_t left_num, page_t* left, int64_t key, pagenum_t right_num,page_t* right) {

    page_t* root = make_node();
    pagenum_t root_num = file_alloc_page();
    
    root->parent_page_num = 0;
    root->is_leaf = 0;
    root->key_num = 1;
    root->left_most_num = left_num;
    root->entry[0].key = key;
    root->entry[0].page_offset = right_num;

    left->parent_page_num = root_num;
    right->parent_page_num = root_num;
    file_read_page(0,(page_t*)head_page_p);
    head_page_p->root_page_num = root_num;

    file_write_page(0,(page_t*)head_page_p);
    file_write_page(left_num,left);
    file_write_page(right_num,right);
    file_write_page(root_num,root);
    free(root);
}



/* First insertion:db_insert
 * start a new tree.
 */
 pagenum_t start_new_tree(int key, char* value) {
    pagenum_t new_num = file_alloc_page();
    page_t* root = make_leaf();
    root->parent_page_num = 0;
    root->key_num++;
    root->right_sibling_page_num = 0;
    root->record[0].key = key;
    strcpy(root->record[0].value,value);
    file_write_page(new_num,root);
    free(root);
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
int get_neighbor_index( page_t* page, pagenum_t num ) {

    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = 0; i <= page->key_num; i++)
        if (page->entry[i].page_offset == num)
            return i-1;

    // Error state.
    exit(EXIT_FAILURE);
}


void remove_entry_from_node(pagenum_t page_num,page_t* n, int64_t key) {

    int i, num_pointers;

    // Remove the key and shift other keys accordingly.
    i = 0;
    if(n->is_leaf){
        while(n->record[i].key != key)
            i++;
        for(++i;i<n->key_num;++i){
            memcpy(&n->record[i-1],&n->record[i],sizeof(Record));
        }
        n->key_num--;
    }
    else{
        while(n->entry[i].key != key)
            i++;
        for(++i;i<n->key_num;++i)
            memcpy(&n->entry[i-1],&n->entry[i],sizeof(Entry));
        n->key_num--;
    }
}


void adjust_root(pagenum_t root_num,page_t * root) {

    page_t new_root;
    pagenum_t new_num;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (root->key_num > 0){
        file_write_page(root_num,root);
        return ;
    }

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.
    file_read_page(0,(page_t*)head_page_p);
    if (!root->is_leaf) {
        new_num = root->left_most_num;
        file_read_page(new_num,&new_root);
        new_root.parent_page_num = 0;
        file_write_page(new_num,&new_root);
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.
    else{
        new_num = 0;
        head_page_p->root_page_num = new_num;
        file_write_page(0,(page_t*)head_page_p);
        file_free_page(root_num);
    }
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 *//*
node * coalesce_nodes(pagenum_t page_num,page_t* page,pagenum_t parent_num,page_t parent_page,neighbor_num, neighbor_index, k_prime) {

    int i, j, neighbor_insertion_index, n_end;
    node * tmp;

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     

    neighbor_insertion_index = neighbor->num_keys;

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     

    if (!n->is_leaf) {

        /* Append k_prime.
         

        neighbor->keys[neighbor_insertion_index] = k_prime;
        neighbor->num_keys++;


        n_end = n->num_keys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
            n->num_keys--;
        }

        /* The number of pointers is always
         * one more than the number of keys.
         

        neighbor->pointers[i] = n->pointers[j];

        /* All children must now point up to the same parent.
         

        for (i = 0; i < neighbor->num_keys + 1; i++) {
            tmp = (node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     

    else {
        for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
        }
        neighbor->pointers[order - 1] = n->pointers[order - 1];
    }

    root = delete_entry(root, n->parent, k_prime, n);
    free(n->keys);
    free(n->pointers);
    free(n); 
    return root;
}
*/

/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 
node * redistribute_nodes(node * root, node * n, node * neighbor, int neighbor_index, 
        int k_prime_index, int k_prime) {  

    int i;
    node * tmp;

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     

    if (neighbor_index != -1) {
        if (!n->is_leaf)
            n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
        for (i = n->num_keys; i > 0; i--) {
            n->keys[i] = n->keys[i - 1];
            n->pointers[i] = n->pointers[i - 1];
        }
        if (!n->is_leaf) {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (node *)n->pointers[0];
            tmp->parent = n;
            neighbor->pointers[neighbor->num_keys] = NULL;
            n->keys[0] = k_prime;
            n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
        }
        else {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;
            n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            n->parent->keys[k_prime_index] = n->keys[0];
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     

    else {  
        if (n->is_leaf) {
            n->keys[n->num_keys] = neighbor->keys[0];
            n->pointers[n->num_keys] = neighbor->pointers[0];
            n->parent->keys[k_prime_index] = neighbor->keys[1];
        }
        else {
            n->keys[n->num_keys] = k_prime;
            n->pointers[n->num_keys + 1] = neighbor->pointers[0];
            tmp = (node *)n->pointers[n->num_keys + 1];
            tmp->parent = n;
            n->parent->keys[k_prime_index] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->num_keys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!n->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     

    n->num_keys++;
    neighbor->num_keys--;

    return root;
}
*/

/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
void delete_entry(pagenum_t page_num, int64_t key) {

    page_t* page = make_node();
    int neighbor_index;
    int k_prime_index;
    int64_t k_prime;


    // Remove key and pointer from node.
    
    file_read_page(0,(page_t*)head_page_p);
    file_read_page(page_num,page);
    remove_entry_from_node(page_num,page,key);

    /* Case:  deletion from the root. 
     */

    if (page_num == head_page_p->root_page_num){
        adjust_root(page_num,page);
        free(page);
        return;
    }


    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */

    

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (page->key_num > 0){
        free(page);
        return ;
    }

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
    pagenum_t parent_num = page->parent_page_num;
    page_t* parent_page = make_node();
    page_t* neighbor_page = make_node();
    file_read_page(parent_num,parent_page);
    neighbor_index = get_neighbor_index( parent_page, page_num );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;

    k_prime =parent_page->entry[k_prime_index].key;
    pagenum_t neighbor_num = neighbor_index== -1 ? parent_page->left_most_num : parent_page->entry[neighbor_index].page_offset;
    file_read_page(neighbor_num,neighbor_page);

    /* Coalescence. */

    if (neighbor_page->key_num < INTERNAL_ORDER-1){
        // coalesce_nodes(page_num,page,parent_num,parent_page,neighbor_num, neighbor_index, k_prime);
        free(parent_page);
        free(page);
        free(neighbor_page);
    }

    /* Redistribution. */

    else{
        //redistribute_nodes(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
        free(parent_page);
        free(page);
        free(neighbor_page);
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
}


int db_find(int64_t key, char * ret_val){
    pagenum_t page_num;
    page_t* leaf_page = make_node();
    int i;

    if(fd==-1) exit(1);

    file_read_page(0,(page_t*)head_page_p);
    page_num = find_leaf(head_page_p->root_page_num,key,0);
    if(page_num == 0) {
        free(leaf_page);
        return 1;
    }
    file_read_page(page_num,leaf_page);
    for(i=0;i<leaf_page->key_num;i++){
        if(leaf_page->record[i].key == key){
            strcpy(ret_val,leaf_page->record[i].value);
            free(leaf_page);
            return 0;
        }
    }
    free(leaf_page);
    return 1;
}


int db_insert(int64_t key, char * value){
    Record* record;
    pagenum_t page_num,n_root,leaf_num;
    page_t* leaf_page;
    char ret_val[120];

    if(fd==-1) exit(1);

    // case 1
    if(db_find(key,ret_val)==0) return 1;

    //case 2    
    file_read_page(0,(page_t*)head_page_p);
    if(head_page_p->root_page_num == 0){
        n_root = start_new_tree(key,value);
        head_page_p->root_page_num = n_root;
        file_write_page(0,(page_t*)head_page_p);
        return 0;
    }

      record = make_record(key,value);
      leaf_page = make_leaf();
      file_read_page(0,(page_t*)head_page_p);
      leaf_num = find_leaf(head_page_p->root_page_num,key,0);
      file_read_page(leaf_num,leaf_page);
      //case 3
      if(leaf_page->key_num < LEAF_ORDER -1 ){
        insert_into_leaf(leaf_num,record);
        free(record);
        free(leaf_page);
        return 0;
      }
      
      //case4
      insert_into_leaf_after_splitting(leaf_num,leaf_page,record);
      free(record);
      free(leaf_page);
      return 0;
}


int db_delete(int64_t key){
    page_t leaf_page;
    pagenum_t leaf_num;

    if(fd==-1) exit(1);

    if(db_find(key,NULL)!=0) return 1;

    file_read_page(0,(page_t*)head_page_p);
    leaf_num = find_leaf(head_page_p->root_page_num,key,0);
    delete_entry(leaf_num,key);

    return 0;
}