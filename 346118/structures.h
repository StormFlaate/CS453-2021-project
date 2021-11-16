#include <stdbool.h>



#ifndef WORD_H
    #define WORD_H

    struct word {
        void *control;
        void *copy_a;
        void *copy_b;
    };

#endif
#ifndef WORDS_H
    #define WORDS_H

    struct word *words;
    
#endif
#ifndef CONTROL_H   
    #define CONTROL_H

    struct control {
        bool valid_a; // Which copy is "valid" from the previous epoch, true for A false for B
        bool can_access_set; // true if no other transaction is currently accessing it, false otherwise
        bool has_written; // wheter the word has been written in the current epoch

    };
    
#endif
