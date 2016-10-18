#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"

////////////////////////////////////////////////////////////////////////////////
/// Returns 1 if the token is a command separator and 0 otherwise.
///
/// @param   token   char *, pointer to NULL-terminated string.
/// @return          int, 1 if the token is a command separator and 0 otherwise.
////////////////////////////////////////////////////////////////////////////////
int separator(const char *token)
{
    char *commandSeparators[] = {SEP_PIPE, SEP_CON, SEP_SEQ, NULL};

    for (int i = 0; commandSeparators[i] != NULL; ++i) {
        if (strcmp(commandSeparators[i], token) == 0) {
            return 1;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// This function searches the given array of tokens (from cp->first to
/// cp->last) and attempts to find the standard input redirection symbol "<" or
/// the standard output redirection symbol ">". Once found, the token following
/// the redirection symbol is treated as the redirection file name and is
/// assigned to either cp->stdin_file (if input redirection) or cp->stdout_file
/// (if ouput redirection).
///
/// @todo Multiple redirections - currently uses last found (same as bash).
///
/// @param   token   const char *[], array of tokens.
/// @param   cp      command_t *, pointer to command struct.
/// @return          int, 0 if no error,
///                       -1 if redirect symbol at end of input,
///                       -2 if ambiguous redirect.
////////////////////////////////////////////////////////////////////////////////
int searchRedirection(char *token[], command_t *cp)
{
    if (cp != NULL) {
        for (int i = cp->first; i <= cp->last; ++i) {
            if (strcmp(token[i], REDIR_IN) == 0 ||
                strcmp(token[i], REDIR_OUT) == 0) {

                // Check that redirection symbol isn't last token in command
                if (i == cp->last) {
                    printf(
                        "sane: syntax error, expected path after token '%s'\n",
                        token[i]);
                    return -1;
                }

                // Check for ambiguity
                // If requested inpath/outpath contains wildcard characters and
                // is ambiguous (glob returns more than 1 path), fail
                glob_t globResult;
                glob(token[i + 1], 0, NULL, &globResult);

                if (globResult.gl_pathc > 1) {
                    // Skip command
                    printf("sane: %s: ambiguous redirect\n", token[i + 1]);
                    return -2;
                }

                globfree(&globResult);

                // Else, handle redirection
                if (strcmp(token[i], REDIR_IN) == 0) {
                    cp->stdin_file = token[i + 1];
                    ++i;
                } else if (strcmp(token[i], REDIR_OUT) == 0) {
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
    // TODO: Doesn't work properly if more than two redirection operators used
    // from first to last
    // continue until find < > | & ;, go back one token, this is the string to
    // glob
    // For each token excluding first, glob and expand.
    // Alloc an array to fit all, use that array as argv
    unsigned int n = (cp->last - cp->first + 1) // number of tokens in command
                     - (cp->stdin_file == NULL
                            ? 0
                            : 2) // remove two tokens for stdin redirection
                     - (cp->stdout_file == NULL
                            ? 0
                            : 2) // remove two tokens for stdout redirection
                     + 1;        // last element in argv must be NULL

    cp->argv = realloc(cp->argv, sizeof(char *) * n);

    int k = 0;
    for (int i = cp->first; i <= cp->last; ++i) {
        if (strcmp(token[i], REDIR_OUT) == 0 ||
            strcmp(token[i], REDIR_IN) == 0) {
            ++i; // don't include the std(in/out) redirection tokens
        } else {
            cp->argv[k] = token[i];
            ++k;
        }
    }
    cp->argv[k] = NULL;
}

int separateCommands(char *token[], int numTokens, command_t command[])
{
    int result = 0;

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

            if (first == last) { // Two consecutive separators
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

            if (first == last &&
                separator(token[first])) { // Two consecutive separators
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

    int numCommands = c;

    for (int i = 0; i < numCommands; ++i) {
        // Search for redirection symbols
        int err = searchRedirection(token, &(command[i]));
        // Redirection operator at end of command (no file specified)
        if (err == -1) {
            return -5;
        } else if (err == -2) {
            result = -6;
        }

        // Build argv for each command.
        buildCommandArgumentArray(token, &(command[i]));
    }

    // Check the last token of the last command
    if (strcmp(token[last], SEP_PIPE) == 0) { // last token is pipe separator
        printf("sane: syntax error, last token should not be pipe\n");
        return -4;
    }


    return (result == 0 ? numCommands : result);
}
