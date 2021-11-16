#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>





int counter = 0;
int remaining = 0;

int get_epoch()
{
    return counter;
}

void enter()
{

}