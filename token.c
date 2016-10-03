#include <string.h>
#include <ctype.h>

#include "token.h"

typedef enum { TOKEN_DEFAULT, TOKEN_STRING } tokeniserState_t;

tokeniserState_t sane_currentTokeniserState = TOKEN_DEFAULT;

int tokenise(char *inputLine, char *token[])
{
    int numTokens = 0;

    char *it = inputLine;
    while (*it && numTokens < MAX_NUM_TOKENS) {
        // Skip characters we aren't interested in such as space, tab, newline
        while (*it && ((*it <= 32) || (*it > 126))) {
            ++it;
        }

        if (*it) {
            if (*it == '"' | *it == '\'') {
                // Quote type is either " or '
                char quoteType = (*it == '"') ? '"' : '\'';
                sane_currentTokeniserState = TOKEN_STRING;

                // Assign start address of string (after quote) to array
                token[numTokens] = ++it;
                ++numTokens;

                // Don't stop consuming characters until we find end of input or
                // end of string
                while (*it && *it != quoteType) {
                    ++it;
                }

                // If we reached end of input before string was closed, return
                // error
                if (!(*it)) {
                    return -2;
                }
            } else {
                // Assign start address of token to array
                token[numTokens] = it;
                ++numTokens;

                // Skip characters we are interested in ('a', 'b', '!', etc.)
                // (not
                // including space, tab, newline etc.)
                while (*it && ((*it > 32) && (*it <= 126))) {
                    ++it;
                }
            }

            // Put null-terminator at end of token
            if (*it) {
                *it = '\0';
                // Advance iterator one character
                ++it;
            }
        }

        // If we've reached maximum number of tokens and still have more input
        // to tokenise, return error
        if (numTokens == MAX_NUM_TOKENS && *it) {
            return -1;
        }
    }

    return numTokens;
}
