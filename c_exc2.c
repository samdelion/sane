#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "command.h"
#include "token.h"
#include "sane.h"

#define INPUT_LINE_SIZE 1024

#define EXIT_CMD "exit"

int main(int argc, char **argv)
{
    char inputLine[INPUT_LINE_SIZE];
    pid_t pid;

    while (1) {
        memset(inputLine, 0, INPUT_LINE_SIZE);
        printf("%% ");
        fgets(inputLine, INPUT_LINE_SIZE, stdin);
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

            /* // For each command */
            /* for (int i = 0; i < err; ++i) { */
            /*     // Print out the tokens for each command */
            /*     for (int j = command[i].first; j <= command[i].last; ++j) {
             */
            /*         printf("%s", token[j]); */

            /*         // Don't print space after last token in command */
            /*         if (j < command[i].last) { */
            /*             printf(" "); */
            /*         } */
            /*     } */
            /*     printf("\tredir: %s\t%s", command[i].stdin_file,
             * command[i].stdout_file); */
            /*     printf("\targv:"); */
            /*     for (int j = 0; command[i].argv[j] != NULL; ++j) { */
            /*         printf("%s ", command[i].argv[j]); */
            /*     } */
            /*     printf("\n"); */
            if (err > 0) {
                sane_execute(err, command);
            }
        }
    }

    return 0;
}
