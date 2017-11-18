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
  //add NULL terminator
  output[i] = NULL;
  flags->argCnt = i;
  return output;
}
// /*
//  * Execute non-built in commands
//  */
// void execCmd(char** args, struct flagStruct * flags, char* cmdStatus){
//     pid_t pid;
//     int status = 0;
//     printf("%s was sent in\n", args[0]);
//     //Fork off child proccess for command
//     pid = fork();
//     //If child proccess
//     if(pid == 0){
//         if(execvp(*args, args) == -1){
//             //if fails write to standard error
//             fprintf(stderr, "Execute Failure\n");
//             exit(EXIT_FAILURE);
//         }
//         printf("Executing cmd: %s", *args);
//         fflush(stdout);
//         exit(EXIT_SUCCESS);
//     }
//     //If fork failed
//     if(pid == -1){
//         //if fails write to standard error
//         fprintf(stderr, "Fork Failure\n");
//         exit(EXIT_FAILURE);
//     }
// }
/*
 * Runs a list of commands from an array of strings
 */
void runCmd(char** args, struct flagStruct * flags, char* cmdStatus){
    printf("Arg 0 is %s\n",args[0]);
    //Check for NULL and check for comment
    if(*args != NULL){
        //Check for exit
        if(strcmp(*args,"exit") == 0){
            //exit for shell from exit command
            exit(EXIT_SUCCESS);
        }
        //check for change directory
        else if(strcmp(args[0],"cd")==0){
            //grab destination
            if(args[1] == NULL){
                //if the cd is blank, go home
                chdir(getenv("HOME"));
            }
            else{
                //Otherwise go to directory
                if(chdir(args[1])!= 0){
                    printf("Directory not found!\n");
                    fflush(stdout);
                }
            }
        }
        else if(strcmp(args[0],"status")){
            //print status of last command
            printf("%s\n", cmdStatus );
            //push output for re-entrant
            fflush(stdout);
        }
        else{
            //Execute commands 
            pid_t pid;
            int status = 0;
            printf("%s was sent in\n", args[0]);
            //Fork off child proccess for command
            pid = fork();
            //If child proccess
            if(pid == 0){
                if(execvp(*args, args) == -1){
                    //if fails write to standard error
                    fprintf(stderr, "Execute Failure\n");
                    exit(EXIT_FAILURE);
                }
                printf("Executing cmd: %s", *args);
                fflush(stdout);
                exit(EXIT_SUCCESS);
            }
            //If fork failed
            if(pid == -1){
                //if fails write to standard error
                fprintf(stderr, "Fork Failure\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


/*
 * Main function that runs when started
 */
int main(int argc, char *argv[]){
    //set status
    char* cmdStatus = malloc(sizeof(char)*36);
    //set run to 1
    run = 1;
    //Main running loop
    while(run){
        //Intilize flagStruct
        struct flagStruct flags = {0,0,0,0,NULL,NULL};
        //String reader
        char* input = readInput();
        //arguemnts
        char** args;
        printf("%s\n",input);
        args = parseInput(input,&flags);
        runCmd(args,&flags,cmdStatus);
        //printf("args passed: %d\n",flags.argCnt);
        //printf("Input file: %s\n",flags.inFile);
        //printf("Output file: %s\n",flags.outFile);
        //printf("%s %s %s\n",args[0],args[1],args[2]);
        //run = 0;
    }
}
