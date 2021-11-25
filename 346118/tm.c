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

// Internal headers
#include "../include/tm.h"
#include "macros.h"

/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
**/


typedef struct wordList_t* wordList_t;
typedef struct word_t word_t;

struct word_t
{
    bool valid_a; // Which copy is "valid" from the previous epoch, true for A false for B
    bool currently_accessed; // true if no other transaction is currently accessing it, false otherwise
    bool writing; // wheter the word has been written in the current epoch
    void* copy_A;
    void* copy_B;
};

// Lined list of word's, giving us a memory list!
struct wordList_t
{
    word_t* word;
    wordList_t* next_word;
};

// @param word_ptr is a pointer to a word instance that will be added to this "Node"


shared_t tm_create(size_t unused(size), size_t unused(align)) {
    // TODO: tm_create(size_t, size_t)
    //  Checks if the size is a positive multiple of align
    if(size%align != 0)
        return invalid_shared;
    // Checks if the size is a power of 2
    else if(align%2 != 0)
        return invalid_shared;
    
    size_t numb_words = (size_t) size/align;

    // First word
    wordList_t inst = calloc(1, sizeof(struct wordList_t));
    (*inst) ->word = calloc(1, sizeof(struct word_t));
    (*inst) ->next_word = NULL;
    ((*inst) ->word) ->copy_A = calloc(1, align);
    ((*inst) ->word) ->copy_B = calloc(1, align);
    ((*inst) ->word) ->currently_accessed = false;
    ((*inst) ->word) ->valid_a = NULL;
    ((*inst) ->word) ->writing = false;

    wordList_t* current_head = inst;

    for(int i = 0; i<numb_words-1; i++)
    {
        (*current_head) ->next_word = calloc(1, sizeof(struct wordList_t));
        current_head = ((*current_head) ->next_word);

        (*current_head) ->word = calloc(1, sizeof(struct word_t));
        (*current_head) ->next_word = NULL;
        ((*current_head) ->word) ->copy_A = calloc(1, align);
        ((*current_head) ->word) ->copy_B = calloc(1, align);
        ((*current_head) ->word) ->currently_accessed = false;
        ((*current_head) ->word) ->valid_a = NULL;
        ((*current_head) ->word) ->writing = false;
    }

    return inst;

}

/** Destroy (i.e. clean-up + free) a given shared memory region.
 * @param shared Shared memory region to destroy, with no running transaction
**/
void tm_destroy(shared_t unused(shared)) {
    // TODO: tm_destroy(shared_t)
    
    wordList_t* tmp;
    wordList_t* head = (wordList_t*) shared;
    
    // Will need to iterate over all next words, to free up their memory
    while(head != NULL)
    {
        // Pointer to temporary and pointer to current head
        tmp = head;
        head = (*head) -> next_word;
        free(((*tmp) -> word) ->copy_A);
        free(((*tmp) -> word) ->copy_B);
        free((*tmp) -> word);
        free(tmp);    
    }
}

/** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
 * @param shared Shared memory region to query
 * @return Start address of the first allocated segment
**/
void* tm_start(shared_t unused(shared)) {
    // TODO: tm_start(shared_t)
    return (*((wordList_t*) shared)) -> word;
}

/** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
 * @param shared Shared memory region to query
 * @return First allocated segment size
**/
size_t tm_size(shared_t unused(shared)) {
    return sizeof((*((wordList_t*) shared)) -> word);
}

/** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
 * @param shared Shared memory region to query
 * @return Alignment used globally
**/
size_t tm_align(shared_t unused(shared)) {
    return sizeof((*((wordList_t*)shared))->word->copy_A);
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


void read_a_or_b(word_t* word_ptr, void* unused(target))
{
    if(word_ptr ->valid_a)
    {
        // Point the target pointer to the adress of copy_A
        target = word_ptr ->copy_A;
    }
    else
    {
        target = word_ptr ->copy_B;
    }
}


bool read_word(word_t* word_ptr, void* unused(target))
{   
    // Transaction is read-only, since no process is currently writing to this very word
    if(!word_ptr ->writing)
    {
        read_a_or_b(word_ptr, target);
        return true;
    }
    else
    {   
        // Someone is writing
        if(word_ptr ->writing)
        {   
            // Is not currently beeing accessed, i.e. the person is done writing, and have updated the writable copy
            if(!word_ptr->currently_accessed)
            {   
                // Will read the writable copy into target, this will be the opposite of valid_a
                if(word_ptr ->valid_a) 
                    target = word_ptr ->copy_B;

                else
                    target = word_ptr ->copy_A;

                // The transaction can continue
                return true; 
            }
            // The person writing is not done writing, can't take the channce on actually figuring out
            // if we should read from the readable copy or the writeable copy.
            else
            {
                return false;
            }
        }
        // The person is currently done with writing, it is therefore now a read-only!
        else
        {
            read_a_or_b(word_ptr, target);
            // To be sure we just check that currently accessed is set to false!
            if(word_ptr -> currently_accessed) 
                word_ptr -> currently_accessed = false;
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
    // TODO: tm_read(shared_t, tx_t, void const*, size_t, void*)
    wordList_t* memory_ptr = (wordList_t*) shared;
    int iterations = size / tm_align(memory_ptr);
    word_t* tmp_ptr = (word_t*) source;

    // source: 0x0000
    // size:   0x030
    // align:  0x005
    // for(int i = 0; i<size//align; i++)
    //      address = source + i*align
    // output: 0x000 -> 0x005 -> 0x010 -> 0x015 -> 0x020 -> 0x025 | Total 30//5 = 6 different memory places

    //for(int i = 0; i<iterations; i++)
    //{
        //read_word(tmp_ptr+1);
    //}


    return false;
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
    
    return 1;
    
}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
**/
bool tm_free(shared_t unused(shared), tx_t unused(tx), void* unused(target)) {
    // TODO: tm_free(shared_t, tx_t, void*)
    return false;
}
