////////////////////////////////////////////////////////////////////////////////
// Reference Stephen Brennan (https://brennan.io/2015/01/16/write-a-shell-in-c/)
//
////////////////////////////////////////////////////////////////////////////////

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "command.h"

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

// Stephen Brennan
// return num executed
int sane_launch(command_t *command)
{
    if (command != NULL) {
        pid_t pid;

        pid = fork();
        if (pid == 0) {
            // Child
            if (execvp(command->argv[0], command->argv) == -1) {
                perror("sane");
            }
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent
            int status;

            // Don't wait for background process
            if (strcmp(command->sep, SEP_CON) != 0) {
                // Wait for child process to finish
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
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
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
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
    int i = 0;
    while (i < numCommands) {
        if (strcmp(commands[i].sep, SEP_PIPE) != 0) {
            // If not pipe sequence, do normal launch
            sane_launch(&commands[i]);
            ++i;
        } else {
            // Else launch the sequence of piped commands
            int numPipedCommands = 1;
            for (int j = i; j < numCommands; ++j) {
                if (strcmp(commands[j].sep, SEP_PIPE) == 0) {
                    ++numPipedCommands;
                }
            }
            sane_launchPipedSequence_r(numPipedCommands, &commands[i]);
            i += numPipedCommands;
        }
    }
}
