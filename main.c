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

// Allow at least 256 characters for each token
#define INPUT_LINE_SIZE (256 * MAX_NUM_TOKENS)

int sane_shouldQuit = 0;

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
    // Setup sigchld handler
    sigset_t s;
    if (sigemptyset(&s) == 0) {
        sigaddset(&s, SIGCHLD);
    }

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
    // Setup sigchld handler
    sigset_t s;
    if (sigemptyset(&s) == 0) {
        sigaddset(&s, SIGUSR1);
    }

    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sane_handleSiguser1;
    // Initialize mask to contain no signals - no signals are blocked during
    // the execution of the handler
    act.sa_mask = 0;

    // Assign handler structure to SIGCHLD
    if (sigaction(SIGUSR1, &act, NULL) != 0) {
        perror("sane: sigaction");
        exit(1);
    }
}

void setupSignalHandlers()
{
    setupChildSignalHandler();
    setupUser1SignalHandler();
}

int main(int argc, char **argv)
{
    // Don't catch SIGQUIT (Ctrl+\), SIGINT (Ctrl+c), SIGTSTP (Ctrl+z) signals
    // during this critical section
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGQUIT);
    sigaddset(&sigset, SIGTSTP);

    sigprocmask(SIG_SETMASK, &sigset, NULL);

    {
        char *inputLine = (char *)malloc(INPUT_LINE_SIZE * sizeof(char));
        char *inputPtr = NULL;

        setupSignalHandlers();

        while (!(sane_shouldQuit)) {
            memset(inputLine, 0, INPUT_LINE_SIZE);
            printf("%% ");

            // Get input buffer from stdin, if getting input fails due to
            // interruption from signal handler, try again.
            do {
                inputPtr = fgets(inputLine, INPUT_LINE_SIZE, stdin);
            } while (inputPtr == NULL && errno == EINTR);

            // TODO: Remove. For now, I would like to know cases where this is
            // true
            assert(inputPtr != NULL && "Couldn't get input!\n");

            if (inputPtr != NULL) {
                // Remove newline at end of input buffer
                inputLine[strcspn(inputLine, "\n")] = 0;

                char *token[MAX_NUM_TOKENS];
                int numTokens = tokenise(inputLine, token);
                if (numTokens == -1) {
                    fprintf(stderr, "sane: Number of tokens provided exceeds "
                                    "MAX_NUM_TOKENS\n");
                }
                if (numTokens == -2) {
                    fprintf(stderr, "sane: String not closed\n");
                }

                command_t command[MAX_NUM_COMMANDS];
                // Reset command array
                memset(command, 0, MAX_NUM_COMMANDS * sizeof(command_t));
                int numCommands = separateCommands(token, numTokens, command);

                if (numCommands > 0) {
                    sane_execute(numCommands, command);
                }
            }
        }

        free(inputLine);
    }

    // Allow SIGINT, SIGQUIT, SIGTSTP signals to be processed again, signals
    // received during critical section will now be processed.
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);

    return 0;
}
