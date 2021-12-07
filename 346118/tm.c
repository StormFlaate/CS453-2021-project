/**
 * @file   tm.c
 * @author [...]
 *
 * @section LICENSE
 *
 * [...]
 *
 * @section DESCRIPTION
 *
 * Implementation of your own transaction manager.
 * You can completely rewrite this file (and create more files) as you wish.
 * Only the interface (i.e. exported symbols and semantic) must be preserved.
**/

// Requested features
#define _GNU_SOURCE
#define _POSIX_C_SOURCE   200809L
#ifdef __STDC_NO_ATOMICS__
    #error Current C11 compiler does not support atomic operations
#endif

// External headers
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Internal headers
#include "../include/tm.h"
#include "macros.h"

/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
**/

// NOTES of potential problems:
// 1. in read_word() function, How to distinguish if a word is in the current access set or not (given the current solution that does not include access set but only a mark if it is accessed or not
// 2. Access set in write word
// 3. How are we suppose to handle when we update which value is valid or not -> i.e. valid_a or valid_b




typedef struct wordNode_instance_t* wordNode_t;

// Linked list of words!
struct wordNode_instance_t
{
    // __CONTROL__
    // Which copy is "valid" from the previous epoch!  true --> A  |  false --> B
    // Is initialized to NULL, since the first time not a or b are valid
    bool valid_a; 
    // true if at least one other transactions is in the access set, false otherwise
    // access set: a set of transactions that are currently accessing this word!
    bool accessed;

    // Mark target for deregistration, if true it will be removed when last transaction is done
    bool free;

    // wheter the word is currently being written to  
    bool writing;
    // If is non-free-able allocated segment
    bool non_free_able;

    // Size of align
    size_t align_size;

    // COPY!
    void* copy_A;
    void* copy_B;

    // wordNode_t is a pointer to another wordNode_instance_t
    wordNode_t next_word; 
    wordNode_t prev_word;
};

struct region {
    //struct share_lock_t lock;
    void* start; // start of the shared memory region

};


/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
**/
shared_t tm_create(size_t unused(size), size_t unused(align)) {
    // TODO: tm_create(size_t, size_t)
    
    // Check if size is not 0, because then it acn still pass the test in the second else if
    if (size-align < 0)
        return invalid_shared;
    //  Checks if the size is a positive multiple of align
    else if(size%align != 0)
        return invalid_shared;
    // Checks if the size is a power of 2
    else if(align%2 != 0)
        return invalid_shared;
    
    size_t numb_words = (size_t) size/align;

    wordNode_t init = calloc(1, sizeof(struct wordNode_instance_t));
    init -> copy_A = calloc(1, align);
    init -> copy_B = calloc(1, align);
    
    
    if(init == NULL) { printf("Could not allocate memory!\n"); return invalid_shared; }
    if((init -> copy_A) == NULL) { printf("Could not allocate memory!\n"); return invalid_shared; }
    if((init -> copy_B) == NULL)  {printf("Could not allocate memory!\n"); return invalid_shared; }

    init -> accessed = false;
    init -> free = false;
    init -> valid_a = true; // Setting default to that copy_A is valid
    init -> writing = false;
    init -> next_word = NULL; // Init to NULL, so it does not point to random place
    init -> non_free_able = true; // This word is part of the non-free-able allocated segment
    init -> align_size = align; // This is so we have the align size with us all the time
    init -> prev_word = NULL; // Since this is head we set the previous word to NULL



    wordNode_t tmp_head = init;
    wordNode_t tmp_prev = tmp_head;
    // If there size is a positive multiple of align
    // that is bigger than 1, we will need to create more word nodes
    for(size_t i = 0; i <(numb_words-1); i++)
    {
        wordNode_t new_word = calloc(1, sizeof(struct wordNode_instance_t));
        if(new_word == NULL) { printf("Could not allocate memory!"); return invalid_shared; }
        

        tmp_head ->next_word = new_word;
        tmp_prev = tmp_head;
        tmp_head = new_word;

        tmp_head -> copy_A = calloc(1, align); 
        tmp_head -> copy_B = calloc(1, align);
        
        //Checking if allocation of memory was successfull
        if((tmp_head -> copy_A) == NULL) { printf("Could not allocate memory!\n"); return invalid_shared; }
        if((tmp_head -> copy_B) == NULL) { printf("Could not allocate memory!\n"); return invalid_shared; }

        tmp_head -> accessed = false;
        tmp_head -> free = false;
        tmp_head -> valid_a = true; // Setting default to that copy_A is valid
        tmp_head -> writing = false;
        tmp_head -> next_word = NULL; // Init to NULL, so it does not point to random place
        tmp_head -> non_free_able = true; // This word is part of the non-free-able allocated segment
        tmp_head -> align_size = align; // This is so we have the align size with us all the time
        tmp_head -> prev_word = tmp_prev;

        

    }
    return init;
}

/** Destroy (i.e. clean-up + free) a given shared memory region.
 * @param shared Shared memory region to destroy, with no running transaction
**/
void tm_destroy(shared_t unused(shared)) {
    // TODO: tm_destroy(shared_t)
    
    wordNode_t head = shared;
    wordNode_t tmp;
    
    // Will need to iterate over all next words, to free up their memory
    while(head != NULL)
    {
        tmp = head -> next_word;
        free(head ->copy_A);
        free(head ->copy_B);
        free(head);
        head = tmp;
    }
}

/** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
 * @param shared Shared memory region to query
 * @return Start address of the first allocated segment
**/
void* tm_start(shared_t unused(shared)) {
    // TODO: tm_start(shared_t)

    // Possible that I shuold implement a pointer back to the first node in the shared memory
    // so that every node points back to the main node.
    // Keep this in mind, and take a executive decision on this later
    return ((wordNode_t) shared);
}

/** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
 * @param shared Shared memory region to query
 * @return First allocated segment size
**/
size_t tm_size(shared_t unused(shared)) {
    size_t size = 0;

    wordNode_t head = shared;
    wordNode_t tmp;

    // If the non-free-able mark is set to false, we no longer have the first allocated segment of the shared memory
    // If the head is equal to NULL, we have no more words on the current word node linked list
    while(head != NULL && (head->non_free_able) == true)
    {
        tmp = head -> next_word;
        size += tm_align(head);
        head = tmp;
    }

    return size;
}

/** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
 * @param shared Shared memory region to query
 * @return Alignment used globally
**/
size_t tm_align(shared_t unused(shared)) {
    return  ((wordNode_t) shared) ->align_size;
}

/** [thread-safe] Begin a new transaction on the given shared memory region.
 * @param shared Shared memory region to start a transaction on
 * @param is_ro  Whether the transaction is read-only
 * @return Opaque transaction ID, 'invalid_tx' on failure
**/
tx_t tm_begin(shared_t unused(shared), bool unused(is_ro)) {
    // TODO: tm_begin(shared_t)
    return invalid_tx;
}

/** [thread-safe] End the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to end
 * @return Whether the whole transaction committed
**/
bool tm_end(shared_t unused(shared), tx_t unused(tx)) {
    // TODO: tm_end(shared_t, tx_t)
    return false;
}


void read_a_or_b(wordNode_t word_ptr, void* unused(target), size_t offset_mult)
{   
    if(word_ptr ->valid_a)
    {
        
        // Copies bytes to destination of target + (offset_mult * word_ptr)
        // Copy align number of bytes into the target (destination) + offset
        size_t offset = offset_mult*word_ptr->align_size;
        memcpy(target+offset, word_ptr->copy_A, word_ptr->align_size);
    }
    else
    {
        // Copies bytes to destination of target + (offset_mult * word_ptr)
        // Copy align number of bytes into the target (destination) + offset
        size_t offset = offset_mult*word_ptr->align_size;
        memcpy(target+offset, word_ptr->copy_B, word_ptr->align_size);
    }
}


bool read_word(wordNode_t word_ptr, void* unused(target), size_t offset_mult)
{   
    
    // Transaction is read-only, since no process is currently writing to this very word
    if(!word_ptr ->writing)
    {
        read_a_or_b(word_ptr, target, offset_mult);
        return true;
    }
    else
    {   
        // Someone is writing
        if(word_ptr ->writing)
        {   
            // Is not currently beeing accessed, i.e. the person is done writing, and have updated the writable copy
            if(!word_ptr->accessed)
            {   
                // Will read the writable copy into target, this will be the opposite of valid_a
                if(word_ptr ->valid_a) 
                    target = word_ptr ->copy_B;

                else
                    target = word_ptr ->copy_A;

                // The transaction can continue
                return true; 
            }
            // The person writing is not done writing, can't take the chance on actually figuring out
            // if we should read from the readable copy or the writeable copy.
            else
            {
                return false;
            }
        }
        // The person is currently done with writing, it is therefore now a read-only!
        else
        {
            read_a_or_b(word_ptr, target, offset_mult);
            // To be sure we just check that currently accessed is set to false!
            if(word_ptr -> accessed) 
                word_ptr -> accessed = false;
            return true;
        }
    }
}


/** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in the shared region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in a private region)
 * @return Whether the whole transaction can continue
**/
bool tm_read(shared_t unused(shared), tx_t unused(tx), void const* unused(source), size_t unused(size), void* unused(target)) {
    
    size_t align_chunks = size/tm_align(shared); // if size = 64 bytes and align = 8 bytes, we will count 8 align chunks


    wordNode_t head = (wordNode_t) source;
    wordNode_t tmp;
    size_t counter = 0;
    // Is expected to exit this by using counter == align_chunks
    // If we hit head before counter == align, we don't have enough space to read from
    while(head != NULL)
    {
        tmp = head -> next_word;
        // Target + counter gives us adress chunk for private memory!
        
        bool result = read_word(head, target, counter); 

        // If one word read fails, we will need to abort the whole transaction!
        if (!result)
        {
            return false;
        }

        head = tmp;
        counter++;

        // Check if we have read all the words, then transaction was sucessfull!
        if (counter == align_chunks)
            return true;
    }

    // If we get down here there was not enough words in our shared memory to satisfy the tm_read, we therefore abort!
    return false;
}



void write_a_or_b(const void* unused(source), wordNode_t word_ptr, size_t offset_mult)
{

    

    // If copy_A is currently valid, that means we have to write into B
    // This way people can still read from copy_A until we are done with copy_B
    if(word_ptr ->valid_a)
    {

        // Copies bytes from source + offset, since we are currently on the k-th time copying from source
        // Copies this into destination of copy_B, this is passed on from a for loop in the other function, so we don't need to keep track of this offset
        size_t offset = offset_mult*(word_ptr->align_size);
        memcpy(word_ptr->copy_B, source+offset, word_ptr->align_size);

        // Updates valid_a variable when we are done writing to the shared area, we now know it is updated
        word_ptr ->valid_a = false;
    }
    // If copy_B is currently valid, that means we have to write into A
    // This way people can still read form Ccopy_B until we are done with copy_A
    else
    {

        // Copies bytes from source + offset, since we are currently on the k-th time copying from source
        // Copies this into destination of copy_B, this is passed on from a for loop in the other function, so we don't need to keep track of this offset
        size_t offset = offset_mult*word_ptr->align_size;
        memcpy(word_ptr->copy_A, source+offset, word_ptr->align_size);
        // Updates valid_a variable when we are done writing to the shared area, we now know it is updated
        word_ptr ->valid_a = true;
    }
}


bool write_word(const void* unused(source), wordNode_t word_ptr, size_t offset_mult)
{
    // If a word has been written in the current epoch, i.e. if someone is currently writing
    if(word_ptr ->writing)
    {   
        // For later we can say that if anyone is current writing we just return false here,
        // then we can optimize this later
        // How to distinguish if a word is in the current access set or not given our current solution
        return false;
        
    }
    else
    {
        if(word_ptr ->accessed)
        {
            return false;
        }
        else
        {
            write_a_or_b(source, word_ptr, offset_mult);
            if(word_ptr -> accessed) 
                word_ptr -> accessed = true;
            word_ptr ->writing = true;
            return true;
        }
    }
}

/** [thread-safe] Write operation in the given transaction, source in a private region and target in the shared region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in a private region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in the shared region)
 * @return Whether the whole transaction can continue
**/
bool tm_write(shared_t unused(shared), tx_t unused(tx), void const* unused(source), size_t unused(size), void* unused(target)) {
    // TODO: tm_write(shared_t, tx_t, void const*, size_t, void*)
    size_t align_chunks = size/tm_align(shared); // if size = 64 bytes and align = 8 bytes, we will count 8 align chunks

    wordNode_t head = (wordNode_t) target;
    wordNode_t tmp;
    size_t counter = 0;
    // Is expected to exit this by using counter == align_chunks
    // If we hit head before counter == align, we don't have enough space to read from
    while(head != NULL)
    {
        tmp = head -> next_word;
        // Target + counter*align gives us adress chunk for private memory!
        
        
        bool result = write_word(source, head, counter);
        // If one word write fails, we will need to abort the whole transaction!
        if (!result)
        {
            return false;
        }

        head = tmp;
        counter++;

        // Check if we have written all the words, then transaction was sucessfull!
        if (counter == align_chunks)
            return true;
    }

    // If we get down here there was not enough words in our shared memory to satisfy the tm_read, we therefore abort!
    return false;
}



/** [thread-safe] Memory allocation in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
 * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
 * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
**/
alloc_t tm_alloc(shared_t unused(shared), tx_t unused(tx), size_t unused(size), void** unused(target)) {
    // TODO: tm_alloc(shared_t, tx_t, size_t, void**)
    // Check if size is not 0, because then it acn still pass the test in the second else if
    size_t align = tm_align(shared);
    
    if (size-align < 0)
        return abort_alloc;
    //  Checks if the size is a positive multiple of align
    else if(size%align != 0)
        return abort_alloc;
    // Checks if the size is a power of 2
    else if(align%2 != 0)
        return abort_alloc;
    
    size_t numb_words = (size_t) size/align;

    wordNode_t init = calloc(1, sizeof(struct wordNode_instance_t));
    init -> copy_A = calloc(1, align);
    init -> copy_B = calloc(1, align);
    
    
    if(init == NULL) { printf("Could not allocate memory!\n"); return abort_alloc; }
    if((init -> copy_A) == NULL) { printf("Could not allocate memory!\n"); return abort_alloc; }
    if((init -> copy_B) == NULL) { printf("Could not allocate memory!\n"); return abort_alloc; }

    init -> accessed = false;
    init -> free = false;
    init -> valid_a = true; // Setting default to that copy_A is valid
    init -> writing = false;
    init -> next_word = NULL; // Init to NULL, so it does not point to random place
    init -> non_free_able = false; // This word is in the set of the allocated segments
    init -> align_size = align;

    wordNode_t tmp_head = init;
    // If there size is a positive multiple of align
    // that is bigger than 1, we will need to create more word nodes
    for(size_t i = 0; i <(numb_words-1); i++)
    {
        wordNode_t new_word = calloc(1, sizeof(struct wordNode_instance_t));
        if(new_word == NULL) { printf("Could not allocate memory!\n"); return abort_alloc; }
        

        tmp_head ->next_word = new_word;
        tmp_head = new_word;

        tmp_head -> copy_A = calloc(1, align); 
        tmp_head -> copy_B = calloc(1, align);
        
        //Checking if allocation of memory was successfull
        if((tmp_head -> copy_A) == NULL) { printf("Could not allocate memory!");return abort_alloc; }
        if((tmp_head -> copy_B) == NULL) { printf("Could not allocate memory!");return abort_alloc; }

        tmp_head -> accessed = false;
        tmp_head -> free = false;
        tmp_head -> valid_a = true; // Setting default to that copy_A is valid
        tmp_head -> writing = false;
        tmp_head -> next_word = NULL; // Init to NULL, so it does not point to random place
        tmp_head -> non_free_able = false; // This word is in the set of the allocated segments
        tmp_head -> align_size = align;

    }

    *target = init;

    // Getting the last node of the shared memory and attaching this address to that
    wordNode_t head = shared;
    wordNode_t tmp;
    
    // Iterating over all nodes in the shared memory
    // When accessing a nde where the next_word is NULL, we have the last node of the linked list!
    while((head ->next_word) != NULL)
    {
        tmp = head -> next_word;
        head = tmp;
    }

    // We now know that the head is the last node in the current linked list!
    // And we can therefore attach our newly created segment to this shared memory!
    head -> next_word = init;

    return success_alloc;
    
}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
**/
bool tm_free(shared_t unused(shared), tx_t unused(tx), void* unused(target)) {
    // TODO: tm_free(shared_t, tx_t, void*)
    
    ((wordNode_t) target ) -> free = true;
    return true;
}


int main()
{

    printf("Now we are running.....\n");
    
    // Input number of nodes:
    size_t c = 5;

    shared_t head = tm_create(4*c,4);

    size_t align_size = tm_align(head);
    size_t full_size = tm_size(head);


    // Write from private source into shared memory
    void* source_1 = malloc(sizeof(int)*3);
    
    
    *((int*)source_1) = 1;
    *((int*)(source_1)+1) = 10;
    *((int*)(source_1)+2) = 100;
    
    // printf("%d\n",*(int*)(source_1) );
    // printf("%d\n", *((int*)(source_1)+1) );
    // printf("%d\n", *((int*)(source_1)+2) );

    
    tm_write(head, 222, source_1, 4*3, head);

    ((wordNode_t) head)->accessed = false;
    ((wordNode_t) head)->writing = false;
    ((wordNode_t) head)->next_word->accessed = false;
    ((wordNode_t) head)->next_word->writing = false;
    ((wordNode_t) head)->next_word->next_word->accessed = false;
    ((wordNode_t) head)->next_word->next_word->writing = false;
    
    void *private_memory = malloc(sizeof(int)*3);
    // Init both private memory spots to 999, so we can see if they have updated!

    *((int*)private_memory) = 999;
    *((int*)(private_memory)+1) = 999;
    *((int*)(private_memory)+2) = 999;
    printf("\n----------------------\n\n");
    printf("%d | %d | %d - initial values on private memory, should change\n\n", *(int*)private_memory, *( ((int*)(private_memory)) +1), *( ( (int*)(private_memory) ) + 2));
    tm_read(head, 123, head, 4*3, private_memory);
    printf("%d | %d | %d - if these values appear below, program works\n", 1, 10 ,100);
    printf("%d | %d | %d\n", *(int*)private_memory, *( ( (int*)(private_memory) ) + 1), *( ( (int*)(private_memory) ) + 2));
    
    

    
    printf("Code executed with success!\n");


    return 0;
}


