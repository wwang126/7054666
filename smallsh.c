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

//By default all is 0, if tripped 1
struct flagStruct{
    int argCnt;
    int bgCmd;
    int redirIn;
    int redirOut;

    char* inFile;
    char* outFile;
};


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
    ssize_t len = 0;
    getline(&input,&len,stdin);
    return input;
}
/*
 * Parses input into seperate commands
 * adds special handling for specific commands
 */
char **parseInput(char* input, struct flagStruct * flags){
  //char delim[3] = " \n";
  //place to store new strings
  char* token;
  //Max is 512 args so only need 512 spots
  char** output = malloc(sizeof(char*) * 512);
  //counter for storage
  int i = 0;
  //get first token
  token = strtok(input, " \n");
  //get next tokens
  while( token != NULL ) {
      //Check if token is special command
      if(strcmp(token, "<") == 0){
          flags->redirIn = 1;//Set flag
          token = strtok(NULL, " \n");
          flags->inFile = token; //Store file name
      }
      else if(strcmp(token, ">") == 0){
          flags->redirOut = 1; //Set flag
          token = strtok(NULL, " \n");
          flags->outFile = token; //Store file name
      }
      else if(strcmp(token, "&") == 0){
          flags->bgCmd = 1;
      }
      else{
          output[i] = token;
          i++;
      }
      //token to next null
      token = strtok(NULL, " \n");
  }
  flags->argCnt = i;
  return output;
}


/*
 * Main function that runs when started
 */
int main(int argc, char *argv[]){
    //set run to 1
    run = 1;
    //Main running loop
    while(run){
        //Intilize flagStruct
        struct flagStruct flags = {0,0,0,0,NULL,NULL};
        //String reader
        char* input = readInput();
        printf("%s\n",input);
        parseInput(input,&flags);
        printf("args passed: %d\n",flags.argCnt);
        printf("Input file: %s\n",flags.inFile);
        printf("Output file: %s\n",flags.outFile);
        run = 0;
    }
}
