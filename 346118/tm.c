// /**
//  * @file   tm.c
//  * @author [...]
//  *
//  * @section LICENSE
//  *
//  * [...]
//  *
//  * @section DESCRIPTION
//  *
//  * Implementation of your own transaction manager.
//  * You can completely rewrite this file (and create more files) as you wish.
//  * Only the interface (i.e. exported symbols and semantic) must be preserved.
// **/

// // Requested features
// #define _GNU_SOURCE
// #define _POSIX_C_SOURCE   200809L
// #ifdef __STDC_NO_ATOMICS__
//     #error Current C11 compiler does not support atomic operations
// #endif

// // External headers
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include <string.h>

// // Internal headers
// #include "../include/tm.h"
// #include "macros.h"

// /** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
//  * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
//  * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
//  * @return Opaque shared memory region handle, 'invalid_shared' on failure
// **/

// // NOTES of potential problems:
// // 1. in read_word() function, How to distinguish if a word is in the current access set or not (given the current solution that does not include access set but only a mark if it is accessed or not
// // 2. Access set in write word
// // 3. How are we suppose to handle when we update which value is valid or not -> i.e. valid_a or valid_b


// #include "shared-lock.h"

// static const tx_t read_only_tx  = UINTPTR_MAX - 10;
// static const tx_t read_write_tx = UINTPTR_MAX - 11;


// typedef struct wordNode_instance_t* wordNode_t;

// // Linked list of words!
// struct wordNode_instance_t
// {
//     // __CONTROL__
//     // Which copy is "valid" from the previous epoch!  true --> A  |  false --> B
//     // Is initialized to NULL, since the first time not a or b are valid
//     bool valid_a; 
//     // true if at least one other transactions is in the access set, false otherwise
//     // access set: a set of transactions that are currently accessing this word!
//     bool accessed;

//     // Mark target for deregistration, if true it will be removed when last transaction is done
//     bool free;

//     // wheter the word is currently being written to  
//     bool writing;

//     // COPY!
//     void* copy_A;
//     void* copy_B;

//     // wordNode_t is a pointer to another wordNode_instance_t
//     wordNode_t next_word; 
//     wordNode_t prev_word;
// };

// struct region {
//     //struct share_lock_t lock;
//     void* start; // start of the shared memory region
    
//     size_t align; // Size of word in the shared memory region (bytes)
//     size_t size; // Size of non-deallocable memory segment (bytes)
//     wordNode_t allocs;

//     /* LOCK */
//     struct shared_lock_t lock;
//     /* LOCK */
// };



// shared_t tm_create(size_t size, size_t align) {

//     // Check if size is not 0, because then it acn still pass the test in the second else if
//     if (size-align < 0)
//         return invalid_shared;
//     //  Checks if the size is a positive multiple of align
//     else if(size%align != 0)
//         return invalid_shared;
//     // Checks if the size is a power of 2
//     else if(align%2 != 0)
//         return invalid_shared;
    
//     struct region* region = (struct region*) calloc(1, sizeof(struct region));
//     if(region == NULL) { printf("Could not allocate memory!\n"); return invalid_shared; }

//     if(posix_memalign(&(region -> start), align, size) != 0){
//         free(region);
//         return invalid_shared;
//     }


//     /* LOCK */
//     if (!shared_lock_init(&(region->lock))) {
//         free(region->start);
//         free(region);
//         return invalid_shared;
//     }
//     /* LOCK */



//     memset(region -> start, 0, size);
//     region -> align = align;
//     region -> size = size;
//     region -> allocs = NULL; // No initial value for memory segments
    
//     return region;
// }


// /** Destroy (i.e. clean-up + free) a given shared memory region.
//  * @param shared Shared memory region to destroy, with no running transaction
// **/
// void tm_destroy(shared_t shared) {
//     // TODO: tm_destroy(shared_t)


//     struct region* region = (struct region*) shared;
    
//     // Defining head as the first allocated segment in the region
//     wordNode_t head = region -> allocs;
//     wordNode_t tmp;

//     // Iterating over all the nodes in the lined list
//     while (head != NULL) { 
//         tmp = head -> next_word;
//         free(head -> copy_A);
//         free(head -> copy_B);
//         free(head);
//         head = tmp;
//     }
//     free(region -> start);

//     /* LOCK */
//     shared_lock_cleanup(&(region->lock));
//     /* LOCK */

//     free(region);
// }


// /** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
//  * @param shared Shared memory region to query
//  * @return Start address of the first allocated segment
// **/
// void* tm_start(shared_t shared) {
//     return ((struct region*) shared)->start;
// }

// /** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
//  * @param shared Shared memory region to query
//  * @return First allocated segment size
// **/
// size_t tm_size(shared_t shared) {
//     return ((struct region*) shared)->size;
// }

// /** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
//  * @param shared Shared memory region to query
//  * @return Alignment used globally
// **/
// size_t tm_align(shared_t shared) {
//     return ((struct region*) shared)->align;
// }

// tx_t tm_begin(shared_t shared, bool is_ro) {
//     // We let read-only transactions run in parallel by acquiring a shared
//     // access. On the other hand, read-write transactions acquire an exclusive
//     // access. At any point in time, the lock can be shared between any number
//     // of read-only transactions or held by a single read-write transaction.
//     if (is_ro) {
//         // Note: "unlikely" is a macro that helps branch prediction.
//         // It tells the compiler (GCC) that the condition is unlikely to be true
//         // and to optimize the code with this additional knowledge.
//         // It of course penalizes executions in which the condition turns up to
//         // be true.
//         if (unlikely(!shared_lock_acquire_shared(&(((struct region*) shared)->lock))))
//             return invalid_tx;
//         return read_only_tx;
//     } else {
//         if (unlikely(!shared_lock_acquire(&(((struct region*) shared)->lock))))
//             return invalid_tx;
//         return read_write_tx;
//     }
// }

// bool tm_end(shared_t shared, tx_t tx) {
//     if (tx == read_only_tx) {
//         shared_lock_release_shared(&(((struct region*) shared)->lock));
//     } else {
//         shared_lock_release(&(((struct region*) shared)->lock));
//     }
//     return true;
// }





// void read_a_or_b(wordNode_t word_ptr, void* unused(target), size_t offset_mult, size_t align)
// {   
//     if(word_ptr ->valid_a)
//     {
        
//         // Copies bytes to destination of target + offset
//         // Copy align number of bytes into the target (destination) + offset
//         size_t offset = offset_mult*align;
//         memcpy(target+offset, word_ptr->copy_A, align);
//     }
//     else
//     {
//         // Copies bytes to destination of target + (offset_mult * word_ptr)
//         // Copy align number of bytes into the target (destination) + offset
//         size_t offset = offset_mult*align;
//         memcpy(target+offset, word_ptr->copy_B, align);
//     }
// }


// bool read_word(wordNode_t word_ptr, void* unused(target), size_t offset_mult, size_t align)
// {   
    
//     // Transaction is read-only, since no process is currently writing to this very word
//     if(!word_ptr ->writing)
//     {
//         read_a_or_b(word_ptr, target, offset_mult, align);
//         return true;
//     }
//     else
//     {   
//         // Someone is writing
//         if(word_ptr ->writing)
//         {   
//             // Is not currently beeing accessed, i.e. the person is done writing, and have updated the writable copy
//             if(!word_ptr->accessed)
//             {   
//                 // Will read the writable copy into target, this will be the opposite of valid_a
//                 if(word_ptr ->valid_a) 
//                     target = word_ptr ->copy_B;

//                 else
//                     target = word_ptr ->copy_A;

//                 // The transaction can continue
//                 return true; 
//             }
//             // The person writing is not done writing, can't take the chance on actually figuring out
//             // if we should read from the readable copy or the writeable copy.
//             else
//             {
//                 return false;
//             }
//         }
//         // The person is currently done with writing, it is therefore now a read-only!
//         else
//         {
//             read_a_or_b(word_ptr, target, offset_mult, align);
//             // To be sure we just check that currently accessed is set to false!
//             if(word_ptr -> accessed) 
//                 word_ptr -> accessed = false;
//             return true;
//         }
//     }
// }

// /** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
//  * @param shared Shared memory region associated with the transaction
//  * @param tx     Transaction to use
//  * @param source Source start address (in the shared region)
//  * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
//  * @param target Target start address (in a private region)
//  * @return Whether the whole transaction can continue
// **/
// bool tm_read(shared_t unused(shared), tx_t unused(tx), void const* source, size_t size, void* target) {
    
    
//     size_t align = tm_align(shared);
//     size_t align_chunks = size/align; // if size = 64 bytes and align = 8 bytes, we will count 8 align chunks
    

//     struct region* region = (struct region*) shared;
    
//     wordNode_t head = (wordNode_t) source;
//     wordNode_t tmp;
//     size_t counter = 0;
//     // Is expected to exit this by using counter == align_chunks
//     // If we hit head before counter == align, we don't have enough space to read from
//     while(head != NULL)
//     {
//         tmp = head -> next_word;
//         // Target + counter gives us adress chunk for private memory!
        
//         bool result = read_word(head, target, counter, align); 

//         // If one word read fails, we will need to abort the whole transaction!
//         if (!result)
//         {
//             return false;
//         }

//         head = tmp;
//         counter++;

//         // Check if we have read all the words, then transaction was sucessfull!
//         if (counter == align_chunks)
//             return true;
//     }

//     // If we get down here there was not enough words in our shared memory to satisfy the tm_read, we therefore abort!
//     return false;
// }





// void write_a_or_b(const void* unused(source), wordNode_t word_ptr, size_t offset_mult, size_t align)
// {

//     // If copy_A is currently valid, that means we have to write into B
//     // This way people can still read from copy_A until we are done with copy_B
//     if(word_ptr ->valid_a)
//     {

//         // Copies bytes from source + offset, since we are currently on the k-th time copying from source
//         // Copies this into destination of copy_B, this is passed on from a for loop in the other function, so we don't need to keep track of this offset
//         size_t offset = offset_mult*align;
//         memcpy(word_ptr->copy_B, source+offset, align);

//         // Updates valid_a variable when we are done writing to the shared area, we now know it is updated
//         word_ptr ->valid_a = false;
//     }
//     // If copy_B is currently valid, that means we have to write into A
//     // This way people can still read form Ccopy_B until we are done with copy_A
//     else
//     {

//         // Copies bytes from source + offset, since we are currently on the k-th time copying from source
//         // Copies this into destination of copy_B, this is passed on from a for loop in the other function, so we don't need to keep track of this offset
//         size_t offset = offset_mult*align;
//         memcpy(word_ptr->copy_A, source+offset, align);
//         // Updates valid_a variable when we are done writing to the shared area, we now know it is updated
//         word_ptr ->valid_a = true;
//     }
// }


// bool write_word(const void* unused(source), wordNode_t word_ptr, size_t offset_mult, size_t align)
// {
//     // If a word has been written in the current epoch, i.e. if someone is currently writing
//     if(word_ptr ->writing)
//     {   
//         // For later we can say that if anyone is current writing we just return false here,
//         // then we can optimize this later
//         // How to distinguish if a word is in the current access set or not given our current solution
//         return false;
        
//     }
//     else
//     {
//         if(word_ptr ->accessed)
//         {
//             return false;
//         }
//         else
//         {
//             write_a_or_b(source, word_ptr, offset_mult, align);
//             if(word_ptr -> accessed) 
//                 word_ptr -> accessed = true;
//             word_ptr ->writing = true;
//             return true;
//         }
//     }
// }

// /** [thread-safe] Write operation in the given transaction, source in a private region and target in the shared region.
//  * @param shared Shared memory region associated with the transaction
//  * @param tx     Transaction to use
//  * @param source Source start address (in a private region)
//  * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
//  * @param target Target start address (in the shared region)
//  * @return Whether the whole transaction can continue
// **/
// bool tm_write(shared_t unused(shared), tx_t unused(tx), void const* unused(source), size_t unused(size), void* unused(target)) {
//     // TODO: tm_write(shared_t, tx_t, void const*, size_t, void*)
    
    
//     struct region* region = (struct region*) shared;
//     size_t align = tm_align(shared);
//     size_t align_chunks = size/align; // if size = 64 bytes and align = 8 bytes, we will count 8 align chunks

//     wordNode_t head = (wordNode_t) target;
//     wordNode_t tmp;
//     size_t counter = 0;
//     // Is expected to exit this by using counter == align_chunks
//     // If we hit head before counter == align, we don't have enough space to read from
//     while(head != NULL)
//     {
//         tmp = head -> next_word;
//         // Target + counter*align gives us adress chunk for private memory!
        
        
//         bool result = write_word(source, head, counter, align);
//         // If one word write fails, we will need to abort the whole transaction!
//         if (!result)
//         {
//             return false;
//         }

//         head = tmp;
//         counter++;

//         // Check if we have written all the words, then transaction was sucessfull!
//         if (counter == align_chunks)
//             return true;
//     }

//     // If we get down here there was not enough words in our shared memory to satisfy the tm_read, we therefore abort!
//     return false;
// }


// /** [thread-safe] Memory allocation in the given transaction.
//  * @param shared Shared memory region associated with the transaction
//  * @param tx     Transaction to use
//  * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
//  * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
//  * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
// **/
// alloc_t tm_alloc(shared_t unused(shared), tx_t unused(tx), size_t unused(size), void** unused(target)) {
//     // TODO: tm_alloc(shared_t, tx_t, size_t, void**)
//     // Check if size is not 0, because then it acn still pass the test in the second else if
//     size_t align = tm_align(shared);
    
//     if (size-align < 0)
//         return abort_alloc;
//     //  Checks if the size is a positive multiple of align
//     else if(size%align != 0)
//         return abort_alloc;
//     // Checks if the size is a power of 2
//     else if(align%2 != 0)
//         return abort_alloc;
    
    
//     struct region* region = (struct region*) shared;
    
//     struct wordNode_t* wordNode;
//     if (unlikely(posix_memalign((void**)&wordNode, align, sizeof(struct wordNode_instance_t) + size) != 0)) // Allocation failed
//         return nomem_alloc;


    
    
//     size_t numb_words = (size_t) size/align;
//     wordNode_t init = calloc(1, sizeof(struct wordNode_instance_t));
//     init -> copy_A = calloc(1, align);
//     init -> copy_B = calloc(1, align);
    
//     init ->prev_word = NULL;
//     init ->next_word = region ->allocs; // Inserting the alloc in front of the first current word node
//     // If the next word is not equal to NULL, we must attach the next word to the current word
//     if(init->next_word)
//     {
//         (init->next_word)->prev_word = init;
//     }

//     region ->allocs = init;
//     void* segment = (void*) ((uintptr_t) wordNode + sizeof(struct wordNode_instance_t));
//     *target = segment;

//     return success_alloc;

    
//     init -> accessed = false;
//     init -> free = false;
//     init -> valid_a = true; // Setting default to that copy_A is valid
//     init -> writing = false;
//     init -> next_word = NULL; // Init to NULL, so it does not point to random place
    

//     wordNode_t tmp_head = init;
//     // If there size is a positive multiple of align
//     // that is bigger than 1, we will need to create more word nodes
//     for(size_t i = 0; i <(numb_words-1); i++)
//     {
//         wordNode_t new_word = calloc(1, sizeof(struct wordNode_instance_t));
//         if(new_word == NULL) { printf("Could not allocate memory!\n"); return abort_alloc; }
        

//         tmp_head ->next_word = new_word;
//         tmp_head = new_word;

//         tmp_head -> copy_A = calloc(1, align); 
//         tmp_head -> copy_B = calloc(1, align);
        
//         //Checking if allocation of memory was successfull
//         if((tmp_head -> copy_A) == NULL) { printf("Could not allocate memory!");return abort_alloc; }
//         if((tmp_head -> copy_B) == NULL) { printf("Could not allocate memory!");return abort_alloc; }

//         tmp_head -> accessed = false;
//         tmp_head -> free = false;
//         tmp_head -> valid_a = true; // Setting default to that copy_A is valid
//         tmp_head -> writing = false;
//         tmp_head -> next_word = NULL; // Init to NULL, so it does not point to random place

//     }

/**
 * @file   tm.c
 * @author Wermeille Bastien <bastien.wermeille@epfl.ch>
 * @version 1.0
 *
 * @section LICENSE
 *
 * [...]
 *
 * @section DESCRIPTION
 *
 * Implementation of my wonderful transaction manager.
**/

// Requested features
#define _GNU_SOURCE
#define _POSIX_C_SOURCE   200809L
#ifdef __STDC_NO_ATOMICS__
#error Current C11 compiler does not support atomic operations
#endif

// External headers
// #include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> //TODO: REMOVE FOR DEBUG PURPOSES
#include <semaphore.h>

#if (defined(__i386__) || defined(__x86_64__)) && defined(USE_MM_PAUSE)
#include <xmmintrin.h>
#else

#include <sched.h>

#endif

// Internal headers
#include <tm.h>

// -------------------------------------------------------------------------- //

/** Define a proposition as likely true.
 * @param prop Proposition
**/
#undef likely
#ifdef __GNUC__
#define likely(prop) \
        __builtin_expect((prop) ? 1 : 0, 1)
#else
#define likely(prop) \
        (prop)
#endif

/** Define a proposition as likely false.
 * @param prop Proposition
**/
#undef unlikely
#ifdef __GNUC__
#define unlikely(prop) \
        __builtin_expect((prop) ? 1 : 0, 0)
#else
#define unlikely(prop) \
        (prop)
#endif

/** Define one or several attributes.
 * @param type... Attribute names
**/
#undef as
#ifdef __GNUC__
#define as(type...) \
        __attribute__((type))
#else
#define as(type...)
#warning This compiler has no support for GCC attributes
#endif

// -------------------------------------------------------------------------- //

/** Pause for a very short amount of time.
**/
static inline void pause() {
#if (defined(__i386__) || defined(__x86_64__)) && defined(USE_MM_PAUSE)
    _mm_pause();
#else
    sched_yield();
#endif
}

// -------------------------------------------------------------------------- //

#define DEFAULT_FLAG 0
#define REMOVED_FLAG 1
#define ADDED_FLAG 2
#define ADDED_REMOVED_FLAG 3

#define BATCHER_NB_TX 12ul
#define MULTIPLE_READERS UINTPTR_MAX - BATCHER_NB_TX

static const tx_t read_only_tx = UINTPTR_MAX - 1ul;
static const tx_t destroy_tx = UINTPTR_MAX - 2ul;

struct mapping_entry {
    void *ptr;
    _Atomic (tx_t) status_owner;     // Identifier of the lock owner
    _Atomic (int) status;            // Whether this blocks need to be added or removed in case of rollback and commit

    size_t size;           // Size of the segment
};

struct batcher {
    atomic_ulong counter;
    atomic_ulong nb_entered;
    atomic_ulong nb_write_tx;

    atomic_ulong pass; // Ticket that acquires the lock
    atomic_ulong take; // Ticket the next thread takes
    atomic_ulong epoch;
};

struct region {
    size_t align;       // Claimed alignment of the shared memory region (in bytes)
    size_t align_alloc; // Actual alignment of the memory allocations (in bytes)

    struct batcher batcher;

    struct mapping_entry *mapping;
    atomic_ulong index;
};

// Return whether it's a success
void init_batcher(struct batcher *batcher) {
    batcher->counter = BATCHER_NB_TX;
    batcher->nb_entered = 0;
    batcher->nb_write_tx = 0;

    batcher->pass = 0;
    batcher->take = 0;
    batcher->epoch = 0;
}

tx_t enter(struct batcher *batcher, bool is_ro) {
    if (is_ro) {
        // Acquire status lock
        unsigned long ticket = atomic_fetch_add_explicit(&(batcher->take), 1ul, memory_order_relaxed);
        while (atomic_load_explicit(&(batcher->pass), memory_order_relaxed) != ticket)
            pause();
        atomic_thread_fence(memory_order_acquire);

        atomic_fetch_add_explicit(&(batcher->nb_entered), 1ul, memory_order_relaxed);

        // Release status lock
        atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);

        // printf("enter read\n");
        return read_only_tx;
    } else {
        while (true) {
            unsigned long ticket = atomic_fetch_add_explicit(&(batcher->take), 1ul, memory_order_relaxed);
            while (atomic_load_explicit(&(batcher->pass), memory_order_relaxed) != ticket)
                pause();
            atomic_thread_fence(memory_order_acquire);

            // Acquire status lock
            if (atomic_load_explicit(&(batcher->counter), memory_order_relaxed) == 0) {
                unsigned long int epoch = atomic_load_explicit(&(batcher->epoch), memory_order_relaxed);
                atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);

                while (atomic_load_explicit(&(batcher->epoch), memory_order_relaxed) == epoch)
                    pause();
                atomic_thread_fence(memory_order_acquire);
            } else {
                atomic_fetch_add_explicit(&(batcher->counter), -1ul, memory_order_release);
                break;
            }
        }
        atomic_fetch_add_explicit(&(batcher->nb_entered), 1ul, memory_order_relaxed);
        atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);

        tx_t tx = atomic_fetch_add_explicit(&(batcher->nb_write_tx), 1ul, memory_order_relaxed) + 1ul;
        atomic_thread_fence(memory_order_release);

        // printf("enter write\n");
        return tx;
    }
}

void batch_commit(struct region *region) {
    atomic_thread_fence(memory_order_acquire);

    for (size_t i = region->index - 1ul; i < region->index; --i) {
        struct mapping_entry *mapping = region->mapping + i;

        if (mapping->status_owner == destroy_tx ||
            (mapping->status_owner != 0 && (
                    mapping->status == REMOVED_FLAG || mapping->status == ADDED_REMOVED_FLAG)
            )
                ) {
            // Free this block
            unsigned long int previous = i + 1;
            if (atomic_compare_exchange_weak(&(region->index), &previous, i)) {
                free(mapping->ptr);
                mapping->ptr = NULL;
                mapping->status = DEFAULT_FLAG;
                mapping->status_owner = 0;
            } else {
                mapping->status_owner = destroy_tx;
                mapping->status = DEFAULT_FLAG;
            }
        } else {
            mapping->status_owner = 0;
            mapping->status = DEFAULT_FLAG;

            // Commit changes
            memcpy(mapping->ptr, ((char *) mapping->ptr) + mapping->size, mapping->size);

            // Reset locks
            memset(((char *) mapping->ptr) + 2 * mapping->size, 0, mapping->size / region->align * sizeof(tx_t));
        }
    };
    atomic_thread_fence(memory_order_release);
}

void leave(struct batcher *batcher, struct region *region, tx_t tx) {
    // Acquire status lock
    unsigned long ticket = atomic_fetch_add_explicit(&(batcher->take), 1ul, memory_order_relaxed);
    while (atomic_load_explicit(&(batcher->pass), memory_order_relaxed) != ticket)
        pause();
    atomic_thread_fence(memory_order_acquire);

    if (atomic_fetch_add_explicit(&batcher->nb_entered, -1ul, memory_order_relaxed) == 1ul) {
        if (atomic_load_explicit(&(batcher->nb_write_tx), memory_order_relaxed) > 0) {
            batch_commit(region);
            atomic_store_explicit(&(batcher->nb_write_tx), 0, memory_order_relaxed);
            atomic_store_explicit(&(batcher->counter), BATCHER_NB_TX, memory_order_relaxed);
            atomic_fetch_add_explicit(&(batcher->epoch), 1ul, memory_order_relaxed);
        } else {
        }
        atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);
    } else if (tx != read_only_tx) {
        unsigned long int epoch = atomic_load_explicit(&(batcher->epoch), memory_order_relaxed);
        atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);

        while (atomic_load_explicit(&(batcher->epoch), memory_order_relaxed) == epoch)
            pause();
    } else {
        atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);
    }
}

struct mapping_entry *get_segment(struct region *region, const void *source) {
    for (size_t i = 0; i < region->index; ++i) {
        if (unlikely(region->mapping[i].status_owner == destroy_tx)) {
            // printf("get_segment NULL\n");
            return NULL;
        }
        char *start = (char *) region->mapping[i].ptr;
        if ((char *) source >= start && (char *) source < start + region->mapping[i].size) {
            return region->mapping + i;
        }
    }
    // printf("Return null\n");
    return NULL;
}

/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
**/
shared_t tm_create(size_t size, size_t align) {
    struct region *region = (struct region *) malloc(sizeof(struct region));
    if (unlikely(!region)) {
        return invalid_shared;
    }

    size_t align_alloc = align < sizeof(void *) ? sizeof(void *) : align;
    size_t control_size = size / align_alloc * sizeof(tx_t);

    region->index = 1;
    region->mapping = malloc(getpagesize());
    if (unlikely(region->mapping == NULL)) {
        return invalid_shared;
    }
    memset(region->mapping, 0, getpagesize());
    region->mapping->size = size;
    region->mapping->status = DEFAULT_FLAG;
    region->mapping->status_owner = 0;

    if (unlikely(posix_memalign(&(region->mapping->ptr), align_alloc, 2 * size + control_size) != 0)) {
        free(region->mapping);
        free(region);
        return invalid_shared;
    }

    init_batcher(&(region->batcher));

    // Do we store a pointer to the control location
    memset(region->mapping->ptr, 0, 2 * size + control_size);
    region->align = align;
    region->align_alloc = align_alloc;

    return region;
}

/** Destroy (i.e. clean-up + free) a given shared memory region.
 * @param shared Shared memory region to destroy, with no running transaction
**/
void tm_destroy(shared_t shared) {
    // printf("Destroy !!!\n");
    struct region *region = (struct region *) shared;

    for (size_t i = 0; i < region->index; ++i) {
        free(region->mapping[i].ptr);
    }
    free(region->mapping);
    free(region);
}

/** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
 * @param shared Shared memory region to query
 * @return Start address of the first allocated segment
**/
void *tm_start(shared_t shared) {
    return ((struct region *) shared)->mapping->ptr;
}

/** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
 * @param shared Shared memory region to query
 * @return First allocated segment size
**/
size_t tm_size(shared_t shared) {
    return ((struct region *) shared)->mapping->size;
}

/** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
 * @param shared Shared memory region to query
 * @return Alignment used globally
**/
size_t tm_align(shared_t shared) {
    return ((struct region *) shared)->align_alloc;
}

/** [thread-safe] Begin a new transaction on the given shared memory region.
 * @param shared Shared memory region to start a transaction on
 * @param is_ro  Whether the transaction is read-only
 * @return Opaque transaction ID, 'invalid_tx' on failure
**/
tx_t tm_begin(shared_t shared, bool is_ro) {
    return enter(&(((struct region *) shared)->batcher), is_ro);
}

void tm_rollback(struct region *region, tx_t tx) {
    unsigned long int index = region->index;
    for (size_t i = 0; i < index; ++i) {
        struct mapping_entry *mapping = region->mapping + i;

        tx_t owner = mapping->status_owner;
        if (owner == tx && (mapping->status == ADDED_FLAG || mapping->status == ADDED_REMOVED_FLAG)) {
            mapping->status_owner = destroy_tx;
        } else if (likely(owner != destroy_tx && mapping->ptr != NULL)) {
            if (owner == tx) {
                mapping->status = DEFAULT_FLAG;
                mapping->status_owner = 0;
            }

            size_t align = region->align;
            size_t size = mapping->size;
            size_t nb = mapping->size / region->align;
            char *ptr = mapping->ptr;
            _Atomic (tx_t) volatile *controls = (_Atomic (tx_t) volatile *) (ptr + 2 * size);

            for (size_t j = 0; j < nb; ++j) {
                if (controls[j] == tx) {
                    memcpy(ptr + j * align + size, ptr + j * align, align);
                    controls[j] = 0;
                } else {
                    tx_t previous = 0ul - tx;
                    atomic_compare_exchange_weak(controls + j, &previous, 0);
                }
                atomic_thread_fence(memory_order_release);
            }
        }
    };

    leave(&(region->batcher), region, tx);
}

/** [thread-safe] End the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to end
 * @return Whether the whole transaction committed
**/
bool tm_end(shared_t shared, tx_t tx) {
    leave(&((struct region *) shared)->batcher, (struct region *) shared, tx);
    return true;
}

bool lock_words(struct region *region, tx_t tx, struct mapping_entry *mapping, void *target, size_t size) {
    size_t index = ((char *) target - (char *) mapping->ptr) / region->align;
    size_t nb = size / region->align;

    _Atomic (tx_t) volatile *controls = (_Atomic (tx_t) volatile *) ((char *) mapping->ptr + mapping->size * 2) + index;

    for (size_t i = 0; i < nb; ++i) {
        tx_t previous = 0;
        tx_t previously_read = 0ul - tx;
        if (!(atomic_compare_exchange_strong_explicit(controls + i, &previous, tx, memory_order_acquire,
                                                      memory_order_relaxed) ||
              previous == tx ||
              atomic_compare_exchange_strong(controls + i, &previously_read, tx))) {
            // printf("Unable to lock %lu - %lu --- %lu %lu %lu\n", tx, i+index, previous, previously_read, 0ul - tx);

            if (i > 1) {
                // printf("Rollback lock i: %ld \t index: %ld\n", i-1, index);
                memset((void *) controls, 0, (i - 1) * sizeof(tx_t));
                atomic_thread_fence(memory_order_release);
            }
            return false;
        }
    }
    return true;
}

bool tm_read_write(shared_t shared, tx_t tx, void const *source, size_t size, void *target) {
    struct region *region = (struct region *) shared;
    struct mapping_entry *mapping = get_segment(region, source);
    if (unlikely(mapping == NULL)) {
        // printf("Rollback in read !!!");
        tm_rollback(region, tx);
        return false;
    }

    size_t align = region->align_alloc;
    size_t index = ((char *) source - (char *) mapping->ptr) / align;
    size_t nb = size / align;

    _Atomic (tx_t) volatile *controls = ((_Atomic (tx_t) volatile *) (mapping->ptr + mapping->size * 2)) + index;

    atomic_thread_fence(memory_order_acquire);
    // Read the data
    for (size_t i = 0; i < nb; ++i) {
        tx_t no_owner = 0;
        tx_t owner = atomic_load(controls + i);
        if (owner == tx) {
            memcpy(((char *) target) + i * align, ((char *) source) + i * align + mapping->size, align);
        } else if (atomic_compare_exchange_strong(controls + i, &no_owner, 0ul - tx) ||
                   no_owner == 0ul - tx || no_owner == MULTIPLE_READERS ||
                   (no_owner > MULTIPLE_READERS &&
                    atomic_compare_exchange_strong(controls + i, &no_owner, MULTIPLE_READERS))
                ) {
            memcpy(((char *) target) + i * align, ((char *) source) + i * align, align);
        } else {
            tm_rollback(region, tx);
            return false;
        }
    }
    return true;
}

/** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in the shared region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in a private region)
 * @return Whether the whole transaction can continue
**/
bool tm_read(shared_t shared, tx_t tx, void const *source, size_t size, void *target) {
    if (likely(tx == read_only_tx)) {
        // Read the data
        memcpy(target, source, size);
        return true;
    } else {
        // printf("%lu read-write\n", tx);
        return tm_read_write(shared, tx, source, size, target);
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
bool tm_write(shared_t shared, tx_t tx, void const *source, size_t size, void *target) {
    // printf("%lu write\n", tx);
    struct region *region = (struct region *) shared;
    struct mapping_entry *mapping = get_segment(region, target);

    if (mapping == NULL || !lock_words(region, tx, mapping, target, size)) {
        tm_rollback(region, tx);
        return false;
    }

    memcpy((char *) target + mapping->size, source, size);
    return true;
}

/** [thread-safe] Memory allocation in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
 * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
 * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
**/
alloc_t tm_alloc(shared_t shared, tx_t tx, size_t size, void **target) {
    struct region *region = (struct region *) shared;
    unsigned long int index = atomic_fetch_add_explicit(&(region->index), 1ul, memory_order_relaxed);
    // printf("alloc ------------------ %lu\n", index);

    struct mapping_entry *mapping = region->mapping + index;
    mapping->status_owner = tx;
    mapping->size = size;
    mapping->status = ADDED_FLAG;

    size_t align_alloc = region->align_alloc;
    size_t control_size = size / align_alloc * sizeof(tx_t);

    void *ptr = NULL;
    if (unlikely(posix_memalign(&ptr, align_alloc, 2 * size + control_size) != 0))
        return nomem_alloc;

    memset(ptr, 0, 2 * size + control_size);
    *target = ptr;
    mapping->ptr = ptr;

    return success_alloc;
}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
**/
bool tm_free(shared_t shared, tx_t tx, void *segment) {
    struct mapping_entry *mapping = get_segment((struct region *) shared, segment);

    tx_t previous = 0;
    if (mapping == NULL || !(atomic_compare_exchange_strong(&mapping->status_owner, &previous, tx) || previous == tx)) {
        tm_rollback((struct region *) shared, tx);
        return false;
    }

    if (mapping->status == ADDED_FLAG) {
        mapping->status = ADDED_REMOVED_FLAG;
    } else {
        mapping->status = REMOVED_FLAG;
    }

    return true;
}