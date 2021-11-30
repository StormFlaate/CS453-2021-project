#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Internal headers
#include "../include/tm.h"
#include "macros.h"

typedef struct wordNode_instance_t* wordNode_t;

// Linked list of words!
struct wordNode_instance_t
{
    // CONTROL!
    // Which copy is "valid" from the previous epoch!  true --> A  |  false --> B
    bool valid_a; 
    // true if at least one other transactions is in the access set, false otherwise
    // access set: a set of transactions that are currently accessing this word!
    bool accessed;
    // wheter the word is currently being written to  
    bool writing;
    // If is non-free-able allocated segment
    bool non_free_able;

    // COPY!
    void* copy_A;
    void* copy_B;

    // wordNode_t is a pointer to another wordNode_instance_t
    wordNode_t next_word; 
};




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

    printf("var size size: %zu\n", size);
    printf("var align size: %zu\n", align);
    printf("number of words: %zu\n", numb_words);

    wordNode_t init = calloc(1, sizeof(struct wordNode_instance_t));
    init -> copy_A = calloc(1, align);
    init -> copy_B = calloc(1, align);
    
    
    if(init == NULL) {printf("Could not allocate memory!");exit(-1);}
    if((init -> copy_A) == NULL) {printf("Could not allocate memory!");exit(-1);}
    if((init -> copy_B) == NULL) {printf("Could not allocate memory!");exit(-1);}

    init -> accessed = false;
    init -> valid_a = NULL; // Neither copy A or copy B is currently valid!
    init -> writing = false;
    init -> next_word = NULL; // Init to NULL, so it does not point to random place
    init -> non_free_able = true; // This word is part of the non-free-able allocated segment

    printf("wordNode_t addr interval: %p - %p\n", init, (init+1));
    printf("copy_A addr interval: %p - %p\n", init ->copy_A, (init ->copy_A)+align);
    printf("copy_B addr interval: %p - %p\n", init ->copy_B, (init ->copy_B)+align);
    printf("accessed addr interval: %p - %p\n", &(init -> accessed), (&(init -> accessed)+1));
    printf("valid_a addr interval: %p - %p\n", &(init -> valid_a), (&(init -> valid_a)+1));
    printf("writing addr interval: %p - %p\n", &(init -> writing), (&(init -> writing)+1));
    
    wordNode_t tmp_head = init;
    // If there size is a positive multiple of align
    // that is bigger than 1, we will need to create more word nodes
    for(int i = 0; i <(numb_words-1); i++)
    {
        wordNode_t new_word = calloc(1, sizeof(struct wordNode_instance_t));
        if(new_word == NULL) {printf("Could not allocate memory!");exit(-1);}
        

        tmp_head ->next_word = new_word;
        tmp_head = new_word;

        tmp_head -> copy_A = calloc(1, align); 
        tmp_head -> copy_B = calloc(1, align);
        
        //Checking if allocation of memory was successfull
        if((tmp_head -> copy_A) == NULL) {printf("Could not allocate memory!");exit(-1);}
        if((tmp_head -> copy_B) == NULL) {printf("Could not allocate memory!");exit(-1);}

        tmp_head -> accessed = false;
        tmp_head -> valid_a = NULL; // Neither copy A or copy B is currently valid!
        tmp_head -> writing = false;
        tmp_head -> next_word = NULL; // Init to NULL, so it does not point to random place
        tmp_head -> non_free_able = true; // This word is part of the non-free-able allocated segment
        
        printf("\n\n");
        printf("wordNode_t addr interval: %p - %p\n", tmp_head, (tmp_head+1));
        printf("copy_A addr interval: %p - %p\n", tmp_head ->copy_A, (tmp_head ->copy_A)+align);
        printf("copy_B addr interval: %p - %p\n", tmp_head ->copy_B, (tmp_head ->copy_B)+align);
        printf("accessed addr interval: %p - %p\n", &(tmp_head -> accessed), (&(tmp_head -> accessed)+1));
        printf("valid_a addr interval: %p - %p\n", &(tmp_head -> valid_a), (&(tmp_head -> valid_a)+1));
        printf("writing addr interval: %p - %p\n", &(tmp_head -> writing), (&(tmp_head -> writing)+1));
    }

    return init;
}

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

size_t tm_align(shared_t unused(shared)) {
    return sizeof( ((wordNode_t) shared) ->copy_A );
}

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
    
    
    if(init == NULL) {printf("Could not allocate memory!");return abort_alloc;}
    if((init -> copy_A) == NULL) {printf("Could not allocate memory!");return abort_alloc;}
    if((init -> copy_B) == NULL) {printf("Could not allocate memory!");return abort_alloc;}
    

    printf("tm_alloc - Successfully allocated memory for word and copies\n");

    init -> accessed = false;
    init -> valid_a = NULL; // Neither copy A or copy B is currently valid!
    init -> writing = false;
    init -> next_word = NULL; // Init to NULL, so it does not point to random place
    init -> non_free_able = false; // This word is in the set of the allocated segments
    printf("\n\n");
    printf("tm_alloc - wordNode_t addr interval: %p - %p\n", init, (init+1));

    wordNode_t tmp_head = init;
    // If there size is a positive multiple of align
    // that is bigger than 1, we will need to create more word nodes
    for(int i = 0; i <(numb_words-1); i++)
    {
        wordNode_t new_word = calloc(1, sizeof(struct wordNode_instance_t));
        if(new_word == NULL) {printf("Could not allocate memory!");return abort_alloc;}
        

        tmp_head ->next_word = new_word;
        tmp_head = new_word;

        tmp_head -> copy_A = calloc(1, align); 
        tmp_head -> copy_B = calloc(1, align);
        
        //Checking if allocation of memory was successfull
        if((tmp_head -> copy_A) == NULL) {printf("Could not allocate memory!");return abort_alloc;}
        if((tmp_head -> copy_B) == NULL) {printf("Could not allocate memory!");return abort_alloc;}
        

        tmp_head -> accessed = false;
        tmp_head -> valid_a = NULL; // Neither copy A or copy B is currently valid!
        tmp_head -> writing = false;
        tmp_head -> next_word = NULL; // Init to NULL, so it does not point to random place
        tmp_head -> non_free_able = false; // This word is in the set of the allocated segments
        printf("tm_alloc - wordNode_t addr interval: %p - %p\n", tmp_head, (tmp_head+1));
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

void tm_destroy(shared_t unused(shared)) {
    // TODO: tm_destroy(shared_t)
    
    wordNode_t head = shared;
    wordNode_t tmp;
    
    // 
    int counter = 1;
    //

    // Will need to iterate over all next words, to free up their memory
    while(head != NULL)
    {
        tmp = head -> next_word;
        free(head ->copy_A);
        free(head ->copy_B);
        free(head);

        //
        printf("Freed %d. node\n", counter);
        counter++;
        //

        head = tmp;
    }
}


int main()
{

    printf("Now we are running.....\n");
    
    // Input number of nodes:
    int c = 2;

    shared_t word_head = tm_create(8*c,8);

    size_t align_size = tm_align(word_head);
    size_t full_size = tm_size(word_head);

    printf("The align size: %zu\n", align_size);
    printf("The size size: %zu\n", full_size);

    // Target area for where we will input the adress
    void* target = calloc(1, sizeof( void* ));
    if (target == NULL)
        { printf("Could not allocate memory"), exit(-1); }
    tm_alloc(word_head, 1, 8*c, &target);

    printf("Current address of target: %p\n", &target);
    printf("Address at target: %p\n", target);



    printf("\n");
    tm_destroy(word_head);
    
    printf("Code executed with success!\n");


    return 0;
}
