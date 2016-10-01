#include <string.h>
#include <ctype.h>

#include "token.h"

int tokenise(char *inputLine, char *token[]) {
    int numTokens = 0;

    char *it = inputLine;
    while (*it && numTokens < MAX_NUM_TOKENS) {
        // Skip characters we aren't interested in such as space, tab, newline
        while (*it && ((*it <= 32) || (*it > 126))) {
            ++it;
        }

        if (*it) {
            // Assign start address of token to array
            token[numTokens] = it;
            ++numTokens;

            // Skip characters we are interested in ('a', 'b', '!', etc.) (not including space, tab, newline etc.)
            while (*it && ((*it > 32) && (*it <= 126))) {
                ++it;
            }

            // Put null-terminator at end of token
            if (*it) {
                *it = '\0';
                // Advance iterator one character
                ++it;
            }
        }
        
        if (numTokens == MAX_NUM_TOKENS) { // We won't have space for the next toke n
            return -1;
        }
    }

    return numTokens;
}
