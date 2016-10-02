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
#include "token.h"
#include "sane.h"

#define INPUT_LINE_SIZE 1024

#define EXIT_CMD "exit"

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

int setupSignalHandlers()
{
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

    return 0;
}

int main(int argc, char **argv)
{
    char inputLine[INPUT_LINE_SIZE];
    char *inputPtr = NULL;
    pid_t pid;

    setupSignalHandlers();

    while (1) {
        memset(inputLine, 0, INPUT_LINE_SIZE);
        printf("%% ");

        // Get input buffer from stdin, if getting input fails due to
        // interruption from signal handler, try again.
        do {
            inputPtr = fgets(inputLine, INPUT_LINE_SIZE, stdin);
        } while (inputPtr == NULL && errno == EINTR);

        // TODO: Remove. For now, I would like to know cases where this is true
        assert(inputPtr != NULL && "Couldn't get input!\n");

        if (inputPtr != NULL) {
            // Remove newline at end of input buffer
            inputLine[strcspn(inputLine, "\n")] = 0;
            if (strncmp(inputLine, EXIT_CMD, strlen(EXIT_CMD)) == 0) {
                break;
            } else {
                char *token[MAX_NUM_TOKENS];
                int numTokens = tokenise(inputLine, token);

                command_t command[MAX_NUM_COMMANDS];
                // Reset command array
                memset(command, 0, MAX_NUM_COMMANDS * sizeof(command_t));
                int err = separateCommands(token, numTokens, command);

                if (err > 0) {
                    sane_execute(err, command);
                }
            }
        }
    }

    return 0;
}
