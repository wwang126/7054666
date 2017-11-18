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
  token = strtok(input, " \n\t\r\a");
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
      token = strtok(NULL, " \n\t\r\a");
  }
  //add NULL terminator
  output[i] = NULL;
  flags->argCnt = i;
  return output;
}
/*
 * Execute non-built in commands
 */
void execCmd(char** args, struct flagStruct * flags, char* cmdStatus){
    pid_t pid;
    int status = 0;
    //Fork off child proccess for command
    pid = fork();
    //If child proccess
    if(pid == 0){
        //Handle input/output redirection
        int inStatus = 0;
        int outStatus = 0;
        //Set up input redirection
        if(flags->redirIn){
            //open up file
            if((inStatus = open(flags->inFile, O_RDONLY))== -1){
                //If Failure
                fprintf(stderr, "File opening error\n");
                exit(EXIT_FAILURE);
            }
            //Otherwise, redirect input
            else{
                if ((dup2(inStatus,0)) == -1){
                    fprintf(stderr, "Redirect failure\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        //set up output redirection
        if(flags->redirOut){
            //open up file
            if((outStatus = open(flags->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1){
                //If Failure
                fprintf(stderr, "File opening error\n");
                exit(EXIT_FAILURE);
            }
            //Otherwise, redirect input
            else{
                if ((dup2(outStatus,1)) == -1){
                    fprintf(stderr, "Redirect failure\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        if(execvp(*args, args) == -1){
            //if fails write to standard error
            fprintf(stderr, "Execute Failure\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    //If fork failed
    if(pid == -1){
        //if fails write to standard error
        fprintf(stderr, "Fork Failure\n");
        exit(EXIT_FAILURE);
    }
    //If parent
    else{
        //if background command
        if(flags->bgCmd == 1){
            //Wait for child process
            waitpid(pid, &status, WUNTRACED);
            if(WIFEXITED(status)){
                //send status of child process
                sprintf(cmdStatus, "Proccess exited with: %d\n", WIFEXITED(status));
            }
            if(WIFSIGNALED(status)){
                sprintf(cmdStatus, "Terminated by signal: %d\n", WIFSIGNALED(status));
                //If killed by signal, print out signal
                printf("%s",cmdStatus);
                fflush(stdout);
            }
        }

    }
}
/*
 * Runs a list of commands from an array of strings
 */
void runCmd(char** args, struct flagStruct * flags, char* cmdStatus){
    //Check for NULL and check for comment
    if(*args != NULL){
        //Check for exit
        if(strcmp(args[0] ,"exit") == 0){
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
        else if(strcmp(args[0],"status") == 0){
            //print status of last command
            printf("%s\n", cmdStatus );
            //push output for re-entrant
            fflush(stdout);
        }
        else{
            //Execute commands
            execCmd(args,flags,cmdStatus);
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
        //Print out command prompt
        printf(": ");
        //Intilize flagStruct
        struct flagStruct flags = {0,0,0,0,NULL,NULL};
        //String reader
        char* input = readInput();
        //arguemnts
        char** args;
        args = parseInput(input,&flags);
        runCmd(args,&flags,cmdStatus);

    }
}
