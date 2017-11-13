#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>

int run = 0; //if this is 1 contiue running

/*
 * Main function that runs when started
 */
 int main(int argc, char *argv[]){
     //set run to 1
     run = 1;
     //Main running loop
     while(run){
         printf("test\n");
         run = 0;
     }
 }
