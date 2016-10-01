#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command.h"

/**
 * Returns 1 if the token is a command separator and 0 otherwise.
 *
 * @param   token   char *, pointer to NULL-terminated string.
 * @return          int, 1 if the token is a command separator and 0 otherwise.
 */
int separator(const char *token)
{
    int i = 0;
    char *commandSeparators[] = {SEP_PIPE, SEP_CON, SEP_SEQ, NULL};

    for (int i = 0; commandSeparators[i] != NULL; ++i) {
        if (strcmp(commandSeparators[i], token) == 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * This function searches the given array of tokens (from cp->first to cp->last)
 * and attempts to find the standard input redirection symbol "<" or the
 * standard output redirection symbol ">". Once found, the token following the
 * redirection symbol is treated as the redirection file name and is assigned to
 * either cp->stdin_file (if input redirection) or cp->stdout_file (if ouput
 * redirection).
 *
 * @todo Multiple redirections - currently uses last found.
 *
 * @param   token   const char *[], array of tokens.
 * @param   cp      command_t *, pointer to command struct.
 * @return          int, 0 if no error,
 *                       -1 if redirect symbol at end of input.
 */
int searchRedirection(char *token[], command_t *cp)
{
    if (cp != NULL)
    {
        for (int i = cp->first; i <= cp->last; ++i)
        {
            if (strcmp(token[i], REDIR_IN) == 0)
            {
                // Redirection token at end of input
                if (i == cp->last) {
                    return -1;
                } else {
                    cp->stdin_file = token[i + 1];
                    ++i;
                }
            } else if (strcmp(token[i], REDIR_OUT) == 0)
            {
                // Redirection token at end of input
                if (i == cp->last) {
                    return -1;
                } else {
                    cp->stdout_file = token[i + 1];
                    ++i;
                }
            }
        }
    }

    return 0;
}

/**
 * Dynamically allocates memory for the cp->argv property.
 *
 * @param   token   char *[], array of tokens.
 * @param   cp      command_t *, pointer to command_t structure to fill.
 */
void buildCommandArgumentArray(char *token[], command_t *cp)
{
    unsigned int n = (cp->last - cp->first + 1) // number of tokens in command
        - (cp->stdin_file == NULL ? 0 : 2)      // remove two tokens for stdin redirection
        - (cp->stdout_file == NULL ? 0 : 2)     // remove two tokens for stdout redirection
        + 1; // last element in argv must be NULL

    cp->argv = realloc(cp->argv, sizeof(char *) * n);

    int k = 0;
    for (int i = cp->first; i <= cp->last; ++i) {
        if (strcmp(token[i], REDIR_OUT) == 0 || strcmp(token[i], REDIR_IN) == 0) {
            ++i; // skip off the std(in/out) redirection
        } else {
            cp->argv[k] = token[i];
            ++k;
        }
    }
    cp->argv[k] = NULL;
}

int separateCommands(char *token[], int numTokens, command_t command[])
{
    // If empty command line
    if (numTokens == 0) {
        return 0;
    }

    // If first token is command separator
    if (separator(token[0])) {
        return -3;
    }

    int first = 0;
    int last = 0;
    const char *sep = SEP_SEQ;

    // Command index
    int c = 0;

    for (int i = 0; i < numTokens; ++i) {
        last = i;
        if (separator(token[i])) {
            sep = token[i];

            if (first == last) {  // Two consecutive separators
                return -2;
            }

            command[c].first = first;
            command[c].last = last - 1;
            command[c].sep = sep;
            ++c;

            // Advance first index
            first = i + 1;
        } else if (i == numTokens - 1) { // Last token is not a separator
            // Add sequence separator
            sep = SEP_SEQ;

            if (first == last && separator(token[first])) {  // Two consecutive separators
                return -2;
            }

            command[c].first = first;
            command[c].last = last;
            command[c].sep = sep;
            ++c;

            // Advance first index
            first = i + 1;
        }
    }

    // Check the last token of the last command
    if (strcmp(token[last], SEP_PIPE) == 0) { // last token is pipe separator
        return -4;
    }

    int numCommands = c;

    for (int i = 0; i < numCommands; ++i) {
        // Search for redirection symbols
        int err = searchRedirection(token, &(command[i]));
        // Redirection operator at end of command (no file specified)
        if (err == -1) {
            return -5;
        }

        // Build argv for each command.
        buildCommandArgumentArray(token, &(command[i]));
    }

    return numCommands;
}
