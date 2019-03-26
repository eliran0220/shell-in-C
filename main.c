#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

#define SIZE 512
#define ZERO 0
#define ERROR_EXCEVP "Error executing"
#define ERROR_FORK "Error fork"
#define EXIT "exit"
#define JOBS "jobs"
#define WRONG_ARG "~"
#define PATH "HOME"
#define EXIT_ZERO "0"
#define EXIT_ONE "1"
#define EXIT_TWO "2"
#define EXIT_THREE "3"


struct Jobs {

    pid_t pid;
    struct Jobs *nextJob;
    char *input;

};

/**
 * Function name: exitCommand
 * The input: char**, struct, char*
 * The output: the exit code
 * The function operation: The function gets the parameters above,
 * frees the input given by the user, prints the pid , frees all the jobs
 * in the linked list and kills each process.
 */
int exitCommand(char **command, struct Jobs *Jobs, char *toEmpty);

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
void insertToJobs(char *job[SIZE], struct Jobs *head, int indexOfAmp, int pid);

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
void jobs(struct Jobs *Jobs);

/**
 * Function name: main
 * The input: none
 * The output: exit code
 * The function operation: The main functions runs the run() function.
 * @return
 */
int main() {
    return run();
}

int run() {
    struct Jobs *Jobs = (struct Jobs *) malloc(sizeof(struct Jobs));
    char *command[SIZE];
    int status;
    pid_t pid;
    char *input = userInput();
    while (strcmp(input, EXIT) != 0) {
        if (strcmp(input, JOBS) == 0) {
            jobs(Jobs);
        } else {
            identifyCommand(input, command);
            int check = checkIsJob(command);
            if ((pid = fork()) < 0) {
                printf("%s\n", ERROR_FORK);
            } else if (pid == 0) {// אני נמצא בבן
                if (execvp(command[ZERO], command) < 0) {
                    printf("%s\n", ERROR_EXCEVP);
                }
            } else {
                if (check > 0) {
                    insertToJobs(command, Jobs, check, pid);
                }
                printf("%d\n", getpid());
                if (!check) {
                    wait(&status);
                }

            }
        }
        input = userInput();
    }

    exitCommand(command, Jobs, input);
}

void cdCommand(char **command) {
    printf("%d\n", getpid());
    if (command[ZERO] == NULL || (strcmp(command[1], WRONG_ARG)) == 0) {
        chdir(getenv(PATH));
    } else {
        chdir(command[ZERO]);
    }
}

int exitCommand(char **command, struct Jobs *Jobs, char *toEmpty) {
    free(toEmpty);
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

    exitCodeReturn(command);

}

char *userInput() {
    char buffer[SIZE];
    int length = 0;
    if (fgets(buffer, SIZE, stdin) != NULL) {
        length = strlen(buffer);
    }
    char *array = (char *) malloc(sizeof(char) * length);
    strcpy(array, buffer);
    array[length - 1] = '\0';
    return array;
}

void identifyCommand(char *command, char *array[SIZE]) {
    char *tokens = strtok(command, " ");
    int i = 0;
    while (tokens != NULL) {
        printf("%s", tokens);
        array[i] = (char *) malloc(sizeof(char) * strlen(tokens));
        strcpy(array[i], tokens);
        i++;
        tokens = strtok(NULL, " ");
    }
}

int checkIsJob(char *array[SIZE]) {
    int indicator = 0;
    int index = 0;
    char *pointer = array[index];
    while (pointer != NULL) {
        index++;
        if ((strcmp(pointer, "&\n")) == 0) {
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

void insertToJobs(char *job[SIZE], struct Jobs *head, int indexOfAmp, int pid) {
    int i = 0;
    int numOfBytes = 0;
    while (i < indexOfAmp - 1) {
        numOfBytes += strlen(job[i]);
        i++;
    }
    char *tempString = (char *) malloc(sizeof(char) * numOfBytes);
    i = 0;
    while (i < indexOfAmp - 1) {
        strcat(tempString, job[i]);
        strcat(tempString, " ");
        i++;
    }
    if (head->input == NULL) {
        head->input = (char *) malloc(sizeof(char) * numOfBytes);
        strcpy(head->input, tempString);
        head->pid = pid;
    } else {
        struct Jobs *pointer = head;
        while (pointer->nextJob != NULL) {
            pointer++;
        }
        pointer->nextJob = (struct Jobs *) malloc(sizeof(struct Jobs));
        pointer->nextJob->input = (char *) malloc(sizeof(char) * numOfBytes);
        strcpy(pointer->nextJob->input, tempString);
        pointer->nextJob->pid = pid;
    }
}

void jobs(struct Jobs *Jobs) {
    struct Jobs *pointer = Jobs;
    if (pointer->input == NULL) {
        return;
    }
    while (pointer != NULL) {
        printf("%d %s %s", pointer->pid, pointer->input, " ");
        pointer = pointer->nextJob;
    }
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