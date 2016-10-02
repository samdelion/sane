////////////////////////////////////////////////////////////////////////////////
/// Reference Stephen Brennan
/// (https://brennan.io/2015/01/16/write-a-shell-in-c/)
///
////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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
int sane_help(char **argv);
/* int sane_prompt(char **argv); */
/* int sane_pwd(char **argv); */

int sane_help(char **argv)
{
    printf("sane shell: Help granted.\n");

    return 0;
}

/* // Strings used to call built-in functions and function pointer */
/* // (note order matches in both arrays) */
/* char *sane_builtinStr[] = {"cd", "exit", "help", "prompt", "pwd"}; */
char *sane_builtinStr[] = {"help"};

/* int (*sane_builtinFunc[])(char **) = {&sane_cd, &sane_exit, &sane_help, */
/*                                       &sane_prompt, &sane_pwd}; */
int (*sane_builtinFuncs[])(char **) = {&sane_help};

// Return the number of shell built-in functions.
int sane_numBuiltins()
{
    return sizeof(sane_builtinStr) / sizeof(char *);
}

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
pid_t sane_launch(command_t *command, int fdIn, int fdOut)
{
    pid_t pid = -1;

    if (command != NULL) {
        pid = fork();
        if (pid == 0) {
            // Child
            //  Handle redirection and piping
            if (command->stdin_file == NULL) {
                // If no redirection, use pipe
                if (fdIn != STDIN_FILENO) {
                    dup2(fdIn, STDIN_FILENO);
                }
            } else {
                // Else use redirection
                // @note: redirection overrides piping, similar to bash shell
                int in = open(command->stdin_file,
                              O_RDONLY); // Open for reading only
                dup2(in, STDIN_FILENO);
                // Close unneeded file descriptor
                close(in);
            }
            if (command->stdout_file == NULL) {
                if (fdOut != STDOUT_FILENO) {
                    dup2(fdOut, STDOUT_FILENO);
                }
            } else {
                int out =
                    open(command->stdout_file, O_WRONLY | O_TRUNC | O_CREAT,
                         S_IRUSR | S_IRGRP | S_IWGRP |
                             S_IWUSR); // Open for writing, truncate file to 0
                                       // (clear it), create file if it does not
                                       // exist, with read and write permissions
                                       // for owner of file and group
                dup2(out, STDOUT_FILENO);
                // Close unneeded file descriptor
                close(out);
            }

            // If is builtin, execute builtin function (Stephen Brennan)
            for (int i = 0; i < sane_numBuiltins(); ++i) {
                if (strcmp(command->argv[0], sane_builtinStr[i]) == 0) {
                    sane_pipesClose();

                    (*sane_builtinFuncs[i])(command->argv);
                    exit(EXIT_SUCCESS);
                }
            }

            // Close any open pipes in child
            sane_pipesClose();

            // Else, execute command
            if (execvp(command->argv[0], command->argv) == -1) {
                perror("sane");
            }
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
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
    int i = 0;
    while (i < numCommands) {
        if (strcmp(commands[i].sep, SEP_SEQ) == 0) {
            // Don't catch SIGCHLD (child terminated) signals during this
            // critical section, otherwise the SIGCHLD signal handler will reap
            // the process created and the below call to waitpid will fail
            sigset_t sigset;
            sigemptyset(&sigset);
            sigaddset(&sigset, SIGCHLD);

            sigprocmask(SIG_SETMASK, &sigset, NULL);

            {
                pid_t pid =
                    sane_launch(&commands[i], STDIN_FILENO, STDOUT_FILENO);

                // Wait for child process to finish
                int status;
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }

            // Allow SIGCHLD signals to be processed again, signals received
            // during critical section will now be processed.
            sigprocmask(SIG_UNBLOCK, &sigset, NULL);

            ++i;
        } else if (strcmp(commands[i].sep, SEP_CON) == 0) {
            pid_t pid = sane_launch(&commands[i], STDIN_FILENO, STDOUT_FILENO);

            // Don't wait for child process to finish
            printf("%d\n", pid);

            ++i;
        }
        // Piped command
        else if (strcmp(commands[i].sep, SEP_PIPE) == 0) {
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
                    sane_launch(&commands[i + k], STDIN_FILENO, sane_pipes[1]);
                } else if (k == numPipedCommands - 1) {
                    /* printf("command: %d, fdIn: %d, fdOut: %d, \n", i + k, */
                    /*        sane_pipes[((sane_numPipes * 2) - 1) - 1], */
                    /*        STDOUT_FILENO); */
                    // Last command in sequence:
                    //  - Use pipe (last read pipe) for in, stdout for out
                    sane_launch(&commands[i + k],
                                sane_pipes[((sane_numPipes * 2) - 1) - 1],
                                STDOUT_FILENO);
                } else {
                    /* printf("command: %d, fdIn: %d, fdOut: %d, \n", i + k, */
                    /*        sane_pipes[(k - 1) * 2], sane_pipes[(k * 2) + 1]);
                     */
                    // 'k'th command in sequence:
                    //  - Use pipe from previous command for in, pipe for this
                    //  command for out
                    sane_launch(&commands[i + k], sane_pipes[((k - 1) * 2) + 0],
                                sane_pipes[(k * 2) + 1]);
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
