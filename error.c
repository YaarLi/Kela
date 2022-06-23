#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "ks_internal.h"

void comp_err(char* message)  //prints the error messages and exits the program
{
    fflush(stdout); //ensure things get printed first
    fprintf(stderr, "[Error on line %d: %s]\n", line_ind, message); //stderr is normally the same as stdout
    //exit(EXIT_FAILURE);  // exit as unsuccessful
    char* msg = malloc(strlen(message)*sizeof(char));
    strcpy(msg, message);
    pthread_exit(msg);
}
