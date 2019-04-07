#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <wait.h>
#include <malloc.h>
#include <unistd.h>


#define SIZE 512
#define ZERO 0
#define FAILURE "Error in system call\n"
#define EXIT "exit"
#define JOBS "jobs"
#define MAN "man"
#define AMP "&"
#define PROMPT "> "
#define WRONG_ARG "~"
#define PATH "HOME"
#define CD "cd"
#define EXIT_ZERO "0"
#define EXIT_ONE "1"
#define EXIT_TWO "2"
#define EXIT_THREE "3"

struct Jobs {

    pid_t pid;
    struct Jobs *nextJob;
    char *input;

};

void freeCommand(char *command[SIZE]);

/**
 * Function name: prev
 * The input: struct Jobs*
 * The output: struct Jobs*
 * The function operation: The function gets a pointer to the linked list
 * Jobs and returns it's previous pointer.
 */
struct Jobs *prev(struct Jobs *Jobs, struct Jobs *dest);

/**
 * Function name: cdCommand
 * The input: char**
 * The output: void
 * The function operation: The function gets the parameters above,
 * and runs the cd operation.
 */
void cdCommand(char **command);

/**
 * Function name: exitCommand
 * The input: char**, struct, char*
 * The     printf("jobs : show jobs which run in the background\n");
output: the exit code
 * The function operation: The function gets the parameters above,
 * frees the input given by the user, prints the pid , frees all the jobs
 * in the linked list and kills each process.
 */
int exitCommand(char **command, struct Jobs *Jobs);

/**
 * Function name: run
 * The input: none
 * The output: the exit code
 * The function operation: The function runs the entire program. when
 * finished returns the exit code given by the exitCommand.
 * @return
 */
int run();

/**
 * Function name: userInput
 * The input: none
 * The output: The command the user has entered
 * The function operation: The function gets the user input into a buffer,
 * allocates the space needed and pastes into the allocated address. returns
 * finally, returns the input.
 */
char *userInput();

/**
 * Function name: identifyCommand
 * The input: char*, char* array[512]
 * The output: none
 * The function operation: The function gets the parameters given above,
 * which consists the command given by the user. splits the command into
 * tokens and inserts into the char* array of size 512.
 */
void identifyCommand(char *command, char *array[SIZE]);

/**
 * Function name: checkIsJob
 * The input: char* array[512]
 * The output: none
 * The function operation: The function checks if the command which was
 * splitted before, contains &, if so it should be a background command, and
 * returns the index where the & is at, at the array.
 * If doesn't consists &, returns 0
 */
int checkIsJob(char *array[SIZE]);

/**
 * Function name: insertsToJob
 * The input: char* job[512], pointer to struct Jobs, int, int
 * The output: none
 * The function operation: The function gets the splitted command given by the
 * user, a pointer to the linked list Jobs, the index of the &, and the pid.
 * Inserts the command to the linked list.
 */
void insertToJobs(char *job[SIZE], struct Jobs *head, int indexOfAmp, int pid,
                  int *size);

/**
 * Function name: exitCodeReturn
 * The input: char**
 * The output: int
 * The function operation: The function gets command splitted, and returns
 * the right exit code given by the argurment[1]
 */
int exitCodeReturn(char **command);

/**
 * Function name: jobs
 * The input: pointer to struct Jobs
 * The output: none
 * The function operation: The function prints all the jobs in the linked list.
 */
struct Jobs *jobs(struct Jobs *Jobs, int*size);

/**
 * Function name: main
 * The input: none
 * The output: exit code
 * The function operation: The main functions runs the run() function.
 * @return
 */

/**
 * Function name: manCommand
 * The input: none
 * The output: none
 * The function operation: The function displays the operation
 */
void manCommand();

int main() {
    return run();
}

int run() {
    struct Jobs *Jobs = (struct Jobs *) malloc(sizeof(struct Jobs));
    char *command[SIZE];
    int status;
    int size = 0;
    pid_t pid;
    printf("%s", PROMPT);
    char *input = userInput();
    // while user didn't enter exit
    while (strcmp(input, EXIT) != 0) {
        if (strcmp(input, JOBS) == 0) {
            Jobs = jobs(Jobs,&size);
        } else if (strcmp(input, MAN) == 0) {
            manCommand();
        } else {
            // splitting the tokens to the command array
            identifyCommand(input, command);
            if (strcmp(command[ZERO], CD) == 0) {
                cdCommand(command);
            } else {
                int check = checkIsJob(command);
                if (check > 0) {
                    // free(command[check-1]);
                    free(command[check - 1]);
                    command[check-1] = NULL;
                }
                if ((pid = fork()) < 0) {
                    fprintf(stderr, FAILURE);
                }
                if (check > 0) {
                    insertToJobs(command, Jobs, check, pid, &size);
                }
                // if it's a job
                // if we are in the father
                if (pid > 0) {
                    printf("%d\n", pid);
                    if (check == 0) {
                        wait(&status);
                    }
                    // we are in the child
                } else if (pid == 0) {
                    if (execvp(command[ZERO], command) < 0) {
                        fprintf(stderr, FAILURE);
                    }
                    exit(0);
                }

            }
        }
        printf("%s", PROMPT);
        free(input);
        freeCommand(command);
        input = userInput();
    }
    identifyCommand(input, command);
    int statusCode = exitCommand(command, Jobs);
    freeCommand(command);
    free(input);;
    return statusCode;

}

void freeCommand(char *command[SIZE]) {
    int i = 0;
    while (command[i] != NULL) {
        free(command[i]);
        command[i] = NULL;
        i++;
    }
}

void manCommand() {
    printf("help:\n"
           "jobs: show jobs which are currently running in the background.\n"
           "cd: move to another directory.\n"
           "exit: quit from program.\n");

}

void cdCommand(char **command) {
    printf("%d\n", getpid());
    if (command[1] == NULL || (!strcmp(command[1], WRONG_ARG))) {
        chdir(getenv(PATH));
    } else {
        if (chdir(command[1]) < 0) {
            fprintf(stderr, FAILURE);
        };
    }
}

int exitCommand(char *command[SIZE], struct Jobs *Jobs) {
    printf("%d\n", getpid());
    struct Jobs *pointer = Jobs;
    struct Jobs *nextPtr;
    while (pointer != NULL) {
        kill(pointer->pid, SIGKILL);
        nextPtr = pointer->nextJob;
        free(pointer->input);
        free(pointer);
        pointer = nextPtr;
    }
    return exitCodeReturn(command);
}

char *userInput() {
    char buffer[SIZE] = "";
    int length = 0;
    fgets(buffer, SIZE, stdin);
    length = strlen(buffer);
    char *array = (char *) malloc(sizeof(char) * length + 1);
    strcpy(array, buffer);
    array[length - 1] = '\0';
    return array;
}

void identifyCommand(char *command, char *array[SIZE]) {
    char *tokens = strtok(command, " ");
    int i = 0;
    while (tokens != NULL) {
        array[i] = (char *) malloc(sizeof(char) * strlen(tokens) + 1);
        strcpy(array[i], tokens);
        i++;
        tokens = strtok(NULL, " ");
    }
    free(tokens);
}

int checkIsJob(char *array[SIZE]) {
    int indicator = 0;
    int index = 0;
    char *pointer = array[index];
    while (pointer != NULL) {
        index++;
        if ((strcmp(pointer, AMP)) == 0) {
            indicator = 1;
            break;
        } else {
            pointer = array[index];
        }
    }
    if (indicator == 1) {
        return index;
    } else {
        return indicator;
    }
}

void
insertToJobs(char *command[SIZE], struct Jobs *head, int indexOfAmp, int pid,
             int *size) {
    int i = 0;
    int numOfBytes = 0;
    // we are counting how much allocation needed
    while (i < indexOfAmp - 1) {
        numOfBytes += strlen(command[i]);
        i++;
    }
    char *tempString = (char *) malloc(sizeof(char) * numOfBytes + 2);
    i = 0;
    while (i < indexOfAmp - 1) {
        strcat(tempString, command[i]);
        if (command[i+1] != NULL) {
            //if (command[i + 1] != NULL) {
            strcat(tempString, " ");
        }
        i++;
    }
    // if there is no job at all, it's the first
    if (*size == 0) {
        head->input = (char *) malloc(strlen(tempString) + 1);
        strcpy(head->input, tempString);
        head->pid = pid;
        // there is at least one job, we we look for the last job and enter
    } else {
        struct Jobs *pointer = head;
        while (pointer->nextJob != NULL) {
            pointer = pointer->nextJob;
        }
        pointer->nextJob = (struct Jobs *) malloc(sizeof(struct Jobs));
       // pointer->nextJob->input = (char *) malloc(
         //       sizeof(char) * numOfBytes + 1);
        pointer->nextJob->input = (char *) malloc(
                strlen(tempString)+1);
        strcpy(pointer->nextJob->input, tempString);
        pointer->nextJob->pid = pid;
    }
    free(tempString);
    (*size)++;
}

struct Jobs *jobs(struct Jobs *Jobs, int *size) {
    struct Jobs *pointer = Jobs;
    struct Jobs *prevPointer;
    if (*size == 0){
        return Jobs;
    }
    struct Jobs *nextPointer;
    pid_t pid;
    int status;
    while (pointer != NULL) {
        pid = waitpid(pointer->pid, &status, WNOHANG);
        if (pid == pointer->pid) {
            prevPointer = prev(Jobs, pointer);
            // in this situation, its the first, we just delete and move on
            if (prevPointer == NULL) {
                nextPointer = pointer->nextJob;
                free(pointer->input);
                free(pointer);
                Jobs = nextPointer;
                pointer = nextPointer;
                (*size)--;
                //we need to delete node in the middle
            } else {
                nextPointer = pointer->nextJob;
                free(pointer->input);
                free(pointer);
                prevPointer->nextJob = nextPointer;
                pointer = nextPointer;
                (*size)--;
            }

        } else {
            pointer = pointer->nextJob;
        }
    }
    pointer = Jobs;
    while (pointer != NULL) {
        printf("%d %s\n", pointer->pid, pointer->input);
        pointer = pointer->nextJob;
    }
    return Jobs;
}

struct Jobs *prev(struct Jobs *Jobs, struct Jobs *dest) {
    struct Jobs *copyPointer = Jobs;
    while (copyPointer != NULL) {
        if (copyPointer->nextJob == dest) {
            return copyPointer;
        } else {
            copyPointer = copyPointer->nextJob;
        }
    }
    return NULL;
}

int exitCodeReturn(char **command) {
    if (strcmp(command[ZERO], EXIT_ZERO) == 0) {
        return 0;
    } else if (strcmp(command[ZERO], EXIT_ONE) == 0) {
        return 1;
    } else if (strcmp(command[ZERO], EXIT_TWO) == 0) {
        return 2;
    } else if (strcmp(command[ZERO], EXIT_THREE) == 0) {
        return 3;
    }
    return 0;
}
