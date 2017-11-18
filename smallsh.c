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
int fgndO; //Foreground only for signaling


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
    //Expand $$ into pids
    while(strstr(input,"$$")){
        sprintf(strstr(input,"$$"), "%d", getpid());
    }
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

void catchInt(int signo){
 if(signo == SIGTSTP){
        if(fgndO == 0){
            fgndO = 1;
            char* msg = "Foreground mode enabled.\n";
            write(STDOUT_FILENO, msg, 24);
        }
        else{
            fgndO = 0;
            char* msg = "Foreground mode disabled.\n";
            write(STDOUT_FILENO, msg, 25);
        }
    }
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
        //If bg child
        if(flags->bgCmd == 1){
            sigaction(SIGINT, sig, NULL); //interruptable
        }

        //Handle input/output redirection
        int inStatus = 0;
        int outStatus = 0;
        if(flags->redirIn == 1 || flags->redirOut == 1){
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
        }
        if(execvp(*args, args) == -1){
            fflush(stdout);
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
        //Save process ids of children to kill later
        pids[numPids] = pid;
        numPids++;
        //if background command
        if(flags->bgCmd != 1){
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
    if(*args != NULL && **args != '#'){
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
 * Parent process kills all child processes
 */
void filicide(){
    //loop thru all pids and kill them
    int i = 0;
    while (i != numPids){
        kill(pids[i],SIGKILL);
        i++;
    }
    numPids = 0;
}

/*
 * Main function that runs when started
 */
int main(int argc, char *argv[]){
    //set status
    char* cmdStatus = malloc(sizeof(char)*36);
    //set run to 1
    run = 1;
    numPids = 0;

    //Intiliaze signal handlers
    struct sigaction SIGTSTP_Action, IGNORE_Action;
    SIGTSTP_Action.sa_handler = catchInt;
    sigfillset(&SIGTSTP_Action.sa_mask);
    SIGTSTP_Action.sa_flags= 0;
    sigaction(SIGTSTP, &SIGTSTP_Action, NULL);
    IGNORE_Action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &IGNORE_Action, NULL);

    //Create flagStruct
    struct flagStruct flags = {0,0,0,0,NULL,NULL};
    //Main running loop
    while(run){
        filicide();
        //Print out command prompt
        printf(": ");
        fflush(stdout);
        //Reset flags struct
        flags.argCnt = 0;
        flags.bgCmd = 0;
        flags.redirIn = 0;
        flags.redirOut = 0;
        flags.inFile = NULL;
        flags.outFile = NULL;
        //String reader
        char* input = readInput();
        //arguemnts
        char** args;
        args = parseInput(input,&flags);
        runCmd(args,&flags,cmdStatus);

    }
}
