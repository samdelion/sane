////////////////////////////////////////////////////////////////////////////////
/// Reference Stephen Brennan
/// (https://brennan.io/2015/01/16/write-a-shell-in-c/)
///
////////////////////////////////////////////////////////////////////////////////

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "command.h"

////////////////////////////////////////////////////////////////////////////////
/// For 'n' commands we need 'n - 1' pipe structures, which are made up of 2
/// file
/// descriptors each.
////////////////////////////////////////////////////////////////////////////////
#define MAX_NUM_PIPES ((MAX_NUM_COMMANDS - 1) * 2)

// Where we store our pipes
int sane_pipes[MAX_NUM_PIPES];
// Number of pipes currently open
unsigned int sane_numPipes = 0;

// Shell built-in function declartions
/* int sane_cd(char **argv); */
/* int sane_exit(char **argv); */
/* int sane_help(char **argv); */
/* int sane_prompt(char **argv); */
/* int sane_pwd(char **argv); */

/* // Strings used to call built-in functions and function pointer */
/* // (note order matches in both arrays) */
/* char *sane_builtinStr[] = {"cd", "exit", "help", "prompt", "pwd"}; */

/* int (*sane_builtinFunc[])(char **) = {&sane_cd, &sane_exit, &sane_help, */
/*                                       &sane_prompt, &sane_pwd}; */

// Return the number of shell built-in functions.
/* int sane_numBuiltins() */
/* { */
/*     return sizeof(sane_builtinStr) / sizeof(char *); */
/* } */

// Close all open pipes, sets sane_numPipes = 0
void sane_pipesClose()
{
    // There are two file descriptors for each 'pipe' structure
    for (int i = 0; i < sane_numPipes * 2; ++i) {
        close(sane_pipes[i]);
    }
    sane_numPipes = 0;
}

// Set all pipe fds to -1, make sure you close all open pipe fds first!
void sane_pipesReset()
{
}

// Create num pipes, sets sane_numPipes to num too.
void sane_pipesCreate(unsigned int num)
{
    for (int i = 0; i < num; ++i) {
        pipe(sane_pipes + (i * 2));
    }
    sane_numPipes = num;
}

// Stephen Brennan
// return num executed
pid_t sane_launch(command_t *command, int fdIn, int fdOut, int shouldWait)
{
    pid_t pid = -1;

    if (command != NULL) {
        pid = fork();
        if (pid == 0) {
            // Child
            if (fdIn != 0) {
                dup2(fdIn, STDIN_FILENO);
            }
            if (fdOut != 1) {
                dup2(fdOut, STDOUT_FILENO);
            }

            sane_pipesClose();

            if (execvp(command->argv[0], command->argv) == -1) {
                perror("sane");
            }
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent
            int status;

            /* // Don't wait for background process */
            /* if (shouldWait) { */
            /*     // Wait for child process to finish */
            /*     do { */
            /*         waitpid(pid, &status, WUNTRACED); */
            /*     } while (!WIFEXITED(status) && !WIFSIGNALED(status)); */
            /* } else { */
            /*     // TODO: Add to jobs list */
            /*     // If is a background process, print pid (like bash) */
            /*     printf("%d\n", pid); */
            /* } */
        } else {
            // Error
            perror("sane");
        }
    }

    return pid;
}

void sane_launchPipedSequence_r(int numCommands, command_t *commands)
{
    if (numCommands > 0 && commands != NULL) {
        pid_t pid;

        pid = fork();
        if (pid == 0) {
            // Child
            if (execvp(commands[0].argv[0], commands[0].argv) == -1) {
                perror("sane");
            }
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Check if more commands in sequence to execute
            --numCommands;
            if (numCommands >= 1) {
                sane_launchPipedSequence_r(numCommands, ++commands);
            }

            // Parent
            int status;

            // Don't wait for background processes
            if (strcmp(commands[0].sep, SEP_CON) != 0) {
                // Wait for child process to finish
                /* do { */
                /*     waitpid(pid, &status, WUNTRACED); */
                /* } while (!WIFEXITED(status) && !WIFSIGNALED(status)); */
            } else {
                // TODO: Add to jobs list
                // If is a background process, print pid (like bash)
                printf("%d\n", pid);
            }
        } else {
            // Error
            perror("sane");
        }
    }
}

int sane_execute(int numCommands, command_t *commands)
{
    /* // For each command */
    /* for (int i = 0; i < numCommands; ++i) { */
    /*     sane_launch(&commands[i]); */
    /*     /\* if () *\/ */
    /*     /\* for (int j = 0; j < sane_numBuiltins(); ++j) { *\/ */
    /*     /\*     return (*sane_builtinFunc[j])(args); *\/ */
    /*     /\* } *\/ */
    /*     // If is built in, execute */
    /*     // Else */
    /*     //     fork */
    /*     //       child execute command */
    /*     //       parent wait if not background job */
    /* } */
    // Iterate through commands and execute them all
    /* int i = 0; */
    /* while (i < numCommands) { */
    /*     if (strcmp(commands[i].sep, SEP_PIPE) != 0) { */
    /*         // If not pipe sequence, do normal launch */
    /*         sane_launch(&commands[i], 0, 1); */
    /*         ++i; */
    /*     } else { */
    /*         // Else launch the sequence of piped commands */
    /*         int numPipedCommands = 1; */
    /*         for (int j = i; j < numCommands; ++j) { */
    /*             if (strcmp(commands[j].sep, SEP_PIPE) == 0) { */
    /*                 ++numPipedCommands; */
    /*             } */
    /*         } */
    /*         /\* sane_launchPipedSequence_r(numPipedCommands, &commands[i]);
     * *\/ */
    /*         int k = 0; */
    /*         int pipes[2]; */
    /*         pipe(pipes); */

    /*         sane_launch */
    /*         i += numPipedCommands; */
    /*     } */
    /* } */
    /* int i = 0; */
    /* sane_pipesCreate(2); */
    /* if (i == 0) { */
    /*     /\* printf("command: 0, fdIn: %d, fdOut: %d\n", 0, sane_pipes[1]);
     * *\/ */
    /*     /\* sane_launch(&commands[i], 0, sane_pipes[1], 0); *\/ */
    /*     /\* printf("command: 1, fdIn: %d, fdOut: %d\n", sane_pipes[0], *\/ */
    /*     /\*        sane_pipes[3]); *\/ */
    /*     /\* sane_launch(&commands[i + 1], sane_pipes[0], sane_pipes[3], 0);
     * *\/ */
    /*     /\* printf("command: 2, fdIn: %d, fdOut: %d\n", sane_pipes[2], 1);
     * *\/ */
    /*     /\* sane_launch(&commands[i + 2], sane_pipes[2], 1, 0); *\/ */
    /* } */
    /* sane_pipesClose(); */
    /* sane_pipesReset(); */
    /* int status; */
    /* for (int i = 0; i < 3; ++i) { */
    /*     wait(&status); */
    /* } */
    int i = 0;
    while (i < numCommands) {
        // Piped command
        if (strcmp(commands[i].sep, SEP_PIPE) == 0) {
            //
            // Execute sequences of pipe commands at once for example, in:
            //
            // 'whoami ; cat out.txt | sort | less ; echo "Hello"'
            //           |_________________________|
            // this section ^ would be considered a sequence of piped commands.

            // Find out how many to execute
            int numPipedCommands = 0;
            for (int j = i; j < numCommands; ++j) {
                if (strcmp(commands[j].sep, SEP_PIPE) == 0) {
                    ++numPipedCommands;
                }
            }
            // Also include last element in pipe sequence (which will not
            // contain a pipe seperator [see 'less' in above example])
            ++numPipedCommands;

            // For n commands we need n-1 pipes
            sane_pipesCreate(numPipedCommands - 1);

            for (int k = 0; k < numPipedCommands; ++k) {
                if (k == 0) {
                    /* printf("command: %d, fdIn: %d, fdOut: %d, \n", i + k, */
                    /*        STDIN_FILENO, sane_pipes[1]); */
                    // First command in sequence:
                    //  - Use stdin for in, pipe for out (first write pipe)
                    sane_launch(&commands[i + k], STDIN_FILENO, sane_pipes[1],
                                0);
                } else if (k == numPipedCommands - 1) {
                    /* printf("command: %d, fdIn: %d, fdOut: %d, \n", i + k, */
                    /*        sane_pipes[((sane_numPipes * 2) - 1) - 1], */
                    /*        STDOUT_FILENO); */
                    // Last command in sequence:
                    //  - Use pipe (last read pipe) for in, stdout for out
                    sane_launch(&commands[i + k],
                                sane_pipes[((sane_numPipes * 2) - 1) - 1],
                                STDOUT_FILENO, 0);
                } else {
                    /* printf("command: %d, fdIn: %d, fdOut: %d, \n", i + k, */
                    /*        sane_pipes[(k - 1) * 2], sane_pipes[(k * 2) + 1]); */
                    // 'k'th command in sequence:
                    //  - Use pipe from previous command for in, pipe for this
                    //  command for out
                    sane_launch(&commands[i + k], sane_pipes[((k - 1) * 2) + 0],
                                sane_pipes[(k * 2) + 1], 0);
                }
            }

            // Close all pipes
            sane_pipesClose();
            // Reset all pipes
            sane_pipesReset();

            // Wait for each forked child to finish
            int status;
            for (int k = 0; k < numPipedCommands; ++k) {
                wait(&status);
            }


            i += numPipedCommands;
        }
    }

    return 1;
}
