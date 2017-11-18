#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

int run = 0; //if this is 1 contiue running
int pids[64]; //background processes
int numPids;

/*
 * Main function that runs when started
 */
int main(int argc, char *argv[]){
    //set run to 1
    run = 1;
    //Main running loop
    while(run){
        //String reader
        printf("test\n");
        char* input = readInput();
        printf("%s\n",input);
        run = 0;
    }
}
/*
 * Takes in input and outputs it in a buffer
 */
char* readInput(){
    //Print out command prompt
    printf(": ");
    //Flush buffer
    fflush(stdout);
    //Set vars for getline
    char* input = NULL;
    size_t len = 0;
    getline(&input,&len,stdin);
    return input;
}
/*
 * Parses input into seperate commands
 */
char **parseInput(char* input){
  char* delim[2] = " ";
  //place to store new strings
  char* token;
  //process out strings
  token = strtok(input, delim);
}
