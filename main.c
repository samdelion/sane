#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"
#include "sane.h"
#include "token.h"

// Allow at least 64 characters for each token
#define INPUT_LINE_SIZE (64 * MAX_NUM_TOKENS)

int sane_shouldQuit = 0;

void sane_handleSigkill(int signo)
{
}

// SIGUSR1 is sent when "exit" inbuilt command is used
void sane_handleSiguser1(int signo)
{
    sane_shouldQuit = 1;
}

// SIGCHLD is sent when a child process finishes
void sane_handleSigchld(int signo)
{
    // TODO: Save errno? - waitpid may change errno
    /* printf("Caught sigchld \n"); */
    int more = 1;
    pid_t pid;
    int status;

    // Continue claiming zombie processes until no more to claim
    while (more) {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) {
            more = 0;
        }
    }
}

void setupChildSignalHandler()
{
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sane_handleSigchld;
    // Initialize mask to contain all signals - all signals are blocked during
    // the execution of the handler
    sigfillset(&(act.sa_mask));

    // Assign handler structure to SIGCHLD
    if (sigaction(SIGCHLD, &act, NULL) != 0) {
        perror("sane: sigaction");
        exit(1);
    }
}

void setupUser1SignalHandler()
{
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sane_handleSiguser1;
    // Initialize mask to contain all signals - all signals are blocked during
    // the execution of the handler
    sigfillset(&(act.sa_mask));

    // Assign handler structure to SIGCHLD
    if (sigaction(SIGUSR1, &act, NULL) != 0) {
        perror("sane: sigaction");
        exit(1);
    }
}

void setupKillSignalHandler()
{
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sane_handleSigkill;
    // Initialize mask to contain all signals - all signals are blocked during
    // the execution of the handler
    sigfillset(&(act.sa_mask));

    // Assign handler structure to SIGCHLD
    if (sigaction(SIGINT, &act, NULL) != 0) {
        perror("sane: sigaction");
        exit(1);
    }
    // Assign handler structure to SIGCHLD
    if (sigaction(SIGQUIT, &act, NULL) != 0) {
        perror("sane: sigaction");
        exit(1);
    }
    // Assign handler structure to SIGCHLD
    if (sigaction(SIGTSTP, &act, NULL) != 0) {
        perror("sane: sigaction");
        exit(1);
    }
}

void setupSignalHandlers()
{
    setupChildSignalHandler();
    setupUser1SignalHandler();
    setupKillSignalHandler();
    // TODO setup_handler(int sig, fnptr handler)
}

int main(int argc, char **argv)
{
    // Initialize shell, check if ok
    if (sane_init() == 0) {
        char *inputLine = (char *)malloc(INPUT_LINE_SIZE * sizeof(char));
        char *inputPtr = NULL;

        setupSignalHandlers();

        while (!(sane_shouldQuit)) {
            memset(inputLine, 0, INPUT_LINE_SIZE);
            printf("%s ", sane_getPrompt());

            // Get input buffer from stdin, if getting input fails due to
            // interruption from signal handler, try again.
            do {
                inputPtr = fgets(inputLine, INPUT_LINE_SIZE, stdin);
            } while (inputPtr == NULL && errno == EINTR);

            // TODO: Remove. For now, I would like to know cases where this
            // is true
            assert(inputPtr != NULL && "Couldn't get input!\n");

            if (inputPtr != NULL) {
                // Remove newline at end of input buffer
                inputLine[strcspn(inputLine, "\n")] = 0;

                char *token[MAX_NUM_TOKENS];
                int numTokens = tokenise(inputLine, token);
                if (numTokens == -1) {
                    fprintf(stderr, "sane: number of tokens provided exceeds "
                                    "MAX_NUM_TOKENS\n");
                } else if (numTokens == -2) {
                    fprintf(stderr, "sane: string not closed\n");
                }

                command_t command[MAX_NUM_COMMANDS];
                // Reset command array
                memset(command, 0, MAX_NUM_COMMANDS * sizeof(command_t));
                int numCommands = separateCommands(token, numTokens, command);
                if (numCommands == -1) {
                    fprintf(stderr, "sane: command array is too small for "
                                    "all commands\n");
                } else if (numCommands == -2) {
                    fprintf(stderr, "sane: at least two successive "
                                    "commands are separated by more than "
                                    "one command separator\n");
                } else if (numCommands == -3) {
                    fprintf(stderr, "sane: first token is command separator\n");
                } else if (numCommands == -4) {
                    fprintf(stderr, "sane: last command followed by "
                                    "command separator '|'\n");
                }

                if (numCommands > 0) {
                    sane_execute(numCommands, command);

                    freeCommands(command, numCommands);
                }
            }
        }

        free(inputLine);
        // Shutdown shell
        sane_shutdown();
    } else {
        fprintf(stderr, "sane: initialization of shell failed\n");
    }

    return 0;
}
