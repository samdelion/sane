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
                    fprintf(
                        stderr,
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
                    fprintf(stderr, "sane: %s: ambiguous redirect\n",
                            token[i + 1]);
                    return -2;
                }

                if (globResult.gl_pathc > 0) { // Exactly one path matched
                    // Else, handle redirection
                    if (strcmp(token[i], REDIR_IN) == 0) {
                        size_t len = strlen(globResult.gl_pathv[0]) +
                                     1; // + 1 for NULL-terminator
                        char *tmp = (char *)malloc(sizeof(char) * len);
                        memset(tmp, '\0', len);
                        cp->stdin_file = strcpy(tmp, globResult.gl_pathv[0]);
                        ++i;
                    } else if (strcmp(token[i], REDIR_OUT) == 0) {
                        size_t len = strlen(globResult.gl_pathv[0]) +
                                     1; // + 1 for NULL-terminator
                        char *tmp = (char *)malloc(sizeof(char) * len);
                        memset(tmp, '\0', len);
                        cp->stdout_file = strcpy(tmp, globResult.gl_pathv[0]);
                        ++i;
                    }
                } else { // No paths matched
                    // Just use token
                    if (strcmp(token[i], REDIR_IN) == 0) {
                        size_t len =
                            strlen(token[i + 1]) + 1; // + 1 for NULL-terminator
                        char *tmp = (char *)malloc(sizeof(char) * len);
                        memset(tmp, '\0', len);
                        cp->stdin_file = strcpy(tmp, token[i + 1]);
                        ++i;
                    } else if (strcmp(token[i], REDIR_OUT) == 0) {
                        size_t len =
                            strlen(token[i + 1]) + 1; // + 1 for NULL-terminator
                        char *tmp = (char *)malloc(sizeof(char) * len);
                        memset(tmp, '\0', len);
                        cp->stdout_file = strcpy(tmp, token[i + 1]);
                        ++i;
                    }
                }

                globfree(&globResult);
            }
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Dynamically allocates memory for the cp->argv property.
///
/// @param   token   char *[], array of tokens.
/// @param   cp      command_t *, pointer to command_t structure to fill.
////////////////////////////////////////////////////////////////////////////////
void buildCommandArgumentArray(char *token[], command_t *cp)
{
    int ii = cp->first;
    for (; ii <= cp->last; ++ii) {
        if (strcmp(token[ii], REDIR_OUT) == 0 ||
            strcmp(token[ii], REDIR_IN) == 0 || separator(token[ii])) {
            /* printf("stop: %s\n", token[ii]); */
            break;
        }
    }

    unsigned int n = (ii - cp->first) + 1; // Last element in argv must be NULL

    cp->argv = realloc(cp->argv, sizeof(char *) * n);
    // Don't match any wildcards in first token (command to execute)
    size_t len1 = strlen(token[cp->first]) + 1;
    char *tmp1 = (char *)malloc(sizeof(char) * len1);
    memset(tmp1, '\0', len1);
    cp->argv[0] = strcpy(tmp1, token[cp->first]);

    int offset = 1;
    for (int i = cp->first + 1; i < ii; ++i) {
        glob_t globResult;
        glob(token[i], GLOB_TILDE, NULL, &globResult);

        if (globResult.gl_pathc > 0) {
            n += globResult.gl_pathc - 1; // already counted one of the paths

            cp->argv = realloc(cp->argv, sizeof(char *) * n);
            for (int j = 0; j < globResult.gl_pathc; ++j) {
                size_t len = strlen(globResult.gl_pathv[j]) +
                             1; // +1 for NULL terminator
                // Look thru array and take decrement length if find '\'
                // (escape
                // character)
                for (int k = 0; k < strlen(globResult.gl_pathv[j]); ++k) {
                    if (globResult.gl_pathv[j][k] == '\\') {
                        --len;
                    }
                }
                char *tmp = (char *)malloc(sizeof(char) * len);
                memset(tmp, '\0', len);

                int k = 0;
                for (char *it = globResult.gl_pathv[j]; *it != '\0'; ++it) {
                    // Handle escape characters
                    if (*it == '\\') {
                        // Make sure to ignore next character
                        tmp[k] = *(it + 1);
                        ++it;
                    } else {
                        tmp[k] = *it;
                    }
                    ++k;
                }
                cp->argv[j + offset] = tmp;
            }

            offset += globResult.gl_pathc;
        } else {
            size_t len = strlen(token[i]) + 1; // +1 for NULL terminator
            for (int k = 0; k < strlen(token[i]); ++k) {
                if (token[i][k] == '\\') {
                    --len;
                }
            }
            char *tmp = (char *)malloc(sizeof(char) * len);
            memset(tmp, '\0', len);

            int k = 0;
            for (char *it = token[i]; *it != '\0'; ++it) {
                // Handle escape characters
                if (*it == '\\') {
                    // Make sure to ignore next character
                    tmp[k] = *(it + 1);
                    ++it;
                } else {
                    tmp[k] = *it;
                }
                ++k;
            }

            cp->argv[offset] = tmp;
            ++offset;
        }

        globfree(&globResult);
    }
    cp->argv[n - 1] = NULL;
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
        fprintf(stderr, "sane: syntax error, last token should not be pipe\n");
        return -4;
    }


    return (result == 0 ? numCommands : result);
}

void freeCommands(command_t command[], int numCommands)
{
    for (unsigned int i = 0; i < numCommands; ++i) {
        // Free input redirection
        if (command[i].stdin_file != NULL) {
            free(command[i].stdin_file);
            command[i].stdin_file = NULL;
        }
        // Free output redirection
        if (command[i].stdout_file != NULL) {
            free(command[i].stdout_file);
            command[i].stdout_file = NULL;
        }

        if (command[i].argv != NULL) {
            // Free each token
            for (int j = 0; command[i].argv[j] != NULL; ++j) {
                free(command[i].argv[j]);
            }
            // Free array
            free(command[i].argv);
            command[i].argv = NULL;
        }
    }
}
