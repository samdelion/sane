////////////////////////////////////////////////////////////////////////////////
/// Reference Stephen Brennan
/// (https://brennan.io/2015/01/16/write-a-shell-in-c/)
///
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"
#include "sane.h"

static char *sane_promptString = NULL;

const char *sane_getPrompt()
{
    return sane_promptString;
}

int sane_init()
{
    int result = 0;

    assert(sane_promptString == NULL &&
           "sane_promptString was set before sane_init()");

    // Set default prompt
    const char *defaultPrompt = "%";

    size_t promptLen = strlen(defaultPrompt) + 1;
    sane_promptString = (char *)malloc(promptLen);

    // Ensure memory allocation did not fail
    if (sane_promptString != NULL) {
        memset(sane_promptString, '\0', promptLen);
        strcpy(sane_promptString, defaultPrompt);
    } else {
        result = 1;
    }

    return result;
}

void sane_shutdown()
{
    if (sane_promptString != NULL) {
        free(sane_promptString);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Shell built-in function declarations.
////////////////////////////////////////////////////////////////////////////////

/* int sane_cd(int argc, char **argv); */
int sane_exit(int argc, char **argv);
int sane_help(int argc, char **argv);
int sane_prompt(int argc, char **argv);
int sane_pwd(int argc, char **argv);
int sane_cd(int argc, char **argv);

int sane_help(int argc, char **argv)
{
    printf("     ___           ___           ___           ___     ");
    printf("\n    /\\  \\         /\\  \\         /\\__\\         /\\  \\    ");
    printf("\n   /::\\  \\       /::\\  \\       /::|  |       /::\\  \\   ");
    printf(
        "\n  /:/\\ \\  \\     /:/\\:\\  \\     /:|:|  |      /:/\\:\\  \\  ");
    printf("\n _\\:\\~\\ \\  \\   /::\\~\\:\\  \\   /:/|:|  |__   /::\\~\\:\\  "
           "\\ ");
    printf("\n/\\ \\:\\ \\ \\__\\ /:/\\:\\ \\:\\__\\ /:/ |:| /\\__\\ /:/\\:\\ "
           "\\:\\__\\");
    printf("\n\\:\\ \\:\\ \\/__/ \\/__\\:\\/:/  / \\/__|:|/:/  / \\:\\~\\:\\ "
           "\\/__/");
    printf(
        "\n \\:\\ \\:\\__\\        \\::/  /      |:/:/  /   \\:\\ \\:\\__\\  ");
    printf("\n  \\:\\/:/  /        /:/  /       |::/  /     \\:\\ \\/__/  ");
    printf("\n   \\::/  /        /:/  /        /:/  /       \\:\\__\\    ");
    printf("\n    \\/__/         \\/__/         \\/__/         \\/__/   ");
    printf("\n\n Samuel Evans-Powell and Nathan Gane shell\n");

    return EXIT_SUCCESS;
}

int sane_exit(int argc, char **argv)
{
    kill(getpid(), SIGUSR1);

    return EXIT_SUCCESS;
}

int sane_prompt(int argc, char **argv)
{
    // Did we receive the correct number of arguments?
    if (argc == 2) {
        // Free previous prompt
        free(sane_promptString);

        // Set new prompt
        size_t promptLen = strlen(argv[1]) + 1;
        sane_promptString = (char *)malloc(promptLen);

        // Ensure memory allocation did not fail
        if (sane_promptString != NULL) {
            memset(sane_promptString, '\0', promptLen);
            strcpy(sane_promptString, argv[1]);
        } else {
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "usage: prompt [string]\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int sane_pwd(int argc, char **argv)
{
    if (argc == 1) {
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "usage: pwd\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int sane_cd(int argc, char **argv)
{
    if (argc == 1) {
        // No arguments given to cd, go home
        if (chdir(getenv("HOME")) == -1) {
            perror("cd");
            return EXIT_FAILURE;
        }
    } else if (argc > 1) {
        if (chdir(argv[1]) == -1) {
            perror("cd");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

// Strings used to call built-in functions and function pointer
// (note order matches in both arrays)
char *sane_builtinStr[] = {"help", "exit", "prompt", "pwd", "cd"};

int (*sane_builtinFuncs[])(int, char **) = {&sane_help, &sane_exit,
                                            &sane_prompt, &sane_pwd, &sane_cd};

// Return the number of shell built-in functions.
int sane_numBuiltins()
{
    return sizeof(sane_builtinStr) / sizeof(char *);
}

////////////////////////////////////////////////////////////////////////////////
/// Pipes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// For 'n' commands we need 'n - 1' pipe structures, which are made up of 2
/// file descriptors each.
////////////////////////////////////////////////////////////////////////////////
#define MAX_NUM_PIPES ((MAX_NUM_COMMANDS - 1) * 2)

// Where we store our pipes
int sane_pipes[MAX_NUM_PIPES];
// Number of pipes currently open
unsigned int sane_numPipes = 0;

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

////////////////////////////////////////////////////////////////////////////////
/// Execution
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @return If executed by main process, returns 0. If executed by child
/// process, returns the pid of that child process. In case of error, returns
/// -1.
////////////////////////////////////////////////////////////////////////////////
pid_t sane_launch(command_t *command, int fdIn, int fdOut)
{
    pid_t pid = -1;

    if (command != NULL) {
        // Determine if command is built-in
        int builtInIt = 0;
        for (; builtInIt < sane_numBuiltins(); ++builtInIt) {
            if (strcmp(command->argv[0], sane_builtinStr[builtInIt]) == 0) {
                break;
            }
        }

        // Reached end of builtins, command is not a builtin
        if (builtInIt == sane_numBuiltins()) {
            // Spawn a child to execute command
            pid = fork();
            if (pid == 0) {
                // Child
                //  Handle redirection and piping
                if (command->stdin_file == NULL) {
                    // If no redirection, use pipe
                    if (fdIn != STDIN_FILENO) {
                        dup2(fdIn, STDIN_FILENO);
                        close(fdIn);
                    }
                } else {
                    // Else use redirection
                    // @note: redirection overrides piping, similar to bash
                    // shell
                    int in = open(command->stdin_file,
                                  O_RDONLY); // Open for reading only

                    if (in > 0) {
                        dup2(in, STDIN_FILENO);
                        // Close unneeded file descriptor
                        close(in);
                    } else {
                        perror("sane open");
                        exit(EXIT_FAILURE);
                    }
                }
                if (command->stdout_file == NULL) {
                    if (fdOut != STDOUT_FILENO) {
                        dup2(fdOut, STDOUT_FILENO);
                        close(fdOut);
                    }
                } else {
                    int out = open(
                        command->stdout_file, O_WRONLY | O_TRUNC | O_CREAT,
                        S_IRUSR | S_IRGRP | S_IWGRP |
                            S_IWUSR); // Open for writing, truncate file to 0
                                      // (clear it), create file if it does not
                                      // exist, with read and write permissions
                                      // for owner of file and group
                    dup2(out, STDOUT_FILENO);
                    // Close unneeded file descriptor
                    close(out);
                }

                // Close any open pipes in child
                sane_pipesClose();

                // Else, execute command
                if (execvp(command->argv[0], command->argv) == -1) {
                    perror("sane exec");
                }
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                // Error
                perror("sane fork");
            }
        } else {
            pid = 0;
            //  Handle redirection and piping
            if (command->stdin_file == NULL) {
                // If no redirection, use pipe
                if (fdIn != STDIN_FILENO) {
                    dup2(fdIn, STDIN_FILENO);
                    close(fdIn);
                }
            } else {
                // Else use redirection
                // @note: redirection overrides piping, similar to bash
                // shell
                int in = open(command->stdin_file,
                              O_RDONLY); // Open for reading only
                if (in > 0) {
                    dup2(in, STDIN_FILENO);
                    // Close unneeded file descriptor
                    close(in);
                } else {
                    perror("sane open");
                    exit(EXIT_FAILURE);
                }
            }
            if (command->stdout_file == NULL) {
                if (fdOut != STDOUT_FILENO) {
                    dup2(fdOut, STDOUT_FILENO);
                    close(fdOut);
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

            int argc = 0;
            for (int i = 0; command->argv[i] != NULL; ++i) {
                ++argc;
            }

            // Execute command
            (*sane_builtinFuncs[builtInIt])(argc, command->argv);
        }
    }

    return pid;
}

void sane_execute(int numCommands, command_t *commands)
{
    int i = 0;

    int stdinCopy = dup(0);
    int stdoutCopy = dup(1);

    while (i < numCommands) {
        if (strcmp(commands[i].sep, SEP_SEQ) == 0) {
            // Don't catch SIGCHLD (child terminated) signals during this
            // critical section, otherwise the SIGCHLD signal handler will
            // reap
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

            ++i;
        }
        // Piped command
        else if (strcmp(commands[i].sep, SEP_PIPE) == 0) {
            // Whether or not main process should wait on job to finish (false
            // if separator of last command is SEP_CON)
            int shouldWait = 1;

            //
            // Execute sequences of pipe commands at once for example, in:
            //
            // 'whoami ; cat out.txt | sort | less ; echo "Hello"'
            //           |_________________________|
            // this section ^ would be considered a sequence of piped
            // commands.

            // Find out how many contiguous pipes to execute
            int numPipedCommands = 0;
            for (int j = i; j < numCommands; ++j) {
                if (strcmp(commands[j].sep, SEP_PIPE) == 0) {
                    ++numPipedCommands;
                } else {
                    if (strcmp(commands[j].sep, SEP_CON) == 0) {
                        shouldWait = 0; // Don't wait for concurrent job
                    }
                    break;
                }
            }

            // Also include last element in pipe sequence (which will not
            // contain a pipe seperator [see 'less' in above example])
            ++numPipedCommands;

            // For n commands we need n-1 pipes
            sane_pipesCreate(numPipedCommands - 1);

            int numCommandsToWaitFor = numPipedCommands;
            for (int k = 0; k < numPipedCommands; ++k) {
                pid_t pid = -1;

                if (k == 0) {
                    // First command in sequence:
                    //  - Use stdin for in, pipe for out (first write pipe)
                    pid = sane_launch(&commands[i + k], STDIN_FILENO,
                                      sane_pipes[1]);

                } else if (k == numPipedCommands - 1) {
                    // Last command in sequence:
                    //  - Use pipe (last read pipe) for in, stdout for out
                    pid = sane_launch(&commands[i + k],
                                      sane_pipes[((sane_numPipes * 2) - 1) - 1],
                                      STDOUT_FILENO);
                } else {
                    // 'k'th command in sequence:
                    //  - Use pipe from previous command for in, pipe for
                    //  this
                    //  command for out
                    pid = sane_launch(&commands[i + k],
                                      sane_pipes[((k - 1) * 2) + 0],
                                      sane_pipes[(k * 2) + 1]);
                }

                // A builtin command was executed
                if (pid == 0) {
                    // Done executing command inbuilt command, rewire stdin and
                    // stdout in
                    // main process
                    dup2(stdinCopy, 0);
                    dup2(stdoutCopy, 1);
                    // close(stdinCopy);
                    // close(stdoutCopy);

                    // No need to wait for builtin command
                    --numCommandsToWaitFor;
                }
            }

            // Close all pipes
            sane_pipesClose();
            // Reset all pipes
            sane_pipesReset();

            if (shouldWait) {
                // Don't catch SIGCHLD (child terminated) signals during this
                // critical section, otherwise the SIGCHLD signal handler will
                // reap
                // the process created and the below call to waitpid will fail
                sigset_t sigset;
                sigemptyset(&sigset);
                sigaddset(&sigset, SIGCHLD);

                // Wait for each forked child to finish
                sigprocmask(SIG_SETMASK, &sigset, NULL);

                {
                    // TODO: wait on a list of pids
                    int status;
                    for (int k = 0; k < numCommandsToWaitFor; ++k) {
                        wait(&status);
                    }
                }

                // Allow SIGCHLD signals to be processed again, signals received
                // during critical section will now be processed.
                sigprocmask(SIG_UNBLOCK, &sigset, NULL);
            }

            i += numPipedCommands;
        }
    }

    // Done executing commands, rewire stdin and stdout in main
    // process
    dup2(stdinCopy, 0);
    dup2(stdoutCopy, 1);
    close(stdinCopy);
    close(stdoutCopy);
}
